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
extern double Lab_L_to_linear(double t);
extern void hsl_to_rgb(double H, double S, double L, double *r, double *g, double *b, int huemode, int secboost);
extern void colour_blowout_double(double *pred, double *pgrn, double *pblu);
extern void colour_blowout_int(uint32_t *pred, uint32_t *pgrn, uint32_t *pblu);
