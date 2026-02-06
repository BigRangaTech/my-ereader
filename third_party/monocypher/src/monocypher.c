// Monocypher version __git__
//
// This file is dual-licensed.  Choose whichever licence you want from
// the two licences listed below.
//
// The first licence is a regular 2-clause BSD licence.  The second licence
// is the CC-0 from Creative Commons. It is intended to release Monocypher
// to the public domain.  The BSD licence serves as a fallback option.
//
// SPDX-License-Identifier: BSD-2-Clause OR CC0-1.0
//
// ------------------------------------------------------------------------
//
// Copyright (c) 2017-2020, Loup Vaillant
// All rights reserved.
//
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ------------------------------------------------------------------------
//
// Written in 2017-2020 by Loup Vaillant
//
// To the extent possible under law, the author(s) have dedicated all copyright
// and related neighboring rights to this software to the public domain
// worldwide.  This software is distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along
// with this software.  If not, see
// <https://creativecommons.org/publicdomain/zero/1.0/>

#include "monocypher.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#if ((defined(MONOCYPHER_ARGON2_PTHREADS) && MONOCYPHER_ARGON2_PTHREADS) || \
     (defined(MONOCYPHER_BLAKE3_PTHREADS) && MONOCYPHER_BLAKE3_PTHREADS) || \
     (defined(MONOCYPHER_CHACHA20_PTHREADS) && MONOCYPHER_CHACHA20_PTHREADS)) \
    && !defined(_WIN32)
#include <pthread.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/syscall.h>
#endif
#endif
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif
#include <immintrin.h>
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
#include <arm_neon.h>
#endif
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define MONO_LITTLE_ENDIAN 1
#else
#define MONO_LITTLE_ENDIAN 0
#endif
#if defined(__GLIBC__) || defined(__OpenBSD__) || defined(__FreeBSD__) || \
    defined(__NetBSD__) || defined(__APPLE__)
extern void explicit_bzero(void *s, size_t n);
#define HAVE_EXPLICIT_BZERO 1
#endif
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
    defined(__NetBSD__) || defined(__DragonFly__) || defined(__illumos__)
#define MONOCYPHER_HAS_ARC4RANDOM 1
#endif
#if defined(__linux__) && defined(SYS_getrandom)
#define MONOCYPHER_HAS_GETRANDOM 1
extern long syscall(long number, ...);
#endif

#ifdef MONOCYPHER_CPP_NAMESPACE
namespace MONOCYPHER_CPP_NAMESPACE {
#endif

/////////////////
/// Utilities ///
/////////////////
#define FOR_T(type, i, start, end) for (type i = (start); i < (end); i++)
#define FOR(i, start, end)         FOR_T(size_t, i, start, end)
#define COPY(dst, src, size)       FOR(_i_, 0, size) (dst)[_i_] = (src)[_i_]
#define ZERO(buf, size)            FOR(_i_, 0, size) (buf)[_i_] = 0
#define WIPE_CTX(ctx)              crypto_wipe(ctx   , sizeof(*(ctx)))
#define WIPE_BUFFER(buffer)        crypto_wipe(buffer, sizeof(buffer))
#define MIN(a, b)                  ((a) <= (b) ? (a) : (b))
#define MAX(a, b)                  ((a) >= (b) ? (a) : (b))

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint64_t u64;

static const u8 zero[128] = {0};

// returns the smallest positive integer y such that
// (x + y) % pow_2  == 0
// Basically, y is the "gap" missing to align x.
// Only works when pow_2 is a power of 2.
// Note: we use ~x+1 instead of -x to avoid compiler warnings
static size_t gap(size_t x, size_t pow_2)
{
	return (~x + 1) & (pow_2 - 1);
}

static u32 load24_le(const u8 s[3])
{
	return
		((u32)s[0] <<  0) |
		((u32)s[1] <<  8) |
		((u32)s[2] << 16);
}

static u32 load32_le(const u8 s[4])
{
	return
		((u32)s[0] <<  0) |
		((u32)s[1] <<  8) |
		((u32)s[2] << 16) |
		((u32)s[3] << 24);
}

static u64 load64_le(const u8 s[8])
{
	return load32_le(s) | ((u64)load32_le(s+4) << 32);
}

static u32 load32_be(const u8 s[4])
{
	return ((u32)s[0] << 24)
		| ((u32)s[1] << 16)
		| ((u32)s[2] <<  8)
		| ((u32)s[3] <<  0);
}

static void store32_le(u8 out[4], u32 in)
{
	out[0] =  in        & 0xff;
	out[1] = (in >>  8) & 0xff;
	out[2] = (in >> 16) & 0xff;
	out[3] = (in >> 24) & 0xff;
}

static void store32_be(u8 out[4], u32 in)
{
	out[0] = (in >> 24) & 0xff;
	out[1] = (in >> 16) & 0xff;
	out[2] = (in >>  8) & 0xff;
	out[3] =  in        & 0xff;
}

static void store64_le(u8 out[8], u64 in)
{
	store32_le(out    , (u32)in );
	store32_le(out + 4, in >> 32);
}

static void store64_be(u8 out[8], u64 in)
{
	out[0] = (in >> 56) & 0xff;
	out[1] = (in >> 48) & 0xff;
	out[2] = (in >> 40) & 0xff;
	out[3] = (in >> 32) & 0xff;
	out[4] = (in >> 24) & 0xff;
	out[5] = (in >> 16) & 0xff;
	out[6] = (in >>  8) & 0xff;
	out[7] =  in        & 0xff;
}

static void load32_le_buf (u32 *dst, const u8 *src, size_t size) {
	FOR(i, 0, size) { dst[i] = load32_le(src + i*4); }
}
static void load64_le_buf (u64 *dst, const u8 *src, size_t size) {
	FOR(i, 0, size) { dst[i] = load64_le(src + i*8); }
}
static void store32_le_buf(u8 *dst, const u32 *src, size_t size) {
	FOR(i, 0, size) { store32_le(dst + i*4, src[i]); }
}
static void store64_le_buf(u8 *dst, const u64 *src, size_t size) {
	FOR(i, 0, size) { store64_le(dst + i*8, src[i]); }
}
static void load32_be_buf (u32 *dst, const u8 *src, size_t size) {
	FOR(i, 0, size) { dst[i] = load32_be(src + i*4); }
}
static void store32_be_buf(u8 *dst, const u32 *src, size_t size) {
	FOR(i, 0, size) { store32_be(dst + i*4, src[i]); }
}

static u64 rotr64(u64 x, u64 n) { return (x >> n) ^ (x << (64 - n)); }
static u32 rotl32(u32 x, u32 n) { return (x << n) ^ (x >> (32 - n)); }
static u32 rotr32(u32 x, u32 n) { return (x >> n) ^ (x << (32 - n)); }

static int check_ptr(const void *ptr, size_t size)
{
	return (size != 0 && ptr == 0) ? CRYPTO_ERR_NULL : CRYPTO_OK;
}

static int check_out_ptr(const void *ptr)
{
	return ptr == 0 ? CRYPTO_ERR_NULL : CRYPTO_OK;
}

static int checked_add_u64(u64 a, u64 b, u64 *out)
{
	if (a > UINT64_MAX - b) {
		return CRYPTO_ERR_OVERFLOW;
	}
	*out = a + b;
	return CRYPTO_OK;
}

static int checked_add_u32(u32 a, u32 b, u32 *out)
{
	if (a > UINT32_MAX - b) {
		return CRYPTO_ERR_OVERFLOW;
	}
	*out = a + b;
	return CRYPTO_OK;
}

static int checked_mul_size(size_t a, size_t b, size_t *out)
{
	if (a != 0 && b > SIZE_MAX / a) {
		return CRYPTO_ERR_OVERFLOW;
	}
	*out = a * b;
	return CRYPTO_OK;
}

#if defined(MONOCYPHER_STRERROR) && MONOCYPHER_STRERROR
const char *crypto_strerror(int err)
{
	switch (err) {
	case CRYPTO_OK:           return "CRYPTO_OK";
	case CRYPTO_ERR_NULL:     return "CRYPTO_ERR_NULL";
	case CRYPTO_ERR_SIZE:     return "CRYPTO_ERR_SIZE";
	case CRYPTO_ERR_OVERFLOW: return "CRYPTO_ERR_OVERFLOW";
	case CRYPTO_ERR_AUTH:     return "CRYPTO_ERR_AUTH";
	case CRYPTO_ERR_CONFIG:   return "CRYPTO_ERR_CONFIG";
	default:                  return "CRYPTO_ERR_UNKNOWN";
	}
}
#endif

static int neq0(u64 diff)
{
	// constant time comparison to zero
	// return diff != 0 ? -1 : 0
	u64 half = (diff >> 32) | ((u32)diff);
	return (1 & ((half - 1) >> 32)) - 1;
}

#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
MONO_TARGET_SSE2
static u64 x16_sse2(const u8 a[16], const u8 b[16])
{
	__m128i va = _mm_loadu_si128((const __m128i*)a);
	__m128i vb = _mm_loadu_si128((const __m128i*)b);
	__m128i v = _mm_xor_si128(va, vb);
	v = _mm_or_si128(v, _mm_srli_si128(v, 8));
	return (u64)_mm_cvtsi128_si64(v);
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static u64 x16_neon(const u8 a[16], const u8 b[16])
{
	uint8x16_t va = vld1q_u8(a);
	uint8x16_t vb = vld1q_u8(b);
	uint8x16_t v = veorq_u8(va, vb);
	uint64x2_t lanes = vreinterpretq_u64_u8(v);
	return vgetq_lane_u64(lanes, 0) | vgetq_lane_u64(lanes, 1);
}
#endif

static u64 x16(const u8 a[16], const u8 b[16])
{
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) { return x16_sse2(a, b); }
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) { return x16_neon(a, b); }
#endif
	return (load64_le(a + 0) ^ load64_le(b + 0))
		|  (load64_le(a + 8) ^ load64_le(b + 8));
}
static u64 x32(const u8 a[32],const u8 b[32]){return x16(a,b)| x16(a+16, b+16);}
static u64 x64(const u8 a[64],const u8 b[64]){return x32(a,b)| x32(a+32, b+32);}
int crypto_verify16(const u8 a[16], const u8 b[16]){ return neq0(x16(a, b)); }
int crypto_verify32(const u8 a[32], const u8 b[32]){ return neq0(x32(a, b)); }
int crypto_verify64(const u8 a[64], const u8 b[64]){ return neq0(x64(a, b)); }
int crypto_verify16_checked(const u8 a[16], const u8 b[16])
{
	int err = check_ptr(a, 16);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(b, 16);
	if (err != CRYPTO_OK) { return err; }
	return crypto_verify16(a, b) ? CRYPTO_ERR_AUTH : CRYPTO_OK;
}

int crypto_verify32_checked(const u8 a[32], const u8 b[32])
{
	int err = check_ptr(a, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(b, 32);
	if (err != CRYPTO_OK) { return err; }
	return crypto_verify32(a, b) ? CRYPTO_ERR_AUTH : CRYPTO_OK;
}

int crypto_verify64_checked(const u8 a[64], const u8 b[64])
{
	int err = check_ptr(a, 64);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(b, 64);
	if (err != CRYPTO_OK) { return err; }
	return crypto_verify64(a, b) ? CRYPTO_ERR_AUTH : CRYPTO_OK;
}

void crypto_wipe(void *secret, size_t size)
{
	if (size == 0 || secret == 0) {
		return;
	}
#if defined(__STDC_LIB_EXT1__)
	memset_s(secret, size, 0, size);
#elif defined(HAVE_EXPLICIT_BZERO)
	explicit_bzero(secret, size);
#else
	volatile u8 *v_secret = (u8*)secret;
	ZERO(v_secret, size);
#endif
}

#if defined(MONOCYPHER_RNG_DIAGNOSTICS) && MONOCYPHER_RNG_DIAGNOSTICS
static int crypto_rng_last_error = 0;

int crypto_random_last_error(void)
{
	return crypto_rng_last_error;
}

static void crypto_rng_set_error(int err)
{
	crypto_rng_last_error = err;
}

static void crypto_rng_clear_error(void)
{
	crypto_rng_last_error = 0;
}
#else
static void crypto_rng_set_error(int err) { (void)err; }
static void crypto_rng_clear_error(void) { }
#endif

static int crypto_random_urandom(u8 *out, size_t out_size)
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		crypto_rng_set_error(errno);
		return CRYPTO_ERR_CONFIG;
	}
	size_t remaining = out_size;
	u8 *p = out;
	while (remaining > 0) {
		ssize_t n = read(fd, p, remaining);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			crypto_rng_set_error(errno);
			close(fd);
			return CRYPTO_ERR_CONFIG;
		}
		if (n == 0) {
			crypto_rng_set_error(EIO);
			close(fd);
			return CRYPTO_ERR_CONFIG;
		}
		p += (size_t)n;
		remaining -= (size_t)n;
	}
	close(fd);
	return CRYPTO_OK;
}

int crypto_random(u8 *out, size_t out_size)
{
	crypto_rng_clear_error();
	if (out_size == 0) {
		return CRYPTO_OK;
	}
	if (out == 0) {
		return CRYPTO_ERR_NULL;
	}
#ifdef _WIN32
	if (out_size > ULONG_MAX) {
		return CRYPTO_ERR_SIZE;
	}
	NTSTATUS status = BCryptGenRandom(NULL, out, (ULONG)out_size,
	                                  BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	if (status != 0) {
		crypto_rng_set_error((int)status);
		return CRYPTO_ERR_CONFIG;
	}
	return CRYPTO_OK;
#else
#if defined(MONOCYPHER_HAS_ARC4RANDOM)
	arc4random_buf(out, out_size);
	return CRYPTO_OK;
#elif defined(MONOCYPHER_HAS_GETRANDOM)
	size_t remaining = out_size;
	u8 *p = out;
	while (remaining > 0) {
		ssize_t n = (ssize_t)syscall(SYS_getrandom, p, remaining, 0);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			if (errno == ENOSYS) {
				return crypto_random_urandom(out, out_size);
			}
			crypto_rng_set_error(errno);
			return CRYPTO_ERR_CONFIG;
		}
		if (n == 0) {
			crypto_rng_set_error(EIO);
			return CRYPTO_ERR_CONFIG;
		}
		p += (size_t)n;
		remaining -= (size_t)n;
	}
	return CRYPTO_OK;
#else
	return crypto_random_urandom(out, out_size);
#endif
#endif
}


/////////////////
/// Chacha 20 ///
/////////////////
#define QUARTERROUND(a, b, c, d)	\
	a += b;  d = rotl32(d ^ a, 16); \
	c += d;  b = rotl32(b ^ c, 12); \
	a += b;  d = rotl32(d ^ a,  8); \
	c += d;  b = rotl32(b ^ c,  7)

static void chacha20_rounds(u32 out[16], const u32 in[16])
{
	// The temporary variables make Chacha20 10% faster.
	u32 t0  = in[ 0];  u32 t1  = in[ 1];  u32 t2  = in[ 2];  u32 t3  = in[ 3];
	u32 t4  = in[ 4];  u32 t5  = in[ 5];  u32 t6  = in[ 6];  u32 t7  = in[ 7];
	u32 t8  = in[ 8];  u32 t9  = in[ 9];  u32 t10 = in[10];  u32 t11 = in[11];
	u32 t12 = in[12];  u32 t13 = in[13];  u32 t14 = in[14];  u32 t15 = in[15];

	FOR (i, 0, 10) { // 20 rounds, 2 rounds per loop.
		QUARTERROUND(t0, t4, t8 , t12); // column 0
		QUARTERROUND(t1, t5, t9 , t13); // column 1
		QUARTERROUND(t2, t6, t10, t14); // column 2
		QUARTERROUND(t3, t7, t11, t15); // column 3
		QUARTERROUND(t0, t5, t10, t15); // diagonal 0
		QUARTERROUND(t1, t6, t11, t12); // diagonal 1
		QUARTERROUND(t2, t7, t8 , t13); // diagonal 2
		QUARTERROUND(t3, t4, t9 , t14); // diagonal 3
	}
	out[ 0] = t0;   out[ 1] = t1;   out[ 2] = t2;   out[ 3] = t3;
	out[ 4] = t4;   out[ 5] = t5;   out[ 6] = t6;   out[ 7] = t7;
	out[ 8] = t8;   out[ 9] = t9;   out[10] = t10;  out[11] = t11;
	out[12] = t12;  out[13] = t13;  out[14] = t14;  out[15] = t15;
}

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#if defined(__GNUC__) || defined(__clang__)
#define MONO_TARGET_SSE2 __attribute__((target("sse2")))
#define MONO_TARGET_SSE41 __attribute__((target("sse4.1")))
#define MONO_TARGET_AVX2 __attribute__((target("avx2")))
#define MONO_HAS_AVX2 1
#define MONO_HAS_SSE41 1
#else
#define MONO_TARGET_SSE2
#define MONO_TARGET_SSE41
#if defined(_MSC_VER) && defined(__AVX2__)
#define MONO_TARGET_AVX2
#define MONO_HAS_AVX2 1
#else
#define MONO_TARGET_AVX2
#define MONO_HAS_AVX2 0
#endif
#if defined(_MSC_VER) && defined(__SSE4_1__)
#define MONO_HAS_SSE41 1
#else
#define MONO_HAS_SSE41 0
#endif
#endif
#if defined(__x86_64__) || defined(_M_X64)
#define MONO_HAS_SSE2 1
#elif defined(__i386__) && defined(__SSE2__)
#define MONO_HAS_SSE2 1
#elif defined(_M_IX86) && defined(_M_IX86_FP) && _M_IX86_FP >= 2
#define MONO_HAS_SSE2 1
#else
#define MONO_HAS_SSE2 0
#endif

static int chacha20_cpu_has_sse2(void)
{
#if defined(__x86_64__) || defined(_M_X64)
	return 1;
#elif defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
	unsigned eax = 0, ebx = 0, ecx = 0, edx = 0;
	if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
		return 0;
	}
	return (edx & bit_SSE2) != 0;
#elif defined(_M_IX86) && defined(_MSC_VER)
	int regs[4] = {0,0,0,0};
	__cpuid(regs, 1);
	return (regs[3] & (1 << 26)) != 0;
#else
	return 0;
#endif
}

static int mono_have_sse2_cached(void)
{
	static int cached = -1;
	if (cached < 0) {
		cached = chacha20_cpu_has_sse2();
	}
	return cached;
}

#if MONO_HAS_SSE41
static int mono_cpu_has_sse41(void)
{
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_cpu_supports("sse4.1");
#elif defined(_MSC_VER)
	int regs[4] = {0,0,0,0};
	__cpuid(regs, 1);
	return (regs[2] & (1 << 19)) != 0;
#else
	return 0;
#endif
}

static int mono_have_sse41_cached(void)
{
	static int cached = -1;
	if (cached < 0) {
		cached = mono_cpu_has_sse41();
	}
	return cached;
}
#endif

#if MONO_HAS_AVX2
static int mono_cpu_has_avx2(void)
{
#if defined(__GNUC__) || defined(__clang__)
	return __builtin_cpu_supports("avx2");
#elif defined(_MSC_VER)
	int regs[4] = {0,0,0,0};
	__cpuid(regs, 1);
	const int osxsave = (regs[2] & (1 << 27)) != 0;
	const int avx = (regs[2] & (1 << 28)) != 0;
	if (!(osxsave && avx)) {
		return 0;
	}
	unsigned long long xcr0 = _xgetbv(0);
	if ((xcr0 & 0x6) != 0x6) {
		return 0;
	}
	__cpuidex(regs, 7, 0);
	return (regs[1] & (1 << 5)) != 0;
#else
	return 0;
#endif
}

static int mono_have_avx2_cached(void)
{
	static int cached = -1;
	if (cached < 0) {
		cached = mono_cpu_has_avx2();
	}
	return cached;
}
#endif

MONO_TARGET_SSE2
static __m128i rotl32_sse2(__m128i v, int n)
{
	return _mm_or_si128(_mm_slli_epi32(v, n), _mm_srli_epi32(v, 32 - n));
}

MONO_TARGET_SSE2
static void chacha20_blocks4_sse2(u8 *cipher_text, const u8 *plain_text,
                                  const u32 input[16], u64 ctr)
{
	const u32 c0 = (u32)ctr;
	const u32 c1 = (u32)(ctr + 1);
	const u32 c2 = (u32)(ctr + 2);
	const u32 c3 = (u32)(ctr + 3);
	const u32 h0 = (u32)(ctr >> 32);
	const u32 h1 = (u32)((ctr + 1) >> 32);
	const u32 h2 = (u32)((ctr + 2) >> 32);
	const u32 h3 = (u32)((ctr + 3) >> 32);

	__m128i v0  = _mm_set1_epi32((i32)input[0]);
	__m128i v1  = _mm_set1_epi32((i32)input[1]);
	__m128i v2  = _mm_set1_epi32((i32)input[2]);
	__m128i v3  = _mm_set1_epi32((i32)input[3]);
	__m128i v4  = _mm_set1_epi32((i32)input[4]);
	__m128i v5  = _mm_set1_epi32((i32)input[5]);
	__m128i v6  = _mm_set1_epi32((i32)input[6]);
	__m128i v7  = _mm_set1_epi32((i32)input[7]);
	__m128i v8  = _mm_set1_epi32((i32)input[8]);
	__m128i v9  = _mm_set1_epi32((i32)input[9]);
	__m128i v10 = _mm_set1_epi32((i32)input[10]);
	__m128i v11 = _mm_set1_epi32((i32)input[11]);
	__m128i v12 = _mm_set_epi32((i32)c3, (i32)c2, (i32)c1, (i32)c0);
	__m128i v13 = _mm_set_epi32((i32)h3, (i32)h2, (i32)h1, (i32)h0);
	__m128i v14 = _mm_set1_epi32((i32)input[14]);
	__m128i v15 = _mm_set1_epi32((i32)input[15]);

	const __m128i i0  = v0;  const __m128i i1  = v1;
	const __m128i i2  = v2;  const __m128i i3  = v3;
	const __m128i i4  = v4;  const __m128i i5  = v5;
	const __m128i i6  = v6;  const __m128i i7  = v7;
	const __m128i i8  = v8;  const __m128i i9  = v9;
	const __m128i i10 = v10; const __m128i i11 = v11;
	const __m128i i12 = v12; const __m128i i13 = v13;
	const __m128i i14 = v14; const __m128i i15 = v15;

#define QR(a,b,c,d)                            \
	a = _mm_add_epi32(a, b);                  \
	d = _mm_xor_si128(d, a);                  \
	d = rotl32_sse2(d, 16);                   \
	c = _mm_add_epi32(c, d);                  \
	b = _mm_xor_si128(b, c);                  \
	b = rotl32_sse2(b, 12);                   \
	a = _mm_add_epi32(a, b);                  \
	d = _mm_xor_si128(d, a);                  \
	d = rotl32_sse2(d,  8);                   \
	c = _mm_add_epi32(c, d);                  \
	b = _mm_xor_si128(b, c);                  \
	b = rotl32_sse2(b,  7)

	for (int i = 0; i < 10; i++) {
		QR(v0, v4, v8 , v12);
		QR(v1, v5, v9 , v13);
		QR(v2, v6, v10, v14);
		QR(v3, v7, v11, v15);
		QR(v0, v5, v10, v15);
		QR(v1, v6, v11, v12);
		QR(v2, v7, v8 , v13);
		QR(v3, v4, v9 , v14);
	}

	v0  = _mm_add_epi32(v0 , i0 );
	v1  = _mm_add_epi32(v1 , i1 );
	v2  = _mm_add_epi32(v2 , i2 );
	v3  = _mm_add_epi32(v3 , i3 );
	v4  = _mm_add_epi32(v4 , i4 );
	v5  = _mm_add_epi32(v5 , i5 );
	v6  = _mm_add_epi32(v6 , i6 );
	v7  = _mm_add_epi32(v7 , i7 );
	v8  = _mm_add_epi32(v8 , i8 );
	v9  = _mm_add_epi32(v9 , i9 );
	v10 = _mm_add_epi32(v10, i10);
	v11 = _mm_add_epi32(v11, i11);
	v12 = _mm_add_epi32(v12, i12);
	v13 = _mm_add_epi32(v13, i13);
	v14 = _mm_add_epi32(v14, i14);
	v15 = _mm_add_epi32(v15, i15);

	u32 tmp[16][4];
	_mm_storeu_si128((__m128i*)tmp[0] , v0 );
	_mm_storeu_si128((__m128i*)tmp[1] , v1 );
	_mm_storeu_si128((__m128i*)tmp[2] , v2 );
	_mm_storeu_si128((__m128i*)tmp[3] , v3 );
	_mm_storeu_si128((__m128i*)tmp[4] , v4 );
	_mm_storeu_si128((__m128i*)tmp[5] , v5 );
	_mm_storeu_si128((__m128i*)tmp[6] , v6 );
	_mm_storeu_si128((__m128i*)tmp[7] , v7 );
	_mm_storeu_si128((__m128i*)tmp[8] , v8 );
	_mm_storeu_si128((__m128i*)tmp[9] , v9 );
	_mm_storeu_si128((__m128i*)tmp[10], v10);
	_mm_storeu_si128((__m128i*)tmp[11], v11);
	_mm_storeu_si128((__m128i*)tmp[12], v12);
	_mm_storeu_si128((__m128i*)tmp[13], v13);
	_mm_storeu_si128((__m128i*)tmp[14], v14);
	_mm_storeu_si128((__m128i*)tmp[15], v15);

	for (int b = 0; b < 4; b++) {
		u8 *out = cipher_text + (size_t)b * 64;
		const u8 *in = plain_text ? (plain_text + (size_t)b * 64) : 0;
		for (int i = 0; i < 16; i++) {
			u32 w = tmp[i][b];
			if (in) {
				w ^= load32_le(in + (size_t)i * 4);
			}
			store32_le(out + (size_t)i * 4, w);
		}
	}
#undef QR
}

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static __m256i rotl32_avx2(__m256i v, int n)
{
	return _mm256_or_si256(_mm256_slli_epi32(v, n), _mm256_srli_epi32(v, 32 - n));
}

MONO_TARGET_AVX2
static void chacha20_blocks8_avx2(u8 *cipher_text, const u8 *plain_text,
                                  const u32 input[16], u64 ctr)
{
	u32 ctr_lo[8] = {
		(u32)ctr, (u32)(ctr + 1), (u32)(ctr + 2), (u32)(ctr + 3),
		(u32)(ctr + 4), (u32)(ctr + 5), (u32)(ctr + 6), (u32)(ctr + 7),
	};
	u32 ctr_hi[8] = {
		(u32)(ctr >> 32), (u32)((ctr + 1) >> 32), (u32)((ctr + 2) >> 32),
		(u32)((ctr + 3) >> 32), (u32)((ctr + 4) >> 32),
		(u32)((ctr + 5) >> 32), (u32)((ctr + 6) >> 32),
		(u32)((ctr + 7) >> 32),
	};

	__m256i v0  = _mm256_set1_epi32((i32)input[0]);
	__m256i v1  = _mm256_set1_epi32((i32)input[1]);
	__m256i v2  = _mm256_set1_epi32((i32)input[2]);
	__m256i v3  = _mm256_set1_epi32((i32)input[3]);
	__m256i v4  = _mm256_set1_epi32((i32)input[4]);
	__m256i v5  = _mm256_set1_epi32((i32)input[5]);
	__m256i v6  = _mm256_set1_epi32((i32)input[6]);
	__m256i v7  = _mm256_set1_epi32((i32)input[7]);
	__m256i v8  = _mm256_set1_epi32((i32)input[8]);
	__m256i v9  = _mm256_set1_epi32((i32)input[9]);
	__m256i v10 = _mm256_set1_epi32((i32)input[10]);
	__m256i v11 = _mm256_set1_epi32((i32)input[11]);
	__m256i v12 = _mm256_loadu_si256((const __m256i*)ctr_lo);
	__m256i v13 = _mm256_loadu_si256((const __m256i*)ctr_hi);
	__m256i v14 = _mm256_set1_epi32((i32)input[14]);
	__m256i v15 = _mm256_set1_epi32((i32)input[15]);

	const __m256i i0  = v0;  const __m256i i1  = v1;
	const __m256i i2  = v2;  const __m256i i3  = v3;
	const __m256i i4  = v4;  const __m256i i5  = v5;
	const __m256i i6  = v6;  const __m256i i7  = v7;
	const __m256i i8  = v8;  const __m256i i9  = v9;
	const __m256i i10 = v10; const __m256i i11 = v11;
	const __m256i i12 = v12; const __m256i i13 = v13;
	const __m256i i14 = v14; const __m256i i15 = v15;

#define QR(a,b,c,d)                            \
	a = _mm256_add_epi32(a, b);               \
	d = _mm256_xor_si256(d, a);               \
	d = rotl32_avx2(d, 16);                   \
	c = _mm256_add_epi32(c, d);               \
	b = _mm256_xor_si256(b, c);               \
	b = rotl32_avx2(b, 12);                   \
	a = _mm256_add_epi32(a, b);               \
	d = _mm256_xor_si256(d, a);               \
	d = rotl32_avx2(d,  8);                   \
	c = _mm256_add_epi32(c, d);               \
	b = _mm256_xor_si256(b, c);               \
	b = rotl32_avx2(b,  7)

	for (int i = 0; i < 10; i++) {
		QR(v0, v4, v8 , v12);
		QR(v1, v5, v9 , v13);
		QR(v2, v6, v10, v14);
		QR(v3, v7, v11, v15);
		QR(v0, v5, v10, v15);
		QR(v1, v6, v11, v12);
		QR(v2, v7, v8 , v13);
		QR(v3, v4, v9 , v14);
	}

	v0  = _mm256_add_epi32(v0 , i0 );
	v1  = _mm256_add_epi32(v1 , i1 );
	v2  = _mm256_add_epi32(v2 , i2 );
	v3  = _mm256_add_epi32(v3 , i3 );
	v4  = _mm256_add_epi32(v4 , i4 );
	v5  = _mm256_add_epi32(v5 , i5 );
	v6  = _mm256_add_epi32(v6 , i6 );
	v7  = _mm256_add_epi32(v7 , i7 );
	v8  = _mm256_add_epi32(v8 , i8 );
	v9  = _mm256_add_epi32(v9 , i9 );
	v10 = _mm256_add_epi32(v10, i10);
	v11 = _mm256_add_epi32(v11, i11);
	v12 = _mm256_add_epi32(v12, i12);
	v13 = _mm256_add_epi32(v13, i13);
	v14 = _mm256_add_epi32(v14, i14);
	v15 = _mm256_add_epi32(v15, i15);

	u32 tmp[16][8];
	_mm256_storeu_si256((__m256i*)tmp[0], v0);
	_mm256_storeu_si256((__m256i*)tmp[1], v1);
	_mm256_storeu_si256((__m256i*)tmp[2], v2);
	_mm256_storeu_si256((__m256i*)tmp[3], v3);
	_mm256_storeu_si256((__m256i*)tmp[4], v4);
	_mm256_storeu_si256((__m256i*)tmp[5], v5);
	_mm256_storeu_si256((__m256i*)tmp[6], v6);
	_mm256_storeu_si256((__m256i*)tmp[7], v7);
	_mm256_storeu_si256((__m256i*)tmp[8], v8);
	_mm256_storeu_si256((__m256i*)tmp[9], v9);
	_mm256_storeu_si256((__m256i*)tmp[10], v10);
	_mm256_storeu_si256((__m256i*)tmp[11], v11);
	_mm256_storeu_si256((__m256i*)tmp[12], v12);
	_mm256_storeu_si256((__m256i*)tmp[13], v13);
	_mm256_storeu_si256((__m256i*)tmp[14], v14);
	_mm256_storeu_si256((__m256i*)tmp[15], v15);

	for (int b = 0; b < 8; b++) {
		u8 *out = cipher_text + (size_t)b * 64;
		const u8 *in = plain_text ? (plain_text + (size_t)b * 64) : 0;
		for (int i = 0; i < 16; i++) {
			u32 w = tmp[i][b];
			if (in) {
				w ^= load32_le(in + (size_t)i * 4);
			}
			store32_le(out + (size_t)i * 4, w);
		}
	}
#undef QR
}
#endif
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static inline uint32x4_t rotl32_neon(uint32x4_t v, int n)
{
	return vorrq_u32(vshlq_n_u32(v, n), vshrq_n_u32(v, 32 - n));
}

static int chacha20_cpu_has_neon(void)
{
#if defined(__aarch64__)
	return 1;
#elif defined(__ARM_NEON) && !defined(MONOCYPHER_DISABLE_NEON)
	return 1;
#else
	return 0;
#endif
}

static int mono_have_neon_cached(void)
{
	static int cached = -1;
	if (cached < 0) {
		cached = chacha20_cpu_has_neon();
	}
	return cached;
}

