v128_t log2_simd_q8_23_deg5(v128_t x)	// input is float, output is Q8.23
{
	// Extract the exponent and mantissa from the float values
	v128_t e = i32x4_add(v128_and(x, i32x4_splat(0xFF800000)), i32x4_splat(-0x3F800000));	// Q8.23
	v128_t m = v128_and(x, i32x4_splat(0x007FFFFF));				// we treat it as [0 , 1[ in UQ0.23

	m = i32x4_shr_u(m, 8);	// UQ0.23 is too much, UQ0.15 gives better numerical precision

	// Polynomial on the mantissa, result is UQ1.23
	v128_t y = i32x4_splat(11516);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(-1630660052));  y = i32x4_shr_s(y, 16);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat( 1767669508));  y = i32x4_shr_s(y, 16);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(-1518776154));  y = i32x4_shr_s(y, 16);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat( 1547901123));  y = i32x4_shr_s(y, 15);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(      37581));  y = i32x4_shr_s(y,  7);

	// Add the Q8.23 exponent and the UQ1.23 mantissa to make the Q8.23 logarithm
	return i32x4_add(e, y);
}

v128_t log2_simd_q8_23_deg4(v128_t x)	// input is float, output is Q8.23
{
	// Extract the exponent and mantissa from the float values
	v128_t e = i32x4_add(v128_and(x, i32x4_splat(0xFF800000)), i32x4_splat(-0x3F800000));	// Q8.23
	v128_t m = v128_and(x, i32x4_splat(0x007FFFFF));				// we treat it as [0 , 1[ in UQ0.23

	m = i32x4_shr_u(m, 8);	// UQ0.23 is too much, UQ0.15 gives better numerical precision

	// Polynomial on the mantissa, result is UQ1.23
	v128_t y = i32x4_splat(-10486);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat( 1354966101));  y = i32x4_shr_s(y, 16);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(-1445138939));  y = i32x4_shr_s(y, 16);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat( 1543297316));  y = i32x4_shr_s(y, 14);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(     257698));  y = i32x4_shr_s(y,  8);

	// Add the Q8.23 exponent and the UQ1.23 mantissa to make the Q8.23 logarithm
	return i32x4_add(e, y);
}

v128_t log2_simd_q8_23_deg3(v128_t x)	// input is float, output is Q8.23
{
	// Extract the exponent and mantissa from the float values
	v128_t e = i32x4_add(v128_and(x, i32x4_splat(0xFF800000)), i32x4_splat(-0x3F800000));	// Q8.23
	v128_t m = v128_and(x, i32x4_splat(0x007FFFFF));				// we treat it as [0 , 1[ in UQ0.23

	m = i32x4_shr_u(m, 8);	// UQ0.23 is too much, UQ0.15 gives better numerical precision

	// Polynomial on the mantissa, result is UQ1.23
	v128_t y = i32x4_splat(10184);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(-1230540898));  y = i32x4_shr_s(y, 16);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat( 1521822479));  y = i32x4_shr_s(y, 15);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(     778821));  y = i32x4_shr_s(y,  7);

	// Add the Q8.23 exponent and the UQ1.23 mantissa to make the Q8.23 logarithm
	return i32x4_add(e, y);
}

v128_t exp2_simd_q8_23_deg4(v128_t x)	// input is Q8.23, output is float
{
	// Extract exponent, add 127 to make it float
	v128_t e = v128_and(i32x4_add(x, i32x4_splat(0x3F800000)), i32x4_splat(0xFF800000));

	// Extract mantissa
	v128_t m = v128_and(x, i32x4_splat(0x007FFFFF));	// UQ0.23
	m = i32x4_shr_u(m, 8);		// UQ0.23 => UQ0.15

	// Polynomial on the mantissa, result is UQ0.23
	v128_t y = i32x4_splat(7174);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(  888569466));  y = i32x4_shr_s(y, 17);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat( 1037821216));  y = i32x4_shr_s(y, 17);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(  744104196));  y = i32x4_shr_s(y, 15);
	y = i32x4_mul(y, m);                                               y = i32x4_shr_s(y,  7);

	return v128_or(e, y);
}

v128_t exp2_simd_q8_23_deg3(v128_t x)	// input is Q8.23, output is float
{
	// Extract exponent, add 127 to make it float
	v128_t e = v128_and(i32x4_add(x, i32x4_splat(0x3F800000)), i32x4_splat(0xFF800000));

	// Extract mantissa
	v128_t m = v128_and(x, i32x4_splat(0x007FFFFF));	// UQ0.23
	m = i32x4_shr_u(m, 8);		// UQ0.23 => UQ0.15

	// Polynomial on the mantissa, result is UQ0.23
	v128_t y = i32x4_splat(10366);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(  964355891));  y = i32x4_shr_s(y, 17);
	y = i32x4_mul(y, m);  y = i32x4_add(y, i32x4_splat(  747760043));  y = i32x4_shr_s(y, 15);
	y = i32x4_mul(y, m);                                               y = i32x4_shr_s(y,  7);

	return v128_or(e, y);
}

