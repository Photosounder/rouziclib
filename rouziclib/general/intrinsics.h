enum cpu_feat_n
{
	CPU_HAS_SSE2,
	CPU_HAS_SSE3,
	CPU_HAS_SSSE3,
	CPU_HAS_SSE4_1,
	CPU_HAS_SSE4_2,
	CPU_HAS_FMA,
	CPU_HAS_BMI1,
	CPU_HAS_BMI2,
	CPU_HAS_AVX,
	CPU_HAS_AVX2,
	CPU_HAS_AVX512F,
	CPU_HAS_AVX512PF,
	CPU_HAS_AVX512ER,
	CPU_HAS_AVX512CD,

	CPU_FEATURE_COUNT
};

#if (defined(_M_AMD64) || defined(_M_X64) || defined(__amd64) ) && !defined(__x86_64__)
	#define __x86_64__
#endif

#if defined(__SSE__) || defined(__SSE2__) || defined(__x86_64__)

#include <immintrin.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifndef RL_EXCL_INTEL_INTR
#define RL_INTEL_INTR
#endif

#ifdef __GNUC__
extern void __cpuid(int *cpuinfo, int info);
extern uint64_t rl_xgetbv(uint32_t index);
#else
#define rl_xgetbv _xgetbv
#endif

extern int check_cpuinfo(const enum cpu_feat_n fid);

#ifndef _mm_storeu_si32
extern void _mm_storeu_si32(void *mem_addr, __m128i a);  // SSE2
#endif

extern __m128 _mm_i32sgather_ps(float const *base_addr, __m128i vindex);  // SSE2

// SSE
#define _mm_abs_ps(v)	_mm_andnot_ps(_mm_set_ps1(-0.f), v)

#endif