static void chacha20_blocks4_neon(u8 *cipher_text, const u8 *plain_text,
                                  const u32 input[16], u64 ctr)
{
	u32 ctr_lo[4] = {(u32)ctr, (u32)(ctr + 1), (u32)(ctr + 2), (u32)(ctr + 3)};
	u32 ctr_hi[4] = {(u32)(ctr >> 32), (u32)((ctr + 1) >> 32),
	                 (u32)((ctr + 2) >> 32), (u32)((ctr + 3) >> 32)};
	uint32x4_t v0  = vdupq_n_u32(input[0]);
	uint32x4_t v1  = vdupq_n_u32(input[1]);
	uint32x4_t v2  = vdupq_n_u32(input[2]);
	uint32x4_t v3  = vdupq_n_u32(input[3]);
	uint32x4_t v4  = vdupq_n_u32(input[4]);
	uint32x4_t v5  = vdupq_n_u32(input[5]);
	uint32x4_t v6  = vdupq_n_u32(input[6]);
	uint32x4_t v7  = vdupq_n_u32(input[7]);
	uint32x4_t v8  = vdupq_n_u32(input[8]);
	uint32x4_t v9  = vdupq_n_u32(input[9]);
	uint32x4_t v10 = vdupq_n_u32(input[10]);
	uint32x4_t v11 = vdupq_n_u32(input[11]);
	uint32x4_t v12 = vld1q_u32(ctr_lo);
	uint32x4_t v13 = vld1q_u32(ctr_hi);
	uint32x4_t v14 = vdupq_n_u32(input[14]);
	uint32x4_t v15 = vdupq_n_u32(input[15]);

	const uint32x4_t i0  = v0;  const uint32x4_t i1  = v1;
	const uint32x4_t i2  = v2;  const uint32x4_t i3  = v3;
	const uint32x4_t i4  = v4;  const uint32x4_t i5  = v5;
	const uint32x4_t i6  = v6;  const uint32x4_t i7  = v7;
	const uint32x4_t i8  = v8;  const uint32x4_t i9  = v9;
	const uint32x4_t i10 = v10; const uint32x4_t i11 = v11;
	const uint32x4_t i12 = v12; const uint32x4_t i13 = v13;
	const uint32x4_t i14 = v14; const uint32x4_t i15 = v15;

#define QR(a,b,c,d)                 \
	a = vaddq_u32(a, b);            \
	d = veorq_u32(d, a);            \
	d = rotl32_neon(d, 16);         \
	c = vaddq_u32(c, d);            \
	b = veorq_u32(b, c);            \
	b = rotl32_neon(b, 12);         \
	a = vaddq_u32(a, b);            \
	d = veorq_u32(d, a);            \
	d = rotl32_neon(d,  8);         \
	c = vaddq_u32(c, d);            \
	b = veorq_u32(b, c);            \
	b = rotl32_neon(b,  7)

	for (int i = 0; i < 10; i++) {
		QR(v0, v4, v8 , v12);
		QR(v1, v5, v9 , v13);
		QR(v2, v6, v10, v14);
		QR(v3, v7, v11, v15);
		QR(v0, v5, v10, v15);
		QR(v1, v6, v11, v12);
		QR(v2, v7, v8 , v13);
		QR(v3, v4, v9 , v14);
	}

	v0  = vaddq_u32(v0 , i0 );
	v1  = vaddq_u32(v1 , i1 );
	v2  = vaddq_u32(v2 , i2 );
	v3  = vaddq_u32(v3 , i3 );
	v4  = vaddq_u32(v4 , i4 );
	v5  = vaddq_u32(v5 , i5 );
	v6  = vaddq_u32(v6 , i6 );
	v7  = vaddq_u32(v7 , i7 );
	v8  = vaddq_u32(v8 , i8 );
	v9  = vaddq_u32(v9 , i9 );
	v10 = vaddq_u32(v10, i10);
	v11 = vaddq_u32(v11, i11);
	v12 = vaddq_u32(v12, i12);
	v13 = vaddq_u32(v13, i13);
	v14 = vaddq_u32(v14, i14);
	v15 = vaddq_u32(v15, i15);

	u32 tmp[16][4];
	vst1q_u32(tmp[0] , v0 );
	vst1q_u32(tmp[1] , v1 );
	vst1q_u32(tmp[2] , v2 );
	vst1q_u32(tmp[3] , v3 );
	vst1q_u32(tmp[4] , v4 );
	vst1q_u32(tmp[5] , v5 );
	vst1q_u32(tmp[6] , v6 );
	vst1q_u32(tmp[7] , v7 );
	vst1q_u32(tmp[8] , v8 );
	vst1q_u32(tmp[9] , v9 );
	vst1q_u32(tmp[10], v10);
	vst1q_u32(tmp[11], v11);
	vst1q_u32(tmp[12], v12);
	vst1q_u32(tmp[13], v13);
	vst1q_u32(tmp[14], v14);
	vst1q_u32(tmp[15], v15);

	for (int b = 0; b < 4; b++) {
		u8 *out = cipher_text + (size_t)b * 64;
		const u8 *in = plain_text ? (plain_text + (size_t)b * 64) : 0;
		for (int i = 0; i < 16; i++) {
			u32 w = tmp[i][b];
			if (in) {
				w ^= load32_le(in + (size_t)i * 4);
			}
			store32_le(out + (size_t)i * 4, w);
		}
	}
#undef QR
}
#endif

static const u8 *chacha20_constant = (const u8*)"expand 32-byte k"; // 16 bytes

void crypto_chacha20_h(u8 out[32], const u8 key[32], const u8 in [16])
{
	u32 block[16];
	load32_le_buf(block     , chacha20_constant, 4);
	load32_le_buf(block +  4, key              , 8);
	load32_le_buf(block + 12, in               , 4);

	chacha20_rounds(block, block);

	// prevent reversal of the rounds by revealing only half of the buffer.
	store32_le_buf(out   , block   , 4); // constant
	store32_le_buf(out+16, block+12, 4); // counter and nonce
	WIPE_BUFFER(block);
}

int crypto_chacha20_h_checked(u8 out[32], const u8 key[32], const u8 in[16])
{
	int err = check_out_ptr(out);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(in, 16);
	if (err != CRYPTO_OK) { return err; }
	crypto_chacha20_h(out, key, in);
	return CRYPTO_OK;
}

#define CHACHA20_THREAD_MIN_BLOCKS 64

#if defined(MONOCYPHER_CHACHA20_PTHREADS) && MONOCYPHER_CHACHA20_PTHREADS && \
    !defined(_WIN32)
typedef struct {
	u8 *cipher_text;
	const u8 *plain_text;
	size_t text_size;
	const u8 *key;
	const u8 *nonce;
	u64 ctr;
} chacha20_job;

static u64 crypto_chacha20_djb_inner(u8 *cipher_text, const u8 *plain_text,
                                     size_t text_size, const u8 key[32],
                                     const u8 nonce[8], u64 ctr,
                                     int allow_threads);

static void *chacha20_thread_main(void *ptr)
{
	chacha20_job *job = (chacha20_job*)ptr;
	crypto_chacha20_djb_inner(job->cipher_text, job->plain_text,
	                          job->text_size, job->key, job->nonce,
	                          job->ctr, 0);
	return 0;
}
#endif

static u64 crypto_chacha20_djb_inner(u8 *cipher_text, const u8 *plain_text,
                                     size_t text_size, const u8 key[32],
                                     const u8 nonce[8], u64 ctr,
                                     int allow_threads)
{
#if !defined(MONOCYPHER_CHACHA20_PTHREADS) || !MONOCYPHER_CHACHA20_PTHREADS || \
    defined(_WIN32)
	(void)allow_threads;
#endif
	u32 input[16];
	load32_le_buf(input     , chacha20_constant, 4);
	load32_le_buf(input +  4, key              , 8);
	load32_le_buf(input + 14, nonce            , 2);
	input[12] = (u32) ctr;
	input[13] = (u32)(ctr >> 32);

	// Whole blocks
	u32    pool[16];
	size_t nb_blocks = text_size >> 6;
	size_t tail      = text_size & 63;
	u64 block_ctr = ctr;
	size_t threaded_blocks = 0;

#if defined(MONOCYPHER_CHACHA20_PTHREADS) && MONOCYPHER_CHACHA20_PTHREADS && \
    !defined(_WIN32)
		if (allow_threads && nb_blocks >= (CHACHA20_THREAD_MIN_BLOCKS * 2)) {
			size_t thread_blocks = nb_blocks / 2;
			size_t main_blocks = nb_blocks - thread_blocks;
			size_t main_bytes = main_blocks * 64;
			size_t thread_bytes = thread_blocks * 64;

		chacha20_job job;
		job.cipher_text = cipher_text + main_bytes;
		job.plain_text = plain_text ? (plain_text + main_bytes) : 0;
		job.text_size = thread_bytes;
		job.key = key;
		job.nonce = nonce;
		job.ctr = ctr + (u64)main_blocks;

		pthread_t thread;
		int spawned = pthread_create(&thread, 0, chacha20_thread_main, &job) == 0;

		// Process first half on this thread without spawning further workers.
		crypto_chacha20_djb_inner(cipher_text, plain_text, main_bytes,
		                          key, nonce, ctr, 0);

		if (spawned) {
			pthread_join(thread, 0);
		} else {
			crypto_chacha20_djb_inner(job.cipher_text, job.plain_text,
			                          job.text_size, job.key, job.nonce,
			                          job.ctr, 0);
		}

		threaded_blocks = main_blocks + thread_blocks;
		nb_blocks = 0;
		block_ctr = ctr + (u64)threaded_blocks;
	}
#endif

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	int use_avx2 = mono_have_avx2_cached();
#else
	int use_avx2 = 0;
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	int use_sse2 = mono_have_sse2_cached();
#else
	int use_sse2 = 0;
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	int use_neon = mono_have_neon_cached();
#else
	int use_neon = 0;
#endif
	if (threaded_blocks != 0) {
		cipher_text += threaded_blocks * 64;
		if (plain_text != NULL) {
			plain_text += threaded_blocks * 64;
		}
	}

	while (nb_blocks >= 8 && use_avx2) {
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
		chacha20_blocks8_avx2(cipher_text, plain_text, input, block_ctr);
#endif
		cipher_text += 64 * 8;
		if (plain_text != NULL) {
			plain_text += 64 * 8;
		}
		block_ctr += 8;
		nb_blocks -= 8;
	}

	while (nb_blocks >= 4 && (use_sse2 || use_neon)) {
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
		if (use_sse2) {
			chacha20_blocks4_sse2(cipher_text, plain_text, input, block_ctr);
		} else
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
		if (use_neon) {
			chacha20_blocks4_neon(cipher_text, plain_text, input, block_ctr);
		} else
#endif
		{
			break;
		}
		cipher_text += 64 * 4;
		if (plain_text != NULL) {
			plain_text += 64 * 4;
		}
		block_ctr += 4;
		nb_blocks -= 4;
	}

	input[12] = (u32)block_ctr;
	input[13] = (u32)(block_ctr >> 32);
	FOR (i, 0, nb_blocks) {
		chacha20_rounds(pool, input);
		if (plain_text != NULL) {
			FOR (j, 0, 16) {
				u32 p = pool[j] + input[j];
				store32_le(cipher_text, p ^ load32_le(plain_text));
				cipher_text += 4;
				plain_text  += 4;
			}
		} else {
			FOR (j, 0, 16) {
				u32 p = pool[j] + input[j];
				store32_le(cipher_text, p);
				cipher_text += 4;
			}
		}
		input[12]++;
		if (input[12] == 0) {
			input[13]++;
		}
	}
	text_size = tail;

	// Last (incomplete) block
	if (text_size > 0) {
		if (plain_text == NULL) {
			plain_text = zero;
		}
		chacha20_rounds(pool, input);
		u8 tmp[64];
		FOR (i, 0, 16) {
			store32_le(tmp + i*4, pool[i] + input[i]);
		}
		FOR (i, 0, text_size) {
			cipher_text[i] = tmp[i] ^ plain_text[i];
		}
		WIPE_BUFFER(tmp);
	}
	ctr = input[12] + ((u64)input[13] << 32) + (text_size > 0);

	WIPE_BUFFER(pool);
	WIPE_BUFFER(input);
	return ctr;
}

u64 crypto_chacha20_djb(u8 *cipher_text, const u8 *plain_text,
                        size_t text_size, const u8 key[32], const u8 nonce[8],
                        u64 ctr)
{
#if defined(MONOCYPHER_CHACHA20_PTHREADS) && MONOCYPHER_CHACHA20_PTHREADS && \
    !defined(_WIN32)
	return crypto_chacha20_djb_inner(cipher_text, plain_text, text_size,
	                                 key, nonce, ctr, 1);
#else
	return crypto_chacha20_djb_inner(cipher_text, plain_text, text_size,
	                                 key, nonce, ctr, 0);
#endif
}

u32 crypto_chacha20_ietf(u8 *cipher_text, const u8 *plain_text,
                         size_t text_size,
                         const u8 key[32], const u8 nonce[12], u32 ctr)
{
	u64 big_ctr = ctr + ((u64)load32_le(nonce) << 32);
	return (u32)crypto_chacha20_djb(cipher_text, plain_text, text_size,
	                                key, nonce + 4, big_ctr);
}

u64 crypto_chacha20_x(u8 *cipher_text, const u8 *plain_text,
                      size_t text_size,
                      const u8 key[32], const u8 nonce[24], u64 ctr)
{
	u8 sub_key[32];
	crypto_chacha20_h(sub_key, key, nonce);
	ctr = crypto_chacha20_djb(cipher_text, plain_text, text_size,
	                          sub_key, nonce + 16, ctr);
	WIPE_BUFFER(sub_key);
	return ctr;
}

int crypto_chacha20_djb_checked(u8 *cipher_text, const u8 *plain_text,
                                size_t text_size,
                                const u8 key[32], const u8 nonce[8],
                                u64 ctr, u64 *ctr_out)
{
	int err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 8);
	if (err != CRYPTO_OK) { return err; }
	if (text_size != 0 && cipher_text == 0) {
		return CRYPTO_ERR_NULL;
	}
	u64 blocks = text_size / 64 + (text_size % 64 != 0);
	u64 new_ctr = 0;
	err = checked_add_u64(ctr, blocks, &new_ctr);
	if (err != CRYPTO_OK) { return err; }
	u64 out_ctr = crypto_chacha20_djb(cipher_text, plain_text, text_size,
	                                  key, nonce, ctr);
	if (ctr_out != 0) {
		*ctr_out = out_ctr;
	}
	return CRYPTO_OK;
}

int crypto_chacha20_ietf_checked(u8 *cipher_text, const u8 *plain_text,
                                 size_t text_size,
                                 const u8 key[32], const u8 nonce[12],
                                 u32 ctr, u32 *ctr_out)
{
	int err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 12);
	if (err != CRYPTO_OK) { return err; }
	if (text_size != 0 && cipher_text == 0) {
		return CRYPTO_ERR_NULL;
	}
	u64 blocks64 = text_size / 64 + (text_size % 64 != 0);
	if (blocks64 > UINT32_MAX) {
		return CRYPTO_ERR_OVERFLOW;
	}
	u32 blocks = (u32)blocks64;
	u32 new_ctr = 0;
	err = checked_add_u32(ctr, blocks, &new_ctr);
	if (err != CRYPTO_OK) { return err; }
	u32 out_ctr = crypto_chacha20_ietf(cipher_text, plain_text, text_size,
	                                   key, nonce, ctr);
	if (ctr_out != 0) {
		*ctr_out = out_ctr;
	}
	return CRYPTO_OK;
}

int crypto_chacha20_x_checked(u8 *cipher_text, const u8 *plain_text,
                              size_t text_size,
                              const u8 key[32], const u8 nonce[24],
                              u64 ctr, u64 *ctr_out)
{
	int err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 24);
	if (err != CRYPTO_OK) { return err; }
	if (text_size != 0 && cipher_text == 0) {
		return CRYPTO_ERR_NULL;
	}
	u64 blocks = text_size / 64 + (text_size % 64 != 0);
	u64 new_ctr = 0;
	err = checked_add_u64(ctr, blocks, &new_ctr);
	if (err != CRYPTO_OK) { return err; }
	u64 out_ctr = crypto_chacha20_x(cipher_text, plain_text, text_size,
	                                key, nonce, ctr);
	if (ctr_out != 0) {
		*ctr_out = out_ctr;
	}
	return CRYPTO_OK;
}


/////////////////
/// Poly 1305 ///
/////////////////

static void poly_load32x4(const u8 *in, u32 out[4])
{
#if MONO_LITTLE_ENDIAN && defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		__m128i v = _mm_loadu_si128((const __m128i*)in);
		_mm_storeu_si128((__m128i*)out, v);
		return;
	}
#endif
#if MONO_LITTLE_ENDIAN && (defined(__ARM_NEON) || defined(__aarch64__))
	if (mono_have_neon_cached()) {
		uint8x16_t v = vld1q_u8(in);
		vst1q_u32(out, vreinterpretq_u32_u8(v));
		return;
	}
#endif
	out[0] = load32_le(in + 0);
	out[1] = load32_le(in + 4);
	out[2] = load32_le(in + 8);
	out[3] = load32_le(in + 12);
}

static u64 poly_mul_sum4_scalar(const u64 s[4], const u32 r[4])
{
	return s[0] * r[0] + s[1] * r[1] + s[2] * r[2] + s[3] * r[3];
}

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static void poly_mul_sum4x2_avx2(const u64 s[4], const u32 r0[4],
                                 const u32 r1[4], u64 *out0, u64 *out1)
{
	u32 s0 = (u32)s[0]; u32 s1 = (u32)s[1];
	u32 s2 = (u32)s[2]; u32 s3 = (u32)s[3];
	u32 sh0 = (u32)(s[0] >> 32); u32 sh1 = (u32)(s[1] >> 32);
	u32 sh2 = (u32)(s[2] >> 32); u32 sh3 = (u32)(s[3] >> 32);
	__m256i vs_lo = _mm256_setr_epi32(s0, s1, s2, s3, s0, s1, s2, s3);
	__m256i vs_hi = _mm256_setr_epi32(sh0, sh1, sh2, sh3,
	                                  sh0, sh1, sh2, sh3);
	__m256i vr = _mm256_setr_epi32(r0[0], r0[1], r0[2], r0[3],
	                               r1[0], r1[1], r1[2], r1[3]);
	__m256i prod_even = _mm256_mul_epu32(vs_lo, vr);
	__m256i prod_odd  = _mm256_mul_epu32(_mm256_srli_si256(vs_lo, 4),
	                                     _mm256_srli_si256(vr, 4));
	__m128i pe_lo = _mm256_castsi256_si128(prod_even);
	__m128i pe_hi = _mm256_extracti128_si256(prod_even, 1);
	__m128i po_lo = _mm256_castsi256_si128(prod_odd);
	__m128i po_hi = _mm256_extracti128_si256(prod_odd, 1);
	__m128i sum0v = _mm_add_epi64(pe_lo, po_lo);
	sum0v = _mm_add_epi64(sum0v, _mm_srli_si128(sum0v, 8));
	u64 sum0 = (u64)_mm_cvtsi128_si64(sum0v);
	__m128i sum1v = _mm_add_epi64(pe_hi, po_hi);
	sum1v = _mm_add_epi64(sum1v, _mm_srli_si128(sum1v, 8));
	u64 sum1 = (u64)_mm_cvtsi128_si64(sum1v);

	prod_even = _mm256_mul_epu32(vs_hi, vr);
	prod_odd  = _mm256_mul_epu32(_mm256_srli_si256(vs_hi, 4),
	                             _mm256_srli_si256(vr, 4));
	pe_lo = _mm256_castsi256_si128(prod_even);
	pe_hi = _mm256_extracti128_si256(prod_even, 1);
	po_lo = _mm256_castsi256_si128(prod_odd);
	po_hi = _mm256_extracti128_si256(prod_odd, 1);
	sum0v = _mm_add_epi64(pe_lo, po_lo);
	sum0v = _mm_add_epi64(sum0v, _mm_srli_si128(sum0v, 8));
	u64 sum0_hi = (u64)_mm_cvtsi128_si64(sum0v);
	sum1v = _mm_add_epi64(pe_hi, po_hi);
	sum1v = _mm_add_epi64(sum1v, _mm_srli_si128(sum1v, 8));
	u64 sum1_hi = (u64)_mm_cvtsi128_si64(sum1v);

	*out0 = sum0 + (sum0_hi << 32);
	*out1 = sum1 + (sum1_hi << 32);
}
#endif

#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
MONO_TARGET_SSE2
static u64 poly_mul_sum4_sse2(const u64 s[4], const u32 r[4])
{
	u32 s0 = (u32)s[0]; u32 s1 = (u32)s[1];
	u32 s2 = (u32)s[2]; u32 s3 = (u32)s[3];
	u32 sh0 = (u32)(s[0] >> 32); u32 sh1 = (u32)(s[1] >> 32);
	u32 sh2 = (u32)(s[2] >> 32); u32 sh3 = (u32)(s[3] >> 32);
	__m128i vs_lo = _mm_setr_epi32(s0, s1, s2, s3);
	__m128i vr    = _mm_loadu_si128((const __m128i*)r);
	__m128i prod02 = _mm_mul_epu32(vs_lo, vr);
	__m128i prod13 = _mm_mul_epu32(_mm_srli_si128(vs_lo, 4),
	                               _mm_srli_si128(vr, 4));
	__m128i sum = _mm_add_epi64(prod02, prod13);
	sum = _mm_add_epi64(sum, _mm_srli_si128(sum, 8));
	u64 sumv = (u64)_mm_cvtsi128_si64(sum);

	__m128i vs_hi = _mm_setr_epi32(sh0, sh1, sh2, sh3);
	__m128i prod02_hi = _mm_mul_epu32(vs_hi, vr);
	__m128i prod13_hi = _mm_mul_epu32(_mm_srli_si128(vs_hi, 4),
	                                  _mm_srli_si128(vr, 4));
	sum = _mm_add_epi64(prod02_hi, prod13_hi);
	sum = _mm_add_epi64(sum, _mm_srli_si128(sum, 8));
	u64 sum_hi = (u64)_mm_cvtsi128_si64(sum);

	return sumv + (sum_hi << 32);
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static u64 poly_mul_sum4_neon(const u64 s[4], const u32 r[4])
{
	u32 s0 = (u32)s[0]; u32 s1 = (u32)s[1];
	u32 s2 = (u32)s[2]; u32 s3 = (u32)s[3];
	u32 sh0 = (u32)(s[0] >> 32); u32 sh1 = (u32)(s[1] >> 32);
	u32 sh2 = (u32)(s[2] >> 32); u32 sh3 = (u32)(s[3] >> 32);
	uint32x4_t vs_lo = (uint32x4_t){s0, s1, s2, s3};
	uint32x4_t vr    = vld1q_u32(r);
	uint64x2_t prod_lo = vmull_u32(vget_low_u32(vs_lo), vget_low_u32(vr));
	uint64x2_t prod_hi = vmull_u32(vget_high_u32(vs_lo), vget_high_u32(vr));
	uint64x2_t sumv = vaddq_u64(prod_lo, prod_hi);
	u64 sum = vgetq_lane_u64(sumv, 0) + vgetq_lane_u64(sumv, 1);

	uint32x4_t vs_hi = (uint32x4_t){sh0, sh1, sh2, sh3};
	prod_lo = vmull_u32(vget_low_u32(vs_hi), vget_low_u32(vr));
	prod_hi = vmull_u32(vget_high_u32(vs_hi), vget_high_u32(vr));
	sumv = vaddq_u64(prod_lo, prod_hi);
	u64 sum_hi = vgetq_lane_u64(sumv, 0) + vgetq_lane_u64(sumv, 1);

	return sum + (sum_hi << 32);
}
#endif

// h = (h + c) * r
// preconditions:
//   ctx->h <= 4_ffffffff_ffffffff_ffffffff_ffffffff
//   ctx->r <=   0ffffffc_0ffffffc_0ffffffc_0fffffff
//   end    <= 1
// Postcondition:
//   ctx->h <= 4_ffffffff_ffffffff_ffffffff_ffffffff
static void poly_blocks(crypto_poly1305_ctx *ctx, const u8 *in,
                        size_t nb_blocks, unsigned end)
{
	// Local all the things!
	const u32 r0 = ctx->r[0];
	const u32 r1 = ctx->r[1];
	const u32 r2 = ctx->r[2];
	const u32 r3 = ctx->r[3];
	const u32 rr0 = (r0 >> 2) * 5;  // lose 2 bits...
	const u32 rr1 = (r1 >> 2) + r1; // rr1 == (r1 >> 2) * 5
	const u32 rr2 = (r2 >> 2) + r2; // rr1 == (r2 >> 2) * 5
	const u32 rr3 = (r3 >> 2) + r3; // rr1 == (r3 >> 2) * 5
	const u32 rr4 = r0 & 3;         // ...recover 2 bits
	u32 h0 = ctx->h[0];
	u32 h1 = ctx->h[1];
	u32 h2 = ctx->h[2];
	u32 h3 = ctx->h[3];
	u32 h4 = ctx->h[4];
	const u32 r_vec0[4] = {r0, rr3, rr2, rr1};
	const u32 r_vec1[4] = {r1, r0 , rr3, rr2};
	const u32 r_vec2[4] = {r2, r1 , r0 , rr3};
	const u32 r_vec3[4] = {r3, r2 , r1 , r0 };
	int poly_simd = 0;
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		poly_simd = 2;
	} else
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		poly_simd = 1;
	}
#endif
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (poly_simd == 0 && mono_have_neon_cached()) {
		poly_simd = 3;
	}
#endif

	FOR (i, 0, nb_blocks) {
		u32 m[4];
		poly_load32x4(in, m);
		in += 16;
		// h + c, without carry propagation
		const u64 s0 = (u64)h0 + m[0];
		const u64 s1 = (u64)h1 + m[1];
		const u64 s2 = (u64)h2 + m[2];
		const u64 s3 = (u64)h3 + m[3];
		const u32 s4 =      h4 + end;

		// (h + c) * r, without carry propagation
		u64 x0 = 0;
		u64 x1 = 0;
		u64 x2 = 0;
		u64 x3 = 0;
		const u64 s_vec[4] = {s0, s1, s2, s3};
		if (poly_simd == 2) {
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
			poly_mul_sum4x2_avx2(s_vec, r_vec0, r_vec1, &x0, &x1);
			poly_mul_sum4x2_avx2(s_vec, r_vec2, r_vec3, &x2, &x3);
#endif
		} else if (poly_simd == 1) {
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
			x0 = poly_mul_sum4_sse2(s_vec, r_vec0);
			x1 = poly_mul_sum4_sse2(s_vec, r_vec1);
			x2 = poly_mul_sum4_sse2(s_vec, r_vec2);
			x3 = poly_mul_sum4_sse2(s_vec, r_vec3);
#endif
		} else if (poly_simd == 3) {
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
			x0 = poly_mul_sum4_neon(s_vec, r_vec0);
			x1 = poly_mul_sum4_neon(s_vec, r_vec1);
			x2 = poly_mul_sum4_neon(s_vec, r_vec2);
			x3 = poly_mul_sum4_neon(s_vec, r_vec3);
#endif
		} else {
			x0 = poly_mul_sum4_scalar(s_vec, r_vec0);
			x1 = poly_mul_sum4_scalar(s_vec, r_vec1);
			x2 = poly_mul_sum4_scalar(s_vec, r_vec2);
			x3 = poly_mul_sum4_scalar(s_vec, r_vec3);
		}
		x0 += (u64)s4 * rr0;
		x1 += (u64)s4 * rr1;
		x2 += (u64)s4 * rr2;
		x3 += (u64)s4 * rr3;
		const u32 x4 = s4 * rr4;

		// partial reduction modulo 2^130 - 5
		const u32 u5 = x4 + (x3 >> 32); // u5 <= 7ffffff5
		const u64 u0 = (u5 >>  2) * 5 + (x0 & 0xffffffff);
		const u64 u1 = (u0 >> 32)     + (x1 & 0xffffffff) + (x0 >> 32);
		const u64 u2 = (u1 >> 32)     + (x2 & 0xffffffff) + (x1 >> 32);
		const u64 u3 = (u2 >> 32)     + (x3 & 0xffffffff) + (x2 >> 32);
		const u32 u4 = (u3 >> 32)     + (u5 & 3); // u4 <= 4

		// Update the hash
		h0 = u0 & 0xffffffff;
		h1 = u1 & 0xffffffff;
		h2 = u2 & 0xffffffff;
		h3 = u3 & 0xffffffff;
		h4 = u4;
	}
	ctx->h[0] = h0;
	ctx->h[1] = h1;
	ctx->h[2] = h2;
	ctx->h[3] = h3;
	ctx->h[4] = h4;
}

void crypto_poly1305_init(crypto_poly1305_ctx *ctx, const u8 key[32])
{
	ZERO(ctx->h, 5); // Initial hash is zero
	ctx->c_idx = 0;
	// load r and pad (r has some of its bits cleared)
	load32_le_buf(ctx->r  , key   , 4);
	load32_le_buf(ctx->pad, key+16, 4);
	FOR (i, 0, 1) { ctx->r[i] &= 0x0fffffff; }
	FOR (i, 1, 4) { ctx->r[i] &= 0x0ffffffc; }
}

void crypto_poly1305_update(crypto_poly1305_ctx *ctx,
                            const u8 *message, size_t message_size)
{
	// Avoid undefined NULL pointer increments with empty messages
	if (message_size == 0) {
		return;
	}

	// Align ourselves with block boundaries
	size_t aligned = MIN(gap(ctx->c_idx, 16), message_size);
	FOR (i, 0, aligned) {
		ctx->c[ctx->c_idx] = *message;
		ctx->c_idx++;
		message++;
		message_size--;
	}

	// If block is complete, process it
	if (ctx->c_idx == 16) {
		poly_blocks(ctx, ctx->c, 1, 1);
		ctx->c_idx = 0;
	}

	// Process the message block by block
	size_t nb_blocks = message_size >> 4;
	poly_blocks(ctx, message, nb_blocks, 1);
	message      += nb_blocks << 4;
	message_size &= 15;

	// remaining bytes (we never complete a block here)
	FOR (i, 0, message_size) {
		ctx->c[ctx->c_idx] = message[i];
		ctx->c_idx++;
	}
}

void crypto_poly1305_final(crypto_poly1305_ctx *ctx, u8 mac[16])
{
	// Process the last block (if any)
	// We move the final 1 according to remaining input length
	// (this will add less than 2^130 to the last input block)
	if (ctx->c_idx != 0) {
		ZERO(ctx->c + ctx->c_idx, 16 - ctx->c_idx);
		ctx->c[ctx->c_idx] = 1;
		poly_blocks(ctx, ctx->c, 1, 0);
	}

	// check if we should subtract 2^130-5 by performing the
	// corresponding carry propagation.
	u64 c = 5;
	FOR (i, 0, 4) {
		c  += ctx->h[i];
		c >>= 32;
	}
	c += ctx->h[4];
	c  = (c >> 2) * 5; // shift the carry back to the beginning
	// c now indicates how many times we should subtract 2^130-5 (0 or 1)
	FOR (i, 0, 4) {
		c += (u64)ctx->h[i] + ctx->pad[i];
		store32_le(mac + i*4, (u32)c);
		c = c >> 32;
	}
	WIPE_CTX(ctx);
}

int crypto_poly1305_init_checked(crypto_poly1305_ctx *ctx, const u8 key[32])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_poly1305_init(ctx, key);
	return CRYPTO_OK;
}

int crypto_poly1305_update_checked(crypto_poly1305_ctx *ctx,
                                   const u8 *message, size_t message_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_poly1305_update(ctx, message, message_size);
	return CRYPTO_OK;
}

int crypto_poly1305_final_checked(crypto_poly1305_ctx *ctx, u8 mac[16])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(mac);
	if (err != CRYPTO_OK) { return err; }
	crypto_poly1305_final(ctx, mac);
	return CRYPTO_OK;
}

void crypto_poly1305(u8     mac[16],  const u8 *message,
                     size_t message_size, const u8  key[32])
{
	crypto_poly1305_ctx ctx;
	crypto_poly1305_init  (&ctx, key);
	crypto_poly1305_update(&ctx, message, message_size);
	crypto_poly1305_final (&ctx, mac);
}

int crypto_poly1305_checked(u8 mac[16], const u8 *message,
                            size_t message_size, const u8 key[32])
{
	int err = check_out_ptr(mac);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_poly1305(mac, message, message_size, key);
	return CRYPTO_OK;
}

////////////////
/// BLAKE2 b ///
////////////////
static const u64 iv[8] = {
	0x6a09e667f3bcc908, 0xbb67ae8584caa73b,
	0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
	0x510e527fade682d1, 0x9b05688c2b3e6c1f,
	0x1f83d9abfb41bd6b, 0x5be0cd19137e2179,
};