v128_t div_by_2_4_simd(v128_t v)	// int32_t division like /3 but it's /2.4, max error 1 ULP
{
	// Division by 2.4 (multiplication by 0.011010101010... in binary)
	v128_t q = i32x4_add(v, i32x4_shr_s(v, 2));	// 1.01
	q = i32x4_add(q, i32x4_shr_s(q, 4));		// 1.010101
	q = i32x4_add(q, i32x4_shr_s(q, 8));		// 1.01010101010101
	q = i32x4_add(q, i32x4_shr_s(q, 16));		// 1.010101010101010101010101010101
	q = i32x4_shr_s(q, 1);				// 0.1010101010101010101010101010101
	q = i32x4_add(q, v);				// 1.1010101010101010101010101010101
	v = i32x4_shr_s(q, 2);				// 0.011010101010101010101010101010101
	return v;
}

// Linear to sRGB

v128_t lsrgb_by_pow_simd(uint16_t *lrgb) // converts [0.0, 1.0] UQ1.15 linear values into [0.0, 255.0] UQ9.14 sRGB values
{
	v128_t v;
	v.u32[0] = lrgb[0];
	v.u32[1] = lrgb[1];
	v.u32[2] = lrgb[2];
	v.u32[3] = lrgb[3];

	// Selection mask
	v128_t sel = i32x4_le_s(v, i32x4_splat(102));

	// Line part = v * 12.92 * 255
	v128_t line = i32x4_mul(v, i32x4_splat(13494682));	// UQ1.15 * 12.92*255*2^12 => UQ0.27
	line = i32x4_shr_s(line, 13);				// UQ0.14

	// curve = 2^(log2(x)/2.4 + log2(1.055*255 / 32768^1/2.4)) - 0.055*255
	// Convert to float and do log2
	v = f32x4_convert_i32x4_u(v);
	v = log2_simd_q8_23_deg3(v);		// float => Q4.23

	// Division by 2.4
	v = div_by_2_4_simd(v);

	// Add log2(1.055*255 / 32768^(1/2.4)) in UQ4.23
	v = i32x4_add(v, i32x4_splat(15280658));

	// exp2
	v = exp2_simd_q8_23_deg3(v);
	v = f32x4_add(v, f32x4_splat(512.f));		// convert to Q9.14, [0.f , 269.025f] + 512.f => [512.f , 781.025f]
	v = i32x4_add(v, i32x4_splat(-0x4403819A));	// subtract 0.055*255+512

	// Select either the line or the curve
	v = v128_bitselect(line, v, sel);	// UQ9.14
	return v;
}

v128_t lsrgb_deg4_simd(uint16_t *lrgb)	// output is in UQ8.22 format, untested
{
	v128_t l;
	l.u32[0] = lrgb[0];
	l.u32[1] = lrgb[1];
	l.u32[2] = lrgb[2];
	l.u32[3] = lrgb[3];

	// Selection mask
	v128_t sel = i32x4_le_s(l, i32x4_splat(102));

	// Line part
	v128_t line = i32x4_mul(l, i32x4_splat(421709));

	// Curve part
	// Convert UQ1.15 to float using l + 0x43800000 - 256.f
	l = i32x4_add(l, i32x4_splat(0x43800000));	// [0 , 32768] => [256.f , 257.f]
	l = f32x4_add(l, f32x4_splat(-256.f));		// [256.f , 257.f] - 256.f => [0.f , 1.f]

	// Float sqrt
	l = f32x4_sqrt(l);

	// Convert float to UQ1.15
	l = f32x4_add(l, f32x4_splat(256.f));		// [0.f , 1.f] + 256.f => [256.f , 257.f]
	l = i32x4_add(l, i32x4_splat(-0x43800000));	// [256.f , 257.f] => [0 , 32768]

	// Fixed point polynomial on square rooted values
	v128_t s = i32x4_splat(-17142);
	s = i32x4_mul(s, l);  s = i32x4_add(s, i32x4_splat( 1592348186));  s = i32x4_shr_s(s, 15);
	s = i32x4_mul(s, l);  s = i32x4_add(s, i32x4_splat(-1878770691));  s = i32x4_shr_s(s, 16);
	s = i32x4_mul(s, l);  s = i32x4_add(s, i32x4_splat( 1531779084));  s = i32x4_shr_s(s, 15);
	s = i32x4_mul(s, l);  s = i32x4_add(s, i32x4_splat(  -38628979));

	// Select either the line or the curve
	s = v128_bitselect(line, s, sel);
	return s;
}

