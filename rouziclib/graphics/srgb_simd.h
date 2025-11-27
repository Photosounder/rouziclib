extern v128_t log2_simd_q8_23_deg5(v128_t x);
extern v128_t log2_simd_q8_23_deg4(v128_t x);
extern v128_t log2_simd_q8_23_deg3(v128_t x);
extern v128_t exp2_simd_q8_23_deg4(v128_t x);
extern v128_t exp2_simd_q8_23_deg3(v128_t x);
extern v128_t div_by_2_4_simd(v128_t v);
extern v128_t lsrgb_by_pow_simd(uint16_t *lrgb);
extern v128_t lsrgb_deg4_simd(uint16_t *lrgb);
extern uint32_t lsrgb_deg4_simd_u8(uint16_t *lrgb);

extern uint64_t slrgb_deg3_simd(uint8_t *srgb);
extern uint64_t slrgb_lut(uint8_t *srgb);

extern double lsrgb_simd_test(double x);