static void blake2b_compress(crypto_blake2b_ctx *ctx, int is_last_block)
{
	static const u8 sigma[12][16] = {
		{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
		{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
		{ 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
		{  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
		{  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
		{  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 },
		{ 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 },
		{ 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 },
		{  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 },
		{ 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13,  0 },
		{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
		{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
	};

	// increment input offset
	u64   *x = ctx->input_offset;
	size_t y = ctx->input_idx;
	x[0] += y;
	if (x[0] < y) {
		x[1]++;
	}

	// init work vector
	u64 v0 = ctx->hash[0];  u64 v8  = iv[0];
	u64 v1 = ctx->hash[1];  u64 v9  = iv[1];
	u64 v2 = ctx->hash[2];  u64 v10 = iv[2];
	u64 v3 = ctx->hash[3];  u64 v11 = iv[3];
	u64 v4 = ctx->hash[4];  u64 v12 = iv[4] ^ ctx->input_offset[0];
	u64 v5 = ctx->hash[5];  u64 v13 = iv[5] ^ ctx->input_offset[1];
	u64 v6 = ctx->hash[6];  u64 v14 = iv[6] ^ (u64)~(is_last_block - 1);
	u64 v7 = ctx->hash[7];  u64 v15 = iv[7];

	// mangle work vector
	u64 *input = ctx->input;
#define BLAKE2_G(a, b, c, d, x, y)	\
	a += b + x;  d = rotr64(d ^ a, 32); \
	c += d;      b = rotr64(b ^ c, 24); \
	a += b + y;  d = rotr64(d ^ a, 16); \
	c += d;      b = rotr64(b ^ c, 63)
#define BLAKE2_ROUND(i)	\
	BLAKE2_G(v0, v4, v8 , v12, input[sigma[i][ 0]], input[sigma[i][ 1]]); \
	BLAKE2_G(v1, v5, v9 , v13, input[sigma[i][ 2]], input[sigma[i][ 3]]); \
	BLAKE2_G(v2, v6, v10, v14, input[sigma[i][ 4]], input[sigma[i][ 5]]); \
	BLAKE2_G(v3, v7, v11, v15, input[sigma[i][ 6]], input[sigma[i][ 7]]); \
	BLAKE2_G(v0, v5, v10, v15, input[sigma[i][ 8]], input[sigma[i][ 9]]); \
	BLAKE2_G(v1, v6, v11, v12, input[sigma[i][10]], input[sigma[i][11]]); \
	BLAKE2_G(v2, v7, v8 , v13, input[sigma[i][12]], input[sigma[i][13]]); \
	BLAKE2_G(v3, v4, v9 , v14, input[sigma[i][14]], input[sigma[i][15]])

#ifdef BLAKE2_NO_UNROLLING
	FOR (i, 0, 12) {
		BLAKE2_ROUND(i);
	}
#else
	BLAKE2_ROUND(0);  BLAKE2_ROUND(1);  BLAKE2_ROUND(2);  BLAKE2_ROUND(3);
	BLAKE2_ROUND(4);  BLAKE2_ROUND(5);  BLAKE2_ROUND(6);  BLAKE2_ROUND(7);
	BLAKE2_ROUND(8);  BLAKE2_ROUND(9);  BLAKE2_ROUND(10); BLAKE2_ROUND(11);
#endif

	// update hash
	ctx->hash[0] ^= v0 ^ v8;   ctx->hash[1] ^= v1 ^ v9;
	ctx->hash[2] ^= v2 ^ v10;  ctx->hash[3] ^= v3 ^ v11;
	ctx->hash[4] ^= v4 ^ v12;  ctx->hash[5] ^= v5 ^ v13;
	ctx->hash[6] ^= v6 ^ v14;  ctx->hash[7] ^= v7 ^ v15;
}

void crypto_blake2b_keyed_init(crypto_blake2b_ctx *ctx, size_t hash_size,
                               const u8 *key, size_t key_size)
{
	// initial hash
	COPY(ctx->hash, iv, 8);
	ctx->hash[0] ^= 0x01010000 ^ (key_size << 8) ^ hash_size;

	ctx->input_offset[0] = 0;  // beginning of the input, no offset
	ctx->input_offset[1] = 0;  // beginning of the input, no offset
	ctx->hash_size       = hash_size;
	ctx->input_idx       = 0;
	ZERO(ctx->input, 16);

	// if there is a key, the first block is that key (padded with zeroes)
	if (key_size > 0) {
		u8 key_block[128] = {0};
		COPY(key_block, key, key_size);
		// same as calling crypto_blake2b_update(ctx, key_block , 128)
		load64_le_buf(ctx->input, key_block, 16);
		ctx->input_idx = 128;
	}
}

void crypto_blake2b_init(crypto_blake2b_ctx *ctx, size_t hash_size)
{
	crypto_blake2b_keyed_init(ctx, hash_size, 0, 0);
}

void crypto_blake2b_update(crypto_blake2b_ctx *ctx,
                           const u8 *message, size_t message_size)
{
	// Avoid undefined NULL pointer increments with empty messages
	if (message_size == 0) {
		return;
	}

	// Align with word boundaries
	if ((ctx->input_idx & 7) != 0) {
		size_t nb_bytes = MIN(gap(ctx->input_idx, 8), message_size);
		size_t word     = ctx->input_idx >> 3;
		size_t byte     = ctx->input_idx & 7;
		FOR (i, 0, nb_bytes) {
			ctx->input[word] |= (u64)message[i] << ((byte + i) << 3);
		}
		ctx->input_idx += nb_bytes;
		message        += nb_bytes;
		message_size   -= nb_bytes;
	}

	// Align with block boundaries (faster than byte by byte)
	if ((ctx->input_idx & 127) != 0) {
		size_t nb_words = MIN(gap(ctx->input_idx, 128), message_size) >> 3;
		load64_le_buf(ctx->input + (ctx->input_idx >> 3), message, nb_words);
		ctx->input_idx += nb_words << 3;
		message        += nb_words << 3;
		message_size   -= nb_words << 3;
	}

	// Process block by block
	size_t nb_blocks = message_size >> 7;
	FOR (i, 0, nb_blocks) {
		if (ctx->input_idx == 128) {
			blake2b_compress(ctx, 0);
		}
		load64_le_buf(ctx->input, message, 16);
		message += 128;
		ctx->input_idx = 128;
	}
	message_size &= 127;

	if (message_size != 0) {
		// Compress block & flush input buffer as needed
		if (ctx->input_idx == 128) {
			blake2b_compress(ctx, 0);
			ctx->input_idx = 0;
		}
		if (ctx->input_idx == 0) {
			ZERO(ctx->input, 16);
		}
		// Fill remaining words (faster than byte by byte)
		size_t nb_words = message_size >> 3;
		load64_le_buf(ctx->input, message, nb_words);
		ctx->input_idx += nb_words << 3;
		message        += nb_words << 3;
		message_size   -= nb_words << 3;

		// Fill remaining bytes
		FOR (i, 0, message_size) {
			size_t word = ctx->input_idx >> 3;
			size_t byte = ctx->input_idx & 7;
			ctx->input[word] |= (u64)message[i] << (byte << 3);
			ctx->input_idx++;
		}
	}
}

void crypto_blake2b_final(crypto_blake2b_ctx *ctx, u8 *hash)
{
	blake2b_compress(ctx, 1); // compress the last block
	size_t hash_size = MIN(ctx->hash_size, 64);
	size_t nb_words  = hash_size >> 3;
	store64_le_buf(hash, ctx->hash, nb_words);
	FOR (i, nb_words << 3, hash_size) {
		hash[i] = (ctx->hash[i >> 3] >> (8 * (i & 7))) & 0xff;
	}
	WIPE_CTX(ctx);
}

int crypto_blake2b_init_checked(crypto_blake2b_ctx *ctx, size_t hash_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	if (hash_size == 0 || hash_size > 64) { return CRYPTO_ERR_SIZE; }
	crypto_blake2b_init(ctx, hash_size);
	return CRYPTO_OK;
}

int crypto_blake2b_keyed_init_checked(crypto_blake2b_ctx *ctx,
                                      size_t hash_size,
                                      const u8 *key, size_t key_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	if (hash_size == 0 || hash_size > 64 || key_size > 64) {
		return CRYPTO_ERR_SIZE;
	}
	err = check_ptr(key, key_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake2b_keyed_init(ctx, hash_size, key, key_size);
	return CRYPTO_OK;
}

int crypto_blake2b_update_checked(crypto_blake2b_ctx *ctx,
                                  const u8 *message, size_t message_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	if (ctx->hash_size == 0 || ctx->hash_size > 64) {
		return CRYPTO_ERR_SIZE;
	}
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake2b_update(ctx, message, message_size);
	return CRYPTO_OK;
}

int crypto_blake2b_final_checked(crypto_blake2b_ctx *ctx, u8 *hash)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(hash);
	if (err != CRYPTO_OK) { return err; }
	if (ctx->hash_size == 0 || ctx->hash_size > 64) {
		return CRYPTO_ERR_SIZE;
	}
	crypto_blake2b_final(ctx, hash);
	return CRYPTO_OK;
}

void crypto_blake2b_keyed(u8 *hash,          size_t hash_size,
                          const u8 *key,     size_t key_size,
                          const u8 *message, size_t message_size)
{
	crypto_blake2b_ctx ctx;
	crypto_blake2b_keyed_init(&ctx, hash_size, key, key_size);
	crypto_blake2b_update    (&ctx, message, message_size);
	crypto_blake2b_final     (&ctx, hash);
}

void crypto_blake2b(u8 *hash, size_t hash_size, const u8 *msg, size_t msg_size)
{
	crypto_blake2b_keyed(hash, hash_size, 0, 0, msg, msg_size);
}

int crypto_blake2b_checked(u8 *hash, size_t hash_size,
                           const u8 *message, size_t message_size)
{
	int err = check_out_ptr(hash);
	if (err != CRYPTO_OK) { return err; }
	if (hash_size == 0 || hash_size > 64) { return CRYPTO_ERR_SIZE; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake2b(hash, hash_size, message, message_size);
	return CRYPTO_OK;
}

int crypto_blake2b_keyed_checked(u8 *hash, size_t hash_size,
                                 const u8 *key, size_t key_size,
                                 const u8 *message, size_t message_size)
{
	int err = check_out_ptr(hash);
	if (err != CRYPTO_OK) { return err; }
	if (hash_size == 0 || hash_size > 64 || key_size > 64) {
		return CRYPTO_ERR_SIZE;
	}
	err = check_ptr(key, key_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake2b_keyed(hash, hash_size, key, key_size,
	                     message, message_size);
	return CRYPTO_OK;
}

/////////////////
/// SHA-256 ////
/////////////////
#define SHA256_BLOCK_SIZE 64

static const u32 sha256_iv[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};

static const u32 sha256_k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

#define SHA256_CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_BIG0(x)      (rotr32((x),  2) ^ rotr32((x), 13) ^ rotr32((x), 22))
#define SHA256_BIG1(x)      (rotr32((x),  6) ^ rotr32((x), 11) ^ rotr32((x), 25))
#define SHA256_SMALL0(x)    (rotr32((x),  7) ^ rotr32((x), 18) ^ ((x) >>  3))
#define SHA256_SMALL1(x)    (rotr32((x), 17) ^ rotr32((x), 19) ^ ((x) >> 10))

static void sha256_compress(crypto_sha256_ctx *ctx)
{
	u32 w[64];
	load32_be_buf(w, ctx->input, 16);
	FOR (i, 16, 64) {
		w[i] = SHA256_SMALL1(w[i - 2]) + w[i - 7]
		     + SHA256_SMALL0(w[i - 15]) + w[i - 16];
	}

	u32 a = ctx->hash[0];
	u32 b = ctx->hash[1];
	u32 c = ctx->hash[2];
	u32 d = ctx->hash[3];
	u32 e = ctx->hash[4];
	u32 f = ctx->hash[5];
	u32 g = ctx->hash[6];
	u32 h = ctx->hash[7];

	FOR (i, 0, 64) {
		u32 t1 = h + SHA256_BIG1(e) + SHA256_CH(e, f, g) + sha256_k[i] + w[i];
		u32 t2 = SHA256_BIG0(a) + SHA256_MAJ(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	ctx->hash[0] += a;  ctx->hash[1] += b;
	ctx->hash[2] += c;  ctx->hash[3] += d;
	ctx->hash[4] += e;  ctx->hash[5] += f;
	ctx->hash[6] += g;  ctx->hash[7] += h;
}

void crypto_sha256_init(crypto_sha256_ctx *ctx)
{
	COPY(ctx->hash, sha256_iv, 8);
	ctx->input_size = 0;
	ctx->input_idx  = 0;
	ZERO(ctx->input, sizeof(ctx->input));
}

void crypto_sha256_update(crypto_sha256_ctx *ctx,
                          const u8 *message, size_t message_size)
{
	if (message_size == 0) {
		return;
	}

	while (message_size > 0) {
		size_t take = MIN(SHA256_BLOCK_SIZE - ctx->input_idx, message_size);
		COPY(ctx->input + ctx->input_idx, message, take);
		ctx->input_idx += take;
		message        += take;
		message_size   -= take;

		if (ctx->input_idx == SHA256_BLOCK_SIZE) {
			ctx->input_size += 512;
			sha256_compress(ctx);
			ctx->input_idx = 0;
			ZERO(ctx->input, sizeof(ctx->input));
		}
	}
}

void crypto_sha256_final(crypto_sha256_ctx *ctx, u8 hash[32])
{
	ctx->input_size += (u64)ctx->input_idx * 8;

	ctx->input[ctx->input_idx] = 0x80;
	ctx->input_idx++;

	if (ctx->input_idx > 56) {
		ZERO(ctx->input + ctx->input_idx, SHA256_BLOCK_SIZE - ctx->input_idx);
		sha256_compress(ctx);
		ctx->input_idx = 0;
		ZERO(ctx->input, sizeof(ctx->input));
	}

	ZERO(ctx->input + ctx->input_idx, 56 - ctx->input_idx);
	store64_be(ctx->input + 56, ctx->input_size);
	sha256_compress(ctx);

	store32_be_buf(hash, ctx->hash, 8);
	WIPE_CTX(ctx);
}

void crypto_sha256(u8 hash[32], const u8 *message, size_t message_size)
{
	crypto_sha256_ctx ctx;
	crypto_sha256_init  (&ctx);
	crypto_sha256_update(&ctx, message, message_size);
	crypto_sha256_final (&ctx, hash);
}

////////////////////
/// HMAC SHA-256 ///
////////////////////
void crypto_sha256_hmac_init(crypto_sha256_hmac_ctx *ctx,
                             const u8 *key, size_t key_size)
{
	if (key_size > 64) {
		crypto_sha256(ctx->key, key, key_size);
		key      = ctx->key;
		key_size = 32;
	}
	FOR (i, 0, key_size)   { ctx->key[i] = key[i] ^ 0x36; }
	FOR (i, key_size, 64)  { ctx->key[i] = 0x36; }
	crypto_sha256_init  (&ctx->ctx);
	crypto_sha256_update(&ctx->ctx, ctx->key, 64);
}

void crypto_sha256_hmac_update(crypto_sha256_hmac_ctx *ctx,
                               const u8 *message, size_t message_size)
{
	crypto_sha256_update(&ctx->ctx, message, message_size);
}

void crypto_sha256_hmac_final(crypto_sha256_hmac_ctx *ctx, u8 hmac[32])
{
	crypto_sha256_final(&ctx->ctx, hmac);
	FOR (i, 0, 64) {
		ctx->key[i] ^= 0x36 ^ 0x5c;
	}
	crypto_sha256_init  (&ctx->ctx);
	crypto_sha256_update(&ctx->ctx, ctx->key, 64);
	crypto_sha256_update(&ctx->ctx, hmac, 32);
	crypto_sha256_final (&ctx->ctx, hmac);
	WIPE_CTX(ctx);
}

void crypto_sha256_hmac(u8 hmac[32], const u8 *key, size_t key_size,
                        const u8 *message, size_t message_size)
{
	crypto_sha256_hmac_ctx ctx;
	crypto_sha256_hmac_init  (&ctx, key, key_size);
	crypto_sha256_hmac_update(&ctx, message, message_size);
	crypto_sha256_hmac_final (&ctx, hmac);
}

////////////////////
/// HKDF SHA-256 ///
////////////////////
void crypto_sha256_hkdf_expand(u8 *okm,  size_t okm_size,
                               const u8 *prk,  size_t prk_size,
                               const u8 *info, size_t info_size)
{
	u8   ctr = 1;
	u8   blk[32];
	u8  *o = okm;
	u8  *r = okm + okm_size;
	u8  *x = okm + 32;
	u8  *m = x < r ? x : r;
	size_t out_size = 0;
	while (o < r) {
		crypto_sha256_hmac_ctx ctx;
		crypto_sha256_hmac_init(&ctx, prk , prk_size);
		if (out_size > 0) {
			crypto_sha256_hmac_update(&ctx, blk , sizeof(blk));
		}
		crypto_sha256_hmac_update(&ctx, info, info_size);
		crypto_sha256_hmac_update(&ctx, &ctr, 1);
		crypto_sha256_hmac_final(&ctx, blk);
		size_t copy_len = (size_t)(m - o);
		COPY(o, blk, copy_len);
		out_size += 1;
		o  = m;
		x += 32;
		m  = x < r ? x : r;
		ctr++;
	}
	WIPE_BUFFER(blk);
}

void crypto_sha256_hkdf(u8 *okm, size_t okm_size,
                        const u8 *ikm, size_t ikm_size,
                        const u8 *salt, size_t salt_size,
                        const u8 *info, size_t info_size)
{
	u8 prk[32];
	crypto_sha256_hmac(prk, salt, salt_size, ikm, ikm_size);
	crypto_sha256_hkdf_expand(okm, okm_size, prk, sizeof(prk), info, info_size);
	WIPE_BUFFER(prk);
}

int crypto_sha256_init_checked(crypto_sha256_ctx *ctx)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_init(ctx);
	return CRYPTO_OK;
}

int crypto_sha256_update_checked(crypto_sha256_ctx *ctx,
                                 const u8 *message, size_t message_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_update(ctx, message, message_size);
	return CRYPTO_OK;
}

int crypto_sha256_final_checked(crypto_sha256_ctx *ctx, u8 hash[32])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(hash);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_final(ctx, hash);
	return CRYPTO_OK;
}

int crypto_sha256_hmac_init_checked(crypto_sha256_hmac_ctx *ctx,
                                    const u8 *key, size_t key_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, key_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_hmac_init(ctx, key, key_size);
	return CRYPTO_OK;
}

int crypto_sha256_hmac_update_checked(crypto_sha256_hmac_ctx *ctx,
                                      const u8 *message,
                                      size_t message_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_hmac_update(ctx, message, message_size);
	return CRYPTO_OK;
}

int crypto_sha256_hmac_final_checked(crypto_sha256_hmac_ctx *ctx,
                                     u8 hmac[32])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(hmac);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_hmac_final(ctx, hmac);
	return CRYPTO_OK;
}

int crypto_sha256_hkdf_expand_checked(u8 *okm, size_t okm_size,
                                      const u8 *prk, size_t prk_size,
                                      const u8 *info, size_t info_size)
{
	int err = check_out_ptr(okm);
	if (err != CRYPTO_OK) { return err; }
	if (okm_size > 32 * 255) { return CRYPTO_ERR_SIZE; }
	err = check_ptr(prk, prk_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(info, info_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_hkdf_expand(okm, okm_size, prk, prk_size, info, info_size);
	return CRYPTO_OK;
}

int crypto_sha256_checked(u8 hash[32],
                          const u8 *message, size_t message_size)
{
	int err = check_out_ptr(hash);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256(hash, message, message_size);
	return CRYPTO_OK;
}

int crypto_sha256_hmac_checked(u8 hmac[32],
                               const u8 *key, size_t key_size,
                               const u8 *message, size_t message_size)
{
	int err = check_out_ptr(hmac);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, key_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_hmac(hmac, key, key_size, message, message_size);
	return CRYPTO_OK;
}

int crypto_sha256_hkdf_checked(u8 *okm, size_t okm_size,
                               const u8 *ikm, size_t ikm_size,
                               const u8 *salt, size_t salt_size,
                               const u8 *info, size_t info_size)
{
	int err = check_out_ptr(okm);
	if (err != CRYPTO_OK) { return err; }
	if (okm_size > 32 * 255) { return CRYPTO_ERR_SIZE; }
	err = check_ptr(ikm, ikm_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(salt, salt_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(info, info_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_sha256_hkdf(okm, okm_size, ikm, ikm_size, salt, salt_size,
	                   info, info_size);
	return CRYPTO_OK;
}

////////////////
/// BLAKE3 ////
////////////////
#define BLAKE3_OUT_LEN   32
#define BLAKE3_KEY_LEN   32
#define BLAKE3_BLOCK_LEN 64
#define BLAKE3_CHUNK_LEN 1024
#define BLAKE3_MAX_DEPTH 54
#define BLAKE3_MAX_SIMD_DEGREE 1
#define BLAKE3_MAX_SIMD_DEGREE_OR_2 2
#define BLAKE3_THREAD_MIN_LEN (4 * BLAKE3_CHUNK_LEN)

enum blake3_flags {
	BLAKE3_CHUNK_START         = 1 << 0,
	BLAKE3_CHUNK_END           = 1 << 1,
	BLAKE3_PARENT              = 1 << 2,
	BLAKE3_ROOT                = 1 << 3,
	BLAKE3_KEYED_HASH          = 1 << 4,
	BLAKE3_DERIVE_KEY_CONTEXT  = 1 << 5,
	BLAKE3_DERIVE_KEY_MATERIAL = 1 << 6,
};

static const u32 blake3_iv[8] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};

static const u8 blake3_msg_schedule[7][16] = {
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
	{2, 6, 3, 10, 7, 0, 4, 13, 1, 11, 12, 5, 9, 14, 15, 8},
	{3, 4, 10, 12, 13, 2, 7, 14, 6, 5, 9, 0, 11, 15, 8, 1},
	{10, 7, 12, 9, 14, 3, 13, 15, 4, 0, 11, 2, 5, 8, 1, 6},
	{12, 13, 9, 11, 15, 10, 14, 8, 7, 2, 5, 3, 0, 1, 6, 4},
	{9, 14, 11, 5, 8, 12, 15, 1, 13, 3, 0, 10, 2, 6, 4, 7},
	{11, 15, 5, 0, 1, 9, 8, 6, 14, 10, 2, 12, 3, 4, 7, 13},
};

static u64 blake3_round_down_to_power_of_2(u64 x)
{
	if (x == 0) {
		return 1;
	}
	u64 p = 1;
	while (p <= x) {
		p <<= 1;
	}
	return p >> 1;
}

static u32 blake3_counter_low(u64 counter) { return (u32)counter; }
static u32 blake3_counter_high(u64 counter) { return (u32)(counter >> 32); }

static void blake3_load_key_words(const u8 key[BLAKE3_KEY_LEN], u32 key_words[8])
{
	FOR (i, 0, 8) {
		key_words[i] = load32_le(key + i * 4);
	}
}

static void blake3_store_cv_words(u8 out[32], const u32 cv_words[8])
{
	FOR (i, 0, 8) {
		store32_le(out + i * 4, cv_words[i]);
	}
}

static u32 blake3_rotr32(u32 w, u32 c)
{
	return (w >> c) | (w << (32 - c));
}

static void blake3_g(u32 *state, size_t a, size_t b, size_t c, size_t d,
                     u32 x, u32 y)
{
	state[a] = state[a] + state[b] + x;
	state[d] = blake3_rotr32(state[d] ^ state[a], 16);
	state[c] = state[c] + state[d];
	state[b] = blake3_rotr32(state[b] ^ state[c], 12);
	state[a] = state[a] + state[b] + y;
	state[d] = blake3_rotr32(state[d] ^ state[a], 8);
	state[c] = state[c] + state[d];
	state[b] = blake3_rotr32(state[b] ^ state[c], 7);
}

static void blake3_round_fn(u32 state[16], const u32 *msg, size_t round)
{
	const u8 *schedule = blake3_msg_schedule[round];
	blake3_g(state, 0, 4, 8, 12, msg[schedule[0]], msg[schedule[1]]);
	blake3_g(state, 1, 5, 9, 13, msg[schedule[2]], msg[schedule[3]]);
	blake3_g(state, 2, 6, 10, 14, msg[schedule[4]], msg[schedule[5]]);
	blake3_g(state, 3, 7, 11, 15, msg[schedule[6]], msg[schedule[7]]);
	blake3_g(state, 0, 5, 10, 15, msg[schedule[8]], msg[schedule[9]]);
	blake3_g(state, 1, 6, 11, 12, msg[schedule[10]], msg[schedule[11]]);
	blake3_g(state, 2, 7, 8, 13, msg[schedule[12]], msg[schedule[13]]);
	blake3_g(state, 3, 4, 9, 14, msg[schedule[14]], msg[schedule[15]]);
}

static void blake3_compress_pre(u32 state[16], const u32 cv[8],
                                const u8 block[BLAKE3_BLOCK_LEN],
                                u8 block_len, u64 counter, u8 flags)
{
	u32 block_words[16];
	FOR (i, 0, 16) {
		block_words[i] = load32_le(block + i * 4);
	}
	state[0] = cv[0];
	state[1] = cv[1];
	state[2] = cv[2];
	state[3] = cv[3];
	state[4] = cv[4];
	state[5] = cv[5];
	state[6] = cv[6];
	state[7] = cv[7];
	state[8] = blake3_iv[0];
	state[9] = blake3_iv[1];
	state[10] = blake3_iv[2];
	state[11] = blake3_iv[3];
	state[12] = blake3_counter_low(counter);
	state[13] = blake3_counter_high(counter);
	state[14] = (u32)block_len;
	state[15] = (u32)flags;

	blake3_round_fn(state, block_words, 0);
	blake3_round_fn(state, block_words, 1);
	blake3_round_fn(state, block_words, 2);
	blake3_round_fn(state, block_words, 3);
	blake3_round_fn(state, block_words, 4);
	blake3_round_fn(state, block_words, 5);
	blake3_round_fn(state, block_words, 6);
}

static void blake3_compress_in_place(u32 cv[8],
                                     const u8 block[BLAKE3_BLOCK_LEN],
                                     u8 block_len, u64 counter, u8 flags)
{
	u32 state[16];
	blake3_compress_pre(state, cv, block, block_len, counter, flags);
	cv[0] = state[0] ^ state[8];
	cv[1] = state[1] ^ state[9];
	cv[2] = state[2] ^ state[10];
	cv[3] = state[3] ^ state[11];
	cv[4] = state[4] ^ state[12];
	cv[5] = state[5] ^ state[13];
	cv[6] = state[6] ^ state[14];
	cv[7] = state[7] ^ state[15];
}

static void blake3_compress_xof(const u32 cv[8],
                                const u8 block[BLAKE3_BLOCK_LEN],
                                u8 block_len, u64 counter, u8 flags,
                                u8 out[64])
{
	u32 state[16];
	blake3_compress_pre(state, cv, block, block_len, counter, flags);

	store32_le(out +  0, state[0] ^ state[8]);
	store32_le(out +  4, state[1] ^ state[9]);
	store32_le(out +  8, state[2] ^ state[10]);
	store32_le(out + 12, state[3] ^ state[11]);
	store32_le(out + 16, state[4] ^ state[12]);
	store32_le(out + 20, state[5] ^ state[13]);
	store32_le(out + 24, state[6] ^ state[14]);
	store32_le(out + 28, state[7] ^ state[15]);
	store32_le(out + 32, state[8] ^ cv[0]);
	store32_le(out + 36, state[9] ^ cv[1]);
	store32_le(out + 40, state[10] ^ cv[2]);
	store32_le(out + 44, state[11] ^ cv[3]);
	store32_le(out + 48, state[12] ^ cv[4]);
	store32_le(out + 52, state[13] ^ cv[5]);
	store32_le(out + 56, state[14] ^ cv[6]);
	store32_le(out + 60, state[15] ^ cv[7]);
}

static void blake3_hash_one(const u8 *input, size_t blocks,
                            const u32 key[8], u64 counter,
                            u8 flags, u8 flags_start, u8 flags_end,
                            u8 out[BLAKE3_OUT_LEN])
{
	u32 cv[8];
	COPY(cv, key, 8);
	u8 block_flags = flags | flags_start;
	while (blocks > 0) {
		if (blocks == 1) {
			block_flags |= flags_end;
		}
		blake3_compress_in_place(cv, input, BLAKE3_BLOCK_LEN,
		                         counter, block_flags);
		input += BLAKE3_BLOCK_LEN;
		blocks -= 1;
		block_flags = flags;
	}
	blake3_store_cv_words(out, cv);
}

static void blake3_hash_many(const u8 *const *inputs, size_t num_inputs,
                             size_t blocks, const u32 key[8],
                             u64 counter, int increment_counter,
                             u8 flags, u8 flags_start, u8 flags_end, u8 *out)
{
	while (num_inputs > 0) {
		blake3_hash_one(inputs[0], blocks, key, counter, flags,
		               flags_start, flags_end, out);
		if (increment_counter) {
			counter += 1;
		}
		inputs += 1;
		num_inputs -= 1;
		out += BLAKE3_OUT_LEN;
	}
}

static void blake3_xof_many(const u32 cv[8], const u8 block[BLAKE3_BLOCK_LEN],
                            u8 block_len, u64 counter, u8 flags,
                            u8 *out, size_t outblocks)
{
	FOR (i, 0, outblocks) {
		blake3_compress_xof(cv, block, block_len, counter + i, flags, out);
		out += 64;
	}
}

static size_t blake3_simd_degree(void)
{
	return BLAKE3_MAX_SIMD_DEGREE;
}

typedef struct {
	u32 input_cv[8];
	u64 counter;
	u8  block[BLAKE3_BLOCK_LEN];
	u8  block_len;
	u8  flags;
} blake3_output_t;

static void blake3_chunk_state_init(crypto_blake3_ctx *self, const u32 key[8],
                                    u8 flags)
{
	COPY(self->chunk.cv, key, 8);
	self->chunk.chunk_counter = 0;
	ZERO(self->chunk.buf, BLAKE3_BLOCK_LEN);
	self->chunk.buf_len = 0;
	self->chunk.blocks_compressed = 0;
	self->chunk.flags = flags;
}

static void blake3_chunk_state_reset(crypto_blake3_ctx *self, const u32 key[8],
                                     u64 chunk_counter)
{
	COPY(self->chunk.cv, key, 8);
	self->chunk.chunk_counter = chunk_counter;
	self->chunk.blocks_compressed = 0;
	ZERO(self->chunk.buf, BLAKE3_BLOCK_LEN);
	self->chunk.buf_len = 0;
}

static size_t blake3_chunk_state_len(const crypto_blake3_ctx *self)
{
	return (BLAKE3_BLOCK_LEN * (size_t)self->chunk.blocks_compressed)
	     + (size_t)self->chunk.buf_len;
}

static size_t blake3_chunk_state_fill_buf(crypto_blake3_ctx *self,
                                          const u8 *input, size_t input_len)
{
	size_t take = BLAKE3_BLOCK_LEN - (size_t)self->chunk.buf_len;
	if (take > input_len) {
		take = input_len;
	}
	COPY(self->chunk.buf + self->chunk.buf_len, input, take);
	self->chunk.buf_len += (u8)take;
	return take;
}

static u8 blake3_chunk_state_maybe_start_flag(const crypto_blake3_ctx *self)
{
	return self->chunk.blocks_compressed == 0 ? BLAKE3_CHUNK_START : 0;
}

static blake3_output_t blake3_make_output(const u32 input_cv[8],
                                          const u8 block[BLAKE3_BLOCK_LEN],
                                          u8 block_len, u64 counter, u8 flags)
{
	blake3_output_t ret;
	COPY(ret.input_cv, input_cv, 8);
	COPY(ret.block, block, BLAKE3_BLOCK_LEN);
	ret.block_len = block_len;
	ret.counter = counter;
	ret.flags = flags;
	return ret;
}

static void blake3_output_chaining_value(const blake3_output_t *self, u8 cv[32])
{
	u32 cv_words[8];
	COPY(cv_words, self->input_cv, 8);
	blake3_compress_in_place(cv_words, self->block, self->block_len,
	                         self->counter, self->flags);
	blake3_store_cv_words(cv, cv_words);
}

static void blake3_output_root_bytes(const blake3_output_t *self, u64 seek,
                                     u8 *out, size_t out_len)
{
	if (out_len == 0) {
		return;
	}
	u64 output_block_counter = seek / 64;
	size_t offset_within_block = (size_t)(seek % 64);
	u8 wide_buf[64];
	if (offset_within_block) {
		blake3_compress_xof(self->input_cv, self->block, self->block_len,
		                    output_block_counter, self->flags | BLAKE3_ROOT,
		                    wide_buf);
		const size_t available_bytes = 64 - offset_within_block;
		const size_t bytes = out_len > available_bytes ? available_bytes : out_len;
		COPY(out, wide_buf + offset_within_block, bytes);
		out += bytes;
		out_len -= bytes;
		output_block_counter += 1;
	}
	if (out_len / 64) {
		blake3_xof_many(self->input_cv, self->block, self->block_len,
		               output_block_counter, self->flags | BLAKE3_ROOT,
		               out, out_len / 64);
	}
	output_block_counter += out_len / 64;
	out += out_len & (size_t)-64;
	out_len -= out_len & (size_t)-64;
	if (out_len) {
		blake3_compress_xof(self->input_cv, self->block, self->block_len,
		                    output_block_counter, self->flags | BLAKE3_ROOT,
		                    wide_buf);
		COPY(out, wide_buf, out_len);
	}
}

static void blake3_chunk_state_update(crypto_blake3_ctx *self,
                                      const u8 *input, size_t input_len)
{
	if (self->chunk.buf_len > 0) {
		size_t take = blake3_chunk_state_fill_buf(self, input, input_len);
		input += take;
		input_len -= take;
		if (input_len > 0) {
			blake3_compress_in_place(self->chunk.cv, self->chunk.buf,
			                         BLAKE3_BLOCK_LEN, self->chunk.chunk_counter,
			                         self->chunk.flags
			                         | blake3_chunk_state_maybe_start_flag(self));
			self->chunk.blocks_compressed += 1;
			self->chunk.buf_len = 0;
			ZERO(self->chunk.buf, BLAKE3_BLOCK_LEN);
		}
	}

	while (input_len > BLAKE3_BLOCK_LEN) {
		blake3_compress_in_place(self->chunk.cv, input, BLAKE3_BLOCK_LEN,
		                         self->chunk.chunk_counter,
		                         self->chunk.flags
		                         | blake3_chunk_state_maybe_start_flag(self));
		self->chunk.blocks_compressed += 1;
		input += BLAKE3_BLOCK_LEN;
		input_len -= BLAKE3_BLOCK_LEN;
	}

	blake3_chunk_state_fill_buf(self, input, input_len);
}

static blake3_output_t blake3_chunk_state_output(const crypto_blake3_ctx *self)
{
	u8 block_flags = self->chunk.flags
	              | blake3_chunk_state_maybe_start_flag(self)
	              | BLAKE3_CHUNK_END;
	return blake3_make_output(self->chunk.cv, self->chunk.buf,
	                          self->chunk.buf_len, self->chunk.chunk_counter,
	                          block_flags);
}

static blake3_output_t blake3_parent_output(const u8 block[BLAKE3_BLOCK_LEN],
                                            const u32 key[8], u8 flags)
{
	return blake3_make_output(key, block, BLAKE3_BLOCK_LEN, 0,
	                          flags | BLAKE3_PARENT);
}

static size_t blake3_left_subtree_len(size_t input_len)
{
	size_t full_chunks = (input_len - 1) / BLAKE3_CHUNK_LEN;
	return (size_t)blake3_round_down_to_power_of_2(full_chunks) * BLAKE3_CHUNK_LEN;
}

static size_t blake3_compress_chunks_parallel(const u8 *input, size_t input_len,
                                              const u32 key[8],
                                              u64 chunk_counter, u8 flags,
                                              u8 *out)
{
	const u8 *chunks_array[BLAKE3_MAX_SIMD_DEGREE];
	size_t input_position = 0;
	size_t chunks_array_len = 0;
	while (input_len - input_position >= BLAKE3_CHUNK_LEN) {
		chunks_array[chunks_array_len] = &input[input_position];
		input_position += BLAKE3_CHUNK_LEN;
		chunks_array_len += 1;
	}

	blake3_hash_many(chunks_array, chunks_array_len,
	                 BLAKE3_CHUNK_LEN / BLAKE3_BLOCK_LEN, key, chunk_counter,
	                 1, flags, BLAKE3_CHUNK_START, BLAKE3_CHUNK_END, out);

	if (input_len > input_position) {
		u64 counter = chunk_counter + (u64)chunks_array_len;
		crypto_blake3_ctx tmp;
		blake3_chunk_state_init(&tmp, key, flags);
		tmp.chunk.chunk_counter = counter;
		blake3_chunk_state_update(&tmp, &input[input_position],
		                          input_len - input_position);
		blake3_output_t output = blake3_chunk_state_output(&tmp);
		blake3_output_chaining_value(&output,
		                             &out[chunks_array_len * BLAKE3_OUT_LEN]);
		return chunks_array_len + 1;
	}
	return chunks_array_len;
}

static size_t blake3_compress_parents_parallel(const u8 *child_cvs,
                                               size_t num_chaining_values,
                                               const u32 key[8], u8 flags,
                                               u8 *out)
{
	const u8 *parents_array[BLAKE3_MAX_SIMD_DEGREE_OR_2];
	size_t parents_array_len = 0;
	while (num_chaining_values - (2 * parents_array_len) >= 2) {
		parents_array[parents_array_len] =
			&child_cvs[2 * parents_array_len * BLAKE3_OUT_LEN];
		parents_array_len += 1;
	}

	blake3_hash_many(parents_array, parents_array_len, 1, key,
	                 0, 0, flags | BLAKE3_PARENT, 0, 0, out);

	if (num_chaining_values > 2 * parents_array_len) {
		COPY(&out[parents_array_len * BLAKE3_OUT_LEN],
		     &child_cvs[2 * parents_array_len * BLAKE3_OUT_LEN],
		     BLAKE3_OUT_LEN);
		return parents_array_len + 1;
	}
	return parents_array_len;
}

#if defined(MONOCYPHER_BLAKE3_PTHREADS) && MONOCYPHER_BLAKE3_PTHREADS && \
    !defined(_WIN32)
typedef struct {
	const u8 *input;
	size_t input_len;
	const u32 *key;
	u64 chunk_counter;
	u8 flags;
	u8 *out;
	size_t out_n;
} blake3_subtree_job;

static size_t blake3_compress_subtree_wide_inner(const u8 *input,
                                                 size_t input_len,
                                                 const u32 key[8],
                                                 u64 chunk_counter, u8 flags,
                                                 u8 *out, int allow_threads);

static void *blake3_subtree_thread_main(void *ptr)
{
	blake3_subtree_job *job = (blake3_subtree_job*)ptr;
	job->out_n = blake3_compress_subtree_wide_inner(job->input,
	                                               job->input_len,
	                                               job->key,
	                                               job->chunk_counter,
	                                               job->flags,
	                                               job->out, 0);
	return 0;
}
#endif

static size_t blake3_compress_subtree_wide_inner(const u8 *input,
                                                 size_t input_len,
                                                 const u32 key[8],
                                                 u64 chunk_counter, u8 flags,
                                                 u8 *out, int allow_threads)
{
#if !defined(MONOCYPHER_BLAKE3_PTHREADS) || !MONOCYPHER_BLAKE3_PTHREADS || \
    defined(_WIN32)
	(void)allow_threads;
#endif
	if (input_len <= blake3_simd_degree() * BLAKE3_CHUNK_LEN) {
		return blake3_compress_chunks_parallel(input, input_len, key,
		                                       chunk_counter, flags, out);
	}

	size_t left_input_len = blake3_left_subtree_len(input_len);
	size_t right_input_len = input_len - left_input_len;
	const u8 *right_input = &input[left_input_len];
	u64 right_chunk_counter =
		chunk_counter + (u64)(left_input_len / BLAKE3_CHUNK_LEN);

	u8 cv_array[2 * BLAKE3_MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN];
	size_t degree = blake3_simd_degree();
	if (left_input_len > BLAKE3_CHUNK_LEN && degree == 1) {
		degree = 2;
	}
	u8 *right_cvs = &cv_array[degree * BLAKE3_OUT_LEN];

	size_t left_n = 0;
	size_t right_n = 0;
#if defined(MONOCYPHER_BLAKE3_PTHREADS) && MONOCYPHER_BLAKE3_PTHREADS && \
    !defined(_WIN32)
	if (allow_threads &&
	    input_len >= BLAKE3_THREAD_MIN_LEN &&
	    left_input_len >= BLAKE3_CHUNK_LEN &&
	    right_input_len >= BLAKE3_CHUNK_LEN) {
		blake3_subtree_job job;
		job.input = input;
		job.input_len = left_input_len;
		job.key = key;
		job.chunk_counter = chunk_counter;
		job.flags = flags;
		job.out = cv_array;
		job.out_n = 0;
		pthread_t thread;
		if (pthread_create(&thread, 0, blake3_subtree_thread_main, &job) == 0) {
			right_n = blake3_compress_subtree_wide_inner(right_input,
			                                             right_input_len,
			                                             key,
			                                             right_chunk_counter,
			                                             flags, right_cvs, 0);
			pthread_join(thread, 0);
			left_n = job.out_n;
		} else {
			left_n = blake3_compress_subtree_wide_inner(input, left_input_len,
			                                            key, chunk_counter,
			                                            flags, cv_array, 0);
			right_n = blake3_compress_subtree_wide_inner(right_input,
			                                             right_input_len,
			                                             key,
			                                             right_chunk_counter,
			                                             flags, right_cvs, 0);
		}
	} else
#endif
	{
		left_n = blake3_compress_subtree_wide_inner(input, left_input_len, key,
		                                            chunk_counter, flags,
		                                            cv_array, 0);
		right_n = blake3_compress_subtree_wide_inner(right_input,
		                                             right_input_len, key,
		                                             right_chunk_counter,
		                                             flags, right_cvs, 0);
	}

	if (left_n == 1) {
		COPY(out, cv_array, 2 * BLAKE3_OUT_LEN);
		return 2;
	}

	size_t num_chaining_values = left_n + right_n;
	return blake3_compress_parents_parallel(cv_array, num_chaining_values, key,
	                                        flags, out);
}

static size_t blake3_compress_subtree_wide(const u8 *input, size_t input_len,
                                           const u32 key[8],
                                           u64 chunk_counter, u8 flags,
                                           u8 *out)
{
	return blake3_compress_subtree_wide_inner(input, input_len, key,
	                                          chunk_counter, flags, out, 1);
}

static void blake3_compress_subtree_to_parent_node(const u8 *input,
                                                   size_t input_len,
                                                   const u32 key[8],
                                                   u64 chunk_counter, u8 flags,
                                                   u8 out[2 * BLAKE3_OUT_LEN])
{
	u8 cv_array[BLAKE3_MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN];
	size_t num_cvs = blake3_compress_subtree_wide(input, input_len, key,
	                                             chunk_counter, flags, cv_array);
	while (num_cvs > 2) {
		u8 out_array[BLAKE3_MAX_SIMD_DEGREE_OR_2 * BLAKE3_OUT_LEN / 2];
		num_cvs = blake3_compress_parents_parallel(cv_array, num_cvs, key,
		                                           flags, out_array);
		COPY(cv_array, out_array, num_cvs * BLAKE3_OUT_LEN);
	}
	COPY(out, cv_array, 2 * BLAKE3_OUT_LEN);
}

static u32 blake3_popcnt(u64 x)
{
	u32 count = 0;
	while (x != 0) {
		count += 1;
		x &= x - 1;
	}
	return count;
}

static void blake3_merge_cv_stack(crypto_blake3_ctx *self, u64 total_len)
{
	size_t post_merge_stack_len = (size_t)blake3_popcnt(total_len);
	while (self->cv_stack_len > post_merge_stack_len) {
		u8 *parent_node =
			&self->cv_stack[(self->cv_stack_len - 2) * BLAKE3_OUT_LEN];
		blake3_output_t output = blake3_parent_output(parent_node,
		                                              self->key,
		                                              self->chunk.flags);
		blake3_output_chaining_value(&output, parent_node);
		self->cv_stack_len -= 1;
	}
}

static void blake3_push_cv(crypto_blake3_ctx *self, u8 new_cv[BLAKE3_OUT_LEN],
                           u64 chunk_counter)
{
	blake3_merge_cv_stack(self, chunk_counter);
	COPY(&self->cv_stack[self->cv_stack_len * BLAKE3_OUT_LEN], new_cv,
	     BLAKE3_OUT_LEN);
	self->cv_stack_len += 1;
}

static void blake3_init_base(crypto_blake3_ctx *self, const u32 key[8],
                             u8 flags)
{
	COPY(self->key, key, 8);
	blake3_chunk_state_init(self, key, flags);
	self->cv_stack_len = 0;
}

void crypto_blake3_init(crypto_blake3_ctx *ctx)
{
	blake3_init_base(ctx, blake3_iv, 0);
}

void crypto_blake3_keyed_init(crypto_blake3_ctx *ctx,
                              const u8 key[BLAKE3_KEY_LEN])
{
	u32 key_words[8];
	blake3_load_key_words(key, key_words);
	blake3_init_base(ctx, key_words, BLAKE3_KEYED_HASH);
}

void crypto_blake3_derive_key_init(crypto_blake3_ctx *ctx,
                                   const u8 *context, size_t context_size)
{
	crypto_blake3_ctx context_hasher;
	blake3_init_base(&context_hasher, blake3_iv, BLAKE3_DERIVE_KEY_CONTEXT);
	crypto_blake3_update(&context_hasher, context, context_size);
	u8 context_key[BLAKE3_KEY_LEN];
	crypto_blake3_final(&context_hasher, context_key, BLAKE3_KEY_LEN);
	u32 context_key_words[8];
	blake3_load_key_words(context_key, context_key_words);
	blake3_init_base(ctx, context_key_words, BLAKE3_DERIVE_KEY_MATERIAL);
	WIPE_BUFFER(context_key);
	WIPE_CTX(&context_hasher);
}

void crypto_blake3_update(crypto_blake3_ctx *self,
                          const u8 *input, size_t input_len)
{
	if (input_len == 0) {
		return;
	}

	const u8 *input_bytes = input;
	if (blake3_chunk_state_len(self) > 0) {
		size_t take = BLAKE3_CHUNK_LEN - blake3_chunk_state_len(self);
		if (take > input_len) {
			take = input_len;
		}
		blake3_chunk_state_update(self, input_bytes, take);
		input_bytes += take;
		input_len -= take;
		if (input_len > 0) {
			blake3_output_t output = blake3_chunk_state_output(self);
			u8 chunk_cv[32];
			blake3_output_chaining_value(&output, chunk_cv);
			blake3_push_cv(self, chunk_cv, self->chunk.chunk_counter);
			blake3_chunk_state_reset(self, self->key,
			                         self->chunk.chunk_counter + 1);
		} else {
			return;
		}
	}

	while (input_len > BLAKE3_CHUNK_LEN) {
		size_t subtree_len = (size_t)blake3_round_down_to_power_of_2(input_len);
		u64 count_so_far = self->chunk.chunk_counter * BLAKE3_CHUNK_LEN;
		while ((((u64)(subtree_len - 1)) & count_so_far) != 0) {
			subtree_len /= 2;
		}
		u64 subtree_chunks = subtree_len / BLAKE3_CHUNK_LEN;
		if (subtree_len <= BLAKE3_CHUNK_LEN) {
			crypto_blake3_ctx chunk_state;
			blake3_chunk_state_init(&chunk_state, self->key, self->chunk.flags);
			chunk_state.chunk.chunk_counter = self->chunk.chunk_counter;
			blake3_chunk_state_update(&chunk_state, input_bytes, subtree_len);
			blake3_output_t output = blake3_chunk_state_output(&chunk_state);
			u8 cv[BLAKE3_OUT_LEN];
			blake3_output_chaining_value(&output, cv);
			blake3_push_cv(self, cv, chunk_state.chunk.chunk_counter);
		} else {
			u8 cv_pair[2 * BLAKE3_OUT_LEN];
			blake3_compress_subtree_to_parent_node(input_bytes, subtree_len,
			                                       self->key,
			                                       self->chunk.chunk_counter,
			                                       self->chunk.flags, cv_pair);
			blake3_push_cv(self, cv_pair, self->chunk.chunk_counter);
			blake3_push_cv(self, &cv_pair[BLAKE3_OUT_LEN],
			               self->chunk.chunk_counter + (subtree_chunks / 2));
		}
		self->chunk.chunk_counter += subtree_chunks;
		input_bytes += subtree_len;
		input_len -= subtree_len;
	}

	if (input_len > 0) {
		blake3_chunk_state_update(self, input_bytes, input_len);
		blake3_merge_cv_stack(self, self->chunk.chunk_counter);
	}
}

void crypto_blake3_final_seek(const crypto_blake3_ctx *self, u64 seek,
                              u8 *out, size_t out_len)
{
	if (out_len == 0) {
		return;
	}

	if (self->cv_stack_len == 0) {
		blake3_output_t output = blake3_chunk_state_output(self);
		blake3_output_root_bytes(&output, seek, out, out_len);
		return;
	}

	blake3_output_t output;
	size_t cvs_remaining;
	if (blake3_chunk_state_len(self) > 0) {
		cvs_remaining = self->cv_stack_len;
		output = blake3_chunk_state_output(self);
	} else {
		cvs_remaining = self->cv_stack_len - 2;
		output = blake3_parent_output(&self->cv_stack[cvs_remaining * 32],
		                              self->key, self->chunk.flags);
	}
	while (cvs_remaining > 0) {
		cvs_remaining -= 1;
		u8 parent_block[BLAKE3_BLOCK_LEN];
		COPY(parent_block, &self->cv_stack[cvs_remaining * 32], 32);
		blake3_output_chaining_value(&output, &parent_block[32]);
		output = blake3_parent_output(parent_block, self->key,
		                              self->chunk.flags);
	}
	blake3_output_root_bytes(&output, seek, out, out_len);
}

void crypto_blake3_final(crypto_blake3_ctx *ctx, u8 *out, size_t out_len)
{
	crypto_blake3_final_seek(ctx, 0, out, out_len);
	WIPE_CTX(ctx);
}

int crypto_blake3_init_checked(crypto_blake3_ctx *ctx)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_init(ctx);
	return CRYPTO_OK;
}

int crypto_blake3_keyed_init_checked(crypto_blake3_ctx *ctx,
                                     const u8 key[BLAKE3_KEY_LEN])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, BLAKE3_KEY_LEN);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_keyed_init(ctx, key);
	return CRYPTO_OK;
}

int crypto_blake3_derive_key_init_checked(crypto_blake3_ctx *ctx,
                                          const u8 *context,
                                          size_t context_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(context, context_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_derive_key_init(ctx, context, context_size);
	return CRYPTO_OK;
}

int crypto_blake3_update_checked(crypto_blake3_ctx *ctx,
                                 const u8 *input, size_t input_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(input, input_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_update(ctx, input, input_size);
	return CRYPTO_OK;
}

int crypto_blake3_final_seek_checked(const crypto_blake3_ctx *ctx, u64 seek,
                                     u8 *out, size_t out_len)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(out, out_len);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_final_seek(ctx, seek, out, out_len);
	return CRYPTO_OK;
}

int crypto_blake3_final_checked(crypto_blake3_ctx *ctx,
                                u8 *out, size_t out_len)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(out, out_len);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_final(ctx, out, out_len);
	return CRYPTO_OK;
}

void crypto_blake3(u8 *out, size_t out_len,
                   const u8 *message, size_t message_size)
{
	crypto_blake3_ctx ctx;
	crypto_blake3_init(&ctx);
	crypto_blake3_update(&ctx, message, message_size);
	crypto_blake3_final(&ctx, out, out_len);
}

void crypto_blake3_keyed(u8 *out, size_t out_len,
                         const u8 key[BLAKE3_KEY_LEN],
                         const u8 *message, size_t message_size)
{
	crypto_blake3_ctx ctx;
	crypto_blake3_keyed_init(&ctx, key);
	crypto_blake3_update(&ctx, message, message_size);
	crypto_blake3_final(&ctx, out, out_len);
}

void crypto_blake3_derive_key(u8 *out, size_t out_len,
                              const u8 *context, size_t context_size,
                              const u8 *message, size_t message_size)
{
	crypto_blake3_ctx ctx;
	crypto_blake3_derive_key_init(&ctx, context, context_size);
	crypto_blake3_update(&ctx, message, message_size);
	crypto_blake3_final(&ctx, out, out_len);
}

int crypto_blake3_checked(u8 *out, size_t out_len,
                          const u8 *message, size_t message_size)
{
	int err = check_out_ptr(out);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3(out, out_len, message, message_size);
	return CRYPTO_OK;
}

int crypto_blake3_keyed_checked(u8 *out, size_t out_len,
                                const u8 key[BLAKE3_KEY_LEN],
                                const u8 *message, size_t message_size)
{
	int err = check_out_ptr(out);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, BLAKE3_KEY_LEN);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_keyed(out, out_len, key, message, message_size);
	return CRYPTO_OK;
}

int crypto_blake3_derive_key_checked(u8 *out, size_t out_len,
                                     const u8 *context, size_t context_size,
                                     const u8 *message, size_t message_size)
{
	int err = check_out_ptr(out);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(context, context_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_blake3_derive_key(out, out_len, context, context_size,
	                         message, message_size);
	return CRYPTO_OK;
}


//////////////
/// Argon2 ///
//////////////
// references to R, Z, Q etc. come from the spec

// Argon2 operates on 1024 byte blocks.
typedef struct { u64 a[128]; } blk;

// updates a BLAKE2 hash with a 32 bit word, little endian.
static void blake_update_32(crypto_blake2b_ctx *ctx, u32 input)
{
	u8 buf[4];
	store32_le(buf, input);
	crypto_blake2b_update(ctx, buf, 4);
	WIPE_BUFFER(buf);
}

static void blake_update_32_buf(crypto_blake2b_ctx *ctx,
                                const u8 *buf, u32 size)
{
	blake_update_32(ctx, size);
	crypto_blake2b_update(ctx, buf, size);
}


static void copy_block(blk *o,const blk*in){FOR(i, 0, 128) o->a[i]  = in->a[i];}
static void  xor_block(blk *o,const blk*in){FOR(i, 0, 128) o->a[i] ^= in->a[i];}

// Hash with a virtually unlimited digest size.
// Doesn't extract more entropy than the base hash function.
// Mainly used for filling a whole kilobyte block with pseudo-random bytes.
// (One could use a stream cipher with a seed hash as the key, but
//  this would introduce another dependency —and point of failure.)
static void extended_hash(u8       *digest, u32 digest_size,
                          const u8 *input , u32 input_size)
{
	crypto_blake2b_ctx ctx;
	crypto_blake2b_init  (&ctx, MIN(digest_size, 64));
	blake_update_32      (&ctx, digest_size);
	crypto_blake2b_update(&ctx, input, input_size);
	crypto_blake2b_final (&ctx, digest);

	if (digest_size > 64) {
		// the conversion to u64 avoids integer overflow on
		// ludicrously big hash sizes.
		u32 r   = (u32)(((u64)digest_size + 31) >> 5) - 2;
		u32 i   =  1;
		u32 in  =  0;
		u32 out = 32;
		while (i < r) {
			// Input and output overlap. This is intentional
			crypto_blake2b(digest + out, 64, digest + in, 64);
			i   +=  1;
			in  += 32;
			out += 32;
		}
		crypto_blake2b(digest + out, digest_size - (32 * r), digest + in , 64);
	}
}

#define LSB(x) ((u64)(u32)x)
#define G(a, b, c, d)	\
	a += b + ((LSB(a) * LSB(b)) << 1);  d ^= a;  d = rotr64(d, 32); \
	c += d + ((LSB(c) * LSB(d)) << 1);  b ^= c;  b = rotr64(b, 24); \
	a += b + ((LSB(a) * LSB(b)) << 1);  d ^= a;  d = rotr64(d, 16); \
	c += d + ((LSB(c) * LSB(d)) << 1);  b ^= c;  b = rotr64(b, 63)
#define ROUND(v0,  v1,  v2,  v3,  v4,  v5,  v6,  v7,	\
              v8,  v9, v10, v11, v12, v13, v14, v15)	\
	G(v0, v4,  v8, v12);  G(v1, v5,  v9, v13); \
	G(v2, v6, v10, v14);  G(v3, v7, v11, v15); \
	G(v0, v5, v10, v15);  G(v1, v6, v11, v12); \
	G(v2, v7,  v8, v13);  G(v3, v4,  v9, v14)

// Core of the compression function G.  Computes Z from R in place.
static void g_rounds(blk *b)
{
	// column rounds (work_block = Q)
	for (int i = 0; i < 128; i += 16) {
		ROUND(b->a[i   ], b->a[i+ 1], b->a[i+ 2], b->a[i+ 3],
		      b->a[i+ 4], b->a[i+ 5], b->a[i+ 6], b->a[i+ 7],
		      b->a[i+ 8], b->a[i+ 9], b->a[i+10], b->a[i+11],
		      b->a[i+12], b->a[i+13], b->a[i+14], b->a[i+15]);
	}
	// row rounds (b = Z)
	for (int i = 0; i < 16; i += 2) {
		ROUND(b->a[i   ], b->a[i+ 1], b->a[i+ 16], b->a[i+ 17],
		      b->a[i+32], b->a[i+33], b->a[i+ 48], b->a[i+ 49],
		      b->a[i+64], b->a[i+65], b->a[i+ 80], b->a[i+ 81],
		      b->a[i+96], b->a[i+97], b->a[i+112], b->a[i+113]);
	}
}

const crypto_argon2_extras crypto_argon2_no_extras = { 0, 0, 0, 0 };

typedef struct {
	blk *blocks;
	u32 lane_size;
	u32 segment_size;
	u32 nb_blocks;
	crypto_argon2_config config;
	u32 pass;
	u32 slice;
	int constant_time;
	u32 pass_offset;
	u32 slice_offset;
	u32 lane;
} argon2_segment_ctx;

static void argon2_fill_segment(const argon2_segment_ctx *ctx)
{
	blk index_block;
	u32 index_ctr = 1;
	blk tmp;

	FOR_T (u32, block, ctx->pass_offset, ctx->segment_size) {
		// Current and previous blocks
		u32  lane_offset   = ctx->lane * ctx->lane_size;
		blk *segment_start = ctx->blocks + lane_offset + ctx->slice_offset;
		blk *current       = segment_start + block;
		blk *previous      =
			block == 0 && ctx->slice_offset == 0
			? segment_start + ctx->lane_size - 1
			: segment_start + block - 1;

		u64 index_seed;
		if (ctx->constant_time) {
			if (block == ctx->pass_offset || (block % 128) == 0) {
				// Fill or refresh deterministic indices block

				// seed the beginning of the block...
				ZERO(index_block.a, 128);
				index_block.a[0] = ctx->pass;
				index_block.a[1] = ctx->lane;
				index_block.a[2] = ctx->slice;
				index_block.a[3] = ctx->nb_blocks;
				index_block.a[4] = ctx->config.nb_passes;
				index_block.a[5] = ctx->config.algorithm;
				index_block.a[6] = index_ctr;
				index_ctr++;

				// ... then shuffle it
				copy_block(&tmp, &index_block);
				g_rounds  (&index_block);
				xor_block (&index_block, &tmp);
				copy_block(&tmp, &index_block);
				g_rounds  (&index_block);
				xor_block (&index_block, &tmp);
			}
			index_seed = index_block.a[block % 128];
		} else {
			index_seed = previous->a[0];
		}

		// Establish the reference set.  *Approximately* comprises:
		// - The last 3 slices (if they exist yet)
		// - The already constructed blocks in the current segment
		u32 next_slice   = ((ctx->slice + 1) % 4) * ctx->segment_size;
		u32 window_start = ctx->pass == 0 ? 0     : next_slice;
		u32 nb_segments  = ctx->pass == 0 ? ctx->slice : 3;
		u32 ref_lane     =
			ctx->pass == 0 && ctx->slice == 0
			? ctx->lane
			: (index_seed >> 32) % ctx->config.nb_lanes;
		u32 window_size  =
			nb_segments * ctx->segment_size +
			(ref_lane == ctx->lane ? block - 1 :
			 block == 0          ? (u32)-1 : 0);

		// Find reference block
		u64  j1        = index_seed & 0xffffffff; // block selector
		u64  x         = (j1 * j1)         >> 32;
		u64  y         = (window_size * x) >> 32;
		u64  z         = (window_size - 1) - y;
		u32  ref       = (window_start + z) % ctx->lane_size;
		u32  index     = ref_lane * ctx->lane_size + ref;
		blk *reference = ctx->blocks + index;

		// Shuffle the previous & reference block into the current block
		copy_block(&tmp, previous);
		xor_block (&tmp, reference);
		if (ctx->pass == 0) { copy_block(current, &tmp); }
		else                { xor_block (current, &tmp); }
		g_rounds  (&tmp);
		xor_block (current, &tmp);
	}

	volatile u64 *p = tmp.a;
	ZERO(p, 128);
}

#if defined(MONOCYPHER_ARGON2_PTHREADS) && MONOCYPHER_ARGON2_PTHREADS && !defined(_WIN32)
static void *argon2_thread_main(void *ptr)
{
	argon2_fill_segment((const argon2_segment_ctx*)ptr);
	return 0;
}
#endif

void crypto_argon2(u8 *hash, u32 hash_size, void *work_area,
                   crypto_argon2_config config,
                   crypto_argon2_inputs inputs,
                   crypto_argon2_extras extras)
{
	const u32 segment_size = config.nb_blocks / config.nb_lanes / 4;
	const u32 lane_size    = segment_size * 4;
	const u32 nb_blocks    = lane_size * config.nb_lanes; // rounding down

	// work area seen as blocks (must be suitably aligned)
	blk *blocks = (blk*)work_area;
	{
		u8 initial_hash[72]; // 64 bytes plus 2 words for future hashes
		crypto_blake2b_ctx ctx;
		crypto_blake2b_init (&ctx, 64);
		blake_update_32     (&ctx, config.nb_lanes ); // p: number of "threads"
		blake_update_32     (&ctx, hash_size);
		blake_update_32     (&ctx, config.nb_blocks);
		blake_update_32     (&ctx, config.nb_passes);
		blake_update_32     (&ctx, 0x13);             // v: version number
		blake_update_32     (&ctx, config.algorithm); // y: Argon2i, Argon2d...
		blake_update_32_buf (&ctx, inputs.pass, inputs.pass_size);
		blake_update_32_buf (&ctx, inputs.salt, inputs.salt_size);
		blake_update_32_buf (&ctx, extras.key,  extras.key_size);
		blake_update_32_buf (&ctx, extras.ad,   extras.ad_size);
		crypto_blake2b_final(&ctx, initial_hash); // fill 64 first bytes only

		// fill first 2 blocks of each lane
		u8 hash_area[1024];
		FOR_T(u32, l, 0, config.nb_lanes) {
			FOR_T(u32, i, 0, 2) {
				store32_le(initial_hash + 64, i); // first  additional word
				store32_le(initial_hash + 68, l); // second additional word
				extended_hash(hash_area, 1024, initial_hash, 72);
				load64_le_buf(blocks[l * lane_size + i].a, hash_area, 128);
			}
		}

		WIPE_BUFFER(initial_hash);
		WIPE_BUFFER(hash_area);
	}

	// Argon2i and Argon2id start with constant time indexing
	int constant_time = config.algorithm != CRYPTO_ARGON2_D;

	// Fill (and re-fill) the rest of the blocks
	//
#if defined(MONOCYPHER_ARGON2_PTHREADS) && MONOCYPHER_ARGON2_PTHREADS && !defined(_WIN32)
	pthread_t *threads = 0;
	argon2_segment_ctx *jobs = 0;
	int have_threads = 0;
	if (config.nb_lanes > 1) {
		threads = (pthread_t*)malloc((size_t)config.nb_lanes * sizeof(*threads));
		jobs = (argon2_segment_ctx*)malloc((size_t)config.nb_lanes * sizeof(*jobs));
		if (threads != 0 && jobs != 0) {
			have_threads = 1;
		}
	}
#endif

	FOR_T(u32, pass, 0, config.nb_passes) {
		FOR_T(u32, slice, 0, 4) {
			// On the first slice of the first pass,
			// blocks 0 and 1 are already filled, hence pass_offset.
			u32 pass_offset  = pass == 0 && slice == 0 ? 2 : 0;
			u32 slice_offset = slice * segment_size;

			// Argon2id switches back to non-constant time indexing
			// after the first two slices of the first pass
			if (slice == 2 && config.algorithm == CRYPTO_ARGON2_ID) {
				constant_time = 0;
			}

			// Each lane segment in this slice may be computed in parallel.
			argon2_segment_ctx base;
			base.blocks        = blocks;
			base.lane_size     = lane_size;
			base.segment_size  = segment_size;
			base.nb_blocks     = nb_blocks;
			base.config        = config;
			base.pass          = pass;
			base.slice         = slice;
			base.constant_time = constant_time;
			base.pass_offset   = pass_offset;
			base.slice_offset  = slice_offset;

#if defined(MONOCYPHER_ARGON2_PTHREADS) && MONOCYPHER_ARGON2_PTHREADS && !defined(_WIN32)
			if (have_threads) {
				u32 created = 0;
				u32 lane = 0;
				int failed = 0;
				for (; lane < config.nb_lanes; lane++) {
					jobs[lane] = base;
					jobs[lane].lane = lane;
					if (pthread_create(&threads[lane], 0,
					                   argon2_thread_main, &jobs[lane]) != 0) {
						failed = 1;
						break;
					}
					created++;
				}
				if (failed) {
					jobs[lane] = base;
					jobs[lane].lane = lane;
					argon2_fill_segment(&jobs[lane]);
					for (u32 l = lane + 1; l < config.nb_lanes; l++) {
						jobs[l] = base;
						jobs[l].lane = l;
						argon2_fill_segment(&jobs[l]);
					}
				}
				for (u32 i = 0; i < created; i++) {
					pthread_join(threads[i], 0);
				}
			} else
#endif
			{
				FOR_T(u32, lane, 0, config.nb_lanes) {
					argon2_segment_ctx ctx = base;
					ctx.lane = lane;
					argon2_fill_segment(&ctx);
				}
			}
		}
	}

	// XOR last blocks of each lane
	blk *last_block = blocks + lane_size - 1;
	FOR_T (u32, lane, 1, config.nb_lanes) {
		blk *next_block = last_block + lane_size;
		xor_block(next_block, last_block);
		last_block = next_block;
	}

	// Serialize last block
	u8 final_block[1024];
	store64_le_buf(final_block, last_block->a, 128);

	// Wipe work area
	volatile u64* p = (u64*)work_area;
	ZERO(p, 128 * nb_blocks);

	// Hash the very last block with H' into the output hash
	extended_hash(hash, hash_size, final_block, 1024);
	WIPE_BUFFER(final_block);

#if defined(MONOCYPHER_ARGON2_PTHREADS) && MONOCYPHER_ARGON2_PTHREADS && !defined(_WIN32)
	if (threads != 0) { free(threads); }
	if (jobs != 0) { free(jobs); }
#endif
}

int crypto_argon2_checked(u8 *hash, u32 hash_size,
                          void *work_area, size_t work_area_size,
                          crypto_argon2_config config,
                          crypto_argon2_inputs inputs,
                          crypto_argon2_extras extras)
{
	int err = check_out_ptr(hash);
	if (err != CRYPTO_OK) { return err; }
	if (hash_size == 0) { return CRYPTO_ERR_SIZE; }
	if (config.algorithm != CRYPTO_ARGON2_D &&
	    config.algorithm != CRYPTO_ARGON2_I &&
	    config.algorithm != CRYPTO_ARGON2_ID) {
		return CRYPTO_ERR_CONFIG;
	}
	if (config.nb_lanes == 0 || config.nb_passes == 0) {
		return CRYPTO_ERR_CONFIG;
	}
	if (config.nb_blocks < 8 * config.nb_lanes) {
		return CRYPTO_ERR_CONFIG;
	}
	if (work_area == 0) {
		return CRYPTO_ERR_NULL;
	}
	size_t needed = 0;
	err = checked_mul_size((size_t)config.nb_blocks, 1024, &needed);
	if (err != CRYPTO_OK) { return err; }
	if (work_area_size < needed) { return CRYPTO_ERR_SIZE; }
	err = check_ptr(inputs.pass, inputs.pass_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(inputs.salt, inputs.salt_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(extras.key, extras.key_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(extras.ad, extras.ad_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_argon2(hash, hash_size, work_area, config, inputs, extras);
	return CRYPTO_OK;
}


////////////////////////////////////
/// Arithmetic modulo 2^255 - 19 ///
////////////////////////////////////
//  Originally taken from SUPERCOP's ref10 implementation.
//  A bit bigger than TweetNaCl, over 4 times faster.

// field element
typedef i32 fe[10];

// field constants
//
// fe_one      : 1
// sqrtm1      : sqrt(-1)
// d           :     -121665 / 121666
// D2          : 2 * -121665 / 121666
// lop_x, lop_y: low order point in Edwards coordinates
// ufactor     : -sqrt(-1) * 2
// A2          : 486662^2  (A squared)
static const fe fe_one  = {1};
static const fe sqrtm1  = {
	-32595792, -7943725, 9377950, 3500415, 12389472,
	-272473, -25146209, -2005654, 326686, 11406482,
};
static const fe d       = {
	-10913610, 13857413, -15372611, 6949391, 114729,
	-8787816, -6275908, -3247719, -18696448, -12055116,
};
static const fe D2      = {
	-21827239, -5839606, -30745221, 13898782, 229458,
	15978800, -12551817, -6495438, 29715968, 9444199,
};
static const fe lop_x   = {
	21352778, 5345713, 4660180, -8347857, 24143090,
	14568123, 30185756, -12247770, -33528939, 8345319,
};
static const fe lop_y   = {
	-6952922, -1265500, 6862341, -7057498, -4037696,
	-5447722, 31680899, -15325402, -19365852, 1569102,
};
static const fe ufactor = {
	-1917299, 15887451, -18755900, -7000830, -24778944,
	544946, -16816446, 4011309, -653372, 10741468,
};
static const fe A2      = {
	12721188, 3529, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void fe_0(fe h) {           ZERO(h  , 10); }
static void fe_1(fe h) { h[0] = 1; ZERO(h+1,  9); }

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static void fe_copy_avx2(fe h, const fe f)
{
	int i = 0;
	for (; i + 7 < 10; i += 8) {
		__m256i v = _mm256_loadu_si256((const __m256i*)&f[i]);
		_mm256_storeu_si256((__m256i*)&h[i], v);
	}
	for (; i < 10; i++) {
		h[i] = f[i];
	}
}

MONO_TARGET_AVX2
static void fe_neg_avx2(fe h, const fe f)
{
	const __m256i zero = _mm256_setzero_si256();
	int i = 0;
	for (; i + 7 < 10; i += 8) {
		__m256i v = _mm256_loadu_si256((const __m256i*)&f[i]);
		v = _mm256_sub_epi32(zero, v);
		_mm256_storeu_si256((__m256i*)&h[i], v);
	}
	for (; i < 10; i++) {
		h[i] = -f[i];
	}
}

MONO_TARGET_AVX2
static void fe_add_avx2(fe h, const fe f, const fe g)
{
	int i = 0;
	for (; i + 7 < 10; i += 8) {
		__m256i vf = _mm256_loadu_si256((const __m256i*)&f[i]);
		__m256i vg = _mm256_loadu_si256((const __m256i*)&g[i]);
		__m256i v = _mm256_add_epi32(vf, vg);
		_mm256_storeu_si256((__m256i*)&h[i], v);
	}
	for (; i < 10; i++) {
		h[i] = f[i] + g[i];
	}
}

MONO_TARGET_AVX2
static void fe_sub_avx2(fe h, const fe f, const fe g)
{
	int i = 0;
	for (; i + 7 < 10; i += 8) {
		__m256i vf = _mm256_loadu_si256((const __m256i*)&f[i]);
		__m256i vg = _mm256_loadu_si256((const __m256i*)&g[i]);
		__m256i v = _mm256_sub_epi32(vf, vg);
		_mm256_storeu_si256((__m256i*)&h[i], v);
	}
	for (; i < 10; i++) {
		h[i] = f[i] - g[i];
	}
}
#endif

static void fe_copy(fe h, const fe f)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_copy_avx2(h, f);
		return;
	}
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			__m128i v = _mm_loadu_si128((const __m128i*)&f[i]);
			_mm_storeu_si128((__m128i*)&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = f[i];
		}
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			int32x4_t v = vld1q_s32(&f[i]);
			vst1q_s32(&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = f[i];
		}
		return;
	}
#endif
	FOR(i, 0, 10) { h[i] = f[i]; }
}

static void fe_neg(fe h, const fe f)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_neg_avx2(h, f);
		return;
	}
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		const __m128i zero = _mm_setzero_si128();
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			__m128i v = _mm_loadu_si128((const __m128i*)&f[i]);
			v = _mm_sub_epi32(zero, v);
			_mm_storeu_si128((__m128i*)&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = -f[i];
		}
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			int32x4_t v = vld1q_s32(&f[i]);
			v = vnegq_s32(v);
			vst1q_s32(&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = -f[i];
		}
		return;
	}
#endif
	FOR(i, 0, 10) { h[i] = -f[i]; }
}

static void fe_add(fe h, const fe f, const fe g)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_add_avx2(h, f, g);
		return;
	}
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			__m128i vf = _mm_loadu_si128((const __m128i*)&f[i]);
			__m128i vg = _mm_loadu_si128((const __m128i*)&g[i]);
			__m128i v = _mm_add_epi32(vf, vg);
			_mm_storeu_si128((__m128i*)&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = f[i] + g[i];
		}
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			int32x4_t vf = vld1q_s32(&f[i]);
			int32x4_t vg = vld1q_s32(&g[i]);
			int32x4_t v = vaddq_s32(vf, vg);
			vst1q_s32(&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = f[i] + g[i];
		}
		return;
	}
#endif
	FOR(i, 0, 10) { h[i] = f[i] + g[i]; }
}

static void fe_sub(fe h, const fe f, const fe g)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_sub_avx2(h, f, g);
		return;
	}
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			__m128i vf = _mm_loadu_si128((const __m128i*)&f[i]);
			__m128i vg = _mm_loadu_si128((const __m128i*)&g[i]);
			__m128i v = _mm_sub_epi32(vf, vg);
			_mm_storeu_si128((__m128i*)&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = f[i] - g[i];
		}
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			int32x4_t vf = vld1q_s32(&f[i]);
			int32x4_t vg = vld1q_s32(&g[i]);
			int32x4_t v = vsubq_s32(vf, vg);
			vst1q_s32(&h[i], v);
		}
		for (; i < 10; i++) {
			h[i] = f[i] - g[i];
		}
		return;
	}
#endif
	FOR(i, 0, 10) { h[i] = f[i] - g[i]; }
}

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static void fe_cswap_avx2(fe f, fe g, int b)
{
	i32 mask = -b;
	__m256i m = _mm256_set1_epi32(mask);
	int i = 0;
	for (; i + 7 < 10; i += 8) {
		__m256i vf = _mm256_loadu_si256((const __m256i*)&f[i]);
		__m256i vg = _mm256_loadu_si256((const __m256i*)&g[i]);
		__m256i x = _mm256_and_si256(_mm256_xor_si256(vf, vg), m);
		vf = _mm256_xor_si256(vf, x);
		vg = _mm256_xor_si256(vg, x);
		_mm256_storeu_si256((__m256i*)&f[i], vf);
		_mm256_storeu_si256((__m256i*)&g[i], vg);
	}
	for (; i < 10; i++) {
		i32 x = (f[i] ^ g[i]) & mask;
		f[i] = f[i] ^ x;
		g[i] = g[i] ^ x;
	}
}
#endif

