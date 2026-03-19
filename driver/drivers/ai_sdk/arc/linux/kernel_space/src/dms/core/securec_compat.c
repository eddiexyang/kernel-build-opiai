/*
 * Minimal securec compatibility for vendor DMS code on mainline kernels.
 */

#include <linux/kernel.h>
#include <linux/minmax.h>
#include <linux/stdarg.h>
#include <linux/string.h>

#ifdef memset_s
#undef memset_s
#endif

#ifdef memmove_s
#undef memmove_s
#endif

#ifdef memcpy_s
#undef memcpy_s
#endif

#ifdef strcpy_s
#undef strcpy_s
#endif

#ifdef strcat_s
#undef strcat_s
#endif

#ifdef strncat_s
#undef strncat_s
#endif

#ifdef vsprintf_s
#undef vsprintf_s
#endif

#ifdef sprintf_s
#undef sprintf_s
#endif

#ifdef vsnprintf_s
#undef vsnprintf_s
#endif

#ifdef snprintf_s
#undef snprintf_s
#endif

int memset_s(void *dest, size_t destMax, int c, size_t count)
{
	if (!dest || count > destMax)
		return ERANGE;

	memset(dest, c, count);
	return 0;
}

int memmove_s(void *dest, size_t destMax, const void *src, size_t count)
{
	if (!dest || !src)
		return EINVAL;
	if (count > destMax)
		return ERANGE;

	memmove(dest, src, count);
	return 0;
}

int memcpy_s(void *dest, size_t destMax, const void *src, size_t count)
{
	if (!dest || !src)
		return EINVAL;
	if (count > destMax)
		return ERANGE;

	memcpy(dest, src, count);
	return 0;
}

int strcpy_s(char *dest, size_t destMax, const char *src)
{
	size_t len;

	if (!dest || !src || !destMax)
		return EINVAL;

	len = strnlen(src, destMax);
	if (len >= destMax) {
		dest[0] = '\0';
		return ERANGE;
	}

	memcpy(dest, src, len + 1);
	return 0;
}

int strcat_s(char *dest, size_t destMax, const char *src)
{
	size_t dst_len;
	size_t src_len;

	if (!dest || !src || !destMax)
		return EINVAL;

	dst_len = strnlen(dest, destMax);
	src_len = strlen(src);
	if (dst_len >= destMax || src_len > destMax - dst_len - 1)
		return ERANGE;

	memcpy(dest + dst_len, src, src_len + 1);
	return 0;
}

int strncat_s(char *dest, size_t destMax, const char *src, size_t count)
{
	size_t dst_len;
	size_t src_len;
	size_t copy;

	if (!dest || !src || !destMax)
		return EINVAL;

	dst_len = strnlen(dest, destMax);
	src_len = strlen(src);
	copy = min(count, src_len);
	if (dst_len >= destMax || copy > destMax - dst_len - 1)
		return ERANGE;

	memcpy(dest + dst_len, src, copy);
	dest[dst_len + copy] = '\0';
	return 0;
}

int vsprintf_s(char *dest, size_t destMax, const char *fmt, va_list args)
{
	int ret;

	if (!dest || !fmt || !destMax)
		return -1;

	ret = vsnprintf(dest, destMax, fmt, args);
	return ret >= destMax ? -1 : ret;
}

int sprintf_s(char *dest, size_t destMax, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vsprintf_s(dest, destMax, fmt, args);
	va_end(args);
	return ret;
}

int vsnprintf_s(char *dest, size_t destMax, size_t count, const char *fmt,
		va_list args)
{
	size_t limit;
	int ret;

	if (!dest || !fmt || !destMax)
		return -1;

	limit = min(destMax, count + 1);
	ret = vsnprintf(dest, limit, fmt, args);
	return ret >= limit ? -1 : ret;
}

int snprintf_s(char *dest, size_t destMax, size_t count, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vsnprintf_s(dest, destMax, count, fmt, args);
	va_end(args);
	return ret;
}
