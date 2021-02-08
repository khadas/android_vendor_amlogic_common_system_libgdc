/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <IONmem.h>
#include <ion/ion.h>
#include <ion_4.12.h>

int ion_mem_init(void)
{
    int ion_fd = -1;
    ion_fd = ion_open();

    if (ion_fd < 0) {
        E_GDC("%s failed: '%s'\n", __func__, strerror(errno));
        return -1;
    }
    D_GDC("%s, ion_fd=%d\n", __func__, ion_fd);

    return ion_fd;
}

int ion_mem_alloc_fd(int ion_fd, size_t size, IONMEM_AllocParams *params, unsigned int flag, unsigned int alloc_hmask)
{
    int ret = -1;
    int num_heaps = 0;
    unsigned int heap_mask = 0;

    if (ion_query_heap_cnt(ion_fd, &num_heaps) >= 0) {
        D_GDC("num_heaps=%d\n", num_heaps);
        struct ion_heap_data *const heaps = (struct ion_heap_data *)malloc(num_heaps * sizeof(struct ion_heap_data));
        if (heaps != NULL && num_heaps) {
            if (ion_query_get_heaps(ion_fd, num_heaps, heaps) >= 0) {
                for (int i = 0; i != num_heaps; ++i) {
                    D_GDC("heaps[%d].type=%d, heap_id=%d\n", i, heaps[i].type, heaps[i].heap_id);
                    if ((1 << heaps[i].type) == alloc_hmask) {
                        heap_mask = 1 << heaps[i].heap_id;
                        D_GDC("%d, m=%x, 1<<heap_id=%x, heap_mask=%x, name=%s, alloc_hmask=%x\n",
                            heaps[i].type, 1<<heaps[i].type, heaps[i].heap_id, heap_mask, heaps[i].name, alloc_hmask);
                        break;
                    }
                }
            }
            free(heaps);
            if (heap_mask)
                ret = ion_alloc_fd(ion_fd, size, 0, heap_mask, flag, &params->mImageFd);
            else
                E_GDC("don't find match heap!!\n");
        } else {
              if (heaps)
                  free(heaps);
            E_GDC("heaps is NULL or no heaps,num_heaps=%d\n", num_heaps);
        }
    } else {
        E_GDC("query_heap_cnt fail! no ion heaps for alloc!!!\n");
    }
    if (ret < 0) {
        E_GDC("ion_alloc failed, errno=%d\n", errno);
        return -ENOMEM;
    }
    return ret;
}

unsigned long ion_mem_alloc(int ion_fd, size_t size, IONMEM_AllocParams *params, bool cache_flag)
{
    int ret = -1;
    int legacy_ion = 0;
    unsigned flag = 0;

    legacy_ion = ion_is_legacy(ion_fd);

    if (cache_flag) {
        flag = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
    }
    D_GDC("%s, cache=%d, bytes=%d, legacy_ion=%d, flag=0x%x\n",
        __func__, cache_flag, size, legacy_ion, flag);

    if (legacy_ion == 1) {
        if (!cache_flag) {
            ret = ion_alloc(ion_fd, size, 0, ION_HEAP_TYPE_DMA_MASK, flag,
                                &params->mIonHnd);
            if (ret < 0)
                ret = ion_alloc(ion_fd, size, 0, ION_HEAP_CARVEOUT_MASK, flag,
                                &params->mIonHnd);
        }
        if (ret < 0) {
            ret = ion_alloc(ion_fd, size, 0, 1 << ION_HEAP_TYPE_CUSTOM, flag,
                                &params->mIonHnd);
            if (ret < 0) {
                E_GDC("%s failed, errno=%d\n", __func__, errno);
                return -ENOMEM;
            }
        }

        ret = ion_share(ion_fd, params->mIonHnd, &params->mImageFd);
        if (ret < 0) {
            E_GDC("ion_share failed, errno=%d\n", errno);
            ion_free(ion_fd, params->mIonHnd);
            return -EINVAL;
        }
        ion_free(ion_fd, params->mIonHnd);
    } else {
        flag |= ION_FLAG_EXTEND_MESON_HEAP;
        ret = ion_mem_alloc_fd(ion_fd, size, params, flag,
                                    ION_HEAP_TYPE_DMA_MASK);
        if (ret < 0)
            ret = ion_mem_alloc_fd(ion_fd, size, params, flag,
                                        ION_HEAP_CARVEOUT_MASK);
        if (ret < 0)
                ret = ion_mem_alloc_fd(ion_fd, size, params, flag,
                                            ION_HEAP_TYPE_CUSTOM);
        if (ret < 0) {
            E_GDC("%s failed, errno=%d\n", __func__, errno);
            return -ENOMEM;
        }
    }

    D_GDC("%s okay done!, legacy_ion=%d\n", __func__, legacy_ion);
    D_GDC("%s, shared_fd=%d\n", __func__, params->mImageFd);

    return ret;
}

static int ion_ioctl(int fd, int req, void* arg) {
    int ret = ioctl(fd, req, arg);
    if (ret < 0) {
        E_GDC("ioctl %x failed with code %d: %s\n", req, ret, strerror(errno));
        return -errno;
    }
    return ret;
}

#define ION_IOC_INVALID_CACHE_       _IOWR(ION_IOC_MAGIC, 9, struct ion_fd_data)

int ion_cache_invalid(int fd, int handle_fd)
{
    struct ion_fd_data data;
    data.fd = handle_fd;
    return ion_ioctl(fd, ION_IOC_INVALID_CACHE_, &data);
}

int ion_mem_invalid_cache(int ion_fd, int shared_fd)
{
    int legacy_ion = 0;

    legacy_ion = ion_is_legacy(ion_fd);
    if (!legacy_ion)
        return 0;
    if (ion_fd >= 0 && shared_fd >= 0) {
        if (ion_cache_invalid(ion_fd, shared_fd)) {
            E_GDC("ion_mem_invalid_cache err!\n");
            return -1;
        }
    } else {
        E_GDC("ion_mem_invalid_cache err!\n");
        return -1;
    }

    return 0;
}

void ion_mem_exit(int ion_fd)
{
    int ret = -1;

    D_GDC("exit: %s, ion_fd=%d\n", __func__, ion_fd);

    ret = ion_close(ion_fd);
    if (ret < 0) {
        E_GDC("%s err (%s)! ion_fd=%d\n", __func__, strerror(errno), ion_fd);
        return;
    }
}
