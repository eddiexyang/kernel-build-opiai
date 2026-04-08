/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
         copyright notice, this list of conditions and the following
         disclaimer in the documentation and/or other materials provided
         with the distribution.
       * Neither the name of The Linux Foundation nor the names of its
         contributors may be used to endorse or promote products derived
         from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Drop-in replacements for Huawei libc_sec functions used in this file. */
#ifndef EOK
#define EOK 0
#endif
typedef int errno_t;

static inline errno_t strncpy_s(char *dest, size_t dmax, const char *src, size_t slen)
{
    if (!dest || dmax == 0) return -1;
    if (!src) { dest[0] = '\0'; return -1; }
    size_t copy = slen < dmax - 1 ? slen : dmax - 1;
    memcpy(dest, src, copy);
    dest[copy] = '\0';
    return EOK;
}

static inline errno_t strncat_s(char *dest, size_t dmax, const char *src, size_t slen)
{
    if (!dest || dmax == 0) return -1;
    size_t dlen = strlen(dest);
    if (dlen >= dmax) return -1;
    size_t remain = dmax - dlen - 1;
    size_t copy = slen < remain ? slen : remain;
    if (!src) return -1;
    memcpy(dest + dlen, src, copy);
    dest[dlen + copy] = '\0';
    return EOK;
}
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <limits.h>

#define HSDT_MAGIC "HSDT" /* Master DTB magic */
#define HSDT_VERSION 1    /* HSDT version */

#define HSDT_DT_TAG "hisi,boardid = <"

#define PAGE_SIZE_DEF 2048
#define PAGE_SIZE_MAX (1024 * 1024)

#define log_err(x...) (void)printf(x)
#define log_info(x...) (void)printf(x)
#define log_dbg(x...) do { \
    if (verbose != 0) { \
        (void)printf(x); \
    } \
} while (0)

#define COPY_BLK 1024 /* File copy block size */

#define RC_SUCCESS 0
#define RC_ERROR (-1)

#define MIN_BOARD_ADC_COUNT 4
#define MAX_BOARD_ADC_COUNT 8

typedef struct dt_entry_t {
    uint8_t boardid[MIN_BOARD_ADC_COUNT];
    uint8_t reserved[4];
    uint32_t dtb_size;
    uint32_t vrl_size;
    uint32_t dtb_offset;
    uint32_t vrl_offset;
    uint64_t dtb_file;
    uint64_t vrl_file;
} dt_entry_t;

typedef struct chipInfo_t {
    struct dt_entry_t dt_entry;
    struct chipInfo_t *prev;
    struct chipInfo_t *next;
} chipInfo_t;

typedef struct dt_table_t {
    uint32_t magic;
    uint32_t version;
    uint32_t num_entries;
} dt_table_t;

struct chipInfo_t *chip_list;

static const char *input_dir = NULL;
static const char *output_file = NULL;
static const char *dtc_path = NULL;
int verbose;
static uint32_t page_size = PAGE_SIZE_DEF;

static void print_help(void)
{
    log_info("dtbTool [options] -o <output file> <input DTB path>\n");
    log_info("  options:\n");
    log_info("  --output-file/-o     output file\n");
    log_info("  --dtc-path/-p        path to dtc\n");
    log_info("  --page-size/-s       page size in bytes\n");
    log_info("  --verbose/-v         verbose\n");
    log_info("  --help/-h            this help screen\n");
}

static long str_to_num(const char *str)
{
    char *endptr = NULL;
    long num;
    const int number_base = 10;

    if (str == NULL) {
        return RC_ERROR;
    }
    errno = 0;
    num = strtol(str, &endptr, number_base);
    if ((endptr == str) || (*endptr != '\0')) {
        return RC_ERROR;
    } else if (((num == LONG_MIN) || (num == LONG_MAX)) && (errno == ERANGE)) {
        return RC_ERROR;
    } else {
        return num;
    }
}

static inline void safe_free(void *ptr)
{
    if (ptr != NULL) {
        free(ptr);
        ptr = NULL;
    }
}

