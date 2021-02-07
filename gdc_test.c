/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
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
#include <pthread.h>
#include <sys/time.h>
#include <gdc_api.h>

int parse_custom_fw(struct gdc_usr_ctx_s *ctx, const char *config_file)
{
	int ret = -1;
	struct fw_input_info_s *in = &ctx->gs_with_fw.fw_info.fw_input_info;
	struct fw_output_info_s *out = &ctx->gs_with_fw.fw_info.fw_output_info;
	union transform_u *trans =
				&ctx->gs_with_fw.fw_info.fw_output_info.trans;
	char gdc_format[8] = {};
	char custom_name[64] = {};

	ret = sscanf(config_file, "equisolid-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%[^-]-%[^_]_%[^_]_%d-%[^.].bin",
		&in->with, &in->height,
		&in->fov, &in->diameter,
		&in->offsetX, &in->offsetY,
		&out->offsetX, &out->offsetY,
		&out->width, &out->height,
		&out->pan, &out->tilt, out->zoom,
		trans->fw_equisolid.strengthX,
		trans->fw_equisolid.strengthY,
		&trans->fw_equisolid.rotation,
		gdc_format);
	ctx->gs_with_fw.fw_info.fw_type = EQUISOLID;

	if (ret > 0)
		printf("equisolid-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%s-%s_%s_%d-%s.bin\n",
			in->with, in->height,
			in->fov, in->diameter,
			in->offsetX, in->offsetY,
			out->offsetX, out->offsetY,
			out->width, out->height,
			out->pan, out->tilt, out->zoom,
			trans->fw_equisolid.strengthX,
			trans->fw_equisolid.strengthY,
			trans->fw_equisolid.rotation,
			gdc_format);

	if (ret <= 0) {
		ret = sscanf(config_file, "cylinder-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%[^-]-%[^_]_%d-%[^.].bin",
			&in->with, &in->height,
			&in->fov, &in->diameter,
			&in->offsetX, &in->offsetY,
			&out->offsetX, &out->offsetY,
			&out->width, &out->height,
			&out->pan, &out->tilt, out->zoom,
			trans->fw_cylinder.strength,
			&trans->fw_cylinder.rotation,
			gdc_format);
		ctx->gs_with_fw.fw_info.fw_type = CYLINDER;

		if (ret > 0)
			printf("cindif: cylinder-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%s-%s_%d-%s.bin\n",
				in->with, in->height,
				in->fov, in->diameter,
				in->offsetX, in->offsetY,
				out->offsetX, out->offsetY,
				out->width, out->height,
				out->pan, out->tilt, out->zoom,
				trans->fw_cylinder.strength,
				trans->fw_cylinder.rotation,
				gdc_format);
	}

	if (ret <= 0) {
		ret = sscanf(config_file, "equidistant-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%[^-]-%[^_]_%d_%d_%d_%d_%d_%d_%d-%[^.].bin",
			&in->with, &in->height,
			&in->fov, &in->diameter,
			&in->offsetX, &in->offsetY,
			&out->offsetX, &out->offsetY,
			&out->width, &out->height,
			&out->pan, &out->tilt, out->zoom,
			trans->fw_equidistant.azimuth,
			&trans->fw_equidistant.elevation,
			&trans->fw_equidistant.rotation,
			&trans->fw_equidistant.fov_width,
			&trans->fw_equidistant.fov_height,
			(int *)&trans->fw_equidistant.keep_ratio,
			&trans->fw_equidistant.cylindricityX,
			&trans->fw_equidistant.cylindricityY,
			gdc_format);
		ctx->gs_with_fw.fw_info.fw_type = EQUIDISTANT;

		if (ret > 0)
			printf("equidistant-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%s-%s_%d_%d_%d_%d_%d_%d_%d-%s.bin\n",
				in->with, in->height,
				in->fov, in->diameter,
				in->offsetX, in->offsetY,
				out->offsetX, out->offsetY,
				out->width, out->height,
				out->pan, out->tilt, out->zoom,
				trans->fw_equidistant.azimuth,
				trans->fw_equidistant.elevation,
				trans->fw_equidistant.rotation,
				trans->fw_equidistant.fov_width,
				trans->fw_equidistant.fov_height,
				trans->fw_equidistant.keep_ratio,
				trans->fw_equidistant.cylindricityX,
				trans->fw_equidistant.cylindricityY,
				gdc_format);
	}
	if (ret <= 0) {
		trans->fw_custom.fw_name = custom_name;
		ret = sscanf(config_file, "custom-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%[^-]-%[^-]-%[^.].bin",
			&in->with, &in->height,
			&in->fov, &in->diameter,
			&in->offsetX, &in->offsetY,
			&out->offsetX, &out->offsetY,
			&out->width, &out->height,
			&out->pan, &out->tilt, out->zoom,
			trans->fw_custom.fw_name,
			gdc_format);
		ctx->gs_with_fw.fw_info.fw_type = CUSTOM;

		if (ret > 0)
			printf("custom-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%s-%s-%s.bin\n",
				in->with, in->height,
				in->fov, in->diameter,
				in->offsetX, in->offsetY,
				out->offsetX, out->offsetY,
				out->width, out->height,
				out->pan, out->tilt, out->zoom,
				trans->fw_custom.fw_name,
				gdc_format);
	}
	if (ret <= 0) {
		ret = sscanf(config_file, "affine-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%[^-]-%d-%[^.].bin",
			&in->with, &in->height,
			&in->fov, &in->diameter,
			&in->offsetX, &in->offsetY,
			&out->offsetX, &out->offsetY,
			&out->width, &out->height,
			&out->pan, &out->tilt, out->zoom,
			&trans->fw_affine.rotation, gdc_format);

	ctx->gs_with_fw.fw_info.fw_type = AFFINE;

	if (ret > 0)
		printf("affine-%d_%d_%d_%d_%d_%d-%d_%d_%d_%d-%d_%d_%s-%d-%s.bin\n",
			in->with, in->height,
			in->fov, in->diameter,
			in->offsetX, in->offsetY,
			out->offsetX, out->offsetY,
			out->width, out->height,
			out->pan, out->tilt, out->zoom,
			trans->fw_affine.rotation, gdc_format);
	}
	if (ret <= 0) {
		ctx->gs_with_fw.fw_info.fw_name = (char *)config_file;
		ctx->gs_with_fw.fw_info.fw_type = FW_TYPE_MAX;
	}


	return 0;
}

