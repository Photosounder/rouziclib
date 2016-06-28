typedef struct
{
	uint32_t codepoint;
	vobj_t *obj;
	double bl, br;	// bounds to the left and right
	double width;
	char *glyphdata;
	int point_count, line_count;
} letter_t;

typedef struct
{
	letter_t *l;
	int letter_count, alloc_count;
	int16_t *codepoint_letter_lut;
} vector_font_t;

// TODO put that into font struct
#define LETTERSPACING	1.5	// spacing between each letter
#define LINEVSPACING	10.	// offset for each line
#define LOWERCASESCALE	0.75

#include "make_font.h"
#include "draw.h"
#include "stats.h"
#include "fit.h"

extern letter_t *get_letter(vector_font_t *font, uint32_t c);
extern letter_t *get_uppercase_letter(vector_font_t *font, uint32_t c);
