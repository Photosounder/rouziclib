blend_func_t cur_blend=blend_add;

// The following functions blend a whole foreground pixel with a background pixel by a ratio of p (1.15 fixed-point format)

void blend_solid(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	*bg = fg;
}

void blend_add(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t r, g, b;

	r = (fg.r * p >> 15) + bg->r;	if (r>ONE) bg->r = ONE; else bg->r = r;
	g = (fg.g * p >> 15) + bg->g;	if (g>ONE) bg->g = ONE; else bg->g = g;
	b = (fg.b * p >> 15) + bg->b;	if (b>ONE) bg->b = ONE; else bg->b = b;
}

void blend_add_fl(frgb_t *bg, frgb_t fg, float p)
{
	float r, g, b;

	bg->r += fg.r * p;
	bg->g += fg.g * p;
	bg->b += fg.b * p;
}

void blend_add_limit_fl(frgb_t *bg, frgb_t fg, float p)
{
	blend_add_fl(bg, fg, p);
	rangelimit_frgb(bg);
}

void blend_sub(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t r, g, b;

	r = -(fg.r * p >> 15) + bg->r;	if (r<0) bg->r = 0; else bg->r = r;
	g = -(fg.g * p >> 15) + bg->g;	if (g<0) bg->g = 0; else bg->g = g;
	b = -(fg.b * p >> 15) + bg->b;	if (b<0) bg->b = 0; else bg->b = b;
}

void blend_mul(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t pinv;

	pinv = 32768 - p;
	bg->r = (bg->r*fg.r >> LBD) * p + bg->r * pinv >> 15;
	bg->g = (bg->g*fg.g >> LBD) * p + bg->g * pinv >> 15;
	bg->b = (bg->b*fg.b >> LBD) * p + bg->b * pinv >> 15;
}

void blend_mul4(lrgb_t *bg, lrgb_t fg, int32_t p)	// multiply by a quarter intensity image (1.0 == sRGB value of 137)
{
	int32_t r, g, b, pinv;

	pinv = 32768 - p;
	r = (bg->r*fg.r >> (LBD-2)) * p + bg->r * pinv >> 15;	if (r>ONE) bg->r = ONE; else bg->r = r;
	g = (bg->g*fg.g >> (LBD-2)) * p + bg->g * pinv >> 15;	if (g>ONE) bg->g = ONE; else bg->g = g;
	b = (bg->b*fg.b >> (LBD-2)) * p + bg->b * pinv >> 15;	if (b>ONE) bg->b = ONE; else bg->b = b;
}

void blend_blend(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t pinv;

	pinv = 32768 - p;
	bg->r = fg.r * p + bg->r * pinv >> 15;
	bg->g = fg.g * p + bg->g * pinv >> 15;
	bg->b = fg.b * p + bg->b * pinv >> 15;
}

int alphablend_one_channel(int Cb, int Ca, int Ab_ai, int Aa, int Aoi)
{
	int Co;
	Co = ( (int64_t) Ca*Aa + ((int64_t) Cb*Ab_ai >> 15) ) * Aoi >> 30;	// 1.LBD format

	return MINN(ONE, Co);
}

lrgb_t blend_alphablend_sep_alpha(lrgb_t Cb, lrgb_t Ca, int Ab, int Aa)	// pixel a on top of pixel b, with separated 1.15 format alpha values
{
	lrgb_t Co={0};
	int Aai, Ao, Aoi, Ab_ai;

	// Special cases
	if (Aa == 32768)		// if the front pixel is opaque
		return Ca;		// just return the front pixel
	else if (Aa == 0)		// if the front pixel is transparent
		return Cb;		// just return the back pixel
	else if (Ab == 0)		// if the back pixel is blank
	{
		Ca.a = Q15_TO_LBD(Aa);	// update the front pixel's alpha that takes the multiplication by p into account
		return Ca;		// the output pixel is the front pixel
	}

	Aai = 32768 - Aa;
	Ab_ai = Ab * Aai;
	Ao = Aa + (Ab_ai >> 15);	// output alpha in 1.15
	Co.a = Q15_TO_LBD(Ao);
	if (Ao==0)
		return Co;

	Aoi = (1L<<30) / Ao;		// inverted to make the division, 0.30 format

	Co.r = alphablend_one_channel(Cb.r, Ca.r, Ab_ai, Aa, Aoi);
	Co.g = alphablend_one_channel(Cb.g, Ca.g, Ab_ai, Aa, Aoi);
	Co.b = alphablend_one_channel(Cb.b, Ca.b, Ab_ai, Aa, Aoi);

	return Co;
}

