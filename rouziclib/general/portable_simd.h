// Vector definition
typedef union
{
	 int8_t  i8[16];
	uint8_t  u8[16];
	 int16_t i16[8];
	uint16_t u16[8];
	 int32_t i32[4];
	uint32_t u32[4];
	 int64_t i64[2];
	uint64_t u64[2];
	float    f32[4];
	double   f64[2];
} v128_t;

// Utilities
static inline int8_t  sat8_s(int16_t v) { if (v > INT8_MAX)  v = INT8_MAX;  if (v < INT8_MIN)  v = INT8_MIN;  return (int8_t) v; }
static inline int16_t sat16_s(int32_t v) { if (v > INT16_MAX) v = INT16_MAX; if (v < INT16_MIN) v = INT16_MIN; return v; }
static inline uint8_t  sat8_u(int16_t v) { if (v > UINT8_MAX)  v = UINT8_MAX;  if (v < 0)  v = 0; return (uint8_t) v; }
static inline uint16_t sat16_u(int32_t v) { if (v > UINT16_MAX) v = UINT16_MAX; if (v < 0)  v = 0; return v; }
static inline float  sat32_sf32(float v) { if (v > (float)INT32_MAX)  v = (float)INT32_MAX;  if (v < (float)INT32_MIN) v = (float)INT32_MIN; return v; }
static inline float  sat32_uf32(float v) { if (v > (float)UINT32_MAX) v = (float)UINT32_MAX; if (v < 0.f) v = 0.f; return v; }
static inline double sat32_sf64(double v) { if (v > (double)INT32_MAX)  v = (double)INT32_MAX;  if (v < (double)INT32_MIN) v = (double)INT32_MIN; return v; }
static inline double sat64_uf64(double v) { if (v > (double)UINT64_MAX) v = (double)UINT64_MAX; if (v < 0.) v = 0.; return v; }

static inline uint8_t popcnt8(uint8_t x)
{
#if defined(__GNUC__)
	return __builtin_popcount(x);
#endif
	x -= ((x >> 1) & 0x55);
	x = (x & 0x33) + ((x >> 2) & 0x33);
	return (x + (x >> 4)) & 0xF;
}

static inline float op96_f32_min(float a, float b) { if (isnan(a)) return b; if (isnan(b)) return a; return (a < b) ? a : b; }
static inline float op97_f32_max(float a, float b) { if (isnan(a)) return b; if (isnan(b)) return a; return (a > b) ? a : b; }
static inline double opA4_f64_min(double a, double b) { if (isnan(a)) return b; if (isnan(b)) return a; return MINN(a, b); }
static inline double opA5_f64_max(double a, double b) { if (isnan(a)) return b; if (isnan(b)) return a; return MAXN(a, b); }
static inline  int32_t opFC00_i32_trunc_sat_f32_s(float  v) { return isnan(v) ? 0 : (int32_t)sat32_sf32(v); }
static inline uint32_t opFC01_i32_trunc_sat_f32_u(float  v) { return isnan(v) ? 0 : (uint32_t)sat32_uf32(v); }
static inline  int32_t opFC02_i32_trunc_sat_f64_s(double v) { return isnan(v) ? 0 : (int32_t)sat32_sf64(v); }
static inline uint64_t opFC07_i64_trunc_sat_f64_u(double v) { return isnan(v) ? 0 : (uint64_t)sat64_uf64(v); }

// Const
static inline v128_t v128_const(uint64_t a, uint64_t b) { v128_t r; r.u64[0] = a; r.u64[1] = b; return r; }

// Lane selection
static inline v128_t i8x16_shuffle(const int8_t *s, v128_t a, v128_t b) { v128_t r; for (int i=0; i<16; i++) r.u8[i] = s[i] < 16 ? a.u8[s[i]] : b.u8[s[i]-16]; return r; }
static inline v128_t i8x16_swizzle(v128_t v, int8_t *s)                 { v128_t r; for (int i=0; i<16; i++) r.u8[i] = s[i] < 16 ? v.u8[s[i]] : 0; return r; }
static inline v128_t i8x16_relaxed_swizzle(v128_t v, int8_t *s) { v128_t r; for (int i=0; i<16; i++) r.u8[i] = v.u8[s[i]]; return r; }
static inline v128_t i8x16_relaxed_laneselect(v128_t a, v128_t b, v128_t c) { for (int i=0; i<16; i++) a.i8[i] = (c.i8[i] < 0) ? a.i8[i] : b.i8[i]; return a; }
static inline v128_t i16x8_relaxed_laneselect(v128_t a, v128_t b, v128_t c) { for (int i=0; i<8; i++) a.i16[i] = (c.i16[i] < 0) ? a.i16[i] : b.i16[i]; return a; }
static inline v128_t i32x4_relaxed_laneselect(v128_t a, v128_t b, v128_t c) { for (int i=0; i<4; i++) a.i32[i] = (c.i32[i] < 0) ? a.i32[i] : b.i32[i]; return a; }
static inline v128_t i64x2_relaxed_laneselect(v128_t a, v128_t b, v128_t c) { for (int i=0; i<2; i++) a.i64[i] = (c.i64[i] < 0) ? a.i64[i] : b.i64[i]; return a; }

