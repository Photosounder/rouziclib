void font_alloc_one(vector_font_t *font)
{
	int newsize;

	font->letter_count++;

	if (font->letter_count >= font->alloc_count)
	{
		newsize = font->alloc_count * 2;

		font->l = realloc(font->l, newsize * sizeof(letter_t));
		memset(&font->l[font->alloc_count], 0, (newsize-font->alloc_count) * sizeof(letter_t));

		font->alloc_count = newsize;

		if (font->l==NULL)
			fprintf_rl(stderr, "realloc failed in font_alloc_one() for font->alloc_count = %d\n", font->alloc_count);
	}
}

void font_parse_p_line(char *line, xy_t *pv, int *pid, letter_t *l)
{
	int ret, n, ip;
	char *p;

	p = skip_whitespace(line);
	if (p[0]=='p')
	{
		ret = sscanf(p, "p%d %n", &ip, &n);	// take the p number

		p = &p[n];
		p = string_parse_fractional_12(p, &pv[l->point_count].x);
		p = string_parse_fractional_12(p, &pv[l->point_count].y);
		pid[ip] = l->point_count;
		l->point_count++;
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

		if (pA!=-1)
		{
			lineA[l->line_count] = pA;
			lineB[l->line_count] = pB;
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

void process_glyphdata(letter_t *l)
{
	int ret, i, n, set_bounds=0;
	char line[128], a[128], *p;
	int pid[100], lineA[100], lineB[100];
	xy_t pv[100], vA, vB;

	l->point_count = 0;
	l->line_count = 0;
	memset(pv, 0, sizeof(pv)/sizeof(*pv));
	memset(pid, 0xFF, sizeof(pid)/sizeof(*pid));

	p = l->glyphdata;
	while (sscanf(p, "%[^\n]\n%n", line, &n) && strlen(p)>0)	// go through each line in glyphdata
	{
		ret = sscanf(line, " %s", a);

		font_parse_p_line(line, pv, pid, l);			// processes pX lines

		if (strcmp(a, "lines")==0)
			font_parse_lines_line(line, pv, pid, lineA, lineB, l);

		if (strcmp(a, "bounds")==0)
		{
			font_parse_bounds_line(line, l);
			set_bounds = 1;
		}

		p = &p[n];
	}

	// make to vector object from the parsed data
	if (l->obj)
		free_vobj(l->obj);
	l->obj = alloc_vobj(l->line_count);
	for (i=0; i < l->line_count; i++)
	{
		vA = pv[pid[lineA[i]]];
		vB = pv[pid[lineB[i]]];
		l->obj->seg[i] = seg_make_xy(vA, vB, 1.);

		if (set_bounds==0)
		{
			l->bl = MINN(l->bl, vA.x);
			l->bl = MINN(l->bl, vB.x);
			l->br = MAXN(l->br, vA.x);
			l->br = MAXN(l->br, vB.x);
		}
	}
	l->width = l->br - l->bl;
}

void make_font_block(char *path, vector_font_t *font)
{
	int ret;
	FILE *file;
	char line[128], a[128], *p;

	file = fopen_utf8(path, "rb");
	
	while (fgets(line, 128, file))
	{
		ret = sscanf(line, "%s", a);

		if (strcmp(a, "glyph")==0)
		{
			if (font->letter_count-1 >= 0)
				process_glyphdata(&font->l[font->letter_count-1]);	// process glyphdata for previous letter

			font_alloc_one(font);
			font->l[font->letter_count-1].glyphdata = calloc(400, sizeof(char));

			ret = sscanf(line, "glyph '%c'", &font->l[font->letter_count-1].codepoint);
			if (ret==0)
				sscanf(line, "glyph %X", &font->l[font->letter_count-1].codepoint);

			font->codepoint_letter_lut[font->l[font->letter_count-1].codepoint] = font->letter_count-1;	// add a LUT reference
		}
		else if (strlen(line) > 1)
			strcat(font->l[font->letter_count-1].glyphdata, line);		// puts the line into the letter's glyphdata
	}
	if (font->letter_count-1 >= 0)
		process_glyphdata(&font->l[font->letter_count-1]);	// process glyphdata for last letter

	fclose (file);
}

vector_font_t *make_font(char *index_path)
{
	vector_font_t *font;
	FILE *indexfile;
	char *dirpath, *p, line[128], a[128], path[256];
	int ret, range0, range1;

	font = calloc(1, sizeof(vector_font_t));

	font->letter_count = 0;
	font->alloc_count = 256;
	font->l = calloc (font->alloc_count, sizeof(letter_t));
	font->codepoint_letter_lut = calloc (0x110000, sizeof(int16_t));
	memset(font->codepoint_letter_lut, 0xFF, 0x110000 * sizeof(int16_t));

	dirpath = remove_after_char_copy(index_path, '/');

	indexfile = fopen_utf8(index_path, "rb");
	
	while (fgets(line, 128, indexfile))			// read the index file
	{
		ret = sscanf(line, "%s", a);

		if (strcmp(a, "range")==0)
		{
			strcpy(path, dirpath);
			sscanf(line, "range %X %X \"\%[^\"]\"", &range0, &range1, &path[strlen(path)]);
			make_font_block(path, font);
		}
	}

	fclose (indexfile);
	free (dirpath);

	return font;
}