static int parse_commandline(int argc, char * const argv[])
{
    int c;
    long num;
    bool parse_result = true;
    struct option long_options[] = {
        {"output-file", 1, NULL, 'o'},
        {"dtc-path",    1, NULL, 'p'},
        {"page-size",   1, NULL, 's'},
        {"verbose",     0, NULL, 'v'},
        {"help",        0, NULL, 'h'},
        {NULL,          0, NULL, 0}
    };

    while (1) {
        c = getopt_long(argc, argv, "-o:p:s:vh", long_options, NULL);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 1:
                if (input_dir == NULL) {
                    input_dir = optarg;
                }
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'p':
                dtc_path = optarg;
                break;
            case 's':
                num = str_to_num(optarg);
                if ((num <= 0) || (num > (PAGE_SIZE_MAX))) {
                    log_err("Invalid page size (> 0 and <=1MB, page_size = %lu\n", num);
                    return RC_ERROR;
                }
                page_size = (uint32_t)num;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'h':
                parse_result = false;
                break;
            default:
                parse_result = false;
                break;
        }
    }
    if (!parse_result) {
        return RC_ERROR;
    }
    if (output_file == NULL) {
        log_err("Output file must be specified\n");
        return RC_ERROR;
    }

    if (input_dir == NULL) {
        input_dir = "./";
    }

    if (dtc_path == NULL) {
        dtc_path = "";
    }

    return RC_SUCCESS;
}

/* Unique entry sorted list add (by boardid sort ascending) */
static int chip_add(struct chipInfo_t *c)
{
    struct chipInfo_t *x = chip_list;

    if (chip_list == NULL) {
        chip_list = c;
        c->next = NULL;
        c->prev = NULL;
        return RC_SUCCESS;
    }

    while (1) {
        if ((c->dt_entry.boardid[0] < x->dt_entry.boardid[0]) ||
            ((c->dt_entry.boardid[0] == x->dt_entry.boardid[0]) && (c->dt_entry.boardid[1] < x->dt_entry.boardid[1])) ||
            ((c->dt_entry.boardid[1] == x->dt_entry.boardid[1]) && (c->dt_entry.boardid[2] < x->dt_entry.boardid[2]))) {
            if (x->prev == NULL) {
                c->next = x;
                c->prev = NULL;
                x->prev = c;
                chip_list = c;
                break;
            } else {
                c->next = x;
                c->prev = x->prev;
                x->prev->next = c;
                x->prev = c;
                break;
            }
        }
        if ((c->dt_entry.boardid[0] == x->dt_entry.boardid[0]) && (c->dt_entry.boardid[1] == x->dt_entry.boardid[1]) &&
            (c->dt_entry.boardid[2] == x->dt_entry.boardid[2]) && (c->dt_entry.boardid[3] == x->dt_entry.boardid[3])) {
            return RC_ERROR; /* duplicate */
        }
        if (x->next == NULL) {
            c->prev = x;
            c->next = NULL;
            x->next = c;
            break;
        }
        x = x->next;
    }
    return RC_SUCCESS;
}

static void chip_deleteall(void)
{
    struct chipInfo_t *c = chip_list, *t;

    while (c != NULL) {
        t = c;
        c = c->next;
        safe_free((void *)t->dt_entry.dtb_file);
        safe_free((void *)t->dt_entry.vrl_file);
        safe_free(t);
    }
}

/* Extract 'hisi,boardid' parameter triplet from DTB
      hisi,boardid = <xxx>;
 */
static struct chipInfo_t *getChipInfo(const char *filename)
{
    const char str1[] = "dtc -I dtb -O dts \"";
    const char str2[] = "\" 2>&1";
    char *buf, *pos;
    char *line = NULL;
    size_t line_size;
    FILE *pfile;
    size_t llen;
    uint8_t boardid_size = 0;
    uint32_t boardid_val = 0;
    uint32_t boardid[MAX_BOARD_ADC_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};
    struct chipInfo_t *chip = NULL;
    char *tok, *sptr = NULL;
    uint8_t i;
    bool isOldDts;