// Splat
static inline v128_t i8x16_splat(uint8_t v)  { v128_t r; for (int i=0; i<16; i++) r.u8[i]  = v; return r; }
static inline v128_t i16x8_splat(uint16_t v) { v128_t r; for (int i=0; i<8;  i++) r.u16[i] = v; return r; }
static inline v128_t i32x4_splat(uint32_t v) { v128_t r; for (int i=0; i<4;  i++) r.u32[i] = v; return r; }
static inline v128_t i64x2_splat(uint64_t v) { v128_t r; for (int i=0; i<2;  i++) r.u64[i] = v; return r; }
static inline v128_t f32x4_splat(float v)    { v128_t r; for (int i=0; i<4;  i++) r.f32[i] = v; return r; }
static inline v128_t f64x2_splat(double v)   { v128_t r; for (int i=0; i<2;  i++) r.f64[i] = v; return r; }

// Extract lane
static inline int32_t i8x16_extract_lane_s(int lane, v128_t a) { return a.i8[lane]; }
static inline int32_t i8x16_extract_lane_u(int lane, v128_t a) { return a.u8[lane]; }
static inline int32_t i16x8_extract_lane_s(int lane, v128_t a) { return a.i16[lane]; }
static inline int32_t i16x8_extract_lane_u(int lane, v128_t a) { return a.u16[lane]; }
static inline int32_t i32x4_extract_lane  (int lane, v128_t a) { return a.i32[lane]; }
static inline int64_t i64x2_extract_lane  (int lane, v128_t a) { return a.i64[lane]; }
static inline float   f32x4_extract_lane  (int lane, v128_t a) { return a.f32[lane]; }
static inline double  f64x2_extract_lane  (int lane, v128_t a) { return a.f64[lane]; }

// Replace lane
static inline v128_t i8x16_replace_lane(const int lane, v128_t v, int32_t n) { v.i8[lane]  = n; return v; }
static inline v128_t i16x8_replace_lane(const int lane, v128_t v, int32_t n) { v.i16[lane] = n; return v; }
static inline v128_t i32x4_replace_lane(const int lane, v128_t v, int32_t n) { v.i32[lane] = n; return v; }
static inline v128_t i64x2_replace_lane(const int lane, v128_t v, int64_t n) { v.i64[lane] = n; return v; }
static inline v128_t f32x4_replace_lane(const int lane, v128_t v, float   n) { v.f32[lane] = n; return v; }
static inline v128_t f64x2_replace_lane(const int lane, v128_t v, double  n) { v.f64[lane] = n; return v; }

// Comparisons
static inline v128_t i8x16_eq  (v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.u8[i] == b.u8[i]); return a; }
static inline v128_t i8x16_ne  (v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.u8[i] != b.u8[i]); return a; }
static inline v128_t i8x16_lt_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.i8[i]  < b.i8[i]); return a; }
static inline v128_t i8x16_lt_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.u8[i]  < b.u8[i]); return a; }
static inline v128_t i8x16_gt_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.i8[i]  > b.i8[i]); return a; }
static inline v128_t i8x16_gt_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.u8[i]  > b.u8[i]); return a; }
static inline v128_t i8x16_le_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.i8[i] <= b.i8[i]); return a; }
static inline v128_t i8x16_le_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.u8[i] <= b.u8[i]); return a; }
static inline v128_t i8x16_ge_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.i8[i] >= b.i8[i]); return a; }
static inline v128_t i8x16_ge_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = -(a.u8[i] >= b.u8[i]); return a; }

static inline v128_t i16x8_eq  (v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.u16[i] == b.u16[i]); return a; }
static inline v128_t i16x8_ne  (v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.u16[i] != b.u16[i]); return a; }
static inline v128_t i16x8_lt_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.i16[i]  < b.i16[i]); return a; }
static inline v128_t i16x8_lt_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.u16[i]  < b.u16[i]); return a; }
static inline v128_t i16x8_gt_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.i16[i]  > b.i16[i]); return a; }
static inline v128_t i16x8_gt_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.u16[i]  > b.u16[i]); return a; }
static inline v128_t i16x8_le_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.i16[i] <= b.i16[i]); return a; }
static inline v128_t i16x8_le_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.u16[i] <= b.u16[i]); return a; }
static inline v128_t i16x8_ge_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.i16[i] >= b.i16[i]); return a; }
static inline v128_t i16x8_ge_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = -(a.u16[i] >= b.u16[i]); return a; }

