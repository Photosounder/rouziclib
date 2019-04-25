typedef struct
{
	uint8_t r, g, b, a;
} srgb_t;			// sRGB

typedef struct
{
	uint32_t r:10;
	uint32_t g:12;
	uint32_t b:10;
} sqrgb_t;			// squared RGB, the stored values are proportional to the square root of the linear value

typedef struct
{
	uint16_t r, g, b, a;	// in 1.LBD format (as it goes up to a fixed-point value of 1.0)
} lrgb_t;			// linear RGB format

typedef struct
{
	float r, g, b, a;
} frgb_t;			// linear RGB format

#ifdef COL_FRGB
	#define col_t 			frgb_t
#else
	#define col_t			lrgb_t
#endif

typedef struct
{
	xyi_t dim;		// formerly [wh]
	lrgb_t *l;
	frgb_t *f;
	srgb_t *srgb;
	sqrgb_t *sq;
	uint8_t *buf;		// generic pointer for other image formats like planar YUV
	size_t buf_size;
	int buf_fmt;		// format of the buffer, 10 for YUV 8-bit, 11 for YUV 10-bit, 12 for YUV 12-bit
	int use_frgb;
	int as;			// alloc size in pixels
	// TODO consider a flag to indicate that the host-side raster has been updated since the last copy to the device to trigger a new copy
	
	#ifdef RL_OPENCL
	void *referencing_fb;		// used to access the framebuffer's device alloc table if *f is referenced there for removal when freeing the raster
	#endif
	int table_index;		// index in the cl_data allocation table
} raster_t;

typedef struct
{
	raster_t *r;				// an array of raster tiles indexed by [y*tilecount.x + x]
	xyi_t fulldim, tiledim, tilecount;	// full size in actual pixels, max size in actual pixels of a tile for this level (used for position calculations), number of tiles
	xy_t scale;
	size_t total_bytes;
} mipmap_level_t;

typedef struct
{
	mipmap_level_t *lvl;
	int lvl_count;
	xyi_t fulldim;
	size_t total_bytes;
} mipmap_t;

typedef struct
{
	size_t start, end;	// position on fb's data_cl device buffer
	int unused;		// 0 means in use, 1 means was used last frame, 2 means unused and should be removed
	void *host_ptr;
} cl_data_alloc_t;

typedef struct
{
	int index;		// sought table index
	int next_index;		// index of the next entry in the overflow table for hash collisions
} hash_table_elem_t;

typedef struct
{
	hash_table_elem_t *elem, *overflow;
	int overflow_as, overflow_first_avail;
} hash_table_t;

typedef struct
{
	raster_t r;
	int32_t w, h;
	rect_t window_dl;	// window draw limit (based on the usual drawing thickness)
	xyi_t maxdim;		// formerly max[wh]
	int use_cl;
	
	#ifdef RL_SDL
	void *window;
	void *renderer;
	void *texture;
	int fullscreen_on;
	recti_t wind_rect;
	#endif

	#ifdef RL_OPENCL
	cl_mem cl_srgb;		// device memory which is the same as the OpenGL texture
	uint32_t gltex;		// ID of the GL texture for cl_srgb
	clctx_t clctx;		// contains the context and the command queue

	// Draw queue data
	int32_t *drawq_data;		// main queue where the full data for each entry is stored, with type ID as the first element and the number of following elements depending on the type
	int32_t *sector_pos;		// for each sector: the position in entry_list
	int32_t *entry_list;		// for each sector: the list of entries, the first element is the number of entries for the sector, positions in the main queue are what follow
	int32_t *sector_list;		// for each entry index: the list of sectors, the first element is the number of sectors for the entry, sector IDs follow
	int32_t *sector_count;		// for each sector: the count of entries
	int32_t *pending_bracket;	// for each sector: the number of pending open brackets, between 0 and 3
	int32_t *entry_pos;		// for each entry: the list of start positions in sector_list, so that sector_list may be randomly accessed by entry ID
	cl_mem drawq_data_cl, sector_pos_cl, entry_list_cl;

	int drawq_size;		// number of floats/ints in the queue
	int list_alloc_size;	// allocation size of entry and sector lists
	int max_sector_count;
	int entry_list_end;	// end (size) of entry_list
	int sectors;		// number of subdivisions (and separate drawing queues) on the screen
	int sector_size;	// size of the sectors in powers of two. sector_size==6 means 64x64 sized sectors
	int sector_w;		// number of sectors per row (for instance rows of 30 64x64 sectors for 1920x1080)
	int *entry_count;	// number of entries in the main queue

	// CL data (for images and what not)
	cl_mem data_cl;				// device buffer that contains all the needed data
	size_t data_cl_as;			// alloc size of data_cl in bytes
	cl_data_alloc_t *data_alloc_table;	// table that lists allocations within the buffer
	int data_alloc_table_count;
	int data_alloc_table_as;		// alloc size of the data_alloc_table in elements
	hash_table_t hash_table;
	#endif
} framebuffer_t;
