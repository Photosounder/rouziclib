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

float3 lsrgb(float3 l)	// converts a [0.0, 1.0] linear value into a [0.0, 1.0] sRGB value
{
	float3 x, line, curve;

	// 13 FR every time + 2 FR once
	line = l * 12.92f;	// 1 FR
	x = native_sqrt(l);	// 4 FR
	curve = ((((0.455f*x - 1.48f)*x + 1.92137f)*x - 1.373254f)*x + 1.51733216f)*x - 0.0404733783f;		// 5 FR + 2 FR once, error 0.145 sRGB units

	return select(line, curve, l > 0.0031308f);	// 3 FR
}

float3 slrgb(float3 s)	// converts a [0.0, 1.0] sRGB value into a [0.0, 1.0] linear value
{
	float3 line, curve;

	// ~8 FR
	line = s * (1.f/12.92f);
	curve = ((((0.05757f*s - 0.2357f)*s + 0.60668f)*s + 0.540468f)*s + 0.0299805f)*s + 0.001010107f;	// error 0.051 sRGB units

	return select(line, curve, s > 0.04045f);
}

float3 s8lrgb(float3 s8)
{
	return slrgb(s8 * (1.f/255.f));
}

#define max_s 255.f

float4 linear_to_srgb(float4 pl, uint seed)
{
	const float dith_scale = M_SQRT1_2_F / max_s;
	const float trans0 = 0.3f * dith_scale;
	const float trans1 = 0.9f * dith_scale;
	const float trans_mul =  1.f / (trans1-trans0);

	// Convert to sRGB
	float3 ps = lsrgb(clamp(pl.xyz, 0.f, 1.f));

	// Random values for dithering
	uint rand_uint = rand_xsm32(seed);
	float rdm_u = (float) rand_uint * (2.3283064e-10f / max_s) - (0.5f / max_s);			// uniform distribution in [-0.5 , 0.5] sRGB units
	float rdm_g = rand01_to_gaussian_approx((float) rand_uint * 2.3283064e-10f) * dith_scale;	// Gaussian distribution in [-3.54 , 3.54] sRGB units

	// Apply dithering
	float3 t = clamp((min(ps, max_s-ps) - trans0) * trans_mul, 0.f, 1.f);
	float3 rdm = mix(rdm_u, rdm_g, t);
	ps = clamp(ps + rdm, 0.f, 1.f);

	// Lower bit depth simulation
	if (max_s < 255.f)
		ps = round(ps * max_s) * (1.f/max_s);

	return (float4)(ps, 1.f);
}
