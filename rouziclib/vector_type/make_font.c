void font_alloc_one(vector_font_t *font)
{
	alloc_more(&font->l, 1, &font->letter_count, &font->alloc_count, sizeof(letter_t), 2.);
}

void add_codepoint_letter_lut_reference(vector_font_t *font)
{
	font->codepoint_letter_lut[font->l[font->letter_count-1].codepoint] = font->letter_count-1;	// add a LUT reference
}

void font_parse_p_line(char *line, xy_t *pv, int *pid, letter_t *l)
{
	int n, ip;
	char *p;

	p = skip_whitespace(line);
	if (p[0]=='p')
	{
		sscanf(p, "p%d %n", &ip, &n);	// take the p number

		p = &p[n];
		p = string_parse_fractional_12(p, &pv[l->point_count].x);
		p = string_parse_fractional_12(p, &pv[l->point_count].y);
		pid[ip + l->pid_offset] = l->point_count;
		l->point_count++;

		l->max_pid = MAXN(l->max_pid, ip + l->pid_offset);
	}
}

void font_parse_curveseg_line(char *line, xy_t *pv, int *pid, letter_t *l)
{
	int n, ip, d1_is_ratio=0, abs_angle=0;
	char *p;
	double th0=0.5*pi, th1=0., d0, d1=0.;
	xy_t p0=XY0, p1=XY0, p2;

	p = skip_string(line, " curveseg %n");
	sscanf(p, "p%d %n", &ip, &n);			// take the p number
	p = &p[n];

	p = string_parse_fractional_12(p, &th1);	// angle (in dozenal twelfths of a turn (0;0;0 to 11;11;11)) of the segment from the previous segment
	if (p[0]=='a')					// if a 'a' follows the angle is considered absolute, only p1 is used
	{
		abs_angle = 1;
		p++;
	}

	th1 += 3.;	// fixes a 90 degree bend
	th1 *= -2.*pi / 12.;				// makes positive angles go clockwise
	p = string_parse_fractional_12(p, &d1);		// distance ratio of the segment compared to the previous one
	if (p[0]=='x')					// if a 'x' follows it's considered a ratio
		d1_is_ratio = 1;

	if (d1==0. || l->point_count < 1)
		return ;

	p1 = pv[l->point_count - 1];
	if (l->point_count - 2 >= 0)
		p0 = pv[l->point_count - 2];
	if (abs_angle==0)
		th0 = atan2(p1.y-p0.y, p1.x-p0.x);		// angle of the previous curve segment

	d0 = hypot_xy(p0, p1);
	if (abs_angle==0 && d1_is_ratio==0)
		d0 = 1.;

	if (d1_is_ratio)
		d1 *= d0;

	p2 = rotate_xy2(xy(0., d1), th0+th1);
	p2 = add_xy(p1, p2);

	pv[l->point_count] = p2;
	pid[ip + l->pid_offset] = l->point_count;
	l->point_count++;

	l->max_pid = MAXN(l->max_pid, ip + l->pid_offset);
}

void font_parse_rect_line(char *line, xy_t *pv, int *pid, letter_t *l)
{
	int i, ip, n=0, pstart=0;
	double num_seg=0., radius=0., start_angle=0.;
	xy_t centre=XY0, pn;
	rect_t r;
	char *p=line;

	sscanf(p, " rect p%d %n", &pstart, &n);
	p = &p[n];

	p = string_parse_fractional_12(p, &r.p0.x);
	p = string_parse_fractional_12(p, &r.p0.y);
	p = string_parse_fractional_12(p, &r.p1.x);
	p = string_parse_fractional_12(p, &r.p1.y);

	for (i=0; i<4; i++)
	{
		ip = pstart + i;
		pn.x = (i&1) ? r.p1.x : r.p0.x;
		pn.y = (i&2) ? r.p1.y : r.p0.y;

		pv[l->point_count] = pn;
		pid[ip + l->pid_offset] = l->point_count;
		l->point_count++;

		l->max_pid = MAXN(l->max_pid, ip + l->pid_offset);
	}
}

