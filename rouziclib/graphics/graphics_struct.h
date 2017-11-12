typedef struct
{
#ifdef SWITCH_RGB_CHANNELS
	uint32_t b:8;
	uint32_t g:8;
	uint32_t r:8;
	uint32_t a:8;
#else
	uint32_t r:8;
	uint32_t g:8;
	uint32_t b:8;
	uint32_t a:8;
#endif
} srgb_t;			// sRGB

typedef struct
{
	uint16_t r, g, b, a;	// in 1.LBD format (as it goes up to a fixed-point value of 1.0)
} lrgb_t;			// linear RGB format

typedef struct
{
	float r, g, b, a;
} frgb_t;			// linear RGB format

typedef struct
{
	int32_t w, h, maxw, maxh;
	rect_t window_dl;
	lrgb_t *l;
	frgb_t *f;
	srgb_t *srgb;
	int use_frgb;
	int use_cl;

	// threading
	volatile int status;	// readiness status, see enum below
	//mtx_t 
	
	#ifdef RL_SDL
	void *window;
	void *renderer;
	void *texture;
	#endif

	#ifdef RL_OPENCL
	cl_mem clbuf, cl_srgb;
	uint64_t clbuf_da;	// device address for clbuf (actually currently an offset from fb's device data buffer)
	uint32_t gltex;		// ID of the GL texture for cl_srgb
	clctx_t clctx;		// contains the context and the command queue

	// Draw queue data
	int32_t *drawq_data, *sector_pos, *entry_list, *sector_list, *sector_count;
	cl_mem drawq_data_cl, sector_pos_cl, entry_list_cl;

	int drawq_size;		// number of floats/ints in the queue
	int list_alloc_size;	// allocation size of entry and sector lists
	int max_sector_count;
	int entry_list_end;	// end (size) of entry_list
	int sectors;		// number of subdivisions (and separate drawing queues) on the screen
	int sector_size;	// size of the sectors in powers of two. sector_size==6 means 64x64 sized sectors
	int sector_w;		// number of sectors per row (for instance rows of 30 64x64 sectors for 1920x1080)

	// CL data (for images and what not)
	cl_mem data_cl;			// device buffer that contains all the needed data
	int64_t data_cl_as;		// alloc size of data_cl in bytes
	xyi_t *data_alloc_table;	// table that lists allocations within the buffer (.x=start, .y=end)
	int data_alloc_table_as;	// alloc size of the data_alloc_table in elements
	#endif
} raster_t;

enum
{
	raster_status_null,		// when there's no image and none being prepared
	raster_status_prep,		// when there's an image being prepared (loading/processing)
	raster_status_ready		// when the image is ready to be used
};