static inline v128_t i32x4_eq  (v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.u32[i] == b.u32[i]); return a; }
static inline v128_t i32x4_ne  (v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.u32[i] != b.u32[i]); return a; }
static inline v128_t i32x4_lt_s(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.i32[i]  < b.i32[i]); return a; }
static inline v128_t i32x4_lt_u(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.u32[i]  < b.u32[i]); return a; }
static inline v128_t i32x4_gt_s(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.i32[i]  > b.i32[i]); return a; }
static inline v128_t i32x4_gt_u(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.u32[i]  > b.u32[i]); return a; }
static inline v128_t i32x4_le_s(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.i32[i] <= b.i32[i]); return a; }
static inline v128_t i32x4_le_u(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.u32[i] <= b.u32[i]); return a; }
static inline v128_t i32x4_ge_s(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.i32[i] >= b.i32[i]); return a; }
static inline v128_t i32x4_ge_u(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.u32[i] >= b.u32[i]); return a; }

static inline v128_t i64x2_eq  (v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.u64[i] == b.u64[i]); return a; }
static inline v128_t i64x2_ne  (v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.u64[i] != b.u64[i]); return a; }
static inline v128_t i64x2_lt_s(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.i64[i]  < b.i64[i]); return a; }
static inline v128_t i64x2_gt_s(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.i64[i]  > b.i64[i]); return a; }
static inline v128_t i64x2_le_s(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.i64[i] <= b.i64[i]); return a; }
static inline v128_t i64x2_ge_s(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.i64[i] >= b.i64[i]); return a; }

static inline v128_t f32x4_eq(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.f32[i] == b.f32[i]); return a; }
static inline v128_t f32x4_ne(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.f32[i] != b.f32[i]); return a; }
static inline v128_t f32x4_lt(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.f32[i]  < b.f32[i]); return a; }
static inline v128_t f32x4_gt(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.f32[i]  > b.f32[i]); return a; }
static inline v128_t f32x4_le(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.f32[i] <= b.f32[i]); return a; }
static inline v128_t f32x4_ge(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = -(a.f32[i] >= b.f32[i]); return a; }

static inline v128_t f64x2_eq(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.f64[i] == b.f64[i]); return a; }
static inline v128_t f64x2_ne(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.f64[i] != b.f64[i]); return a; }
static inline v128_t f64x2_lt(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.f64[i]  < b.f64[i]); return a; }
static inline v128_t f64x2_gt(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.f64[i]  > b.f64[i]); return a; }
static inline v128_t f64x2_le(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.f64[i] <= b.f64[i]); return a; }
static inline v128_t f64x2_ge(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] = -(a.f64[i] >= b.f64[i]); return a; }

// Bitwise operations
static inline v128_t v128_not   (v128_t a)           { for (int i=0; i<2; i++) a.u64[i] = ~a.u64[i]; return a; }
static inline v128_t v128_and   (v128_t a, v128_t b) { for (int i=0; i<2; i++) a.u64[i] &= b.u64[i]; return a; }
static inline v128_t v128_andnot(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.u64[i] &= ~b.u64[i]; return a; }
static inline v128_t v128_or    (v128_t a, v128_t b) { for (int i=0; i<2; i++) a.u64[i] |= b.u64[i]; return a; }
static inline v128_t v128_xor   (v128_t a, v128_t b) { for (int i=0; i<2; i++) a.u64[i] ^= b.u64[i]; return a; }

static inline v128_t v128_bitselect(v128_t a, v128_t b, v128_t c) { for (int i=0; i<4; i++) a.u32[i] = a.u32[i] & c.u32[i] | b.u32[i] & ~c.u32[i]; return a; }
static inline int32_t i8x16_bitmask(v128_t v) { int32_t m=0; for (int i=0; i<16; i++) m |= ((v.i8[i] < 0) << i); return m; }
static inline int32_t i16x8_bitmask(v128_t v) { int32_t m=0; for (int i=0; i<8; i++) m |= ((v.i16[i] < 0) << i); return m; }
static inline int32_t i32x4_bitmask(v128_t v) { int32_t m=0; for (int i=0; i<4; i++) m |= ((v.i32[i] < 0) << i); return m; }
static inline int32_t i64x2_bitmask(v128_t v) { int32_t m=0; for (int i=0; i<2; i++) m |= ((v.i64[i] < 0) << i); return m; }