static int gdc_set_input_image(struct gdc_usr_ctx_s *ctx,
			       const char *f_name)
{
	FILE *fp = NULL;
	int r_size = -1;
	int i;

	if (f_name == NULL || ctx == NULL) {
		E_GDC("Error input param\n");
		return r_size;
	}

	fp = fopen(f_name, "rb");
	if (fp == NULL) {
		E_GDC("Error open file %s\n", f_name);
		return -1;
	}

	for (i = 0; i < ctx->plane_number; i++) {
		if (ctx->i_buff[i] == NULL || ctx->i_len[i] == 0) {
			printf("Error input param, plane_id=%d\n", i);
			return r_size;
		}
		r_size = fread(ctx->i_buff[i], ctx->i_len[i], 1, fp);
		if (r_size <= 0) {
			E_GDC("Failed to read file %s\n", f_name);
			return -1;
		}
	}
	fclose(fp);

	return r_size;
}

static void save_imgae(struct gdc_usr_ctx_s *ctx, const char *file_name)
{
	FILE *fp = NULL;
	int i;

	if (ctx == NULL || file_name == NULL) {
		E_GDC("%s:wrong paras\n", __func__);
		return;
	}
	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		E_GDC("%s:Error open file\n", __func__);
		return;
	}
	for (i = 0; i < ctx->plane_number; i++) {
		if (ctx->o_buff[i] == NULL || ctx->o_len[i] == 0) {
			E_GDC("%s:Error input param\n", __func__);
			break;
		}
		printf("gdc: 0x%2x, 0x%2x,0x%2x,0x%2x, 0x%2x,0x%2x,0x%2x,0x%2x\n",
			ctx->o_buff[i][0],
			ctx->o_buff[i][1],
			ctx->o_buff[i][2],
			ctx->o_buff[i][3],
			ctx->o_buff[i][4],
			ctx->o_buff[i][5],
			ctx->o_buff[i][6],
			ctx->o_buff[i][7]);

		fwrite(ctx->o_buff[i], ctx->o_len[i], 1, fp);
	}
	fclose(fp);
}

