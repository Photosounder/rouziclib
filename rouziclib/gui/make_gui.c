// must match the enum in make_gui.h
const char *layout_elem_type_name[] =
{
	"(null)",
	"none",
	"label",
	"button",
	"checkbox",
	"knob",
	"textedit",
};

void free_layout_elem(layout_elem_t *elem)
{
	free(elem->label);

	if (elem->data)
	{
		switch (elem->type)
		{
			case gui_type_knob:
				free(((knob_t *) elem->data)->fmt_str);
				break;

			case gui_type_textedit:
				textedit_free((textedit_t *) elem->data);
				break;
		}

		free(elem->data);
		elem->data = NULL;
	}

	memset(elem, 0, sizeof(layout_elem_t));
}

void free_gui_layout(gui_layout_t *layout)
{
	int i;

	for (i=0; i < layout->elem_as; i++)
		free_layout_elem(&layout->elem[i]);

	memset(layout, 0, sizeof(gui_layout_t));
}

void make_gui_layout(gui_layout_t *layout, const char **src, const int linecount)
{
	int i, il, n, cur_elem_id=0, vint;
	char *line, a[128], b[32], *p;
	layout_elem_t *cur_elem=NULL;
	knob_t *knob_data;
	textedit_t *te;

	if (layout->init)
		return ;

	layout->init = 1;

	for (il=0; il < linecount; il++)
	{
		line = src[il];
		a[0] = '\0';
		n = 0;
		sscanf(line, "%s %n", a, &n);
		p = &line[n];

		if (strcmp(a, "elem")==0)		// create new element
		{
			if (sscanf(line, "elem %d", &cur_elem_id)==1)
			{
				if (cur_elem_id < 0 || cur_elem_id > 1000000)
				{
					fprintf_rl(stderr, "ID %d is out of the valid [0 , 1000000] range in line %d: \"%s\"\n", cur_elem_id, il, line);
					return ;
				}
				else
				{
					alloc_enough(&layout->elem, cur_elem_id+1, &layout->elem_as, sizeof(layout_elem_t), 1.5);

					cur_elem = &layout->elem[cur_elem_id];

					if (cur_elem->type != 0)		// if an element was already there
						free_layout_elem(cur_elem);	// remove it

					cur_elem->type = gui_type_none;
					cur_elem->colour = GUI_COL_DEF;
				}
			}
			else
			{
				fprintf_rl(stderr, "No valid element ID in line %d: \"%s\"\n", il, line);
				return ;
			}
		}

		if (strcmp(a, "type")==0)		// define the type of the element
		{
			for (i=0; i < gui_type_count; i++)
				if (strcmp(p, layout_elem_type_name[i])==0)
				{
					cur_elem->type = i;
					break;
				}

			if (cur_elem->type == gui_type_null)
				fprintf_rl(stderr, "No valid element type in line %d: \"%s\"\n", il, line);

			// initialise the element with some defaults
			switch (cur_elem->type)
			{
				case gui_type_knob:
					cur_elem->data = calloc(1, sizeof(knob_t));
					knob_data = (knob_t *) cur_elem->data;
					knob_data->min = 0.f;
					knob_data->default_value = 0.f;
					knob_data->max = 1.f;
					knob_data->func = knobf_linear;
					knob_data->main_label = cur_elem->label;
					break;

				case gui_type_textedit:
					cur_elem->data = calloc(1, sizeof(textedit_t));
					te = (textedit_t *) cur_elem->data;
					break;
			}
		}

		if (strcmp(a, "label")==0)		// set the label string
		{
			alloc_enough(&cur_elem->label, strlen(p)+1, &cur_elem->label_as, sizeof(char), 1.);
			strcpy(cur_elem->label, p);
			cur_elem->label_set = 1;

			switch (cur_elem->type)
			{
				case gui_type_knob:
					if (cur_elem->data)
						((knob_t *) cur_elem->data)->main_label = cur_elem->label;
					break;

				case gui_type_textedit:
					if (cur_elem->data)
						*((textedit_t *) cur_elem->data) = string_to_textedit(make_string_copy(cur_elem->label));
					break;
			}
		}

		if (strcmp(a, "label_alloc")==0)	// set the minimum allocation size of the label
		{
			vint = 0;
			if (sscanf(line, "label_alloc %d", &vint)==1)
			{
				alloc_enough(&cur_elem->label, vint+1, &cur_elem->label_as, sizeof(char), 1.);
				cur_elem->label_as_set = 1;
			}
			else
				fprintf_rl(stderr, "No valid label allocation size in line %d: \"%s\"\n", il, line);
		}

		// define the area rect
		if (strcmp(a, "pos")==0)
		{
			p = string_parse_fractional_12(p, &cur_elem->pos.x);
			p = string_parse_fractional_12(p, &cur_elem->pos.y);
		}

		if (strcmp(a, "dim")==0)
		{
			p = string_parse_fractional_12(p, &cur_elem->dim.x);
			if (p[0])
				p = string_parse_fractional_12(p, &cur_elem->dim.y);
			else
				cur_elem->dim.y = cur_elem->dim.x;
		}

		if (strcmp(a, "off")==0)
		{
			p = string_parse_fractional_12(p, &cur_elem->pos_off.x);
			if (p[0])
				p = string_parse_fractional_12(p, &cur_elem->pos_off.y);
			else
				cur_elem->pos_off.y = cur_elem->pos_off.x;
		}

		if (strcmp(a, "pos")==0 || strcmp(a, "dim")==0 || strcmp(a, "off")==0)		// update the area rect
			cur_elem->area = make_rect_off(cur_elem->pos, cur_elem->dim, cur_elem->pos_off);

		if (strcmp(a, "knob")==0)	// sets the knob data structure
		{
			if (cur_elem->data==NULL)
			{
				fprintf_rl(stderr, "Knob element data should have been initialised by a 'type knob' command before line %d: \"%s\"\n", il, line);
				return ;
			}

			knob_data = (knob_t *) cur_elem->data;

			n = 0;
			if (sscanf(line, "knob %lg %lg %lg %s %n", &knob_data->min, &knob_data->default_value, &knob_data->max, b, &n)==4)
			{
				knob_data->func = knob_func_name_to_ptr(b);
				if (knob_data->func==NULL)
				{
					fprintf_rl(stderr, "The knob function name '%s' isn't valid in line %d: \"%s\"\n", b, il, line);
					return ;
				}

				if (knob_data->min >= knob_data->max)
				{
					fprintf_rl(stderr, "The maximum value of a knob cannot be lower than the minimum in line %d: \"%s\"\n", il, line);
					return ;
				}

				if (knob_data->min > knob_data->default_value || knob_data->default_value > knob_data->max)
				{
					fprintf_rl(stderr, "The default value of a knob cannot be outside of the [min , max] range in line %d: \"%s\"\n", il, line);
					return ;
				}

				if (knob_data->func==knobf_log && MINN(knob_data->min, knob_data->default_value) <= 0.)
				{
					fprintf_rl(stderr, "When the knob function is logarithmic no value can be 0 or lower in line %d: \"%s\"\n", il, line);
					return ;
				}

				if (n==0)
					n = strlen(line);
				p = &line[n];
				knob_data->fmt_str = make_string_copy(p[0] ? p : VALFMT_DEFAULT);
				cur_elem->fmt_str_set = p[0]!='\0';
			}
			else
			{
				fprintf_rl(stderr, "The 'knob' command needs at least the first 4 arguments: '<min> <default> <max> <function (linear|log|recip)> (<format string>)' at line %d: \"%s\"\n", il, line);
				return ;
			}
		}
	}
}

