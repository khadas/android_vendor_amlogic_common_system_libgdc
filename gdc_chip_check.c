/*
 *   Copyright 2018 Amlogic, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <test_data.h>
#include <IONmem.h>

#include "gdc_api.h"
#define TEST_NUM 3

static int compare_data(void *std, char *data)
{
	int i = 0, j = 0, offset = 0, err_hit = 0;
	int thresh_hold = 1;
	int ret = -1;
	patern_output_image_t *golden = NULL;
	char *golden_y_line;
	char *golden_uv_line;
	int line_stride;

	if (!data || !std) {
		E_GDC("cmp data, para err\n");
		return -1;
	}

	golden = (patern_output_image_t *)std;
	line_stride = golden->image_w;

	/* set a y line */
	golden_y_line = (char *)malloc(line_stride);
	if (!golden_y_line) {
		E_GDC("%s, malloc y failed\n", __func__);
		return -1;
	}

#ifdef DUMP_OUTPUT
	printf("=== y ===\n");
#endif
	for (i = 0; i < golden->y_array_cnt; i++) {
		for (j = 0; j < golden->y[i].len_pixel; j++) {
			golden_y_line[offset] = golden->y[i].color_y;
			#ifdef DUMP_OUTPUT
			printf("0x%x ", golden_y_line[offset]);
			#endif
			offset++;
		}
	}

	/* set a uv line */
	golden_uv_line = (char *)malloc(line_stride);
	if (!golden_uv_line) {
		E_GDC("%s, malloc uv failed\n", __func__);
		ret = -1;
		goto free_y;
	}

#ifdef DUMP_OUTPUT
	printf("\n=== uv ===\n");
#endif
	offset = 0;
	for (i = 0; i < golden->uv_array_cnt; i++) {
		for (j = 0; j < golden->uv[i].len_pixel; j++) {
			golden_uv_line[offset] = golden->uv[i].color_u;
			golden_uv_line[offset + 1] = golden->uv[i].color_v;
			#ifdef DUMP_OUTPUT
			printf("0x%x 0x%x ", golden_uv_line[offset], golden_uv_line[offset + 1]);
			#endif
			offset += 2;
		}
	}

	/* compare y */
	offset = 0;
	for (i = 0; i < golden->image_h; i++) {
		ret = memcmp(data + offset, golden_y_line, line_stride);
		if (ret) {
			E_GDC("err_hit y one\n");
			err_hit++;
		}
		if (err_hit > thresh_hold) {
			E_GDC("bad chip found,err_hit=%d\n",err_hit);
			ret = -1;
			goto free;
		}
		offset += line_stride;
	}

	/* compare uv */
	for (i = 0; i < golden->image_h / 2; i++) {
		ret = memcmp(data + offset, golden_uv_line, line_stride);
		if (ret) {
			E_GDC("err_hit uv one\n");
			err_hit++;
		}
		if (err_hit > thresh_hold) {
			E_GDC("bad chip found,err_hit=%d\n",err_hit);
			ret = -1;
			goto free;
		}
		offset += line_stride;
	}

#ifdef DUMP_OUTPUT
	printf("final offset:%d\n", offset);
#endif

free:
	free(golden_uv_line);
free_y:
	free(golden_y_line);

	return ret;
}

static int slt_gdc_set_config_param(struct gdc_usr_ctx_s *ctx,
				char *config, int len)
{
	if (config == NULL || ctx == NULL || ctx->c_buff == NULL) {
		E_GDC("Error input param\n");
		return -1;
	}

	memcpy(ctx->c_buff, config, len);

	return 0;
}

#ifdef DUMP_OUTPUT
static void save_imgae(char *data, int byte_size, char *file_name)
{
	FILE *fp = NULL;
	int i;

	if (!data || !byte_size || !file_name) {
		E_GDC("%s:wrong paras\n", __func__);
		return;
	}

	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		E_GDC("%s:Error open file\n", __func__);
		return;
	}
	D_GDC("gdc: 0x%2x, 0x%2x,0x%2x,0x%2x, 0x%2x,0x%2x,0x%2x,0x%2x\n",
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7]);

	fwrite(data, byte_size, 1, fp);
	fclose(fp);
}
#endif

