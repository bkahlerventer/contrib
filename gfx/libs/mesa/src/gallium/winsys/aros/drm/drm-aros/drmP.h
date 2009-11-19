/*-
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#if !defined(DRMP_H)
#define DRMP_H

#include "drm.h"
#include "drm_compat_types.h"
#include "drm_redefines.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>


#define DEBUG 0
#include <aros/debug.h>

#include <hidd/irq.h>
struct Library          *OOPBase;

/* HACK */

/* Enable hacks for running under hosted AROS */
/* THIS AND ALL "HOSTED_BUILD" MARKED CODE MUST BE DELETED IN FINAL VERSION */
#define HOSTED_BUILD

#if defined(HOSTED_BUILD)
/* These defines simulate certain gfx cards. Need to be removed in final version. Works only with HOSTED_BUILD */
//#define HOSTED_BUILD_ARCH       0x04    /* NV04 family */
//#define HOSTED_BUILD_CHIPSET    5       /* NV05 chip Riva TNT 2 */
//#define HOSTED_BUILD_ARCH       0x10    /* NV10 family */
//#define HOSTED_BUILD_CHIPSET    16      /* NV10 chip GeForce 256 */
//#define HOSTED_BUILD_ARCH       0x10    /* NV10 family */
//#define HOSTED_BUILD_CHIPSET    21      /* NV15 chip GeForce 2 GTS */
//#define HOSTED_BUILD_ARCH       0x20    /* NV20 family */
//#define HOSTED_BUILD_CHIPSET    32      /* NV20 chip GeForce 3 Ti 200 */
//#define HOSTED_BUILD_ARCH       0x20    /* NV20 family */
//#define HOSTED_BUILD_CHIPSET    37      /* NV25 chip GeForce Ti 4200 */
#define HOSTED_BUILD_ARCH       0x30    /* NV30 family */
#define HOSTED_BUILD_CHIPSET    52      /* NV34 chip GeForce FX 5200 */
//#define HOSTED_BUILD_ARCH       0x40    /* NV40 family */
//#define HOSTED_BUILD_CHIPSET    67      /* NV43 chip GeForce 6600 */
//#define HOSTED_BUILD_ARCH       0x80    /* NV50 family */
//#define HOSTED_BUILD_CHIPSET    132     /* G84 chip GeForce 8600 GT */
#endif

/* HACK ENDS */


/* NEEDED */

/* Config */
#define CONFIG_AGP

#include "drm_mm.h"

#define DRM_ERROR(fmt, ...) bug("[" DRM_NAME "(ERROR):%s] " fmt, __func__ , ##__VA_ARGS__)
#define DRM_DEBUG(fmt, ...) D(bug("[" DRM_NAME "(DEBUG):%s] " fmt, __func__ , ##__VA_ARGS__))
#define DRM_IMPL(fmt, ...) bug("------IMPLEMENT(%s): " fmt, __func__ , ##__VA_ARGS__)


/*
 * DRM_READMEMORYBARRIER() prevents reordering of reads.
 * DRM_WRITEMEMORYBARRIER() prevents reordering of writes.
 * DRM_MEMORYBARRIER() prevents reordering of reads and writes.
 */

#if defined(__i386__)
#define DRM_READMEMORYBARRIER()     __asm __volatile("lock; addl $0,0(%%esp)" : : : "memory");
#define DRM_WRITEMEMORYBARRIER()    __asm __volatile("" : : : "memory");
#define DRM_MEMORYBARRIER()         __asm __volatile("lock; addl $0,0(%%esp)" : : : "memory");
#else
#error IMPLEMENT momory bariers for non-x86
/* FIXME: Implementation for other architextures */
#endif

#define DRM_UDELAY(d)       udelay(d)
#define DRM_CURRENTPID      1
#define DRM_IRQ_ARGS        void *arg
typedef void                irqreturn_t;

struct drm_file;
struct drm_device;
struct file;

typedef int drm_ioctl_t(struct drm_device *dev, void *data,
            struct drm_file *file_priv);

#define DRM_AUTH            0x1
#define DRM_MASTER          0x2
#define DRM_ROOT_ONLY       0x4
#define DRM_CONTROL_ALLOW   0x8

struct drm_ioctl_desc 
{
    unsigned int cmd;
    int flags;
    drm_ioctl_t *func;
};

/* Memory management */

struct drm_local_map
{
    resource_size_t offset;         /* Requested physical address (0 for SAREA)*/
    unsigned long size;             /* Requested physical size (bytes) */
    enum drm_map_type type;         /* Type of memory to map */
    enum drm_map_flags flags;       /* Flags */
    void *handle;                   /* Memory address */
    int mtrr;                       /* MTRR slot used */
};