void font_parse_circle_line(char *line, xy_t *pv, int *pid, letter_t *l)
{
	int ip, n=0, pstart=0, pend=0;
	double num_seg=0., radius=0., start_angle=0.;
	xy_t centre=XY0, pn;
	char *p=line;

	sscanf(p, " circle p%d p%d %lf %n", &pstart, &pend, &num_seg, &n);
	p = &p[n];

	p = string_parse_fractional_12(p, &radius);
	p = string_parse_fractional_12(p, &centre.x);
	p = string_parse_fractional_12(p, &centre.y);
	p = string_parse_fractional_12(p, &start_angle);
	start_angle = (start_angle + 0.) * -2.*pi / 12.;	// makes positive angles go clockwise

	if (radius==0.)
		return ;

	radius /= cos(pi / num_seg);		// this makes the ideal circle fit entirely inside the polygon

	for (ip=pstart; ip<=pend; ip++)
	{
		pn = rotate_xy2(xy(0., radius), start_angle - (double) (ip-pstart) * 2.*pi / num_seg);
		pn = add_xy(pn, centre);

		pv[l->point_count] = pn;
		pid[ip + l->pid_offset] = l->point_count;
		l->point_count++;

		l->max_pid = MAXN(l->max_pid, ip + l->pid_offset);
	}
}

void font_parse_mirror_line(char *line, xy_t *pv, int *pid, letter_t *l)
{
	int ip, op, n=0, p_first=0, p_last=0, p_start=0;
	xy_t pn;
	double mirror_pos=0.;
	char hv, *p=line;

	sscanf(p, " mirror %c %n%*s p%d p%d p%d", &hv, &n, &p_first, &p_last, &p_start);
	p = &p[n];

	p = string_parse_fractional_12(p, &mirror_pos);

	for (op=p_first; op<=p_last; op++)
	{
		ip = p_start + op-p_first;
		pn = pv[pid[op + l->pid_offset]];

		if (hv=='h')
			pn.y = 2*mirror_pos - pn.y;
		else if (hv=='v')
			pn.x = 2*mirror_pos - pn.x;

		pv[l->point_count] = pn;
		pid[ip + l->pid_offset] = l->point_count;
		l->point_count++;

		l->max_pid = MAXN(l->max_pid, ip + l->pid_offset);
	}
}

void font_parse_lines_line(char *line, xy_t *pv, int *pid, int *lineA, int *lineB, letter_t *l)
{
	int n, pA=-1, pB;
	char *p;

	p = skip_string(line, " lines %n");

	while (sscanf(p, "p%d %n", &pB, &n) && strlen(p)>1)
	{
		p = &p[n];

		if (pB + l->pid_offset < 0)	// make sure the actual point referenced isn't < 0
			pB = l->pid_offset;

		if (pA!=-1)
		{
			lineA[l->line_count] = pA + l->pid_offset;
			lineB[l->line_count] = pB + l->pid_offset;
			l->line_count++;
		}

		pA = pB;
	}
}

void font_parse_bounds_line(char *line, letter_t *l)
{
	char *p;

	p = skip_string(line, " bounds %n");
	
	p = string_parse_fractional_12(p, &l->bl);
	p = string_parse_fractional_12(p, &l->br);
}

void font_parse_vbounds_line(char *line, letter_t *l)
{
	char *p;

	p = skip_string(line, " vbounds %n");
	
	p = string_parse_fractional_12(p, &l->bb);
	p = string_parse_fractional_12(p, &l->bt);
}

