#ifndef DVPP_MEM_FLAG_DEF_H
#define DVPP_MEM_FLAG_DEF_H

#define DVPP_MEM_CACHED          0
#define DVPP_MEM_UNCACHED        1
#define DVPP_MEM_WC              2

#define DVPP_MEM_FLAG_NOMAP      (0)
#define DVPP_MEM_FLAG_CACHED     (1U << DVPP_MEM_CACHED)
#define DVPP_MEM_FLAG_UNCACHED   (1U << DVPP_MEM_UNCACHED)
#define DVPP_MEM_FLAG_WC         (1U << DVPP_MEM_WC)

#define DVPP_MEM_CACHE_MASK      (DVPP_MEM_FLAG_CACHED | \
                                  DVPP_MEM_FLAG_UNCACHED | \
                                  DVPP_MEM_FLAG_WC)


#define DVPP_MEM_MBUF             8
/* mmz by default */
#define DVPP_MEM_MMZ              9

#define DVPP_MEM_PLACEMENT_MBUF  (1UL << DVPP_MEM_MBUF)
#define DVPP_MEM_PLACEMENT_MMZ   (1UL << DVPP_MEM_MMZ)

#define DVPP_MEM_PLACEMENT_MASK  (DVPP_MEM_PLACEMENT_MBUF| \
                                  DVPP_MEM_PLACEMENT_MMZ)


#define DVPP_PAGE_NORMAL          12
#define DVPP_PAGE_HUGE_PRIOR      13
#define DVPP_PAGE_HUGE_ONLY       14
#define DVPP_DEVICE_FLAG          24

#define DVPP_PAGE_FLAG_NORMAL      (1UL << DVPP_PAGE_NORMAL)
#define DVPP_PAGE_FLAG_HUGE_PRIOR  (1UL << DVPP_PAGE_HUGE_PRIOR)
#define DVPP_PAGE_FLAG_HUGE_ONLY   (1UL << DVPP_PAGE_HUGE_ONLY)

#define DVPP_PAGE_FLAG_MASK       (DVPP_PAGE_FLAG_NORMAL | \
                                   DVPP_PAGE_FLAG_HUGE_PRIOR | \
                                   DVPP_PAGE_FLAG_HUGE_ONLY)

#define DVPP_MEM_READONLY_BIT      16 // k2u内存(用户态)只读属性flag为第16位

#define DVPP_MEM_READONLY (1UL << DVPP_MEM_READONLY_BIT)

/* dvpp buffer type */
enum dvpp_buffer_type {
    /* buffer create by dvpp driver, buffer life cycle own by driver */
    dvpp_buffer_type_kernel,

    /* buffer send from user, give up ref after task process */
    dvpp_buffer_type_user
};

/* bus address type */
enum dvpp_iova_addr_type {
    /* hardware work on phyical address */
    dvpp_addr_phy,

    /* hardware work on virtual address */
    dvpp_addr_virt,

    /* hardware work on iova address */
    dvpp_addr_iova
};

#endif // end of DVPP_MEM_FLAG_DEF_H
