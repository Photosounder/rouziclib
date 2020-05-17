int check_cpuinfo(const enum cpu_feat_n fid)
{
#ifdef RL_INTEL_INTR
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
		break;	case CPU_HAS_BMI1:	ret = info7[1] & (1 << 3);
		break;	case CPU_HAS_BMI2:	ret = info7[1] & (1 << 8);
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
#endif
	return 0;
}

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

#ifndef _mm_storeu_si32
void _mm_storeu_si32(void* mem_addr, __m128i a)	// replacement for missing _mm_storeu_si32
{
	_mm_store_ss((float*) mem_addr, _mm_castsi128_ps(a));
	return;
}
#endif

__m128 _mm_i32sgather_ps(float const *base_addr, __m128i vindex)	// faster than _mm_i32gather_ps and without AVX2
{
	uint32_t index[4];
	__m128 r;
	_mm_store_si128((__m128i *) &index, vindex);
	r = _mm_set_ps(base_addr[index[3]], base_addr[index[2]], base_addr[index[1]], base_addr[index[0]]);
	return r;
}

#endif
