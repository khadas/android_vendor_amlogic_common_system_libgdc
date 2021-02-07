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

struct dewarp_params {
    int win_num;
    struct input_param input_param;
    int color_mode;
    struct output_param output_param;
    struct proj_param proj_param[WIN_MAX];
    struct win_param win_param[WIN_MAX];
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
    YUV420_MODE_MAX
} data_mode_t;

int dewarp_gen_config(struct dewarp_params *dewarp_params, int *fw_buffer);

#if defined (__cplusplus)
}
#endif

#endif