static int slt_gdc_init_cfg(struct gdc_usr_ctx_s *ctx, struct gdc_param *tparm,
				char *f_name)
{
	struct gdc_settings_ex *gdc_gs = NULL;
	int ret = -1;
	uint32_t format = 0;
	uint32_t i_width = 0;
	uint32_t i_height = 0;
	uint32_t o_width = 0;
	uint32_t o_height = 0;
	uint32_t i_y_stride = 0;
	uint32_t i_c_stride = 0;
	uint32_t o_y_stride = 0;
	uint32_t o_c_stride = 0;
	uint32_t i_len = 0;
	uint32_t o_len = 0;
	uint32_t c_len = 0;
	int plane_number = 1;
	gdc_alloc_buffer_t buf;

	if (ctx == NULL || tparm == NULL || f_name == NULL) {
		E_GDC("Error invalid input param\n");
		return ret;
	}

	plane_number = ctx->plane_number;
	i_width = tparm->i_width;
	i_height = tparm->i_height;
	o_width = tparm->o_width;
	o_height = tparm->o_height;

	format = tparm->format;

	i_y_stride = AXI_WORD_ALIGN(i_width);
	o_y_stride = AXI_WORD_ALIGN(o_width);

	if (format == NV12 || format == YUV444_P || format == RGB444_P) {
		i_c_stride = AXI_WORD_ALIGN(i_width);
		o_c_stride = AXI_WORD_ALIGN(o_width);
	} else if (format == YV12) {
		i_c_stride = AXI_WORD_ALIGN(i_width) / 2;
		o_c_stride = AXI_WORD_ALIGN(o_width) / 2;
	} else if (format == Y_GREY) {
		i_c_stride = 0;
		o_c_stride = 0;
	} else {
		E_GDC("Error unknow format\n");
		return ret;
	}

	gdc_gs = &ctx->gs_ex;

	gdc_gs->gdc_config.input_width = i_width;
	gdc_gs->gdc_config.input_height = i_height;
	gdc_gs->gdc_config.input_y_stride = i_y_stride;
	gdc_gs->gdc_config.input_c_stride = i_c_stride;
	gdc_gs->gdc_config.output_width = o_width;
	gdc_gs->gdc_config.output_height = o_height;
	gdc_gs->gdc_config.output_y_stride = o_y_stride;
	gdc_gs->gdc_config.output_c_stride = o_c_stride;
	gdc_gs->gdc_config.format = format;
	gdc_gs->magic = sizeof(*gdc_gs);

	buf.format = format;

	ret = gdc_create_ctx(ctx);
	if (ret < 0)
		return -1;

	if (!ctx->custom_fw) {
		if (ctx->dev_type == AML_GDC)
			c_len = sizeof(config_data_aml);
		else
			c_len = sizeof(config_data);
		if (c_len <= 0) {
			gdc_destroy_ctx(ctx);
			E_GDC("Error gdc config file size\n");
			return ret;
		}

		buf.plane_number = 1;
		buf.len[0] = c_len;
		ret = gdc_alloc_buffer(ctx, CONFIG_BUFF_TYPE, &buf, false);
		if (ret < 0) {
			gdc_destroy_ctx(ctx);
			E_GDC("Error alloc gdc cfg buff\n");
			return ret;
		}

		ret = slt_gdc_set_config_param(ctx, f_name, c_len);
		if (ret < 0) {
			gdc_destroy_ctx(ctx);
			E_GDC("Error cfg gdc param buff\n");
			return ret;
		}

		gdc_gs->gdc_config.config_size = c_len / 4;
	}
	buf.plane_number = plane_number;
	if ((plane_number == 1) || (format == Y_GREY)) {
		if (format == RGB444_P || format == YUV444_P)
			i_len = i_y_stride * i_height * 3;
		else if (format == NV12 || format == YV12)
			i_len = i_y_stride * i_height * 3 / 2;
		else if (format == Y_GREY)
			i_len = i_y_stride * i_height;
		buf.plane_number = 1;
		buf.len[0] = i_len;
	} else if ((plane_number == 2) && (format == NV12)) {
		buf.len[0] = i_y_stride * i_height;
		buf.len[1] = i_y_stride * i_height / 2;
	} else if ((plane_number == 3) &&
		(format == YV12 ||
		(format == YUV444_P) ||
		(format == RGB444_P))) {
		buf.len[0] = i_y_stride * i_height;
		if (format == YV12) {
			buf.len[1] = i_y_stride * i_height / 4;
			buf.len[2] = i_y_stride * i_height / 4;
		} else if ((format == YUV444_P) ||
			(format == RGB444_P)) {
			buf.len[1] = i_y_stride * i_height;
			buf.len[2] = i_y_stride * i_height;
		}
	}
	ret = gdc_alloc_buffer(ctx, INPUT_BUFF_TYPE, &buf, false);
	if (ret < 0) {
		gdc_destroy_ctx(ctx);
		E_GDC("Error alloc gdc input buff\n");
		return ret;
	}

	buf.plane_number = plane_number;
	if ((plane_number == 1) || (format == Y_GREY)) {
		if (format == RGB444_P || format == YUV444_P)
			o_len = o_y_stride * o_height * 3;
		else if (format == NV12 || format == YV12)
			o_len = o_y_stride * o_height * 3 / 2;
		else if (format == Y_GREY)
			o_len = o_y_stride * o_height;
		buf.plane_number = 1;
		buf.len[0] = o_len;
	} else if ((plane_number == 2) && (format == NV12)) {
		buf.len[0] = o_y_stride * o_height;
		buf.len[1] = o_y_stride * o_height / 2;
	} else if ((plane_number == 3) &&
		(format == YV12 ||
		(format == YUV444_P) ||
		(format == RGB444_P))) {
		buf.len[0] = o_y_stride * o_height;
		if (format == YV12) {
			buf.len[1] = o_y_stride * o_height / 4;
			buf.len[2] = o_y_stride * o_height / 4;
		} else if ((format == YUV444_P) ||
			(format == RGB444_P)) {
			buf.len[1] = o_y_stride * o_height;
			buf.len[2] = o_y_stride * o_height;
		}
	}

	ret = gdc_alloc_buffer(ctx, OUTPUT_BUFF_TYPE, &buf, false);
	if (ret < 0) {
		gdc_destroy_ctx(ctx);
		E_GDC("Error alloc gdc input buff\n");
		return ret;
	}
	return ret;
}