void font_parse_transform_line(char *line, char *a, letter_t *l, glyphdata_t *gd, int group_start, int last_start)
{
	int i, n, todo=0, to_offset=1, start;
	char *p, b[32];
	double th;
	xy_t o;
	xyz_t v;
	matrix_t m = matrix_xyz(xyz(1., 0., 0.),	// see https://en.wikipedia.org/wiki/File:2D_affine_transformation_matrix.svg
				xyz(0., 1., 0.),
				xyz(0., 0., 1.));

	p = skip_string(line, " %*s %n");

	start = group_start;				// default does the points added from the start of the glyph, not the parents
	if (sscanf(p, "%s %n", b, &n))		// detect if "all" or "loc" is added after the command
	{
		if (strcmp(b, "all")==0)		// all does all the points, even those of the parents (so, probably no point in using that)
		{
			start = 0;
			p = &p[n];
		}

		if (strcmp(b, "last")==0)		// last only does the points added since right before the last copy
		{
			start = last_start;
			p = &p[n];
		}

		if (strcmp(b, "loc")==0)		// all does all the points added locally since after the last copy
		{
			start = l->pid_offset;
			p = &p[n];
		}
	}

	if (strcmp(a, "scale")==0)
	{
		todo = 1;
		p = string_parse_fractional_12(p, &m.x.x);
		p = skip_whitespace(p);
		if (strlen(p) > 0)
			p = string_parse_fractional_12(p, &m.y.y);
		else
			m.y.y = m.x.x;
	}

	if (strcmp(a, "move")==0)
	{
		todo = 1;
		to_offset = 0;
		p = string_parse_fractional_12(p, &m.x.z);
		p = string_parse_fractional_12(p, &m.y.z);
	}

	if (strcmp(a, "rotate")==0)
	{
		todo = 1;
		p = string_parse_fractional_12(p, &th);			// angle, in dozenal twelfths of a turn (0;0;0 to 11;11;11)
		th *= 2.*pi / 12.;
		m.x.x = cos(th);
		m.x.y = sin(th);
		m.y.x = -m.x.y;
		m.y.y = m.x.x;
	}

	if (strcmp(a, "shearX")==0)
	{
		todo = 1;
		p = string_parse_fractional_12(p, &th);			// angle, in dozenal twelfths of a turn (0;0;0 to 11;11;11)
		th *= 2.*pi / 12.;
		m.x.y = tan(th);
	}

	if (strcmp(a, "shearY")==0)
	{
		todo = 1;
		p = string_parse_fractional_12(p, &th);			// angle, in dozenal twelfths of a turn (0;0;0 to 11;11;11)
		th *= 2.*pi / 12.;
		m.y.x = tan(th);
	}

	if (todo)
	{
		// get point to transform around
		if (to_offset)
		{
			p = string_parse_fractional_12(p, &o.x);		// origin X
			p = string_parse_fractional_12(p, &o.y);		// origin Y
			m.x.z = o.x - o.x * m.x.x - o.y * m.x.y;
			m.y.z = o.y - o.x * m.y.x - o.y * m.y.y;
		}

		// do transform to each point
		for (i=start; i<l->point_count; i++)
		{
			v = xyz(gd->pv[i].x, gd->pv[i].y, 1.);
			gd->pv[i] = xyz_to_xy( matrix_mul(v, m) );
		}
	}
}

void process_glyphdata_core(vector_font_t *font, letter_t *l, char *p, glyphdata_t *gd, int req_subgl)
{
	int n, group_start, last_start, subgl=0;
	char c, line[128], a[128];
	uint32_t copy_cp, copy_subgl;
	letter_t *lcopy;

	if (p==NULL)
		return ;

	group_start = l->pid_offset;
	last_start = l->max_pid;

	while (sscanf(p, "%[^\n]\n%n", line, &n) && strlen(p)>0)	// go through each line in glyphdata
	{
		sscanf(line, " %s", a);

		if (strcmp(a, "subglyph")==0)				// entering subglyph section
			if (sscanf(line, " subglyph %c", &c)==1)
				subgl = c;

		if (strcmp(a, "subend")==0)
			subgl = 0;

		if (subgl==req_subgl)
		{
			font_parse_p_line(line, gd->pv, gd->pid, l);		// processes pX lines

			if (strcmp(a, "curveseg")==0)
				font_parse_curveseg_line(line, gd->pv, gd->pid, l);

			if (strcmp(a, "circle")==0)
				font_parse_circle_line(line, gd->pv, gd->pid, l);

			if (strcmp(a, "rect")==0)
				font_parse_rect_line(line, gd->pv, gd->pid, l);

			if (strcmp(a, "mirror")==0)
				font_parse_mirror_line(line, gd->pv, gd->pid, l);

			if (strcmp(a, "lines")==0)
				font_parse_lines_line(line, gd->pv, gd->pid, gd->lineA, gd->lineB, l);

			if (strcmp(a, "bounds")==0)
			{
				font_parse_bounds_line(line, l);
				gd->set_bounds = 1;
			}

			if (strcmp(a, "vbounds")==0)
			{
				font_parse_vbounds_line(line, l);
				gd->set_vbounds = 1;
			}

			if (strcmp(a, "copy")==0)
			{
				last_start = l->max_pid;

				copy_cp = 0;
				copy_subgl = 0;
				if (sscanf(line, " copy '%c'", &c)==1)
					copy_cp = c;
				else
					sscanf(line, " copy %X", &copy_cp);

				if (sscanf(line, " copy %*s %c", &c)==1)
					copy_subgl = c;

				lcopy = get_letter(font, copy_cp);
				l->pid_offset = l->max_pid;
				if (lcopy)
					process_glyphdata_core(font, l, lcopy->glyphdata, gd, copy_subgl);
				l->pid_offset = l->max_pid;
			}

			font_parse_transform_line(line, a, l, gd, group_start, last_start);
		}

		p = &p[n];
	}

	//if (l->codepoint==0xFE94)
	//	fprintf_rl(stdout, "group_start = %d\n", group_start);
		//fprintf_rl(stdout, "%d, %d, %d, %d\n", l->point_count, l->line_count, l->pid_offset, l->max_pid);
}