static void fe_cswap(fe f, fe g, int b)
{
	i32 mask = -b; // -1 = 0xffffffff
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_cswap_avx2(f, g, b);
		return;
	}
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		__m128i m = _mm_set1_epi32(mask);
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			__m128i vf = _mm_loadu_si128((const __m128i*)&f[i]);
			__m128i vg = _mm_loadu_si128((const __m128i*)&g[i]);
			__m128i x = _mm_and_si128(_mm_xor_si128(vf, vg), m);
			vf = _mm_xor_si128(vf, x);
			vg = _mm_xor_si128(vg, x);
			_mm_storeu_si128((__m128i*)&f[i], vf);
			_mm_storeu_si128((__m128i*)&g[i], vg);
		}
		for (; i < 10; i++) {
			i32 x = (f[i] ^ g[i]) & mask;
			f[i] = f[i] ^ x;
			g[i] = g[i] ^ x;
		}
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		int32x4_t m = vdupq_n_s32(mask);
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			int32x4_t vf = vld1q_s32(&f[i]);
			int32x4_t vg = vld1q_s32(&g[i]);
			int32x4_t x = vandq_s32(veorq_s32(vf, vg), m);
			vf = veorq_s32(vf, x);
			vg = veorq_s32(vg, x);
			vst1q_s32(&f[i], vf);
			vst1q_s32(&g[i], vg);
		}
		for (; i < 10; i++) {
			i32 x = (f[i] ^ g[i]) & mask;
			f[i] = f[i] ^ x;
			g[i] = g[i] ^ x;
		}
		return;
	}
