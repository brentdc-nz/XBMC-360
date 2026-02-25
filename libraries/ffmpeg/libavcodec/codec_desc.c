/*
 * This file is part of FFmpeg.
 *
 * This table was generated from the long and short names of AVCodecs
 * please see the respective codec sources for authorship
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

#include <string.h>

#include "avcodec.h"

#include "libavutil/common.h"
#include "libavutil/internal.h"

static const AVCodecDescriptor codec_descriptors[] = {
    {
        AV_CODEC_ID_MPEG1VIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mpeg1video", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-1 video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MPEG2VIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mpeg2video", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-2 video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MPEG2VIDEO_XVMC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mpegvideo_xvmc", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-1/2 video XvMC (X-Video Motion Compensation)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_H261, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "h261", /* name */
        NULL_IF_CONFIG_SMALL("H.261"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_H263, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "h263", /* name */
        NULL_IF_CONFIG_SMALL("H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RV10, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rv10", /* name */
        NULL_IF_CONFIG_SMALL("RealVideo 1.0"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RV20, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rv20", /* name */
        NULL_IF_CONFIG_SMALL("RealVideo 1.0"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MJPEG, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mjpeg", /* name */
        NULL_IF_CONFIG_SMALL("Motion JPEG"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MJPEGB, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mjpegb", /* name */
        NULL_IF_CONFIG_SMALL("Apple MJPEG-B"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_LJPEG, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ljpeg", /* name */
        NULL_IF_CONFIG_SMALL("Lossless JPEG"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_SP5X, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "sp5x", /* name */
        NULL_IF_CONFIG_SMALL("Sunplus JPEG (SP5X)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_JPEGLS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "jpegls", /* name */
        NULL_IF_CONFIG_SMALL("JPEG-LS"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MPEG4, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mpeg4", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-4 part 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RAWVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rawvideo", /* name */
        NULL_IF_CONFIG_SMALL("raw video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MSMPEG4V1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "msmpeg4v1", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MSMPEG4V2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "msmpeg4v2", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MSMPEG4V3, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "msmpeg4v3", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-4 part 2 Microsoft variant version 3"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMV1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "wmv1", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Video 7"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMV2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "wmv2", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Video 8"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_H263P, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "h263p", /* name */
        NULL_IF_CONFIG_SMALL("H.263+ / H.263-1998 / H.263 version 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_H263I, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "h263i", /* name */
        NULL_IF_CONFIG_SMALL("Intel H.263"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FLV1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "flv1", /* name */
        NULL_IF_CONFIG_SMALL("FLV / Sorenson Spark / Sorenson H.263 (Flash Video)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SVQ1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "svq1", /* name */
        NULL_IF_CONFIG_SMALL("Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SVQ3, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "svq3", /* name */
        NULL_IF_CONFIG_SMALL("Sorenson Vector Quantizer 3 / Sorenson Video 3 / SVQ3"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_DVVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dvvideo", /* name */
        NULL_IF_CONFIG_SMALL("DV (Digital Video)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_HUFFYUV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "huffyuv", /* name */
        NULL_IF_CONFIG_SMALL("HuffYUV"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_CYUV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cyuv", /* name */
        NULL_IF_CONFIG_SMALL("Creative YUV (CYUV)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_H264, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "h264", /* name */
        NULL_IF_CONFIG_SMALL("H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10"), /* long_name */
        AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_INDEO3, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "indeo3", /* name */
        NULL_IF_CONFIG_SMALL("Intel Indeo 3"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VP3, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp3", /* name */
        NULL_IF_CONFIG_SMALL("On2 VP3"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_THEORA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "theora", /* name */
        NULL_IF_CONFIG_SMALL("Theora"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ASV1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "asv1", /* name */
        NULL_IF_CONFIG_SMALL("ASUS V1"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ASV2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "asv2", /* name */
        NULL_IF_CONFIG_SMALL("ASUS V2"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FFV1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ffv1", /* name */
        NULL_IF_CONFIG_SMALL("FFmpeg video codec #1"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_4XM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "4xm", /* name */
        NULL_IF_CONFIG_SMALL("4X Movie"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VCR1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vcr1", /* name */
        NULL_IF_CONFIG_SMALL("ATI VCR1"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_CLJR, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cljr", /* name */
        NULL_IF_CONFIG_SMALL("Cirrus Logic AccuPak"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MDEC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mdec", /* name */
        NULL_IF_CONFIG_SMALL("Sony PlayStation MDEC (Motion DECoder)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ROQ, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "roq", /* name */
        NULL_IF_CONFIG_SMALL("id RoQ video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_INTERPLAY_VIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "interplayvideo", /* name */
        NULL_IF_CONFIG_SMALL("Interplay MVE video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_XAN_WC3, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "xan_wc3", /* name */
        NULL_IF_CONFIG_SMALL("Wing Commander III / Xan"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_XAN_WC4, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "xan_wc4", /* name */
        NULL_IF_CONFIG_SMALL("Wing Commander IV / Xxan"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RPZA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rpza", /* name */
        NULL_IF_CONFIG_SMALL("QuickTime video (RPZA)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_CINEPAK, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cinepak", /* name */
        NULL_IF_CONFIG_SMALL("Cinepak"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WS_VQA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ws_vqa", /* name */
        NULL_IF_CONFIG_SMALL("Westwood Studios VQA (Vector Quantized Animation) video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MSRLE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "msrle", /* name */
        NULL_IF_CONFIG_SMALL("Microsoft RLE"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MSVIDEO1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "msvideo1", /* name */
        NULL_IF_CONFIG_SMALL("Microsoft Video 1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_IDCIN, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "idcin", /* name */
        NULL_IF_CONFIG_SMALL("id Quake II CIN video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_8BPS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "8bps", /* name */
        NULL_IF_CONFIG_SMALL("QuickTime 8BPS video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_SMC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "smc", /* name */
        NULL_IF_CONFIG_SMALL("QuickTime Graphics (SMC)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FLIC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "flic", /* name */
        NULL_IF_CONFIG_SMALL("Autodesk Animator Flic video"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_TRUEMOTION1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "truemotion1", /* name */
        NULL_IF_CONFIG_SMALL("Duck TrueMotion 1.0"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VMDVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vmdvideo", /* name */
        NULL_IF_CONFIG_SMALL("Sierra VMD video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MSZH, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mszh", /* name */
        NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) MSZH"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ZLIB, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "zlib", /* name */
        NULL_IF_CONFIG_SMALL("LCL (LossLess Codec Library) ZLIB"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_QTRLE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "qtrle", /* name */
        NULL_IF_CONFIG_SMALL("QuickTime Animation (RLE) video"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_SNOW, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "snow", /* name */
        NULL_IF_CONFIG_SMALL("Snow"), /* long_name */
        AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_TSCC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tscc", /* name */
        NULL_IF_CONFIG_SMALL("TechSmith Screen Capture Codec"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ULTI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ulti", /* name */
        NULL_IF_CONFIG_SMALL("IBM UltiMotion"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_QDRAW, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "qdraw", /* name */
        NULL_IF_CONFIG_SMALL("Apple QuickDraw"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_VIXL, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vixl", /* name */
        NULL_IF_CONFIG_SMALL("Miro VideoXL"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_QPEG, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "qpeg", /* name */
        NULL_IF_CONFIG_SMALL("Q-team QPEG"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PNG, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "png", /* name */
        NULL_IF_CONFIG_SMALL("PNG (Portable Network Graphics) image"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PPM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ppm", /* name */
        NULL_IF_CONFIG_SMALL("PPM (Portable PixelMap) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PBM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "pbm", /* name */
        NULL_IF_CONFIG_SMALL("PBM (Portable BitMap) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PGM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "pgm", /* name */
        NULL_IF_CONFIG_SMALL("PGM (Portable GrayMap) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PGMYUV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "pgmyuv", /* name */
        NULL_IF_CONFIG_SMALL("PGMYUV (Portable GrayMap YUV) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PAM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "pam", /* name */
        NULL_IF_CONFIG_SMALL("PAM (Portable AnyMap) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_FFVHUFF, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ffvhuff", /* name */
        NULL_IF_CONFIG_SMALL("Huffyuv FFmpeg variant"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_RV30, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rv30", /* name */
        NULL_IF_CONFIG_SMALL("RealVideo 3.0"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RV40, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rv40", /* name */
        NULL_IF_CONFIG_SMALL("RealVideo 4.0"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VC1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vc1", /* name */
        NULL_IF_CONFIG_SMALL("SMPTE VC-1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMV3, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "wmv3", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Video 9"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_LOCO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "loco", /* name */
        NULL_IF_CONFIG_SMALL("LOCO"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_WNV1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "wnv1", /* name */
        NULL_IF_CONFIG_SMALL("Winnov WNV1"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AASC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "aasc", /* name */
        NULL_IF_CONFIG_SMALL("Autodesk RLE"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_INDEO2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "indeo2", /* name */
        NULL_IF_CONFIG_SMALL("Intel Indeo 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FRAPS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "fraps", /* name */
        NULL_IF_CONFIG_SMALL("Fraps"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_TRUEMOTION2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "truemotion2", /* name */
        NULL_IF_CONFIG_SMALL("Duck TrueMotion 2.0"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BMP, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "bmp", /* name */
        NULL_IF_CONFIG_SMALL("BMP (Windows and OS/2 bitmap)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_CSCD, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cscd", /* name */
        NULL_IF_CONFIG_SMALL("CamStudio"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MMVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mmvideo", /* name */
        NULL_IF_CONFIG_SMALL("American Laser Games MM Video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ZMBV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "zmbv", /* name */
        NULL_IF_CONFIG_SMALL("Zip Motion Blocks Video"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_AVS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "avs", /* name */
        NULL_IF_CONFIG_SMALL("AVS (Audio Video Standard) video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SMACKVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "smackvideo", /* name */
        NULL_IF_CONFIG_SMALL("Smacker video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_NUV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "nuv", /* name */
        NULL_IF_CONFIG_SMALL("NuppelVideo/RTJPEG"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_KMVC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "kmvc", /* name */
        NULL_IF_CONFIG_SMALL("Karl Morton's video codec"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FLASHSV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "flashsv", /* name */
        NULL_IF_CONFIG_SMALL("Flash Screen Video v1"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_CAVS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cavs", /* name */
        NULL_IF_CONFIG_SMALL("Chinese AVS (Audio Video Standard) (AVS1-P2, JiZhun profile)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_JPEG2000, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "jpeg2000", /* name */
        NULL_IF_CONFIG_SMALL("JPEG 2000"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_VMNC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vmnc", /* name */
        NULL_IF_CONFIG_SMALL("VMware Screen Codec / VMware Video"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_VP5, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp5", /* name */
        NULL_IF_CONFIG_SMALL("On2 VP5"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VP6, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp6", /* name */
        NULL_IF_CONFIG_SMALL("On2 VP6"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VP6F, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp6f", /* name */
        NULL_IF_CONFIG_SMALL("On2 VP6 (Flash version)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TARGA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "targa", /* name */
        NULL_IF_CONFIG_SMALL("Truevision Targa image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_DSICINVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dsicinvideo", /* name */
        NULL_IF_CONFIG_SMALL("Delphine Software International CIN video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TIERTEXSEQVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tiertexseqvideo", /* name */
        NULL_IF_CONFIG_SMALL("Tiertex Limited SEQ video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TIFF, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tiff", /* name */
        NULL_IF_CONFIG_SMALL("TIFF image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_GIF, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "gif", /* name */
        NULL_IF_CONFIG_SMALL("GIF (Graphics Interchange Format)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_DXA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dxa", /* name */
        NULL_IF_CONFIG_SMALL("Feeble Files/ScummVM DXA"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_DNXHD, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dnxhd", /* name */
        NULL_IF_CONFIG_SMALL("VC3/DNxHD"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_THP, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "thp", /* name */
        NULL_IF_CONFIG_SMALL("Nintendo Gamecube THP video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SGI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "sgi", /* name */
        NULL_IF_CONFIG_SMALL("SGI image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_SGIRLE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "sgirle", /* name */
        NULL_IF_CONFIG_SMALL("SGI RLE 8-bit"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_C93, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "c93", /* name */
        NULL_IF_CONFIG_SMALL("Interplay C93"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BETHSOFTVID, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "bethsoftvid", /* name */
        NULL_IF_CONFIG_SMALL("Bethesda VID video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PTX, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ptx", /* name */
        NULL_IF_CONFIG_SMALL("V.Flash PTX image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TXD, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "txd", /* name */
        NULL_IF_CONFIG_SMALL("Renderware TXD (TeXture Dictionary) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VP6A, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp6a", /* name */
        NULL_IF_CONFIG_SMALL("On2 VP6 (Flash version, with alpha channel)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AMV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "amv", /* name */
        NULL_IF_CONFIG_SMALL("AMV Video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VB, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vb", /* name */
        NULL_IF_CONFIG_SMALL("Beam Software VB"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PCX, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "pcx", /* name */
        NULL_IF_CONFIG_SMALL("PC Paintbrush PCX image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_SUNRAST, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "sunrast", /* name */
        NULL_IF_CONFIG_SMALL("Sun Rasterfile image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_INDEO4, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "indeo4", /* name */
        NULL_IF_CONFIG_SMALL("Intel Indeo Video Interactive 4"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_INDEO5, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "indeo5", /* name */
        NULL_IF_CONFIG_SMALL("Intel Indeo Video Interactive 5"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MIMIC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mimic", /* name */
        NULL_IF_CONFIG_SMALL("Mimic"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RL2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "rl2", /* name */
        NULL_IF_CONFIG_SMALL("RL2 video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ESCAPE124, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "escape124", /* name */
        NULL_IF_CONFIG_SMALL("Escape 124"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_DIRAC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dirac", /* name */
        NULL_IF_CONFIG_SMALL("Dirac"), /* long_name */
        AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_BFI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "bfi", /* name */
        NULL_IF_CONFIG_SMALL("Brute Force & Ignorance"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_CMV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cmv", /* name */
        NULL_IF_CONFIG_SMALL("Electronic Arts CMV video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MOTIONPIXELS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "motionpixels", /* name */
        NULL_IF_CONFIG_SMALL("Motion Pixels video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TGV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tgv", /* name */
        NULL_IF_CONFIG_SMALL("Electronic Arts TGV video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TGQ, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tgq", /* name */
        NULL_IF_CONFIG_SMALL("Electronic Arts TGQ video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TQI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tqi", /* name */
        NULL_IF_CONFIG_SMALL("Electronic Arts TQI video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AURA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "aura", /* name */
        NULL_IF_CONFIG_SMALL("Auravision AURA"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AURA2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "aura2", /* name */
        NULL_IF_CONFIG_SMALL("Auravision Aura 2"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_V210X, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "v210x", /* name */
        NULL, /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_TMV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tmv", /* name */
        NULL_IF_CONFIG_SMALL("8088flex TMV"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_V210, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "v210", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_DPX, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dpx", /* name */
        NULL_IF_CONFIG_SMALL("DPX image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MAD, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mad", /* name */
        NULL_IF_CONFIG_SMALL("Electronic Arts Madcow Video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FRWU, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "frwu", /* name */
        NULL_IF_CONFIG_SMALL("Forward Uncompressed"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_FLASHSV2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "flashsv2", /* name */
        NULL_IF_CONFIG_SMALL("Flash Screen Video v2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_CDGRAPHICS, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cdgraphics", /* name */
        NULL_IF_CONFIG_SMALL("CD Graphics video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_R210, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "r210", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed RGB 10-bit"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ANM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "anm", /* name */
        NULL_IF_CONFIG_SMALL("Deluxe Paint Animation"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BINKVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "binkvideo", /* name */
        NULL_IF_CONFIG_SMALL("Bink video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_IFF_ILBM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "iff_ilbm", /* name */
        NULL_IF_CONFIG_SMALL("IFF ILBM"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_IFF_BYTERUN1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "iff_byterun1", /* name */
        NULL_IF_CONFIG_SMALL("IFF ByteRun1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_KGV1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "kgv1", /* name */
        NULL_IF_CONFIG_SMALL("Kega Game Video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_YOP, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "yop", /* name */
        NULL_IF_CONFIG_SMALL("Psygnosis YOP Video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VP8, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp8", /* name */
        NULL_IF_CONFIG_SMALL("On2 VP8"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VP9, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vp9", /* name */
        NULL_IF_CONFIG_SMALL("Google VP9"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PICTOR, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "pictor", /* name */
        NULL_IF_CONFIG_SMALL("Pictor/PC Paint"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ANSI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ansi", /* name */
        NULL_IF_CONFIG_SMALL("ASCII/ANSI art"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_A64_MULTI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "a64_multi", /* name */
        NULL_IF_CONFIG_SMALL("Multicolor charset for Commodore 64"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_A64_MULTI5, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "a64_multi5", /* name */
        NULL_IF_CONFIG_SMALL("Multicolor charset for Commodore 64, extended with 5th color (colram)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_R10K, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "r10k", /* name */
        NULL_IF_CONFIG_SMALL("AJA Kona 10-bit RGB Codec"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MVC1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mvc1", /* name */
        NULL_IF_CONFIG_SMALL("Silicon Graphics Motion Video Compressor 1"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MVC2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mvc2", /* name */
        NULL_IF_CONFIG_SMALL("Silicon Graphics Motion Video Compressor 2"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MXPEG, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mxpeg", /* name */
        NULL_IF_CONFIG_SMALL("Mobotix MxPEG video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_LAGARITH, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "lagarith", /* name */
        NULL_IF_CONFIG_SMALL("Lagarith lossless"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PRORES, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "prores", /* name */
        NULL_IF_CONFIG_SMALL("Apple ProRes (iCodec Pro)"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_JV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "jv", /* name */
        NULL_IF_CONFIG_SMALL("Bitmap Brothers JV video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_DFA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dfa", /* name */
        NULL_IF_CONFIG_SMALL("Chronomaster DFA"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMV3IMAGE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "wmv3image", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Video 9 Image"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VC1IMAGE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vc1image", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Video 9 Image v2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_UTVIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "utvideo", /* name */
        NULL_IF_CONFIG_SMALL("Ut Video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_BMV_VIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "bmv_video", /* name */
        NULL_IF_CONFIG_SMALL("Discworld II BMV video"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_VBLE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "vble", /* name */
        NULL_IF_CONFIG_SMALL("VBLE Lossless Codec"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_DXTORY, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "dxtory", /* name */
        NULL_IF_CONFIG_SMALL("Dxtory"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_V410, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "v410", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed 4:4:4 10-bit"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_XWD, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "xwd", /* name */
        NULL_IF_CONFIG_SMALL("XWD (X Window Dump) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_CDXL, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cdxl", /* name */
        NULL_IF_CONFIG_SMALL("Commodore CDXL video"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_XBM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "xbm", /* name */
        NULL_IF_CONFIG_SMALL("XBM (X BitMap) image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ZEROCODEC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "zerocodec", /* name */
        NULL_IF_CONFIG_SMALL("ZeroCodec Lossless Video"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MSS1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mss1", /* name */
        NULL_IF_CONFIG_SMALL("MS Screen 1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MSA1, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "msa1", /* name */
        NULL_IF_CONFIG_SMALL("MS ATC Screen"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TSCC2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "tscc2", /* name */
        NULL_IF_CONFIG_SMALL("TechSmith Screen Codec 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MTS2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mts2", /* name */
        NULL_IF_CONFIG_SMALL("MS Expression Encoder Screen"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_CLLC, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cllc", /* name */
        NULL_IF_CONFIG_SMALL("Canopus Lossless Codec"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MSS2, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "mss2", /* name */
        NULL_IF_CONFIG_SMALL("MS Windows Media Video V9 Screen"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_Y41P, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "y41p", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed YUV 4:1:1 12-bit"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_ESCAPE130, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "escape130", /* name */
        NULL_IF_CONFIG_SMALL("Escape 130"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_EXR, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "exr", /* name */
        NULL_IF_CONFIG_SMALL("OpenEXR image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_AVRP, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "avrp", /* name */
        NULL_IF_CONFIG_SMALL("Avid 1:1 10-bit RGB Packer"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_012V, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "012v", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed 4:2:2 10-bit"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_G2M, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "g2m", /* name */
        NULL_IF_CONFIG_SMALL("GoToMeeting"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_AVUI, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "avui", /* name */
        NULL_IF_CONFIG_SMALL("Avid Meridien Uncompressed"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_AYUV, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "ayuv", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed packed MS 4:4:4:4"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_TARGA_Y216, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "targa_y216", /* name */
        NULL_IF_CONFIG_SMALL("Pinnacle TARGA CineWave YUV16"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_V308, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "v308", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed packed 4:4:4"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_V408, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "v408", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed packed QT 4:4:4:4"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_YUV4, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "yuv4", /* name */
        NULL_IF_CONFIG_SMALL("Uncompressed packed 4:2:0"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_SANM, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "sanm", /* name */
        NULL_IF_CONFIG_SMALL("LucasArts SMUSH video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PAF_VIDEO, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "paf_video", /* name */
        NULL_IF_CONFIG_SMALL("Amazing Studio Packed Animation File Video"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AVRN, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "avrn", /* name */
        NULL_IF_CONFIG_SMALL("Avid AVI Codec"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_CPIA, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "cpia", /* name */
        NULL_IF_CONFIG_SMALL("CPiA video format"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_XFACE, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "xface", /* name */
        NULL_IF_CONFIG_SMALL("X-face image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BRENDER_PIX, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "brender_pix", /* name */
        NULL_IF_CONFIG_SMALL("BRender PIX image"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S16LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s16le", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 16-bit little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S16BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s16be", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 16-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U16LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u16le", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 16-bit little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U16BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u16be", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 16-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S8, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s8", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 8-bit"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U8, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u8", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 8-bit"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_MULAW, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_mulaw", /* name */
        NULL_IF_CONFIG_SMALL("PCM mu-law / G.711 mu-law"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PCM_ALAW, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_alaw", /* name */
        NULL_IF_CONFIG_SMALL("PCM A-law / G.711 A-law"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PCM_S32LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s32le", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 32-bit little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S32BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s32be", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 32-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U32LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u32le", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 32-bit little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U32BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u32be", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 32-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S24LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s24le", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 24-bit little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S24BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s24be", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 24-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U24LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u24le", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 24-bit little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_U24BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_u24be", /* name */
        NULL_IF_CONFIG_SMALL("PCM unsigned 24-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S24DAUD, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s24daud", /* name */
        NULL_IF_CONFIG_SMALL("PCM D-Cinema audio signed 24-bit"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_ZORK, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_zork", /* name */
        NULL_IF_CONFIG_SMALL("PCM Zork"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PCM_S16BE_PLANAR, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s16be_planar", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 16-bit big-endian planar"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S16LE_PLANAR, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s16le_planar", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 16-bit little-endian planar"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S24LE_PLANAR, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s24le_planar", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 24-bit little-endian planar"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_S32LE_PLANAR, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s32le_planar", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 32-bit little-endian planar"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_DVD, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_dvd", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 20|24-bit big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_F32BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_f32be", /* name */
        NULL_IF_CONFIG_SMALL("PCM 32-bit floating point big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_F32LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_f32le", /* name */
        NULL_IF_CONFIG_SMALL("PCM 32-bit floating point little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_F64BE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_f64be", /* name */
        NULL_IF_CONFIG_SMALL("PCM 64-bit floating point big-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_F64LE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_f64le", /* name */
        NULL_IF_CONFIG_SMALL("PCM 64-bit floating point little-endian"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_BLURAY, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_bluray", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 16|20|24-bit big-endian for Blu-ray media"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_PCM_LXF, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_lxf", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 20-bit little-endian planar"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_S302M, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "s302m", /* name */
        NULL_IF_CONFIG_SMALL("SMPTE 302M"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_PCM_S8_PLANAR, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "pcm_s8_planar", /* name */
        NULL_IF_CONFIG_SMALL("PCM signed 8-bit planar"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_QT, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_qt", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA QuickTime"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_WAV, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_wav", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA WAV"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_DK3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_dk3", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Duck DK3"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_DK4, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_dk4", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Duck DK4"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_WS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_ws", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Westwood"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_SMJPEG, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_smjpeg", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Loki SDL MJPEG"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_MS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ms", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Microsoft"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_4XM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_4xm", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM 4X Movie"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_XA, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_xa", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM CDROM XA"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_ADX, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_adx", /* name */
        NULL_IF_CONFIG_SMALL("SEGA CRI ADX ADPCM"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_EA, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ea", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_G726, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_g726", /* name */
        NULL_IF_CONFIG_SMALL("G.726 ADPCM"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_CT, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ct", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Creative Technology"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_SWF, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_swf", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Shockwave Flash"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_YAMAHA, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_yamaha", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Yamaha"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_SBPRO_4, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_sbpro_4", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Sound Blaster Pro 4-bit"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_SBPRO_3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_sbpro_3", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Sound Blaster Pro 2.6-bit"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_SBPRO_2, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_sbpro_2", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Sound Blaster Pro 2-bit"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_THP, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_thp", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Nintendo Gamecube THP"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_AMV, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_amv", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA AMV"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_EA_R1, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ea_r1", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts R1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_EA_R3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ea_r3", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts R3"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_EA_R2, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ea_r2", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts R2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_EA_SEAD, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_ea_sead", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Electronic Arts SEAD"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_EA_EACS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_ea_eacs", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Electronic Arts EACS"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_EA_XAS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ea_xas", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts XAS"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_EA_MAXIS_XA, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ea_maxis_xa", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Electronic Arts Maxis CDROM XA"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_ISS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_iss", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Funcom ISS"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_G722, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_g722", /* name */
        NULL_IF_CONFIG_SMALL("G.722 ADPCM"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_APC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_apc", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA CRYO APC"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_AFC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_afc", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM Nintendo Gamecube AFC"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ADPCM_IMA_OKI, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "adpcm_ima_oki", /* name */
        NULL_IF_CONFIG_SMALL("ADPCM IMA Dialogic OKI"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AMR_NB, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "amr_nb", /* name */
        NULL_IF_CONFIG_SMALL("AMR-NB (Adaptive Multi-Rate NarrowBand)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AMR_WB, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "amr_wb", /* name */
        NULL_IF_CONFIG_SMALL("AMR-WB (Adaptive Multi-Rate WideBand)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RA_144, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "ra_144", /* name */
        NULL_IF_CONFIG_SMALL("RealAudio 1.0 (14.4K)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RA_288, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "ra_288", /* name */
        NULL_IF_CONFIG_SMALL("RealAudio 2.0 (28.8K)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ROQ_DPCM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "roq_dpcm", /* name */
        NULL_IF_CONFIG_SMALL("DPCM id RoQ"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_INTERPLAY_DPCM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "interplay_dpcm", /* name */
        NULL_IF_CONFIG_SMALL("DPCM Interplay"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_XAN_DPCM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "xan_dpcm", /* name */
        NULL_IF_CONFIG_SMALL("DPCM Xan"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SOL_DPCM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "sol_dpcm", /* name */
        NULL_IF_CONFIG_SMALL("DPCM Sol"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MP2, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mp2", /* name */
        NULL_IF_CONFIG_SMALL("MP2 (MPEG audio layer 2)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MP3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mp3", /* name */
        NULL_IF_CONFIG_SMALL("MP3 (MPEG audio layer 3)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AAC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "aac", /* name */
        NULL_IF_CONFIG_SMALL("AAC (Advanced Audio Coding)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AC3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "ac3", /* name */
        NULL_IF_CONFIG_SMALL("ATSC A/52A (AC-3)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_DTS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "dts", /* name */
        NULL_IF_CONFIG_SMALL("DCA (DTS Coherent Acoustics)"), /* long_name */
        AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_VORBIS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "vorbis", /* name */
        NULL_IF_CONFIG_SMALL("Vorbis"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_DVAUDIO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "dvaudio", /* name */
        NULL, /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMAV1, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wmav1", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Audio 1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMAV2, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wmav2", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Audio 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MACE3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mace3", /* name */
        NULL_IF_CONFIG_SMALL("MACE (Macintosh Audio Compression/Expansion) 3:1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MACE6, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mace6", /* name */
        NULL_IF_CONFIG_SMALL("MACE (Macintosh Audio Compression/Expansion) 6:1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VMDAUDIO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "vmdaudio", /* name */
        NULL_IF_CONFIG_SMALL("Sierra VMD audio"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FLAC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "flac", /* name */
        NULL_IF_CONFIG_SMALL("FLAC (Free Lossless Audio Codec)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MP3ADU, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mp3adu", /* name */
        NULL_IF_CONFIG_SMALL("ADU (Application Data Unit) MP3 (MPEG audio layer 3)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MP3ON4, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mp3on4", /* name */
        NULL_IF_CONFIG_SMALL("MP3onMP4"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SHORTEN, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "shorten", /* name */
        NULL_IF_CONFIG_SMALL("Shorten"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ALAC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "alac", /* name */
        NULL_IF_CONFIG_SMALL("ALAC (Apple Lossless Audio Codec)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_WESTWOOD_SND1, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "westwood_snd1", /* name */
        NULL_IF_CONFIG_SMALL("Westwood Audio (SND1)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_GSM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "gsm", /* name */
        NULL_IF_CONFIG_SMALL("GSM"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_QDM2, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "qdm2", /* name */
        NULL_IF_CONFIG_SMALL("QDesign Music Codec 2"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_COOK, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "cook", /* name */
        NULL_IF_CONFIG_SMALL("Cook / Cooker / Gecko (RealAudio G2)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TRUESPEECH, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "truespeech", /* name */
        NULL_IF_CONFIG_SMALL("DSP Group TrueSpeech"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TTA, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "tta", /* name */
        NULL_IF_CONFIG_SMALL("TTA (True Audio)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_SMACKAUDIO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "smackaudio", /* name */
        NULL_IF_CONFIG_SMALL("Smacker audio"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_QCELP, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "qcelp", /* name */
        NULL_IF_CONFIG_SMALL("QCELP / PureVoice"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WAVPACK, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wavpack", /* name */
        NULL_IF_CONFIG_SMALL("WavPack"), /* long_name */
        AV_CODEC_PROP_LOSSY | AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_DSICINAUDIO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "dsicinaudio", /* name */
        NULL_IF_CONFIG_SMALL("Delphine Software International CIN audio"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_IMC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "imc", /* name */
        NULL_IF_CONFIG_SMALL("IMC (Intel Music Coder)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MUSEPACK7, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "musepack7", /* name */
        NULL_IF_CONFIG_SMALL("Musepack SV7"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MLP, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mlp", /* name */
        NULL_IF_CONFIG_SMALL("MLP (Meridian Lossless Packing)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_GSM_MS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "gsm_ms", /* name */
        NULL_IF_CONFIG_SMALL("GSM Microsoft variant"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ATRAC3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "atrac3", /* name */
        NULL_IF_CONFIG_SMALL("Atrac 3 (Adaptive TRansform Acoustic Coding 3)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VOXWARE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "voxware", /* name */
        NULL_IF_CONFIG_SMALL("Voxware RT29 Metasound"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_APE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "ape", /* name */
        NULL_IF_CONFIG_SMALL("Monkey's Audio"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_NELLYMOSER, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "nellymoser", /* name */
        NULL_IF_CONFIG_SMALL("Nellymoser Asao"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MUSEPACK8, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "musepack8", /* name */
        NULL_IF_CONFIG_SMALL("Musepack SV8"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SPEEX, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "speex", /* name */
        NULL_IF_CONFIG_SMALL("Speex"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMAVOICE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wmavoice", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Audio Voice"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMAPRO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wmapro", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Audio 9 Professional"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_WMALOSSLESS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wmalossless", /* name */
        NULL_IF_CONFIG_SMALL("Windows Media Audio Lossless"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ATRAC3P, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "atrac3p", /* name */
        NULL_IF_CONFIG_SMALL("Sony ATRAC3+"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_EAC3, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "eac3", /* name */
        NULL_IF_CONFIG_SMALL("ATSC A/52B (AC-3, E-AC-3)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SIPR, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "sipr", /* name */
        NULL_IF_CONFIG_SMALL("RealAudio SIPR / ACELP.NET"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_MP1, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mp1", /* name */
        NULL_IF_CONFIG_SMALL("MP1 (MPEG audio layer 1)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TWINVQ, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "twinvq", /* name */
        NULL_IF_CONFIG_SMALL("VQF TwinVQ"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TRUEHD, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "truehd", /* name */
        NULL_IF_CONFIG_SMALL("TrueHD"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_MP4ALS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "mp4als", /* name */
        NULL_IF_CONFIG_SMALL("MPEG-4 Audio Lossless Coding (ALS)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_ATRAC1, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "atrac1", /* name */
        NULL_IF_CONFIG_SMALL("Atrac 1 (Adaptive TRansform Acoustic Coding)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BINKAUDIO_RDFT, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "binkaudio_rdft", /* name */
        NULL_IF_CONFIG_SMALL("Bink Audio (RDFT)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BINKAUDIO_DCT, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "binkaudio_dct", /* name */
        NULL_IF_CONFIG_SMALL("Bink Audio (DCT)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_AAC_LATM, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "aac_latm", /* name */
        NULL_IF_CONFIG_SMALL("AAC LATM (Advanced Audio Coding LATM syntax)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_QDMC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "qdmc", /* name */
        NULL_IF_CONFIG_SMALL("QDesign Music"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_CELT, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "celt", /* name */
        NULL_IF_CONFIG_SMALL("Constrained Energy Lapped Transform (CELT)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_G723_1, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "g723_1", /* name */
        NULL_IF_CONFIG_SMALL("G.723.1"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_G729, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "g729", /* name */
        NULL_IF_CONFIG_SMALL("G.729"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_8SVX_EXP, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "8svx_exp", /* name */
        NULL_IF_CONFIG_SMALL("8SVX exponential"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_8SVX_FIB, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "8svx_fib", /* name */
        NULL_IF_CONFIG_SMALL("8SVX fibonacci"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_BMV_AUDIO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "bmv_audio", /* name */
        NULL_IF_CONFIG_SMALL("Discworld II BMV audio"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_RALF, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "ralf", /* name */
        NULL_IF_CONFIG_SMALL("RealAudio Lossless"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_IAC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "iac", /* name */
        NULL_IF_CONFIG_SMALL("IAC (Indeo Audio Coder)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_ILBC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "ilbc", /* name */
        NULL_IF_CONFIG_SMALL("iLBC (Internet Low Bitrate Codec)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_VIMA, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "vima", /* name */
        NULL_IF_CONFIG_SMALL("LucasArts VIMA audio"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_FFWAVESYNTH, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "wavesynth", /* name */
        NULL_IF_CONFIG_SMALL("Wave synthesis pseudo-codec"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SONIC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "sonic", /* name */
        NULL_IF_CONFIG_SMALL("Sonic"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SONIC_LS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "sonicls", /* name */
        NULL_IF_CONFIG_SMALL("Sonic lossless"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_PAF_AUDIO, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "paf_audio", /* name */
        NULL_IF_CONFIG_SMALL("Amazing Studio Packed Animation File Audio"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_OPUS, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "opus", /* name */
        NULL_IF_CONFIG_SMALL("Opus (Opus Interactive Audio Codec)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_COMFORT_NOISE, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "comfortnoise", /* name */
        NULL_IF_CONFIG_SMALL("RFC 3389 Comfort Noise"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_TAK, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "tak", /* name */
        NULL_IF_CONFIG_SMALL("TAK (Tom's lossless Audio Kompressor)"), /* long_name */
        AV_CODEC_PROP_LOSSLESS, /* props */
    },
    {
        AV_CODEC_ID_EVRC, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "evrc", /* name */
        NULL_IF_CONFIG_SMALL("EVRC (Enhanced Variable Rate Codec)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_SMV, /* id */
        AVMEDIA_TYPE_AUDIO, /* type */
        "smv", /* name */
        NULL_IF_CONFIG_SMALL("SMV (Selectable Mode Vocoder)"), /* long_name */
        AV_CODEC_PROP_LOSSY, /* props */
    },
    {
        AV_CODEC_ID_DVD_SUBTITLE, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "dvd_subtitle", /* name */
        NULL_IF_CONFIG_SMALL("DVD subtitles"), /* long_name */
        AV_CODEC_PROP_BITMAP_SUB, /* props */
    },
    {
        AV_CODEC_ID_DVB_SUBTITLE, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "dvb_subtitle", /* name */
        NULL_IF_CONFIG_SMALL("DVB subtitles"), /* long_name */
        AV_CODEC_PROP_BITMAP_SUB, /* props */
    },
    {
        AV_CODEC_ID_TEXT, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "text", /* name */
        NULL_IF_CONFIG_SMALL("raw UTF-8 text"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_XSUB, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "xsub", /* name */
        NULL_IF_CONFIG_SMALL("XSUB"), /* long_name */
        AV_CODEC_PROP_BITMAP_SUB, /* props */
    },
    {
        AV_CODEC_ID_SSA, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "ssa", /* name */
        NULL_IF_CONFIG_SMALL("SSA (SubStation Alpha) / ASS (Advanced SSA) subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_MOV_TEXT, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "mov_text", /* name */
        NULL_IF_CONFIG_SMALL("MOV text"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_HDMV_PGS_SUBTITLE, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "hdmv_pgs_subtitle", /* name */
        NULL_IF_CONFIG_SMALL("HDMV Presentation Graphic Stream subtitles"), /* long_name */
        AV_CODEC_PROP_BITMAP_SUB, /* props */
    },
    {
        AV_CODEC_ID_DVB_TELETEXT, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "dvb_teletext", /* name */
        NULL_IF_CONFIG_SMALL("DVB teletext"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SRT, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "srt", /* name */
        NULL_IF_CONFIG_SMALL("SubRip subtitle with embedded timing"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SUBRIP, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "subrip", /* name */
        NULL_IF_CONFIG_SMALL("SubRip subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_MICRODVD, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "microdvd", /* name */
        NULL_IF_CONFIG_SMALL("MicroDVD subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_MPL2, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "mpl2", /* name */
        NULL_IF_CONFIG_SMALL("MPL2 subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_EIA_608, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "eia_608", /* name */
        NULL_IF_CONFIG_SMALL("EIA-608 closed captions"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_JACOSUB, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "jacosub", /* name */
        NULL_IF_CONFIG_SMALL("JACOsub subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_PJS, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "pjs", /* name */
        NULL_IF_CONFIG_SMALL("PJS (Phoenix Japanimation Society) subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SAMI, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "sami", /* name */
        NULL_IF_CONFIG_SMALL("SAMI subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_REALTEXT, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "realtext", /* name */
        NULL_IF_CONFIG_SMALL("RealText subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SUBVIEWER1, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "subviewer1", /* name */
        NULL_IF_CONFIG_SMALL("SubViewer v1 subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_SUBVIEWER, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "subviewer", /* name */
        NULL_IF_CONFIG_SMALL("SubViewer subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_VPLAYER, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "vplayer", /* name */
        NULL_IF_CONFIG_SMALL("VPlayer subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_WEBVTT, /* id */
        AVMEDIA_TYPE_SUBTITLE, /* type */
        "webvtt", /* name */
        NULL_IF_CONFIG_SMALL("WebVTT subtitle"), /* long_name */
        0, /* props */
    },
    {
        AV_CODEC_ID_BINTEXT, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "bintext", /* name */
        NULL_IF_CONFIG_SMALL("Binary text"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_XBIN, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "xbin", /* name */
        NULL_IF_CONFIG_SMALL("eXtended BINary text"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_IDF, /* id */
        AVMEDIA_TYPE_VIDEO, /* type */
        "idf", /* name */
        NULL_IF_CONFIG_SMALL("iCEDraw text"), /* long_name */
        AV_CODEC_PROP_INTRA_ONLY, /* props */
    },
    {
        AV_CODEC_ID_SMPTE_KLV, /* id */
        AVMEDIA_TYPE_DATA, /* type */
        "klv", /* name */
        NULL_IF_CONFIG_SMALL("SMPTE 336M Key-Length-Value (KLV) metadata"), /* long_name */
        0, /* props */
    },
};

const AVCodecDescriptor *avcodec_descriptor_get(enum AVCodecID id)
{
    int i;

    for (i = 0; i < FF_ARRAY_ELEMS(codec_descriptors); i++)
        if (codec_descriptors[i].id == id)
            return &codec_descriptors[i];
    return NULL;
}

const AVCodecDescriptor *avcodec_descriptor_next(const AVCodecDescriptor *prev)
{
    if (!prev)
        return &codec_descriptors[0];
    if (prev - codec_descriptors < FF_ARRAY_ELEMS(codec_descriptors) - 1)
        return prev + 1;
    return NULL;
}

const AVCodecDescriptor *avcodec_descriptor_get_by_name(const char *name)
{
    const AVCodecDescriptor *desc = NULL;

    while ((desc = avcodec_descriptor_next(desc))) {
        if (!strcmp(desc->name, name))
            return desc;
    }
    return NULL;
}