uint32_t lsrgb_deg4_simd_u8(uint16_t *lrgb)
{
	v128_t s = lsrgb_deg4_simd(lrgb);

	s = i32x4_add(s, i32x4_splat(2097152));		// round
	s = i32x4_shr_s(s, 22);				// UQ8.22 => UQ8.0
	s = i16x8_narrow_i32x4_u(s, s);			// i32x4 => i16x4
	s = i8x16_narrow_i16x8_u(s, s);			// i16x4 => i8x4
	return s.u32[0];
}

// sRGB to linear

uint64_t slrgb_deg3_simd(uint8_t *srgb)	// UQ8.0 => UQ1.15
{
	v128_t s;
	s.u32[0] = srgb[0];
	s.u32[1] = srgb[1];
	s.u32[2] = srgb[2];
	s.u32[3] = srgb[3];

	// Selection mask
	v128_t sel = i32x4_le_s(s, i32x4_splat(10));

	// Line part
	v128_t line = i32x4_mul(s, i32x4_splat(167914300));
	line = i32x4_shr_s(line, 24);

	// Curve part
	v128_t l = i32x4_splat(-1787535);
	l = i32x4_mul(l, s);  l = i32x4_add(l, i32x4_splat(1565190841));  l = i32x4_shr_s(l, 9);
	l = i32x4_mul(l, s);  l = i32x4_add(l, i32x4_splat(1548300717));  l = i32x4_shr_s(l, 9);
	l = i32x4_mul(l, s);  l = i32x4_add(l, i32x4_splat(  27439799));  l = i32x4_shr_s(l, 15);
	l = i32x4_mul(l, l);                                              l = i32x4_shr_s(l, 15);

	// Select either the line or the curve
	l = v128_bitselect(line, l, sel);
	l = i16x8_narrow_i32x4_u(l, l);
	return l.u64[0];
}

uint64_t slrgb_lut(uint8_t *srgb)
{
	static const uint16_t srgb_lut[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 99, 110, 120, 132, 144, 157, 170, 184, 198, 213, 229, 246, 263, 281, 299, 319, 338, 359, 381, 403, 425, 449, 473, 498, 524, 551, 578, 606, 635, 665, 695, 727, 759, 792, 825, 860, 895, 931, 969, 1006, 1045, 1085, 1125, 1167, 1209, 1252, 1296, 1341, 1386, 1433, 1481, 1529, 1578, 1629, 1680, 1732, 1785, 1839, 1894, 1950, 2007, 2065, 2123, 2183, 2244, 2306, 2368, 2432, 2496, 2562, 2629, 2696, 2765, 2834, 2905, 2977, 3049, 3123, 3198, 3273, 3350, 3428, 3507, 3587, 3668, 3750, 3833, 3917, 4002, 4089, 4176, 4264, 4354, 4444, 4536, 4629, 4723, 4818, 4914, 5011, 5109, 5209, 5309, 5411, 5514, 5618, 5723, 5829, 5936, 6045, 6155, 6265, 6377, 6490, 6605, 6720, 6837, 6954, 7073, 7193, 7315, 7437, 7561, 7686, 7812, 7939, 8068, 8197, 8328, 8460, 8593, 8728, 8864, 9001, 9139, 9278, 9419, 9561, 9704, 9848, 9994, 10141, 10289, 10438, 10589, 10741, 10894, 11048, 11204, 11361, 11519, 11679, 11839, 12001, 12165, 12329, 12495, 12663, 12831, 13001, 13172, 13344, 13518, 13693, 13870, 14047, 14226, 14407, 14588, 14771, 14956, 15141, 15328, 15517, 15706, 15897, 16090, 16284, 16479, 16675, 16873, 17072, 17273, 17474, 17678, 17882, 18088, 18296, 18504, 18715, 18926, 19139, 19353, 19569, 19786, 20005, 20225, 20446, 20669, 20893, 21118, 21345, 21574, 21803, 22035, 22267, 22501, 22737, 22974, 23212, 23452, 23693, 23936, 24180, 24425, 24672, 24921, 25171, 25422, 25675, 25929, 26185, 26442, 26701, 26961, 27223, 27486, 27750, 28016, 28284, 28553, 28823, 29095, 29369, 29644, 29920, 30198, 30478, 30759, 31041, 31325, 31611, 31898, 32186, 32476, 32768 };

	v128_t l;
	for (int i=0; i < 4; i++)
		l.u16[i] = srgb_lut[srgb[i]];
	return l.u64[0];
}

// Test functions

double lsrgb_simd_test(double x)
{
	v128_t v = {0};
	v.u16[0] = nearbyint(x * 32768.);
	v = lsrgb_by_pow_simd(v.u16);
	return (double) v.i32[0] * (1./16384.);
}