void blend_alphablend(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	*bg = blend_alphablend_sep_alpha(*bg, fg, LBD_TO_Q15(bg->a), LBD_TO_Q15(fg.a) * p >> 15);
}

void blend_alphablendfg(lrgb_t *bg, lrgb_t fg, int32_t p)	// alpha blending (doesn't take framebuffer's alpha into account though, assumed to be 1.0)
{
	int32_t pinv;

	p = p * fg.a >> LBD;
	pinv = 32768 - p;

	bg->r = fg.r * p + bg->r * pinv >> 15;
	bg->g = fg.g * p + bg->g * pinv >> 15;
	bg->b = fg.b * p + bg->b * pinv >> 15;
}

void blend_blendalphaonly(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	int32_t pinv;

	pinv = 32768 - p;
	bg->a = fg.a * p + bg->a * pinv >> 15;
}

blend_func_fl_t get_blend_fl_equivalent(const blend_func_t bf)
{
	if (bf==blend_add)
		return blend_add_limit_fl;

	return blend_add_limit_fl;
}

#ifdef RL_INTEL_INTR
#ifdef __GNUC__
__attribute__((__target__("avx2")))
#endif
void alphablend_lrgb_on_srgb_simd128(__int32 *s0_ptr, __m128i *l_ptr, __int64 *s1_ptr, int32_t *lut0, int32_t *lut1)	// AVX2
{
	__m128i s1, la, lb, Ca, Cb, Aa, Aai, alpha_shuf, shuf_a, shuf_b, opaque_mask, alpha_mask, Co;

	// Load 2 lrgb pixels
	Ca = _mm_lddqu_si128(l_ptr);					// load 2 lrgb pixels (8 x i16)
	alpha_mask = _mm_set_epi16(0xFFFF, 0, 0, 0, 0xFFFF, 0, 0, 0);

	if (_mm_test_all_zeros(Ca, alpha_mask))				// if all 2 lrgb pixels are transparent
	{
		_mm_storeu_si64(s1_ptr, _mm_loadl_epi64(s0_ptr));	// copy the original 2 srgb pixels
		return;
	}

	opaque_mask = _mm_set_epi16(0x8000, 0, 0, 0, 0x8000, 0, 0, 0);
	Aa = _mm_xor_si128(Ca, opaque_mask);				// zeroes alpha values that are 32768

	if (_mm_test_all_zeros(Aa, alpha_mask)==0)			// we do the blending math only if Aa != 32768
	{
		// RGBA -> AAAA for the 2 lrgb pixels
		alpha_shuf = _mm_set_epi8(8+7, 8+6, 8+7, 8+6, 8+7, 8+6, 8+7, 8+6, 7, 6, 7, 6, 7, 6, 7, 6);
		Aa = _mm_shuffle_epi8(Ca, alpha_shuf);
		Aai = _mm_sub_epi16(_mm_set1_epi16(32768), Aa);		// 32768 - Aa

		// Convert 2 srgb pixels to lrgb
		la = _mm_load_4xi8_as_4xi32(s0_ptr);			// load first srgb0 pixel
		la = _mm_i32gather_epi32(lut0, la, 4);			// slrgb_l lookup, la is latent
		lb = _mm_load_4xi8_as_4xi32(&s0_ptr[1]);		// load second srgb0 pixel
		lb = _mm_i32gather_epi32(lut0, lb, 4);			// slrgb_l lookup, lb is latent

		// Make Cb by shuffling and uniting la and lb
		shuf_a = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 13, 12,  9,  8,  5,  4,  1,  0);
		shuf_b = _mm_set_epi8(13, 12,  9,  8,  5,  4,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1);
		la = _mm_shuffle_epi8(la, shuf_a);
		lb = _mm_shuffle_epi8(lb, shuf_b);
		Cb = _mm_or_si128(la, lb);

		// Do the blending math
		Ca = _mm_mulhi_epu16(Ca, Aa);				// Ca*Aa >> 16
		Cb = _mm_mulhi_epu16(Cb, Aai);				// Cb*Aai >> 16
		Ca = _mm_add_epi16(Ca, Cb);
		Ca = _mm_slli_epi16(Ca, 1);				// <<= 1 to convert from 1.14 to 1.15
	}

	// Prepare for lsrgb lookup by splitting Ca
	shuf_a = _mm_set_epi8(-1, -1,  7,  6, -1, -1,  5,  4, -1, -1,  3,  2, -1, -1,  1,  0);
	shuf_b = _mm_set_epi8(-1, -1, 15, 14, -1, -1, 13, 12, -1, -1, 11, 10, -1, -1,  9,  8);
	la = _mm_shuffle_epi8(Ca, shuf_a);
	lb = _mm_shuffle_epi8(Ca, shuf_b);

	// Lsrgb lookups
	la = _mm_i32gather_epi32(lut1, la, 4);			// lsrgb_l lookup, la is latent
	lb = _mm_i32gather_epi32(lut1, lb, 4);			// lsrgb_l lookup, lb is latent
	la = _mm_srli_epi32(la, 5);				// >>= 5 to make it 8 bit
	lb = _mm_srli_epi32(lb, 5);				// >>= 5 to make it 8 bit

	// Pack from 32-bit to 8-bit
	shuf_a = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12,  8,  4,  0);
	shuf_b = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 12,  8,  4,  0, -1, -1, -1, -1);
	la = _mm_shuffle_epi8(la, shuf_a);
	lb = _mm_shuffle_epi8(lb, shuf_b);
	Co = _mm_or_si128(la, lb);

	// Store result
	_mm_storeu_si64(s1_ptr, Co);

	/* Blending algorithm
	if (Aa == 32768) return Ca;
	else if (Aa == 0) return Cb;

	Aai = 32768 - Aa;
	Co = ( Ca*Aa + Cb*Aai ) >> 15;	// 1.LBD format*/
}

