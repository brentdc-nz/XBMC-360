#include <xtl.h>
#include "libavcodec/dsputil.h"
#include "libavcodec/h264data.h"
#include "libavcodec/h264dsp.h"

__vector4 vec_mladd(__vector4 a, __vector4 b, __vector4 v0){
//	__vector4 c = __vmaddfp(vec_unpack(a),vec_unpack(b),v0); //FIXME MARTY: Why broken?

	return ;
};

void biweight_h264_WxH_vmx(uint8_t *dst, uint8_t *src, int stride, int log2_denom,
                               int weightd, int weights, int offset, int w, int h)
{
    int y, dst_aligned, src_aligned;
    //vec_u8 vsrc, vdst;
    //vec_s16 vtemp, vweights, vweightd, voffset, v0, v1, v2, v3;
    //vec_u16 vlog2_denom;

	__vector4 vsrc, vdst;
    __vector4 vtemp, vweights, vweightd, voffset, v0, v1, v2, v3;
    __vector4 vlog2_denom;

    DECLARE_ALIGNED(16, int32_t, temp)[4];
    //LOAD_ZERO;

    offset = ((offset + 1) | 1) << log2_denom;
    temp[0] = log2_denom+1;
    temp[1] = weights;
    temp[2] = weightd;
    temp[3] = offset;

    vtemp = __lvx(temp, 0);
/*   
	vlog2_denom = (vec_u16)vec_splat(vtemp, 1);
    vweights = vec_splat(vtemp, 3);
    vweightd = vec_splat(vtemp, 5);
    voffset = vec_splat(vtemp, 7);
*/
	vlog2_denom = __vsplth(vtemp,1);
	vweights = __vsplth(vtemp,3);
	vweightd = __vsplth(vtemp,5);
	voffset = __vsplth(vtemp,7);

	dst_aligned = !((unsigned long)dst & 0xf);
    src_aligned = !((unsigned long)src & 0xf);

    for (y=0; y<h; y++) {
        vdst = __lvx(dst, 0);
        vsrc = __lvx(src, 0);

        v0 = __vmrghb(__vzero(), vdst);
        v1 = __vmrglb(__vzero(), vdst);
        v2 = __vmrghb(__vzero(), vsrc);
        v3 = __vmrglb(__vzero(), vsrc);

        if (w == 8) {
            if (src_aligned)
                v3 = v2;
            else
                v2 = v3;
        }

        if (w == 16 || dst_aligned) {
            v0 = vec_mladd(v0, vweightd, __vzero());
            v2 = vec_mladd(v2, vweights, __vzero());

            v0 = __vaddshs (v0, voffset);
            v0 = __vaddshs(v0, v2);
            v0 = __vsrah(v0, vlog2_denom);
        }
        if (w == 16 || !dst_aligned) {
            v1 = vec_mladd(v1, vweightd, __vzero());
            v3 = vec_mladd(v3, vweights, __vzero());

            v1 = __vaddshs(v1, voffset);
            v1 = __vaddshs(v1, v3);
            v1 = __vsrah(v1, vlog2_denom);
        }
        vdst = __vpkshus(v0, v1);
        __stvx(vdst, dst, 0);

        dst += stride;
        src += stride;
    }
}

#define H264_WEIGHT(W,H) \
void ff_biweight_h264_pixels ## W ## x ## H ## _vmx(uint8_t *dst, uint8_t *src, int stride, int log2_denom, int weightd, int weights, int offset){ \
    biweight_h264_WxH_vmx(dst, src, stride, log2_denom, weightd, weights, offset, W, H); \
}

H264_WEIGHT(16,16)
H264_WEIGHT(16, 8)
H264_WEIGHT( 8,16)
H264_WEIGHT( 8, 8)
H264_WEIGHT( 8, 4)