static inline int32_t v128_any_true(v128_t a) { return (a.u64[0] != 0) | (a.u64[1] != 0); }
static inline int32_t i8x16_all_true(v128_t v) { for (int i=0; i<16; i++) if (v.u8[i] == 0) return 0; return 1; }
static inline int32_t i16x8_all_true(v128_t v) { for (int i=0; i<8; i++) if (v.u16[i] == 0) return 0; return 1; }
static inline int32_t i32x4_all_true(v128_t v) { for (int i=0; i<4; i++) if (v.u32[i] == 0) return 0; return 1; }
static inline int32_t i64x2_all_true(v128_t v) { for (int i=0; i<2; i++) if (v.u64[i] == 0) return 0; return 1; }

// Math
static inline v128_t i8x16_abs(v128_t v) { for (int i=0; i<16; i++) v.u8[i] = abs(v.i8[i]); return v; }
static inline v128_t i8x16_neg(v128_t v) { for (int i=0; i<16; i++) v.i8[i] = -v.i8[i]; return v; }
static inline v128_t i8x16_popcnt(v128_t v) { for (int i=0; i<16; i++) v.u8[i] = popcnt8(v.u8[i]); return v; }
static inline v128_t i8x16_shl(v128_t a, int32_t s) { for (int i=0; i<16; i++) a.u8[i] = a.u8[i] << s; return a; }
static inline v128_t i8x16_shr_s(v128_t a, int32_t s) { for (int i=0; i<16; i++) a.i8[i] = a.i8[i] >> s; return a; }
static inline v128_t i8x16_shr_u(v128_t a, int32_t s) { for (int i=0; i<16; i++) a.u8[i] = a.u8[i] >> s; return a; }
static inline v128_t i8x16_add(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.u8[i] += b.u8[i]; return a; }
static inline v128_t i8x16_add_sat_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) { int16_t s = a.i8[i] + b.i8[i]; a.i8[i] = sat8_s(s); } return a; }
static inline v128_t i8x16_add_sat_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) { int16_t s = a.u8[i] + b.u8[i]; a.u8[i] = sat8_u(s); } return a; }
static inline v128_t i8x16_sub(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.u8[i] -= b.u8[i]; return a; }
static inline v128_t i8x16_sub_sat_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) { int16_t s = a.i8[i] - b.i8[i]; a.i8[i] = sat8_s(s); } return a; }
static inline v128_t i8x16_sub_sat_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) { int16_t s = a.u8[i] - b.u8[i]; a.u8[i] = sat8_u(s); } return a; }
static inline v128_t i8x16_min_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = MINN(a.i8[i], b.i8[i]); return a; }
static inline v128_t i8x16_min_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.u8[i] = MINN(a.u8[i], b.u8[i]); return a; }
static inline v128_t i8x16_max_s(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.i8[i] = MAXN(a.i8[i], b.i8[i]); return a; }
static inline v128_t i8x16_max_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.u8[i] = MAXN(a.u8[i], b.u8[i]); return a; }
static inline v128_t i8x16_avgr_u(v128_t a, v128_t b) { for (int i=0; i<16; i++) a.u8[i] = (uint8_t) ((uint16_t) a.u8[i] + (uint16_t) b.u8[i] + 1) >> 1; return a; }

static inline v128_t i16x8_abs(v128_t v) { for (int i=0; i<8; i++) v.u16[i] = abs(v.i16[i]); return v; }
static inline v128_t i16x8_neg(v128_t v) { for (int i=0; i<8; i++) v.i16[i] = -v.i16[i]; return v; }
static inline v128_t i16x8_shl(v128_t a, int32_t s) { for (int i=0; i<8; i++) a.u16[i] = a.u16[i] << s; return a; }
static inline v128_t i16x8_shr_s(v128_t a, int32_t s) { for (int i=0; i<8; i++) a.i16[i] = a.i16[i] >> s; return a; }
static inline v128_t i16x8_shr_u(v128_t a, int32_t s) { for (int i=0; i<8; i++) a.u16[i] = a.u16[i] >> s; return a; }
static inline v128_t i16x8_add(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.u16[i] += b.u16[i]; return a; }
static inline v128_t i16x8_add_sat_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) { int32_t s = a.i16[i] + b.i16[i]; a.i16[i] = sat16_s(s); } return a; }
static inline v128_t i16x8_add_sat_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) { int32_t s = a.u16[i] + b.u16[i]; a.u16[i] = sat16_u(s); } return a; }
static inline v128_t i16x8_sub(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.u16[i] -= b.u16[i]; return a; }
static inline v128_t i16x8_sub_sat_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) { int32_t s = a.i16[i] - b.i16[i]; a.i16[i] = sat16_s(s); } return a; }
static inline v128_t i16x8_sub_sat_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) { int32_t s = a.u16[i] - b.u16[i]; a.u16[i] = sat16_u(s); } return a; }
static inline v128_t i16x8_mul(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.u16[i] *= b.u16[i]; return a; }
static inline v128_t i16x8_min_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = MINN(a.i16[i], b.i16[i]); return a; }
static inline v128_t i16x8_min_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.u16[i] = MINN(a.u16[i], b.u16[i]); return a; }
static inline v128_t i16x8_max_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = MAXN(a.i16[i], b.i16[i]); return a; }
static inline v128_t i16x8_max_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.u16[i] = MAXN(a.u16[i], b.u16[i]); return a; }
static inline v128_t i16x8_avgr_u(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.u16[i] = (uint16_t) ((uint32_t) a.u16[i] + (uint32_t) b.u16[i] + 1) >> 1; return a; }
static inline v128_t i16x8_q15mulr_sat_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = sat16_s(((int32_t) a.i16[i] * (int32_t) b.i16[i] + 16384) >> 15); return a; }
static inline v128_t opFD111_i16x8_relaxed_q15mulr_s(v128_t a, v128_t b) { for (int i=0; i<8; i++) a.i16[i] = (int16_t) (((int32_t) a.i16[i] * (int32_t) b.i16[i] + 16384) >> 15); return a; }