void process_glyphdata(vector_font_t *font, letter_t *l, glyphdata_t *gd)
{
	if (l==NULL)
		return ;

	if (l->glyphdata == NULL)
		return ;

	l->pid_offset = 0;
	l->max_pid = 0;
	l->point_count = 0;
	l->line_count = 0;
	gd->set_bounds = 0;
	gd->set_vbounds = 0;
	memset(gd->pid, 0xFF, sizeof(gd->pid));
	memset(gd->pv, 0, sizeof(gd->pv));
	memset(gd->lineA, 0, sizeof(gd->lineA));
	memset(gd->lineB, 0, sizeof(gd->lineB));

	process_glyphdata_core(font, l, l->glyphdata, gd, 0);
}

void make_glyph_vobj(letter_t *l, glyphdata_t *gd)
{
	int i, pidA, pidB;
	xy_t vA, vB;

	l->bb = 10.;
	l->bt = 0.;

	// make to vector object from the parsed data
	free_vobj(l->obj);
	l->obj = alloc_vobj(l->line_count);
	for (i=0; i < l->line_count; i++)
	{
		pidA = gd->pid[gd->lineA[i]];
		pidB = gd->pid[gd->lineB[i]];
		if (pidA > -1 && pidB > -1)
		{
			vA = gd->pv[pidA];
			vB = gd->pv[pidB];
			l->obj->seg[i] = seg_make_xy(vA, vB, 1.);

			if (gd->set_bounds==0)
			{
				l->bl = MINN(l->bl, vA.x);
				l->bl = MINN(l->bl, vB.x);
				l->br = MAXN(l->br, vA.x);
				l->br = MAXN(l->br, vB.x);
			}

			if (gd->set_vbounds==0)
			{
				l->bb = MINN(l->bb, vA.y);
				l->bb = MINN(l->bb, vB.y);
				l->bt = MAXN(l->bt, vA.y);
				l->bt = MAXN(l->bt, vB.y);
			}
		}
	}
	l->width = l->br - l->bl;
}

void font_block_process_line(char *line, vector_font_t *font)
{
	char a[128], c, *p;
	uint32_t cp;

	sscanf(line, "%s", a);

	if (strcmp(a, "glyph")==0)
	{
		cp = 0;
		if(sscanf(line, "glyph '%c'", &c)==1)
			cp = c;
		else
			sscanf(line, "glyph %X", &cp);

		// realloc previous glyphdata
		if (font->letter_count > 0)
			if (font->l[font->letter_count-1].glyphdata)
			{
				font->l[font->letter_count-1].gd_alloc = strlen(font->l[font->letter_count-1].glyphdata) + 1;
				font->l[font->letter_count-1].glyphdata = realloc(font->l[font->letter_count-1].glyphdata, font->l[font->letter_count-1].gd_alloc);
			}

		font_alloc_one(font);
		font->l[font->letter_count-1].codepoint = cp;
		font->l[font->letter_count-1].gd_alloc = 100;
		font->l[font->letter_count-1].glyphdata = calloc(font->l[font->letter_count-1].gd_alloc, sizeof(char));

		add_codepoint_letter_lut_reference(font);
	}
	else if (strlen(line) > 1)
	{
		while (strlen(line) + strlen(font->l[font->letter_count-1].glyphdata) + 1 >= font->l[font->letter_count-1].gd_alloc)
		{
			font->l[font->letter_count-1].gd_alloc *= 2;
			font->l[font->letter_count-1].glyphdata = realloc(font->l[font->letter_count-1].glyphdata, font->l[font->letter_count-1].gd_alloc);
		}

		strcat(font->l[font->letter_count-1].glyphdata, line);			// puts the line into the letter's glyphdata
	}
}