/*void alphablend_lrgb_on_srgb_simd256(__int64 *s0_ptr, __m256i *l_ptr, __m128i *s1_ptr, int32_t *lut0, int32_t *lut1)	// AVX2, ~24% slower than the 128-bit version
{
	__m256i s1, la, lb, Ca, Cb, Aa, Aai, alpha_shuf, shuf_a, shuf_b, opaque_mask, alpha_mask;
	__m128i Co;

	// Load 4 lrgb pixels
	Ca = _mm256_lddqu_si256(l_ptr);					// load 4 lrgb pixels (16 x i16)
	alpha_mask = _mm256_set_epi8(0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0);

	if (_mm256_testz_si256(Ca, alpha_mask))				// if all 4 lrgb pixels are transparent
	{
		_mm_storeu_si128(s1_ptr, _mm_lddqu_si128(s0_ptr));	// copy the original 4 srgb pixels
		return;
	}

	opaque_mask = _mm256_set_epi8(0x80, 0x00, 0, 0, 0, 0, 0, 0, 0x80, 0x00, 0, 0, 0, 0, 0, 0, 0x80, 0x00, 0, 0, 0, 0, 0, 0, 0x80, 0x00, 0, 0, 0, 0, 0, 0);
	Aa = _mm256_xor_si256(Ca, opaque_mask);				// zeroes alpha values that are 32768

	if (_mm256_testz_si256(Aa, alpha_mask)==0)			// we do the blending math only if Aa != 32768
	{
		// RGBA -> AAAA for the 4 lrgb pixels
		alpha_shuf = _mm256_set_epi8(24+7, 24+6, 24+7, 24+6, 24+7, 24+6, 24+7, 24+6, 16+7, 16+6, 16+7, 16+6, 16+7, 16+6, 16+7, 16+6, 8+7, 8+6, 8+7, 8+6, 8+7, 8+6, 8+7, 8+6, 7, 6, 7, 6, 7, 6, 7, 6);
		Aa = _mm256_shuffle32_epi8(Ca, alpha_shuf);
		Aai = _mm256_sub_epi16(_mm256_set1_epi16(32768), Aa);	// 32768 - Aa

		// Convert 4 srgb pixels to lrgb
		la = _mm256_load_8xi8_as_8xi32(s0_ptr);			// load 2 first srgb0 pixels
		la = _mm256_i32gather_epi32(lut0, la, 4);		// slrgb_l lookup, la is latent
		lb = _mm256_load_8xi8_as_8xi32(&s0_ptr[1]);		// load 2 other srgb0 pixels
		lb = _mm256_i32gather_epi32(lut0, lb, 4);		// slrgb_l lookup, lb is latent

		// Make Cb by shuffling and uniting la and lb
		shuf_a = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 29, 28, 25, 24, 21, 20, 17, 16, 13, 12,  9,  8,  5,  4,  1,  0);
		shuf_b = _mm256_set_epi8(29, 28, 25, 24, 21, 20, 17, 16, 13, 12,  9,  8,  5,  4,  1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
		la = _mm256_shuffle32_epi8(la, shuf_a);
		lb = _mm256_shuffle32_epi8(lb, shuf_b);
		Cb = _mm256_or_si256(la, lb);

		// Do the blending math
		Ca = _mm256_mulhi_epu16(Ca, Aa);			// Ca*Aa >> 16
		Cb = _mm256_mulhi_epu16(Cb, Aai);			// Cb*Aai >> 16
		Ca = _mm256_add_epi16(Ca, Cb);
		Ca = _mm256_slli_epi16(Ca, 1);				// <<= 1 to convert from 1.14 to 1.15
	}

	// Prepare for lsrgb lookup by splitting Ca
	shuf_a = _mm256_set_epi8(-1, -1, 15, 14, -1, -1, 13, 12, -1, -1, 11, 10, -1, -1,  9,  8, -1, -1,  7,  6, -1, -1,  5,  4, -1, -1,  3,  2, -1, -1,  1,  0);
	shuf_b = _mm256_set_epi8(-1, -1, 31, 30, -1, -1, 29, 28, -1, -1, 27, 26, -1, -1, 25, 24, -1, -1, 23, 22, -1, -1, 21, 20, -1, -1, 19, 18, -1, -1, 17, 16);
	la = _mm256_shuffle32_epi8(Ca, shuf_a);
	lb = _mm256_shuffle32_epi8(Ca, shuf_b);

	// Lsrgb lookups
	la = _mm256_i32gather_epi32(lut1, la, 4);		// lsrgb_l lookup, la is latent
	lb = _mm256_i32gather_epi32(lut1, lb, 4);		// lsrgb_l lookup, lb is latent
	la = _mm256_srli_epi32(la, 5);				// >>= 5 to make it 8 bit
	lb = _mm256_srli_epi32(lb, 5);				// >>= 5 to make it 8 bit

	// Pack from 32-bit to 8-bit
	shuf_a = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 28, 24, 20, 16, 12,  8,  4,  0);
	shuf_b = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 28, 24, 20, 16, 12,  8,  4,  0, -1, -1, -1, -1, -1, -1, -1, -1);
	la = _mm256_shuffle32_epi8(la, shuf_a);
	lb = _mm256_shuffle32_epi8(lb, shuf_b);
	Co = _mm256_castsi256_si128(_mm256_or_si256(la, lb));

	// Store result
	_mm_storeu_si128(s1_ptr, Co);
}*/
#endif
