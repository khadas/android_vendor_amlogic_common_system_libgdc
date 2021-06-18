/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef DEWARP_API_H_
#define DEWARP_API_H_

#if defined (__cplusplus)
extern "C" {
#endif

#define WIN_MAX 4

struct input_param {
    int width;
    int height;
    int offset_x;
    int offset_y;
    int fov;
};

struct output_param {
    int width;
    int height;
};

struct proj_param {
    int projection_mode;
    int pan;
    int tilt;
    int rotation;
    float zoom;
    float strength_hor;
    float strength_ver;
};

struct win_param {
    int win_start_x;
    int win_end_x;
    int win_start_y;
    int win_end_y;
    int img_start_x;
    int img_end_x;
    int img_start_y;
    int img_end_y;
};

struct clb_param {
    double fx;
    double fy;
    double cx;
    double cy;
    double k1;
    double k2;
    double k3;
    double p1;
    double p2;
    double k4;
    double k5;
    double k6;
};

struct meshin_param {
    int x_start;
    int y_start;
    int x_len;
    int y_len;
    int x_step;
    int y_step;
    float *meshin_data_table;
};

struct dewarp_params {
    int win_num;
    struct input_param input_param;
    int color_mode;
    struct output_param output_param;
    struct proj_param proj_param[WIN_MAX];
    struct clb_param clb_param[WIN_MAX];
    struct meshin_param meshin_param[WIN_MAX];
    struct win_param win_param[WIN_MAX];
    int prm_mode; /* 0. use proj_param, 1.use clb_param, 2. meshin mode*/
    int tile_x_step;
    int tile_y_step;
};

typedef enum _dw_proj_mode_ {
    PROJ_MODE_EQUISOLID = 0,
    PROJ_MODE_EQUIDISTANCE,
    PROJ_MODE_STEREOGRAPHIC,
    PROJ_MODE_ORTHOGONAL,
    PROJ_MODE_LINEAR
} dw_proj_mode_t;

typedef enum _data_mode_ {
    YUV420_PLANAR = 0,
    YUV420_SEMIPLANAR,
    YONLY,
    DEWARP_COLOR_MODE_MAX
} data_mode_t;

/* Description  : generate FW data to fw_buffer
 * Params       : dewarp_params and fw_buffer vaddr
 * Returen      : success -- total bytes of FW
 *                   fail -- minus
 */
int dewarp_gen_config(struct dewarp_params *dewarp_params, int *fw_buffer);

#if defined (__cplusplus)
}
#endif

#endif
