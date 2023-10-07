enum cpu_feat_n
{
	CPU_HAS_SSE2,
	CPU_HAS_SSE3,
	CPU_HAS_SSSE3,
	CPU_HAS_SSE4_1,
	CPU_HAS_SSE4_2,
	CPU_HAS_FMA,
	CPU_HAS_F16C,
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

extern int check_cpuinfo(const enum cpu_feat_n fid);

static int check_sse2()  { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_SSE2);	return v; }
static int check_sse3()  { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_SSE3);	return v; }
static int check_ssse3() { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_SSSE3);	return v; }
static int check_sse41() { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_SSE4_1);	return v; }
static int check_sse42() { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_SSE4_2);	return v; }
static int check_fma()   { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_FMA);	return v; }
static int check_f16c()  { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_F16C);	return v; }
static int check_avx()   { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_AVX);	return v; }
static int check_avx2()  { static int v=2; if (v==2) v = check_cpuinfo(CPU_HAS_AVX2);	return v; }

#if ( defined(_M_AMD64) || defined(__amd64) ) && !defined(__x86_64__)
  #define __x86_64__
#endif

#if defined(__SSE__) || defined(__SSE2__) || defined(__x86_64__) || _M_IX86_FP>0

  #include <immintrin.h>
  #ifdef _MSC_VER
    #include <intrin.h>
  #endif

  #ifdef __EMSCRIPTEN__
    #include <xmmintrin.h>
  #endif

  #ifndef RL_EXCL_INTEL_INTR
    #define RL_INTEL_INTR
  #endif

  #ifdef __GNUC__
    #ifndef __EMSCRIPTEN__
      extern void __cpuid(int *cpuinfo, int info);
      extern uint64_t rl_xgetbv(uint32_t index);
    #endif
  #else
    #define rl_xgetbv _xgetbv
  #endif

#endif

#ifdef RL_INTEL_INTR
      
  #ifdef __GNUC__
    #define GNU_SSE
  #endif

  #ifdef RL_STOREU_SI32
    extern void _mm_storeu_si32(void *mem_addr, __m128i a);  // SSE2
  #endif

  extern __m128 _mm_i32sgather_ps(float const *base_addr, __m128i vindex);   // SSE2
  extern __m128d _mm_i32sgather_pd(double const *base_addr, __m128i vindex); // SSE2
  extern __m128d _mm_i64sgather_pd(double const *base_addr, __m128i vindex); // SSE2

  // SSE
  #define _mm_abs_ps(v)		_mm_andnot_ps(_mm_set_ps1(-0.f), v)
  #define _mm_abs_pd(v)		_mm_andnot_pd(_mm_set_pd1(-0.f), v)
  #define _mm_neg_ps(v)		_mm_xor_ps(v, _mm_set_ps1(-0.f))
  #define _mm_clamp_ps(v)		_mm_min_ps(_mm_max_ps(v, _mm_setzero_ps()), _mm_set_ps1(1.f))
  #define _mm_hmul_sd(v)		_mm_mul_sd(v, _mm_shuffle_pd(v, v, 1))
  #define _mm_hlomul_ss(v)	_mm_mul_ss(v, _mm_shuffle_ps(v, v, 1))
  #define _mm_xy_to_ps(v)		_mm_cvtpd_ps(_mm_load_pd((double *) &v))
  #define _mm_xyi_to_ps(v)	_mm_set_ps(0.f, 0.f, v.y, v.x)

  // SSSE3
  #define _mm_cvtepu32_epi16(v)	_mm_shuffle_epi8(v, _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 13, 12, 9, 8, 5, 4, 1, 0))
  #define _mm_cvtepu32_epi8(v)	_mm_shuffle_epi8(v, _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0))

#endif

// _mm_pause() equivalent, adapted from https://github.com/libsdl-org/SDL/blob/main/include/SDL_atomic.h
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__i386__) || defined(__x86_64__))
  #define rl_cpu_pause() __asm__ __volatile__("pause\n")  /* Some assemblers can't do REP NOP, so go with PAUSE. */
#elif (defined(__arm__) && __ARM_ARCH__ >= 7) || defined(__aarch64__)
  #define rl_cpu_pause() __asm__ __volatile__("yield" ::: "memory")
#elif (defined(__powerpc__) || defined(__powerpc64__))
  #define rl_cpu_pause() __asm__ __volatile__("or 27,27,27");
#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
  #define rl_cpu_pause() _mm_pause()  /* this is actually "rep nop" and not a SIMD instruction. No inline asm in MSVC x86-64! */
#elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
  #define rl_cpu_pause() __yield()
#elif defined(__WATCOMC__) && defined(__386__)
  /* watcom assembler rejects PAUSE if CPU < i686, and it refuses REP NOP as an invalid combination. Hardcode the bytes.  */
  extern __inline void rl_cpu_pause(void);
  #pragma aux rl_cpu_pause = "db 0f3h,90h"
#else
  #define rl_cpu_pause()
#endif
