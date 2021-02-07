/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef IONMEM_H
#define IONMEM_H

#include <linux/ion.h>
#include <stdbool.h>
#include <log/log.h>

#if defined (__cplusplus)
extern "C" {
#endif

/* #define __DEBUG */

#ifdef __DEBUG
#define D_GDC(fmt, args...) ALOGD("gdc Debug: " fmt, ## args)
#else
#define D_GDC(fmt, args...)
#endif
#define E_GDC(fmt, args...) ALOGE("gdc error: " fmt, ## args)

#define ION_FLAG_EXTEND_MESON_HEAP          (1 << 30)

typedef struct IONMEM_AllocParams {
    ion_user_handle_t   mIonHnd;
    int                 mImageFd;
    size_t size;
    unsigned char *usr_ptr;
} IONMEM_AllocParams;

int ion_mem_init(void);
unsigned long ion_mem_alloc(int ion_fd, size_t size, IONMEM_AllocParams *params, bool cache_flag);
int ion_mem_invalid_cache(int ion_fd, int shared_fd);
int ion_mem_free(IONMEM_AllocParams *params);
void ion_mem_exit(int ion_fd);

#if defined (__cplusplus)
}
#endif

#endif