void sprint_gui_layout(gui_layout_t *layout, char **str, int *str_as)
{
	int id;
	layout_elem_t *cur_elem=NULL;
	knob_t *knob_data;
	char a[32], b[32];

	sprintf_realloc(str, str_as, 0, "");	// clears/inits the string

	for (id=0; id < layout->elem_as; id++)
	{
		cur_elem = &layout->elem[id];

		if (cur_elem->type != gui_type_null)
		{
			sprintf_realloc(str, str_as, 1, "elem %d\n", id);
			if (cur_elem->type >= 0 && cur_elem->type < gui_type_count)
				sprintf_realloc(str, str_as, 1, "type %s\n", layout_elem_type_name[cur_elem->type]);
			if (cur_elem->label_set)
				sprintf_realloc(str, str_as, 1, "label %s\n", cur_elem->label);
			if (cur_elem->label_as_set)
				sprintf_realloc(str, str_as, 1, "label_alloc %d\n", cur_elem->label_as);

			if (cur_elem->type == gui_type_knob && cur_elem->data)
			{
				knob_data = (knob_t *) cur_elem->data;

				sprintf_realloc(str, str_as, 1, "knob %g %g %g %s", knob_data->min, knob_data->default_value, knob_data->max, knob_func_ptr_to_name(knob_data->func));
				if (cur_elem->fmt_str_set && knob_data->fmt_str)
					sprintf_realloc(str, str_as, 1, " %s", knob_data->fmt_str);
				sprintf_realloc(str, str_as, 1, "\n");
			}

			sprintf_realloc(str, str_as, 1, "pos\t%s\t%s\n", sprint_fractional_12(a, cur_elem->pos.x), sprint_fractional_12(b, cur_elem->pos.y));

			sprintf_realloc(str, str_as, 1, "dim\t%s", sprint_fractional_12(a, cur_elem->dim.x));
			if (fastabs64(double_diff_ulp(cur_elem->dim.x, cur_elem->dim.y)) > 1000000)
				sprintf_realloc(str, str_as, 1, "\t%s", sprint_fractional_12(a, cur_elem->dim.y));
			sprintf_realloc(str, str_as, 1, "\n");

			sprintf_realloc(str, str_as, 1, "off\t%s", sprint_fractional_12(a, cur_elem->pos_off.x));
			if (fastabs64(double_diff_ulp(cur_elem->pos_off.x, cur_elem->pos_off.y)) > 1000000)
				sprintf_realloc(str, str_as, 1, "\t%s", sprint_fractional_12(a, cur_elem->pos_off.y));
			sprintf_realloc(str, str_as, 1, "\n");

			sprintf_realloc(str, str_as, 1, "\n");
		}
	}
}