static inline v128_t i32x4_abs(v128_t v) { for (int i=0; i<4; i++) v.u32[i] = abs(v.i32[i]); return v; }
static inline v128_t i32x4_neg(v128_t v) { for (int i=0; i<4; i++) v.i16[i] = -v.i16[i]; return v; }
static inline v128_t i32x4_shl(v128_t a, int32_t s) { for (int i=0; i<4; i++) a.u32[i] = a.u32[i] << s; return a; }
static inline v128_t i32x4_shr_s(v128_t a, int32_t s) { for (int i=0; i<4; i++) a.i32[i] = a.i32[i] >> s; return a; }
static inline v128_t i32x4_shr_u(v128_t a, int32_t s) { for (int i=0; i<4; i++) a.u32[i] = a.u32[i] >> s; return a; }
static inline v128_t i32x4_add(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] += b.i32[i]; return a; }
static inline v128_t i32x4_sub(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] -= b.i32[i]; return a; }
static inline v128_t i32x4_mul(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] *= b.i32[i]; return a; }
static inline v128_t i32x4_min_s(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = MINN(a.i32[i], b.i32[i]); return a; }
static inline v128_t i32x4_min_u(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.u32[i] = MINN(a.u32[i], b.u32[i]); return a; }
static inline v128_t i32x4_max_s(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.i32[i] = MAXN(a.i32[i], b.i32[i]); return a; }
static inline v128_t i32x4_max_u(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.u32[i] = MAXN(a.u32[i], b.u32[i]); return a; }

static inline v128_t i64x2_abs(v128_t v) { for (int i=0; i<2; i++) v.u64[i] = llabs(v.i64[i]); return v; }
static inline v128_t i64x2_neg(v128_t v) { for (int i=0; i<2; i++) v.i64[i] = -v.i64[i]; return v; }
static inline v128_t i64x2_shl(v128_t a, int32_t s) { for (int i=0; i<2; i++) a.u64[i] = a.u64[i] << s; return a; }
static inline v128_t i64x2_shr_s(v128_t a, int32_t s) { for (int i=0; i<2; i++) a.i64[i] = a.i64[i] >> s; return a; }
static inline v128_t i64x2_shr_u(v128_t a, int32_t s) { for (int i=0; i<2; i++) a.u64[i] = a.u64[i] >> s; return a; }
static inline v128_t i64x2_add(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] += b.i64[i]; return a; }
static inline v128_t i64x2_sub(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] -= b.i64[i]; return a; }
static inline v128_t i64x2_mul(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.i64[i] *= b.i64[i]; return a; }