    line_size = 1024;
    line = (char *)malloc(line_size);
    if (line == NULL) {
        log_err("Out of memory\n");
        return NULL;
    }

    llen = sizeof(char) * (strlen(dtc_path) + strlen(str1) + strlen(str2) + strlen(filename) + 1UL);
    buf = (char *)malloc(llen);
    if (buf == NULL) {
        log_err("Out of memory\n");
        free(line);
        return NULL;
    }

    errno_t ret = strncpy_s(buf, llen, dtc_path, strlen(dtc_path));
    if (ret != EOK) {
        log_err("str copy fail with %d\n", ret);
        free(line);
        free(buf);
        return NULL;
    }
    ret = strncat_s(buf, llen, str1, strlen(str1));
    if (ret != EOK) {
        log_err("str cat fail with %d\n", ret);
        free(line);
        free(buf);
        return NULL;
    }
    ret = strncat_s(buf, llen, filename, strlen(filename));
    if (ret != EOK) {
        log_err("str cat fail with %d\n", ret);
        free(line);
        free(buf);
        return NULL;
    }
    ret = strncat_s(buf, llen, str2, strlen(str2));
    if (ret != EOK) {
        log_err("str cat fail with %d\n", ret);
        free(line);
        free(buf);
        return NULL;
    }

    pfile = popen(buf, "r");
    free(buf);

    if (pfile == NULL) {
        log_err("... skip, fail to decompile dtb\n");
    } else {
        /* Find "hisi,boardid = <" */
        while (getline(&line, &line_size, pfile) != -1) {
            pos = strstr(line, HSDT_DT_TAG);
            if (pos != NULL) {
                pos += strlen(HSDT_DT_TAG);
                boardid_size = 0;
                isOldDts = false;
                tok = strtok_r(pos, " \t", &sptr);
                while (tok != NULL) {
                    if (boardid_size >= MAX_BOARD_ADC_COUNT) {
                        log_err("Invalid Boardid: boardid length is exceeds limit.\n");
                        goto OUT;
                    }
                    errno = 0;
                    unsigned long num = strtoul(tok, NULL, 0);
                    if (((num == 0) && (errno != 0)) || (num == (ULONG_MAX))) {
                        log_err("Invalid Boardid: string to number fail with %lu.\n", num);
                        goto OUT;
                    }
                    boardid[boardid_size] = (uint32_t)num;

                    // If boardid in a~f, only support old mode.
                    if (boardid[boardid_size] > 9) {
                        log_err("Warning: exists boardid [%d] not in 0~9.\n", boardid[boardid_size]);
                        isOldDts = true;
                    }
                    boardid_size++;
                    tok = strtok_r(NULL, " \t", &sptr);
                }
                if ((boardid_size < MIN_BOARD_ADC_COUNT) || (isOldDts && (boardid_size > MIN_BOARD_ADC_COUNT))) {
                    log_err("Invalid Boardid: boardid length [%d] is not valid.\n", boardid_size);
                    goto OUT;
                }

                chip = (chipInfo_t *)malloc(sizeof(struct chipInfo_t));
                if (chip == NULL) {
                    log_err("Out of memory: malloc failed for chipInfo.\n");
                    goto OUT;
                }
                (void)memset(chip, 0, sizeof(struct chipInfo_t));

                if (boardid_size == MIN_BOARD_ADC_COUNT) {
                    for (i = 0; i < MIN_BOARD_ADC_COUNT; i++) {
                        chip->dt_entry.boardid[i] = (uint8_t)boardid[i];
                    }
                } else {
                    // set slotid
                    chip->dt_entry.boardid[0] = (uint8_t)((boardid[0] & 0xffU) | (1U << 7));

                    // set boardid
                    for (i = 1; i < boardid_size; i++) {
                        boardid_val = (boardid_val * 10U) + boardid[i];
                    }
                    chip->dt_entry.boardid[1] = (uint8_t)((boardid_val & 0xFF0000U) >> 16);
                    chip->dt_entry.boardid[2] = (uint8_t)((boardid_val & 0x00FF00U) >> 8);
                    chip->dt_entry.boardid[3] = (uint8_t)(boardid_val & 0x0000FFU);
                }
                chip->dt_entry.dtb_size = 0;
                chip->dt_entry.dtb_file = 0;
                chip->dt_entry.vrl_size = 0;
                chip->dt_entry.vrl_file = 0;
                chip->prev = NULL;
                chip->next = NULL;
            }
        }
    }

OUT:
    if (line != NULL) {
        free(line);
        line = NULL;
    }
    if (pfile != NULL) {
        (void)pclose(pfile);
        pfile = NULL;
    }