//**** Element functions ****

int check_layout_id_validity(gui_layout_t *layout, const int id)
{
	layout->elem[id].implemented = 0;

	if (id < 0 || id >= layout->elem_as)
	{
		fprintf_rl(stderr, "Error in check_layout_id_validity(): ID %d isn't valid (highest ID is %d)\n", id, layout->elem_as-1);
		return 0;
	}

	if (layout->elem[id].type <= gui_type_null || layout->elem[id].type >= gui_type_count)
	{
		fprintf_rl(stderr, "Error in check_layout_id_validity(): element at ID %d %s\n", id, layout->elem[id].type == gui_type_null ? "isn't initialised" : "has an invalid type");
		return 0;
	}

	layout->elem[id].implemented = 1;

	return 1;
}

// takes care of the resizing, returns 1 if ID is invalid
int ctrl_fromlayout_resizing_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		gui_layout_t *layout, const int id, const int phase)
{
	static ctrl_id_t hover_save={0};
	layout_elem_t *cur_elem=NULL;
	rect_t box_os;

	if (phase & 1)
	{
		if (check_layout_id_validity(layout, id)==0)	// if id isn't a valid layout element
			return 1;				// instruct caller to abort

		cur_elem = &layout->elem[id];

		if ((cur_elem->type == gui_type_knob || cur_elem->type == gui_type_textedit) && cur_elem->data == NULL)
			return 1;

		if (layout->edit_on)
		{
			box_os = offset_scale_rect(cur_elem->area, layout->offset, layout->sm);

			if (layout->sel_id == id)		// if this control is the selected one
			{					// use the resizing rect
				if (ctrl_resizing_rect(&cur_elem->resize_ctrl, &box_os))
				{
					cur_elem->area = offset_scale_inv_rect(box_os, layout->offset, layout->sm);
					rect_to_pos_dim(cur_elem->area, &cur_elem->pos, &cur_elem->dim, cur_elem->pos_off);
					cur_elem->area = make_rect_off(cur_elem->pos, cur_elem->dim, cur_elem->pos_off);	// this just makes pos/dim master
				}
			}
			else if (ctrl_button_invis(box_os))	// invisible selection button
				layout->sel_id = id;

			hover_save = mouse.ctrl_id->hover_new;		// suspends input processing for following controls
		}
	}

	if (phase & 2)
	{
		if (layout->edit_on)
			mouse.ctrl_id->hover_new = hover_save;
	}

	return 0;
}