static inline v128_t f32x4_ceil (v128_t v) { for (int i=0; i<4; i++) v.f32[i] =  ceilf(v.f32[i]); return v; }
static inline v128_t f32x4_floor(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = floorf(v.f32[i]); return v; }
static inline v128_t f32x4_trunc(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = truncf(v.f32[i]); return v; }
static inline v128_t f32x4_nearest(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = nearbyintf(v.f32[i]); return v; }
static inline v128_t f32x4_abs(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = fabsf(v.f32[i]); return v; }
static inline v128_t f32x4_neg(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = -v.f32[i]; return v; }
static inline v128_t f32x4_sqrt(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = sqrtf(v.f32[i]); return v; }
static inline v128_t f32x4_add(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] += b.f32[i]; return a; }
static inline v128_t f32x4_sub(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] -= b.f32[i]; return a; }
static inline v128_t f32x4_mul(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] *= b.f32[i]; return a; }
static inline v128_t f32x4_div(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] /= b.f32[i]; return a; }
static inline v128_t f32x4_min(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] = op96_f32_min(a.f32[i], b.f32[i]); return a; }
static inline v128_t f32x4_max(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] = op97_f32_max(a.f32[i], b.f32[i]); return a; }
static inline v128_t f32x4_pmin(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] = MINN(a.f32[i], b.f32[i]); return a; }
static inline v128_t f32x4_pmax(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] = MAXN(a.f32[i], b.f32[i]); return a; }
static inline v128_t f32x4_relaxed_min(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] = fminf(a.f32[i], b.f32[i]); return a; }
static inline v128_t f32x4_relaxed_max(v128_t a, v128_t b) { for (int i=0; i<4; i++) a.f32[i] = fmaxf(a.f32[i], b.f32[i]); return a; }
static inline v128_t f32x4_relaxed_madd(v128_t a, v128_t b, v128_t c)  { for (int i=0; i<4; i++) a.f32[i] = fmaf( a.f32[i], b.f32[i], c.f32[i]); return a; }
static inline v128_t f32x4_relaxed_nmadd(v128_t a, v128_t b, v128_t c) { for (int i=0; i<4; i++) a.f32[i] = fmaf(-a.f32[i], b.f32[i], c.f32[i]); return a; }

static inline v128_t f64x2_ceil (v128_t v) { for (int i=0; i<2; i++) v.f64[i] =  ceil(v.f64[i]); return v; }
static inline v128_t f64x2_floor(v128_t v) { for (int i=0; i<2; i++) v.f64[i] = floor(v.f64[i]); return v; }
static inline v128_t f64x2_trunc(v128_t v) { for (int i=0; i<2; i++) v.f64[i] = trunc(v.f64[i]); return v; }
static inline v128_t f64x2_nearest(v128_t v) { for (int i=0; i<2; i++) v.f64[i] = nearbyint(v.f64[i]); return v; }
static inline v128_t f64x2_abs(v128_t v) { for (int i=0; i<2; i++) v.f64[i] = fabs(v.f64[i]); return v; }
static inline v128_t f64x2_neg(v128_t v) { for (int i=0; i<2; i++) v.f64[i] = -v.f64[i]; return v; }
static inline v128_t f64x2_sqrt(v128_t v) { for (int i=0; i<2; i++) v.f64[i] = sqrt(v.f64[i]); return v; }
static inline v128_t f64x2_add(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] += b.f64[i]; return a; }
static inline v128_t f64x2_sub(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] -= b.f64[i]; return a; }
static inline v128_t f64x2_mul(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] *= b.f64[i]; return a; }
static inline v128_t f64x2_div(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] /= b.f64[i]; return a; }
static inline v128_t f64x2_min(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] = opA4_f64_min(a.f64[i], b.f64[i]); return a; }
static inline v128_t f64x2_max(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] = opA5_f64_max(a.f64[i], b.f64[i]); return a; }
static inline v128_t f64x2_pmin(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] = MINN(a.f64[i], b.f64[i]); return a; }
static inline v128_t f64x2_pmax(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] = MAXN(a.f64[i], b.f64[i]); return a; }
static inline v128_t f64x2_relaxed_min(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] = fmin(a.f64[i], b.f64[i]); return a; }
static inline v128_t f64x2_relaxed_max(v128_t a, v128_t b) { for (int i=0; i<2; i++) a.f64[i] = fmax(a.f64[i], b.f64[i]); return a; }
static inline v128_t f64x2_relaxed_madd(v128_t a, v128_t b, v128_t c)  { for (int i=0; i<2; i++) a.f64[i] = fma( a.f64[i], b.f64[i], c.f64[i]); return a; }
static inline v128_t f64x2_relaxed_nmadd(v128_t a, v128_t b, v128_t c) { for (int i=0; i<2; i++) a.f64[i] = fma(-a.f64[i], b.f64[i], c.f64[i]); return a; }

// Conversions
static inline v128_t f32x4_demote_f64x2_zero(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.f32[i] = (float) v.f64[i]; return r; }
static inline v128_t f64x2_promote_low_f32x4(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.f64[i] = v.f32[i]; return r; }

