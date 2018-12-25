#ifndef SECP256K1_FIELD_REPR_IMPL_H
#define SECP256K1_FIELD_REPR_IMPL_H

#if defined HAVE_CONFIG_H
#include "libsecp256k1-config.h"
#endif

#include "util.h"
#include "num.h"
#include "field.h"

extern "C" {
	void __cdecl secp256k1_fe_4x64_add_inner(secp256k1_fe *r, const secp256k1_fe *a);
	void __cdecl secp256k1_fe_4x64_negate_inner(secp256k1_fe *r, const secp256k1_fe *a);
	void __cdecl secp256k1_fe_4x64_mul_int_inner(secp256k1_fe *r, int a);
	void __cdecl secp256k1_fe_4x64_mul_inner(uint64_t *r, const uint64_t *a, const uint64_t * SECP256K1_RESTRICT b);
	void __cdecl secp256k1_fe_4x64_sqr_inner(uint64_t *r, const uint64_t *a);

	void __cdecl secp256k1_fe_4x64_add_inner_bmi2(secp256k1_fe *r, const secp256k1_fe *a);
	void __cdecl secp256k1_fe_4x64_negate_inner_bmi2(secp256k1_fe *r, const secp256k1_fe *a);
	void __cdecl secp256k1_fe_4x64_mul_int_inner_bmi2(secp256k1_fe *r, int a);
	void __cdecl secp256k1_fe_4x64_mul_inner_bmi2(uint64_t *r, const uint64_t *a, const uint64_t * SECP256K1_RESTRICT b);
	void __cdecl secp256k1_fe_4x64_sqr_inner_bmi2(uint64_t *r, const uint64_t *a);
}

static bool s_bHasBMI2 = Ext::CpuInfo().Features.BMI2;

static void (__cdecl *s_pfn_4x64_add_inner)(secp256k1_fe *r, const secp256k1_fe *a)			= s_bHasBMI2 ? secp256k1_fe_4x64_add_inner_bmi2 : secp256k1_fe_4x64_add_inner;
static void (__cdecl *s_pfn_4x64_negate_inner)(secp256k1_fe *r, const secp256k1_fe *a)		= s_bHasBMI2 ? secp256k1_fe_4x64_negate_inner_bmi2 : secp256k1_fe_4x64_negate_inner;
static void (__cdecl *s_pfn_4x64_mul_int_inner)(secp256k1_fe *r, int a)						= s_bHasBMI2 ? secp256k1_fe_4x64_mul_int_inner_bmi2 : secp256k1_fe_4x64_mul_int_inner;
static void(__cdecl *s_pfn_4x64_sqr_inner)(uint64_t *r, const uint64_t *a)					= s_bHasBMI2 ? secp256k1_fe_4x64_sqr_inner_bmi2 : secp256k1_fe_4x64_sqr_inner;
static void (__cdecl *s_pfn_4x64_mul_inner)(uint64_t *r, const uint64_t *a, const uint64_t * SECP256K1_RESTRICT b) = s_bHasBMI2 ? secp256k1_fe_4x64_mul_inner_bmi2 : secp256k1_fe_4x64_mul_inner;


SECP256K1_INLINE void secp256k1_fe_normalize(secp256k1_fe *r) {
}

SECP256K1_INLINE void secp256k1_fe_normalize_var(secp256k1_fe *r) {
}

SECP256K1_INLINE void secp256k1_fe_normalize_weak(secp256k1_fe *r) {
}

SECP256K1_INLINE static void secp256k1_fe_set_int(secp256k1_fe *r, int a) {
    r->n[0] = a;
    r->n[1] = r->n[2] = r->n[3] = 0;
}

SECP256K1_INLINE int secp256k1_fe_is_zero(const secp256k1_fe *a) {
    const uint64_t *t = a->n;
    return (t[0] | t[1] | t[2] | t[3]) == 0;
}

SECP256K1_INLINE int secp256k1_fe_normalizes_to_zero(secp256k1_fe *r) {
	return !r->n[0] && !r->n[1] && !r->n[2] && !r->n[3];
}

SECP256K1_INLINE static int __cdecl secp256k1_fe_normalizes_to_zero_var(secp256k1_fe *r) {
	return !r->n[0] && !r->n[1] && !r->n[2] && !r->n[3];
}

SECP256K1_INLINE static int secp256k1_fe_is_odd(const secp256k1_fe *a) {
    return a->n[0] & 1;
}

SECP256K1_INLINE static void secp256k1_fe_clear(secp256k1_fe *r) {
	memset(r->n, 0, sizeof(r->n));
}

static SECP256K1_INLINE int secp256k1_fe_cmp_var(const secp256k1_fe *a, const secp256k1_fe *b) {
	return memcmp(a->n, b->n, sizeof(a->n));
}

static int secp256k1_fe_set_b32(secp256k1_fe *r, const unsigned char *a) {
	for (int i=0; i<sizeof(r->n); ++i)
		((uint8_t*)r->n)[i] = a[sizeof(r->n) - i - 1];
    return 1;
}

/** Convert a field element to a 32-byte big endian value. Requires the input to be normalized */
static void secp256k1_fe_get_b32(unsigned char *r, const secp256k1_fe *a) {
	for (int i=0; i<sizeof(a->n); ++i)
		r[sizeof(a->n) - i - 1] = ((uint8_t*)a->n)[i];
}

SECP256K1_INLINE static void secp256k1_fe_negate(secp256k1_fe *r, const secp256k1_fe *a, int m) {
	s_pfn_4x64_negate_inner(r, a);
}

SECP256K1_INLINE static void secp256k1_fe_mul_int(secp256k1_fe *r, int a) {
	s_pfn_4x64_mul_int_inner(r, a);
}

SECP256K1_INLINE static void secp256k1_fe_add(secp256k1_fe *r, const secp256k1_fe *a) {
	s_pfn_4x64_add_inner(r, a);
}

static SECP256K1_INLINE void secp256k1_fe_mul(secp256k1_fe *r, const secp256k1_fe *a, const secp256k1_fe * SECP256K1_RESTRICT b) {
	s_pfn_4x64_mul_inner(r->n, a->n, b->n);
}

static SECP256K1_INLINE void secp256k1_fe_sqr(secp256k1_fe *r, const secp256k1_fe *a) {
    s_pfn_4x64_sqr_inner(r->n, a->n);
}

static SECP256K1_INLINE void secp256k1_fe_cmov(secp256k1_fe *r, const secp256k1_fe *a, int flag) {
	if (flag)
		memcpy(r->n, a->n, sizeof(r->n));
}

static SECP256K1_INLINE void secp256k1_fe_storage_cmov(secp256k1_fe_storage *r, const secp256k1_fe_storage *a, int flag) {
	if (flag)
		memcpy(r->n, a->n, sizeof(r->n));
}

static SECP256K1_INLINE void secp256k1_fe_to_storage(secp256k1_fe_storage *r, const secp256k1_fe *a) {
	memcpy(r->n, a->n, sizeof(r->n));
}

static SECP256K1_INLINE void secp256k1_fe_from_storage(secp256k1_fe *r, const secp256k1_fe_storage *a) {
	memcpy(r->n, a->n, sizeof(r->n));
}

#endif /* SECP256K1_FIELD_REPR_IMPL_H */
