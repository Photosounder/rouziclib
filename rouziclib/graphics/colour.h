enum
{
	HUERAD,
	HUEDEG,
	HUE03,
};

#define WEIGHT_R 0.125		// perceptual weights for each channel used to compute perceptual luminosity
#define WEIGHT_G 0.680
#define WEIGHT_B 0.195

extern lrgb_t make_colour_lin(double r, double g, double b, double a);
extern lrgb_t make_colour_srgb(int r, int g, int b, int a);
extern lrgb_t make_grey_lin(double v);
extern double Lab_L_to_linear(double t);
extern double linear_to_Lab_L(double t);
extern double Lab_L_invert(double x);
extern void rgb_to_hsl(double r, double g, double b, double *H, double *S, double *L, int huemode);
extern void hsl_to_rgb(double H, double S, double L, double *r, double *g, double *b, int huemode, int secboost);
extern lrgb_t make_colour_hsl(double H, double S, double L, int huemode, int secboost);
extern double get_rgb_channel(lrgb_t col, int ch);
extern void lrgb_to_rgb(lrgb_t col, double *r, double *g, double *b);
extern void lrgb_to_hsl(lrgb_t col, double *H, double *S, double *L, int huemode);
extern void colour_blowout_double(double *pred, double *pgrn, double *pblu);
extern void colour_blowout_int(uint32_t *pred, uint32_t *pgrn, uint32_t *pblu);