static inline v128_t i32x4_relaxed_trunc_f32x4_s(v128_t v) { for (int i=0; i<4; i++) v.i32[i] = (int32_t) v.f32[i]; return v; }
static inline v128_t i32x4_relaxed_trunc_f32x4_u(v128_t v) { for (int i=0; i<4; i++) v.u32[i] = (uint32_t) v.f32[i]; return v; }
static inline v128_t i32x4_relaxed_trunc_f64x2_s_zero(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.i32[i] =  (int32_t) v.f64[i]; return r; }
static inline v128_t i32x4_relaxed_trunc_f64x2_u_zero(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.u32[i] = (uint32_t) v.f64[i]; return r; }
static inline v128_t i32x4_trunc_sat_f32x4_s(v128_t v) { v128_t r={0}; for (int i=0; i<4; i++) r.i32[i] = opFC00_i32_trunc_sat_f32_s(v.f32[i]); return r; }
static inline v128_t i32x4_trunc_sat_f32x4_u(v128_t v) { v128_t r={0}; for (int i=0; i<4; i++) r.u32[i] = opFC01_i32_trunc_sat_f32_u(v.f32[i]); return r; }
static inline v128_t i32x4_trunc_sat_f64x2_s_zero(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.i32[i] = opFC02_i32_trunc_sat_f64_s(v.f64[i]); return r; }
static inline v128_t i32x4_trunc_sat_f64x2_u_zero(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.u32[i] = (uint32_t) opFC07_i64_trunc_sat_f64_u(v.f64[i]); return r; }

static inline v128_t f32x4_convert_i32x4_s(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = (float) v.i32[i]; return v; }
static inline v128_t f32x4_convert_i32x4_u(v128_t v) { for (int i=0; i<4; i++) v.f32[i] = (float) v.u32[i]; return v; }
static inline v128_t f64x2_convert_low_i32x4_s(v128_t v) { v128_t r; for (int i=0; i<2; i++) r.f64[i] = v.i32[i]; return r; }
static inline v128_t f64x2_convert_low_i32x4_u(v128_t v) { v128_t r; for (int i=0; i<2; i++) r.f64[i] = v.u32[i]; return r; }

static inline v128_t i8x16_narrow_i16x8_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) { r.i8[i] = sat8_s(a.i16[i]); r.i8[i+8] = sat8_s(b.i16[i]); } return r; }
static inline v128_t i8x16_narrow_i16x8_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) { r.u8[i] = sat8_u(a.i16[i]); r.i8[i+8] = sat8_u(b.i16[i]); } return r; }
static inline v128_t i16x8_narrow_i32x4_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) { r.i16[i] = sat16_s(a.i32[i]); r.i16[i+4] = sat16_s(b.i32[i]); } return r; }
static inline v128_t i16x8_narrow_i32x4_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) { r.u16[i] = sat16_u(a.i32[i]); r.i16[i+4] = sat16_u(b.i32[i]); } return r; }

static inline v128_t i16x8_extend_low_i8x16_s (v128_t v) { v128_t r={0}; for (int i=0; i<8; i++) r.i16[i] = v.i8[i];   return r; }
static inline v128_t i16x8_extend_high_i8x16_s(v128_t v) { v128_t r={0}; for (int i=0; i<8; i++) r.i16[i] = v.i8[i+8]; return r; }
static inline v128_t i16x8_extend_low_i8x16_u (v128_t v) { v128_t r={0}; for (int i=0; i<8; i++) r.i16[i] = v.u8[i];   return r; }
static inline v128_t i16x8_extend_high_i8x16_u(v128_t v) { v128_t r={0}; for (int i=0; i<8; i++) r.i16[i] = v.u8[i+8]; return r; }

static inline v128_t i32x4_extend_low_i16x8_s (v128_t v) { v128_t r={0}; for (int i=0; i<4; i++) r.i32[i] = v.i16[i];   return r; }
static inline v128_t i32x4_extend_high_i16x8_s(v128_t v) { v128_t r={0}; for (int i=0; i<4; i++) r.i32[i] = v.i16[i+4]; return r; }
static inline v128_t i32x4_extend_low_i16x8_u (v128_t v) { v128_t r={0}; for (int i=0; i<4; i++) r.i32[i] = v.u16[i];   return r; }
static inline v128_t i32x4_extend_high_i16x8_u(v128_t v) { v128_t r={0}; for (int i=0; i<4; i++) r.i32[i] = v.u16[i+4]; return r; }

static inline v128_t i64x2_extend_low_i32x4_s (v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.i64[i] = v.i32[i];   return r; }
static inline v128_t i64x2_extend_high_i32x4_s(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.i64[i] = v.i32[i+2]; return r; }
static inline v128_t i64x2_extend_low_i32x4_u (v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.i64[i] = v.u32[i];   return r; }
static inline v128_t i64x2_extend_high_i32x4_u(v128_t v) { v128_t r={0}; for (int i=0; i<2; i++) r.i64[i] = v.u32[i+2]; return r; }

