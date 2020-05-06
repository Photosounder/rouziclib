enum cpu_feat_n
{
	CPU_HAS_SSE2,
	CPU_HAS_SSE3,
	CPU_HAS_SSSE3,
	CPU_HAS_SSE4_1,
	CPU_HAS_SSE4_2,
	CPU_HAS_FMA,
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

#define RL_INTEL_INTR

#ifdef __GNUC__
extern void __cpuid(int *cpuinfo, int info);
extern uint64_t _xgetbv(uint32_t index);
#endif

extern int check_cpuinfo(const enum cpu_feat_n fid);

// AVX2 required
extern __m256i _mm256_shuffle32_epi8(__m256i reg, __m256i shuf);
extern __m256i _mm256_load_8xi8_as_8xi32(__int64 const *in);
extern __m256i _mm256_load_16xi8_as_16xi16(__m128i const *in);
extern __m256i _mm256_load_8xi16_as_8xi32(__m128i const *in);

#endif
