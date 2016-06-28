extern int32_t fastabs(int32_t x);
extern int32_t log2_ffo32(uint32_t x);
extern int32_t log2_ffo64(uint64_t x);
extern int64_t mulshift (int64_t a, int64_t b, int s);

#define fpdiv	fpdiv_d2
#define isqrt	isqrt_d1i
#define fplog2	fplog2_d1i
#define fpexp2	fpexp2_d1i
#define fpcos	fpcos_d2
#define fpatan2	fpatan2_d2
#define fpwsinc	fpwsinc_d1i
#define fpgauss	fpgauss_d0
#define fperfr	fperfr_d0

extern int64_t fpdiv_d2(int32_t y, int32_t x, int32_t outfmt);
extern uint32_t isqrt_d1i(uint64_t x);
extern int32_t fplog2_d1i(uint32_t x);
extern uint32_t fpexp2_d1i(int32_t x);
extern int32_t fppow(uint32_t x, int32_t y, const uint32_t fmt);

#define fpsin(x) fpcos((x)+0xC0000000)
#define fpsin_d2(x) fpcos_d2((x)+0xC0000000)
#define fpsin_d1i(x) fpcos_d1i((x)+0xC0000000)
extern int32_t fpcos_d2(uint32_t x);
extern int32_t fpcos_d1i(uint32_t x);
extern int64_t fpatan2_d2(int32_t y, int32_t x);
extern int32_t fpwsinc_d1i(int32_t x);
extern int32_t fpgauss_d1i(int32_t x);
extern int32_t fpgauss_d0(int32_t x);
extern int32_t fpgauss_d0_nocheck(int32_t x);
extern int32_t fperfr_d1i(int32_t x);
extern int32_t fperfr_d0(int32_t x);
