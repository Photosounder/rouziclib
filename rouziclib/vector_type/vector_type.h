// in vector_type/vector_type_struct.h:
// letter_t, vector_font_t

#define ALIG_LEFT	0
#define ALIG_CENTRE	1
#define ALIG_RIGHT	2
#define PROPORTIONAL	0
#define MONOSPACE	4
#define MONODIGITS	8

// TODO put that into font struct
#define LETTERSPACING	1.5	// spacing between each letter
#define LINEVSPACING	10.	// offset for each line
#define LOWERCASESCALE	0.75

#include "make_font.h"
#include "draw.h"
#include "stats.h"
#include "fit.h"
#include "cjk.h"
#include "insert_rect.h"

extern int get_letter_index(vector_font_t *font, uint32_t c);
extern letter_t *get_letter(vector_font_t *font, uint32_t c);
extern vobj_t *get_letter_obj(vector_font_t *font, uint32_t c);
extern char *get_letter_glyphdata(vector_font_t *font, uint32_t c);
extern letter_t *get_dominant_letter(vector_font_t *font, uint32_t c, int *lowerscale);
extern uint32_t substitute_rtl_punctuation(uint32_t c);