typedef struct drm_local_map drm_local_map_t;

struct drm_map_list
{
    struct list_head head;                  /* list head */
/* FIXME: Commented out fields */    
//    struct drm_hash_item hash;
    struct drm_local_map *map;              /* mapping */
    uint64_t user_token;
    struct drm_master *master;
    struct drm_mm_node *file_offset_node;   /* fake offset */
};

struct drm_sg_mem {
    unsigned long handle;
    void *virtual;
    int pages;
#if !defined(__AROS__)    
    struct page **pagelist;
#else
    void * buffer;
#endif    
    dma_addr_t *busaddr;
};

//FIXME
struct agp_bridge_data;

struct drm_agp_head {
//FIXME:    DRM_AGP_KERN agp_info;      /**< AGP device information */
    struct list_head memory;
    unsigned long mode;     /* AGP mode */
    struct agp_bridge_data *bridge;
    int enabled;            /* whether the AGP bus as been enabled */
    int acquired;           /* whether the AGP device has been acquired */
    unsigned long base;
    int agp_mtrr;
    int cant_use_aperture;
    unsigned long page_mask;
};

/* Contains a collection of functions common to each drm driver */

#define DRIVER_MODESET     0x2000

struct drm_driver
{
    int         (*load)(struct drm_device *, unsigned long);
    int         (*firstopen)(struct drm_device *);
    void        (*preclose)(struct drm_device *dev, struct drm_file *);
    void        (*lastclose)(struct drm_device *);
    int         (*unload)(struct drm_device *);
    
    /* IRQ */
    irqreturn_t (*irq_handler)(DRM_IRQ_ARGS);
    void        (*irq_preinstall)(struct drm_device *);
    int         (*irq_postinstall)(struct drm_device *);
    void        (*irq_uninstall)(struct drm_device *);

    int                     version_patchlevel;
    unsigned int            driver_features;
    struct drm_ioctl_desc   *ioctls;
};

struct drm_device
{
    struct list_head maplist;       /* Linked list of regions */

    int irq_enabled;                /* True if irq handler is enabled */
    int pci_vendor;                 /* PCI vendor id */
    int pci_device;                 /* PCI device id */
    
    struct drm_agp_head *agp;       /* AGP data */
    struct drm_sg_mem *sg;          /* Scatter gather memory */
    
    struct drm_driver *driver;      /* Driver functions */
    void *dev_private;              /* Device private data */
    struct mutex  struct_mutex;

    /* AROS specific fields */
    OOP_Object              *pci;
    OOP_Object              *pciDevice;
    OOP_Object              *pcidriver;
    HIDDT_IRQ_Handler       *IntHandler;
};

static __inline__ int drm_core_check_feature(struct drm_device *dev,
                         int feature)
{
    return ((dev->driver->driver_features & feature) ? 1 : 0);
}

struct drm_crtc
{
    /* Don't remove it */
    int dummy;
};

struct drm_file
{
    /* Don't remove it */
    int dummy;
};

struct drm_gem_object 
{
    struct file *filp;    
    void *driver_private;
};

static __inline__ int drm_device_is_agp(struct drm_device *dev)
{
    /* FIXME: Implement */
    DRM_IMPL("\n");
    return 0;
}

// /* drm_bufs.c */
int drm_order(unsigned long size);
resource_size_t drm_get_resource_len(struct drm_device *dev,
                       unsigned int resource);
resource_size_t drm_get_resource_start(struct drm_device *dev,
                         unsigned int resource);
// int drm_addmap(struct drm_device *dev, unsigned int offset,
//               unsigned int size, enum drm_map_type type,
//               enum drm_map_flags flags, drm_local_map_t ** map_ptr);
// int drm_rmmap(struct drm_device *dev, drm_local_map_t *map);              
// struct drm_map_list *drm_find_matching_map(struct drm_device *dev,
//                           drm_local_map_t *map);   

/* drm_drv.c */
//int drm_lastclose(struct drm_device *dev);
void drm_exit(struct drm_driver *driver);
int drm_init(struct drm_driver *driver);

/* drm_scatter.c */
//int drm_sg_alloc(struct drm_device *dev, struct drm_scatter_gather * request);
void drm_sg_cleanup(struct drm_sg_mem * entry);

/* drm_irq.c */
int drm_irq_install(struct drm_device *dev);
int drm_irq_uninstall(struct drm_device *dev);

