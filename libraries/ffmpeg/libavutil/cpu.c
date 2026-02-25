/*
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

#include "cpu.h"
#include "config.h"
#include "opt.h"

static int flags, checked;

void av_force_cpu_flags(int arg){
    if (   (arg & ( AV_CPU_FLAG_3DNOW    |
                    AV_CPU_FLAG_3DNOWEXT |
                    AV_CPU_FLAG_SSE      |
                    AV_CPU_FLAG_SSE2     |
                    AV_CPU_FLAG_SSE2SLOW |
                    AV_CPU_FLAG_SSE3     |
                    AV_CPU_FLAG_SSE3SLOW |
                    AV_CPU_FLAG_SSSE3    |
                    AV_CPU_FLAG_SSE4     |
                    AV_CPU_FLAG_SSE42    |
                    AV_CPU_FLAG_AVX      |
                    AV_CPU_FLAG_XOP      |
                    AV_CPU_FLAG_FMA4     ))
        && !(arg & AV_CPU_FLAG_MMX)) {
        av_log(NULL, AV_LOG_WARNING, "MMX implied by specified flags\n");
        arg |= AV_CPU_FLAG_MMX;
    }

    flags   = arg;
    checked = arg != -1;
}

int av_get_cpu_flags(void)
{
    if (checked)
        return flags;

    if (ARCH_ARM) flags = ff_get_cpu_flags_arm();
    if (ARCH_PPC) flags = ff_get_cpu_flags_ppc();
    if (ARCH_X86) flags = ff_get_cpu_flags_x86();

    checked = 1;
    return flags;
}

void av_set_cpu_flags_mask(int mask)
{
    checked       = 0;
    flags         = av_get_cpu_flags() & mask;
    checked       = 1;
}

int av_parse_cpu_flags(const char *s)
{
#define CPUFLAG_MMXEXT   (AV_CPU_FLAG_MMX      | AV_CPU_FLAG_MMXEXT | AV_CPU_FLAG_CMOV)
#define CPUFLAG_3DNOW    (AV_CPU_FLAG_3DNOW    | AV_CPU_FLAG_MMX)
#define CPUFLAG_3DNOWEXT (AV_CPU_FLAG_3DNOWEXT | CPUFLAG_3DNOW)
#define CPUFLAG_SSE      (AV_CPU_FLAG_SSE      | CPUFLAG_MMXEXT)
#define CPUFLAG_SSE2     (AV_CPU_FLAG_SSE2     | CPUFLAG_SSE)
#define CPUFLAG_SSE2SLOW (AV_CPU_FLAG_SSE2SLOW | CPUFLAG_SSE2)
#define CPUFLAG_SSE3     (AV_CPU_FLAG_SSE3     | CPUFLAG_SSE2)
#define CPUFLAG_SSE3SLOW (AV_CPU_FLAG_SSE3SLOW | CPUFLAG_SSE3)
#define CPUFLAG_SSSE3    (AV_CPU_FLAG_SSSE3    | CPUFLAG_SSE3)
#define CPUFLAG_SSE4     (AV_CPU_FLAG_SSE4     | CPUFLAG_SSSE3)
#define CPUFLAG_SSE42    (AV_CPU_FLAG_SSE42    | CPUFLAG_SSE4)
#define CPUFLAG_AVX      (AV_CPU_FLAG_AVX      | CPUFLAG_SSE42)
#define CPUFLAG_XOP      (AV_CPU_FLAG_XOP      | CPUFLAG_AVX)
#define CPUFLAG_FMA4     (AV_CPU_FLAG_FMA4     | CPUFLAG_AVX)
    static const AVOption cpuflags_opts[] = {
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#if   ARCH_PPC,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#elif ARCH_X86,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#elif ARCH_ARM,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#endif,
    { NULL },
};
    static const AVClass class = {
    "cpuflags", /* class_name */
    av_default_item_name, /* item_name */
    cpuflags_opts, /* option */
    LIBAVUTIL_VERSION_INT, /* version */
};

    int flags = 0, ret;
    const AVClass *pclass = &class;

    if ((ret = av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, &flags)) < 0)
        return ret;

    return flags & INT_MAX;
}