#endif
	FOR (i, 0, 10) {
		i32 x = (f[i] ^ g[i]) & mask;
		f[i] = f[i] ^ x;
		g[i] = g[i] ^ x;
	}
}

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static void fe_ccopy_avx2(fe f, const fe g, int b)
{
	i32 mask = -b; // -1 = 0xffffffff
	__m256i m = _mm256_set1_epi32(mask);
	int i = 0;
	for (; i + 7 < 10; i += 8) {
		__m256i vf = _mm256_loadu_si256((const __m256i*)&f[i]);
		__m256i vg = _mm256_loadu_si256((const __m256i*)&g[i]);
		__m256i x = _mm256_and_si256(_mm256_xor_si256(vf, vg), m);
		vf = _mm256_xor_si256(vf, x);
		_mm256_storeu_si256((__m256i*)&f[i], vf);
	}
	for (; i < 10; i++) {
		i32 x = (f[i] ^ g[i]) & mask;
		f[i] = f[i] ^ x;
	}
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static void fe_ccopy_neon(fe f, const fe g, int b)
{
	i32 mask = -b; // -1 = 0xffffffff
	int32x4_t m = vdupq_n_s32(mask);
	int i = 0;
	for (; i + 3 < 10; i += 4) {
		int32x4_t vf = vld1q_s32(&f[i]);
		int32x4_t vg = vld1q_s32(&g[i]);
		int32x4_t x = vandq_s32(veorq_s32(vf, vg), m);
		vf = veorq_s32(vf, x);
		vst1q_s32(&f[i], vf);
	}
	for (; i < 10; i++) {
		i32 x = (f[i] ^ g[i]) & mask;
		f[i] = f[i] ^ x;
	}
}
#endif

static void fe_ccopy(fe f, const fe g, int b)
{
	i32 mask = -b; // -1 = 0xffffffff
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_ccopy_avx2(f, g, b);
		return;
	}
#endif
#if defined(MONO_HAS_SSE2) && MONO_HAS_SSE2
	if (mono_have_sse2_cached()) {
		__m128i m = _mm_set1_epi32(mask);
		int i = 0;
		for (; i + 3 < 10; i += 4) {
			__m128i vf = _mm_loadu_si128((const __m128i*)&f[i]);
			__m128i vg = _mm_loadu_si128((const __m128i*)&g[i]);
			__m128i x = _mm_and_si128(_mm_xor_si128(vf, vg), m);
			vf = _mm_xor_si128(vf, x);
			_mm_storeu_si128((__m128i*)&f[i], vf);
		}
		for (; i < 10; i++) {
			i32 x = (f[i] ^ g[i]) & mask;
			f[i] = f[i] ^ x;
		}
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		fe_ccopy_neon(f, g, b);
		return;
	}
#endif
	FOR (i, 0, 10) {
		i32 x = (f[i] ^ g[i]) & mask;
		f[i] = f[i] ^ x;
	}
}


// Signed carry propagation
// ------------------------
//
// Let t be a number.  It can be uniquely decomposed thus:
//
//    t = h*2^26 + l
//    such that -2^25 <= l < 2^25
//
// Let c = (t + 2^25) / 2^26            (rounded down)
//     c = (h*2^26 + l + 2^25) / 2^26   (rounded down)
//     c =  h   +   (l + 2^25) / 2^26   (rounded down)
//     c =  h                           (exactly)
// Because 0 <= l + 2^25 < 2^26
//
// Let u = t          - c*2^26
//     u = h*2^26 + l - h*2^26
//     u = l
// Therefore, -2^25 <= u < 2^25
//
// Additionally, if |t| < x, then |h| < x/2^26 (rounded down)
//
// Notations:
// - In C, 1<<25 means 2^25.
// - In C, x>>25 means floor(x / (2^25)).
// - All of the above applies with 25 & 24 as well as 26 & 25.
//
//
// Note on negative right shifts
// -----------------------------
//
// In C, x >> n, where x is a negative integer, is implementation
// defined.  In practice, all platforms do arithmetic shift, which is
// equivalent to division by 2^26, rounded down.  Some compilers, like
// GCC, even guarantee it.
//
// If we ever stumble upon a platform that does not propagate the sign
// bit (we won't), visible failures will show at the slightest test, and
// the signed shifts can be replaced by the following:
//
//     typedef struct { i64 x:39; } s25;
//     typedef struct { i64 x:38; } s26;
//     i64 shift25(i64 x) { s25 s; s.x = ((u64)x)>>25; return s.x; }
//     i64 shift26(i64 x) { s26 s; s.x = ((u64)x)>>26; return s.x; }
//
// Current compilers cannot optimise this, causing a 30% drop in
// performance.  Fairly expensive for something that never happens.
//
//
// Precondition
// ------------
//
// |t0|       < 2^63
// |t1|..|t9| < 2^62
//
// Algorithm
// ---------
// c   = t0 + 2^25 / 2^26   -- |c|  <= 2^36
// t0 -= c * 2^26           -- |t0| <= 2^25
// t1 += c                  -- |t1| <= 2^63
//
// c   = t4 + 2^25 / 2^26   -- |c|  <= 2^36
// t4 -= c * 2^26           -- |t4| <= 2^25
// t5 += c                  -- |t5| <= 2^63
//
// c   = t1 + 2^24 / 2^25   -- |c|  <= 2^38
// t1 -= c * 2^25           -- |t1| <= 2^24
// t2 += c                  -- |t2| <= 2^63
//
// c   = t5 + 2^24 / 2^25   -- |c|  <= 2^38
// t5 -= c * 2^25           -- |t5| <= 2^24
// t6 += c                  -- |t6| <= 2^63
//
// c   = t2 + 2^25 / 2^26   -- |c|  <= 2^37
// t2 -= c * 2^26           -- |t2| <= 2^25        < 1.1 * 2^25  (final t2)
// t3 += c                  -- |t3| <= 2^63
//
// c   = t6 + 2^25 / 2^26   -- |c|  <= 2^37
// t6 -= c * 2^26           -- |t6| <= 2^25        < 1.1 * 2^25  (final t6)
// t7 += c                  -- |t7| <= 2^63
//
// c   = t3 + 2^24 / 2^25   -- |c|  <= 2^38
// t3 -= c * 2^25           -- |t3| <= 2^24        < 1.1 * 2^24  (final t3)
// t4 += c                  -- |t4| <= 2^25 + 2^38 < 2^39
//
// c   = t7 + 2^24 / 2^25   -- |c|  <= 2^38
// t7 -= c * 2^25           -- |t7| <= 2^24        < 1.1 * 2^24  (final t7)
// t8 += c                  -- |t8| <= 2^63
//
// c   = t4 + 2^25 / 2^26   -- |c|  <= 2^13
// t4 -= c * 2^26           -- |t4| <= 2^25        < 1.1 * 2^25  (final t4)
// t5 += c                  -- |t5| <= 2^24 + 2^13 < 1.1 * 2^24  (final t5)
//
// c   = t8 + 2^25 / 2^26   -- |c|  <= 2^37
// t8 -= c * 2^26           -- |t8| <= 2^25        < 1.1 * 2^25  (final t8)
// t9 += c                  -- |t9| <= 2^63
//
// c   = t9 + 2^24 / 2^25   -- |c|  <= 2^38
// t9 -= c * 2^25           -- |t9| <= 2^24        < 1.1 * 2^24  (final t9)
// t0 += c * 19             -- |t0| <= 2^25 + 2^38*19 < 2^44
//
// c   = t0 + 2^25 / 2^26   -- |c|  <= 2^18
// t0 -= c * 2^26           -- |t0| <= 2^25        < 1.1 * 2^25  (final t0)
// t1 += c                  -- |t1| <= 2^24 + 2^18 < 1.1 * 2^24  (final t1)
//
// Postcondition
// -------------
//   |t0|, |t2|, |t4|, |t6|, |t8|  <  1.1 * 2^25
//   |t1|, |t3|, |t5|, |t7|, |t9|  <  1.1 * 2^24
#define FE_CARRY	\
	i64 c; \
	c = (t0 + ((i64)1<<25)) >> 26;  t0 -= c * ((i64)1 << 26);  t1 += c; \
	c = (t4 + ((i64)1<<25)) >> 26;  t4 -= c * ((i64)1 << 26);  t5 += c; \
	c = (t1 + ((i64)1<<24)) >> 25;  t1 -= c * ((i64)1 << 25);  t2 += c; \
	c = (t5 + ((i64)1<<24)) >> 25;  t5 -= c * ((i64)1 << 25);  t6 += c; \
	c = (t2 + ((i64)1<<25)) >> 26;  t2 -= c * ((i64)1 << 26);  t3 += c; \
	c = (t6 + ((i64)1<<25)) >> 26;  t6 -= c * ((i64)1 << 26);  t7 += c; \
	c = (t3 + ((i64)1<<24)) >> 25;  t3 -= c * ((i64)1 << 25);  t4 += c; \
	c = (t7 + ((i64)1<<24)) >> 25;  t7 -= c * ((i64)1 << 25);  t8 += c; \
	c = (t4 + ((i64)1<<25)) >> 26;  t4 -= c * ((i64)1 << 26);  t5 += c; \
	c = (t8 + ((i64)1<<25)) >> 26;  t8 -= c * ((i64)1 << 26);  t9 += c; \
	c = (t9 + ((i64)1<<24)) >> 25;  t9 -= c * ((i64)1 << 25);  t0 += c * 19; \
	c = (t0 + ((i64)1<<25)) >> 26;  t0 -= c * ((i64)1 << 26);  t1 += c; \
	h[0]=(i32)t0;  h[1]=(i32)t1;  h[2]=(i32)t2;  h[3]=(i32)t3;  h[4]=(i32)t4; \
	h[5]=(i32)t5;  h[6]=(i32)t6;  h[7]=(i32)t7;  h[8]=(i32)t8;  h[9]=(i32)t9

// Decodes a field element from a byte buffer.
// mask specifies how many bits we ignore.
// Traditionally we ignore 1. It's useful for EdDSA,
// which uses that bit to denote the sign of x.
// Elligator however uses positive representatives,
// which means ignoring 2 bits instead.
static void fe_frombytes_mask(fe h, const u8 s[32], unsigned nb_mask)
{
	u32 mask = 0xffffff >> nb_mask;
	i64 t0 =  load32_le(s);                    // t0 < 2^32
	i64 t1 =  load24_le(s +  4) << 6;          // t1 < 2^30
	i64 t2 =  load24_le(s +  7) << 5;          // t2 < 2^29
	i64 t3 =  load24_le(s + 10) << 3;          // t3 < 2^27
	i64 t4 =  load24_le(s + 13) << 2;          // t4 < 2^26
	i64 t5 =  load32_le(s + 16);               // t5 < 2^32
	i64 t6 =  load24_le(s + 20) << 7;          // t6 < 2^31
	i64 t7 =  load24_le(s + 23) << 5;          // t7 < 2^29
	i64 t8 =  load24_le(s + 26) << 4;          // t8 < 2^28
	i64 t9 = (load24_le(s + 29) & mask) << 2;  // t9 < 2^25
	FE_CARRY;                                  // Carry precondition OK
}

static void fe_frombytes(fe h, const u8 s[32])
{
	fe_frombytes_mask(h, s, 1);
}


// Precondition
//   |h[0]|, |h[2]|, |h[4]|, |h[6]|, |h[8]|  <  1.1 * 2^25
//   |h[1]|, |h[3]|, |h[5]|, |h[7]|, |h[9]|  <  1.1 * 2^24
//
// Therefore, |h| < 2^255-19
// There are two possibilities:
//
// - If h is positive, all we need to do is reduce its individual
//   limbs down to their tight positive range.
// - If h is negative, we also need to add 2^255-19 to it.
//   Or just remove 19 and chop off any excess bit.
static void fe_tobytes(u8 s[32], const fe h)
{
	i32 t[10];
	COPY(t, h, 10);
	i32 q = (19 * t[9] + (((i32) 1) << 24)) >> 25;
	//                 |t9|                    < 1.1 * 2^24
	//  -1.1 * 2^24  <  t9                     < 1.1 * 2^24
	//  -21  * 2^24  <  19 * t9                < 21  * 2^24
	//  -2^29        <  19 * t9 + 2^24         < 2^29
	//  -2^29 / 2^25 < (19 * t9 + 2^24) / 2^25 < 2^29 / 2^25
	//  -16          < (19 * t9 + 2^24) / 2^25 < 16
	FOR (i, 0, 5) {
		q += t[2*i  ]; q >>= 26; // q = 0 or -1
		q += t[2*i+1]; q >>= 25; // q = 0 or -1
	}
	// q =  0 iff h >= 0
	// q = -1 iff h <  0
	// Adding q * 19 to h reduces h to its proper range.
	q *= 19;  // Shift carry back to the beginning
	FOR (i, 0, 5) {
		t[i*2  ] += q;  q = t[i*2  ] >> 26;  t[i*2  ] -= q * ((i32)1 << 26);
		t[i*2+1] += q;  q = t[i*2+1] >> 25;  t[i*2+1] -= q * ((i32)1 << 25);
	}
	// h is now fully reduced, and q represents the excess bit.

	store32_le(s +  0, ((u32)t[0] >>  0) | ((u32)t[1] << 26));
	store32_le(s +  4, ((u32)t[1] >>  6) | ((u32)t[2] << 19));
	store32_le(s +  8, ((u32)t[2] >> 13) | ((u32)t[3] << 13));
	store32_le(s + 12, ((u32)t[3] >> 19) | ((u32)t[4] <<  6));
	store32_le(s + 16, ((u32)t[5] >>  0) | ((u32)t[6] << 25));
	store32_le(s + 20, ((u32)t[6] >>  7) | ((u32)t[7] << 19));
	store32_le(s + 24, ((u32)t[7] >> 13) | ((u32)t[8] << 12));
	store32_le(s + 28, ((u32)t[8] >> 20) | ((u32)t[9] <<  6));

	WIPE_BUFFER(t);
}

// Precondition
// -------------
//   |f0|, |f2|, |f4|, |f6|, |f8|  <  1.65 * 2^26
//   |f1|, |f3|, |f5|, |f7|, |f9|  <  1.65 * 2^25
//
//   |g0|, |g2|, |g4|, |g6|, |g8|  <  1.65 * 2^26
//   |g1|, |g3|, |g5|, |g7|, |g9|  <  1.65 * 2^25
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static void fe_mul_small_avx2(fe h, const fe f, i32 g)
{
	__m256i vg = _mm256_set1_epi32(g);
	__m256i vf = _mm256_loadu_si256((const __m256i*)f);
	__m256i prod_even = _mm256_mul_epi32(vf, vg);
	__m256i vf_odd = _mm256_srli_si256(vf, 4);
	__m256i prod_odd = _mm256_mul_epi32(vf_odd, vg);
	__m128i even_lo = _mm256_castsi256_si128(prod_even);
	__m128i even_hi = _mm256_extracti128_si256(prod_even, 1);
	__m128i odd_lo  = _mm256_castsi256_si128(prod_odd);
	__m128i odd_hi  = _mm256_extracti128_si256(prod_odd, 1);
	i64 t0 = (i64)_mm_cvtsi128_si64(even_lo);
	i64 t2 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(even_lo, even_lo));
	i64 t4 = (i64)_mm_cvtsi128_si64(even_hi);
	i64 t6 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(even_hi, even_hi));
	i64 t1 = (i64)_mm_cvtsi128_si64(odd_lo);
	i64 t3 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(odd_lo, odd_lo));
	i64 t5 = (i64)_mm_cvtsi128_si64(odd_hi);
	i64 t7 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(odd_hi, odd_hi));
	i64 t8 = f[8] * (i64)g;
	i64 t9 = f[9] * (i64)g;
	// |t0|, |t2|, |t4|, |t6|, |t8|  <  1.65 * 2^26 * 2^31  < 2^58
	// |t1|, |t3|, |t5|, |t7|, |t9|  <  1.65 * 2^25 * 2^31  < 2^57

	FE_CARRY; // Carry precondition OK
}
#endif

#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
MONO_TARGET_SSE41
static void fe_mul_small_sse41(fe h, const fe f, i32 g)
{
	__m128i vg = _mm_set1_epi32(g);
	__m128i vf0 = _mm_loadu_si128((const __m128i*)&f[0]);
	__m128i vf1 = _mm_loadu_si128((const __m128i*)&f[4]);
	__m128i prod0 = _mm_mul_epi32(vf0, vg);
	__m128i prod1 = _mm_mul_epi32(_mm_srli_si128(vf0, 4),
	                              _mm_srli_si128(vg, 4));
	__m128i prod2 = _mm_mul_epi32(vf1, vg);
	__m128i prod3 = _mm_mul_epi32(_mm_srli_si128(vf1, 4),
	                              _mm_srli_si128(vg, 4));
	i64 t0 = (i64)_mm_cvtsi128_si64(prod0);
	i64 t2 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(prod0, prod0));
	i64 t1 = (i64)_mm_cvtsi128_si64(prod1);
	i64 t3 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(prod1, prod1));
	i64 t4 = (i64)_mm_cvtsi128_si64(prod2);
	i64 t6 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(prod2, prod2));
	i64 t5 = (i64)_mm_cvtsi128_si64(prod3);
	i64 t7 = (i64)_mm_cvtsi128_si64(_mm_unpackhi_epi64(prod3, prod3));
	i64 t8 = f[8] * (i64)g;
	i64 t9 = f[9] * (i64)g;

	FE_CARRY; // Carry precondition OK
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static void fe_mul_small_neon(fe h, const fe f, i32 g)
{
	int32x2_t vg = vdup_n_s32(g);
	int32x4_t vf0 = vld1q_s32(&f[0]);
	int32x4_t vf1 = vld1q_s32(&f[4]);
	int64x2_t prod0 = vmull_s32(vget_low_s32(vf0), vg);
	int64x2_t prod1 = vmull_s32(vget_high_s32(vf0), vg);
	int64x2_t prod2 = vmull_s32(vget_low_s32(vf1), vg);
	int64x2_t prod3 = vmull_s32(vget_high_s32(vf1), vg);
	i64 t0 = vgetq_lane_s64(prod0, 0);
	i64 t1 = vgetq_lane_s64(prod0, 1);
	i64 t2 = vgetq_lane_s64(prod1, 0);
	i64 t3 = vgetq_lane_s64(prod1, 1);
	i64 t4 = vgetq_lane_s64(prod2, 0);
	i64 t5 = vgetq_lane_s64(prod2, 1);
	i64 t6 = vgetq_lane_s64(prod3, 0);
	i64 t7 = vgetq_lane_s64(prod3, 1);
	i64 t8 = f[8] * (i64)g;
	i64 t9 = f[9] * (i64)g;
	// |t0|, |t2|, |t4|, |t6|, |t8|  <  1.65 * 2^26 * 2^31  < 2^58
	// |t1|, |t3|, |t5|, |t7|, |t9|  <  1.65 * 2^25 * 2^31  < 2^57

	FE_CARRY; // Carry precondition OK
}
#endif

static void fe_mul_small(fe h, const fe f, i32 g)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_mul_small_avx2(h, f, g);
		return;
	}
#endif
#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
	if (mono_have_sse41_cached()) {
		fe_mul_small_sse41(h, f, g);
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		fe_mul_small_neon(h, f, g);
		return;
	}
#endif
	i64 t0 = f[0] * (i64) g;  i64 t1 = f[1] * (i64) g;
	i64 t2 = f[2] * (i64) g;  i64 t3 = f[3] * (i64) g;
	i64 t4 = f[4] * (i64) g;  i64 t5 = f[5] * (i64) g;
	i64 t6 = f[6] * (i64) g;  i64 t7 = f[7] * (i64) g;
	i64 t8 = f[8] * (i64) g;  i64 t9 = f[9] * (i64) g;
	// |t0|, |t2|, |t4|, |t6|, |t8|  <  1.65 * 2^26 * 2^31  < 2^58
	// |t1|, |t3|, |t5|, |t7|, |t9|  <  1.65 * 2^25 * 2^31  < 2^57

	FE_CARRY; // Carry precondition OK
}

// Precondition
// -------------
//   |f0|, |f2|, |f4|, |f6|, |f8|  <  1.65 * 2^26
//   |f1|, |f3|, |f5|, |f7|, |f9|  <  1.65 * 2^25
//
//   |g0|, |g2|, |g4|, |g6|, |g8|  <  1.65 * 2^26
//   |g1|, |g3|, |g5|, |g7|, |g9|  <  1.65 * 2^25
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
// AVX2-targeted clone to enable compiler auto-vectorization.
MONO_TARGET_AVX2
static i64 muladd4_avx2(i32 a0, i32 a1, i32 a2, i32 a3,
                        i32 b0, i32 b1, i32 b2, i32 b3)
{
	__m256i va = _mm256_setr_epi32(a0, 0, a1, 0, a2, 0, a3, 0);
	__m256i vb = _mm256_setr_epi32(b0, 0, b1, 0, b2, 0, b3, 0);
	__m256i prod = _mm256_mul_epi32(va, vb);
	__m128i lo = _mm256_castsi256_si128(prod);
	__m128i hi = _mm256_extracti128_si256(prod, 1);
	__m128i sum = _mm_add_epi64(lo, hi);
	sum = _mm_add_epi64(sum, _mm_srli_si128(sum, 8));
	return (i64)_mm_cvtsi128_si64(sum);
}
#endif

