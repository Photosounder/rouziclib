#ifndef RL_EXCL_APPROX
#ifdef RL_INTEL_INTR

// All require AVX2
extern __m128 _mm_gaussian_d1_ps(__m128 x);
extern __m128 _mm_erfr_d1_ps(__m128 x);
extern __m128 _mm_frgb_to_srgb(__m128 x);

#endif
#endif
