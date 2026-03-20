#ifndef HW_VDAVINCI_H
#define HW_VDAVINCI_H

#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/pci.h>
#include <linux/scatterlist.h>
#include <linux/types.h>
#include <linux/uuid.h>

#ifndef HW_DVT_MAX_TYPE_NAME
#define HW_DVT_MAX_TYPE_NAME 64
#endif

#ifndef HW_BAR_SPARSE_MAP_MAX
#define HW_BAR_SPARSE_MAP_MAX 16
#endif

#ifndef HW_VDAVINCI_TYPE_MAX
#define HW_VDAVINCI_TYPE_MAX 12
#endif

struct vdavinci_dev {
	struct device *dev;
	struct device *resource_dev;
	u32 fid;
	u32 dev_index;
};

struct vdavinci_type {
	char template_name[HW_DVT_MAX_TYPE_NAME];
	u32 type;
	u32 share;
	u32 aicore_num;
	u32 aicpu_num;
	u32 jpegd_num;
	u32 jpege_num;
	u32 vpc_num;
	u32 vdec_num;
	u32 venc_num;
	u64 hbmmem_size;
	u64 ddrmem_size;
	u64 bar0_size;
	u64 bar2_size;
	u64 bar4_size;
};

struct dvt_devinfo {
	u32 aicore_num;
	u32 aicpu_num;
	u32 jpegd_num;
	u32 jpege_num;
	u32 vpc_num;
	u32 vdec_num;
	u32 venc_num;
	u64 mem_size;
	u64 hbmmem_size;
	u64 ddrmem_size;
};

struct vdavinci_sparse_map {
	u64 offset;
	u64 paddr;
	u64 size;
};

struct vdavinci_mapinfo {
	u64 num;
	struct vdavinci_sparse_map map_info[HW_BAR_SPARSE_MAP_MAX];
};

struct vdavinci_priv_ops {
	int (*vdavinci_create)(struct vdavinci_dev *vdev, void *vdavinci,
		struct vdavinci_type *type, uuid_le uuid);
	void (*vdavinci_destroy)(struct vdavinci_dev *vdev);
	void (*vdavinci_release)(struct vdavinci_dev *vdev);
	int (*vdavinci_reset)(struct vdavinci_dev *vdev);
	int (*vdavinci_flr)(struct vdavinci_dev *vdev);
	void (*vdavinci_notify)(struct vdavinci_dev *vdev, int db_index);
	int (*vdavinci_getmapinfo)(struct vdavinci_dev *vdev,
		struct vdavinci_type *type, u32 bar_id,
		struct vdavinci_mapinfo *map_info);
	int (*vdavinci_putmapinfo)(struct vdavinci_dev *vdev);
	int (*davinci_getdevnum)(struct device *dev);
	int (*davinci_getdevinfo)(struct device *dev, u32 dev_index,
		struct dvt_devinfo *dev_info);
	int (*vascend_enable_sriov)(struct pci_dev *pdev, int numvfs);
};

struct vdavinci_priv {
	struct device *dev;
	struct vdavinci_priv_ops *ops;
};

static inline int hw_dvt_hypervisor_inject_msix(void *vdavinci, u32 vector)
{
	return -EOPNOTSUPP;
}

static inline int hw_dvt_hypervisor_dma_pool_init(void *vdavinci)
{
	return 0;
}

static inline void hw_dvt_hypervisor_dma_pool_uninit(void *vdavinci)
{
}

static inline int hw_dvt_hypervisor_dma_map_guest_page(void *vdavinci,
	unsigned long gfn, unsigned long size, struct sg_table **dma_sgt)
{
	return -EOPNOTSUPP;
}

static inline void hw_dvt_hypervisor_dma_unmap_guest_page(void *vdavinci,
	struct sg_table *dma_sgt)
{
}

static inline bool hw_dvt_hypervisor_dma_pool_active(void *vdavinci)
{
	return false;
}

static inline int hw_dvt_hypervisor_dma_map_guest_page_batch(void *vdavinci,
	unsigned long *gfn, unsigned long *dma_addr, unsigned long count)
{
	return -EOPNOTSUPP;
}

static inline void hw_dvt_hypervisor_dma_unmap_guest_page_batch(void *vdavinci,
	unsigned long *gfn, unsigned long *dma_addr, unsigned long count)
{
}

static inline int hw_dvt_hypervisor_get_reserve_iova(struct device *dev,
	dma_addr_t *iova_addr, size_t *size)
{
	return -EOPNOTSUPP;
}

static inline int hw_dvt_hypervisor_mmio_get(void **base, int *size,
	void *vdavinci, u32 region_index)
{
	if (base != NULL)
		*base = NULL;
	if (size != NULL)
		*size = 0;
	return -EOPNOTSUPP;
}

static inline void *hw_dvt_hypervisor_dma_alloc_coherent(struct device *dev,
	size_t size, dma_addr_t *dma_handle, gfp_t gfp)
{
	return dma_alloc_coherent(dev, size, dma_handle, gfp);
}

static inline void hw_dvt_hypervisor_dma_free_coherent(struct device *dev,
	size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
	dma_free_coherent(dev, size, cpu_addr, dma_handle);
}

static inline int hw_dvt_init(void *vdavinci_set)
{
	return 0;
}

static inline int hw_dvt_uninit(void *vdavinci_set)
{
	return 0;
}

#endif
