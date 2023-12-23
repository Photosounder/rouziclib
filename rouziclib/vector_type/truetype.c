#ifdef RL_TRUETYPE

#undef pi
#define optimize tt_optimize
#define FAILED TT_FAILED
#define TTF_NO_FILESYSTEM
#include "../libraries/orig/ttf2mesh.c"
#undef optimize
#undef FAILED
#undef pi
#define pi RL_PI

vector_font_t *load_truetype_font_as_triangles(const uint8_t *data, size_t size, xyi_t *range, int range_count, int quality)
{
	// Load TTF file
	ttf_t *ttf;
	if (ttf_load_from_mem(data, size, &ttf, 0) != TTF_DONE)
		return NULL;

	vector_font_t *font = init_font();

	// Calculate the scale and typographic parameters
	double scale = 6. / (double) ttf->os2.sTypoAscender;
	font->letter_spacing = 0.;
	font->line_vspacing = scale * (ttf->os2.sTypoLineGap + ttf->os2.sTypoAscender - ttf->os2.sTypoDescender);

	for (int ic=0; ic < ttf->nchars; ic++)
	for (int ir=0; ir < range_count; ir++)
	{
		uint32_t cp = ttf->chars[ic];
		if (cp >= range[ir].x && cp <= range[ir].y)
		{
			ttf_glyph_t *g = &ttf->glyphs[ttf->char2glyph[ic]];

			// Alloc new letter in the vector font
			font_create_letter(font, cp);
			letter_t *l = &font->l[font->letter_count-1];

			// Set bounds
			l->bl = 0.;
			l->br = scale * g->advance;
			l->width = l->br - l->bl;
			l->tri_mesh.count = -1;

			// Turn the outline into a mesh of triangles
			if (g->outline && g->outline->total_points >= 3)
			{
				ttf_mesh_t *m;
				ttf_glyph2mesh(g, &m, quality, 0);

				if (m)
				{
					l->tri_mesh.count = m->nfaces;
					l->tri_mesh.tri = calloc(l->tri_mesh.count, sizeof(triangle_t));

					// Set the triangles
					for (int it=0; it < m->nfaces; it++)
						l->tri_mesh.tri[it] = mul_triangle(triangle(
								xy(m->vert[m->faces[it].v1].x, m->vert[m->faces[it].v1].y), 
								xy(m->vert[m->faces[it].v2].x, m->vert[m->faces[it].v2].y), 
								xy(m->vert[m->faces[it].v3].x, m->vert[m->faces[it].v3].y)), set_xy(scale));

					ttf_free_mesh(m);
				}
			}
		}
	}

        ttf_free(ttf);

	return font;
}

#endif
