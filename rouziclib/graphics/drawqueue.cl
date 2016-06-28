// cp drawqueue.cl drawqueue.cl.c && gcc -E -P drawqueue.cl.c > dq.cl && ttc dq.cl && mv dq.cl.h drawqueue.cl.h && rm drawqueue.cl.c dq.cl

#include "drawqueue_enums.h"
#include "blending.cl"
#include "drawline.cl"
#include "blit.cl"
#include "srgb.cl"

float4 drawgradienttest(float4 pv)
{
	const float2 pf = (float2) (get_global_id(0), get_global_id(1));
	const float2 ss = (float2) (get_global_size(0), get_global_size(1));
	const float2 c = ss * 0.5f;

	pv = -(pf.x - c.x) / (ss.x * 0.1f);

	return pv;
}

kernel void draw_queue_kernel(global float *df, global int *poslist, global int *entrylist, write_only image2d_t srgb, const int sector_w, const int sector_size, const int randseed)
{
	const int2 p = (int2) (get_global_id(0), get_global_id(1));
	const int fbi = p.y * get_global_size(0) + p.x;
	const int sec = (p.y >> sector_size) * sector_w + (p.x >> sector_size);	// sector index
	global int *di = (global int *) df;	// main queue pointer
	int i, eli, entry_count, qi;
	float4 pv = 0.f;		// pixel value (linear)
	int brlvl = 0;			// bracket level
	float4 br[4];			// bracket pixels

	eli = poslist[sec];		// entry list index

	if (eli < 0)			// if the index is -1
		return ; 		// that means there's nothing to do

	entry_count = entrylist[eli];

	for (i=0; i<entry_count; i++)
	{
		qi = entrylist[eli + i + 1];	// queue index

		switch (di[qi])			// type of the entry
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

			case DQT_LINE_THIN_ADD:
				pv = draw_line_thin_add(&df[qi+1], pv);
				break;

			case DQT_POINT_ADD:
				pv = draw_point_add(&df[qi+1], pv);
				break;

			case DQT_BLIT_SPRITE:
				pv = blit_sprite(&df[qi+1], pv);
				break;

			case DQT_TEST1:
				pv = drawgradienttest(pv);
				break;
				
			default:
				break;
		}
	}

	if (pv.s0==0.f)
	if (pv.s1==0.f)
	if (pv.s2==0.f)
		return ;

	write_imagef(srgb, p, linear_to_srgb(pv, randseed+fbi));
}
