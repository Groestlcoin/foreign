#define UCFG_DETECT_MISMATCH 0

#include "openssl-config.h"

#include <openssl/crypto/bn/bn_lcl.h>

#undef bn_mul_mont

extern "C" {
	int bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp, const BN_ULONG *np,const BN_ULONG *n0, int num);

	unsigned char cleanse_ctr = 0;

	void Empty_OPENSSL_cleanse(void *ptr, size_t len) {
	}

} // "C"

#if 0 //!!!D
void C_bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp, const BN_ULONG *np,const BN_ULONG *n0, int num) {
	BN_ULONG tbuf[14];
	BN_ULONG tbuf2[14];
	memset(tbuf, 0, sizeof tbuf);
	memset(tbuf2, 0, sizeof tbuf);
	BN_ULONG  *tp = &tbuf[1];
	BN_ULONG  *tp2 = &tbuf2[1];

	for (int i=0; i<num; ++i) {
		BN_ULONG a = ap[i];
		BN_ULONG mul2 = (a * bp[0] + tp[0]) * (*n0);

		uint32_t c1 = 0, c2 = 0;
		for (int j=0; j<num; ++j) {
			uint64_t sum1 = uint64_t(bp[j]) * a + tp[j] + c1;
			uint64_t sum2 = uint64_t(np[j]) * mul2 + c2;
			tp[j-1] = sum1+sum2;
			c1 = sum1 >> 32;
			c2 = (sum2 + uint32_t(sum1)) >> 32;
		}
		uint64_t c = uint64_t(c1) + c2 + tp[num];
		tp[num-1] = c;	
		tp[num] = c >> 32;
	}

	/*
	for (int i=0; i<num; ++i) {
		BN_ULONG a = ap[i];
		BN_ULONG tlow = a * bp[0] + tp[0];
		BN_ULONG mul2 = (tp2[0]-tlow) * (*n0);

		uint32_t c1 = 0, c2 = 0;
		for (int j=0; j<num; ++j) {
			uint64_t sum1 = uint64_t(bp[j]) * a + tp[j] + c1;
			tp[j-1] = (uint32_t)sum1;
			c1 = (uint32_t)(sum1 >> 32);

			uint64_t sum2 = uint64_t(np[j]) * mul2 + tp2[j] + c2;
			tp2[j-1] = (uint32_t)sum2;
			c2 = (uint32_t)(sum2 >> 32);
		}
		tp[num-1] = c1;
		tp2[num-1] = c2;
	}*/
//	ImpSubBignums(tp, tp2, rp, num+1);
//	ImpAddBignums(rp, np, rp, num);
	memcpy(rp, tp, num*4);
}

int My_bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp, const BN_ULONG *np,const BN_ULONG *n0, int num) {
	int num2 = num;
	BN_ULONG buf_OSL[100];
	BN_ULONG buf_SSE[100];
	BN_ULONG buf_C[100];

	bn_mul_mont(buf_OSL, ap, bp, np, n0, num2);
	C_bn_mul_mont(buf_C, ap, bp, np, n0, num2);
	((PFN_bn_mul_mont)&MontgomeryMul32_SSE)(buf_SSE, ap, bp, np, n0, num2);
	if (memcmp(buf_SSE, buf_C, 4 * num))
		ap = ap;
	if (memcmp(buf_C, buf_OSL, 4 * num))
		ap = ap;
	if (memcmp(buf_OSL, buf_SSE, 4 * num))
		ap = ap;
	return bn_mul_mont(rp, ap, bp, np, n0, num);
}
#endif

#ifdef _M_IX86
#	if UCFG_STDSTL
		PFN_bn_mul_mont g_pfn_bn_mul_mont = &bn_mul_mont;
#	else
		PFN_bn_mul_mont g_pfn_bn_mul_mont = g_bHasSse2 ? (PFN_bn_mul_mont)&MontgomeryMul32_SSE : &bn_mul_mont;
#	endif
#endif