void make_fallback_font(vector_font_t *font)
{
	#include "fallback_font.h"		// contains const char *fallback_font[]

	int i;

	for (i=0; fallback_font[i]; i++)	// loop ends when reaching the last element (NULL)
		font_block_process_line(fallback_font[i], font);
}

void make_font_block(char *path, vector_font_t *font)
{
	FILE *file;
	char line[128];

	file = fopen_utf8(path, "rb");
	if (file == NULL)
		return ;

	while (fgets(line, sizeof(line), file))
		font_block_process_line(line, font);

	fclose (file);
}

void make_font_aliases(char *path, vector_font_t *font)
{
	int ret;
	uint32_t cp, tgt;
	FILE *file;
	char line[128], a[128], c, *p;

	file = fopen_utf8(path, "rb");
	if (file == NULL)
		return ;
	
	while (fgets(line, sizeof(line), file))
	{
		ret = sscanf(line, "%X = %s", &cp, a);

		if (ret==2)
		{
			font_alloc_one(font);
			font->l[font->letter_count-1].codepoint = cp;
			if (a[0]=='\'')
				if (sscanf(a, "'%c'", &c)==1)
					font->l[font->letter_count-1].alias = c;
			else
				sscanf(a, "%X", &font->l[font->letter_count-1].alias);

			add_codepoint_letter_lut_reference(font);
		}
	}

	fclose (file);
}

void process_one_glyph(vector_font_t *font, int i)
{
	glyphdata_t gd;

	if (i < 0 || i >= font->letter_count)
		return ;

	if (font->l[i].alias || font->l[i].obj)
		return ;

	if (font->l[i].glyphdata)
	{
		process_glyphdata(font, &font->l[i], &gd);
		make_glyph_vobj(&font->l[i], &gd);
	}
	else
		make_cjkdec_vobj(font, &font->l[i]);
}

vector_font_t *make_font(char *index_path)
{
	vector_font_t *font;
	FILE *indexfile;
	char *dirpath, *p, line[128], a[128], path[256];
	int i, range0, range1;

	font = calloc(1, sizeof(vector_font_t));

	font->letter_count = 0;
	font->alloc_count = 256;
	font->l = calloc (font->alloc_count, sizeof(letter_t));
	font->codepoint_letter_lut = calloc (0x110000, sizeof(int32_t));
	memset(font->codepoint_letter_lut, 0xFF, 0x110000 * sizeof(int32_t));

	dirpath = remove_after_char_copy(index_path, '/');

	indexfile = fopen_utf8(index_path, "rb");
	if (indexfile == NULL)
	{
		make_fallback_font(font);
		free(dirpath);
		return font;
	}

	while (fgets(line, sizeof(line), indexfile))			// read the index file
	{
		sscanf(line, "%s", a);

		if (strcmp(a, "range")==0)
		{
			strcpy(path, dirpath);
			sscanf(line, "range %X %X \"%[^\"]\"", &range0, &range1, &path[strlen(path)]);
			make_font_block(path, font);
		}

		if (strcmp(a, "substitutions")==0)
		{
			strcpy(path, dirpath);
			sscanf(line, "substitutions \"%[^\"]\"", &path[strlen(path)]);
			make_font_aliases(path, font);
		}

		if (strcmp(a, "cjkdecomp")==0)
		{
			strcpy(path, dirpath);
			sscanf(line, "cjkdecomp \"%[^\"]\"", &path[strlen(path)]);
			cjkdec_load_data(path, font);
		}
	}

	//for (i=0; i < font->letter_count; i++)
	//	process_one_glyph(font, i);

	fclose(indexfile);
	free(dirpath);

	return font;
}

void free_font(vector_font_t *font)
{
	int i;

	if (font==NULL)
		return ;

	for (i=0; i<font->letter_count; i++)
	{
		free_vobj (font->l[i].obj);
		free(font->l[i].glyphdata);
	}

	free(font->codepoint_letter_lut);
	free(font->l);
	free(font->cjkdec_pos);
	free(font->cjkdec_data);
	free(font);
}

vector_font_t *remake_font(char *index_path, vector_font_t *oldfont)
{
	static char *old_path=NULL;

	if (index_path)		// copy the path for possible later use
	{
		free(old_path);
		old_path = calloc (strlen(index_path)+1, sizeof(char));

		strcpy(old_path, index_path);
	}

	free_font(oldfont);

	return make_font(old_path);
}