/* drm_memory.c */
void drm_core_ioremap(struct drm_local_map *map, struct drm_device *dev);
// void drm_core_ioremapfree(struct drm_map *map, struct drm_device *dev);
// void *drm_calloc(size_t nmemb, size_t size, int area);
// /* FIXME: make them inline? */
// void *drm_alloc(size_t size, int area);
// void drm_free(void *pt, size_t size, int area);

/* GEM */
struct drm_gem_object *drm_gem_object_alloc(struct drm_device *dev,
                        size_t size);
static inline void
drm_gem_object_unreference(struct drm_gem_object *obj)
{
    if (obj == NULL)
        return;

//FIXME: kref_put(&obj->refcount, drm_gem_object_free);
}

/* MTRR */
#define DRM_MTRR_WC     0
static inline int drm_mtrr_add(unsigned long offset, unsigned long size,
                   unsigned int flags)
{
    return -ENODEV;
}

/* AROS specific functions */
/* drm_aros.c */
LONG        drm_aros_find_supported_video_card(struct drm_device *dev);
void        drm_aros_pci_shutdown(struct drm_device *dev);
APTR        drm_aros_pci_ioremap(OOP_Object *driver, APTR buf, IPTR size);
void        drm_aros_pci_iounmap(OOP_Object *driver, APTR buf, IPTR size);
APTR        drm_aros_pci_resource_start(OOP_Object *pciDevice,  unsigned int resource);
IPTR        drm_aros_pci_resource_len(OOP_Object *pciDevice,  unsigned int resource);
dma_addr_t  drm_aros_dma_map_buf(APTR buf, IPTR offset, IPTR size);
//void        drm_aros_dma_unmap_buf(dma_addr_t dma_address, IPTR size);

/* NEEDED ENDS */


//#define max(a,b) (a > b) ? a : b



// /* FIXME: Need to find a way to remove the need for these defines */

// #define EXPORT_SYMBOL(a)

// 

// #define IRQ_HANDLED         /* nothing */
// #define IRQ_NONE            /* nothing */
// 

// #define DRM_ARRAY_SIZE(x) ARRAY_SIZE(x)
// 
//    
// #define DRM_MEM_DRIVER     2
// #define DRM_MEM_MAPS       5
// #define DRM_MEM_BUFS       7
// #define DRM_MEM_PAGES      9
// #define DRM_MEM_SGLISTS   20
// /* FIXME: Implement missing */



// /**
//  * Creates a driver or general drm_ioctl_desc array entry for the given
//  * ioctl, for use by drm_ioctl().
//  */
// #define DRM_IOCTL_NR(n)     ((n) & 0xff)
// #define DRM_IOCTL_DEF(ioctl, func, flags) [DRM_IOCTL_NR(ioctl)] = {ioctl, func, flags}

/* */
// 


// 
// /* FIXME: Implement missing */
// /** Read a byte from a MMIO region */
// //#define DRM_READ8(map, offset)      readb((map)->handle + (offset))
// /** Read a word from a MMIO region */
// //#define DRM_READ16(map, offset)     readw((map)->handle + (offset))
// /** Read a dword from a MMIO region */
// #define DRM_READ32(map, offset)         readl((map)->handle + (offset))
// /** Write a byte into a MMIO region */
// //#define DRM_WRITE8(map, offset, val)    writeb(val, (map)->handle + (offset))
// /** Write a word into a MMIO region */
// //#define DRM_WRITE16(map, offset, val)   writew(val, (map)->handle + (offset))
// /** Write a dword into a MMIO region */
// #define DRM_WRITE32(map, offset, val)   writel(val, (map)->handle + (offset))




// //#define DRM_INFO(fmt, ...) bug("[" DRM_NAME "(INFO)] " fmt, ##__VA_ARGS__)
// #define DRM_INFO(fmt, ...)


// #define printk(fmt, ...)
// 











// 

// 
// static inline int drm_mtrr_del(int handle, unsigned long offset,
//                    unsigned long size, unsigned int flags)
// {
//     return -ENODEV;
// }
// 
// #define drm_core_has_MTRR(dev) (0)

// static __inline__ int mtrr_add(unsigned long base, unsigned long size,
//                    unsigned int type, char increment)
// {
//     return -ENODEV;
// }
// 
// static __inline__ int mtrr_del(int reg, unsigned long base, unsigned long size)
// {
//     return -ENODEV;
// }
// 
// #define MTRR_TYPE_WRCOMB     1
// /*  */
// 

// 
           
// 

// 


// 

// 
// static __inline__ int drm_device_is_pcie(struct drm_device *dev)
// {
//     /* FIXME: Implement */
//     DRM_IMPL("\n");
//     return 0;
// }
// 

#endif