    return chip;
}

int main(int argc, char **argv)
{
    char buf[COPY_BLK];
    struct chipInfo_t *chip;
    struct dirent *dp;
    FILE *pInputFile;
    char *filename;
    char *dtb_vrl_filename;
    uint32_t padding;
    uint8_t *filler = NULL;
    uint32_t numBytesRead = 0;
    uint32_t totBytesRead = 0;
    int out_fd;
    size_t flen;
    int rc = RC_SUCCESS;
    uint32_t dtb_count = 0, dtb_offset = 0;
    ssize_t wrote = 0, vrl_offset = 0;
    struct stat st;
    uint32_t version = HSDT_VERSION;
    uint32_t dtb_size;
    ssize_t vrl_size;
    uint32_t expected = 0;
    const char *dtb_fname = ".dtb";

    log_info("DTB combiner:\n");

    if (parse_commandline(argc, argv) != RC_SUCCESS) {
        print_help();
        return RC_ERROR;
    }

    log_info("  Input directory: '%s'\n", input_dir);
    log_info("  Output file: '%s'\n", output_file);

    DIR *dir = opendir(input_dir);
    if (dir == NULL) {
        log_err("Failed to open input directory '%s'\n", input_dir);
        return RC_ERROR;
    }

    filler = (uint8_t *)malloc((size_t)page_size);
    if (filler == NULL) {
        log_err("Out of memory\n");
        (void)closedir(dir);
        return RC_ERROR;
    }
    (void)memset(filler, 0, (size_t)page_size);

    log_info("\nGenerating master DTB... \n");

    /* Open the .dtb files in the specified path, decompile and
     * extract "hisi,boardid" parameter
     */
    size_t dtb_flen = strlen(dtb_fname);
    while (1) {
        dp = readdir(dir);
        if (dp == NULL) {
            break;
        }
        flen = strlen(dp->d_name);
        if ((flen > dtb_flen) && (strncmp(&dp->d_name[flen - dtb_flen], dtb_fname, dtb_flen) == 0)) {
            flen = strlen(input_dir) + strlen(dp->d_name) + 1UL;
            filename = (char *)malloc(flen);
            if (filename == NULL) {
                log_err("Out of memory\n");
                rc = RC_ERROR;
                break;
            }
            errno_t ret = strncpy_s(filename, flen, input_dir, strlen(input_dir));
            if (ret != EOK) {
                safe_free(filename);
                rc = RC_ERROR;
                log_err("str copy fail with %d\n", ret);
                break;
            }
            ret = strncat_s(filename, flen, dp->d_name, strlen(dp->d_name));
            if (ret != EOK) {
                safe_free(filename);
                rc = RC_ERROR;
                log_err("str cat fail with %d\n", ret);
                break;
            }
            dtb_vrl_filename = (char *)malloc(flen);
            if (dtb_vrl_filename == NULL) {
                safe_free(filename);
                rc = RC_ERROR;
                log_err("Out of memory\n");
                break;
            }
            ret = strncpy_s(dtb_vrl_filename, flen, filename, flen);
            if (ret != EOK) {
                safe_free(filename);
                safe_free(dtb_vrl_filename);
                rc = RC_ERROR;
                log_err("str copy fail with %d\n", ret);
                break;
            }
            ret = strncpy_s(dtb_vrl_filename + (flen - dtb_flen), dtb_flen, "bin", dtb_flen);
            if (ret != EOK) {
                safe_free(filename);
                safe_free(dtb_vrl_filename);
                rc = RC_ERROR;
                log_err("str copy fail with %d\n", ret);
                break;
            }
            log_info("==== %s\n", filename);
            if (stat(filename, &st) == 0) {
                /* Extract the dtb file, decompile boardid parameter and
                 * make chip struct to add to a list
                 */
                if (!S_ISREG((signed short)st.st_mode) || (st.st_size == 0)) {
                    safe_free(filename);
                    safe_free(dtb_vrl_filename);
                    log_err("skip, it's not a regular file or failed to get DTB size\n");
                    goto cleanup;
                }

                chip = getChipInfo(filename);
                if (chip == NULL) {
                    safe_free(filename);
                    safe_free(dtb_vrl_filename);
                    log_err("skip, failed to scan for '%s' tag\n", HSDT_DT_TAG);
                    goto cleanup;
                }

                rc = chip_add(chip);
                if (rc != RC_SUCCESS) {
                    safe_free(filename);
                    safe_free(dtb_vrl_filename);
                    log_err("Error: duplicate boardid info!\n");
                    goto cleanup;
                }

                dtb_count++;

                /* Get dtb file name and
                 * Calculate dtb file size including page padding
                 */
                chip->dt_entry.dtb_size = (uint32_t)((uint32_t)st.st_size +
                    (page_size - ((uint32_t)st.st_size % page_size)));
                chip->dt_entry.dtb_file = (uint64_t)filename;
                chip->dt_entry.vrl_file = (uint64_t)dtb_vrl_filename;
            } else {
                safe_free(filename);
                safe_free(dtb_vrl_filename);
                log_err("Error: failed to get file state!\n");
                goto cleanup;
            }
        }
    }
    log_info("=> Found %u unique DTB(s)\n", dtb_count);

    if (dtb_count == 0) {
        goto cleanup;
    }


    /* Generate the master DTB file:
     * Simplify write error handling by just checking for actual vs
     * expected bytes written at the end.
     */

    log_info("\nGenerating master DTB... \n");
    out_fd = open(output_file, (O_WRONLY | O_CREAT), (S_IRUSR | S_IWUSR));
    if (out_fd < 0) {
        log_err("Cannot create '%s'\n", output_file);
        rc = RC_ERROR;
        goto cleanup;
    }

    /* Write header info */
    wrote += write(out_fd, HSDT_MAGIC, sizeof(uint8_t) * 4UL); /* magic */
    wrote += write(out_fd, &version, sizeof(uint32_t));      /* version */
    wrote += write(out_fd, &dtb_count, sizeof(uint32_t));
    /* #DTB */

    /* Calculate offset of first DTB block */
    dtb_offset = (sizeof(struct dt_table_t)) +    /* header */
        (sizeof(struct dt_entry_t) * dtb_count) + /* DTB table entries */
        4UL;                                      /* end of table indicator */
    /* Round up to page size */
    padding = page_size - (dtb_offset % page_size);
    dtb_offset += padding;
    expected = dtb_offset;
    /* Write index table:
         boardid
         reserved
         dtb size
         vrl size
         dtb offset
         vrl offset
         dtb file
         vrl file
     */
    for (chip = chip_list; chip != NULL; chip = chip->next) {
        if (chip->dt_entry.dtb_offset == 0) {
            chip->dt_entry.dtb_offset = expected;
            expected += chip->dt_entry.dtb_size;
        }
    }

    vrl_offset = expected;
    for (chip = chip_list; chip != NULL; chip = chip->next) {
        if ((stat((char *)chip->dt_entry.vrl_file, &st) == 0) && (st.st_size != 0)) {
            chip->dt_entry.vrl_offset = (uint32_t)vrl_offset;
            vrl_size = st.st_size + (page_size - ((uint32_t)st.st_size % page_size));
            vrl_offset += vrl_size;
            chip->dt_entry.vrl_size = (uint32_t)vrl_size;
        } else {
            chip->dt_entry.vrl_offset = 0;
            chip->dt_entry.vrl_size = 0;
        }
    }

    for (chip = chip_list; chip != NULL; chip = chip->next) {
        uint64_t dtb_file, vrl_file;
        dtb_file = chip->dt_entry.dtb_file;
        vrl_file = chip->dt_entry.vrl_file;
        chip->dt_entry.dtb_file = 0;
        chip->dt_entry.vrl_file = 0;
        wrote += write(out_fd, &chip->dt_entry, sizeof(struct dt_entry_t));
        chip->dt_entry.dtb_file = dtb_file;
        chip->dt_entry.vrl_file = vrl_file;
    }

    rc = RC_SUCCESS;
    wrote += write(out_fd, &rc, sizeof(uint32_t)); /* end of table indicator */
    if (padding > 0) {
        wrote += write(out_fd, filler, (size_t)padding);
    }
    /* Write DTB's */
    for (chip = chip_list; chip != NULL; chip = chip->next) {
        filename = (char *)(chip->dt_entry.dtb_file);
        dtb_size = chip->dt_entry.dtb_size;

        pInputFile = fopen(filename, "r");
        if (pInputFile != NULL) {
            totBytesRead = 0;
            while (1) {
                numBytesRead = (uint32_t)fread(buf, 1, COPY_BLK, pInputFile);
                if (numBytesRead == 0) {
                    break;
                }
                wrote += write(out_fd, buf, numBytesRead);
                totBytesRead += numBytesRead;
            }
            (void)fclose(pInputFile);
            padding = page_size - (totBytesRead % page_size);

            if ((totBytesRead + padding) != dtb_size) {
                log_err("DTB size mismatch, please re-run: expected %u vs actual %u (%s)\n", dtb_size,
                    totBytesRead + padding, filename);
                rc = RC_ERROR;
                break;
            }

            if (padding > 0) {
                wrote += write(out_fd, filler, (size_t)padding);
            }
        } else {
            log_err("failed to open DTB '%s'\n", filename);
            rc = RC_ERROR;
            break;
        }
    }
    if (expected != (uint32_t)wrote) {
        log_err("[Write DTB]error writing output file, please rerun: size mismatch %u vs %zd\n", expected, wrote);
        rc = RC_ERROR;
    } else {
        log_dbg("Total wrote %zd bytes\n", wrote);
    }

    /* Write VRL's */
    for (chip = chip_list; chip != NULL; chip = chip->next) {
        dtb_vrl_filename = (char *)(chip->dt_entry.vrl_file);
        pInputFile = fopen(dtb_vrl_filename, "r");
        if (pInputFile != NULL) {
            totBytesRead = 0;
            while (1) {
                numBytesRead = (uint32_t)fread(buf, 1, COPY_BLK, pInputFile);
                if (numBytesRead == 0) {
                    break;
                }
                wrote += write(out_fd, buf, numBytesRead);
                totBytesRead += numBytesRead;
                log_err("numBytesRead is [%u], totBytesRead is [%u] \n", numBytesRead, totBytesRead);
            }
            (void)fclose(pInputFile);
            padding = page_size - (totBytesRead % page_size);

            if (padding > 0) {
                log_err("padding is [%u] \n", padding);
                wrote += write(out_fd, filler, (size_t)padding);
            } else {
                log_err("file size is 0! '%s'\n", dtb_vrl_filename);
            }
        } else {
            break;
        }
    }

    (void)close(out_fd);

    if (rc != RC_SUCCESS) {
        (void)unlink(output_file);
    } else {
        log_info("completed\n");
    }

cleanup:
    closedir(dir);
    free(filler);
    chip_deleteall();
    return rc;
}