void draw_label_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		gui_layout_t *layout, const int id, const int mode)
{
	int ret;
	layout_elem_t *cur_elem=NULL;

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return ;
	cur_elem = &layout->elem[id];

	draw_label(cur_elem->label, offset_scale_rect(cur_elem->area, layout->offset, layout->sm), cur_elem->colour, mode);

	ctrl_fromlayout_resizing(layout, id, 2);
}

/*void draw_rect_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		const int type, gui_layout_t *layout, const int id)
{
	int ret;
	layout_elem_t *cur_elem=NULL;

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return ;
	cur_elem = &layout->elem[id];

	rect_t box_sc = sc_rect( offset_scale_rect(cur_elem->area, layout->offset, layout->sm) );

	ret = ctrl_button_chamf(cur_elem->label, offset_scale_rect(cur_elem->area, layout->offset, layout->sm), cur_elem->colour);

	switch (type)
	{
		case 0:
			draw_rect(fb, box_sc, drawing_thickness, cur_elem->colour, blend_add, intensity);
			break;
		draw_rect_full(fb, box_sc, drawing_thickness, cur_elem->colour, intensity);
		draw_black_rect(fb, box_sc, drawing_thickness);

	ctrl_fromlayout_resizing(layout, id, 2);
}*/

int ctrl_button_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		gui_layout_t *layout, const int id)
{
	int ret;
	layout_elem_t *cur_elem=NULL;

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return 0;
	cur_elem = &layout->elem[id];

	ret = ctrl_button_chamf(cur_elem->label, offset_scale_rect(cur_elem->area, layout->offset, layout->sm), cur_elem->colour);

	ctrl_fromlayout_resizing(layout, id, 2);
	return ret;
}

int ctrl_checkbox_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		int8_t *state, gui_layout_t *layout, const int id)
{
	int ret;
	layout_elem_t *cur_elem=NULL;

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return 0;
	cur_elem = &layout->elem[id];

	ret = ctrl_checkbox(state, cur_elem->label, offset_scale_rect(cur_elem->area, layout->offset, layout->sm), cur_elem->colour);

	ctrl_fromlayout_resizing(layout, id, 2);
	return ret;
}

int ctrl_knob_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		double *v, gui_layout_t *layout, const int id)
{
	int ret;
	layout_elem_t *cur_elem=NULL;

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return 0;
	cur_elem = &layout->elem[id];

	ret = ctrl_knob(v, *((knob_t *) cur_elem->data), offset_scale_rect(cur_elem->area, layout->offset, layout->sm), cur_elem->colour);

	ctrl_fromlayout_resizing(layout, id, 2);
	return ret;
}

int ctrl_textedit_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		gui_layout_t *layout, const int id)
{
	int ret;
	layout_elem_t *cur_elem=NULL;

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return 0;
	cur_elem = &layout->elem[id];

	ret = ctrl_textedit((textedit_t *) cur_elem->data, offset_scale_rect(cur_elem->area, layout->offset, layout->sm), cur_elem->colour);

	ctrl_fromlayout_resizing(layout, id, 2);
	return ret;
}
/*	// Label
		draw_label_fromlayout(&layout, id, ALIG_CENTRE | MONODIGITS);

	// Button
		if (ctrl_button_fromlayout(&layout, id))

	// Checkbox
		static int8_t state=0;
		ctrl_checkbox_fromlayout(&state, &layout, id);

	// Knob
		static double value=NAN;
		ctrl_knob_fromlayout(&value, &layout, id);

	// Text editor
		static textedit_t te={0};
		ctrl_textedit_fromlayout(&te, &layout, id);*/

void gui_layout_unimplemented_elems_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness,
		gui_layout_t *layout)
{
	int id;

	if (layout->edit_on==0)
		return ;

	for (id=0; id < layout->elem_as; id++)
	{
		if (layout->elem[id].type && layout->elem[id].implemented==0)
		{
			switch (layout->elem[id].type)
			{
				case gui_type_none:
					break;

				case gui_type_label:
					draw_label_fromlayout(layout, id, ALIG_CENTRE | MONODIGITS);
					break;

				case gui_type_button:
					ctrl_button_fromlayout(layout, id);
					break;

				case gui_type_checkbox:
					ctrl_checkbox_fromlayout(NULL, layout, id);
					break;

				case gui_type_knob:
					ctrl_knob_fromlayout(NULL, layout, id);
					break;

				case gui_type_textedit:
					break;
			}

			layout->elem[id].implemented = 0;
		}
	}
}