#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
MONO_TARGET_SSE41
static i64 muladd4_sse41(i32 a0, i32 a1, i32 a2, i32 a3,
                         i32 b0, i32 b1, i32 b2, i32 b3)
{
	__m128i va = _mm_setr_epi32(a0, a1, a2, a3);
	__m128i vb = _mm_setr_epi32(b0, b1, b2, b3);
	__m128i prod02 = _mm_mul_epi32(va, vb);
	__m128i prod13 = _mm_mul_epi32(_mm_srli_si128(va, 4),
	                               _mm_srli_si128(vb, 4));
	__m128i sum = _mm_add_epi64(prod02, prod13);
	sum = _mm_add_epi64(sum, _mm_srli_si128(sum, 8));
	return (i64)_mm_cvtsi128_si64(sum);
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static i64 muladd4_neon(i32 a0, i32 a1, i32 a2, i32 a3,
                        i32 b0, i32 b1, i32 b2, i32 b3)
{
	int32x4_t va = {a0, a1, a2, a3};
	int32x4_t vb = {b0, b1, b2, b3};
	int64x2_t prod0 = vmull_s32(vget_low_s32(va), vget_low_s32(vb));
	int64x2_t prod1 = vmull_s32(vget_high_s32(va), vget_high_s32(vb));
	int64x2_t sum = vaddq_s64(prod0, prod1);
	return vgetq_lane_s64(sum, 0) + vgetq_lane_s64(sum, 1);
}
#endif

#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
MONO_TARGET_AVX2
static void fe_mul_avx2(fe h, const fe f, const fe g)
{
	// Everything is unrolled and put in temporary variables.
	// We could roll the loop, but that would make curve25519 twice as slow.
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 g0 = g[0]; i32 g1 = g[1]; i32 g2 = g[2]; i32 g3 = g[3]; i32 g4 = g[4];
	i32 g5 = g[5]; i32 g6 = g[6]; i32 g7 = g[7]; i32 g8 = g[8]; i32 g9 = g[9];
	i32 F1 = f1*2; i32 F3 = f3*2; i32 F5 = f5*2; i32 F7 = f7*2; i32 F9 = f9*2;
	i32 G1 = g1*19;  i32 G2 = g2*19;  i32 G3 = g3*19;
	i32 G4 = g4*19;  i32 G5 = g5*19;  i32 G6 = g6*19;
	i32 G7 = g7*19;  i32 G8 = g8*19;  i32 G9 = g9*19;
	// |F1|, |F3|, |F5|, |F7|, |F9|  <  1.65 * 2^26
	// |G0|, |G2|, |G4|, |G6|, |G8|  <  2^31
	// |G1|, |G3|, |G5|, |G7|, |G9|  <  2^30

	i64 t0 = muladd4_avx2(f0, F1, f2, F3, g0, G9, G8, G7)
	       + muladd4_avx2(f4, F5, f6, F7, G6, G5, G4, G3)
	       + f8*(i64)G2 + F9*(i64)G1;
	i64 t1 = muladd4_avx2(f0, f1, f2, f3, g1, g0, G9, G8)
	       + muladd4_avx2(f4, f5, f6, f7, G7, G6, G5, G4)
	       + f8*(i64)G3 + f9*(i64)G2;
	i64 t2 = muladd4_avx2(f0, F1, f2, F3, g2, g1, g0, G9)
	       + muladd4_avx2(f4, F5, f6, F7, G8, G7, G6, G5)
	       + f8*(i64)G4 + F9*(i64)G3;
	i64 t3 = muladd4_avx2(f0, f1, f2, f3, g3, g2, g1, g0)
	       + muladd4_avx2(f4, f5, f6, f7, G9, G8, G7, G6)
	       + f8*(i64)G5 + f9*(i64)G4;
	i64 t4 = muladd4_avx2(f0, F1, f2, F3, g4, g3, g2, g1)
	       + muladd4_avx2(f4, F5, f6, F7, g0, G9, G8, G7)
	       + f8*(i64)G6 + F9*(i64)G5;
	i64 t5 = muladd4_avx2(f0, f1, f2, f3, g5, g4, g3, g2)
	       + muladd4_avx2(f4, f5, f6, f7, g1, g0, G9, G8)
	       + f8*(i64)G7 + f9*(i64)G6;
	i64 t6 = muladd4_avx2(f0, F1, f2, F3, g6, g5, g4, g3)
	       + muladd4_avx2(f4, F5, f6, F7, g2, g1, g0, G9)
	       + f8*(i64)G8 + F9*(i64)G7;
	i64 t7 = muladd4_avx2(f0, f1, f2, f3, g7, g6, g5, g4)
	       + muladd4_avx2(f4, f5, f6, f7, g3, g2, g1, g0)
	       + f8*(i64)G9 + f9*(i64)G8;
	i64 t8 = muladd4_avx2(f0, F1, f2, F3, g8, g7, g6, g5)
	       + muladd4_avx2(f4, F5, f6, F7, g4, g3, g2, g1)
	       + f8*(i64)g0 + F9*(i64)G9;
	i64 t9 = muladd4_avx2(f0, f1, f2, f3, g9, g8, g7, g6)
	       + muladd4_avx2(f4, f5, f6, f7, g5, g4, g3, g2)
	       + f8*(i64)g1 + f9*(i64)g0;
	// t0 < 0.67 * 2^61
	// t1 < 0.41 * 2^61
	// t2 < 0.52 * 2^61
	// t3 < 0.32 * 2^61
	// t4 < 0.38 * 2^61
	// t5 < 0.22 * 2^61
	// t6 < 0.23 * 2^61
	// t7 < 0.13 * 2^61
	// t8 < 0.09 * 2^61
	// t9 < 0.03 * 2^61

	FE_CARRY; // Everything below 2^62, Carry precondition OK
}
#endif

#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
MONO_TARGET_SSE41
static void fe_mul_sse41(fe h, const fe f, const fe g)
{
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 g0 = g[0]; i32 g1 = g[1]; i32 g2 = g[2]; i32 g3 = g[3]; i32 g4 = g[4];
	i32 g5 = g[5]; i32 g6 = g[6]; i32 g7 = g[7]; i32 g8 = g[8]; i32 g9 = g[9];
	i32 F1 = f1*2; i32 F3 = f3*2; i32 F5 = f5*2; i32 F7 = f7*2; i32 F9 = f9*2;
	i32 G1 = g1*19;  i32 G2 = g2*19;  i32 G3 = g3*19;
	i32 G4 = g4*19;  i32 G5 = g5*19;  i32 G6 = g6*19;
	i32 G7 = g7*19;  i32 G8 = g8*19;  i32 G9 = g9*19;

	i64 t0 = muladd4_sse41(f0, F1, f2, F3, g0, G9, G8, G7)
	       + muladd4_sse41(f4, F5, f6, F7, G6, G5, G4, G3)
	       + f8*(i64)G2 + F9*(i64)G1;
	i64 t1 = muladd4_sse41(f0, f1, f2, f3, g1, g0, G9, G8)
	       + muladd4_sse41(f4, f5, f6, f7, G7, G6, G5, G4)
	       + f8*(i64)G3 + f9*(i64)G2;
	i64 t2 = muladd4_sse41(f0, F1, f2, F3, g2, g1, g0, G9)
	       + muladd4_sse41(f4, F5, f6, F7, G8, G7, G6, G5)
	       + f8*(i64)G4 + F9*(i64)G3;
	i64 t3 = muladd4_sse41(f0, f1, f2, f3, g3, g2, g1, g0)
	       + muladd4_sse41(f4, f5, f6, f7, G9, G8, G7, G6)
	       + f8*(i64)G5 + f9*(i64)G4;
	i64 t4 = muladd4_sse41(f0, F1, f2, F3, g4, g3, g2, g1)
	       + muladd4_sse41(f4, F5, f6, F7, g0, G9, G8, G7)
	       + f8*(i64)G6 + F9*(i64)G5;
	i64 t5 = muladd4_sse41(f0, f1, f2, f3, g5, g4, g3, g2)
	       + muladd4_sse41(f4, f5, f6, f7, g1, g0, G9, G8)
	       + f8*(i64)G7 + f9*(i64)G6;
	i64 t6 = muladd4_sse41(f0, F1, f2, F3, g6, g5, g4, g3)
	       + muladd4_sse41(f4, F5, f6, F7, g2, g1, g0, G9)
	       + f8*(i64)G8 + F9*(i64)G7;
	i64 t7 = muladd4_sse41(f0, f1, f2, f3, g7, g6, g5, g4)
	       + muladd4_sse41(f4, f5, f6, f7, g3, g2, g1, g0)
	       + f8*(i64)G9 + f9*(i64)G8;
	i64 t8 = muladd4_sse41(f0, F1, f2, F3, g8, g7, g6, g5)
	       + muladd4_sse41(f4, F5, f6, F7, g4, g3, g2, g1)
	       + f8*(i64)g0 + F9*(i64)G9;
	i64 t9 = muladd4_sse41(f0, f1, f2, f3, g9, g8, g7, g6)
	       + muladd4_sse41(f4, f5, f6, f7, g5, g4, g3, g2)
	       + f8*(i64)g1 + f9*(i64)g0;

	FE_CARRY; // Everything below 2^62, Carry precondition OK
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static void fe_mul_neon(fe h, const fe f, const fe g)
{
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 g0 = g[0]; i32 g1 = g[1]; i32 g2 = g[2]; i32 g3 = g[3]; i32 g4 = g[4];
	i32 g5 = g[5]; i32 g6 = g[6]; i32 g7 = g[7]; i32 g8 = g[8]; i32 g9 = g[9];
	i32 F1 = f1*2; i32 F3 = f3*2; i32 F5 = f5*2; i32 F7 = f7*2; i32 F9 = f9*2;
	i32 G1 = g1*19;  i32 G2 = g2*19;  i32 G3 = g3*19;
	i32 G4 = g4*19;  i32 G5 = g5*19;  i32 G6 = g6*19;
	i32 G7 = g7*19;  i32 G8 = g8*19;  i32 G9 = g9*19;

	i64 t0 = muladd4_neon(f0, F1, f2, F3, g0, G9, G8, G7)
	       + muladd4_neon(f4, F5, f6, F7, G6, G5, G4, G3)
	       + f8*(i64)G2 + F9*(i64)G1;
	i64 t1 = muladd4_neon(f0, f1, f2, f3, g1, g0, G9, G8)
	       + muladd4_neon(f4, f5, f6, f7, G7, G6, G5, G4)
	       + f8*(i64)G3 + f9*(i64)G2;
	i64 t2 = muladd4_neon(f0, F1, f2, F3, g2, g1, g0, G9)
	       + muladd4_neon(f4, F5, f6, F7, G8, G7, G6, G5)
	       + f8*(i64)G4 + F9*(i64)G3;
	i64 t3 = muladd4_neon(f0, f1, f2, f3, g3, g2, g1, g0)
	       + muladd4_neon(f4, f5, f6, f7, G9, G8, G7, G6)
	       + f8*(i64)G5 + f9*(i64)G4;
	i64 t4 = muladd4_neon(f0, F1, f2, F3, g4, g3, g2, g1)
	       + muladd4_neon(f4, F5, f6, F7, g0, G9, G8, G7)
	       + f8*(i64)G6 + F9*(i64)G5;
	i64 t5 = muladd4_neon(f0, f1, f2, f3, g5, g4, g3, g2)
	       + muladd4_neon(f4, f5, f6, f7, g1, g0, G9, G8)
	       + f8*(i64)G7 + f9*(i64)G6;
	i64 t6 = muladd4_neon(f0, F1, f2, F3, g6, g5, g4, g3)
	       + muladd4_neon(f4, F5, f6, F7, g2, g1, g0, G9)
	       + f8*(i64)G8 + F9*(i64)G7;
	i64 t7 = muladd4_neon(f0, f1, f2, f3, g7, g6, g5, g4)
	       + muladd4_neon(f4, f5, f6, f7, g3, g2, g1, g0)
	       + f8*(i64)G9 + f9*(i64)G8;
	i64 t8 = muladd4_neon(f0, F1, f2, F3, g8, g7, g6, g5)
	       + muladd4_neon(f4, F5, f6, F7, g4, g3, g2, g1)
	       + f8*(i64)g0 + F9*(i64)G9;
	i64 t9 = muladd4_neon(f0, f1, f2, f3, g9, g8, g7, g6)
	       + muladd4_neon(f4, f5, f6, f7, g5, g4, g3, g2)
	       + f8*(i64)g1 + f9*(i64)g0;

	FE_CARRY; // Everything below 2^62, Carry precondition OK
}
#endif

static void fe_mul(fe h, const fe f, const fe g)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_mul_avx2(h, f, g);
		return;
	}
#endif
#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
	if (mono_have_sse41_cached()) {
		fe_mul_sse41(h, f, g);
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		fe_mul_neon(h, f, g);
		return;
	}
#endif
	// Everything is unrolled and put in temporary variables.
	// We could roll the loop, but that would make curve25519 twice as slow.
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 g0 = g[0]; i32 g1 = g[1]; i32 g2 = g[2]; i32 g3 = g[3]; i32 g4 = g[4];
	i32 g5 = g[5]; i32 g6 = g[6]; i32 g7 = g[7]; i32 g8 = g[8]; i32 g9 = g[9];
	i32 F1 = f1*2; i32 F3 = f3*2; i32 F5 = f5*2; i32 F7 = f7*2; i32 F9 = f9*2;
	i32 G1 = g1*19;  i32 G2 = g2*19;  i32 G3 = g3*19;
	i32 G4 = g4*19;  i32 G5 = g5*19;  i32 G6 = g6*19;
	i32 G7 = g7*19;  i32 G8 = g8*19;  i32 G9 = g9*19;
	// |F1|, |F3|, |F5|, |F7|, |F9|  <  1.65 * 2^26
	// |G0|, |G2|, |G4|, |G6|, |G8|  <  2^31
	// |G1|, |G3|, |G5|, |G7|, |G9|  <  2^30

	i64 t0 = f0*(i64)g0 + F1*(i64)G9 + f2*(i64)G8 + F3*(i64)G7 + f4*(i64)G6
	       + F5*(i64)G5 + f6*(i64)G4 + F7*(i64)G3 + f8*(i64)G2 + F9*(i64)G1;
	i64 t1 = f0*(i64)g1 + f1*(i64)g0 + f2*(i64)G9 + f3*(i64)G8 + f4*(i64)G7
	       + f5*(i64)G6 + f6*(i64)G5 + f7*(i64)G4 + f8*(i64)G3 + f9*(i64)G2;
	i64 t2 = f0*(i64)g2 + F1*(i64)g1 + f2*(i64)g0 + F3*(i64)G9 + f4*(i64)G8
	       + F5*(i64)G7 + f6*(i64)G6 + F7*(i64)G5 + f8*(i64)G4 + F9*(i64)G3;
	i64 t3 = f0*(i64)g3 + f1*(i64)g2 + f2*(i64)g1 + f3*(i64)g0 + f4*(i64)G9
	       + f5*(i64)G8 + f6*(i64)G7 + f7*(i64)G6 + f8*(i64)G5 + f9*(i64)G4;
	i64 t4 = f0*(i64)g4 + F1*(i64)g3 + f2*(i64)g2 + F3*(i64)g1 + f4*(i64)g0
	       + F5*(i64)G9 + f6*(i64)G8 + F7*(i64)G7 + f8*(i64)G6 + F9*(i64)G5;
	i64 t5 = f0*(i64)g5 + f1*(i64)g4 + f2*(i64)g3 + f3*(i64)g2 + f4*(i64)g1
	       + f5*(i64)g0 + f6*(i64)G9 + f7*(i64)G8 + f8*(i64)G7 + f9*(i64)G6;
	i64 t6 = f0*(i64)g6 + F1*(i64)g5 + f2*(i64)g4 + F3*(i64)g3 + f4*(i64)g2
	       + F5*(i64)g1 + f6*(i64)g0 + F7*(i64)G9 + f8*(i64)G8 + F9*(i64)G7;
	i64 t7 = f0*(i64)g7 + f1*(i64)g6 + f2*(i64)g5 + f3*(i64)g4 + f4*(i64)g3
	       + f5*(i64)g2 + f6*(i64)g1 + f7*(i64)g0 + f8*(i64)G9 + f9*(i64)G8;
	i64 t8 = f0*(i64)g8 + F1*(i64)g7 + f2*(i64)g6 + F3*(i64)g5 + f4*(i64)g4
	       + F5*(i64)g3 + f6*(i64)g2 + F7*(i64)g1 + f8*(i64)g0 + F9*(i64)G9;
	i64 t9 = f0*(i64)g9 + f1*(i64)g8 + f2*(i64)g7 + f3*(i64)g6 + f4*(i64)g5
	       + f5*(i64)g4 + f6*(i64)g3 + f7*(i64)g2 + f8*(i64)g1 + f9*(i64)g0;
	// t0 < 0.67 * 2^61
	// t1 < 0.41 * 2^61
	// t2 < 0.52 * 2^61
	// t3 < 0.32 * 2^61
	// t4 < 0.38 * 2^61
	// t5 < 0.22 * 2^61
	// t6 < 0.23 * 2^61
	// t7 < 0.13 * 2^61
	// t8 < 0.09 * 2^61
	// t9 < 0.03 * 2^61

	FE_CARRY; // Everything below 2^62, Carry precondition OK
}

// Precondition
// -------------
//   |f0|, |f2|, |f4|, |f6|, |f8|  <  1.65 * 2^26
//   |f1|, |f3|, |f5|, |f7|, |f9|  <  1.65 * 2^25
//
// Note: we could use fe_mul() for this, but this is significantly faster
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
// AVX2-targeted clone to enable compiler auto-vectorization.
MONO_TARGET_AVX2
static void fe_sq_avx2(fe h, const fe f)
{
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 f0_2  = f0*2;   i32 f1_2  = f1*2;   i32 f2_2  = f2*2;   i32 f3_2 = f3*2;
	i32 f4_2  = f4*2;   i32 f5_2  = f5*2;   i32 f6_2  = f6*2;   i32 f7_2 = f7*2;
	i32 f5_38 = f5*38;  i32 f6_19 = f6*19;  i32 f7_38 = f7*38;
	i32 f8_19 = f8*19;  i32 f9_38 = f9*38;
	// |f0_2| , |f2_2| , |f4_2| , |f6_2| , |f8_2|  <  1.65 * 2^27
	// |f1_2| , |f3_2| , |f5_2| , |f7_2| , |f9_2|  <  1.65 * 2^26
	// |f5_38|, |f6_19|, |f7_38|, |f8_19|, |f9_38| <  2^31

	i64 t0 = muladd4_avx2(f0, f1_2, f2_2, f3_2, f0, f9_38, f8_19, f7_38)
	       + f4_2*(i64)f6_19 + f5*(i64)f5_38;
	i64 t1 = muladd4_avx2(f0_2, f2, f3_2, f4, f1, f9_38, f8_19, f7_38)
	       + f5_2*(i64)f6_19;
	i64 t2 = muladd4_avx2(f0_2, f1_2, f3_2, f4_2, f2, f1, f9_38, f8_19)
	       + f5_2*(i64)f7_38 + f6*(i64)f6_19;
	i64 t3 = muladd4_avx2(f0_2, f1_2, f4, f5_2, f3, f2, f9_38, f8_19)
	       + f6*(i64)f7_38;
	i64 t4 = muladd4_avx2(f0_2, f1_2, f2, f5_2, f4, f3_2, f2, f9_38)
	       + f6_2*(i64)f8_19 + f7*(i64)f7_38;
	i64 t5 = muladd4_avx2(f0_2, f1_2, f2_2, f6, f5, f4, f3, f9_38)
	       + f7_2*(i64)f8_19;
	i64 t6 = muladd4_avx2(f0_2, f1_2, f2_2, f3_2, f6, f5_2, f4, f3)
	       + f7_2*(i64)f9_38 + f8*(i64)f8_19;
	i64 t7 = muladd4_avx2(f0_2, f1_2, f2_2, f3_2, f7, f6, f5, f4)
	       + f8*(i64)f9_38;
	i64 t8 = muladd4_avx2(f0_2, f1_2, f2_2, f3_2, f8, f7_2, f6, f5_2)
	       + f4*(i64)f4 + f9*(i64)f9_38;
	i64 t9 = muladd4_avx2(f0_2, f1_2, f2_2, f3_2, f9, f8, f7, f6)
	       + f4*(i64)f5_2;
	// t0 < 0.67 * 2^61
	// t1 < 0.41 * 2^61
	// t2 < 0.52 * 2^61
	// t3 < 0.32 * 2^61
	// t4 < 0.38 * 2^61
	// t5 < 0.22 * 2^61
	// t6 < 0.23 * 2^61
	// t7 < 0.13 * 2^61
	// t8 < 0.09 * 2^61
	// t9 < 0.03 * 2^61

	FE_CARRY;
}
#endif

#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
MONO_TARGET_SSE41
static void fe_sq_sse41(fe h, const fe f)
{
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 f0_2  = f0*2;   i32 f1_2  = f1*2;   i32 f2_2  = f2*2;   i32 f3_2 = f3*2;
	i32 f4_2  = f4*2;   i32 f5_2  = f5*2;   i32 f6_2  = f6*2;   i32 f7_2 = f7*2;
	i32 f5_38 = f5*38;  i32 f6_19 = f6*19;  i32 f7_38 = f7*38;
	i32 f8_19 = f8*19;  i32 f9_38 = f9*38;

	i64 t0 = muladd4_sse41(f0, f1_2, f2_2, f3_2, f0, f9_38, f8_19, f7_38)
	       + f4_2*(i64)f6_19 + f5*(i64)f5_38;
	i64 t1 = muladd4_sse41(f0_2, f2, f3_2, f4, f1, f9_38, f8_19, f7_38)
	       + f5_2*(i64)f6_19;
	i64 t2 = muladd4_sse41(f0_2, f1_2, f3_2, f4_2, f2, f1, f9_38, f8_19)
	       + f5_2*(i64)f7_38 + f6*(i64)f6_19;
	i64 t3 = muladd4_sse41(f0_2, f1_2, f4, f5_2, f3, f2, f9_38, f8_19)
	       + f6*(i64)f7_38;
	i64 t4 = muladd4_sse41(f0_2, f1_2, f2, f5_2, f4, f3_2, f2, f9_38)
	       + f6_2*(i64)f8_19 + f7*(i64)f7_38;
	i64 t5 = muladd4_sse41(f0_2, f1_2, f2_2, f6, f5, f4, f3, f9_38)
	       + f7_2*(i64)f8_19;
	i64 t6 = muladd4_sse41(f0_2, f1_2, f2_2, f3_2, f6, f5_2, f4, f3)
	       + f7_2*(i64)f9_38 + f8*(i64)f8_19;
	i64 t7 = muladd4_sse41(f0_2, f1_2, f2_2, f3_2, f7, f6, f5, f4)
	       + f8*(i64)f9_38;
	i64 t8 = muladd4_sse41(f0_2, f1_2, f2_2, f3_2, f8, f7_2, f6, f5_2)
	       + f4*(i64)f4 + f9*(i64)f9_38;
	i64 t9 = muladd4_sse41(f0_2, f1_2, f2_2, f3_2, f9, f8, f7, f6)
	       + f4*(i64)f5_2;

	FE_CARRY;
}
#endif

#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
static void fe_sq_neon(fe h, const fe f)
{
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 f0_2  = f0*2;   i32 f1_2  = f1*2;   i32 f2_2  = f2*2;   i32 f3_2 = f3*2;
	i32 f4_2  = f4*2;   i32 f5_2  = f5*2;   i32 f6_2  = f6*2;   i32 f7_2 = f7*2;
	i32 f5_38 = f5*38;  i32 f6_19 = f6*19;  i32 f7_38 = f7*38;
	i32 f8_19 = f8*19;  i32 f9_38 = f9*38;

	i64 t0 = muladd4_neon(f0, f1_2, f2_2, f3_2, f0, f9_38, f8_19, f7_38)
	       + f4_2*(i64)f6_19 + f5*(i64)f5_38;
	i64 t1 = muladd4_neon(f0_2, f2, f3_2, f4, f1, f9_38, f8_19, f7_38)
	       + f5_2*(i64)f6_19;
	i64 t2 = muladd4_neon(f0_2, f1_2, f3_2, f4_2, f2, f1, f9_38, f8_19)
	       + f5_2*(i64)f7_38 + f6*(i64)f6_19;
	i64 t3 = muladd4_neon(f0_2, f1_2, f4, f5_2, f3, f2, f9_38, f8_19)
	       + f6*(i64)f7_38;
	i64 t4 = muladd4_neon(f0_2, f1_2, f2, f5_2, f4, f3_2, f2, f9_38)
	       + f6_2*(i64)f8_19 + f7*(i64)f7_38;
	i64 t5 = muladd4_neon(f0_2, f1_2, f2_2, f6, f5, f4, f3, f9_38)
	       + f7_2*(i64)f8_19;
	i64 t6 = muladd4_neon(f0_2, f1_2, f2_2, f3_2, f6, f5_2, f4, f3)
	       + f7_2*(i64)f9_38 + f8*(i64)f8_19;
	i64 t7 = muladd4_neon(f0_2, f1_2, f2_2, f3_2, f7, f6, f5, f4)
	       + f8*(i64)f9_38;
	i64 t8 = muladd4_neon(f0_2, f1_2, f2_2, f3_2, f8, f7_2, f6, f5_2)
	       + f4*(i64)f4 + f9*(i64)f9_38;
	i64 t9 = muladd4_neon(f0_2, f1_2, f2_2, f3_2, f9, f8, f7, f6)
	       + f4*(i64)f5_2;

	FE_CARRY;
}
#endif

static void fe_sq(fe h, const fe f)
{
#if defined(MONO_HAS_AVX2) && MONO_HAS_AVX2
	if (mono_have_avx2_cached()) {
		fe_sq_avx2(h, f);
		return;
	}
#endif
#if defined(MONO_HAS_SSE41) && MONO_HAS_SSE41
	if (mono_have_sse41_cached()) {
		fe_sq_sse41(h, f);
		return;
	}
#endif
#if (defined(__ARM_NEON) || defined(__aarch64__)) && !defined(MONOCYPHER_DISABLE_NEON)
	if (mono_have_neon_cached()) {
		fe_sq_neon(h, f);
		return;
	}
#endif
	i32 f0 = f[0]; i32 f1 = f[1]; i32 f2 = f[2]; i32 f3 = f[3]; i32 f4 = f[4];
	i32 f5 = f[5]; i32 f6 = f[6]; i32 f7 = f[7]; i32 f8 = f[8]; i32 f9 = f[9];
	i32 f0_2  = f0*2;   i32 f1_2  = f1*2;   i32 f2_2  = f2*2;   i32 f3_2 = f3*2;
	i32 f4_2  = f4*2;   i32 f5_2  = f5*2;   i32 f6_2  = f6*2;   i32 f7_2 = f7*2;
	i32 f5_38 = f5*38;  i32 f6_19 = f6*19;  i32 f7_38 = f7*38;
	i32 f8_19 = f8*19;  i32 f9_38 = f9*38;
	// |f0_2| , |f2_2| , |f4_2| , |f6_2| , |f8_2|  <  1.65 * 2^27
	// |f1_2| , |f3_2| , |f5_2| , |f7_2| , |f9_2|  <  1.65 * 2^26
	// |f5_38|, |f6_19|, |f7_38|, |f8_19|, |f9_38| <  2^31

	i64 t0 = f0  *(i64)f0    + f1_2*(i64)f9_38 + f2_2*(i64)f8_19
	       + f3_2*(i64)f7_38 + f4_2*(i64)f6_19 + f5  *(i64)f5_38;
	i64 t1 = f0_2*(i64)f1    + f2  *(i64)f9_38 + f3_2*(i64)f8_19
	       + f4  *(i64)f7_38 + f5_2*(i64)f6_19;
	i64 t2 = f0_2*(i64)f2    + f1_2*(i64)f1    + f3_2*(i64)f9_38
	       + f4_2*(i64)f8_19 + f5_2*(i64)f7_38 + f6  *(i64)f6_19;
	i64 t3 = f0_2*(i64)f3    + f1_2*(i64)f2    + f4  *(i64)f9_38
	       + f5_2*(i64)f8_19 + f6  *(i64)f7_38;
	i64 t4 = f0_2*(i64)f4    + f1_2*(i64)f3_2  + f2  *(i64)f2
	       + f5_2*(i64)f9_38 + f6_2*(i64)f8_19 + f7  *(i64)f7_38;
	i64 t5 = f0_2*(i64)f5    + f1_2*(i64)f4    + f2_2*(i64)f3
	       + f6  *(i64)f9_38 + f7_2*(i64)f8_19;
	i64 t6 = f0_2*(i64)f6    + f1_2*(i64)f5_2  + f2_2*(i64)f4
	       + f3_2*(i64)f3    + f7_2*(i64)f9_38 + f8  *(i64)f8_19;
	i64 t7 = f0_2*(i64)f7    + f1_2*(i64)f6    + f2_2*(i64)f5
	       + f3_2*(i64)f4    + f8  *(i64)f9_38;
	i64 t8 = f0_2*(i64)f8    + f1_2*(i64)f7_2  + f2_2*(i64)f6
	       + f3_2*(i64)f5_2  + f4  *(i64)f4    + f9  *(i64)f9_38;
	i64 t9 = f0_2*(i64)f9    + f1_2*(i64)f8    + f2_2*(i64)f7
	       + f3_2*(i64)f6    + f4  *(i64)f5_2;
	// t0 < 0.67 * 2^61
	// t1 < 0.41 * 2^61
	// t2 < 0.52 * 2^61
	// t3 < 0.32 * 2^61
	// t4 < 0.38 * 2^61
	// t5 < 0.22 * 2^61
	// t6 < 0.23 * 2^61
	// t7 < 0.13 * 2^61
	// t8 < 0.09 * 2^61
	// t9 < 0.03 * 2^61

	FE_CARRY;
}

//  Parity check.  Returns 0 if even, 1 if odd
static int fe_isodd(const fe f)
{
	u8 s[32];
	fe_tobytes(s, f);
	u8 isodd = s[0] & 1;
	WIPE_BUFFER(s);
	return isodd;
}

// Returns 1 if equal, 0 if not equal
static int fe_isequal(const fe f, const fe g)
{
	u8 fs[32];
	u8 gs[32];
	fe_tobytes(fs, f);
	fe_tobytes(gs, g);
	int isdifferent = crypto_verify32(fs, gs);
	WIPE_BUFFER(fs);
	WIPE_BUFFER(gs);
	return 1 + isdifferent;
}

// Inverse square root.
// Returns true if x is a square, false otherwise.
// After the call:
//   isr = sqrt(1/x)        if x is a non-zero square.
//   isr = sqrt(sqrt(-1)/x) if x is not a square.
//   isr = 0                if x is zero.
// We do not guarantee the sign of the square root.
//
// Notes:
// Let quartic = x^((p-1)/4)
//
// x^((p-1)/2) = chi(x)
// quartic^2   = chi(x)
// quartic     = sqrt(chi(x))
// quartic     = 1 or -1 or sqrt(-1) or -sqrt(-1)
//
// Note that x is a square if quartic is 1 or -1
// There are 4 cases to consider:
//
// if   quartic         = 1  (x is a square)
// then x^((p-1)/4)     = 1
//      x^((p-5)/4) * x = 1
//      x^((p-5)/4)     = 1/x
//      x^((p-5)/8)     = sqrt(1/x) or -sqrt(1/x)
//
// if   quartic                = -1  (x is a square)
// then x^((p-1)/4)            = -1
//      x^((p-5)/4) * x        = -1
//      x^((p-5)/4)            = -1/x
//      x^((p-5)/8)            = sqrt(-1)   / sqrt(x)
//      x^((p-5)/8) * sqrt(-1) = sqrt(-1)^2 / sqrt(x)
//      x^((p-5)/8) * sqrt(-1) = -1/sqrt(x)
//      x^((p-5)/8) * sqrt(-1) = -sqrt(1/x) or sqrt(1/x)
//
// if   quartic         = sqrt(-1)  (x is not a square)
// then x^((p-1)/4)     = sqrt(-1)
//      x^((p-5)/4) * x = sqrt(-1)
//      x^((p-5)/4)     = sqrt(-1)/x
//      x^((p-5)/8)     = sqrt(sqrt(-1)/x) or -sqrt(sqrt(-1)/x)
//
// Note that the product of two non-squares is always a square:
//   For any non-squares a and b, chi(a) = -1 and chi(b) = -1.
//   Since chi(x) = x^((p-1)/2), chi(a)*chi(b) = chi(a*b) = 1.
//   Therefore a*b is a square.
//
//   Since sqrt(-1) and x are both non-squares, their product is a
//   square, and we can compute their square root.
//
// if   quartic                = -sqrt(-1)  (x is not a square)
// then x^((p-1)/4)            = -sqrt(-1)
//      x^((p-5)/4) * x        = -sqrt(-1)
//      x^((p-5)/4)            = -sqrt(-1)/x
//      x^((p-5)/8)            = sqrt(-sqrt(-1)/x)
//      x^((p-5)/8)            = sqrt( sqrt(-1)/x) * sqrt(-1)
//      x^((p-5)/8) * sqrt(-1) = sqrt( sqrt(-1)/x) * sqrt(-1)^2
//      x^((p-5)/8) * sqrt(-1) = sqrt( sqrt(-1)/x) * -1
//      x^((p-5)/8) * sqrt(-1) = -sqrt(sqrt(-1)/x) or sqrt(sqrt(-1)/x)
static int invsqrt(fe isr, const fe x)
{
	fe t0, t1, t2;

	// t0 = x^((p-5)/8)
	// Can be achieved with a simple double & add ladder,
	// but it would be slower.
	fe_sq(t0, x);
	fe_sq(t1,t0);                     fe_sq(t1, t1);    fe_mul(t1, x, t1);
	fe_mul(t0, t0, t1);
	fe_sq(t0, t0);                                      fe_mul(t0, t1, t0);
	fe_sq(t1, t0);  FOR (i, 1,   5) { fe_sq(t1, t1); }  fe_mul(t0, t1, t0);
	fe_sq(t1, t0);  FOR (i, 1,  10) { fe_sq(t1, t1); }  fe_mul(t1, t1, t0);
	fe_sq(t2, t1);  FOR (i, 1,  20) { fe_sq(t2, t2); }  fe_mul(t1, t2, t1);
	fe_sq(t1, t1);  FOR (i, 1,  10) { fe_sq(t1, t1); }  fe_mul(t0, t1, t0);
	fe_sq(t1, t0);  FOR (i, 1,  50) { fe_sq(t1, t1); }  fe_mul(t1, t1, t0);
	fe_sq(t2, t1);  FOR (i, 1, 100) { fe_sq(t2, t2); }  fe_mul(t1, t2, t1);
	fe_sq(t1, t1);  FOR (i, 1,  50) { fe_sq(t1, t1); }  fe_mul(t0, t1, t0);
	fe_sq(t0, t0);  FOR (i, 1,   2) { fe_sq(t0, t0); }  fe_mul(t0, t0, x);

	// quartic = x^((p-1)/4)
	i32 *quartic = t1;
	fe_sq (quartic, t0);
	fe_mul(quartic, quartic, x);

	i32 *check = t2;
	fe_0  (check);          int z0 = fe_isequal(x      , check);
	fe_1  (check);          int p1 = fe_isequal(quartic, check);
	fe_neg(check, check );  int m1 = fe_isequal(quartic, check);
	fe_neg(check, sqrtm1);  int ms = fe_isequal(quartic, check);

	// if quartic == -1 or sqrt(-1)
	// then  isr = x^((p-1)/4) * sqrt(-1)
	// else  isr = x^((p-1)/4)
	fe_mul(isr, t0, sqrtm1);
	fe_ccopy(isr, t0, 1 - (m1 | ms));

	WIPE_BUFFER(t0);
	WIPE_BUFFER(t1);
	WIPE_BUFFER(t2);
	return p1 | m1 | z0;
}

// Inverse in terms of inverse square root.
// Requires two additional squarings to get rid of the sign.
//
//   1/x = x * (+invsqrt(x^2))^2
//       = x * (-invsqrt(x^2))^2
//
// A fully optimised exponentiation by p-1 would save 6 field
// multiplications, but it would require more code.
static void fe_invert(fe out, const fe x)
{
	fe tmp;
	fe_sq(tmp, x);
	invsqrt(tmp, tmp);
	fe_sq(tmp, tmp);
	fe_mul(out, tmp, x);
	WIPE_BUFFER(tmp);
}

// trim a scalar for scalar multiplication
void crypto_eddsa_trim_scalar(u8 out[32], const u8 in[32])
{
	COPY(out, in, 32);
	out[ 0] &= 248;
	out[31] &= 127;
	out[31] |= 64;
}

