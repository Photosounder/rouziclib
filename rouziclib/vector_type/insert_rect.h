enum		// inserting-spacing codepoints (must match font in ../vector_type/private_use.txt)
{
	cp_ins_start=0xE010,
	cp_ins_w0=cp_ins_start,
	cp_ins_w1,
	cp_ins_w2,
	cp_ins_w3,
	cp_ins_w4,
	cp_ins_w5,
	cp_ins_w6,
	cp_ins_w7,
	cp_ins_w8,
	cp_ins_w9,
	cp_ins_w10,
	cp_ins_w11,
	cp_ins_w12,

	cp_ins_nul=0xE020,

	cp_ins_end,

	cp_ins_index_base=0xE0100	// variation selectors, represents index 0
};

extern void reset_insert_rect_array();
extern void report_insert_rect_pos(xy_t pos, xy_t dim, int bidi, uint32_t cp, int index);
extern rect_t get_insert_rect_zc(zoom_t zc, int index);
extern rect_t insert_rect_change_height(rect_t r, double low, double high);

#define get_insert_rect(index)	get_insert_rect_zc(zc, index)
