enum
{
 DQ_END = 0,
 DQ_ENTRY_START = 1,
 DQ_END_HEADER_SL,
};
enum dq_type
{
 DQT_NOTYPE,
 DQT_BRACKET_OPEN,
 DQT_BRACKET_CLOSE,
 DQT_LINE_THIN_ADD,
 DQT_POINT_ADD,
 DQT_RECT_FULL,
 DQT_RECT_BLACK,
 DQT_PLAIN_FILL,
 DQT_GAIN,
 DQT_GAIN_PARAB,
 DQT_LUMA_COMPRESS,
 DQT_COL_MATRIX,
 DQT_CLIP,
 DQT_CLAMP,
 DQT_CIRCLE_FULL,
 DQT_CIRCLE_HOLLOW,
 DQT_BLIT_BILINEAR,
 DQT_BLIT_FLATTOP,
 DQT_BLIT_FLATTOP_ROT,
 DQT_BLIT_PHOTO,
 DQT_TEST1,
};
enum dq_blend
{
 DQB_ADD,
 DQB_SUB,
 DQB_MUL,
 DQB_DIV,
 DQB_BLEND,
 DQB_SOLID,
};
float gaussian(float x)
{
 return native_exp(-x*x);
}
float erf_fast(float x)
{
 float y, xa = min(fabs(x), 4.45f);
 y = ((((-0.001133515f*xa + 0.00830125f)*xa - 0.003218f)*xa - 0.06928657f)*xa - 0.14108256f)*xa + 1.f;
 y = y*y;
 y = y*y;
 y = y*y;
 y = 1.f - y;
y *= sign(x);
 return y;
}
float4 blend_pixel(float4 bg, float4 fg, int blendmode)
{
 float4 pv;
 switch (blendmode)
 {
  case DQB_ADD:
   pv = bg + fg;
   break;
  case DQB_SUB:
   pv = bg - fg;
   break;
  case DQB_MUL:
   pv = bg * fg;
   break;
  case DQB_DIV:
   pv = bg / fg;
   break;
  case DQB_BLEND:
   pv = fg * fg.w + bg * (1.f - fg.w);
   break;
  case DQB_SOLID:
   pv = fg;
   break;
 }
 return pv;
}
float4 draw_line_thin_add(global float *le, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 float v, r1x, r1y, r2x, costh, sinth;
 float2 rp;
 float4 col;
 r1x = le[0];
 r1y = le[1];
 r2x = le[2];
 costh = le[3];
 sinth = le[4];
 col.s0 = le[5];
 col.s1 = le[6];
 col.s2 = le[7];
 col.s3 = le[8];
 rp.y = pf.x * sinth + pf.y * costh;
 rp.x = pf.x * costh - pf.y * sinth;
 v = (erf_fast(rp.x-r1x) - erf_fast(rp.x-r2x)) * 0.5f;
 v *= gaussian(rp.y-r1y);
 pv += v * col;
 return pv;
}
float4 draw_point_add(global float *le, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 const float gl = 4.f;
 float rad, d;
 float2 dp;
 float4 col;
 dp.x = le[0];
 dp.y = le[1];
 rad = le[2];
 col.s0 = le[3];
 col.s1 = le[4];
 col.s2 = le[5];
 col.s3 = 1.f;
 d = fast_distance(dp, pf) * rad;
 pv += gaussian(d) * col;
 return pv;
}
float4 draw_plain_fill_add(global float *le, float4 pv)
{
 float4 col;
 col.s0 = le[0];
 col.s1 = le[1];
 col.s2 = le[2];
 col.s3 = 1.f;
 pv += col;
 return pv;
}
float4 draw_rect_full_add(global float *le, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 float rad, d;
 float2 p0, p1, d0, d1, gv;
 float4 col;
 p0.x = le[0];
 p0.y = le[1];
 p1.x = le[2];
 p1.y = le[3];
 rad = le[4];
 col.s0 = le[5];
 col.s1 = le[6];
 col.s2 = le[7];
 col.s3 = 1.f;
 d0 = (pf - p0) * rad;
 d1 = (pf - p1) * rad;
 gv.x = (erf_fast(d0.x) - erf_fast(d1.x)) * 0.5f;
 gv.y = (erf_fast(d0.y) - erf_fast(d1.y)) * 0.5f;
 pv += gv.x*gv.y * col;
 return pv;
}
float4 draw_black_rect(global float *le, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 float rad, d;
 float2 p0, p1, d0, d1, gv;
 p0.x = le[0];
 p0.y = le[1];
 p1.x = le[2];
 p1.y = le[3];
 rad = le[4];
 d0 = (pf - p0) * rad;
 d1 = (pf - p1) * rad;
 gv.x = (erf_fast(d0.x) - erf_fast(d1.x)) * 0.5f;
 gv.y = (erf_fast(d0.y) - erf_fast(d1.y)) * 0.5f;
 pv *= 1.f - gv.x*gv.y;
 return pv;
}
float4 draw_circle_full_add(global float *le, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 float4 col;
 float2 pc;
 float circrad, rad, dc, dn, df;
 pc.x = le[0];
 pc.y = le[1];
 circrad = le[2];
 rad = le[3];
 col.s0 = le[4];
 col.s1 = le[5];
 col.s2 = le[6];
 col.s3 = 1.f;
 dc = fast_distance(pf, pc);
 dn = (circrad - dc) * rad;
 df = -(circrad + dc) * rad;
 pv += (erf_fast(dn) - erf_fast(df)) *0.5f * col;
 return pv;
}
float4 draw_circle_hollow_add(global float *le, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 float4 col;
 float2 pc;
 float circrad, rad, dc, dn, df;
 pc.x = le[0];
 pc.y = le[1];
 circrad = le[2];
 rad = le[3];
 col.s0 = le[4];
 col.s1 = le[5];
 col.s2 = le[6];
 col.s3 = 1.f;
 dc = fast_distance(pf, pc);
 dn = (circrad - dc) * rad;
 df = -(circrad + dc) * rad;
 pv += (gaussian(dn) + gaussian(df)) * col;
 return pv;
}
int pow_mod(int base, uint expon, uint mod)
{
 int x = 1, power = base % mod;
 for (; expon > 0; expon >>= 1)
 {
  if (expon & 1)
   x = (x * power) % mod;
  power = (power * power) % mod;
 }
 return x;
}
uint rand_minstd(uint pos)
{
 return pow_mod(16807, pos, 2147483647);
}
uint rand_minstd16(uint pos)
{
 return rand_minstd(pos) >> 13 & 0xFFFF;
}
uint rand_minstd32(uint pos)
{
 return rand_minstd16(pos) << 16 | rand_minstd16(pos + 0x80000000);
}
float rand_minstd_01(uint pos)
{
 return (float) rand_minstd32(pos) * 2.3283064e-10f;
}
float rand_minstd_exc01(uint pos)
{
 return (float) (rand_minstd32(pos) + 1) * 0.00000000023283064365386962890625f;
}
float gaussian_rand_minstd(uint pos)
{
 float r;
 pos <<= 1;
 r = native_sqrt(-2.f * native_log(rand_minstd_exc01(pos)));
 return r * native_sin(2.f*M_PI_F * rand_minstd_01(pos+1)) * M_SQRT1_2_F;
}
float gaussian_rand_minstd_approx(uint pos)
{
 float r = ((float) rand_minstd(pos) - 1073741823.f) * 9.3132254e-10f;
 return copysign(0.88622693f * native_sqrt(- native_log(1.f - r*r)), r);
}
float4 colour_blowout(float4 c)
{
 float maxv, t, L;
 maxv = max(max(c.s0, c.s1), c.s2);
 if (maxv > 1.f)
 {
  L = 0.16f*c.s0 + 0.73f*c.s1 + 0.11f*c.s2;
  if (L < 1.f)
  {
   t = (1.f-L) / (maxv-L);
   c.s0 = c.s0*t + L*(1.f-t);
   c.s1 = c.s1*t + L*(1.f-t);
   c.s2 = c.s2*t + L*(1.f-t);
  }
  else
  {
   c.s0 = c.s1 = c.s2 = 1.f;
  }
 }
 return c;
}
float lsrgb(float l)
{
 float x, line, curve;
 line = l * 12.92f;
 x = native_sqrt(l);
 curve = ((((0.455f*x - 1.48f)*x + 1.92137f)*x - 1.373254f)*x + 1.51733216f)*x - 0.0404733783f;
 return l <= 0.0031308f ? line : curve;
}
float slrgb(float s)
{
 float line, curve;
 line = s * (1.f/12.92f);
 curve = ((((0.05757f*s - 0.2357f)*s + 0.60668f)*s + 0.540468f)*s + 0.0299805f)*s + 0.001010107f;
 return s <= 0.04045f ? line : curve;
}
float s8lrgb(float s8)
{
 return slrgb(s8 * (1.f/255.f));
}
float apply_dithering(float pv, float dv)
{
 const float threshold = 1.2f / 255.f;
 const float it = 1.f / threshold;
 const float rounding_offset = 0.5f / 255.f;
 if (pv < threshold)
 {
  if (pv <= 0.f)
   return 0.f;
  else
   dv *= pv * it;
 }
 if (pv > 1.f - threshold)
 {
  if (pv >= 1.f)
   return 1.f;
  else
   dv *= (1.f-pv) * it;
 }
 return pv += dv + rounding_offset;
}
float4 linear_to_srgb(float4 pl0, uint seed)
{
 float4 pl1;
 float dith;
 const float dith_scale = M_SQRT1_2_F / 255.f;
 pl0 = clamp(pl0, 0.f, 1.f);
 pl1.s0 = lsrgb(pl0.s0);
 pl1.s1 = lsrgb(pl0.s1);
 pl1.s2 = lsrgb(pl0.s2);
 dith = gaussian_rand_minstd_approx(seed) * dith_scale;
 pl1.s0 = apply_dithering(pl1.s0, dith);
 pl1.s1 = apply_dithering(pl1.s1, dith);
 pl1.s2 = apply_dithering(pl1.s2, dith);
 if (255.f < 255.f)
  pl1 = round(pl1 * 255.f) / 255.f;
 return pl1;
}
float Lab_L_to_linear(float t)
{
 const float stn=6.f/29.f;
 t = (t+0.16f) / 1.16f;
 if (t > stn)
  return t*t*t;
 else
  return 3.f*stn*stn*(t - 4.f/29.f);
}
float linear_to_Lab_L(float t)
{
 const float thr = 6.f/29.f, thr3 = thr*thr*thr;
 if (t > thr3)
  t = cbrt(t);
 else
  t = t * 841.f/108.f + 4.f/29.f;
 return 1.16f * t - 0.16f;
}
float4 gain_parabolic(float4 pv, float gain)
{
 pv = min(pv, 1.f);
 pv = 1.f - pow((1.f-pv), gain);
 return pv;
}
float4 luma_compression(float4 pv0, float lvl_lin)
{
 float4 pv1;
 float grey0_lin, grey0_perc, grey1_perc, grey1_lin, white1_perc, white1_lin, lvl_perc, ratio;
 grey0_lin = 0.16f*pv0.s0 + 0.73f*pv0.s1 + 0.11f*pv0.s2;
 if (grey0_lin==0.f)
  return pv0;
 grey0_perc = linear_to_Lab_L(grey0_lin);
 lvl_perc = linear_to_Lab_L(lvl_lin);
 grey1_perc = grey0_perc + lvl_perc;
 grey1_lin = Lab_L_to_linear(grey1_perc) - lvl_lin;
 white1_perc = 1.f + lvl_perc;
 white1_lin = Lab_L_to_linear(white1_perc) - lvl_lin;
 ratio = (grey1_lin / grey0_lin) / white1_lin;
 pv1 = pv0 * ratio;
 return pv1;
}
float4 colour_matrix(global float *le, float4 pv)
{
 float4 v;
 v.x = le[0]*pv.x + le[3]*pv.y + le[6]*pv.z;
 v.y = le[1]*pv.x + le[4]*pv.y + le[7]*pv.z;
 v.z = le[2]*pv.x + le[5]*pv.y + le[8]*pv.z;
 v.w = pv.w;
 return v;
}
float hue_to_channel(float oh)
{
 float t;
 oh -= 3.f*floor((oh+1.f) * (1.f/3.f));
 t = fabs(clamp(oh, -1.f, 1.f));
 if (t <= 0.5f)
  return 1.f;
 else
  return Lab_L_to_linear(2.f * (1.f - t));
}
float3 hsl_to_rgb_cw(float3 w, float3 hsl)
{
 float3 rgb, rgbw;
 float Y;
 rgb.x = hue_to_channel(hsl.x);
 rgb.y = hue_to_channel(hsl.x-1.f);
 rgb.z = hue_to_channel(hsl.x-2.f);
 rgbw = rgb * w;
 Y = rgbw.x + rgbw.y + rgbw.z;
 Y = hsl.z / Y;
 rgb *= Y;
 rgb = mix(hsl.z, rgb, hsl.y);
 return rgb;
}
int idiv_ceil(int a, int b)
{
 int d = a / b;
 if (d*b < a)
  d += 1;
 return d;
}
ulong get_bits_in_stream(global uchar *stream, ulong start_bit, uint bit_count)
{
 ulong r=0, b, start_byte, actual_start_bit;
 int bits_to_read, b_sh;
 uchar mask;
 if (bit_count==0)
  return 0;
 start_byte = start_bit >> 3;
 start_bit &= 7;
 bits_to_read = min((int) (8-start_bit), (int) bit_count);
 b_sh = max((int) 0, (int) (bit_count - bits_to_read));
 while (bit_count > 0)
 {
  bits_to_read = min((int) (8-start_bit), (int) bit_count);
  actual_start_bit = 8-start_bit - bits_to_read;
  mask = (((1<<bits_to_read)-1) << actual_start_bit);
  b = (stream[start_byte] & mask) >> actual_start_bit;
  r |= b << b_sh;
  b_sh = max((int) 0, (int) (b_sh-8));
  bit_count -= bits_to_read;
  start_bit = 0;
  start_byte++;
 }
 return r;
}
ulong get_bits_in_stream_inc(global uchar *stream, ulong *start_bit, uint bit_count)
{
 ulong r = get_bits_in_stream(stream, *start_bit, bit_count);
 *start_bit += bit_count;
 return r;
}
float4 read_frgb_pixel(global float4 *im, int index)
{
 return im[index];
}
float4 read_sqrgb_pixel(global uint *im, int index)
{
 float4 pv;
 uint4 pvi;
 uint v;
 const float mul_rb = 1.f / (1023.f*1023.f);
 const float mul_g = 1.f / (4092.f*4092.f);
 const float4 mul_pvi = (float4) (mul_rb, mul_g, mul_rb, 1.f);
 v = im[index];
 pvi.z = v >> 22;
 pvi.y = (v >> 10) & 4095;
 pvi.x = v & 1023;
 pvi.w = 1;
 pv = convert_float4(pvi*pvi) * mul_pvi;
 return pv;
}
float4 read_srgb_pixel(global uint *im, int index)
{
 float4 pv;
 uint v;
 v = im[index];
 pv.z = s8lrgb((v >> 16) & 255);
 pv.y = s8lrgb((v >> 8) & 255);
 pv.x = s8lrgb(v & 255);
 pv.w = 1.f;
 return pv;
}
float4 raw_yuv_to_lrgb(float3 raw, float depth_mul)
{
 float y, u, v, r, g, b;
 float4 pv;
 raw *= depth_mul;
 y = raw.x - 16.f;
 y *= 255.f / 219.f;
 u = raw.y - 128.f;
 v = raw.z - 128.f;
 r = y + 1.596f * v;
        g = y - 0.813f * v - 0.391f * u;
        b = y + 2.018f * u;
 pv.x = s8lrgb(r);
 pv.y = s8lrgb(g);
 pv.z = s8lrgb(b);
 pv.w = 1.f;
 return pv;
}
float4 read_yuv420p8_pixel(global uchar *im, int2 im_dim, int2 i)
{
 float4 pv;
 float y, u, v, r, g, b;
 int2 im_dimh = im_dim / 2;
 int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
 global uchar *u_plane, *v_plane;
 u_plane = &im[size_full];
 v_plane = &im[size_full + size_half];
 y_index = i.y * im_dim.x + i.x;
 uv_index = i.y/2 * im_dimh.x + i.x/2;
 pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), 1.f );
 return pv;
}
float4 read_yuv420pN_pixel(global ushort *im, int2 im_dim, int2 i, float depth_mul)
{
 float4 pv;
 float y, u, v, r, g, b;
 int2 im_dimh = im_dim / 2;
 int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;
 global ushort *u_plane, *v_plane;
 u_plane = &im[size_full];
 v_plane = &im[size_full + size_half];
 y_index = i.y * im_dim.x + i.x;
 uv_index = i.y/2 * im_dimh.x + i.x/2;
 pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), depth_mul );
 return pv;
}
uint bits_to_mask(uint bits)
{
 return (1UL << bits) - 1;
}
float bits_to_mul(uint bits)
{
 return convert_float((int) ((1ULL << bits) - 1));
}
float4 decompr_rgb(int bits_col, uchar3 c)
{
 float4 rgb;
 float ratio = 1.f / bits_to_mul(bits_col);
 rgb.x = (float) c.x * ratio;
 rgb.y = (float) c.y * ratio;
 rgb.z = (float) c.z * ratio;
 return rgb;
}
float4 compr_hsl_to_rgb(float3 hsl)
{
 float4 rgb;
 float3 w = (float3) (0.124f, 0.686f, 0.19f);
 hsl.x *= 3.f;
 hsl.z = Lab_L_to_linear(hsl.z);
 rgb = (float4) (hsl_to_rgb_cw(w, hsl), 1.f);
 rgb.x = linear_to_Lab_L(rgb.x);
 rgb.y = linear_to_Lab_L(rgb.y);
 rgb.z = linear_to_Lab_L(rgb.z);
 return rgb;
}
typedef struct
{
 int init;
 int block_size, bits_per_block, quincunx, bits_col, bits_per_pixel;
 int linew0, linew1, pix;
 int2 block_pos, block_start;
 ulong di;
 float4 col0, col1, colm, pv;
 float pix_mul;
} comp_decode_t;
float4 read_compressed_texture1_pixel(global uchar *d8, int2 im_dim, int2 i, comp_decode_t *d)
{
 global ushort *d16 = (global ushort *) d8;
 ulong di, block_start_bit = 64;
 int line_count0, line_count1;
 int pix, qoff;
 uchar3 c0, c1, cm;
 int2 block_pos, ib;
 float t;
 if (d->init==0)
 {
  d->block_size = 8;
  d->bits_per_block = 240;
  d->quincunx = 1;
  d->bits_col = 8;
  d->bits_per_pixel = 3;
  d->linew0 = idiv_ceil(im_dim.x, d->block_size);
  d->linew1 = idiv_ceil(im_dim.x + d->quincunx*(d->block_size>>1), d->block_size);
  d->pix_mul = 2.f / bits_to_mul(d->bits_per_pixel);
  d->init = 1;
  d->block_pos = (int2) (-1, -1);
  d->pix = -1;
 }
 block_pos.y = (i.y / d->block_size);
 qoff = (block_pos.y&1) * d->quincunx * (d->block_size>>1);
 block_pos.x = (i.x + qoff) / d->block_size;
 if (block_pos.x != d->block_pos.x || block_pos.y != d->block_pos.y)
 {
  line_count0 = (block_pos.y+1) >> 1;
  line_count1 = block_pos.y >> 1;
  di = line_count0*d->linew0 + line_count1*d->linew1;
  di += block_pos.x;
  di = di*d->bits_per_block + block_start_bit;
  d->block_start = block_pos * d->block_size;
  d->block_start.x -= qoff;
  c0.x = get_bits_in_stream_inc(d8, &di, d->bits_col);
  c0.y = get_bits_in_stream_inc(d8, &di, d->bits_col);
  c0.z = get_bits_in_stream_inc(d8, &di, d->bits_col);
  c1.x = get_bits_in_stream_inc(d8, &di, d->bits_col);
  c1.y = get_bits_in_stream_inc(d8, &di, d->bits_col);
  c1.z = get_bits_in_stream_inc(d8, &di, d->bits_col);
  d->di = di;
  d->col0 = decompr_rgb(d->bits_col, c0);
  d->col1 = decompr_rgb(d->bits_col, c1);
  d->block_pos = block_pos;
  d->pix = -1;
 }
 ib = i - d->block_start;
 di = d->di + (ib.y*d->block_size + ib.x) * d->bits_per_pixel;
 pix = get_bits_in_stream(d8, di, d->bits_per_pixel);
 if (pix != d->pix)
 {
  t = convert_float(pix) * d->pix_mul;
  d->pv = mix(d->col0, d->col1, t*0.5f);
  d->pv.x = Lab_L_to_linear(d->pv.x);
  d->pv.y = Lab_L_to_linear(d->pv.y);
  d->pv.z = Lab_L_to_linear(d->pv.z);
  d->pix = pix;
 }
 return d->pv;
}
float4 read_fmt_pixel(const int fmt, global uchar *im, int2 im_dim, int2 i, comp_decode_t *cd1)
{
 switch (fmt)
 {
  case 0:
   return read_frgb_pixel((global float4 *) im, i.y * im_dim.x + i.x);
  case 1:
   return read_sqrgb_pixel((global uint *) im, i.y * im_dim.x + i.x);
  case 2:
   return read_srgb_pixel((global uint *) im, i.y * im_dim.x + i.x);
  case 10:
   return read_yuv420p8_pixel(im, im_dim, i);
  case 11:
   return read_yuv420pN_pixel((global ushort *) im, im_dim, i, 0.25f);
  case 12:
   return read_yuv420pN_pixel((global ushort *) im, im_dim, i, 0.0625f);
  case 20:
   return read_compressed_texture1_pixel(im, im_dim, i, cd1);
 }
 return 0.f;
}
float calc_flattop_weight(float2 pif, float2 i, float2 knee, float2 slope, float2 pscale)
{
 float2 d, w;
 d = fabs(pif - i);
 d = max(d, knee);
 w = slope * (d - pscale);
 return w.x * w.y;
}
float4 image_filter_flattop(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale, float2 slope)
{
 float4 pv = 0.f;
 float2 knee, i, start, end;
 comp_decode_t cd1={0};
 knee = 0.5f - fabs(fmod(pscale, 1.f) - 0.5f);
 start = max(0.f, ceil(pif - pscale));
 end = min(convert_float2(im_dim - 1), floor(pif + pscale));
 for (i.y = start.y; i.y <= end.y; i.y+=1.f)
  for (i.x = start.x; i.x <= end.x; i.x+=1.f)
   pv += read_fmt_pixel(fmt, (global uchar *) im, im_dim, convert_int2(i), &cd1) * calc_flattop_weight(pif, i, knee, slope, pscale);
 return pv;
}
float4 blit_sprite_flattop(global uint *lei, global uchar *data_cl, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 global float *lef = (global float *) lei;
 global float4 *im;
 int2 im_dim;
 int fmt;
 float2 pscale, pos, pif, slope;
 im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
 im_dim.x = lei[2];
 im_dim.y = lei[3];
 pscale.x = lef[4];
 pscale.y = lef[5];
 pos.x = lef[6];
 pos.y = lef[7];
 fmt = lei[8];
 slope.x = lef[9];
 slope.y = lef[10];
 pif = pscale * (pf + pos);
 pscale = max(1.f, pscale);
 pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);
 return pv;
}
float4 blit_sprite_flattop_rot(global uint *lei, global uchar *data_cl, float4 pv)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const float2 pf = convert_float2(p);
 global float *lef = (global float *) lei;
 global float4 *im;
 int2 im_dim;
 int fmt;
 float2 pscale, pos, pif, pifo, slope;
 float costh, sinth;
 im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];
 im_dim.x = lei[2];
 im_dim.y = lei[3];
 pscale.x = lef[4];
 pscale.y = pscale.x;
 pos.x = lef[5];
 pos.y = lef[6];
 fmt = lei[7];
 slope.x = lef[8];
 slope.y = slope.x;
 costh = lef[9];
 sinth = lef[10];
 pifo = pscale * (pf + pos);
 pif.x = pifo.x * costh - pifo.y * sinth;
 pif.y = pifo.x * sinth + pifo.y * costh;
 pscale = max(1.f, pscale);
 pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);
 return pv;
}
float4 drawgradienttest(float4 pv)
{
 const float2 pf = (float2) (get_global_id(0), get_global_id(1));
 const float2 ss = (float2) (get_global_size(0), get_global_size(1));
 const float2 c = ss * 0.5f;
 pv = -(pf.x - c.x) / (ss.x * 0.1f);
 return pv;
}
float4 draw_queue(global float *df, global int *poslist, global int *entrylist, global uchar *data_cl, const int sector_w, const int sector_size)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const int sec = (p.y >> sector_size) * sector_w + (p.x >> sector_size);
 global int *di = (global int *) df;
 int i, eli, entry_count, qi;
 float4 pv = 0.f;
 int brlvl = 0;
 float4 br[4];
 eli = poslist[sec];
 if (eli < 0)
  return pv;
 entry_count = entrylist[eli];
 for (i=0; i < entry_count; i++)
 {
  qi = entrylist[eli + i + 1];
  switch (di[qi])
  {
   case DQT_BRACKET_OPEN:
    br[brlvl] = pv;
    pv = 0.f;
    brlvl++;
    break;
   case DQT_BRACKET_CLOSE:
    brlvl--;
    pv = blend_pixel(br[brlvl], pv, di[qi+1]);
    break;
   case DQT_LINE_THIN_ADD: pv = draw_line_thin_add(&df[qi+1], pv); break;
   case DQT_POINT_ADD: pv = draw_point_add(&df[qi+1], pv); break;
   case DQT_RECT_FULL: pv = draw_rect_full_add(&df[qi+1], pv); break;
   case DQT_RECT_BLACK: pv = draw_black_rect(&df[qi+1], pv); break;
   case DQT_PLAIN_FILL: pv = draw_plain_fill_add(&df[qi+1], pv); break;
   case DQT_GAIN: pv = pv * df[qi+1]; break;
   case DQT_GAIN_PARAB: pv = gain_parabolic(pv, df[qi+1]); break;
   case DQT_LUMA_COMPRESS: pv = luma_compression(pv, df[qi+1]); break;
   case DQT_COL_MATRIX: pv = colour_matrix(&df[qi+1], pv); break;
   case DQT_CLIP: pv = min(pv, df[qi+1]); break;
   case DQT_CLAMP: pv = clamp(pv, 0.f, 1.f); break;
   case DQT_CIRCLE_FULL: pv = draw_circle_full_add(&df[qi+1], pv); break;
   case DQT_CIRCLE_HOLLOW: pv = draw_circle_hollow_add(&df[qi+1], pv); break;
   case DQT_BLIT_FLATTOP: pv = blit_sprite_flattop((global uint *) &di[qi+1], data_cl, pv); break;
   case DQT_BLIT_FLATTOP_ROT: pv = blit_sprite_flattop_rot((global uint *) &di[qi+1], data_cl, pv); break;
   case DQT_TEST1: pv = drawgradienttest(pv); break;
   default:
    break;
  }
 }
 return pv;
}
kernel __attribute__((reqd_work_group_size(16,16,1))) void draw_queue_srgb_kernel(const ulong df_index, const ulong poslist_index, const ulong entrylist_index, global uchar *data_cl, write_only image2d_t srgb, const int sector_w, const int sector_size, const int randseed)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const int fbi = p.y * get_global_size(0) + p.x;
 float4 pv;
 global float *df = (global float *) &data_cl[df_index];
 global int *poslist = (global int *) &data_cl[poslist_index];
 global int *entrylist = (global int *) &data_cl[entrylist_index];
 pv = draw_queue(df, poslist, entrylist, data_cl, sector_w, sector_size);
 if (pv.s0==0.f)
 if (pv.s1==0.f)
 if (pv.s2==0.f)
 {
  write_imageui(srgb, p, 0);
  return ;
 }
 write_imagef(srgb, p, linear_to_srgb(pv, randseed+fbi));
}
kernel void draw_queue_srgb_buf_kernel(const ulong df_index, const ulong poslist_index, const ulong entrylist_index, global uchar *data_cl, global uchar4 *srgb, const int sector_w, const int sector_size, const int randseed)
{
 const int2 p = (int2) (get_global_id(0), get_global_id(1));
 const int fbi = p.y * get_global_size(0) + p.x;
 float4 pv;
 uchar4 ps;
 global float *df = (global float *) &data_cl[df_index];
 global int *poslist = (global int *) &data_cl[poslist_index];
 global int *entrylist = (global int *) &data_cl[entrylist_index];
 pv = draw_queue(df, poslist, entrylist, data_cl, sector_w, sector_size);
 if (pv.s0==0.f)
 if (pv.s1==0.f)
 if (pv.s2==0.f)
 {
  srgb[fbi] = 0;
  return ;
 }
 ps = convert_uchar4(linear_to_srgb(pv, randseed+fbi) * 255.f);
 srgb[fbi].x = ps.z;
 srgb[fbi].y = ps.y;
 srgb[fbi].z = ps.x;
}
