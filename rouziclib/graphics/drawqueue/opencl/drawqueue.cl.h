"enum\n{\n DQ_END = 0,\n DQ_ENTRY_START = 1,\n DQ_END_HEADER_SL,\n};\nenum dq_type\n{\n DQT_NOTYPE,\n DQT_BRACKET_OPEN,\n DQT_BRACKET_CLOSE,\n DQT_LINE_THIN_ADD,\n DQT_POINT_ADD,\n DQT_RECT_FULL,\n DQT_RECT_BLACK,\n DQT_RECT_BLACK_INV,\n DQT_PLAIN_FILL,\n DQT_GAIN,\n DQT_GAIN_PARAB,\n DQT_LUMA_COMPRESS,\n DQT_COL_MATRIX,\n DQT_CLIP,\n DQT_CLAMP,\n DQT_CIRCLE_FULL,\n DQT_CIRCLE_HOLLOW,\n DQT_BLIT_BILINEAR,\n DQT_BLIT_FLATTOP,\n DQT_BLIT_FLATTOP_ROT,\n DQT_BLIT_AANEAREST,\n DQT_BLIT_AANEAREST_ROT,\n DQT_BLIT_PHOTO,\n DQT_TEST1,\n};\nenum dq_blend\n{\n DQB_ADD,\n DQB_SUB,\n DQB_MUL,\n DQB_DIV,\n DQB_BLEND,\n DQB_SOLID,\n};\nfloat gaussian(float x)\n{\n return native_exp(-x*x);\n}\nfloat erf_fast(float x)\n{\n float y, xa = fabs(x);\n y = ((-0.06388f*xa - 0.66186f)*xa - 1.123613f)*xa;\n y = 1.f - native_exp(y);\n y = copysign(y, x);\n return y;\n}\nfloat erf_tri_corner_approx(float x, float y)\n{\n float x2, y2, z;\n x2 = x*x;\n y2 = y*y;\n z = ((( 5.4082e-6f*y2 - 4.5063e-5f )*x2 +\n  ( (7.7728e-6f*y2 - 0.0001990703f)*y2 + 0.00119672f ))*x2 +\n  ( (-0.000136548f*y2 + 0.00256433f)*y2 - 0.0146921f ))*x2 +\n  (((1.209847e-5f*y2 - 0.000377797f)*y2 + 0.00542338f)*y2 - 0.04804065f)*y2 + 0.793529f;\n z *= z;\n z *= z;\n z *= z;\n z *= x * y;\n return z;\n}\nfloat acos_tr_fast_xpositive(float x)\n{\n float r;\n r = (0.0081f*x - 0.0326f)*x + 0.24993433f;\n r *= native_sqrt(1.f - x);\n return r;\n}\nfloat acos_tr_fast(float x)\n{\n float r;\n r = acos_tr_fast_xpositive(fabs(x));\n return x < 0.f ? 0.5f-r : r;\n}\nfloat asin_tr_fast(float x)\n{\n return 0.25f - acos_tr_fast(x);\n}\nfloat4 blend_pixel(float4 bg, float4 fg, int blendmode)\n{\n float4 pv;\n switch (blendmode)\n {\n  case DQB_ADD:\n   pv = bg + fg;\n   break;\n  case DQB_SUB:\n   pv = bg - fg;\n   break;\n  case DQB_MUL:\n   pv = bg * fg;\n   break;\n  case DQB_DIV:\n   pv = bg / fg;\n   break;\n  case DQB_BLEND:\n   pv = fg * fg.w + bg * (1.f - fg.w);\n   break;\n  case DQB_SOLID:\n   pv = fg;\n   break;\n }\n return pv;\n}\nfloat4 draw_line_thin_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_gl"
"obal_id(1));\n const float2 pf = convert_float2(p);\n float v, r1x, r1y, r2x, costh, sinth;\n float2 rp;\n float4 col;\n r1x = le[0];\n r1y = le[1];\n r2x = le[2];\n costh = le[3];\n sinth = le[4];\n col.s0 = le[5];\n col.s1 = le[6];\n col.s2 = le[7];\n col.s3 = le[8];\n rp.y = pf.x * sinth + pf.y * costh;\n rp.x = pf.x * costh - pf.y * sinth;\n v = (erf_fast(rp.x-r1x) - erf_fast(rp.x-r2x)) * 0.5f;\n v *= gaussian(rp.y-r1y);\n pv += v * col;\n return pv;\n}\nfloat4 draw_point_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n const float gl = 4.f;\n float rad, d;\n float2 dp;\n float4 col;\n dp.x = le[0];\n dp.y = le[1];\n rad = le[2];\n col.s0 = le[3];\n col.s1 = le[4];\n col.s2 = le[5];\n col.s3 = 1.f;\n d = fast_distance(dp, pf) * rad;\n pv += gaussian(d) * col;\n return pv;\n}\nfloat4 draw_plain_fill_add(global float *le, float4 pv)\n{\n float4 col;\n col.s0 = le[0];\n col.s1 = le[1];\n col.s2 = le[2];\n col.s3 = 1.f;\n pv += col;\n return pv;\n}\nfloat4 draw_rect_full_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float rad, d;\n float2 p0, p1, d0, d1, gv;\n float4 col;\n p0.x = le[0];\n p0.y = le[1];\n p1.x = le[2];\n p1.y = le[3];\n rad = le[4];\n col.s0 = le[5];\n col.s1 = le[6];\n col.s2 = le[7];\n col.s3 = 1.f;\n d0 = (pf - p0) * rad;\n d1 = (pf - p1) * rad;\n gv.x = erf_fast(d0.x) - erf_fast(d1.x);\n gv.y = erf_fast(d0.y) - erf_fast(d1.y);\n pv += gv.x*gv.y*0.25f * col;\n return pv;\n}\nfloat4 draw_black_rect(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float rad, d, intensity;\n float2 p0, p1, d0, d1, gv;\n p0.x = le[0];\n p0.y = le[1];\n p1.x = le[2];\n p1.y = le[3];\n rad = le[4];\n intensity = le[5];\n d0 = (pf - p0) * rad;\n d1 = (pf - p1) * rad;\n gv.x = erf_fast(d0.x) - erf_fast(d1.x);\n gv.y = erf_fast(d0.y) - erf_fast(d1.y);\n pv *= 1.f - 0.25f*gv.x*gv.y*intensity;\n re"
"turn pv;\n}\nfloat4 draw_black_rect_inv(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float rad, d, intensity;\n float2 p0, p1, d0, d1, gv;\n p0.x = le[0];\n p0.y = le[1];\n p1.x = le[2];\n p1.y = le[3];\n rad = le[4];\n intensity = le[5];\n d0 = (pf - p0) * rad;\n d1 = (pf - p1) * rad;\n gv.x = erf_fast(d0.x) - erf_fast(d1.x);\n gv.y = erf_fast(d0.y) - erf_fast(d1.y);\n pv *= 0.25f*gv.x*gv.y * intensity + (1.f-intensity);\n return pv;\n}\nfloat4 draw_circle_full_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float4 col;\n float2 pc;\n float circrad, rad, dc, dn, df;\n pc.x = le[0];\n pc.y = le[1];\n circrad = le[2];\n rad = le[3];\n col.s0 = le[4];\n col.s1 = le[5];\n col.s2 = le[6];\n col.s3 = 1.f;\n dc = fast_distance(pf, pc);\n dn = (circrad - dc) * rad;\n df = -(circrad + dc) * rad;\n pv += (erf_fast(dn) - erf_fast(df)) *0.5f * col;\n return pv;\n}\nfloat4 draw_circle_hollow_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float4 col;\n float2 pc;\n float circrad, rad, dc, dn, df;\n pc.x = le[0];\n pc.y = le[1];\n circrad = le[2];\n rad = le[3];\n col.s0 = le[4];\n col.s1 = le[5];\n col.s2 = le[6];\n col.s3 = 1.f;\n dc = fast_distance(pf, pc);\n dn = (circrad - dc) * rad;\n df = -(circrad + dc) * rad;\n pv += (gaussian(dn) + gaussian(df)) * col;\n return pv;\n}\nuint rand_xsm32(uint x)\n{\n x ^= x >> 15;\n x *= 0xd168aaadu;\n x ^= x >> 15;\n x *= 0xaf723597u;\n x ^= x >> 15;\n return x;\n}\nfloat gaussian_rand_approx(uint pos)\n{\n float r = ((float) rand_xsm32(pos) - 2147483647.5f) * 4.6566125e-10f;\n return copysign(0.88622693f * native_sqrt(- native_log(1.f - r*r)), r);\n}\nfloat4 colour_blowout(float4 c)\n{\n float maxv, t, L;\n maxv = max(max(c.s0, c.s1), c.s2);\n if (maxv > 1.f)\n {\n  L = 0.16f*c.s0 + 0.73f*c.s1 + 0.11f*c.s2;\n  if (L < 1.f)\n  {\n   t = (1.f-L) / (maxv-L)"
";\n   c.s0 = c.s0*t + L*(1.f-t);\n   c.s1 = c.s1*t + L*(1.f-t);\n   c.s2 = c.s2*t + L*(1.f-t);\n  }\n  else\n  {\n   c.s0 = c.s1 = c.s2 = 1.f;\n  }\n }\n return c;\n}\nfloat lsrgb(float l)\n{\n float x, line, curve;\n line = l * 12.92f;\n x = native_sqrt(l);\n curve = ((((0.455f*x - 1.48f)*x + 1.92137f)*x - 1.373254f)*x + 1.51733216f)*x - 0.0404733783f;\n return l <= 0.0031308f ? line : curve;\n}\nfloat slrgb(float s)\n{\n float line, curve;\n line = s * (1.f/12.92f);\n curve = ((((0.05757f*s - 0.2357f)*s + 0.60668f)*s + 0.540468f)*s + 0.0299805f)*s + 0.001010107f;\n return s <= 0.04045f ? line : curve;\n}\nfloat s8lrgb(float s8)\n{\n return slrgb(s8 * (1.f/255.f));\n}\nfloat apply_dithering(float pv, float dv)\n{\n const float threshold = 1.2f / 255.f;\n const float it = 1.f / threshold;\n const float rounding_offset = 0.5f / 255.f;\n if (pv < threshold)\n {\n  if (pv <= 0.f)\n   return 0.f;\n  else\n   dv *= pv * it;\n }\n if (pv > 1.f - threshold)\n {\n  if (pv >= 1.f)\n   return 1.f;\n  else\n   dv *= (1.f-pv) * it;\n }\n return pv += dv + rounding_offset;\n}\nfloat4 linear_to_srgb(float4 pl0, uint seed)\n{\n float4 pl1;\n float dith;\n const float dith_scale = M_SQRT1_2_F / 255.f;\n pl0 = clamp(pl0, 0.f, 1.f);\n pl1.s0 = lsrgb(pl0.s0);\n pl1.s1 = lsrgb(pl0.s1);\n pl1.s2 = lsrgb(pl0.s2);\n dith = gaussian_rand_approx(seed) * dith_scale;\n pl1.s0 = apply_dithering(pl1.s0, dith);\n pl1.s1 = apply_dithering(pl1.s1, dith);\n pl1.s2 = apply_dithering(pl1.s2, dith);\n if (255.f < 255.f)\n  pl1 = round(pl1 * 255.f) / 255.f;\n return pl1;\n}\nfloat Lab_L_to_linear(float t)\n{\n const float stn=6.f/29.f;\n t = (t+0.16f) / 1.16f;\n if (t > stn)\n  return t*t*t;\n else\n  return 3.f*stn*stn*(t - 4.f/29.f);\n}\nfloat linear_to_Lab_L(float t)\n{\n const float thr = 6.f/29.f, thr3 = thr*thr*thr;\n if (t > thr3)\n  t = cbrt(t);\n else\n  t = t * 841.f/108.f + 4.f/29.f;\n return 1.16f * t - 0.16f;\n}\nfloat4 gain_parabolic(float4 pv, float gain)\n{\n pv = min(pv, 1.f);\n pv = 1.f - pow((1.f-pv), gain);\n return pv;\n}\nfloat4 luma_compression(float4 pv0, float lvl_lin)"
"\n{\n float4 pv1;\n float grey0_lin, grey0_perc, grey1_perc, grey1_lin, white1_perc, white1_lin, lvl_perc, ratio;\n grey0_lin = 0.16f*pv0.s0 + 0.73f*pv0.s1 + 0.11f*pv0.s2;\n if (grey0_lin==0.f)\n  return pv0;\n grey0_perc = linear_to_Lab_L(grey0_lin);\n lvl_perc = linear_to_Lab_L(lvl_lin);\n grey1_perc = grey0_perc + lvl_perc;\n grey1_lin = Lab_L_to_linear(grey1_perc) - lvl_lin;\n white1_perc = 1.f + lvl_perc;\n white1_lin = Lab_L_to_linear(white1_perc) - lvl_lin;\n ratio = (grey1_lin / grey0_lin) / white1_lin;\n pv1 = pv0 * ratio;\n return pv1;\n}\nfloat4 colour_matrix(global float *le, float4 pv)\n{\n float4 v;\n v.x = le[0]*pv.x + le[3]*pv.y + le[6]*pv.z;\n v.y = le[1]*pv.x + le[4]*pv.y + le[7]*pv.z;\n v.z = le[2]*pv.x + le[5]*pv.y + le[8]*pv.z;\n v.w = pv.w;\n return v;\n}\nfloat hue_to_channel(float oh)\n{\n float t;\n oh -= 3.f*floor((oh+1.f) * (1.f/3.f));\n t = fabs(clamp(oh, -1.f, 1.f));\n if (t <= 0.5f)\n  return 1.f;\n else\n  return Lab_L_to_linear(2.f * (1.f - t));\n}\nfloat3 hsl_to_rgb_cw(float3 w, float3 hsl)\n{\n float3 rgb, rgbw;\n float Y;\n rgb.x = hue_to_channel(hsl.x);\n rgb.y = hue_to_channel(hsl.x-1.f);\n rgb.z = hue_to_channel(hsl.x-2.f);\n rgbw = rgb * w;\n Y = rgbw.x + rgbw.y + rgbw.z;\n Y = hsl.z / Y;\n rgb *= Y;\n rgb = mix(hsl.z, rgb, hsl.y);\n return rgb;\n}\nint idiv_ceil(int a, int b)\n{\n int d = a / b;\n if (d*b < a)\n  d += 1;\n return d;\n}\nulong get_bits_in_stream(global uchar *stream, ulong start_bit, uint bit_count)\n{\n ulong r=0, b, start_byte, actual_start_bit;\n int bits_to_read, b_sh;\n uchar mask;\n if (bit_count==0)\n  return 0;\n start_byte = start_bit >> 3;\n start_bit &= 7;\n bits_to_read = min((int) (8-start_bit), (int) bit_count);\n b_sh = max((int) 0, (int) (bit_count - bits_to_read));\n while (bit_count > 0)\n {\n  bits_to_read = min((int) (8-start_bit), (int) bit_count);\n  actual_start_bit = 8-start_bit - bits_to_read;\n  mask = (((1<<bits_to_read)-1) << actual_start_bit);\n  b = (stream[start_byte] & mask) >> actual_start_bit;\n  r |= b << b_sh;\n  b_sh = max((int) 0, (int) (b_sh-8));"
"\n  bit_count -= bits_to_read;\n  start_bit = 0;\n  start_byte++;\n }\n return r;\n}\nulong get_bits_in_stream_inc(global uchar *stream, ulong *start_bit, uint bit_count)\n{\n ulong r = get_bits_in_stream(stream, *start_bit, bit_count);\n *start_bit += bit_count;\n return r;\n}\nbool check_image_bounds(int2 pi, int2 im_dim)\n{\n if (pi.x >= 0 && pi.x < im_dim.x)\n  if (pi.y >= 0 && pi.y < im_dim.y)\n   return true;\n return false;\n}\nfloat4 read_frgb_pixel(global float4 *im, int index)\n{\n return im[index];\n}\nfloat4 read_float1_pixel(global float *im, int index)\n{\n float4 pv = 0.f;\n pv.x = im[index];\n pv.w = 1.f;\n return pv;\n}\nfloat4 read_float2_pixel(global float *im, int index)\n{\n float4 pv = 0.f;\n index <<= 1;\n pv.x = im[index];\n pv.y = im[index+1];\n pv.w = 1.f;\n return pv;\n}\nfloat4 read_float3_pixel(global float *im, int index)\n{\n float4 pv;\n index *= 3;\n pv.x = im[index];\n pv.y = im[index+1];\n pv.z = im[index+2];\n pv.w = 1.f;\n return pv;\n}\nfloat4 read_sqrgb_pixel(global uint *im, int index)\n{\n float4 pv;\n uint4 pvi;\n uint v;\n const float mul_rb = 1.f / (1023.f*1023.f);\n const float mul_g = 1.f / (4092.f*4092.f);\n const float4 mul_pvi = (float4) (mul_rb, mul_g, mul_rb, 1.f);\n v = im[index];\n pvi.z = v >> 22;\n pvi.y = (v >> 10) & 4095;\n pvi.x = v & 1023;\n pvi.w = 1;\n pv = convert_float4(pvi*pvi) * mul_pvi;\n return pv;\n}\nfloat4 read_srgb_pixel(global uint *im, int index)\n{\n float4 pv;\n uint v;\n v = im[index];\n pv.z = s8lrgb((v >> 16) & 255);\n pv.y = s8lrgb((v >> 8) & 255);\n pv.x = s8lrgb(v & 255);\n pv.w = 1.f;\n return pv;\n}\nfloat4 read_lrgb_pixel(global ushort *im, int index)\n{\n float4 pv;\n pv.x = im[index] * 0.000030517578125f;\n pv.y = im[index+1] * 0.000030517578125f;\n pv.z = im[index+2] * 0.000030517578125f;\n pv.w = 1.f;\n return pv;\n}\nfloat4 raw_yuv_to_lrgb(float3 raw, float depth_mul)\n{\n float y, u, v, r, g, b;\n float4 pv;\n raw *= depth_mul;\n y = raw.x - 16.f;\n y *= 255.f / 219.f;\n u = raw.y - 128.f;\n v = raw.z - 128.f;\n r = y + 1.596f * v;\n        g = y - 0.813f * v - 0.391f * u"
";\n        b = y + 2.018f * u;\n pv.x = s8lrgb(r);\n pv.y = s8lrgb(g);\n pv.z = s8lrgb(b);\n pv.w = 1.f;\n return pv;\n}\nfloat4 read_yuv420p8_pixel(global uchar *im, int2 im_dim, int2 i)\n{\n float4 pv;\n float y, u, v, r, g, b;\n int2 im_dimh = im_dim / 2;\n int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;\n global uchar *u_plane, *v_plane;\n u_plane = &im[size_full];\n v_plane = &im[size_full + size_half];\n y_index = i.y * im_dim.x + i.x;\n uv_index = i.y/2 * im_dimh.x + i.x/2;\n pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), 1.f );\n return pv;\n}\nfloat4 read_yuv420pN_pixel(global ushort *im, int2 im_dim, int2 i, float depth_mul)\n{\n float4 pv;\n float y, u, v, r, g, b;\n int2 im_dimh = im_dim / 2;\n int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;\n global ushort *u_plane, *v_plane;\n u_plane = &im[size_full];\n v_plane = &im[size_full + size_half];\n y_index = i.y * im_dim.x + i.x;\n uv_index = i.y/2 * im_dimh.x + i.x/2;\n pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), depth_mul );\n return pv;\n}\nuint bits_to_mask(uint bits)\n{\n return (1UL << bits) - 1;\n}\nfloat bits_to_mul(uint bits)\n{\n return convert_float((int) ((1ULL << bits) - 1));\n}\nfloat4 decompr_rgb(int bits_col, uchar3 c)\n{\n float4 rgb;\n float ratio = 1.f / bits_to_mul(bits_col);\n rgb.x = (float) c.x * ratio;\n rgb.y = (float) c.y * ratio;\n rgb.z = (float) c.z * ratio;\n return rgb;\n}\nfloat4 compr_hsl_to_rgb(float3 hsl)\n{\n float4 rgb;\n float3 w = (float3) (0.124f, 0.686f, 0.19f);\n hsl.x *= 3.f;\n hsl.z = Lab_L_to_linear(hsl.z);\n rgb = (float4) (hsl_to_rgb_cw(w, hsl), 1.f);\n rgb.x = linear_to_Lab_L(rgb.x);\n rgb.y = linear_to_Lab_L(rgb.y);\n rgb.z = linear_to_Lab_L(rgb.z);\n return rgb;\n}\ntypedef struct\n{\n int init;\n int block_size, bits_per_block, quincunx, bits_col, bits_per_pixel;\n int linew0, linew1, pix;\n int2 block_pos, block_start;\n ulong di;\n float4 col0, col1, colm, pv;\n float pix_mul;\n} comp_decode"
"_t;\nfloat4 read_compressed_texture1_pixel(global uchar *d8, int2 im_dim, int2 i, comp_decode_t *d)\n{\n global ushort *d16 = (global ushort *) d8;\n ulong di, block_start_bit = 64;\n int line_count0, line_count1;\n int pix, qoff;\n uchar3 c0, c1, cm;\n int2 block_pos, ib;\n float t;\n if (d->init==0)\n {\n  d->block_size = 8;\n  d->bits_per_block = 240;\n  d->quincunx = 1;\n  d->bits_col = 8;\n  d->bits_per_pixel = 3;\n  d->linew0 = idiv_ceil(im_dim.x, d->block_size);\n  d->linew1 = idiv_ceil(im_dim.x + d->quincunx*(d->block_size>>1), d->block_size);\n  d->pix_mul = 2.f / bits_to_mul(d->bits_per_pixel);\n  d->init = 1;\n  d->block_pos = (int2) (-1, -1);\n  d->pix = -1;\n }\n block_pos.y = (i.y / d->block_size);\n qoff = (block_pos.y&1) * d->quincunx * (d->block_size>>1);\n block_pos.x = (i.x + qoff) / d->block_size;\n if (block_pos.x != d->block_pos.x || block_pos.y != d->block_pos.y)\n {\n  line_count0 = (block_pos.y+1) >> 1;\n  line_count1 = block_pos.y >> 1;\n  di = line_count0*d->linew0 + line_count1*d->linew1;\n  di += block_pos.x;\n  di = di*d->bits_per_block + block_start_bit;\n  d->block_start = block_pos * d->block_size;\n  d->block_start.x -= qoff;\n  c0.x = get_bits_in_stream_inc(d8, &di, d->bits_col);\n  c0.y = get_bits_in_stream_inc(d8, &di, d->bits_col);\n  c0.z = get_bits_in_stream_inc(d8, &di, d->bits_col);\n  c1.x = get_bits_in_stream_inc(d8, &di, d->bits_col);\n  c1.y = get_bits_in_stream_inc(d8, &di, d->bits_col);\n  c1.z = get_bits_in_stream_inc(d8, &di, d->bits_col);\n  d->di = di;\n  d->col0 = decompr_rgb(d->bits_col, c0);\n  d->col1 = decompr_rgb(d->bits_col, c1);\n  d->block_pos = block_pos;\n  d->pix = -1;\n }\n ib = i - d->block_start;\n di = d->di + (ib.y*d->block_size + ib.x) * d->bits_per_pixel;\n pix = get_bits_in_stream(d8, di, d->bits_per_pixel);\n if (pix != d->pix)\n {\n  t = convert_float(pix) * d->pix_mul;\n  d->pv = mix(d->col0, d->col1, t*0.5f);\n  d->pv.x = Lab_L_to_linear(d->pv.x);\n  d->pv.y = Lab_L_to_linear(d->pv.y);\n  d->pv.z = Lab_L_to_linear(d->pv.z);\n  d->pix = pix;\n }\n ret"
"urn d->pv;\n}\nfloat4 read_fmt_pixel(const int fmt, global float4 *im, int2 im_dim, int2 i, comp_decode_t *cd1)\n{\n switch (fmt)\n {\n  case 0:\n   return read_frgb_pixel((global float4 *) im, i.y * im_dim.x + i.x);\n  case 1:\n   return read_sqrgb_pixel((global uint *) im, i.y * im_dim.x + i.x);\n  case 2:\n   return read_srgb_pixel((global uint *) im, i.y * im_dim.x + i.x);\n  case 3:\n   return read_lrgb_pixel((global ushort *) im, 4*(i.y * im_dim.x + i.x));\n  case 10:\n   return read_yuv420p8_pixel((global uchar *) im, im_dim, i);\n  case 11:\n   return read_yuv420pN_pixel((global ushort *) im, im_dim, i, 0.25f);\n  case 12:\n   return read_yuv420pN_pixel((global ushort *) im, im_dim, i, 0.0625f);\n  case 20:\n   return read_compressed_texture1_pixel((global uchar *) im, im_dim, i, cd1);\n  case 31:\n   return read_float1_pixel((global float *) im, i.y * im_dim.x + i.x);\n  case 32:\n   return read_float2_pixel((global float *) im, i.y * im_dim.x + i.x);\n  case 33:\n   return read_float3_pixel((global float *) im, i.y * im_dim.x + i.x);\n }\n return 0.f;\n}\nfloat4 read_fmt_pixel_checked(global float4 *im, int2 im_dim, const int fmt, int2 pi, comp_decode_t *cd1)\n{\n float4 pv;\n if (check_image_bounds(pi, im_dim))\n  pv = read_fmt_pixel(fmt, im, im_dim, pi, cd1);\n else\n  pv = 0.f;\n return pv;\n}\nfloat calc_flattop_weight(float2 pif, float2 i, float2 knee, float2 slope, float2 pscale)\n{\n float2 d, w;\n d = fabs(pif - i);\n d = max(d, knee);\n w = slope * (d - pscale);\n return w.x * w.y;\n}\nfloat4 image_filter_flattop(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale, float2 slope)\n{\n float4 pv = 0.f;\n float2 knee, i, start, end;\n comp_decode_t cd1={0};\n knee = 0.5f - fabs(fmod(pscale, 1.f) - 0.5f);\n start = max(0.f, ceil(pif - pscale));\n end = min(convert_float2(im_dim - 1), floor(pif + pscale));\n for (i.y = start.y; i.y <= end.y; i.y+=1.f)\n  for (i.x = start.x; i.x <= end.x; i.x+=1.f)\n   pv += read_fmt_pixel(fmt, im, im_dim, convert_int2(i), &cd1) * calc_flattop_weight(pif, "
"i, knee, slope, pscale);\n return pv;\n}\nfloat2 calc_aa_nearest_weights(float2 pif, float2 i, float2 pscale)\n{\n float2 d, w;\n d = fabs(pif - i);\n w = clamp(native_divide(0.5f - d, pscale) + 0.5f, 0.f, 1.f);\n return w;\n}\nfloat4 image_filter_aa_nearest(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale)\n{\n float4 pv = 0.f;\n comp_decode_t cd1={0};\n float2 pif00, w00;\n int2 pi00;\n float w;\n pif00 = floor(pif);\n pi00 = convert_int2(pif00);\n w00 = calc_aa_nearest_weights(pif, pif00, pscale);\n w = w00.x * w00.y;\n pv = read_fmt_pixel_checked(im, im_dim, fmt, pi00, &cd1) * w;\n if (w < 1.f)\n {\n  pv += read_fmt_pixel_checked(im, im_dim, fmt, pi00 + (int2)(0, 1), &cd1) * w00.x * (1.f - w00.y);\n  pv += read_fmt_pixel_checked(im, im_dim, fmt, pi00 + (int2)(1, 0), &cd1) * (1.f - w00.x) * w00.y;\n  pv += read_fmt_pixel_checked(im, im_dim, fmt, pi00 + (int2)(1, 1), &cd1) * (1.f - w00.x) * (1.f - w00.y);\n }\n return pv;\n}\nfloat4 blit_sprite_flattop(global uint *lei, global uchar *data_cl, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n global float *lef = (global float *) lei;\n global float4 *im;\n int2 im_dim;\n int fmt;\n float2 pscale, pos, pif, slope;\n im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];\n im_dim.x = lei[2];\n im_dim.y = lei[3];\n pscale.x = lef[4];\n pscale.y = lef[5];\n pos.x = lef[6];\n pos.y = lef[7];\n fmt = lei[8];\n slope.x = lef[9];\n slope.y = lef[10];\n pif = pscale * (pf + pos);\n pscale = max(1.f, pscale);\n pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);\n return pv;\n}\nfloat4 blit_sprite_flattop_rot(global uint *lei, global uchar *data_cl, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n global float *lef = (global float *) lei;\n global float4 *im;\n int2 im_dim;\n int fmt;\n float2 pscale, pos, pif, pifo, slope;\n float costh, sinth;\n im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];\n im_dim.x = lei[2];"
"\n im_dim.y = lei[3];\n pscale.x = lef[4];\n pscale.y = pscale.x;\n pos.x = lef[5];\n pos.y = lef[6];\n fmt = lei[7];\n slope.x = lef[8];\n slope.y = slope.x;\n costh = lef[9];\n sinth = lef[10];\n pifo = pscale * (pf + pos);\n pif.x = pifo.x * costh - pifo.y * sinth;\n pif.y = pifo.x * sinth + pifo.y * costh;\n pscale = max(1.f, pscale);\n pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);\n return pv;\n}\nfloat4 blit_sprite_aa_nearest(global uint *lei, global uchar *data_cl, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n global float *lef = (global float *) lei;\n global float4 *im;\n int2 im_dim;\n int fmt;\n float2 pscale, pos, pif;\n im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];\n im_dim.x = lei[2];\n im_dim.y = lei[3];\n pscale.x = lef[4];\n pscale.y = lef[5];\n pos.x = lef[6];\n pos.y = lef[7];\n fmt = lei[8];\n pif = pscale * (pf + pos);\n pv += image_filter_aa_nearest(im, im_dim, fmt, pif, pscale);\n return pv;\n}\nfloat4 blit_sprite_aa_nearest_rot(global uint *lei, global uchar *data_cl, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n global float *lef = (global float *) lei;\n global float4 *im;\n int2 im_dim;\n int fmt;\n float2 pscale, pos, pif, pifo;\n float costh, sinth;\n im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];\n im_dim.x = lei[2];\n im_dim.y = lei[3];\n pscale.x = lef[4];\n pscale.y = pscale.x;\n pos.x = lef[5];\n pos.y = lef[6];\n fmt = lei[7];\n costh = lef[8];\n sinth = lef[9];\n pifo = pscale * (pf + pos);\n pif.x = pifo.x * costh - pifo.y * sinth;\n pif.y = pifo.x * sinth + pifo.y * costh;\n pv += image_filter_aa_nearest(im, im_dim, fmt, pif, pscale);\n return pv;\n}\nfloat4 drawgradienttest(float4 pv)\n{\n const float2 pf = (float2) (get_global_id(0), get_global_id(1));\n const float2 ss = (float2) (get_global_size(0), get_global_size(1));\n const float2 c = ss * 0.5f;\n pv = -(pf.x - c.x) / (ss.x * 0.1f);\n return pv;\n}\nfloat4 draw_queue(gl"
"obal float *df, global int *poslist, global int *entrylist, global uchar *data_cl, const int sector_w, const int sector_size)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const int sec = (p.y >> sector_size) * sector_w + (p.x >> sector_size);\n global int *di = (global int *) df;\n int i, eli, entry_count, qi;\n float4 pv = 0.f;\n int brlvl = 0;\n float4 br[4];\n eli = poslist[sec];\n if (eli < 0)\n  return pv;\n entry_count = entrylist[eli];\n for (i=0; i < entry_count; i++)\n {\n  qi = entrylist[eli + i + 1];\n  switch (di[qi])\n  {\n   case DQT_BRACKET_OPEN:\n    br[brlvl] = pv;\n    pv = 0.f;\n    brlvl++;\n    break;\n   case DQT_BRACKET_CLOSE:\n    brlvl--;\n    pv = blend_pixel(br[brlvl], pv, di[qi+1]);\n    break;\n    case DQT_LINE_THIN_ADD: pv = draw_line_thin_add(&df[qi+1], pv);\n   break; case DQT_POINT_ADD: pv = draw_point_add(&df[qi+1], pv);\n   break; case DQT_RECT_FULL: pv = draw_rect_full_add(&df[qi+1], pv);\n   break; case DQT_RECT_BLACK: pv = draw_black_rect(&df[qi+1], pv);\n   break; case DQT_RECT_BLACK_INV: pv = draw_black_rect_inv(&df[qi+1], pv);\n   break; case DQT_PLAIN_FILL: pv = draw_plain_fill_add(&df[qi+1], pv);\n   break; case DQT_GAIN: pv = pv * df[qi+1];\n   break; case DQT_GAIN_PARAB: pv = gain_parabolic(pv, df[qi+1]);\n   break; case DQT_LUMA_COMPRESS: pv = luma_compression(pv, df[qi+1]);\n   break; case DQT_COL_MATRIX: pv = colour_matrix(&df[qi+1], pv);\n   break; case DQT_CLIP: pv = min(pv, df[qi+1]);\n   break; case DQT_CLAMP: pv = clamp(pv, 0.f, 1.f);\n   break; case DQT_CIRCLE_FULL: pv = draw_circle_full_add(&df[qi+1], pv);\n   break; case DQT_CIRCLE_HOLLOW: pv = draw_circle_hollow_add(&df[qi+1], pv);\n   break; case DQT_BLIT_FLATTOP: pv = blit_sprite_flattop((global uint *) &di[qi+1], data_cl, pv);\n   break; case DQT_BLIT_FLATTOP_ROT: pv = blit_sprite_flattop_rot((global uint *) &di[qi+1], data_cl, pv);\n   break; case DQT_BLIT_AANEAREST: pv = blit_sprite_aa_nearest((global uint *) &di[qi+1], data_cl, pv);\n   break; case DQT_BLIT_AANEAREST_ROT: pv"
" = blit_sprite_aa_nearest_rot((global uint *) &di[qi+1], data_cl, pv);\n   break; case DQT_TEST1: pv = drawgradienttest(pv);\n   default:\n    break;\n  }\n }\n return pv;\n}\nkernel __attribute__((reqd_work_group_size(16,16,1))) void draw_queue_srgb_kernel(const ulong df_index, const ulong poslist_index, const ulong entrylist_index, global uchar *data_cl, write_only image2d_t srgb, const int sector_w, const int sector_size, const int randseed)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const int fbi = p.y * get_global_size(0) + p.x;\n float4 pv;\n global float *df = (global float *) &data_cl[df_index];\n global int *poslist = (global int *) &data_cl[poslist_index];\n global int *entrylist = (global int *) &data_cl[entrylist_index];\n pv = draw_queue(df, poslist, entrylist, data_cl, sector_w, sector_size);\n if (pv.s0==0.f)\n if (pv.s1==0.f)\n if (pv.s2==0.f)\n {\n  write_imageui(srgb, p, 0);\n  return ;\n }\n write_imagef(srgb, p, linear_to_srgb(pv, randseed+fbi));\n}\nkernel void draw_queue_srgb_buf_kernel(const ulong df_index, const ulong poslist_index, const ulong entrylist_index, global uchar *data_cl, global uchar4 *srgb, const int sector_w, const int sector_size, const int randseed)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const int fbi = p.y * get_global_size(0) + p.x;\n float4 pv;\n uchar4 ps;\n global float *df = (global float *) &data_cl[df_index];\n global int *poslist = (global int *) &data_cl[poslist_index];\n global int *entrylist = (global int *) &data_cl[entrylist_index];\n pv = draw_queue(df, poslist, entrylist, data_cl, sector_w, sector_size);\n if (pv.s0==0.f)\n if (pv.s1==0.f)\n if (pv.s2==0.f)\n {\n  srgb[fbi] = 0;\n  return ;\n }\n ps = convert_uchar4(linear_to_srgb(pv, randseed+fbi) * 255.f);\n srgb[fbi].x = ps.z;\n srgb[fbi].y = ps.y;\n srgb[fbi].z = ps.x;\n}\n";