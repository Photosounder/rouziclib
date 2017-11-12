enum
{
	DQ_END = 0,			// points to after the last entered number in the buffer

	// sector_list specific
	DQ_ENTRY_START = 1,		// points to the start of the entry
	DQ_END_HEADER_SL,		// the end of the header, where the first entry should start
};

enum	// entry types
{
	DQT_NOTYPE,
	DQT_BRACKET_OPEN,
	DQT_BRACKET_CLOSE,
	DQT_LINE_THIN_ADD,
	DQT_POINT_ADD,
	DQT_RECT_FULL,
	DQT_RECT_BLACK,
	DQT_PLAIN_FILL,
	DQT_CIRCLE_FULL,
	DQT_BLIT_SPRITE,
	DQT_BLIT_PHOTO,
	DQT_TEST1,
};

enum	// blending modes
{
	DQB_ADD,
	DQB_SUB,
	DQB_MUL,
	DQB_DIV,
	DQB_BLEND,
	DQB_SOLID,
};
