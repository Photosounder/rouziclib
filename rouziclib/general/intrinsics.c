#ifdef RL_INTEL_INTR

#ifdef __GNUC__
void __cpuid(int *cpuinfo, int info)
{
	__asm__ __volatile__(
		"xchg %%ebx, %%edi;"
		"cpuid;"
		"xchg %%ebx, %%edi;"
		:"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
		:"0" (info)
	);
}

uint64_t rl_xgetbv(uint32_t index)
{
	uint32_t eax, edx;
	__asm__ __volatile__(
		"xgetbv;"
		: "=a" (eax), "=d"(edx)
		: "c" (index)
	);
	return ((uint64_t) edx << 32) | eax;
}
#endif

int check_cpuinfo(const enum cpu_feat_n fid)
{
	static int info1[4], info7[4], init=1;
	int ret=0;

	if (init)
	{
		init = 0;
		__cpuid(info1, 1);
		__cpuid(info7, 7);
	}

	switch (fid)
	{
			case CPU_HAS_SSE2:	ret = info1[3] & (1 << 26);
		break;	case CPU_HAS_SSE3:	ret = info1[2] & (1 << 0);
		break;	case CPU_HAS_SSSE3:	ret = info1[2] & (1 << 9);
		break;	case CPU_HAS_SSE4_1:	ret = info1[2] & (1 << 19);
		break;	case CPU_HAS_SSE4_2:	ret = info1[2] & (1 << 20);
		break;	case CPU_HAS_FMA:	ret = info1[2] & (1 << 12);
		break;	case CPU_HAS_AVX512F:	ret = info7[1] & (1 << 16);
		break;	case CPU_HAS_AVX512PF:	ret = info7[1] & (1 << 26);
		break;	case CPU_HAS_AVX512ER:	ret = info7[1] & (1 << 27);
		break;	case CPU_HAS_AVX512CD:	ret = info7[1] & (1 << 28);
		break;

		case CPU_HAS_AVX:
			if (info1[2] & (1 << 28) && info1[2] & (1 << 27))
				ret = (rl_xgetbv(0) & 6) == 6;
			break;

		case CPU_HAS_AVX2:
			if (check_cpuinfo(CPU_HAS_AVX))
				ret = info7[1] & (1 << 5);
			break;
	}

	return ret!=0;
}

#ifdef __GNUC__
__attribute__((__target__("avx2")))
#endif
__m256i _mm256_shuffle32_epi8(__m256i reg, __m256i shuf)	// AVX2
{	// from https://stackoverflow.com/a/32535489/1675589
	__m256i regAll0 = _mm256_permute2x128_si256(reg, reg, 0x00);
	__m256i regAll1 = _mm256_permute2x128_si256(reg, reg, 0x11);
	__m256i resR0 = _mm256_shuffle_epi8(regAll0, shuf);
	__m256i resR1 = _mm256_shuffle_epi8(regAll1, shuf);
	__m256i res = _mm256_blendv_epi8(resR1, resR0, _mm256_cmpgt_epi8(_mm256_set1_epi8(16), shuf));
	return res;
}

#ifdef __GNUC__
__attribute__((__target__("avx2")))
#endif
__m256i _mm256_load_8xi8_as_8xi32(__int64 const *in)	// AVX2
{
	__m256i y0, y1, shuf_mask;

	// Load the 8 bytes into the lower 8 bytes of a __m256i
	y0 = _mm256_castsi128_si256(_mm_loadu_si64(in));

	// Shuffle mask to move the 8 bytes in the correct places
	shuf_mask = _mm256_set_epi8(-1, -1, -1, 7, -1, -1, -1, 6, -1, -1, -1, 5, -1, -1, -1, 4, -1, -1, -1, 3, -1, -1, -1, 2, -1, -1, -1, 1, -1, -1, -1, 0);

	// Shuffle
	y1 = _mm256_shuffle32_epi8(y0, shuf_mask);

	return y1;
}

#ifdef __GNUC__
//extern __m128i _mm_loadu_si32(void const* mem_addr);	// GCC bug?
#endif

#ifdef __GNUC__
__attribute__((__target__("ssse3")))
#endif
__m128i _mm_load_4xi8_as_4xi32(__int32 const *in)	// SSSE3
{
	__m128i y0, y1, shuf_mask;

	// Load the 4 bytes into the lower 4 bytes of a __m128i
	y0 = _mm_loadu_si32(in);

	// Shuffle mask to move the 8 bytes in the correct places
	shuf_mask = _mm_set_epi8(-1, -1, -1, 3, -1, -1, -1, 2, -1, -1, -1, 1, -1, -1, -1, 0);

	// Shuffle
	y1 = _mm_shuffle_epi8(y0, shuf_mask);

	return y1;
}

#endif