int av_parse_cpu_caps(unsigned *flags, const char *s)
{
        static const AVOption cpuflags_opts[] = {
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#if   ARCH_PPC,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#elif ARCH_X86,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#define CPU_FLAG_P2 AV_CPU_FLAG_CMOV | AV_CPU_FLAG_MMX,
#define CPU_FLAG_P3 CPU_FLAG_P2 | AV_CPU_FLAG_MMX2 | AV_CPU_FLAG_SSE,
#define CPU_FLAG_P4 CPU_FLAG_P3| AV_CPU_FLAG_SSE2,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#define CPU_FLAG_K62 AV_CPU_FLAG_MMX | AV_CPU_FLAG_3DNOW,
#define CPU_FLAG_ATHLON   CPU_FLAG_K62 | AV_CPU_FLAG_CMOV | AV_CPU_FLAG_3DNOWEXT | AV_CPU_FLAG_MMX2,
#define CPU_FLAG_ATHLONXP CPU_FLAG_ATHLON | AV_CPU_FLAG_SSE,
#define CPU_FLAG_K8  CPU_FLAG_ATHLONXP | AV_CPU_FLAG_SSE2,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#elif ARCH_ARM,
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
    {
        0, /* name */
        0, /* help */
        0, /* offset */
        0, /* type */
        0, /* default_val */
        0, /* min */
        0, /* max */
        0, /* flags */
        "flags", /* unit */
    },
#endif,
    { NULL },
};
    static const AVClass class = {
    "cpuflags", /* class_name */
    av_default_item_name, /* item_name */
    cpuflags_opts, /* option */
    LIBAVUTIL_VERSION_INT, /* version */
};
    const AVClass *pclass = &class;

    return av_opt_eval_flags(&pclass, &cpuflags_opts[0], s, flags);
}
#ifdef TEST

#include <stdio.h>

static const struct {
    int flag;
    const char *name;
} cpu_flag_tab[] = {
#if   ARCH_ARM
    { AV_CPU_FLAG_ARMV5TE,   "armv5te"    },
    { AV_CPU_FLAG_ARMV6,     "armv6"      },
    { AV_CPU_FLAG_ARMV6T2,   "armv6t2"    },
    { AV_CPU_FLAG_VFP,       "vfp"        },
    { AV_CPU_FLAG_VFPV3,     "vfpv3"      },
    { AV_CPU_FLAG_NEON,      "neon"       },
#elif ARCH_PPC
    { AV_CPU_FLAG_ALTIVEC,   "altivec"    },
#elif ARCH_X86
    { AV_CPU_FLAG_MMX,       "mmx"        },
    { AV_CPU_FLAG_MMXEXT,    "mmxext"     },
    { AV_CPU_FLAG_SSE,       "sse"        },
    { AV_CPU_FLAG_SSE2,      "sse2"       },
    { AV_CPU_FLAG_SSE2SLOW,  "sse2(slow)" },
    { AV_CPU_FLAG_SSE3,      "sse3"       },
    { AV_CPU_FLAG_SSE3SLOW,  "sse3(slow)" },
    { AV_CPU_FLAG_SSSE3,     "ssse3"      },
    { AV_CPU_FLAG_ATOM,      "atom"       },
    { AV_CPU_FLAG_SSE4,      "sse4.1"     },
    { AV_CPU_FLAG_SSE42,     "sse4.2"     },
    { AV_CPU_FLAG_AVX,       "avx"        },
    { AV_CPU_FLAG_XOP,       "xop"        },
    { AV_CPU_FLAG_FMA4,      "fma4"       },
    { AV_CPU_FLAG_3DNOW,     "3dnow"      },
    { AV_CPU_FLAG_3DNOWEXT,  "3dnowext"   },
    { AV_CPU_FLAG_CMOV,      "cmov"       },
#endif
    { 0 }
};

int main(void)
{
    int cpu_flags = av_get_cpu_flags();
    int i;

    printf("cpu_flags = 0x%08X\n", cpu_flags);
    printf("cpu_flags =");
    for (i = 0; cpu_flag_tab[i].flag; i++)
        if (cpu_flags & cpu_flag_tab[i].flag)
            printf(" %s", cpu_flag_tab[i].name);
    printf("\n");

    return 0;
}

#endif