int crypto_eddsa_trim_scalar_checked(u8 out[32], const u8 in[32])
{
	int err = check_out_ptr(out);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(in, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_trim_scalar(out, in);
	return CRYPTO_OK;
}

// get bit from scalar at position i
static int scalar_bit(const u8 s[32], int i)
{
	if (i < 0) { return 0; } // handle -1 for sliding windows
	return (s[i>>3] >> (i&7)) & 1;
}

///////////////
/// X-25519 /// Taken from SUPERCOP's ref10 implementation.
///////////////
static void scalarmult(u8 q[32], const u8 scalar[32], const u8 p[32],
                       int nb_bits)
{
	// computes the scalar product
	fe x1;
	fe_frombytes(x1, p);

	// computes the actual scalar product (the result is in x2 and z2)
	fe x2, z2, x3, z3, t0, t1;
	// Montgomery ladder
	// In projective coordinates, to avoid divisions: x = X / Z
	// We don't care about the y coordinate, it's only 1 bit of information
	fe_1(x2);        fe_0(z2); // "zero" point
	fe_copy(x3, x1); fe_1(z3); // "one"  point
	int swap = 0;
	for (int pos = nb_bits-1; pos >= 0; --pos) {
		// constant time conditional swap before ladder step
		int b = scalar_bit(scalar, pos);
		swap ^= b; // xor trick avoids swapping at the end of the loop
		fe_cswap(x2, x3, swap);
		fe_cswap(z2, z3, swap);
		swap = b;  // anticipates one last swap after the loop

		// Montgomery ladder step: replaces (P2, P3) by (P2*2, P2+P3)
		// with differential addition
		fe_sub(t0, x3, z3);
		fe_sub(t1, x2, z2);
		fe_add(x2, x2, z2);
		fe_add(z2, x3, z3);
		fe_mul(z3, t0, x2);
		fe_mul(z2, z2, t1);
		fe_sq (t0, t1    );
		fe_sq (t1, x2    );
		fe_add(x3, z3, z2);
		fe_sub(z2, z3, z2);
		fe_mul(x2, t1, t0);
		fe_sub(t1, t1, t0);
		fe_sq (z2, z2    );
		fe_mul_small(z3, t1, 121666);
		fe_sq (x3, x3    );
		fe_add(t0, t0, z3);
		fe_mul(z3, x1, z2);
		fe_mul(z2, t1, t0);
	}
	// last swap is necessary to compensate for the xor trick
	// Note: after this swap, P3 == P2 + P1.
	fe_cswap(x2, x3, swap);
	fe_cswap(z2, z3, swap);

	// normalises the coordinates: x == X / Z
	fe_invert(z2, z2);
	fe_mul(x2, x2, z2);
	fe_tobytes(q, x2);

	WIPE_BUFFER(x1);
	WIPE_BUFFER(x2);  WIPE_BUFFER(z2);  WIPE_BUFFER(t0);
	WIPE_BUFFER(x3);  WIPE_BUFFER(z3);  WIPE_BUFFER(t1);
}

void crypto_x25519(u8       raw_shared_secret[32],
                   const u8 your_secret_key  [32],
                   const u8 their_public_key [32])
{
	// restrict the possible scalar values
	u8 e[32];
	crypto_eddsa_trim_scalar(e, your_secret_key);
	scalarmult(raw_shared_secret, e, their_public_key, 255);
	WIPE_BUFFER(e);
}

int crypto_x25519_public_key_checked(u8 public_key[32],
                                     const u8 secret_key[32])
{
	int err = check_out_ptr(public_key);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(secret_key, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_x25519_public_key(public_key, secret_key);
	return CRYPTO_OK;
}

int crypto_x25519_checked(u8 raw_shared_secret[32],
                          const u8 your_secret_key[32],
                          const u8 their_public_key[32])
{
	int err = check_out_ptr(raw_shared_secret);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(your_secret_key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(their_public_key, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_x25519(raw_shared_secret, your_secret_key, their_public_key);
	return CRYPTO_OK;
}


void crypto_x25519_public_key(u8       public_key[32],
                              const u8 secret_key[32])
{
	static const u8 base_point[32] = {9};
	crypto_x25519(public_key, secret_key, base_point);
}

///////////////////////////
/// Arithmetic modulo L ///
///////////////////////////
static const u32 L[8] = {
	0x5cf5d3ed, 0x5812631a, 0xa2f79cd6, 0x14def9de,
	0x00000000, 0x00000000, 0x00000000, 0x10000000,
};

//  p = a*b + p
static void multiply(u32 p[16], const u32 a[8], const u32 b[8])
{
	FOR (i, 0, 8) {
		u64 carry = 0;
		FOR (j, 0, 8) {
			carry  += p[i+j] + (u64)a[i] * b[j];
			p[i+j]  = (u32)carry;
			carry >>= 32;
		}
		p[i+8] = (u32)carry;
	}
}

static int is_above_l(const u32 x[8])
{
	// We work with L directly, in a 2's complement encoding
	// (-L == ~L + 1)
	u64 carry = 1;
	FOR (i, 0, 8) {
		carry  += (u64)x[i] + (~L[i] & 0xffffffff);
		carry >>= 32;
	}
	return (int)carry; // carry is either 0 or 1
}

// Final reduction modulo L, by conditionally removing L.
// if x < l     , then r = x
// if l <= x 2*l, then r = x-l
// otherwise the result will be wrong
static void remove_l(u32 r[8], const u32 x[8])
{
	u64 carry = (u64)is_above_l(x);
	u32 mask  = ~(u32)carry + 1; // carry == 0 or 1
	FOR (i, 0, 8) {
		carry += (u64)x[i] + (~L[i] & mask);
		r[i]   = (u32)carry;
		carry >>= 32;
	}
}

// Full reduction modulo L (Barrett reduction)
static void mod_l(u8 reduced[32], const u32 x[16])
{
	static const u32 r[9] = {
		0x0a2c131b,0xed9ce5a3,0x086329a7,0x2106215d,
		0xffffffeb,0xffffffff,0xffffffff,0xffffffff,0xf,
	};
	// xr = x * r
	u32 xr[25] = {0};
	FOR (i, 0, 9) {
		u64 carry = 0;
		FOR (j, 0, 16) {
			carry  += xr[i+j] + (u64)r[i] * x[j];
			xr[i+j] = (u32)carry;
			carry >>= 32;
		}
		xr[i+16] = (u32)carry;
	}
	// xr = floor(xr / 2^512) * L
	// Since the result is guaranteed to be below 2*L,
	// it is enough to only compute the first 256 bits.
	// The division is performed by saying xr[i+16]. (16 * 32 = 512)
	ZERO(xr, 8);
	FOR (i, 0, 8) {
		u64 carry = 0;
		FOR (j, 0, 8-i) {
			carry   += xr[i+j] + (u64)xr[i+16] * L[j];
			xr[i+j] = (u32)carry;
			carry >>= 32;
		}
	}
	// xr = x - xr
	u64 carry = 1;
	FOR (i, 0, 8) {
		carry  += (u64)x[i] + (~xr[i] & 0xffffffff);
		xr[i]   = (u32)carry;
		carry >>= 32;
	}
	// Final reduction modulo L (conditional subtraction)
	remove_l(xr, xr);
	store32_le_buf(reduced, xr, 8);

	WIPE_BUFFER(xr);
}

void crypto_eddsa_reduce(u8 reduced[32], const u8 expanded[64])
{
	u32 x[16];
	load32_le_buf(x, expanded, 16);
	mod_l(reduced, x);
	WIPE_BUFFER(x);
}

int crypto_eddsa_reduce_checked(u8 reduced[32], const u8 expanded[64])
{
	int err = check_out_ptr(reduced);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(expanded, 64);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_reduce(reduced, expanded);
	return CRYPTO_OK;
}

// r = (a * b) + c
void crypto_eddsa_mul_add(u8 r[32],
                          const u8 a[32], const u8 b[32], const u8 c[32])
{
	u32 A[8];  load32_le_buf(A, a, 8);
	u32 B[8];  load32_le_buf(B, b, 8);
	u32 p[16]; load32_le_buf(p, c, 8);  ZERO(p + 8, 8);
	multiply(p, A, B);
	mod_l(r, p);
	WIPE_BUFFER(p);
	WIPE_BUFFER(A);
	WIPE_BUFFER(B);
}

int crypto_eddsa_mul_add_checked(u8 r[32],
                                 const u8 a[32], const u8 b[32],
                                 const u8 c[32])
{
	int err = check_out_ptr(r);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(a, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(b, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(c, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_mul_add(r, a, b, c);
	return CRYPTO_OK;
}

///////////////
/// Ed25519 ///
///////////////

// Point (group element, ge) in a twisted Edwards curve,
// in extended projective coordinates.
// ge        : x  = X/Z, y  = Y/Z, T  = XY/Z
// ge_cached : Yp = X+Y, Ym = X-Y, T2 = T*D2
// ge_precomp: Z  = 1
typedef struct { fe X;  fe Y;  fe Z; fe T;  } ge;
typedef struct { fe Yp; fe Ym; fe Z; fe T2; } ge_cached;
typedef struct { fe Yp; fe Ym;       fe T2; } ge_precomp;

static void ge_zero(ge *p)
{
	fe_0(p->X);
	fe_1(p->Y);
	fe_1(p->Z);
	fe_0(p->T);
}

static void ge_tobytes(u8 s[32], const ge *h)
{
	fe recip, x, y;
	fe_invert(recip, h->Z);
	fe_mul(x, h->X, recip);
	fe_mul(y, h->Y, recip);
	fe_tobytes(s, y);
	s[31] ^= fe_isodd(x) << 7;

	WIPE_BUFFER(recip);
	WIPE_BUFFER(x);
	WIPE_BUFFER(y);
}

// h = -s, where s is a point encoded in 32 bytes
//
// Variable time!  Inputs must not be secret!
// => Use only to *check* signatures.
//
// From the specifications:
//   The encoding of s contains y and the sign of x
//   x = sqrt((y^2 - 1) / (d*y^2 + 1))
// In extended coordinates:
//   X = x, Y = y, Z = 1, T = x*y
//
//    Note that num * den is a square iff num / den is a square
//    If num * den is not a square, the point was not on the curve.
// From the above:
//   Let num =   y^2 - 1
//   Let den = d*y^2 + 1
//   x = sqrt((y^2 - 1) / (d*y^2 + 1))
//   x = sqrt(num / den)
//   x = sqrt(num^2 / (num * den))
//   x = num * sqrt(1 / (num * den))
//
// Therefore, we can just compute:
//   num =   y^2 - 1
//   den = d*y^2 + 1
//   isr = invsqrt(num * den)  // abort if not square
//   x   = num * isr
// Finally, negate x if its sign is not as specified.
static int ge_frombytes_neg_vartime(ge *h, const u8 s[32])
{
	fe_frombytes(h->Y, s);
	fe_1(h->Z);
	fe_sq (h->T, h->Y);        // t =   y^2
	fe_mul(h->X, h->T, d   );  // x = d*y^2
	fe_sub(h->T, h->T, h->Z);  // t =   y^2 - 1
	fe_add(h->X, h->X, h->Z);  // x = d*y^2 + 1
	fe_mul(h->X, h->T, h->X);  // x = (y^2 - 1) * (d*y^2 + 1)
	int is_square = invsqrt(h->X, h->X);
	if (!is_square) {
		return -1;             // Not on the curve, abort
	}
	fe_mul(h->X, h->T, h->X);  // x = sqrt((y^2 - 1) / (d*y^2 + 1))
	if (fe_isodd(h->X) == (s[31] >> 7)) {
		fe_neg(h->X, h->X);
	}
	fe_mul(h->T, h->X, h->Y);
	return 0;
}

static void ge_cache(ge_cached *c, const ge *p)
{
	fe_add (c->Yp, p->Y, p->X);
	fe_sub (c->Ym, p->Y, p->X);
	fe_copy(c->Z , p->Z      );
	fe_mul (c->T2, p->T, D2  );
}

// Internal buffers are not wiped! Inputs must not be secret!
// => Use only to *check* signatures.
static void ge_add(ge *s, const ge *p, const ge_cached *q)
{
	fe a, b;
	fe_add(a   , p->Y, p->X );
	fe_sub(b   , p->Y, p->X );
	fe_mul(a   , a   , q->Yp);
	fe_mul(b   , b   , q->Ym);
	fe_add(s->Y, a   , b    );
	fe_sub(s->X, a   , b    );

	fe_add(s->Z, p->Z, p->Z );
	fe_mul(s->Z, s->Z, q->Z );
	fe_mul(s->T, p->T, q->T2);
	fe_add(a   , s->Z, s->T );
	fe_sub(b   , s->Z, s->T );

	fe_mul(s->T, s->X, s->Y);
	fe_mul(s->X, s->X, b   );
	fe_mul(s->Y, s->Y, a   );
	fe_mul(s->Z, a   , b   );
}

// Internal buffers are not wiped! Inputs must not be secret!
// => Use only to *check* signatures.
static void ge_sub(ge *s, const ge *p, const ge_cached *q)
{
	ge_cached neg;
	fe_copy(neg.Ym, q->Yp);
	fe_copy(neg.Yp, q->Ym);
	fe_copy(neg.Z , q->Z );
	fe_neg (neg.T2, q->T2);
	ge_add(s, p, &neg);
}

static void ge_madd(ge *s, const ge *p, const ge_precomp *q, fe a, fe b)
{
	fe_add(a   , p->Y, p->X );
	fe_sub(b   , p->Y, p->X );
	fe_mul(a   , a   , q->Yp);
	fe_mul(b   , b   , q->Ym);
	fe_add(s->Y, a   , b    );
	fe_sub(s->X, a   , b    );

	fe_add(s->Z, p->Z, p->Z );
	fe_mul(s->T, p->T, q->T2);
	fe_add(a   , s->Z, s->T );
	fe_sub(b   , s->Z, s->T );

	fe_mul(s->T, s->X, s->Y);
	fe_mul(s->X, s->X, b   );
	fe_mul(s->Y, s->Y, a   );
	fe_mul(s->Z, a   , b   );
}

// Internal buffers are not wiped! Inputs must not be secret!
// => Use only to *check* signatures.
static void ge_msub(ge *s, const ge *p, const ge_precomp *q, fe a, fe b)
{
	ge_precomp neg;
	fe_copy(neg.Ym, q->Yp);
	fe_copy(neg.Yp, q->Ym);
	fe_neg (neg.T2, q->T2);
	ge_madd(s, p, &neg, a, b);
}

static void ge_double(ge *s, const ge *p, ge *q)
{
	fe_sq (q->X, p->X);
	fe_sq (q->Y, p->Y);
	fe_sq (q->Z, p->Z);          // qZ = pZ^2
	fe_mul_small(q->Z, q->Z, 2); // qZ = pZ^2 * 2
	fe_add(q->T, p->X, p->Y);
	fe_sq (s->T, q->T);
	fe_add(q->T, q->Y, q->X);
	fe_sub(q->Y, q->Y, q->X);
	fe_sub(q->X, s->T, q->T);
	fe_sub(q->Z, q->Z, q->Y);

	fe_mul(s->X, q->X , q->Z);
	fe_mul(s->Y, q->T , q->Y);
	fe_mul(s->Z, q->Y , q->Z);
	fe_mul(s->T, q->X , q->T);
}

// 5-bit signed window in cached format (Niels coordinates, Z=1)
static const ge_precomp b_window[8] = {
	{{25967493,-14356035,29566456,3660896,-12694345,
	  4014787,27544626,-11754271,-6079156,2047605,},
	 {-12545711,934262,-2722910,3049990,-727428,
	  9406986,12720692,5043384,19500929,-15469378,},
	 {-8738181,4489570,9688441,-14785194,10184609,
	  -12363380,29287919,11864899,-24514362,-4438546,},},
	{{15636291,-9688557,24204773,-7912398,616977,
	  -16685262,27787600,-14772189,28944400,-1550024,},
	 {16568933,4717097,-11556148,-1102322,15682896,
	  -11807043,16354577,-11775962,7689662,11199574,},
	 {30464156,-5976125,-11779434,-15670865,23220365,
	  15915852,7512774,10017326,-17749093,-9920357,},},
	{{10861363,11473154,27284546,1981175,-30064349,
	  12577861,32867885,14515107,-15438304,10819380,},
	 {4708026,6336745,20377586,9066809,-11272109,
	  6594696,-25653668,12483688,-12668491,5581306,},
	 {19563160,16186464,-29386857,4097519,10237984,
	  -4348115,28542350,13850243,-23678021,-15815942,},},
	{{5153746,9909285,1723747,-2777874,30523605,
	  5516873,19480852,5230134,-23952439,-15175766,},
	 {-30269007,-3463509,7665486,10083793,28475525,
	  1649722,20654025,16520125,30598449,7715701,},
	 {28881845,14381568,9657904,3680757,-20181635,
	  7843316,-31400660,1370708,29794553,-1409300,},},
	{{-22518993,-6692182,14201702,-8745502,-23510406,
	  8844726,18474211,-1361450,-13062696,13821877,},
	 {-6455177,-7839871,3374702,-4740862,-27098617,
	  -10571707,31655028,-7212327,18853322,-14220951,},
	 {4566830,-12963868,-28974889,-12240689,-7602672,
	  -2830569,-8514358,-10431137,2207753,-3209784,},},
	{{-25154831,-4185821,29681144,7868801,-6854661,
	  -9423865,-12437364,-663000,-31111463,-16132436,},
	 {25576264,-2703214,7349804,-11814844,16472782,
	  9300885,3844789,15725684,171356,6466918,},
	 {23103977,13316479,9739013,-16149481,817875,
	  -15038942,8965339,-14088058,-30714912,16193877,},},
	{{-33521811,3180713,-2394130,14003687,-16903474,
	  -16270840,17238398,4729455,-18074513,9256800,},
	 {-25182317,-4174131,32336398,5036987,-21236817,
	  11360617,22616405,9761698,-19827198,630305,},
	 {-13720693,2639453,-24237460,-7406481,9494427,
	  -5774029,-6554551,-15960994,-2449256,-14291300,},},
	{{-3151181,-5046075,9282714,6866145,-31907062,
	  -863023,-18940575,15033784,25105118,-7894876,},
	 {-24326370,15950226,-31801215,-14592823,-11662737,
	  -5090925,1573892,-2625887,2198790,-15804619,},
	 {-3099351,10324967,-2241613,7453183,-5446979,
	  -2735503,-13812022,-16236442,-32461234,-12290683,},},
};

// Incremental sliding windows (left to right)
// Based on Roberto Maria Avanzi[2005]
typedef struct {
	i16 next_index; // position of the next signed digit
	i8  next_digit; // next signed digit (odd number below 2^window_width)
	u8  next_check; // point at which we must check for a new window
} slide_ctx;

static void slide_init(slide_ctx *ctx, const u8 scalar[32])
{
	// scalar is guaranteed to be below L, either because we checked (s),
	// or because we reduced it modulo L (h_ram). L is under 2^253, so
	// so bits 253 to 255 are guaranteed to be zero. No need to test them.
	//
	// Note however that L is very close to 2^252, so bit 252 is almost
	// always zero.  If we were to start at bit 251, the tests wouldn't
	// catch the off-by-one error (constructing one that does would be
	// prohibitively expensive).
	//
	// We should still check bit 252, though.
	int i = 252;
	while (i > 0 && scalar_bit(scalar, i) == 0) {
		i--;
	}
	ctx->next_check = (u8)(i + 1);
	ctx->next_index = -1;
	ctx->next_digit = -1;
}

static int slide_step(slide_ctx *ctx, int width, int i, const u8 scalar[32])
{
	if (i == ctx->next_check) {
		if (scalar_bit(scalar, i) == scalar_bit(scalar, i - 1)) {
			ctx->next_check--;
		} else {
			// compute digit of next window
			int w = MIN(width, i + 1);
			int v = -(scalar_bit(scalar, i) << (w-1));
			FOR_T (int, j, 0, w-1) {
				v += scalar_bit(scalar, i-(w-1)+j) << j;
			}
			v += scalar_bit(scalar, i-w);
			int lsb = v & (~v + 1); // smallest bit of v
			int s   =               // log2(lsb)
				(((lsb & 0xAA) != 0) << 0) |
				(((lsb & 0xCC) != 0) << 1) |
				(((lsb & 0xF0) != 0) << 2);
			ctx->next_index  = (i16)(i-(w-1)+s);
			ctx->next_digit  = (i8) (v >> s   );
			ctx->next_check -= (u8) w;
		}
	}
	return i == ctx->next_index ? ctx->next_digit: 0;
}

#define P_W_WIDTH 3 // Affects the size of the stack
#define B_W_WIDTH 5 // Affects the size of the binary
#define P_W_SIZE  (1<<(P_W_WIDTH-2))

int crypto_eddsa_check_equation(const u8 signature[64], const u8 public_key[32],
                                const u8 h[32])
{
	ge minus_A; // -public_key
	ge minus_R; // -first_half_of_signature
	const u8 *s = signature + 32;

	// Check that A and R are on the curve
	// Check that 0 <= S < L (prevents malleability)
	// *Allow* non-cannonical encoding for A and R
	{
		u32 s32[8];
		load32_le_buf(s32, s, 8);
		if (ge_frombytes_neg_vartime(&minus_A, public_key) ||
		    ge_frombytes_neg_vartime(&minus_R, signature)  ||
		    is_above_l(s32)) {
			return -1;
		}
	}

	// look-up table for minus_A
	ge_cached lutA[P_W_SIZE];
	{
		ge minus_A2, tmp;
		ge_double(&minus_A2, &minus_A, &tmp);
		ge_cache(&lutA[0], &minus_A);
		FOR (i, 1, P_W_SIZE) {
			ge_add(&tmp, &minus_A2, &lutA[i-1]);
			ge_cache(&lutA[i], &tmp);
		}
	}

	// sum = [s]B - [h]A
	// Merged double and add ladder, fused with sliding
	slide_ctx h_slide;  slide_init(&h_slide, h);
	slide_ctx s_slide;  slide_init(&s_slide, s);
	int i = MAX(h_slide.next_check, s_slide.next_check);
	ge *sum = &minus_A; // reuse minus_A for the sum
	ge_zero(sum);
	while (i >= 0) {
		ge tmp;
		ge_double(sum, sum, &tmp);
		int h_digit = slide_step(&h_slide, P_W_WIDTH, i, h);
		int s_digit = slide_step(&s_slide, B_W_WIDTH, i, s);
		if (h_digit > 0) { ge_add(sum, sum, &lutA[ h_digit / 2]); }
		if (h_digit < 0) { ge_sub(sum, sum, &lutA[-h_digit / 2]); }
		fe t1, t2;
		if (s_digit > 0) { ge_madd(sum, sum, b_window +  s_digit/2, t1, t2); }
		if (s_digit < 0) { ge_msub(sum, sum, b_window + -s_digit/2, t1, t2); }
		i--;
	}

	// Compare [8](sum-R) and the zero point
	// The multiplication by 8 eliminates any low-order component
	// and ensures consistency with batched verification.
	ge_cached cached;
	u8 check[32];
	static const u8 zero_point[32] = {1}; // Point of order 1
	ge_cache(&cached, &minus_R);
	ge_add(sum, sum, &cached);
	ge_double(sum, sum, &minus_R); // reuse minus_R as temporary
	ge_double(sum, sum, &minus_R); // reuse minus_R as temporary
	ge_double(sum, sum, &minus_R); // reuse minus_R as temporary
	ge_tobytes(check, sum);
	return crypto_verify32(check, zero_point);
}

int crypto_eddsa_check_equation_checked(const u8 signature[64],
                                        const u8 public_key[32],
                                        const u8 h[32])
{
	int err = check_ptr(signature, 64);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(public_key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(h, 32);
	if (err != CRYPTO_OK) { return err; }
	return crypto_eddsa_check_equation(signature, public_key, h)
		? CRYPTO_ERR_AUTH
		: CRYPTO_OK;
}

// 5-bit signed comb in cached format (Niels coordinates, Z=1)
static const ge_precomp b_comb_low[8] = {
	{{-6816601,-2324159,-22559413,124364,18015490,
	  8373481,19993724,1979872,-18549925,9085059,},
	 {10306321,403248,14839893,9633706,8463310,
	  -8354981,-14305673,14668847,26301366,2818560,},
	 {-22701500,-3210264,-13831292,-2927732,-16326337,
	  -14016360,12940910,177905,12165515,-2397893,},},
	{{-12282262,-7022066,9920413,-3064358,-32147467,
	  2927790,22392436,-14852487,2719975,16402117,},
	 {-7236961,-4729776,2685954,-6525055,-24242706,
	  -15940211,-6238521,14082855,10047669,12228189,},
	 {-30495588,-12893761,-11161261,3539405,-11502464,
	  16491580,-27286798,-15030530,-7272871,-15934455,},},
	{{17650926,582297,-860412,-187745,-12072900,
	  -10683391,-20352381,15557840,-31072141,-5019061,},
	 {-6283632,-2259834,-4674247,-4598977,-4089240,
	  12435688,-31278303,1060251,6256175,10480726,},
	 {-13871026,2026300,-21928428,-2741605,-2406664,
	  -8034988,7355518,15733500,-23379862,7489131,},},
	{{6883359,695140,23196907,9644202,-33430614,
	  11354760,-20134606,6388313,-8263585,-8491918,},
	 {-7716174,-13605463,-13646110,14757414,-19430591,
	  -14967316,10359532,-11059670,-21935259,12082603,},
	 {-11253345,-15943946,10046784,5414629,24840771,
	  8086951,-6694742,9868723,15842692,-16224787,},},
	{{9639399,11810955,-24007778,-9320054,3912937,
	  -9856959,996125,-8727907,-8919186,-14097242,},
	 {7248867,14468564,25228636,-8795035,14346339,
	  8224790,6388427,-7181107,6468218,-8720783,},
	 {15513115,15439095,7342322,-10157390,18005294,
	  -7265713,2186239,4884640,10826567,7135781,},},
	{{-14204238,5297536,-5862318,-6004934,28095835,
	  4236101,-14203318,1958636,-16816875,3837147,},
	 {-5511166,-13176782,-29588215,12339465,15325758,
	  -15945770,-8813185,11075932,-19608050,-3776283,},
	 {11728032,9603156,-4637821,-5304487,-7827751,
	  2724948,31236191,-16760175,-7268616,14799772,},},
	{{-28842672,4840636,-12047946,-9101456,-1445464,
	  381905,-30977094,-16523389,1290540,12798615,},
	 {27246947,-10320914,14792098,-14518944,5302070,
	  -8746152,-3403974,-4149637,-27061213,10749585,},
	 {25572375,-6270368,-15353037,16037944,1146292,
	  32198,23487090,9585613,24714571,-1418265,},},
	{{19844825,282124,-17583147,11004019,-32004269,
	  -2716035,6105106,-1711007,-21010044,14338445,},
	 {8027505,8191102,-18504907,-12335737,25173494,
	  -5923905,15446145,7483684,-30440441,10009108,},
	 {-14134701,-4174411,10246585,-14677495,33553567,
	  -14012935,23366126,15080531,-7969992,7663473,},},
};

static const ge_precomp b_comb_high[8] = {
	{{33055887,-4431773,-521787,6654165,951411,
	  -6266464,-5158124,6995613,-5397442,-6985227,},
	 {4014062,6967095,-11977872,3960002,8001989,
	  5130302,-2154812,-1899602,-31954493,-16173976,},
	 {16271757,-9212948,23792794,731486,-25808309,
	  -3546396,6964344,-4767590,10976593,10050757,},},
	{{2533007,-4288439,-24467768,-12387405,-13450051,
	  14542280,12876301,13893535,15067764,8594792,},
	 {20073501,-11623621,3165391,-13119866,13188608,
	  -11540496,-10751437,-13482671,29588810,2197295,},
	 {-1084082,11831693,6031797,14062724,14748428,
	  -8159962,-20721760,11742548,31368706,13161200,},},
	{{2050412,-6457589,15321215,5273360,25484180,
	  124590,-18187548,-7097255,-6691621,-14604792,},
	 {9938196,2162889,-6158074,-1711248,4278932,
	  -2598531,-22865792,-7168500,-24323168,11746309,},
	 {-22691768,-14268164,5965485,9383325,20443693,
	  5854192,28250679,-1381811,-10837134,13717818,},},
	{{-8495530,16382250,9548884,-4971523,-4491811,
	  -3902147,6182256,-12832479,26628081,10395408,},
	 {27329048,-15853735,7715764,8717446,-9215518,
	  -14633480,28982250,-5668414,4227628,242148,},
	 {-13279943,-7986904,-7100016,8764468,-27276630,
	  3096719,29678419,-9141299,3906709,11265498,},},
	{{11918285,15686328,-17757323,-11217300,-27548967,
	  4853165,-27168827,6807359,6871949,-1075745,},
	 {-29002610,13984323,-27111812,-2713442,28107359,
	  -13266203,6155126,15104658,3538727,-7513788,},
	 {14103158,11233913,-33165269,9279850,31014152,
	  4335090,-1827936,4590951,13960841,12787712,},},
	{{1469134,-16738009,33411928,13942824,8092558,
	  -8778224,-11165065,1437842,22521552,-2792954,},
	 {31352705,-4807352,-25327300,3962447,12541566,
	  -9399651,-27425693,7964818,-23829869,5541287,},
	 {-25732021,-6864887,23848984,3039395,-9147354,
	  6022816,-27421653,10590137,25309915,-1584678,},},
	{{-22951376,5048948,31139401,-190316,-19542447,
	  -626310,-17486305,-16511925,-18851313,-12985140,},
	 {-9684890,14681754,30487568,7717771,-10829709,
	  9630497,30290549,-10531496,-27798994,-13812825,},
	 {5827835,16097107,-24501327,12094619,7413972,
	  11447087,28057551,-1793987,-14056981,4359312,},},
	{{26323183,2342588,-21887793,-1623758,-6062284,
	  2107090,-28724907,9036464,-19618351,-13055189,},
	 {-29697200,14829398,-4596333,14220089,-30022969,
	  2955645,12094100,-13693652,-5941445,7047569,},
	 {-3201977,14413268,-12058324,-16417589,-9035655,
	  -7224648,9258160,1399236,30397584,-5684634,},},
};

static void lookup_add(ge *p, ge_precomp *tmp_c, fe tmp_a, fe tmp_b,
                       const ge_precomp comb[8], const u8 scalar[32], int i)
{
	u8 teeth = (u8)((scalar_bit(scalar, i)          ) +
	                (scalar_bit(scalar, i + 32) << 1) +
	                (scalar_bit(scalar, i + 64) << 2) +
	                (scalar_bit(scalar, i + 96) << 3));
	u8 high  = teeth >> 3;
	u8 index = (teeth ^ (high - 1)) & 7;
	FOR (j, 0, 8) {
		i32 select = 1 & (((j ^ index) - 1) >> 8);
		fe_ccopy(tmp_c->Yp, comb[j].Yp, select);
		fe_ccopy(tmp_c->Ym, comb[j].Ym, select);
		fe_ccopy(tmp_c->T2, comb[j].T2, select);
	}
	fe_neg(tmp_a, tmp_c->T2);
	fe_cswap(tmp_c->T2, tmp_a    , high ^ 1);
	fe_cswap(tmp_c->Yp, tmp_c->Ym, high ^ 1);
	ge_madd(p, p, tmp_c, tmp_a, tmp_b);
}

// p = [scalar]B, where B is the base point
static void ge_scalarmult_base(ge *p, const u8 scalar[32])
{
	// twin 4-bits signed combs, from Mike Hamburg's
	// Fast and compact elliptic-curve cryptography (2012)
	// 1 / 2 modulo L
	static const u8 half_mod_L[32] = {
		247,233,122,46,141,49,9,44,107,206,123,81,239,124,111,10,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,
	};
	// (2^256 - 1) / 2 modulo L
	static const u8 half_ones[32] = {
		142,74,204,70,186,24,118,107,184,231,190,57,250,173,119,99,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,7,
	};

	// All bits set form: 1 means 1, 0 means -1
	u8 s_scalar[32];
	crypto_eddsa_mul_add(s_scalar, scalar, half_mod_L, half_ones);

	// Double and add ladder
	fe tmp_a, tmp_b;  // temporaries for addition
	ge_precomp tmp_c; // temporary for comb lookup
	ge tmp_d;         // temporary for doubling
	fe_1(tmp_c.Yp);
	fe_1(tmp_c.Ym);
	fe_0(tmp_c.T2);

	// Save a double on the first iteration
	ge_zero(p);
	lookup_add(p, &tmp_c, tmp_a, tmp_b, b_comb_low , s_scalar, 31);
	lookup_add(p, &tmp_c, tmp_a, tmp_b, b_comb_high, s_scalar, 31+128);
	// Regular double & add for the rest
	for (int i = 30; i >= 0; i--) {
		ge_double(p, p, &tmp_d);
		lookup_add(p, &tmp_c, tmp_a, tmp_b, b_comb_low , s_scalar, i);
		lookup_add(p, &tmp_c, tmp_a, tmp_b, b_comb_high, s_scalar, i+128);
	}
	// Note: we could save one addition at the end if we assumed the
	// scalar fit in 252 bits.  Which it does in practice if it is
	// selected at random.  However, non-random, non-hashed scalars
	// *can* overflow 252 bits in practice.  Better account for that
	// than leaving that kind of subtle corner case.

	WIPE_BUFFER(tmp_a);  WIPE_CTX(&tmp_d);
	WIPE_BUFFER(tmp_b);  WIPE_CTX(&tmp_c);
	WIPE_BUFFER(s_scalar);
}

void crypto_eddsa_scalarbase(u8 point[32], const u8 scalar[32])
{
	ge P;
	ge_scalarmult_base(&P, scalar);
	ge_tobytes(point, &P);
	WIPE_CTX(&P);
}

int crypto_eddsa_scalarbase_checked(u8 point[32], const u8 scalar[32])
{
	int err = check_out_ptr(point);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(scalar, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_scalarbase(point, scalar);
	return CRYPTO_OK;
}

void crypto_eddsa_key_pair(u8 secret_key[64], u8 public_key[32], u8 seed[32])
{
	// To allow overlaps, observable writes happen in this order:
	// 1. seed
	// 2. secret_key
	// 3. public_key
	u8 a[64];
	COPY(a, seed, 32);
	crypto_wipe(seed, 32);
	COPY(secret_key, a, 32);
	crypto_blake2b(a, 64, a, 32);
	crypto_eddsa_trim_scalar(a, a);
	crypto_eddsa_scalarbase(secret_key + 32, a);
	COPY(public_key, secret_key + 32, 32);
	WIPE_BUFFER(a);
}

int crypto_eddsa_key_pair_checked(u8 secret_key[64],
                                  u8 public_key[32],
                                  u8 seed[32])
{
	int err = check_out_ptr(secret_key);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(public_key);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(seed, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_key_pair(secret_key, public_key, seed);
	return CRYPTO_OK;
}

static void hash_reduce(u8 h[32],
                        const u8 *a, size_t a_size,
                        const u8 *b, size_t b_size,
                        const u8 *c, size_t c_size)
{
	u8 hash[64];
	crypto_blake2b_ctx ctx;
	crypto_blake2b_init  (&ctx, 64);
	crypto_blake2b_update(&ctx, a, a_size);
	crypto_blake2b_update(&ctx, b, b_size);
	crypto_blake2b_update(&ctx, c, c_size);
	crypto_blake2b_final (&ctx, hash);
	crypto_eddsa_reduce(h, hash);
}

// Digital signature of a message with from a secret key.
//
// The secret key comprises two parts:
// - The seed that generates the key (secret_key[ 0..31])
// - The public key                  (secret_key[32..63])
//
// The seed and the public key are bundled together to make sure users
// don't use mismatched seeds and public keys, which would instantly
// leak the secret scalar and allow forgeries (allowing this to happen
// has resulted in critical vulnerabilities in the wild).
//
// The seed is hashed to derive the secret scalar and a secret prefix.
// The sole purpose of the prefix is to generate a secret random nonce.
// The properties of that nonce must be as follows:
// - Unique: we need a different one for each message.
// - Secret: third parties must not be able to predict it.
// - Random: any detectable bias would break all security.
//
// There are two ways to achieve these properties.  The obvious one is
// to simply generate a random number.  Here that would be a parameter
// (Monocypher doesn't have an RNG).  It works, but then users may reuse
// the nonce by accident, which _also_ leaks the secret scalar and
// allows forgeries.  This has happened in the wild too.
//
// This is no good, so instead we generate that nonce deterministically
// by reducing modulo L a hash of the secret prefix and the message.
// The secret prefix makes the nonce unpredictable, the message makes it
// unique, and the hash/reduce removes all bias.
//
// The cost of that safety is hashing the message twice.  If that cost
// is unacceptable, there are two alternatives:
//
// - Signing a hash of the message instead of the message itself.  This
//   is fine as long as the hash is collision resistant. It is not
//   compatible with existing "pure" signatures, but at least it's safe.
//
// - Using a random nonce.  Please exercise **EXTREME CAUTION** if you
//   ever do that.  It is absolutely **critical** that the nonce is
//   really an unbiased random number between 0 and L-1, never reused,
//   and wiped immediately.
//
//   To lower the likelihood of complete catastrophe if the RNG is
//   either flawed or misused, you can hash the RNG output together with
//   the secret prefix and the beginning of the message, and use the
//   reduction of that hash instead of the RNG output itself.  It's not
//   foolproof (you'd need to hash the whole message) but it helps.
//
// Signing a message involves the following operations:
//
//   scalar, prefix = HASH(secret_key)
//   r              = HASH(prefix || message) % L
//   R              = [r]B
//   h              = HASH(R || public_key || message) % L
//   S              = ((h * a) + r) % L
//   signature      = R || S
void crypto_eddsa_sign(u8 signature [64], const u8 secret_key[64],
                       const u8 *message, size_t message_size)
{
	u8 a[64];  // secret scalar and prefix
	u8 r[32];  // secret deterministic "random" nonce
	u8 h[32];  // publically verifiable hash of the message (not wiped)
	u8 R[32];  // first half of the signature (allows overlapping inputs)

	crypto_blake2b(a, 64, secret_key, 32);
	crypto_eddsa_trim_scalar(a, a);
	hash_reduce(r, a + 32, 32, message, message_size, 0, 0);
	crypto_eddsa_scalarbase(R, r);
	hash_reduce(h, R, 32, secret_key + 32, 32, message, message_size);
	COPY(signature, R, 32);
	crypto_eddsa_mul_add(signature + 32, h, a, r);

	WIPE_BUFFER(a);
	WIPE_BUFFER(r);
}

int crypto_eddsa_sign_checked(u8 signature[64], const u8 secret_key[64],
                              const u8 *message, size_t message_size)
{
	int err = check_out_ptr(signature);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(secret_key, 64);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_sign(signature, secret_key, message, message_size);
	return CRYPTO_OK;
}

// To check the signature R, S of the message M with the public key A,
// there are 3 steps:
//
//   compute h = HASH(R || A || message) % L
//   check that A is on the curve.
//   check that R == [s]B - [h]A
//
// The last two steps are done in crypto_eddsa_check_equation()
int crypto_eddsa_check(const u8  signature[64], const u8 public_key[32],
                       const u8 *message, size_t message_size)
{
	u8 h[32];
	hash_reduce(h, signature, 32, public_key, 32, message, message_size);
	return crypto_eddsa_check_equation(signature, public_key, h);
}

int crypto_eddsa_check_checked(const u8 signature[64],
                               const u8 public_key[32],
                               const u8 *message, size_t message_size)
{
	int err = check_ptr(signature, 64);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(public_key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(message, message_size);
	if (err != CRYPTO_OK) { return err; }
	return crypto_eddsa_check(signature, public_key, message, message_size)
		? CRYPTO_ERR_AUTH
		: CRYPTO_OK;
}

/////////////////////////
/// EdDSA <--> X25519 ///
/////////////////////////
void crypto_eddsa_to_x25519(u8 x25519[32], const u8 eddsa[32])
{
	// (u, v) = ((1+y)/(1-y), sqrt(-486664)*u/x)
	// Only converting y to u, the sign of x is ignored.
	fe t1, t2;
	fe_frombytes(t2, eddsa);
	fe_add(t1, fe_one, t2);
	fe_sub(t2, fe_one, t2);
	fe_invert(t2, t2);
	fe_mul(t1, t1, t2);
	fe_tobytes(x25519, t1);
	WIPE_BUFFER(t1);
	WIPE_BUFFER(t2);
}

int crypto_eddsa_to_x25519_checked(u8 x25519[32], const u8 eddsa[32])
{
	int err = check_out_ptr(x25519);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(eddsa, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_eddsa_to_x25519(x25519, eddsa);
	return CRYPTO_OK;
}

void crypto_x25519_to_eddsa(u8 eddsa[32], const u8 x25519[32])
{
	// (x, y) = (sqrt(-486664)*u/v, (u-1)/(u+1))
	// Only converting u to y, x is assumed positive.
	fe t1, t2;
	fe_frombytes(t2, x25519);
	fe_sub(t1, t2, fe_one);
	fe_add(t2, t2, fe_one);
	fe_invert(t2, t2);
	fe_mul(t1, t1, t2);
	fe_tobytes(eddsa, t1);
	WIPE_BUFFER(t1);
	WIPE_BUFFER(t2);
}

int crypto_x25519_to_eddsa_checked(u8 eddsa[32], const u8 x25519[32])
{
	int err = check_out_ptr(eddsa);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(x25519, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_x25519_to_eddsa(eddsa, x25519);
	return CRYPTO_OK;
}

/////////////////////////////////////////////
/// Dirty ephemeral public key generation ///
/////////////////////////////////////////////

// Those functions generates a public key, *without* clearing the
// cofactor.  Sending that key over the network leaks 3 bits of the
// private key.  Use only to generate ephemeral keys that will be hidden
// with crypto_curve_to_hidden().
//
// The public key is otherwise compatible with crypto_x25519(), which
// properly clears the cofactor.
//
// Note that the distribution of the resulting public keys is almost
// uniform.  Flipping the sign of the v coordinate (not provided by this
// function), covers the entire key space almost perfectly, where
// "almost" means a 2^-128 bias (undetectable).  This uniformity is
// needed to ensure the proper randomness of the resulting
// representatives (once we apply crypto_curve_to_hidden()).
//
// Recall that Curve25519 has order C = 2^255 + e, with e < 2^128 (not
// to be confused with the prime order of the main subgroup, L, which is
// 8 times less than that).
//
// Generating all points would require us to multiply a point of order C
// (the base point plus any point of order 8) by all scalars from 0 to
// C-1.  Clamping limits us to scalars between 2^254 and 2^255 - 1. But
// by negating the resulting point at random, we also cover scalars from
// -2^255 + 1 to -2^254 (which modulo C is congruent to e+1 to 2^254 + e).
//
// In practice:
// - Scalars from 0         to e + 1     are never generated
// - Scalars from 2^255     to 2^255 + e are never generated
// - Scalars from 2^254 + 1 to 2^254 + e are generated twice
//
// Since e < 2^128, detecting this bias requires observing over 2^100
// representatives from a given source (this will never happen), *and*
// recovering enough of the private key to determine that they do, or do
// not, belong to the biased set (this practically requires solving
// discrete logarithm, which is conjecturally intractable).
//
// In practice, this means the bias is impossible to detect.

// s + (x*L) % 8*L
// Guaranteed to fit in 256 bits iff s fits in 255 bits.
//   L             < 2^253
//   x%8           < 2^3
//   L * (x%8)     < 2^255
//   s             < 2^255
//   s + L * (x%8) < 2^256
static void add_xl(u8 s[32], u8 x)
{
	u64 mod8  = x & 7;
	u64 carry = 0;
	FOR (i , 0, 8) {
		carry = carry + load32_le(s + 4*i) + L[i] * mod8;
		store32_le(s + 4*i, (u32)carry);
		carry >>= 32;
	}
}

// "Small" dirty ephemeral key.
// Use if you need to shrink the size of the binary, and can afford to
// slow down by a factor of two (compared to the fast version)
//
// This version works by decoupling the cofactor from the main factor.
//
// - The trimmed scalar determines the main factor
// - The clamped bits of the scalar determine the cofactor.
//
// Cofactor and main factor are combined into a single scalar, which is
// then multiplied by a point of order 8*L (unlike the base point, which
// has prime order).  That "dirty" base point is the addition of the
// regular base point (9), and a point of order 8.
void crypto_x25519_dirty_small(u8 public_key[32], const u8 secret_key[32])
{
	// Base point of order 8*L
	// Raw scalar multiplication with it does not clear the cofactor,
	// and the resulting public key will reveal 3 bits of the scalar.
	//
	// The low order component of this base point  has been chosen
	// to yield the same results as crypto_x25519_dirty_fast().
	static const u8 dirty_base_point[32] = {
		0xd8, 0x86, 0x1a, 0xa2, 0x78, 0x7a, 0xd9, 0x26,
		0x8b, 0x74, 0x74, 0xb6, 0x82, 0xe3, 0xbe, 0xc3,
		0xce, 0x36, 0x9a, 0x1e, 0x5e, 0x31, 0x47, 0xa2,
		0x6d, 0x37, 0x7c, 0xfd, 0x20, 0xb5, 0xdf, 0x75,
	};
	// separate the main factor & the cofactor of the scalar
	u8 scalar[32];
	crypto_eddsa_trim_scalar(scalar, secret_key);

	// Separate the main factor and the cofactor
	//
	// The scalar is trimmed, so its cofactor is cleared.  The three
	// least significant bits however still have a main factor.  We must
	// remove it for X25519 compatibility.
	//
	//   cofactor = lsb * L            (modulo 8*L)
	//   combined = scalar + cofactor  (modulo 8*L)
	add_xl(scalar, secret_key[0]);
	scalarmult(public_key, scalar, dirty_base_point, 256);
	WIPE_BUFFER(scalar);
}

// Select low order point
// We're computing the [cofactor]lop scalar multiplication, where:
//
//   cofactor = tweak & 7.
//   lop      = (lop_x, lop_y)
//   lop_x    = sqrt((sqrt(d + 1) + 1) / d)
//   lop_y    = -lop_x * sqrtm1
//
// The low order point has order 8. There are 4 such points.  We've
// chosen the one whose both coordinates are positive (below p/2).
// The 8 low order points are as follows:
//
// [0]lop = ( 0       ,  1    )
// [1]lop = ( lop_x   ,  lop_y)
// [2]lop = ( sqrt(-1), -0    )
// [3]lop = ( lop_x   , -lop_y)
// [4]lop = (-0       , -1    )
// [5]lop = (-lop_x   , -lop_y)
// [6]lop = (-sqrt(-1),  0    )
// [7]lop = (-lop_x   ,  lop_y)
//
// The x coordinate is either 0, sqrt(-1), lop_x, or their opposite.
// The y coordinate is either 0,      -1 , lop_y, or their opposite.
// The pattern for both is the same, except for a rotation of 2 (modulo 8)
//
// This helper function captures the pattern, and we can use it thus:
//
//    select_lop(x, lop_x, sqrtm1, cofactor);
//    select_lop(y, lop_y, fe_one, cofactor + 2);
//
// This is faster than an actual scalar multiplication,
// and requires less code than naive constant time look up.
static void select_lop(fe out, const fe x, const fe k, u8 cofactor)
{
	fe tmp;
	fe_0(out);
	fe_ccopy(out, k  , (cofactor >> 1) & 1); // bit 1
	fe_ccopy(out, x  , (cofactor >> 0) & 1); // bit 0
	fe_neg  (tmp, out);
	fe_ccopy(out, tmp, (cofactor >> 2) & 1); // bit 2
	WIPE_BUFFER(tmp);
}

// "Fast" dirty ephemeral key
// We use this one by default.
//
// This version works by performing a regular scalar multiplication,
// then add a low order point.  The scalar multiplication is done in
// Edwards space for more speed (*2 compared to the "small" version).
// The cost is a bigger binary for programs that don't also sign messages.
void crypto_x25519_dirty_fast(u8 public_key[32], const u8 secret_key[32])
{
	// Compute clean scalar multiplication
	u8 scalar[32];
	ge pk;
	crypto_eddsa_trim_scalar(scalar, secret_key);
	ge_scalarmult_base(&pk, scalar);

	// Compute low order point
	fe t1, t2;
	select_lop(t1, lop_x, sqrtm1, secret_key[0]);
	select_lop(t2, lop_y, fe_one, secret_key[0] + 2);
	ge_precomp low_order_point;
	fe_add(low_order_point.Yp, t2, t1);
	fe_sub(low_order_point.Ym, t2, t1);
	fe_mul(low_order_point.T2, t2, t1);
	fe_mul(low_order_point.T2, low_order_point.T2, D2);

	// Add low order point to the public key
	ge_madd(&pk, &pk, &low_order_point, t1, t2);

	// Convert to Montgomery u coordinate (we ignore the sign)
	fe_add(t1, pk.Z, pk.Y);
	fe_sub(t2, pk.Z, pk.Y);
	fe_invert(t2, t2);
	fe_mul(t1, t1, t2);

	fe_tobytes(public_key, t1);

	WIPE_BUFFER(t1);    WIPE_CTX(&pk);
	WIPE_BUFFER(t2);    WIPE_CTX(&low_order_point);
	WIPE_BUFFER(scalar);
}

int crypto_x25519_dirty_small_checked(u8 pk[32], const u8 sk[32])
{
	int err = check_out_ptr(pk);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(sk, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_x25519_dirty_small(pk, sk);
	return CRYPTO_OK;
}

int crypto_x25519_dirty_fast_checked(u8 pk[32], const u8 sk[32])
{
	int err = check_out_ptr(pk);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(sk, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_x25519_dirty_fast(pk, sk);
	return CRYPTO_OK;
}

///////////////////
/// Elligator 2 ///
///////////////////
static const fe A = {486662};

// Elligator direct map
//
// Computes the point corresponding to a representative, encoded in 32
// bytes (little Endian).  Since positive representatives fits in 254
// bits, The two most significant bits are ignored.
//
// From the paper:
// w = -A / (fe(1) + non_square * r^2)
// e = chi(w^3 + A*w^2 + w)
// u = e*w - (fe(1)-e)*(A//2)
// v = -e * sqrt(u^3 + A*u^2 + u)
//
// We ignore v because we don't need it for X25519 (the Montgomery
// ladder only uses u).
//
// Note that e is either 0, 1 or -1
// if e = 0    u = 0  and v = 0
// if e = 1    u = w
// if e = -1   u = -w - A = w * non_square * r^2
//
// Let r1 = non_square * r^2
// Let r2 = 1 + r1
// Note that r2 cannot be zero, -1/non_square is not a square.
// We can (tediously) verify that:
//   w^3 + A*w^2 + w = (A^2*r1 - r2^2) * A / r2^3
// Therefore:
//   chi(w^3 + A*w^2 + w) = chi((A^2*r1 - r2^2) * (A / r2^3))
//   chi(w^3 + A*w^2 + w) = chi((A^2*r1 - r2^2) * (A / r2^3)) * 1
//   chi(w^3 + A*w^2 + w) = chi((A^2*r1 - r2^2) * (A / r2^3)) * chi(r2^6)
//   chi(w^3 + A*w^2 + w) = chi((A^2*r1 - r2^2) * (A / r2^3)  *     r2^6)
//   chi(w^3 + A*w^2 + w) = chi((A^2*r1 - r2^2) *  A * r2^3)
// Corollary:
//   e =  1 if (A^2*r1 - r2^2) *  A * r2^3) is a non-zero square
//   e = -1 if (A^2*r1 - r2^2) *  A * r2^3) is not a square
//   Note that w^3 + A*w^2 + w (and therefore e) can never be zero:
//     w^3 + A*w^2 + w = w * (w^2 + A*w + 1)
//     w^3 + A*w^2 + w = w * (w^2 + A*w + A^2/4 - A^2/4 + 1)
//     w^3 + A*w^2 + w = w * (w + A/2)^2        - A^2/4 + 1)
//     which is zero only if:
//       w = 0                   (impossible)
//       (w + A/2)^2 = A^2/4 - 1 (impossible, because A^2/4-1 is not a square)
//
// Let isr   = invsqrt((A^2*r1 - r2^2) *  A * r2^3)
//     isr   = sqrt(1        / ((A^2*r1 - r2^2) *  A * r2^3)) if e =  1
//     isr   = sqrt(sqrt(-1) / ((A^2*r1 - r2^2) *  A * r2^3)) if e = -1
//
// if e = 1
//   let u1 = -A * (A^2*r1 - r2^2) * A * r2^2 * isr^2
//       u1 = w
//       u1 = u
//
// if e = -1
//   let ufactor = -non_square * sqrt(-1) * r^2
//   let vfactor = sqrt(ufactor)
//   let u2 = -A * (A^2*r1 - r2^2) * A * r2^2 * isr^2 * ufactor
//       u2 = w * -1 * -non_square * r^2
//       u2 = w * non_square * r^2
//       u2 = u
void crypto_elligator_map(u8 curve[32], const u8 hidden[32])
{
	fe r, u, t1, t2, t3;
	fe_frombytes_mask(r, hidden, 2); // r is encoded in 254 bits.
	fe_sq(r, r);
	fe_add(t1, r, r);
	fe_add(u, t1, fe_one);
	fe_sq (t2, u);
	fe_mul(t3, A2, t1);
	fe_sub(t3, t3, t2);
	fe_mul(t3, t3, A);
	fe_mul(t1, t2, u);
	fe_mul(t1, t3, t1);
	int is_square = invsqrt(t1, t1);
	fe_mul(u, r, ufactor);
	fe_ccopy(u, fe_one, is_square);
	fe_sq (t1, t1);
	fe_mul(u, u, A);
	fe_mul(u, u, t3);
	fe_mul(u, u, t2);
	fe_mul(u, u, t1);
	fe_neg(u, u);
	fe_tobytes(curve, u);

	WIPE_BUFFER(t1);  WIPE_BUFFER(r);
	WIPE_BUFFER(t2);  WIPE_BUFFER(u);
	WIPE_BUFFER(t3);
}

int crypto_elligator_map_checked(u8 curve[32], const u8 hidden[32])
{
	int err = check_out_ptr(curve);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(hidden, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_elligator_map(curve, hidden);
	return CRYPTO_OK;
}

// Elligator inverse map
//
// Computes the representative of a point, if possible.  If not, it does
// nothing and returns -1.  Note that the success of the operation
// depends only on the point (more precisely its u coordinate).  The
// tweak parameter is used only upon success
//
// The tweak should be a random byte.  Beyond that, its contents are an
// implementation detail. Currently, the tweak comprises:
// - Bit  1  : sign of the v coordinate (0 if positive, 1 if negative)
// - Bit  2-5: not used
// - Bits 6-7: random padding
//
// From the paper:
// Let sq = -non_square * u * (u+A)
// if sq is not a square, or u = -A, there is no mapping
// Assuming there is a mapping:
//    if v is positive: r = sqrt(-u     / (non_square * (u+A)))
//    if v is negative: r = sqrt(-(u+A) / (non_square * u    ))
//
// We compute isr = invsqrt(-non_square * u * (u+A))
// if it wasn't a square, abort.
// else, isr = sqrt(-1 / (non_square * u * (u+A))
//
// If v is positive, we return isr * u:
//   isr * u = sqrt(-1 / (non_square * u * (u+A)) * u
//   isr * u = sqrt(-u / (non_square * (u+A))
//
// If v is negative, we return isr * (u+A):
//   isr * (u+A) = sqrt(-1     / (non_square * u * (u+A)) * (u+A)
//   isr * (u+A) = sqrt(-(u+A) / (non_square * u)
int crypto_elligator_rev(u8 hidden[32], const u8 public_key[32], u8 tweak)
{
	fe t1, t2, t3;
	fe_frombytes(t1, public_key);    // t1 = u

	fe_add(t2, t1, A);               // t2 = u + A
	fe_mul(t3, t1, t2);
	fe_mul_small(t3, t3, -2);
	int is_square = invsqrt(t3, t3); // t3 = sqrt(-1 / non_square * u * (u+A))
	if (is_square) {
		// The only variable time bit.  This ultimately reveals how many
		// tries it took us to find a representable key.
		// This does not affect security as long as we try keys at random.

		fe_ccopy    (t1, t2, tweak & 1); // multiply by u if v is positive,
		fe_mul      (t3, t1, t3);        // multiply by u+A otherwise
		fe_mul_small(t1, t3, 2);
		fe_neg      (t2, t3);
		fe_ccopy    (t3, t2, fe_isodd(t1));
		fe_tobytes(hidden, t3);

		// Pad with two random bits
		hidden[31] |= tweak & 0xc0;
	}

	WIPE_BUFFER(t1);
	WIPE_BUFFER(t2);
	WIPE_BUFFER(t3);
	return is_square - 1;
}

int crypto_elligator_rev_checked(u8 hidden[32], const u8 public_key[32],
                                 u8 tweak)
{
	int err = check_out_ptr(hidden);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(public_key, 32);
	if (err != CRYPTO_OK) { return err; }
	return crypto_elligator_rev(hidden, public_key, tweak)
		? CRYPTO_ERR_AUTH
		: CRYPTO_OK;
}

void crypto_elligator_key_pair(u8 hidden[32], u8 secret_key[32], u8 seed[32])
{
	u8 pk [32]; // public key
	u8 buf[64]; // seed + representative
	COPY(buf + 32, seed, 32);
	do {
		crypto_chacha20_djb(buf, 0, 64, buf+32, zero, 0);
		crypto_x25519_dirty_fast(pk, buf); // or the "small" version
	} while(crypto_elligator_rev(buf+32, pk, buf[32]));
	// Note that the return value of crypto_elligator_rev() is
	// independent from its tweak parameter.
	// Therefore, buf[32] is not actually reused.  Either we loop one
	// more time and buf[32] is used for the new seed, or we succeeded,
	// and buf[32] becomes the tweak parameter.

	crypto_wipe(seed, 32);
	COPY(hidden    , buf + 32, 32);
	COPY(secret_key, buf     , 32);
	WIPE_BUFFER(buf);
	WIPE_BUFFER(pk);
}

int crypto_elligator_key_pair_checked(u8 hidden[32],
                                      u8 secret_key[32],
                                      u8 seed[32])
{
	int err = check_out_ptr(hidden);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(secret_key);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(seed, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_elligator_key_pair(hidden, secret_key, seed);
	return CRYPTO_OK;
}

///////////////////////
/// Scalar division ///
///////////////////////

// Montgomery reduction.
// Divides x by (2^256), and reduces the result modulo L
//
// Precondition:
//   x < L * 2^256
// Constants:
//   r = 2^256                 (makes division by r trivial)
//   k = (r * (1/r) - 1) // L  (1/r is computed modulo L   )
// Algorithm:
//   s = (x * k) % r
//   t = x + s*L      (t is always a multiple of r)
//   u = (t/r) % L    (u is always below 2*L, conditional subtraction is enough)
static void redc(u32 u[8], u32 x[16])
{
	static const u32 k[8] = {
		0x12547e1b, 0xd2b51da3, 0xfdba84ff, 0xb1a206f2,
		0xffa36bea, 0x14e75438, 0x6fe91836, 0x9db6c6f2,
	};

	// s = x * k (modulo 2^256)
	// This is cheaper than the full multiplication.
	u32 s[8] = {0};
	FOR (i, 0, 8) {
		u64 carry = 0;
		FOR (j, 0, 8-i) {
			carry  += s[i+j] + (u64)x[i] * k[j];
			s[i+j]  = (u32)carry;
			carry >>= 32;
		}
	}
	u32 t[16] = {0};
	multiply(t, s, L);

	// t = t + x
	u64 carry = 0;
	FOR (i, 0, 16) {
		carry  += (u64)t[i] + x[i];
		t[i]    = (u32)carry;
		carry >>= 32;
	}

	// u = (t / 2^256) % L
	// Note that t / 2^256 is always below 2*L,
	// So a constant time conditional subtraction is enough
	remove_l(u, t+8);

	WIPE_BUFFER(s);
	WIPE_BUFFER(t);
}

void crypto_x25519_inverse(u8 blind_salt [32], const u8 private_key[32],
                           const u8 curve_point[32])
{
	static const  u8 Lm2[32] = { // L - 2
		0xeb, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58,
		0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
	};
	// 1 in Montgomery form
	u32 m_inv [8] = {
		0x8d98951d, 0xd6ec3174, 0x737dcf70, 0xc6ef5bf4,
		0xfffffffe, 0xffffffff, 0xffffffff, 0x0fffffff,
	};

	u8 scalar[32];
	crypto_eddsa_trim_scalar(scalar, private_key);

	// Convert the scalar in Montgomery form
	// m_scl = scalar * 2^256 (modulo L)
	u32 m_scl[8];
	{
		u32 tmp[16];
		ZERO(tmp, 8);
		load32_le_buf(tmp+8, scalar, 8);
		mod_l(scalar, tmp);
		load32_le_buf(m_scl, scalar, 8);
		WIPE_BUFFER(tmp); // Wipe ASAP to save stack space
	}

	// Compute the inverse
	u32 product[16];
	for (int i = 252; i >= 0; i--) {
		ZERO(product, 16);
		multiply(product, m_inv, m_inv);
		redc(m_inv, product);
		if (scalar_bit(Lm2, i)) {
			ZERO(product, 16);
			multiply(product, m_inv, m_scl);
			redc(m_inv, product);
		}
	}
	// Convert the inverse *out* of Montgomery form
	// scalar = m_inv / 2^256 (modulo L)
	COPY(product, m_inv, 8);
	ZERO(product + 8, 8);
	redc(m_inv, product);
	store32_le_buf(scalar, m_inv, 8); // the *inverse* of the scalar

	// Clear the cofactor of scalar:
	//   cleared = scalar * (3*L + 1)      (modulo 8*L)
	//   cleared = scalar + scalar * 3 * L (modulo 8*L)
	// Note that (scalar * 3) is reduced modulo 8, so we only need the
	// first byte.
	add_xl(scalar, scalar[0] * 3);

	// Recall that 8*L < 2^256. However it is also very close to
	// 2^255. If we spanned the ladder over 255 bits, random tests
	// wouldn't catch the off-by-one error.
	scalarmult(blind_salt, scalar, curve_point, 256);

	WIPE_BUFFER(scalar);   WIPE_BUFFER(m_scl);
	WIPE_BUFFER(product);  WIPE_BUFFER(m_inv);
}

int crypto_x25519_inverse_checked(u8 blind_salt[32],
                                  const u8 private_key[32],
                                  const u8 curve_point[32])
{
	int err = check_out_ptr(blind_salt);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(private_key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(curve_point, 32);
	if (err != CRYPTO_OK) { return err; }
	crypto_x25519_inverse(blind_salt, private_key, curve_point);
	return CRYPTO_OK;
}

////////////////////////////////
/// Authenticated encryption ///
////////////////////////////////
static void lock_auth(u8 mac[16], const u8  auth_key[32],
                      const u8 *ad         , size_t ad_size,
                      const u8 *cipher_text, size_t text_size)
{
	u8 sizes[16]; // Not secret, not wiped
	store64_le(sizes + 0, ad_size);
	store64_le(sizes + 8, text_size);
	crypto_poly1305_ctx poly_ctx;           // auto wiped...
	crypto_poly1305_init  (&poly_ctx, auth_key);
	crypto_poly1305_update(&poly_ctx, ad         , ad_size);
	crypto_poly1305_update(&poly_ctx, zero       , gap(ad_size, 16));
	crypto_poly1305_update(&poly_ctx, cipher_text, text_size);
	crypto_poly1305_update(&poly_ctx, zero       , gap(text_size, 16));
	crypto_poly1305_update(&poly_ctx, sizes      , 16);
	crypto_poly1305_final (&poly_ctx, mac); // ...here
}

void crypto_aead_init_x(crypto_aead_ctx *ctx,
                        u8 const key[32], const u8 nonce[24])
{
	crypto_chacha20_h(ctx->key, key, nonce);
	COPY(ctx->nonce, nonce + 16, 8);
	ctx->counter = 0;
}

void crypto_aead_init_djb(crypto_aead_ctx *ctx,
                          const u8 key[32], const u8 nonce[8])
{
	COPY(ctx->key  , key  , 32);
	COPY(ctx->nonce, nonce,  8);
	ctx->counter = 0;
}

void crypto_aead_init_ietf(crypto_aead_ctx *ctx,
                           const u8 key[32], const u8 nonce[12])
{
	COPY(ctx->key  , key      , 32);
	COPY(ctx->nonce, nonce + 4,  8);
	ctx->counter = (u64)load32_le(nonce) << 32;
}

int crypto_aead_init_x_checked(crypto_aead_ctx *ctx,
                               const u8 key[32], const u8 nonce[24])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 24);
	if (err != CRYPTO_OK) { return err; }
	crypto_aead_init_x(ctx, key, nonce);
	return CRYPTO_OK;
}

int crypto_aead_init_djb_checked(crypto_aead_ctx *ctx,
                                 const u8 key[32], const u8 nonce[8])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 8);
	if (err != CRYPTO_OK) { return err; }
	crypto_aead_init_djb(ctx, key, nonce);
	return CRYPTO_OK;
}

int crypto_aead_init_ietf_checked(crypto_aead_ctx *ctx,
                                  const u8 key[32], const u8 nonce[12])
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 12);
	if (err != CRYPTO_OK) { return err; }
	crypto_aead_init_ietf(ctx, key, nonce);
	return CRYPTO_OK;
}

void crypto_aead_write(crypto_aead_ctx *ctx, u8 *cipher_text, u8 mac[16],
                       const u8 *ad,         size_t ad_size,
                       const u8 *plain_text, size_t text_size)
{
	u8 auth_key[64]; // the last 32 bytes are used for rekeying.
	crypto_chacha20_djb(auth_key, 0, 64, ctx->key, ctx->nonce, ctx->counter);
	crypto_chacha20_djb(cipher_text, plain_text, text_size,
	                    ctx->key, ctx->nonce, ctx->counter + 1);
	lock_auth(mac, auth_key, ad, ad_size, cipher_text, text_size);
	COPY(ctx->key, auth_key + 32, 32);
	WIPE_BUFFER(auth_key);
}

int crypto_aead_read(crypto_aead_ctx *ctx, u8 *plain_text, const u8 mac[16],
                     const u8 *ad,          size_t ad_size,
                     const u8 *cipher_text, size_t text_size)
{
	u8 auth_key[64]; // the last 32 bytes are used for rekeying.
	u8 real_mac[16];
	crypto_chacha20_djb(auth_key, 0, 64, ctx->key, ctx->nonce, ctx->counter);
	lock_auth(real_mac, auth_key, ad, ad_size, cipher_text, text_size);
	int mismatch = crypto_verify16(mac, real_mac);
	if (!mismatch) {
		crypto_chacha20_djb(plain_text, cipher_text, text_size,
		                    ctx->key, ctx->nonce, ctx->counter + 1);
		COPY(ctx->key, auth_key + 32, 32);
	}
	WIPE_BUFFER(auth_key);
	WIPE_BUFFER(real_mac);
	return mismatch;
}

int crypto_aead_read_safe(crypto_aead_ctx *ctx, u8 *plain_text,
                          const u8 mac[16], const u8 *ad, size_t ad_size,
                          const u8 *cipher_text, size_t text_size)
{
	int mismatch = crypto_aead_read(ctx, plain_text, mac, ad, ad_size,
	                                cipher_text, text_size);
	if (mismatch) {
		crypto_wipe(plain_text, text_size);
	}
	return mismatch;
}

void crypto_aead_lock(u8 *cipher_text, u8 mac[16], const u8 key[32],
                      const u8  nonce[24], const u8 *ad, size_t ad_size,
                      const u8 *plain_text, size_t text_size)
{
	crypto_aead_ctx ctx;
	crypto_aead_init_x(&ctx, key, nonce);
	crypto_aead_write(&ctx, cipher_text, mac, ad, ad_size,
	                  plain_text, text_size);
	crypto_wipe(&ctx, sizeof(ctx));
}

int crypto_aead_unlock(u8 *plain_text, const u8  mac[16], const u8 key[32],
                       const u8 nonce[24], const u8 *ad, size_t ad_size,
                       const u8 *cipher_text, size_t text_size)
{
	crypto_aead_ctx ctx;
	crypto_aead_init_x(&ctx, key, nonce);
	int mismatch = crypto_aead_read(&ctx, plain_text, mac, ad, ad_size,
	                                cipher_text, text_size);
	crypto_wipe(&ctx, sizeof(ctx));
	return mismatch;
}

int crypto_aead_unlock_safe(u8 *plain_text, const u8 mac[16], const u8 key[32],
                            const u8 nonce[24], const u8 *ad, size_t ad_size,
                            const u8 *cipher_text, size_t text_size)
{
	int mismatch = crypto_aead_unlock(plain_text, mac, key, nonce, ad,
	                                  ad_size, cipher_text, text_size);
	if (mismatch) {
		crypto_wipe(plain_text, text_size);
	}
	return mismatch;
}

int crypto_aead_lock_checked(u8 *cipher_text, u8 mac[16],
                             const u8 key[32], const u8 nonce[24],
                             const u8 *ad, size_t ad_size,
                             const u8 *plain_text, size_t text_size)
{
	int err = check_out_ptr(cipher_text);
	if (err != CRYPTO_OK) { return err; }
	err = check_out_ptr(mac);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 24);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(ad, ad_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(plain_text, text_size);
	if (err != CRYPTO_OK) { return err; }
	crypto_aead_lock(cipher_text, mac, key, nonce, ad, ad_size,
	                 plain_text, text_size);
	return CRYPTO_OK;
}

int crypto_aead_unlock_checked(u8 *plain_text, const u8 mac[16],
                               const u8 key[32], const u8 nonce[24],
                               const u8 *ad, size_t ad_size,
                               const u8 *cipher_text, size_t text_size)
{
	int err = check_out_ptr(plain_text);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(mac, 16);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 24);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(ad, ad_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(cipher_text, text_size);
	if (err != CRYPTO_OK) { return err; }
	int mismatch = crypto_aead_unlock(plain_text, mac, key, nonce, ad,
	                                  ad_size, cipher_text, text_size);
	return mismatch ? CRYPTO_ERR_AUTH : CRYPTO_OK;
}

int crypto_aead_unlock_safe_checked(u8 *plain_text, const u8 mac[16],
                                    const u8 key[32], const u8 nonce[24],
                                    const u8 *ad, size_t ad_size,
                                    const u8 *cipher_text, size_t text_size)
{
	int err = check_out_ptr(plain_text);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(mac, 16);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(key, 32);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(nonce, 24);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(ad, ad_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(cipher_text, text_size);
	if (err != CRYPTO_OK) { return err; }
	int mismatch = crypto_aead_unlock(plain_text, mac, key, nonce, ad,
	                                  ad_size, cipher_text, text_size);
	if (mismatch) {
		crypto_wipe(plain_text, text_size);
		return CRYPTO_ERR_AUTH;
	}
	return CRYPTO_OK;
}

int crypto_aead_write_checked(crypto_aead_ctx *ctx, u8 *cipher_text,
                              u8 mac[16], const u8 *ad, size_t ad_size,
                              const u8 *plain_text, size_t text_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	if (text_size != 0 && cipher_text == 0) {
		return CRYPTO_ERR_NULL;
	}
	err = check_out_ptr(mac);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(ad, ad_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(plain_text, text_size);
	if (err != CRYPTO_OK) { return err; }
	u64 blocks = text_size / 64 + (text_size % 64 != 0);
	u64 ctr1 = 0;
	err = checked_add_u64(ctx->counter, 1, &ctr1);
	if (err != CRYPTO_OK) { return err; }
	u64 end = 0;
	err = checked_add_u64(ctr1, blocks, &end);
	if (err != CRYPTO_OK) { return err; }
	crypto_aead_write(ctx, cipher_text, mac, ad, ad_size, plain_text, text_size);
	return CRYPTO_OK;
}

int crypto_aead_read_checked(crypto_aead_ctx *ctx, u8 *plain_text,
                             const u8 mac[16], const u8 *ad, size_t ad_size,
                             const u8 *cipher_text, size_t text_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	if (text_size != 0 && plain_text == 0) {
		return CRYPTO_ERR_NULL;
	}
	err = check_ptr(mac, 16);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(ad, ad_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(cipher_text, text_size);
	if (err != CRYPTO_OK) { return err; }
	u64 blocks = text_size / 64 + (text_size % 64 != 0);
	u64 ctr1 = 0;
	err = checked_add_u64(ctx->counter, 1, &ctr1);
	if (err != CRYPTO_OK) { return err; }
	u64 end = 0;
	err = checked_add_u64(ctr1, blocks, &end);
	if (err != CRYPTO_OK) { return err; }
	int mismatch = crypto_aead_read(ctx, plain_text, mac, ad, ad_size,
	                                cipher_text, text_size);
	return mismatch ? CRYPTO_ERR_AUTH : CRYPTO_OK;
}

int crypto_aead_read_safe_checked(crypto_aead_ctx *ctx, u8 *plain_text,
                                  const u8 mac[16], const u8 *ad,
                                  size_t ad_size,
                                  const u8 *cipher_text, size_t text_size)
{
	int err = check_out_ptr(ctx);
	if (err != CRYPTO_OK) { return err; }
	if (text_size != 0 && plain_text == 0) {
		return CRYPTO_ERR_NULL;
	}
	err = check_ptr(mac, 16);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(ad, ad_size);
	if (err != CRYPTO_OK) { return err; }
	err = check_ptr(cipher_text, text_size);
	if (err != CRYPTO_OK) { return err; }
	u64 blocks = text_size / 64 + (text_size % 64 != 0);
	u64 ctr1 = 0;
	err = checked_add_u64(ctx->counter, 1, &ctr1);
	if (err != CRYPTO_OK) { return err; }
	u64 end = 0;
	err = checked_add_u64(ctr1, blocks, &end);
	if (err != CRYPTO_OK) { return err; }
	int mismatch = crypto_aead_read(ctx, plain_text, mac, ad, ad_size,
	                                cipher_text, text_size);
	if (mismatch) {
		crypto_wipe(plain_text, text_size);
		return CRYPTO_ERR_AUTH;
	}
	return CRYPTO_OK;
}


#ifdef MONOCYPHER_CPP_NAMESPACE
}
#endif
