/*
 * Aspect ratio modification video filter
 * Copyright (c) 2010 Bobby Bingham

 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * aspect ratio modification video filter
 */

#include "avfilter.h"

typedef struct {
    AVRational aspect;
} AspectContext;

static av_cold int init(AVFilterContext *ctx, const char *args, void *opaque)
{
    AspectContext *aspect = ctx->priv;
    double  ratio;
    int64_t gcd;

    if(args) {
        if(sscanf(args, "%d:%d", &aspect->aspect.num, &aspect->aspect.den) < 2) {
            if(sscanf(args, "%lf", &ratio) < 1)
                return -1;
            aspect->aspect = av_d2q(ratio, 100);
        } else {
            gcd = av_gcd(FFABS(aspect->aspect.num), FFABS(aspect->aspect.den));
            if(gcd) {
                aspect->aspect.num /= gcd;
                aspect->aspect.den /= gcd;
            }
        }
    }

    if(aspect->aspect.den == 0)
#ifndef _MSC_VER
        aspect->aspect = (AVRational) {0, 1};
#else
        aspect->aspect = av_create_rational(0, 1);
#endif

    return 0;
}

static void start_frame(AVFilterLink *link, AVFilterPicRef *picref)
{
    AspectContext *aspect = link->dst->priv;

    picref->pixel_aspect = aspect->aspect;
    avfilter_start_frame(link->dst->outputs[0], picref);
}

#if CONFIG_ASPECT_FILTER
/* for aspect filter, convert from frame aspect ratio to pixel aspect ratio */
static int frameaspect_config_props(AVFilterLink *inlink)
{
    AspectContext *aspect = inlink->dst->priv;

    av_reduce(&aspect->aspect.num, &aspect->aspect.den,
               aspect->aspect.num * inlink->h,
               aspect->aspect.den * inlink->w, 100);

    return 0;
}


AVFilterPad avfilter_vf_aspect_inputs[] = {
	{
		/*name*/ "default",
		/*type*/ AVMEDIA_TYPE_VIDEO,
		/*min_perms*/ 0,
		/*rej_perms*/ 0,
		/*start_frame*/ start_frame,
		/*get_video_buffer*/ avfilter_null_get_video_buffer,
		/*end_frame*/ avfilter_null_end_frame,
		/*draw_slice*/ 0,
		/*poll_frame*/ 0,
		/*request_frame*/ 0,
		/*config_props*/ frameaspect_config_props
	},
	{0}
};

AVFilterPad avfilter_vf_aspect_outputs[] = {
	{
		/*name*/ "default",
		/*type*/ AVMEDIA_TYPE_VIDEO,
		/*min_perms*/ 0,
		/*rej_perms*/ 0,
		/*start_frame*/ 0,
		/*get_video_buffer*/ 0,
		/*end_frame*/ 0,
		/*draw_slice*/ 0,
		/*poll_frame*/ 0,
		/*request_frame*/ 0,
		/*config_props*/ 0
	},
	{0}
};

AVFilter avfilter_vf_aspect = {
#ifndef MSC_STRUCTS
    .name      = "aspect",
    .description = NULL_IF_CONFIG_SMALL("Set the frame aspect ratio."),

    .init      = init,

    .priv_size = sizeof(AspectContext),

    .inputs    = (AVFilterPad[]) {{ .name             = "default",
                                    .type             = AVMEDIA_TYPE_VIDEO,
                                    .config_props     = frameaspect_config_props,
                                    .get_video_buffer = avfilter_null_get_video_buffer,
                                    .start_frame      = start_frame,
                                    .end_frame        = avfilter_null_end_frame },
                                  { .name = NULL}},

    .outputs   = (AVFilterPad[]) {{ .name             = "default",
                                    .type             = AVMEDIA_TYPE_VIDEO, },
                                  { .name = NULL}},
};
#else
	/*name*/ "aspect",
	/*priv_size*/ sizeof(AspectContext),
	/*init*/ init,
	/*uninit*/ 0,
	/*query_formats*/ 0,
	/*inputs*/ avfilter_vf_aspect_inputs,
	/*outputs*/ avfilter_vf_aspect_outputs,
	/*description*/ NULL_IF_CONFIG_SMALL("Set the frame aspect ratio."),
};
#endif
#endif /* CONFIG_ASPECT_FILTER */

#if CONFIG_PIXELASPECT_FILTER
AVFilterPad avfilter_vf_pixelaspect_inputs[] = {
	{
		/*name*/ "default",
		/*type*/ AVMEDIA_TYPE_VIDEO,
		/*min_perms*/ 0,
		/*rej_perms*/ 0,
		/*start_frame*/ start_frame,
		/*get_video_buffer*/ avfilter_null_get_video_buffer,
		/*end_frame*/ avfilter_null_end_frame,
		/*draw_slice*/ 0,
		/*poll_frame*/ 0,
		/*request_frame*/ 0,
		/*config_props*/ 0
	},
	{0}
};

AVFilterPad avfilter_vf_pixelaspect_outputs[] = {
	{
		/*name*/ "default",
		/*type*/ AVMEDIA_TYPE_VIDEO,
		/*min_perms*/ 0,
		/*rej_perms*/ 0,
		/*start_frame*/ 0,
		/*get_video_buffer*/ 0,
		/*end_frame*/ 0,
		/*draw_slice*/ 0,
		/*poll_frame*/ 0,
		/*request_frame*/ 0,
		/*config_props*/ 0
	},
	{0}
};

AVFilter avfilter_vf_pixelaspect = {
#ifndef MSC_STRUCTS
    .name      = "pixelaspect",
    .description = NULL_IF_CONFIG_SMALL("Set the pixel aspect ratio."),

    .init      = init,

    .priv_size = sizeof(AspectContext),

    .inputs    = (AVFilterPad[]) {{ .name             = "default",
                                    .type             = AVMEDIA_TYPE_VIDEO,
                                    .get_video_buffer = avfilter_null_get_video_buffer,
                                    .start_frame      = start_frame,
                                    .end_frame        = avfilter_null_end_frame },
                                  { .name = NULL}},

    .outputs   = (AVFilterPad[]) {{ .name             = "default",
                                    .type             = AVMEDIA_TYPE_VIDEO, },
                                  { .name = NULL}},
};
#else
	/*name*/ "pixelaspect",
	/*priv_size*/ sizeof(AspectContext),
	/*init*/ init,
	/*uninit*/ 0,
	/*query_formats*/ 0,
	/*inputs*/ avfilter_vf_pixelaspect_inputs,
	/*outputs*/ avfilter_vf_pixelaspect_outputs,
	/*description*/ NULL_IF_CONFIG_SMALL("Set the pixel aspect ratio."),
};
#endif
#endif /* CONFIG_PIXELASPECT_FILTER */