static int pattern_set_input_image(struct gdc_usr_ctx_s *ctx,
				   patern_input_image_t *input_image)
{
	int i, j, k, offset = 0;

	if (input_image == NULL || ctx == NULL) {
		E_GDC("Error input param\n");
		return -1;
	}

	if (ctx->i_buff[0] == NULL || ctx->i_len[0] == 0) {
		D_GDC("Error input param\n");
		return -1;
	}
	/* y */
	for (i = 0; i < input_image->image_h; i++) {
		for (j = 0; j < (int)(sizeof(input_image->y) / sizeof(input_image->y[0])); j++)
			for (k = 0; k < input_image->y[j].len_pixel; k++) {
				ctx->i_buff[0][offset] = input_image->y[j].color_y;
				offset++;
			}
	}

	/* uv */
	for (i = 0; i < input_image->image_h / 2; i++) {
		for (j = 0; j < (int)(sizeof(input_image->uv) / sizeof(input_image->uv[0])); j++)
			for (k = 0; k < input_image->uv[j].len_pixel; k++) {
				ctx->i_buff[0][offset] = input_image->uv[j].color_u;
				ctx->i_buff[0][offset + 1] = input_image->uv[j].color_v;
				offset += 2;
			}
	}

	return 0;
}

int main(int argc, char* argv[])
{
	int ret = 0;
	struct gdc_usr_ctx_s ctx;
	uint32_t format;
	uint32_t in_width;
	uint32_t in_height;
	uint32_t out_width;
	uint32_t out_height;
	int plane_number, mem_type;
	void *golden = NULL;
	struct gdc_param g_param;
	int i = 0, test_cnt = 0, err_cnt = 0;
	int is_custom_fw;
	/* 0: default value, means gdc
	 * 1: means v1 dewarp (t7 chip)
	 * 2: means v2 dewarp (p1 and later chips)
	 */
	int dev_type = ARM_GDC;
	char *config = NULL;

	/* set params */
	if (argc > 1)
		dev_type = atoi(argv[1]);

	is_custom_fw = 0;
	format = 1;
	plane_number = 1;
	in_width = 640;
	in_height = 480;
	out_width = 640;
	out_height = 480;
	mem_type = 0;
	/* set params end */

	g_param.i_width = in_width;
	g_param.i_height = in_height;
	g_param.o_width = out_width;
	g_param.o_height = out_height;
	g_param.format = format;

	memset(&ctx, 0, sizeof(ctx));
	ctx.custom_fw = is_custom_fw;
	ctx.mem_type = mem_type;
	ctx.plane_number = plane_number;
	ctx.dev_type = dev_type > 0 ? AML_GDC : ARM_GDC;

	if (dev_type == AML_GDC_V2) {
		config = (char *)config_data_aml;
		golden = &output_golden_aml_v2;
	} else if (dev_type == AML_GDC) {
		config = (char *)config_data_aml;
		golden = &output_golden_aml;
	} else {
		config = (char *)config_data;
		golden = &output_golden;
	}

	ret = slt_gdc_init_cfg(&ctx, &g_param, config);
	if (ret < 0) {
		E_GDC("Error gdc init\n");
		gdc_destroy_ctx(&ctx);
		return -1;
	}

	printf("Start GDC chip check...\n");
	for (test_cnt = 0; test_cnt < TEST_NUM; test_cnt++) {
		pattern_set_input_image(&ctx, &input_image);

		#ifdef DUMP_OUTPUT
		save_imgae(ctx.i_buff[0], ctx.i_len[0], "save_input_file.raw");
		printf("Success save input image\n");
		#endif
		ret = gdc_process(&ctx);
		if (ret < 0) {
			E_GDC("ioctl failed\n");
			gdc_destroy_ctx(&ctx);
			return ret;
		}

		ion_mem_invalid_cache(ctx.ion_fd,
			ctx.gs_ex.output_buffer.shared_fd);
		ret = compare_data(golden, ctx.o_buff[i]);
		if (ret < 0)
			err_cnt++;
	}
	if (err_cnt >= TEST_NUM)
		printf("===gdc_slt_test:failed===\n");
	else
		printf("===gdc_slt_test:pass===\n");

	#ifdef DUMP_OUTPUT
	save_imgae(ctx.o_buff[0], ctx.o_len[0], "save_output_file.raw");
	printf("Success save output image\n");
	#endif

	gdc_destroy_ctx(&ctx);

	return 0;
}
