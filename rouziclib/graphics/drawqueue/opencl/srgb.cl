#include "rand.cl"

float4 colour_blowout(float4 c)
{
	float maxv, t, L;
	
	maxv = max(max(c.s0, c.s1), c.s2);	// max is the maximum value of the 3 colours
	
	if (maxv > 1.f)			// if the colour is out of gamut
	{
		L = 0.16f*c.s0 + 0.73f*c.s1 + 0.11f*c.s2;	// Luminosity of the colour's grey point
		
		if (L < 1.f)		// if the grey point is no brighter than white
		{
			t = (1.f-L) / (maxv-L);
		
			c.s0 = c.s0*t + L*(1.f-t);
			c.s1 = c.s1*t + L*(1.f-t);
			c.s2 = c.s2*t + L*(1.f-t);
		}
		else			// if it's too bright regardless of saturation
		{
			c.s0 = c.s1 = c.s2 = 1.f;
		}
	}

	return c;
}

float lsrgb(float linear)	// converts a [0.0, 1.0] linear value into a [0.0, 1.0] sRGB value
{
	if (linear <= 0.0031308f)
		return linear * 12.92f;
	else
		return 1.055f * pow(linear, 1.f/2.4f) - 0.055f;
}

float slrgb(float s)	// converts a [0.0, 1.0] sRGB value into a [0.0, 1.0] linear value
{
	if (s <= 0.04045f)
		return s * (1.f/12.92f);
	else
		return pow((s + 0.055f) * (1.f/1.055f), 2.4f);
}

float s8lrgb(float s8)
{
	return slrgb(s8 * (1.f/255.f));
}

#define max_s 255.f

float apply_dithering(float pv, float dv)
{
	const float threshold = 1.2f / max_s;
	const float it = 1.f / threshold;
	const float rounding_offset = 0.5f / max_s;

	// Reduce the scale of the dithering if pv is close to 0
	if (pv < threshold)	// 1.2 is the threshold so that the crushing happens at 1.2*sqrt(2) = 1.7 sigma
		if (pv <= 0.f)
			return 0.f;
		else
			dv *= pv * it;

	// Same if pv is close to 1
	if (pv > 1.f - threshold)
		if (pv >= 1.f)
			return 1.f;
		else
			dv *= (1.f-pv) * it;

	return pv += dv + rounding_offset;
}

float4 linear_to_srgb(float4 pl0, uint seed)
{
	float4 pl1;
	float dith;
	const float dith_scale = M_SQRT1_2_F / max_s;

	pl0 = mix(colour_blowout(pl0), clamp(pl0, 0.f, 1.f), 0.666f);
	//pl0 = clamp(pl0, 0.f, 1.f);

	pl1.s0 = lsrgb(pl0.s0);		// blue
	pl1.s1 = lsrgb(pl0.s1);		// green
	pl1.s2 = lsrgb(pl0.s2);		// red

	// Dithering
	dith = gaussian_rand_minstd_approx(seed) * dith_scale;

	pl1.s0 = apply_dithering(pl1.s0, dith);
	pl1.s1 = apply_dithering(pl1.s1, dith);
	pl1.s2 = apply_dithering(pl1.s2, dith);

	// Lower bit depth simulation
	if (max_s < 255.f)
		pl1 = round(pl1 * max_s) / max_s;

	return pl1;
}