static inline unsigned long myclock()
{
	struct timeval tv;

	gettimeofday (&tv, NULL);

	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void print_usage(void)
{
	printf ("Usage: gdc_test [options]\n\n");
	printf ("Options:\n\n");
	printf ("  -h                      Print usage information.\n");
	printf ("  -c <string>             config file name.\n");
	printf ("  -t <num>                use custom_fw or not.\n");
	printf ("  -f <num>                format. 1:NV12,2:YV12,3:Y_GREY,4:YUV444_P,5:RGB444_P,\n");
	printf ("  -i <string>             input file name.\n");
	printf ("  -o <string>             output file name.\n");
	printf ("  -p <num>                image plane num.\n");
	printf ("  -w <width x height>     image width x height.\n");
	printf ("  -n <num>                num of process time.\n");
	printf ("  -m <num>                memtype. 0:ION,1:DMABUF.\n");
	printf ("  -d <num>                dev_type, 0:ARM_GDC 1:AML_GDC.\n");
	printf ("\n");
}

int main(int argc, char* argv[]) {
	int c;
	int ret = 0;
	struct gdc_usr_ctx_s ctx;
	uint32_t format;
	uint32_t in_width = 1920;
	uint32_t in_height = 1920;
	uint32_t out_width = 1920;
	uint32_t out_height = 1920;
	uint32_t in_y_stride = 0, in_c_stride = 0;
	uint32_t out_y_stride = 0, out_c_stride = 0;
	int num = 1, plane_number = 1, mem_type = 1;
	int i=0;
	const char *input_file = "input_file";
	const char *output_file = "output_file";
	const char *config_file = "config.bin";
	struct gdc_param g_param;
	int is_custom_fw;
	unsigned long stime;
	int dev_type = 0;

	while (1) {
		static struct option opts[] = {
			{"help", no_argument, 0, 'h'},
			{"config_file", required_argument, 0, 'c'},
			{"custom_fw", required_argument, 0, 't'},
			{"format", required_argument, 0, 'f'},
			{"input_file", required_argument, 0, 'i'},
			{"output_file", required_argument, 0, 'o'},
			{"plane_num", required_argument, 0, 'p'},
			{"stride", required_argument, 0, 's'},
			{"width x height", required_argument, 0, 'w'},
			{"num_of_iter", required_argument, 0, 'n'},
			{"memory_type", required_argument, 0, 'm'},
			{"dev type", required_argument, 0, 'd'}
		};
		int i = 0;
		c = getopt_long(argc, argv, "hc:t:f:i:o:p:s:w:n:m:d:", opts, &i);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_usage();
			return 0;
		case 'c':
			is_custom_fw = 0;
			config_file = optarg;
			printf("config_file: %s\n", config_file);
			break;
		case 't':
			is_custom_fw = 1;
			config_file = optarg;
			printf("custom_fw: %s\n", config_file);
			break;
		case 'f':
			format = atol(optarg);
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			printf("output_file: %s\n", output_file);
			break;
		case 'p':
			plane_number = atol(optarg);
			break;
		case 's':
			sscanf(optarg, "%dx%d-%dx%d",
				&in_y_stride, &in_c_stride,
				&out_y_stride, &out_c_stride);
			printf("parse stride, in: y-%d uv-%d, out:y-%d uv-%d\n",
			in_y_stride, in_c_stride, out_y_stride, out_c_stride);
			break;
		case 'w':
			sscanf(optarg, "%dx%d-%dx%d",
				&in_width, &in_height, &out_width, &out_height);
			printf("parse wxh, in: %dx%d, out:%dx%d\n",
				in_width, in_height, out_width, out_height);
			break;
		case 'n':
			num = atol(optarg);
			break;
		case 'm':
			mem_type = atol(optarg);
			break;
		case 'd':
			dev_type = atol(optarg);
			break;
		}
	}

	ret = check_plane_number(plane_number, format);
	if (ret < 0) {
		E_GDC("Error plane_number=[%d]\n", plane_number);
		return -1;
	}

	g_param.i_width = in_width;
	g_param.i_height = in_height;
	g_param.o_width = out_width;
	g_param.o_height = out_height;
	g_param.format = format;

	memset(&ctx, 0, sizeof(ctx));
	ctx.custom_fw = is_custom_fw;
	ctx.mem_type = mem_type;
	ctx.plane_number = plane_number;
	ctx.dev_type = dev_type;

	ret = gdc_init_cfg(&ctx, &g_param, config_file);
	if (ret < 0) {
		E_GDC("Error gdc init\n");
		gdc_destroy_ctx(&ctx);
		return -1;
	}

	gdc_set_input_image(&ctx, input_file);

	stime = myclock();
	for (i=0; i < num; i++) {
		if (!ctx.custom_fw) {
			ret = gdc_process(&ctx);
			if (ret < 0) {
				E_GDC("ioctl failed\n");
				gdc_destroy_ctx(&ctx);
				return ret;
			}
		} else {
			struct gdc_settings_with_fw *gdc_gw = &ctx.gs_with_fw;
			struct gdc_settings_ex *gdc_gs = &ctx.gs_ex;
			memcpy(&gdc_gw->input_buffer, &gdc_gs->input_buffer,
				sizeof(gdc_buffer_info_t));
			memcpy(&gdc_gw->output_buffer, &gdc_gs->output_buffer,
				sizeof(gdc_buffer_info_t));
			memcpy(&gdc_gw->gdc_config, &gdc_gs->gdc_config,
				sizeof(gdc_config_t));
			parse_custom_fw(&ctx, config_file);
			ret = gdc_process_with_builtin_fw(&ctx);
			if (ret < 0) {
				E_GDC("ioctl failed\n");
				gdc_destroy_ctx(&ctx);
				return ret;
			}
		}
	}
	printf("time=%ld ms\n", myclock() - stime);

	save_imgae(&ctx, output_file);
	gdc_destroy_ctx(&ctx);
	printf("Success save output image, loop=%d\n", i);

	return 0;
}
