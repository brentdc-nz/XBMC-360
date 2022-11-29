void put_h264_chroma_mc4_vmx(uint8_t *dst/*align 8*/, uint8_t *src/*align 1*/, int stride, int h, int x, int y)
	const int A=(8-x)*(8-y);
	const int B=(  x)*(8-y);
	const int C=(8-x)*(  y);
	const int D=(  x)*(  y);

	int32_t * src32 = (int32_t *) src;
	int32_t * dst32 = (int32_t *) dst;

	//__vector4 vABCD = {((8-x)*(8-y)), ((  x)*(8-y)), ((8-x)*(  y)), ((  x)*(  y)) };

	__vector4 vAs,vBs,vCs,vDs,vOp;

	__vector4 v1,v6,v32;

	__vector4 vDst;

	v1=__vsplitsw(1);
	v6=__vsplitsw(6);
	v32=__vsplitsw(32);

	if(D){
		for(i=0; i<h; i++){
			/** Original c code to convert 
			OP(dst[0], (A*src[0] + B*src[1] + C*src[stride+0] + D*src[stride+1]));
			OP(dst[1], (A*src[1] + B*src[2] + C*src[stride+1] + D*src[stride+2]));
			OP(dst[2], (A*src[2] + B*src[3] + C*src[stride+2] + D*src[stride+3]));
			OP(dst[3], (A*src[3] + B*src[4] + C*src[stride+3] + D*src[stride+4]));	
			**/
			
			//Vmx doesn't have any word multiplication so we leave it ...
			/*
			vadduws  
				vDst.x = vSrcA.x + vSrcB.x
				vDst.y = vSrcA.y + vSrcB.y
				vDst.z = vSrcA.z + vSrcB.z
				vDst.w = vSrcA.w + vSrcB.w
			*/
			vAs = { A*src[0], A*src[1], A*src[2], A*src[3] };
			vBs = { B*src[1], B*src[2], B*src[3], B*src[4] };
			vCs = { C*src[stride+0], C*src[stride+1], C*src[stride+2], C*src[stride+3] };
			vDs = { D*src[stride+1], D*src[stride+2], D*src[stride+3], D*src[stride+4] };
				
			//vadduws  or vadduwm   ??
			vT0 = __vadduwm(vAs,vBs);//vAs+vBs
			vT1 = __vadduwm(vCs,vDs);//vCs+vDs
			
			vOp = __vadduwm(vCs,vDs);///vT0,vT1;
			
			//Now do Operation ... (OP #define)
			// #define op_put(a, b) a = (((b) + 32)>>6) //in all case ?
			vDst = __vsrw(__vadduwm(vOp,v32),v6);
			
			//Or
			
			// #define op_avg(a, b) a = (((a)+(((b) + 32)>>6)+1)>>1)
			//vDst = __vavguw(vDst,  __vadduwm(vOp,v32),v6);
			
			//Save dst
			__stvx(vDst,dst,0);
			
			dst+= stride;
			src+= stride;
		}
	}
	else{
		/**
		OP(dst[0], (A*src[0] + E*src[step+0]));\
		OP(dst[1], (A*src[1] + E*src[step+1]));\
		OP(dst[2], (A*src[2] + E*src[step+2]));\
		OP(dst[3], (A*src[3] + E*src[step+3]));\
		dst+= stride;\
		src+= stride;\
		*/
	}
}