// Extending math
static inline v128_t i16x8_extadd_pairwise_i8x16_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) r.i16[i] = (int16_t)  a.i8[i*2] + (int16_t)  b.i8[i*2+1]; return r; }
static inline v128_t i16x8_extadd_pairwise_i8x16_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) r.u16[i] = (uint16_t) a.u8[i*2] + (uint16_t) b.u8[i*2+1]; return r; }
static inline v128_t i16x8_extmul_low_i8x16_s (v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) r.i16[i] = (int16_t) a.i8[i]   * (int16_t) b.i8[i];   return r; }
static inline v128_t i16x8_extmul_high_i8x16_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) r.i16[i] = (int16_t) a.i8[i+8] * (int16_t) b.i8[i+8]; return r; }
static inline v128_t i16x8_extmul_low_i8x16_u (v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) r.u16[i] = (int16_t) a.u8[i]   * (int16_t) b.u8[i];   return r; }
static inline v128_t i16x8_extmul_high_i8x16_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) r.u16[i] = (int16_t) a.u8[i+8] * (int16_t) b.u8[i+8]; return r; }

static inline v128_t i32x4_extadd_pairwise_i16x8_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) r.i32[i] = (int32_t)  a.i16[i*2] + (int32_t)  b.i16[i*2+1]; return r; }
static inline v128_t i32x4_extadd_pairwise_i16x8_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) r.u32[i] = (uint32_t) a.u16[i*2] + (uint32_t) b.u16[i*2+1]; return r; }
static inline v128_t i32x4_extmul_low_i16x8_s (v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) r.i32[i] = (int32_t) a.i16[i]   * (int32_t) b.i16[i];   return r; }
static inline v128_t i32x4_extmul_high_i16x8_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) r.i32[i] = (int32_t) a.i16[i+4] * (int32_t) b.i16[i+4]; return r; }
static inline v128_t i32x4_extmul_low_i16x8_u (v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) r.u32[i] = (int32_t) a.u16[i]   * (int32_t) b.u16[i];   return r; }
static inline v128_t i32x4_extmul_high_i16x8_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) r.u32[i] = (int32_t) a.u16[i+4] * (int32_t) b.u16[i+4]; return r; }

static inline v128_t i64x2_extmul_low_i32x4_s (v128_t a, v128_t b) { v128_t r; for (int i=0; i<2; i++) r.i64[i] = (int64_t) a.i32[i]   * (int64_t) b.i32[i];   return r; }
static inline v128_t i64x2_extmul_high_i32x4_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<2; i++) r.i64[i] = (int64_t) a.i32[i+2] * (int64_t) b.i32[i+2]; return r; }
static inline v128_t i64x2_extmul_low_i32x4_u (v128_t a, v128_t b) { v128_t r; for (int i=0; i<2; i++) r.u64[i] = (int64_t) a.u32[i]   * (int64_t) b.u32[i];   return r; }
static inline v128_t i64x2_extmul_high_i32x4_u(v128_t a, v128_t b) { v128_t r; for (int i=0; i<2; i++) r.u64[i] = (int64_t) a.u32[i+2] * (int64_t) b.u32[i+2]; return r; }

// Dot math
static inline v128_t i32x4_dot_i16x8_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<4; i++) { int i1=i*2, i2=i1+1; r.i32[i] = (int32_t)a.i16[i1] * (int32_t)b.i16[i1] + (int32_t)a.i16[i2] * (int32_t)b.i16[i2]; } return r; }
static inline int ext_i7(int8_t v) { return (int) (((int8_t) v << 1) >> 1); }
static inline v128_t opFD112_i16x8_relaxed_dot_i8x16_i7x16_s(v128_t a, v128_t b) { v128_t r; for (int i=0; i<8; i++) { int i1=i*2, i2=i1+1; r.i16[i] = (int16_t)a.i8[i1] * ext_i7(b.i8[i1]) + (int16_t)a.i8[i2] * ext_i7(b.i8[i2]); } return r; }
static inline v128_t opFD113_i32x4_relaxed_dot_i8x16_i7x16_add_s(v128_t a, v128_t b, v128_t c) { v128_t r; for (int i=0; i<4; i++) { int i4 = i * 4; int32_t d = 0; for (int j=i4; j<i4+4; j++) d += (int32_t) a.i8[j] * ext_i7(b.i8[j]); r.i32[i] = d + c.i32[i]; } return r; }
static inline float bf16(uint16_t bf) { return u32_as_float((uint32_t) bf << 16); }
static inline v128_t opFD114_f32x4_relaxed_dot_bf16x8_add_f32x4(v128_t a, v128_t b, v128_t c) { v128_t r; for (int i=0; i<4; i++) { int i1=i*2, i2=i1+1; r.f32[i] = bf16(a.u16[i1]) * bf16(b.u16[i1]) + bf16(a.u16[i2]) * bf16(b.u16[i2]) + c.f32[i]; } return r; }
