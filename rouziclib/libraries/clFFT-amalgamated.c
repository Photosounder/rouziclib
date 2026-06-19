/* I, Michel Rouzic, defiler of codebases, have asked ChatGPT 5.5
   to amalgamate clFFT into two files, drop anything I don't need
   and convert it all to C99, so that I can include it directly
   in rouziclib and not use dynamic libraries.
*/

/* ************************************************************************
 * Copyright 2013-2015 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/

#ifndef CLFFT_AMALGAMATED_BUFFER_T
  #define CLFFT_AMALGAMATED_BUFFER_T
  #define BUFFER_NPOS ((size_t) -1)

typedef struct buffer_t buffer_t;
static inline void bufreserve(buffer_t *s, size_t req);
static inline void bufappend(buffer_t *s, const char *src);
static inline void bufappendn(buffer_t *s, const char *src, size_t len);

struct buffer_t
{
	char *buf;
	size_t len;
	size_t as;
};

static inline void bufinit(buffer_t *s)
{
	// Initialize an owning buffer explicitly instead of relying on constructor
	// syntax.
	if (!s)
		return;
	s->buf = NULL;
	s->len = 0;
	s->as = 0;
}

static inline void buffree(buffer_t *s)
{
	// Release an owning buffer explicitly and leave it empty for safe reuse.
	if (!s)
		return;
	free(s->buf);
	s->buf = NULL;
	s->len = 0;
	s->as = 0;
}

static inline buffer_t buffer_empty(void)
{
	// Return an explicitly initialized empty owning buffer.
	buffer_t out;
	bufinit(&out);
	return out;
}

static inline buffer_t buffer_from_span(const char *src, size_t len)
{
	// Build an owning buffer from a counted byte span.
	buffer_t out;
	bufinit(&out);
	bufappendn(&out, src, len);
	return out;
}

static inline buffer_t buffer_from_cstr(const char *src)
{
	// Build an owning buffer from a NUL-terminated string.
	return buffer_from_span(src, src ? strlen(src) : 0);
}

static inline buffer_t buffer_copy(const buffer_t *src)
{
	// Build an owning buffer from another buffer's current contents.
	return buffer_from_span(src ? src->buf : NULL, src ? src->len : 0);
}

static inline const char *bufcstr(const buffer_t *s)
{
	// Return the buffer text or an empty string for NULL storage.
	return (s && s->buf) ? s->buf : "";
}

static inline size_t buffindc(const buffer_t *s, const char *needle, size_t pos)
{
	// Search for a C-string needle inside a buffer.
	if (!s || !needle || pos > s->len)
		return BUFFER_NPOS;
	const char *base = s->buf ? s->buf : "";
	const char *found = strstr(base + pos, needle);
	return found ? (size_t) (found - base) : BUFFER_NPOS;
}

static inline void bufreserve(buffer_t *s, size_t req)
{
	// Grow the buffer allocation while preserving existing contents.
	if (!s || req <= s->as)
		return;
	size_t new_as = s->as ? s->as : 32;
	while (new_as < req)
		new_as += new_as >> 1;
	char *new_buf = (char *) realloc(s->buf, new_as);
	if (!new_buf)
		abort();
	s->buf = new_buf;
	s->as = new_as;
	if (s->len == 0)
		s->buf[0] = '\0';
}

static inline void bufappendn(buffer_t *s, const char *src, size_t len)
{
	// Append a counted byte span and keep the buffer NUL terminated.
	if (!s || !src || len == 0)
		return;
	bufreserve(s, s->len + len + 1);
	memcpy(s->buf + s->len, src, len);
	s->len += len;
	s->buf[s->len] = '\0';
}

static inline void bufappend(buffer_t *s, const char *src)
{
	// Append a C string when one is provided.
	if (src)
		bufappendn(s, src, strlen(src));
}

static inline void bufclear(buffer_t *s)
{
	// Clear the buffer contents while keeping allocated storage.
	if (s)
	{
		s->len = 0;
		if (s->buf && s->as > 0)
			s->buf[0] = '\0';
	}
}

static inline int bufcmp(const buffer_t *a, const buffer_t *b)
{
	// Compare buffer contents as null-terminated strings.
	const char *ac = (a && a->buf) ? a->buf : "";
	const char *bc = (b && b->buf) ? b->buf : "";
	return strcmp(ac, bc);
}

static inline void bufprintf(buffer_t *s, const char *fmt, ...)
{
	if (!s || !fmt)
		return;
	va_list args;
	va_start(args, fmt);
	va_list copy;
	va_copy(copy, args);
	int added = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);
	if (added > 0)
	{
		bufreserve(s, s->len + (size_t) added + 1);
		vsnprintf(s->buf + s->len, s->as - s->len, fmt, args);
		s->len += (size_t) added;
	}
	va_end(args);
}

static inline void bufcatbuf(buffer_t *dst, const buffer_t *src)
{
	// Append another buffer's contents to the destination buffer.
	if (dst && src)
		bufappendn(dst, src->buf, src->len);
}

static inline void bufcatcstr(buffer_t *dst, const char *src)
{
	// Append a NUL-terminated string to the destination buffer.
	if (dst)
		bufappend(dst, src);
}

static inline void bufcatchar(buffer_t *dst, char src)
{
	// Append one byte to the destination buffer.
	if (dst)
		bufappendn(dst, &src, 1);
}

  #define BUFCAT_BUFFER_VALUE(dst_, expr_) \
		do \
		{ \
			buffer_t bufcat_tmp_ = (expr_); \
			bufcatbuf((dst_), &bufcat_tmp_); \
			buffree(&bufcat_tmp_); \
		} while (0)

static inline void bufsetbuf(buffer_t *dst, const buffer_t *src)
{
	// Copy a buffer into another buffer after clearing the destination.
	if (dst && src)
	{
		bufclear(dst);
		bufappendn(dst, src->buf, src->len);
	}
}

static inline void bufsetcstr(buffer_t *dst, const char *src)
{
	// Copy a NUL-terminated string into the destination buffer.
	if (dst)
	{
		bufclear(dst);
		bufappend(dst, src);
	}
}

static inline void bufset_linear_reg(buffer_t *dst, bool linearRegs, const buffer_t *regBaseCount)
{
	// Select the appropriate register prefix representation.
	if (linearRegs)
		bufsetcstr(dst, "(*R");
	else
		bufsetbuf(dst, regBaseCount);
}

typedef struct array_size_t
{
	size_t *buf;
	size_t len;
	size_t as;
} array_size_t;

typedef struct array_char
{
	char *buf;
	size_t len;
	size_t as;
} array_char;

typedef struct array_float
{
	float *buf;
	size_t len;
	size_t as;
} array_float;

typedef struct array_cl_mem
{
	cl_mem *buf;
	size_t len;
	size_t as;
} array_cl_mem;

typedef struct array_cl_device_id
{
	cl_device_id *buf;
	size_t len;
	size_t as;
} array_cl_device_id;

typedef struct array_size_t_array
{
	array_size_t *buf;
	size_t len;
	size_t as;
} array_size_t_array;

static inline void array_reserve_raw(void **buf, size_t *capacity, size_t req, size_t elem_size)
{
	// Grow raw array storage when the requested capacity exceeds the
	// allocation.
	if (!buf || !capacity || req <= *capacity)
		return;
	size_t new_as = *capacity ? *capacity : 4;
	while (new_as < req)
		new_as += new_as >> 1;
	void *new_buf = realloc(*buf, new_as * elem_size);
	if (!new_buf)
		abort();
	*buf = new_buf;
	*capacity = new_as;
}

static inline void array_resize_raw(void **buf, size_t *len, size_t *capacity, size_t count, size_t elem_size)
{
	// Resize raw storage while preserving the previous logical length.
	if (!buf || !len || !capacity)
		return;
	size_t old_len = *len;
	array_reserve_raw(buf, capacity, count, elem_size);
	*len = count;

	// Clear newly exposed elements for deterministic C-style initialization.
	if (count > old_len && *buf)
		memset((void *) ((char *) *buf + old_len * elem_size), 0, (count - old_len) * elem_size);
}

static inline void array_append_raw(void **buf, size_t *len, size_t *capacity, const void *src, size_t count, size_t elem_size)
{
	// Append a raw span after ensuring enough backing storage exists.
	if (!buf || !len || !capacity || !src || count == 0)
		return;
	array_reserve_raw(buf, capacity, *len + count, elem_size);
	memcpy((void *) ((char *) *buf + *len * elem_size), src, count * elem_size);
	*len += count;
}

static inline void array_erase_raw(void *buf, size_t *len, size_t elem_size, void *pos)
{
	// Shift following elements left over the erased raw slot.
	if (!buf || !len || !pos || elem_size == 0)
		return;
	char *base = (char *) buf;
	char *cursor = (char *) pos;
	size_t index = (size_t) ((cursor - base) / (ptrdiff_t) elem_size);
	if (index >= *len)
		return;
	if (index + 1 < *len)
		memmove((void *) (base + index * elem_size), (const void *) (base + (index + 1) * elem_size), (*len - index - 1) * elem_size);
	--*len;
}

  #define array_init(array_) \
		do \
		{ \
			if ((array_) != NULL) \
			{ \
				(array_)->buf = NULL; \
				(array_)->len = 0; \
				(array_)->as = 0; \
			} \
		} while (0)
  #define array_size(array_) ((array_) ? (array_)->len : 0)
  #define array_clear(array_) \
		do \
		{ \
			if ((array_) != NULL) \
			{ \
				(array_)->len = 0; \
			} \
		} while (0)
  #define array_free(array_) \
		do \
		{ \
			if ((array_) != NULL) \
			{ \
				free((array_)->buf); \
				array_init(array_); \
			} \
		} while (0)
  #define array_reserve(array_, req_) \
		do \
		{ \
			if ((array_) != NULL) \
				array_reserve_raw((void **) &((array_)->buf), &((array_)->as), (req_), sizeof(*(array_)->buf)); \
		} while (0)
  #define array_resize(array_, count_) \
		do \
		{ \
			if ((array_) != NULL) \
				array_resize_raw((void **) &((array_)->buf), &((array_)->len), &((array_)->as), (count_), sizeof(*(array_)->buf)); \
		} while (0)
  #define array_push_back(array_, value_) \
		do \
		{ \
			if ((array_) != NULL) \
			{ \
				array_reserve((array_), (array_)->len + 1); \
				(array_)->buf[(array_)->len] = (value_); \
				++(array_)->len; \
			} \
		} while (0)
  #define array_erase(array_, pos_) \
		do \
		{ \
			if ((array_) != NULL) \
				array_erase_raw((array_)->buf, &((array_)->len), sizeof(*(array_)->buf), (void *) (pos_)); \
		} while (0)
  #define array_append(array_, src_, count_) \
		do \
		{ \
			if ((array_) != NULL) \
				array_append_raw((void **) &((array_)->buf), &((array_)->len), &((array_)->as), (const void *) (src_), (count_), sizeof(*(array_)->buf)); \
		} while (0)

static inline void array_push_back_size_t_array(array_size_t_array *array, const array_size_t *value)
{
	// Append a deep copy of a size_t row to the nested dynamic array.
	if (!array || !value)
		return;
	array_reserve(array, array->len + 1);
	array_init(&array->buf[array->len]);
	array_append(&array->buf[array->len], value->buf, value->len);
	++array->len;
}

static inline void array_free_size_t_array(array_size_t_array *array)
{
	// Release each nested size_t row before releasing the outer array.
	if (!array)
		return;
	for (size_t i = 0; i < array->len; ++i)
		array_free(&array->buf[i]);
	array_free(array);
}

  #define ARRAY_MAP_BEGIN(map_) ((map_).entries.buf)
  #define ARRAY_MAP_END(map_) ((map_).entries.buf ? (map_).entries.buf + (map_).entries.len : NULL)
  #define ARRAY_MAP_ENDP(map_) ((map_)->entries.buf ? (map_)->entries.buf + (map_)->entries.len : NULL)

static inline size_t clfft_min_size_t(size_t a, size_t b)
{
	// Return the smaller size value without relying on C++ library helpers.
	return a < b ? a : b;
}

static inline size_t clfft_max_size_t(size_t a, size_t b)
{
	// Return the larger size value without relying on C++ library helpers.
	return a > b ? a : b;
}

static inline cl_uint clfft_min_cl_uint(cl_uint a, cl_uint b)
{
	// Return the smaller OpenCL unsigned value without relying on C++ library
	// helpers.
	return a < b ? a : b;
}

static inline cl_uint clfft_max_cl_uint(cl_uint a, cl_uint b)
{
	// Return the larger OpenCL unsigned value without relying on C++ library
	// helpers.
	return a > b ? a : b;
}

static inline unsigned long long clfft_max_ull(unsigned long long a, unsigned long long b)
{
	// Return the larger unsigned long long value without relying on C++ library
	// helpers.
	return a > b ? a : b;
}

static inline bool array_contains_size_t(const array_size_t *array, size_t value)
{
	// Scan the C-like dynamic array for the requested value.
	for (size_t i = 0; i < array_size(array); ++i)
		if (array->buf[i] == value)
			return true;
	return false;
}

static inline void array_sort_unique_size_t(array_size_t *array)
{
	// Sort the small radix list in ascending order with a simple insertion
	// sort.
	for (size_t i = 1; i < array_size(array); ++i)
	{
		size_t value = array->buf[i];
		size_t j = i;
		while (j > 0 && array->buf[j - 1] > value)
		{
			array->buf[j] = array->buf[j - 1];
			--j;
		}
		array->buf[j] = value;
	}

	// Compact adjacent duplicates after sorting.
	size_t out = 0;
	for (size_t i = 0; i < array_size(array); ++i)
	{
		if (i == 0 || array->buf[i] != array->buf[out - 1])
		{
			array->buf[out] = array->buf[i];
			++out;
		}
	}
	array_resize(array, out);
}

static inline void *clfft_checked_malloc(size_t size)
{
	// Allocate at least one byte so zero-sized requests stay freeable.
	if (size == 0)
		size = 1;

	void *ptr = malloc(size);
	if (!ptr)
		abort();
	return ptr;
}

static inline void *clfft_checked_calloc(size_t count, size_t size)
{
	// Allocate at least one byte so zero-sized requests stay freeable.
	if (count == 0 || size == 0)
	{
		count = 1;
		size = 1;
	}

	void *ptr = calloc(count, size);
	if (!ptr)
		abort();
	return ptr;
}

typedef struct buffer_stream_t
{
	buffer_t text;
	int precision_value;
	bool scientific_mode;
} buffer_stream_t;

static inline void bufstream_init(buffer_stream_t *s)
{
	// Reset stream formatting state for a freshly declared stream.
	if (!s)
		return;
	bufinit(&s->text);
	s->precision_value = 6;
	s->scientific_mode = false;
}

static inline void bufstream_cat_cstr(buffer_stream_t *s, const char *v)
{
	if (s)
		bufcatcstr(&s->text, v);
}
static inline void bufstream_cat_char(buffer_stream_t *s, char v)
{
	if (s)
		bufcatchar(&s->text, v);
}
static inline void bufstream_cat_size(buffer_stream_t *s, size_t v)
{
	if (s)
		bufprintf(&s->text, "%zu", v);
}
static inline void bufstream_cat_double(buffer_stream_t *s, double v)
{
	if (s)
		bufprintf(&s->text, s->scientific_mode ? "%.*e" : "%.*f", s->precision_value, v);
}
static inline void bufstream_endline(buffer_stream_t *s)
{
	if (s)
		bufcatchar(&s->text, '\n');
}
static inline void bufstream_scientific(buffer_stream_t *s)
{
	if (s)
		s->scientific_mode = true;
}

#endif

/* Begin inlined header: src\include\unicode.compatibility.h */

#if !defined(amd_unicode_h)
  #define amd_unicode_h

//	Typedefs to support unicode and ansii compilation
typedef buffer_t tstring;

#endif
/* End inlined header: src\include\unicode.compatibility.h */

/* Begin inlined header: src\library\generator.h */

//	Enum to help provide descriptive names to array indices, when indexing into
// our various vectors
typedef enum clfftGenerators
{
	Stockham, // Using the Stockham autosort frameworks
	Transpose_GCN,
	Transpose_SQUARE,
	Transpose_NONSQUARE,
	Copy,
	ENDGENERATORS ///< This value will always be last, and marks the length of
	///< clfftGenerators
} clfftGenerators;

/* End inlined header: src\library\generator.h */

/* Begin inlined header: src\library\lock.h */

typedef struct lockRAII
{
#if defined(_WIN32)
	CRITICAL_SECTION cs;
#else
	pthread_mutex_t mutex;
	pthread_mutexattr_t mAttr;
#endif
} lockRAII;

static void lockRAIIInit(lockRAII *lock)
{
	// Initialize the platform-specific mutex state.
#if defined(_WIN32)
	InitializeCriticalSection(&lock->cs);
#else
	pthread_mutexattr_init(&lock->mAttr);
	pthread_mutexattr_settype(&lock->mAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lock->mutex, &lock->mAttr);
#endif
}

static void lockRAIIInitNamed(lockRAII *lock, const char *name)
{
	// Initialize the mutex state and accept the former diagnostic name
	// argument.
	(void) name;
	lockRAIIInit(lock);
}

static lockRAII *lockRAIICreate(void)
{
	// Allocate and initialize a heap-owned lock object.
	lockRAII *lock = ((lockRAII *) (calloc(1, sizeof(lockRAII))));
	lockRAIIInit(lock);
	return lock;
}

static void lockRAIIDestroy(lockRAII *lock)
{
	// Ignore null lock pointers so cleanup callers stay simple.
	if (lock == NULL)
		return;

	// Release the platform-specific mutex state before freeing the wrapper.
#if defined(_WIN32)
	DeleteCriticalSection(&lock->cs);
#else
	pthread_mutex_destroy(&lock->mutex);
	pthread_mutexattr_destroy(&lock->mAttr);
#endif
	free(lock);
}

static void lockRAIISetName(lockRAII *lock, const buffer_t *name)
{
	// Accept the former diagnostic name arguments without storing them.
	(void) lock;
	(void) name;
}

static void lockRAIIEnter(lockRAII *lock)
{
	// Enter the platform-specific mutex.
#if defined(_WIN32)
	EnterCriticalSection(&lock->cs);
#else
	pthread_mutex_lock(&lock->mutex);
#endif
}

static void lockRAIILeave(lockRAII *lock)
{
	// Leave the platform-specific mutex.
#if defined(_WIN32)
	LeaveCriticalSection(&lock->cs);
#else
	pthread_mutex_unlock(&lock->mutex);
#endif
}

static clfftStatus clfftReturnLocked(lockRAII *lock, clfftStatus status)
{
	// Leave the active lock before returning the status code.
	lockRAIILeave(lock);
	return status;
}

/* End inlined header: src\library\lock.h */

/* Begin inlined header: src\library\private.h */

#if defined(_MSC_VER)
  //	Microsoft Visual C++ compiler
  //
  #define SPRINTF(_buffer, _count, _format, ...) _snprintf_s(_buffer, _count, _TRUNCATE, _format, __VA_ARGS__)
#elif defined(__GNUC__)
  //	Gnu G++
  //
  #define SPRINTF(_buffer, _count, _format, ...) \
	  { \
		  size_t len = (_count) - 1; \
		  snprintf(_buffer, len, _format, __VA_ARGS__); \
		  _buffer[len] = 0; \
	  }
#else
  #error Unknown/unsupported C++ compiler.
#endif

//	Creating a portable defintion of countof
//  This excludes mingw compilers; mingw32 does not have _countof
#if defined(_MSC_VER)
  #define countOf _countof
#else
  #define countOf(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

// This excludes mingw compilers; mingw32 does not have <intrin.h>
#if defined(_MSC_VER)
  #include <intrin.h>

  #if defined(_WIN64)
static inline void BSF(unsigned long *index, const size_t *mask)
{
	_BitScanForward64(index, *mask);
}
  #else
static inline void BSF(unsigned long *index, const size_t *mask)
{
	_BitScanForward(index, *mask);
}
  #endif
#elif defined(__GNUC__)
static inline void BSF(unsigned long *index, const size_t *mask)
{
	*index = __builtin_ctz(*mask);
}
#endif

//	This header file is not visible to clients, and contains internal structures
// and functions for use 	by the FFT library.  Since this header is private to
// this implementation, there is no need to keep 	strict C compliance.

//	Enum to help provide descriptive names to array indices, when indexing into
// our various vectors
enum clfftDim_Index
{
	DimX,	    ///< 1 Dimension
	DimY,	    ///< 2 Dimension
	DimZ,	    ///< 3 Dimension
	DimW,	    ///< 4th Dimension
	ENDDIMINDEX ///< This value will always be last, and marks the length of
		    ///< clfftDim_Index
};

//(currently) true if length is a power of 2,3,5,7,11,13
static inline bool IsASupportedLength(size_t length)
{
	while (length > 1)
		if (length % 2 == 0)
			length /= 2;
		else if (length % 3 == 0)
			length /= 3;
		else if (length % 5 == 0)
			length /= 5;
		else if (length % 7 == 0)
			length /= 7;
		else if (length % 11 == 0)
			length /= 11;
		else if (length % 13 == 0)
			length /= 13;
		else
			return false;
	return true;
}

//	This is used to either wrap an OpenCL function call, or to explicitly check
// a variable for an OpenCL error condition. 	If an error occurs, we issue a
// return statement to exit the calling function.
#define OPENCL_V(fn, msg) \
	{ \
		clfftStatus vclStatus = ((clfftStatus) (fn)); \
		switch (vclStatus) \
		{ \
			case CL_SUCCESS: /**< No error */ break; \
			default: \
			{ \
				return vclStatus; \
			} \
		} \
	}

#define OPENCL_V_LOCKED(lock, fn, msg) \
	{ \
		clfftStatus vclStatus = ((clfftStatus) (fn)); \
		switch (vclStatus) \
		{ \
			case CL_SUCCESS: /**< No error */ break; \
			default: \
			{ \
				return clfftReturnLocked(lock, vclStatus); \
			} \
		} \
	}
static inline bool IsPo2(size_t u)
{
	return (u != 0) && (0 == (u & (u - 1)));
}

static inline size_t DivRoundingUpSizeT(size_t a, size_t b)
{
	return (a + (b - 1)) / b;
}

static inline unsigned long long DivRoundingUpULL(unsigned long long a, unsigned long long b)
{
	return (a + (b - 1)) / b;
}

static inline size_t BitScanF(size_t n)
{
	assert(n != 0);
	unsigned long tmp = 0;
	BSF(&tmp, &n);
	return (size_t) tmp;
}

#define ARG_CHECK(_proposition) \
	{ \
		bool btmp = (_proposition); \
		assert(btmp); \
		if (!btmp) \
			return CLFFT_INVALID_ARG_VALUE; \
	}

#define BUG_CHECK(_proposition) \
	{ \
		bool btmp = (_proposition); \
		assert(btmp); \
		if (!btmp) \
			return CLFFT_BUGCHECK; \
	}

clfftStatus clfftLocalMemSize(const clfftPlanHandle plHandle, cl_ulong *local_mem_size);

/*! @brief Save to disk a file containing the contents of a baked plan.
 *  @details A plan is a repository of state for calculating FFT's. Saves
 * the details for a plan to allow the user to easily recreate a plan and
 * execute it without having to first build the kernel.
 *  @param[in] plHandle Handle to the plan to be written to disk
 *  @param[in] filename The desired name of the output file for the stored
 * plan
 *  @return Enum describing error condition; superset of OpenCL error codes
 */
clfftStatus clfftWritePlanToDisk(clfftPlanHandle plHandle, const char *filename);

/*! @brief Read from disk a file containing the contents of a baked plan.
 *  @details A plan is a repository of state for calculating FFT's. Reads
 * the details for a plan from a file on disk and duplicates the plan in the
 * provided plan handle.
 *  @param[out] plHandle Handle to the plan to be set to details from the
 * file
 *  @param[in] filename The name of the file containing the stored plan
 *  @return Enum describing error condition; superset of OpenCL error codes
 */
clfftStatus clfftReadPlanFromDisk(clfftPlanHandle plHandle, const char *filename);

/* End inlined header: src\library\private.h */

/* Begin inlined header: src\library\plan.h */

// TODO:  These arbitrary parameters should be tuned for the type of GPU
// being used.  These values are probably OK for Radeon 58xx and 68xx.
enum
{
	CLFFT_ARBITRARY_MAX_DIMS = 3,
	//  The clEnqueuNDRangeKernel accepts a multi-dimensional domain array.
	//  The # of dimensions is arbitrary, but limited by the OpenCL
	//  implementation usually to 3 dimensions
	//  (CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS). The kernel generator also assumes
	//  a limit on the # of dimensions.

	CLFFT_ARBITRARY_SIMD_WIDTH = 64,
	//  Workgroup size.  This is the # of work items that share
	//  local data storage (LDS).  This # is best for Evergreen gpus,
	//  but might change in the future.

	CLFFT_ARBITRARY_LDS_BANK_BITS = 5,
	CLFFT_ARBITRARY_LDS_BANK_SIZE = (1 << CLFFT_ARBITRARY_LDS_BANK_BITS),
	CLFFT_ARBITRARY_LDS_PADDING = 0, // true,
					 //   On AMD hardware, the low-order bits of the local_id enumerate
					 //   the work items that access LDS in parallel.  Ideally, we will
					 //   pad our LDS arrays so that these work items access different
					 //   banks of the LDS. 2 ** LDS_BANK_BITS is the number of LDS banks.
					 //   If LDS_PADDING is non-zero, the kernel generator should pad the
					 //   LDS arrays to reduce or eliminate bank conflicts.

	CLFFT_ARBITRARY_LDS_FRACTION_IDEAL = 6, // i.e., 1/6th
	CLFFT_ARBITRARY_LDS_FRACTION_MAX = 4,	// i.e., 1/4
						//  For best performance, each workgroup should use 1/IDEAL'th the
						//  amount of LDS revealed by clGetDeviceInfo (..
						//  CL_DEVICE_LOCAL_MEM_SIZE, ...) However, we can use up to 1/MAX'th
						//  of LDS per workgroup when necessary to perform the FFT in a
						//  single pass instead of multiple passes. This tuning parameter is
						//  a good value for Evergreen gpus, but might change in the future.

	CLFFT_ARBITRARY_LDS_COMPLEX = 0,
	//  This is the default value for fft_LdsComplex in FFTKernelGenKeyParams.
	//  The generated kernels require so many bytes of LDS for each single
	//  precision
	//..complex number in the vector.
	//  If LDS_COMPLEX, then we declare an LDS array of complex numbers (8 bytes
	//  each) and swap data between workitems with a single barrier. If !
	//  LDS_COMPLEX, then we declare an LDS array or scalar numbers (4 bytes
	//  each) and swap data between workitems in two phases, with extra
	//  barriers. The former approach uses fewer instructions and barriers; The
	//  latter uses half as much LDS space, so twice as many wavefronts can be
	//  run in parallel.

	CLFFT_ARBITRARY_TWIDDLE_DEE = 8,
	//  number of bits per row of matrix.
};

typedef enum BlockComputeType
{
	BCT_C2C, // Column to column
	BCT_C2R, // Column to row
	BCT_R2C, // Row to column
} BlockComputeType;

// NonSquareKernelType
typedef enum NonSquareTransposeKernelType
{
	NON_SQUARE_TRANS_PARENT,
	NON_SQUARE_TRANS_TRANSPOSE_BATCHED_LEADING,
	NON_SQUARE_TRANS_TRANSPOSE_BATCHED,
	NON_SQUARE_TRANS_SWAP
} NonSquareTransposeKernelType;

/*
There are three ways of conducting inplace transpose with 1:2 (or 2:1) dimension
ratio. A. first conduct line swapping kernels for the whole non square matrix
then conduct batched square transpose along column dim (a 'real' batched
transpose) B. first conduct batched square transpose along column dim (a 'real'
batched transpose) then conduct line swapping kernels for the whole non square
matrix (for 2:1 case) C. first conduct batched square transpose along leading
dim (row dim) then conduct line swapping kernels for the whole non square matrix
Note that the twiddle computation has to go at the begining of the first kernel
or the end of the second kernel

if leading dimension is bigger, it makes more sense (faster) to swap line first
and then conduct batched square transpose if leading dimension is smaller, it
makes more sense (faster) to conduct batched transpose and then swap lines.
*/
typedef enum NON_SQUARE_KERNEL_ORDER
{
	NOT_A_TRANSPOSE,
	SWAP_AND_TRANSPOSE,	    // A.
	TRANSPOSE_AND_SWAP,	    // B.
	TRANSPOSE_LEADING_AND_SWAP, // C.
} NON_SQUARE_KERNEL_ORDER;

#define CLFFT_CB_SIZE 32
#define CLFFT_MAX_INTERNAL_DIM 16

/*! @brief Data structure to store the callback function string and other
 * metadata passed by client
 *  @details Client sets the callback function and other required parameters
 * through clfftSetPlanCallback() in order to register the callback function.
 * The library populates these values into this data structure
 */
typedef struct clfftCallbackParam_
{
	int localMemSize;	/*!< optional local memory size if needed by callback */
	const char *funcname;	/*!< callback function name */
	const char *funcstring; /*!< callback function in string form */
} clfftCallbackParam;

typedef struct FFTKernelGenKeyParams
{
	/*
	 *	This structure distills a subset of the fftPlan data,
	 *	including all information that is used to generate the OpenCL kernel.
	 *	This structure can be used as a key to reusing kernels that have already
	 *	been compiled.
	 */
	size_t fft_DataDim;			      // Dimensionality of the data
	size_t fft_N[CLFFT_MAX_INTERNAL_DIM];	      // [0] is FFT size, e.g. 1024
						      // This must be <= size of LDS!
	size_t fft_inStride[CLFFT_MAX_INTERNAL_DIM];  // input strides
	size_t fft_outStride[CLFFT_MAX_INTERNAL_DIM]; // output strides

	clfftResultLocation fft_placeness;
	clfftLayout fft_inputLayout;
	clfftLayout fft_outputLayout;
	clfftPrecision fft_precision;
	double fft_fwdScale;
	double fft_backScale;

	size_t fft_SIMD;    // Assume this SIMD/workgroup size
	size_t fft_LDSsize; // Limit the use of LDS to this many bytes.
	size_t fft_R;	    // # of complex values to keep in working registers
			    // SIMD size * R must be <= size of LDS!

	size_t fft_MaxWorkGroupSize; // Limit for work group size

	bool fft_3StepTwiddle; // This is one pass of the "3-step" algorithm;
	// so extra twiddles are applied on output.
	bool fft_twiddleFront; // do twiddle scaling at the beginning pass

	bool fft_realSpecial; // this is the flag to control the special case step
	// (4th step) in the 5-step real 1D large breakdown
	size_t fft_realSpecial_Nr;

	bool fft_RCsimple;

	bool transOutHorizontal; // tiles traverse the output buffer in horizontal
				 // direction

	bool blockCompute;
	BlockComputeType blockComputeType;
	size_t blockSIMD;
	size_t blockLDS;

	NonSquareTransposeKernelType nonSquareKernelType;
	// sometimes non square matrix are broken down into a number of
	// square matrix during inplace transpose
	// let's call this number transposeMiniBatchSize
	// no user of the library should set its value
	size_t transposeMiniBatchSize;
	// transposeBatchSize is the number of batchs times transposeMiniBatchSzie
	// no user of the library should set its value
	size_t transposeBatchSize;
	// no user of the library should set its value
	NON_SQUARE_KERNEL_ORDER nonSquareKernelOrder;

	bool fft_hasPreCallback;
	clfftCallbackParam fft_preCallback;

	bool fft_hasPostCallback;
	clfftCallbackParam fft_postCallback;

	cl_ulong limit_LocalMemSize;

} FFTKernelGenKeyParams;

typedef struct FFTPlan FFTPlan;
typedef struct FFTRepo FFTRepo;

// Action ID
typedef enum FFTActionImplID
{
	FFT_DEFAULT_STOCKHAM_ACTION,
	FFT_DEFAULT_TRANSPOSE_ACTION,
	FFT_DEFAULT_COPY_ACTION,
	FFT_STATIC_STOCKHAM_ACTION
} FFTActionImplID;

//
// FFTKernelSignatureHeader
//
// This structure is a wrapper for the FFTKernelSignature.
// It stores the signature size and the action ID. This ensure that every
// FFTKernelSignature (even with an empty DATA) is unique
//
// This struct is used as the return type of FFTActionGetSignatureData()
//
typedef struct FFTKernelSignatureHeader
{
	int datasize;
	FFTActionImplID id;

	// clfftLayout           fft_inputLayout;
	// clfftLayout           fft_outputLayout;

} FFTKernelSignatureHeader;

//
// FFTKernelSignature*
//
// These structs represent the signatures of generated actions. The header
// stores the action ID and signature size, and the data block stores the bytes
// that characterize the generated kernel.
//
// They are used as keys in the dynamic cache managed by FFTRepo.
//
typedef struct FFTKernelSignatureCopy
{
	FFTKernelSignatureHeader header;
	FFTKernelGenKeyParams data;

} FFTKernelSignatureCopy;

typedef struct FFTKernelSignatureStockham
{
	FFTKernelSignatureHeader header;
	FFTKernelGenKeyParams data;

} FFTKernelSignatureStockham;

typedef struct FFTKernelSignatureTranspose
{
	FFTKernelSignatureHeader header;
	FFTKernelGenKeyParams data;

} FFTKernelSignatureTranspose;

static void FFTKernelSignatureHeaderInit(FFTKernelSignatureHeader *header, int size, FFTActionImplID id)
{
	// Clear the whole signature block because the header is its first field.
	memset((void *) header, 0, (size_t) size);

	// Store the signature metadata after clearing the payload.
	header->datasize = size;
	header->id = id;
}

static void FFTKernelSignatureCopyInit(FFTKernelSignatureCopy *signature)
{
	// Initialize a copy-action signature payload.
	FFTKernelSignatureHeaderInit(&signature->header, sizeof(*signature), FFT_DEFAULT_COPY_ACTION);
}

static void FFTKernelSignatureStockhamInit(FFTKernelSignatureStockham *signature)
{
	// Initialize a stockham-action signature payload.
	FFTKernelSignatureHeaderInit(&signature->header, sizeof(*signature), FFT_DEFAULT_STOCKHAM_ACTION);
}

static void FFTKernelSignatureTransposeInit(FFTKernelSignatureTranspose *signature)
{
	// Initialize a transpose-action signature payload.
	FFTKernelSignatureHeaderInit(&signature->header, sizeof(*signature), FFT_DEFAULT_TRANSPOSE_ACTION);
}

//
// FFTAction is the base struct for all actions used to implement FFT computes
//
// An action basically implements some OpenCL related actions, for instance:
//  - Generating a kernel source code from kernel characteristics
//  - Computing the work-group local sizes according to a kernel
//  - Enqueuing arguments to the kernel call
//
// Kernels generated and compiled by an action are stored in the different
// cache mechanism (see repo.h for the dynamic cache and fft_binary_lookup.h
// for disk cache for more information). Each cache entry can be obtained from
// the action signature which is set of byte characterizing the action itself.
//
// At the moment, FFTAction only implements the enqueue method which is
// inherited by every action subclasses. But it may change in the time. There
// are no clear rules and the choices made until now are still subject to
// change.
//
typedef struct FFTAction
{
	FFTPlan *plan;
	clfftGenerators generator;
	void *owner;
} FFTAction;

static void FFTActionDestroy(FFTAction *action);

static void FFTActionInit(FFTAction *action, FFTPlan *fftPlan, clfftGenerators actionGenerator, void *actionOwner, clfftStatus *err)
{
	// Populate the embedded base action fields explicitly.
	action->plan = fftPlan;
	action->generator = actionGenerator;
	action->owner = actionOwner;

	// Report successful base initialization to the caller.
	if (err != NULL)
		*err = CLFFT_SUCCESS;
}

//	The "envelope" is a set of limits imposed by the hardware
//	This will depend on the GPU(s) in the OpenCL context.
//	If there are multiple devices, this should be the least
//	common denominators.
//
typedef struct FFTEnvelope
{
	cl_ulong limit_LocalMemSize;
	//  this is the minimum of CL_DEVICE_LOCAL_MEM_SIZE
	size_t limit_Dimensions;
	//  this is the minimum of CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS
	size_t limit_Size[8];
	//  these are the minimima of CL_DEVICE_MAX_WORK_ITEM_SIZES[0..n]
	size_t limit_WorkGroupSize;
	//  this is the minimum of CL_DEVICE_MAX_WORK_GROUP_SIZE

	// ??  CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE

} FFTEnvelope;

//	This struct contains objects that are specific to a particular FFT
// transform, and the data herein is useful 	for us to know ahead of
// transform time such that we can optimize for these settings
struct FFTPlan
{
	bool baked;

	//	Properties provided by the user.
	clfftDim dim;
	clfftLayout inputLayout;
	clfftLayout outputLayout;
	clfftResultLocation placeness;
	clfftResultTransposed transposed;
	clfftPrecision precision;
	cl_context context;
	double forwardScale, backwardScale;
	size_t iDist, oDist;
	size_t batchsize;

	// Note the device passed to BakePlan, assuming we are baking for one device
	// TODO, change this logic for handling multiple GPUs/devices
	cl_device_id bakeDevice;

	// Disabling devices member, plan has 1-on-1 mapping with single device as
	// identified by bakeDevice
	//	Devices that the user specified in the context passed to the create
	// function
	// array_cl_device_id devices;

	//	Length of the FFT in each dimension
	array_size_t length;

	//	Stride of the FFT in each dimension
	array_size_t inStride, outStride;

	//	Hardware Limits
	FFTEnvelope envelope;

	// Reserved copy for large 1d, 2d, and 3d plan
	size_t tmpBufSize;
	cl_mem intBuffer;
	bool libCreatedIntBuffer;

	// for RC copies
	size_t tmpBufSizeRC;
	cl_mem intBufferRC;

	// for C-to-R transforms that are OUTOFPLACE
	// we need this because the user supplied output buffer is not big enough
	// to hold intermediate results for any problem other than normal 1D
	size_t tmpBufSizeC2R;
	cl_mem intBufferC2R;

	size_t large1D;
	bool large2D;
	bool twiddleFront;

	clfftPlanHandle planX;
	clfftPlanHandle planY;
	clfftPlanHandle planZ;

	bool transflag;
	bool transOutHorizontal;
	clfftPlanHandle planTX;
	clfftPlanHandle planTY;
	clfftPlanHandle planTZ; // reserve for 3D transpose

	clfftPlanHandle planRCcopy;
	clfftPlanHandle planCopy;

	// Plan resources
	//
	cl_mem const_buffer;

	// Generator type
	clfftGenerators gen;

	// Real-Complex simple flag
	// if this is set we do real to-and-from full complex using simple algorithm
	// where imaginary of input is set to zero in forward and imaginary not
	// written in backward
	bool RCsimple;

	// Real FFT special flag
	// if this is set it means we are doing the 4th step in the 5-step real FFT
	// breakdown algorithm
	bool realSpecial;

	size_t realSpecial_Nr; // this value stores the logical column height (N0) of
			       // matrix in the 4th step length[1] should be 1 + N0/2

	// User created plan
	bool userPlan;

	// Allocate no extra memory
	bool allOpsInplace;

	// flag to indicate transpose placeness in 2D breakdown
	bool transpose_in_2d_inplace;

	// A flag to say that blocked FFTs are going to be performed
	// It can only be one of these: column to row, row to column or column to
	// column row to row is just the normal case where blocking is not needed
	bool blockCompute;
	BlockComputeType blockComputeType;

	bool hasPreCallback;
	bool hasPostCallback;

	clfftCallbackParam preCallback;
	clfftCallbackParam postCallbackParam;

	cl_mem precallUserData;
	cl_mem postcallUserData;

	clfftPlanHandle plHandle;

	// The action
	FFTAction *action;

	NonSquareTransposeKernelType nonSquareKernelType;
	// sometimes non square matrix are broken down into a number of
	// square matrix during inplace transpose
	// let's call this number transposeMiniBatchSize
	// no user of the library should set its value
	size_t transposeMiniBatchSize;
	NON_SQUARE_KERNEL_ORDER nonSquareKernelOrder;
};

static size_t FFTPlanElementSize(const FFTPlan *fftPlan);
static clfftStatus FFTPlanAllocateBuffers(FFTPlan *fftPlan);
static clfftStatus FFTPlanReleaseBuffers(FFTPlan *fftPlan);
static clfftStatus FFTPlanGetMax1DLength(const FFTPlan *fftPlan, size_t *longest);
static clfftStatus FFTPlanConstructAndEnqueueConstantBuffers(FFTPlan *fftPlan, cl_command_queue *commQueueFFT);
static clfftStatus FFTPlanGetEnvelope(const FFTPlan *fftPlan, const FFTEnvelope **ppEnvelope);
static clfftStatus FFTPlanSetEnvelope(FFTPlan *fftPlan);
static clfftStatus FFTPlanGetMax1DLengthStockham(const FFTPlan *fftPlan, size_t *longest);

static void FFTPlanInit(FFTPlan *fftPlan)
{
	// Return immediately when no plan storage was supplied.
	if (fftPlan == NULL)
		return;

	// Clear all fields before assigning the public default state.
	memset((void *) fftPlan, 0, sizeof(*fftPlan));

	// Assign the default transform configuration.
	fftPlan->baked = false;
	fftPlan->dim = CLFFT_1D;
	fftPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
	fftPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
	fftPlan->placeness = CLFFT_INPLACE;
	fftPlan->transposed = CLFFT_NOTRANSPOSE;
	fftPlan->precision = CLFFT_SINGLE;
	fftPlan->context = NULL;
	fftPlan->forwardScale = 1.0;
	fftPlan->backwardScale = 1.0;
	fftPlan->iDist = 1;
	fftPlan->oDist = 1;
	fftPlan->batchsize = 1;

	// Assign the default resource and decomposition state.
	fftPlan->intBuffer = NULL;
	fftPlan->intBufferRC = NULL;
	fftPlan->intBufferC2R = NULL;
	fftPlan->gen = Stockham;
	fftPlan->blockComputeType = BCT_C2C;
	fftPlan->nonSquareKernelType = NON_SQUARE_TRANS_PARENT;
	fftPlan->transposeMiniBatchSize = 1;
	fftPlan->nonSquareKernelOrder = NOT_A_TRANSPOSE;
}

static void FFTPlanRelease(FFTPlan *fftPlan)
{
	// Release buffers and generated action storage owned by the plan.
	if (fftPlan == NULL)
		return;

	FFTPlanReleaseBuffers(fftPlan);

	if (fftPlan->action != NULL)
	{
		FFTActionDestroy(fftPlan->action);
		fftPlan->action = 0;
	}
}

static FFTPlan *FFTPlanCreate(void)
{
	// Allocate raw plan storage and initialize it explicitly.
	FFTPlan *fftPlan = (FFTPlan *) clfft_checked_malloc(sizeof(FFTPlan));
	FFTPlanInit(fftPlan);
	return fftPlan;
}

static void FFTPlanDestroy(FFTPlan *fftPlan)
{
	// Return immediately when no plan object is supplied.
	if (fftPlan == NULL)
		return;

	// Release OpenCL buffers and generated action storage first.
	FFTPlanRelease(fftPlan);

	// Release dynamic arrays embedded in the plan.
	array_clear(&fftPlan->length);
	free(fftPlan->length.buf);
	array_clear(&fftPlan->inStride);
	free(fftPlan->inStride.buf);
	array_clear(&fftPlan->outStride);
	free(fftPlan->outStride.buf);

	// Free the raw plan storage.
	free(fftPlan);
}

static bool Is1DPossible(size_t length, size_t large1DThreshold)
{
	if (length > large1DThreshold)
		return false;

	if ((length % 7 == 0) && (length % 5 == 0) && (length % 3 == 0))
		return false;

	// radix 11 & 2 is ok, anything else we cannot do in 1 kernel
	if ((length % 11 == 0) && ((length % 13 == 0) || (length % 7 == 0) || (length % 5 == 0) || (length % 3 == 0)))
		return false;

	// radix 13 & 2 is ok, anything else we cannot do in 1 kernel
	if ((length % 13 == 0) && ((length % 11 == 0) || (length % 7 == 0) || (length % 5 == 0) || (length % 3 == 0)))
		return false;

	return true;
}

/* End inlined header: src\library\plan.h */

/* Begin inlined header: src\library\repo.h */

//	This struct contains objects that we wish to retain between individual calls
// into the FFT interface. 	These objects will be shared across different
// individual FFT plans, and we wish to keep only one 	copy of these programs,
// objects and events.  When the client decides that they either want to reset
//	the library or release all resources, this Repo will release all acquired
// resources and clean itself 	up as much as it can.  It is implemented as a
// Singleton object.
typedef struct FFTRepoKey
{
	clfftGenerators gen;
	const FFTKernelSignatureHeader *data;
	cl_context context;
	cl_device_id device;
	bool dataIsPrivate;
} FFTRepoKey;

static bool fftRepoKeyEqual(const FFTRepoKey a, const FFTRepoKey b)
{
	// Compare inexpensive fields before checking the signature bytes.
	if (a.gen != b.gen || a.context != b.context || a.device != b.device)
		return false;
	if (a.data == b.data)
		return true;
	if (!a.data || !b.data)
		return false;
	if (a.data->datasize != b.data->datasize)
		return false;
	return memcmp(a.data, b.data, a.data->datasize) == 0;
}

typedef struct fftRepoValue
{
	buffer_t ProgramString;
	buffer_t EntryPoint_fwd;
	buffer_t EntryPoint_back;
	cl_program clProgram;
} fftRepoValue;

typedef struct fftRepoEntry
{
	FFTRepoKey first;
	fftRepoValue second;
} fftRepoEntry;

typedef struct array_fftRepoEntry
{
	fftRepoEntry *buf;
	size_t len;
	size_t as;
} array_fftRepoEntry;

typedef struct fftRepoType
{
	array_fftRepoEntry entries;
} fftRepoType;

typedef fftRepoEntry *fftRepoEntryPtr;

static fftRepoEntryPtr fftRepoFind(fftRepoType *map, FFTRepoKey key)
{
	// Search cached FFT programs linearly by their generated-key data.
	if (map == NULL)
		return NULL;
	for (size_t i = 0; i < map->entries.len; ++i)
		if (fftRepoKeyEqual(map->entries.buf[i].first, key))
			return map->entries.buf + i;
	return map->entries.buf ? map->entries.buf + map->entries.len : NULL;
}

static fftRepoValue *fftRepoGet(fftRepoType *map, FFTRepoKey key)
{
	// Reuse an existing program entry when the key is already cached.
	fftRepoEntryPtr it = fftRepoFind(map, key);
	if (it != ARRAY_MAP_ENDP(map))
		return &it->second;

	// Append a zeroed program entry for the new key.
	fftRepoEntry entry;
	memset((void *) &entry, 0, sizeof(entry));
	entry.first = key;
	array_push_back(&map->entries, entry);
	return &map->entries.buf[map->entries.len - 1].second;
}

typedef struct fftKernels
{
	cl_kernel kernel_fwd;
	cl_kernel kernel_back;
	lockRAII *kernel_fwd_lock;
	lockRAII *kernel_back_lock;
} fftKernels;

typedef struct mapKernelEntry
{
	cl_program first;
	fftKernels second;
} mapKernelEntry;

typedef struct array_mapKernelEntry
{
	mapKernelEntry *buf;
	size_t len;
	size_t as;
} array_mapKernelEntry;

typedef struct mapKernelType
{
	array_mapKernelEntry entries;
} mapKernelType;

typedef mapKernelEntry *KernelEntryPtr;

static KernelEntryPtr mapKernelFind(mapKernelType *map, cl_program key)
{
	// Search cached kernels linearly by their OpenCL program handle.
	if (map == NULL)
		return NULL;
	for (size_t i = 0; i < map->entries.len; ++i)
		if (map->entries.buf[i].first == key)
			return map->entries.buf + i;
	return map->entries.buf ? map->entries.buf + map->entries.len : NULL;
}

static fftKernels *mapKernelGet(mapKernelType *map, cl_program key)
{
	// Reuse an existing kernel entry when the program is already cached.
	KernelEntryPtr it = mapKernelFind(map, key);
	if (it != ARRAY_MAP_ENDP(map))
		return &it->second;

	// Append a zeroed kernel entry for the new program.
	mapKernelEntry entry;
	memset((void *) &entry, 0, sizeof(entry));
	entry.first = key;
	array_push_back(&map->entries, entry);
	return &map->entries.buf[map->entries.len - 1].second;
}

typedef struct repoPlansValue
{
	FFTPlan *first;
	lockRAII *second;
} repoPlansValue;

typedef struct repoPlansEntry
{
	clfftPlanHandle first;
	repoPlansValue second;
} repoPlansEntry;

typedef struct array_repoPlansEntry
{
	repoPlansEntry *buf;
	size_t len;
	size_t as;
} array_repoPlansEntry;

typedef struct repoPlansType
{
	array_repoPlansEntry entries;
} repoPlansType;

typedef repoPlansEntry *repoPlansEntryPtr;

static repoPlansEntryPtr repoPlansFind(repoPlansType *map, clfftPlanHandle key)
{
	// Search cached plans linearly by public plan handle.
	if (map == NULL)
		return NULL;
	for (size_t i = 0; i < map->entries.len; ++i)
		if (map->entries.buf[i].first == key)
			return map->entries.buf + i;
	return map->entries.buf ? map->entries.buf + map->entries.len : NULL;
}

static repoPlansValue *repoPlansGet(repoPlansType *map, clfftPlanHandle key)
{
	// Reuse an existing plan entry when the handle is already present.
	repoPlansEntryPtr it = repoPlansFind(map, key);
	if (it != ARRAY_MAP_ENDP(map))
		return &it->second;

	// Append a zeroed plan entry for the new handle.
	repoPlansEntry entry;
	memset((void *) &entry, 0, sizeof(entry));
	entry.first = key;
	array_push_back(&map->entries, entry);
	return &map->entries.buf[map->entries.len - 1].second;
}

struct FFTRepo
{
	fftRepoType mapFFTs;
	mapKernelType mapKernels;
	repoPlansType repoPlans;
	clfftSetupData setupData;
};

static size_t FFTRepoPlanCount = 1;

static lockRAII *FFTRepoLockRepo(void)
{
	static lockRAII lock;
	static bool lockInitialized = false;
	// Initialize the repository lock on first use.
	if (!lockInitialized)
	{
		lockRAIIInitNamed(&lock, "FFTRepo");
		lockInitialized = true;
	}
	return &lock;
}

static FFTRepo *FFTRepoGetInstance(void)
{
	static FFTRepo fftRepo;
	return &fftRepo;
}

static clfftStatus FFTRepoReleaseResources(FFTRepo *repo);
static clfftStatus FFTRepoSetProgramCode(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, const buffer_t *kernel, cl_device_id device,
	cl_context planContext);
static clfftStatus FFTRepoGetProgramCode(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, buffer_t *kernel, cl_device_id device, cl_context planContext);
static clfftStatus FFTRepoSetProgramEntryPoints(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, const char *kernel_fwd, const char *kernel_back,
	cl_device_id device, cl_context planContext);
static clfftStatus FFTRepoGetProgramEntryPoint(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, clfftDirection dir, buffer_t *kernel, cl_device_id device,
	cl_context planContext);
static clfftStatus FFTRepoSetclProgram(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, cl_program prog, cl_device_id device, cl_context planContext);
static clfftStatus FFTRepoGetclProgram(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, cl_program *prog, cl_device_id device, cl_context planContext);
static clfftStatus FFTRepoSetclKernel(FFTRepo *repo, cl_program prog, clfftDirection dir, cl_kernel kernel);
static clfftStatus FFTRepoGetclKernel(FFTRepo *repo, cl_program prog, clfftDirection dir, cl_kernel *kernel, lockRAII **kernelLock);
static clfftStatus FFTRepoCreatePlan(FFTRepo *repo, clfftPlanHandle *plHandle, FFTPlan **fftPlan);
static clfftStatus FFTRepoGetPlan(FFTRepo *repo, clfftPlanHandle plHandle, FFTPlan **fftPlan, lockRAII **planLock);
static clfftStatus FFTRepoDeletePlan(FFTRepo *repo, clfftPlanHandle *plHandle);

static void FFTRepoKeyPrivatizeData(FFTRepoKey *key)
{
	// Copy signature data into private key-owned storage.
	if (!key || !key->data)
		return;
	char *tmp = (char *) clfft_checked_malloc(key->data->datasize);
	memcpy(tmp, key->data, key->data->datasize);
	key->data = (FFTKernelSignatureHeader *) tmp;
	key->dataIsPrivate = true;
}

static void FFTRepoKeyDeleteData(FFTRepoKey *key)
{
	// Release private signature data held by a repository key.
	if (!key)
		return;
	if (key->dataIsPrivate && (key->data != NULL))
	{
		char *tmp = (char *) key->data;
		free(tmp);
		key->data = 0;
	}
}

static inline void FFTRepoValueFree(fftRepoValue *value)
{
	// Release owned repository strings explicitly before entry storage is
	// cleared.
	if (!value)
		return;
	buffree(&value->ProgramString);
	buffree(&value->EntryPoint_fwd);
	buffree(&value->EntryPoint_back);
}

/* End inlined header: src\library\repo.h */

/* Begin inlined header: src\library\action.h */

//
// FFTGeneratedCopyAction
//
// Implements a Copy action for the FFT
// Its signature is represented by FFTKernelGenKeyParams structure
//
// This struct implements:
//  - the generation of the kernel string
//  - the build of the kernel
//
// The structure FFTKernelGenKeyParams is used to characterize and generate
// the appropriate copy kernel. That structure is used for the signature of
// this action. It is common to Stockham, copy and transpose methods. For
// convenience, this structure is used for every FFTGenerated*Action struct,
// but in practice the copy action only use a few information of that
// structure, so a proper structure should be used instead.
//
typedef struct FFTGeneratedCopyAction
{
	FFTAction base;

	FFTKernelSignatureCopy signature;
} FFTGeneratedCopyAction;

//
// FFTGeneratedStockhamAction
//
// Represents a Stockham action for the FFT. This struct implements the former
// mechanism of kernel generation and compilation for Stockham method.
//
// This struct implements:
//  - the generation of the kernel string
//  - the build of the kernel
//
// The structure FFTKernelGenKeyParams is used to characterize and generate
// the appropriate kernel. That structure is used for the signature of this
// action. For convenience, this structure is used for every
// FFTGenerated*Action struct, but a "Stockham-specific" version of that
// structure should be used instead.
//
typedef struct FFTGeneratedStockhamAction
{
	FFTAction base;

	FFTKernelSignatureStockham signature;
} FFTGeneratedStockhamAction;

// FFTGeneratedTransposeGCNAction
//
// Implements a TransposeGCN action for the FFT
// Its signature is represented by FFTKernelGenKeyParams structure
//
// This struct implements:
//  - the generation of the kernel string
//  - the build of the kernel
//
// The structure FFTKernelGenKeyParams is used to characterize and generate
// the appropriate transpose kernel. That structure is used for the signature of
// this action. It is common to Stockham, copy and transpose methods. For
// convenience, this structure is used for every FFTGenerated*Action struct,
// but in practice the transpose action only use a few information of that
// structure, so a proper structure should be used instead.
//
typedef struct FFTGeneratedTransposeGCNAction
{
	FFTAction base;

	FFTKernelSignatureTranspose signature;
} FFTGeneratedTransposeGCNAction;

// FFTGeneratedTransposeSquareAction
//
// Implements a TransposeSquare action for the FFT
// Its signature is represented by FFTKernelGenKeyParams structure
//
// This struct implements:
//  - the generation of the kernel string
//  - the build of the kernel
//
// The structure FFTKernelGenKeyParams is used to characterize and generate
// the appropriate transpose kernel. That structure is used for the signature of
// this action. It is common to Stockham, copy and transpose methods. For
// convenience, this structure is used for every FFTGenerated*Action struct,
// but in practice the transpose action only use a few information of that
// structure, so a proper structure should be used instead.
//
typedef struct FFTGeneratedTransposeSquareAction
{
	FFTAction base;

	FFTKernelSignatureTranspose signature;
} FFTGeneratedTransposeSquareAction;

// FFTGeneratedTransposeNonSquareAction
//
// Implements a TransposeSquare action for the FFT
// Its signature is represented by FFTKernelGenKeyParams structure
//
// This struct implements:
//  - the generation of the kernel string
//  - the build of the kernel
//
// The structure FFTKernelGenKeyParams is used to characterize and generate
// the appropriate transpose kernel. That structure is used for the signature of
// this action. It is common to Stockham, copy and transpose methods. For
// convenience, this structure is used for every FFTGenerated*Action struct,
// but in practice the transpose action only use a few information of that
// structure, so a proper structure should be used instead.
//
typedef struct FFTGeneratedTransposeNonSquareAction
{
	FFTAction base;

	FFTKernelSignatureTranspose signature;
} FFTGeneratedTransposeNonSquareAction;

static clfftStatus FFTGeneratedCopyActionGenerateKernel(FFTGeneratedCopyAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT);
static clfftStatus FFTGeneratedCopyActionGetWorkSizes(FFTGeneratedCopyAction *action, array_size_t *globalws, array_size_t *localws);
static clfftStatus FFTGeneratedCopyActionInitParams(FFTGeneratedCopyAction *action);
static bool FFTGeneratedCopyActionBuildForwardKernel(FFTGeneratedCopyAction *action);
static bool FFTGeneratedCopyActionBuildBackwardKernel(FFTGeneratedCopyAction *action);
static clfftStatus FFTGeneratedStockhamActionGenerateKernel(FFTGeneratedStockhamAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT);
static clfftStatus FFTGeneratedStockhamActionGetWorkSizes(FFTGeneratedStockhamAction *action, array_size_t *globalws, array_size_t *localws);
static clfftStatus FFTGeneratedStockhamActionInitParams(FFTGeneratedStockhamAction *action);
static bool FFTGeneratedStockhamActionBuildForwardKernel(FFTGeneratedStockhamAction *action);
static bool FFTGeneratedStockhamActionBuildBackwardKernel(FFTGeneratedStockhamAction *action);
static clfftStatus FFTGeneratedTransposeGCNActionGenerateKernel(FFTGeneratedTransposeGCNAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT);
static clfftStatus FFTGeneratedTransposeGCNActionGetWorkSizes(FFTGeneratedTransposeGCNAction *action, array_size_t *globalws, array_size_t *localws);
static clfftStatus FFTGeneratedTransposeGCNActionInitParams(FFTGeneratedTransposeGCNAction *action);
static bool FFTGeneratedTransposeGCNActionBuildForwardKernel(FFTGeneratedTransposeGCNAction *action);
static bool FFTGeneratedTransposeGCNActionBuildBackwardKernel(FFTGeneratedTransposeGCNAction *action);
static clfftStatus FFTGeneratedTransposeSquareActionGenerateKernel(FFTGeneratedTransposeSquareAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT);
static clfftStatus FFTGeneratedTransposeSquareActionGetWorkSizes(FFTGeneratedTransposeSquareAction *action, array_size_t *globalws, array_size_t *localws);
static clfftStatus FFTGeneratedTransposeSquareActionInitParams(FFTGeneratedTransposeSquareAction *action);
static bool FFTGeneratedTransposeSquareActionBuildForwardKernel(FFTGeneratedTransposeSquareAction *action);
static bool FFTGeneratedTransposeSquareActionBuildBackwardKernel(FFTGeneratedTransposeSquareAction *action);
static clfftStatus FFTGeneratedTransposeNonSquareActionGenerateKernel(FFTGeneratedTransposeNonSquareAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT);
static clfftStatus FFTGeneratedTransposeNonSquareActionGetWorkSizes(FFTGeneratedTransposeNonSquareAction *action, array_size_t *globalws, array_size_t *localws);
static clfftStatus FFTGeneratedTransposeNonSquareActionInitParams(FFTGeneratedTransposeNonSquareAction *action);
static bool FFTGeneratedTransposeNonSquareActionBuildForwardKernel(FFTGeneratedTransposeNonSquareAction *action);
static bool FFTGeneratedTransposeNonSquareActionBuildBackwardKernel(FFTGeneratedTransposeNonSquareAction *action);

static clfftGenerators FFTActionGetGenerator(const FFTAction *action);
static clfftStatus FFTActionGenerateKernel(FFTAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT);
static clfftStatus FFTActionGetWorkSizes(FFTAction *action, array_size_t *globalws, array_size_t *localws);
static const FFTKernelSignatureHeader *FFTActionGetSignatureData(FFTAction *action);
static bool FFTActionBuildForwardKernel(FFTAction *action);
static bool FFTActionBuildBackwardKernel(FFTAction *action);
static clfftStatus FFTActionSelectBufferArguments(FFTPlan *plan, cl_mem *clInputBuffers, cl_mem *clOutputBuffers, array_cl_mem *inputBuff, array_cl_mem *outputBuff);
static clfftStatus FFTActionEnqueue(FFTAction *action, clfftPlanHandle plHandle, clfftDirection dir, cl_uint numQueuesAndEvents, cl_command_queue *commQueues,
	cl_uint numWaitEvents, const cl_event *waitEvents, cl_event *outEvents, cl_mem *clInputBuffers, cl_mem *clOutputBuffers);
static clfftStatus FFTActionWriteKernel(clfftPlanHandle plHandle, clfftGenerators gen, const FFTKernelSignatureHeader *data, cl_context context, cl_device_id device);
static clfftStatus FFTActionCompileKernels(FFTAction *action, const cl_command_queue commQueueFFT, const clfftPlanHandle plHandle, FFTPlan *fftPlan);

static clfftGenerators FFTActionGetGenerator(const FFTAction *action)
{
	// Return the explicit generator field from the action data.
	return action->generator;
}

static clfftStatus FFTActionGenerateKernel(FFTAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT)
{
	// Dispatch kernel generation through the explicit action kind.
	switch (action->generator)
	{
		case Stockham: return FFTGeneratedStockhamActionGenerateKernel((FFTGeneratedStockhamAction *) action->owner, fftRepo, commQueueFFT);
		case Transpose_GCN: return FFTGeneratedTransposeGCNActionGenerateKernel((FFTGeneratedTransposeGCNAction *) action->owner, fftRepo, commQueueFFT);
		case Transpose_SQUARE: return FFTGeneratedTransposeSquareActionGenerateKernel((FFTGeneratedTransposeSquareAction *) action->owner, fftRepo, commQueueFFT);
		case Transpose_NONSQUARE: return FFTGeneratedTransposeNonSquareActionGenerateKernel((FFTGeneratedTransposeNonSquareAction *) action->owner, fftRepo, commQueueFFT);
		case Copy: return FFTGeneratedCopyActionGenerateKernel((FFTGeneratedCopyAction *) action->owner, fftRepo, commQueueFFT);
		default: return CLFFT_NOTIMPLEMENTED;
	}
}

static clfftStatus FFTActionGetWorkSizes(FFTAction *action, array_size_t *globalws, array_size_t *localws)
{
	// Dispatch work-size calculation through the explicit action kind.
	switch (action->generator)
	{
		case Stockham: return FFTGeneratedStockhamActionGetWorkSizes((FFTGeneratedStockhamAction *) action->owner, globalws, localws);
		case Transpose_GCN: return FFTGeneratedTransposeGCNActionGetWorkSizes((FFTGeneratedTransposeGCNAction *) action->owner, globalws, localws);
		case Transpose_SQUARE: return FFTGeneratedTransposeSquareActionGetWorkSizes((FFTGeneratedTransposeSquareAction *) action->owner, globalws, localws);
		case Transpose_NONSQUARE: return FFTGeneratedTransposeNonSquareActionGetWorkSizes((FFTGeneratedTransposeNonSquareAction *) action->owner, globalws, localws);
		case Copy: return FFTGeneratedCopyActionGetWorkSizes((FFTGeneratedCopyAction *) action->owner, globalws, localws);
		default: return CLFFT_NOTIMPLEMENTED;
	}
}

static const FFTKernelSignatureHeader *FFTActionGetSignatureData(FFTAction *action)
{
	// Dispatch signature access through the explicit action kind.
	switch (action->generator)
	{
		case Stockham: return &((FFTGeneratedStockhamAction *) (action->owner))->signature.header;
		case Transpose_GCN: return &((FFTGeneratedTransposeGCNAction *) (action->owner))->signature.header;
		case Transpose_SQUARE: return &((FFTGeneratedTransposeSquareAction *) (action->owner))->signature.header;
		case Transpose_NONSQUARE: return &((FFTGeneratedTransposeNonSquareAction *) (action->owner))->signature.header;
		case Copy: return &((FFTGeneratedCopyAction *) (action->owner))->signature.header;
		default: return NULL;
	}
}

static bool FFTActionBuildForwardKernel(FFTAction *action)
{
	// Dispatch forward-kernel selection through the explicit action kind.
	switch (action->generator)
	{
		case Stockham: return FFTGeneratedStockhamActionBuildForwardKernel((FFTGeneratedStockhamAction *) action->owner);
		case Transpose_GCN: return FFTGeneratedTransposeGCNActionBuildForwardKernel((FFTGeneratedTransposeGCNAction *) action->owner);
		case Transpose_SQUARE: return FFTGeneratedTransposeSquareActionBuildForwardKernel((FFTGeneratedTransposeSquareAction *) action->owner);
		case Transpose_NONSQUARE: return FFTGeneratedTransposeNonSquareActionBuildForwardKernel((FFTGeneratedTransposeNonSquareAction *) action->owner);
		case Copy: return FFTGeneratedCopyActionBuildForwardKernel((FFTGeneratedCopyAction *) action->owner);
		default: return false;
	}
}

static bool FFTActionBuildBackwardKernel(FFTAction *action)
{
	// Dispatch backward-kernel selection through the explicit action kind.
	switch (action->generator)
	{
		case Stockham: return FFTGeneratedStockhamActionBuildBackwardKernel((FFTGeneratedStockhamAction *) action->owner);
		case Transpose_GCN: return FFTGeneratedTransposeGCNActionBuildBackwardKernel((FFTGeneratedTransposeGCNAction *) action->owner);
		case Transpose_SQUARE: return FFTGeneratedTransposeSquareActionBuildBackwardKernel((FFTGeneratedTransposeSquareAction *) action->owner);
		case Transpose_NONSQUARE: return FFTGeneratedTransposeNonSquareActionBuildBackwardKernel((FFTGeneratedTransposeNonSquareAction *) action->owner);
		case Copy: return FFTGeneratedCopyActionBuildBackwardKernel((FFTGeneratedCopyAction *) action->owner);
		default: return false;
	}
}

static void FFTActionDestroy(FFTAction *action)
{
	// Destroy actions through the explicit action kind.
	if (action == NULL)
		return;

	switch (action->generator)
	{
		case Stockham: free((FFTGeneratedStockhamAction *) (action->owner)); break;
		case Transpose_GCN: free((FFTGeneratedTransposeGCNAction *) (action->owner)); break;
		case Transpose_SQUARE: free((FFTGeneratedTransposeSquareAction *) (action->owner)); break;
		case Transpose_NONSQUARE: free((FFTGeneratedTransposeNonSquareAction *) (action->owner)); break;
		case Copy: free((FFTGeneratedCopyAction *) (action->owner)); break;
		default: free(action); break;
	}
}
/* End inlined header: src\library\action.h */

/* Begin inlined header: src\library\generator.stockham.h */

#ifdef _MSC_VER
  #pragma warning(disable:4996)
#endif

typedef union
{
	cl_float f;
	cl_uint u;
	cl_int i;
} cb_t;

// Precision
typedef enum Precision
{
	P_SINGLE,
	P_DOUBLE,
} Precision;

static inline size_t StockhamPrecisionWidth(Precision pr)
{
	switch (pr)
	{
		case P_SINGLE: return 1;
		case P_DOUBLE: return 2;
		default: assert(false); return 1;
	}
}

static inline buffer_t ClPragma(Precision pr)
{
	switch (pr)
	{
		case P_SINGLE: return buffer_from_cstr("");
		case P_DOUBLE:
			return buffer_from_cstr("\n#ifdef cl_khr_fp64\n"
						"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
						"#else\n"
						"#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n"
						"#endif\n\n");
		default: assert(false); return buffer_from_cstr("");
	}
}

// Convert unsigned integers to string
static inline buffer_t SztToStr(size_t i)
{
	buffer_stream_t ss;
	// Initialize stream formatting state explicitly.
	bufstream_init(&ss);
	bufstream_cat_size(&ss, i);
	return ss.text;
}

static inline buffer_t FloatToStr(double f)
{
	buffer_stream_t ss;
	// Initialize stream formatting state explicitly.
	bufstream_init(&ss);
	ss.precision_value = 16;
	bufstream_scientific(&ss);
	bufstream_cat_double(&ss, f);
	return ss.text;
}

//	Find the smallest power of 2 that is >= n; return its power of 2 factor
//	e.g., CeilPo2 (7) returns 3 : (2^3 >= 7)
static inline size_t CeilPo2(size_t n)
{
	size_t v = 1, t = 0;
	while (v < n)
	{
		v <<= 1;
		t++;
	}

	return t;
}

static inline size_t FloorPo2(size_t n)
//	return the largest power of 2 that is <= n.
//	e.g., FloorPo2 (7) returns 4.
// *** TODO use x86 BSR instruction, using compiler intrinsics.
{
	size_t tmp;
	while (0 != (tmp = n & (n - 1)))
		n = tmp;
	return n;
}

typedef struct stringpair
{
	buffer_t first;
	buffer_t second;
} stringpair;
static inline stringpair ComplexMul(const char *type, const char *a, const char *b, bool forward)
{
	stringpair result;
	// Initialize the pair buffers before appending generated expression text.
	bufinit(&result.first);
	bufinit(&result.second);
	bufsetcstr(&result.first, "(");
	bufcatcstr(&result.first, type);
	bufcatcstr(&result.first, ") ((");
	bufcatcstr(&result.first, a);
	bufcatcstr(&result.first, ".x * ");
	bufcatcstr(&result.first, b);
	bufcatcstr(&result.first, (forward ? ".x - " : ".x + "));
	bufcatcstr(&result.first, a);
	bufcatcstr(&result.first, ".y * ");
	bufcatcstr(&result.first, b);
	bufcatcstr(&result.first, ".y),");
	bufsetcstr(&result.second, "(");
	bufcatcstr(&result.second, a);
	bufcatcstr(&result.second, ".y * ");
	bufcatcstr(&result.second, b);
	bufcatcstr(&result.second, (forward ? ".x + " : ".x - "));
	bufcatcstr(&result.second, a);
	bufcatcstr(&result.second, ".x * ");
	bufcatcstr(&result.second, b);
	bufcatcstr(&result.second, ".y))");
	return result;
}

// Register data base types
static inline buffer_t RegBaseType(Precision pr, size_t count)
{
	switch (pr)
	{
		case P_SINGLE:
			switch (count)
			{
				case 1: return buffer_from_cstr("float");
				case 2: return buffer_from_cstr("float2");
				case 4: return buffer_from_cstr("float4");
				default: assert(false); return buffer_from_cstr("");
			}
			break;
		case P_DOUBLE:
			switch (count)
			{
				case 1: return buffer_from_cstr("double");
				case 2: return buffer_from_cstr("double2");
				case 4: return buffer_from_cstr("double4");
				default: assert(false); return buffer_from_cstr("");
			}
			break;
		default: assert(false); return buffer_from_cstr("");
	}
}

static inline buffer_t FloatSuffix(Precision pr)
{
	// Suffix for constants
	buffer_t sfx = buffer_empty();
	switch (pr)
	{
		case P_SINGLE: bufsetcstr(&sfx, "f"); break;
		case P_DOUBLE: bufsetcstr(&sfx, ""); break;
		default: assert(false);
	}

	return sfx;
}

static inline buffer_t ButterflyName(size_t radix, size_t count, bool fwd)
{
	buffer_t str = buffer_empty();
	if (fwd)
		bufcatcstr(&str, "Fwd");
	else
		bufcatcstr(&str, "Inv");
	bufcatcstr(&str, "Rad");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(radix));
	bufcatcstr(&str, "B");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(count));
	return str;
}

static inline buffer_t PassName(size_t pos, bool fwd)
{
	buffer_t str = buffer_empty();
	if (fwd)
		bufcatcstr(&str, "Fwd");
	else
		bufcatcstr(&str, "Inv");
	bufcatcstr(&str, "Pass");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(pos));
	return str;
}

static inline buffer_t TwTableName(void)
{
	return buffer_from_cstr("twiddles");
}

static inline buffer_t TwTableLargeName(void)
{
	return buffer_from_cstr("twiddle_dee");
}

static inline buffer_t TwTableLargeFunc(void)
{
	return buffer_from_cstr("TW3step");
}

// Twiddle factors table for large N
// used in 3-step algorithm
typedef struct TwiddleTableLarge
{
	size_t N; // length
	size_t X, Y;
	size_t tableSize;
	double *wc, *ws; // cosine, sine arrays

} TwiddleTableLarge;

static void TwiddleTableLargeGenerateTwiddleTable(TwiddleTableLarge *table, Precision pr, buffer_t *twStr)
{
	// Generate twiddle table source from explicit table state.
	const double TWO_PI = -6.283185307179586476925286766559;

	// Generate the table
	size_t nt = 0;
	double phi = TWO_PI / (double) table->N;
	for (size_t iY = 0; iY < table->Y; ++iY)
	{
		size_t i = (size_t) 1 << (iY * CLFFT_ARBITRARY_TWIDDLE_DEE);
		for (size_t iX = 0; iX < table->X; ++iX)
		{
			size_t j = i * iX;

			double c = cos(phi * (double) j);
			double s = sin(phi * (double) j);

			// if (fabs(c) < 1.0E-12)	c = 0.0;
			// if (fabs(s) < 1.0E-12)	s = 0.0;

			table->wc[nt] = c;
			table->ws[nt++] = s;
		}
	}

	buffer_t sfx = FloatSuffix(pr);

	// Cache generated names so formatting uses explicit buffer pointers.
	buffer_t regBase2 = RegBaseType(pr, 2);
	buffer_t twTableName = TwTableLargeName();
	buffer_t twTableFunc = TwTableLargeFunc();

	// Stringize the table
	buffer_stream_t ss;
	// Initialize stream formatting state explicitly.
	bufstream_init(&ss);
	ss.precision_value = 34;
	bufstream_scientific(&ss);
	nt = 0;

	bufprintf(&ss.text, "\n __constant %s %s[%llu][%llu] = {\n", bufcstr(&regBase2), bufcstr(&twTableName), (unsigned long long) (table->Y), (unsigned long long) (table->X));
	for (size_t iY = 0; iY < table->Y; ++iY)
	{
		bufstream_cat_cstr(&ss, "{ ");
		for (size_t iX = 0; iX < table->X; ++iX)
		{
			// Preserve the original stream sequencing for the twiddle index.
			const size_t twiddleIndex = nt++;
			bufprintf(&ss.text, "(%s)(%.*e%s, %.*e%s),\n", bufcstr(&regBase2), ss.precision_value, table->wc[twiddleIndex], bufcstr(&sfx), ss.precision_value,
				table->ws[twiddleIndex], bufcstr(&sfx));
		}
		bufstream_cat_cstr(&ss, " },\n");
	}
	bufstream_cat_cstr(&ss, "};\n\n");

	// Twiddle calc function
	bufprintf(&ss.text, "__attribute__((always_inline)) %s\n%s(size_t u)\n{\n", bufcstr(&regBase2), bufcstr(&twTableFunc));
	bufprintf(&ss.text, "\tsize_t j = u & %llu;\n\t%s result = %s[0][j];\n", (unsigned long long) ((unsigned) (table->X - 1)), bufcstr(&regBase2), bufcstr(&twTableName));

	for (size_t iY = 1; iY < table->Y; ++iY)
	{
		buffer_t phasor = buffer_copy(&twTableName);
		bufcatcstr(&phasor, "[");
		BUFCAT_BUFFER_VALUE(&phasor, SztToStr(iY));
		bufcatcstr(&phasor, "][j]");

		stringpair product = ComplexMul(bufcstr(&regBase2), "result", bufcstr(&phasor), true);

		bufprintf(&ss.text, "\tu >>= %llu;\n\tj = u & %llu;\n\tresult = %s\n\t\t%s;\n", (unsigned long long) ((unsigned) (CLFFT_ARBITRARY_TWIDDLE_DEE)),
			(unsigned long long) ((unsigned) (table->X - 1)), bufcstr(&product.first), bufcstr(&product.second));
	}
	bufstream_cat_cstr(&ss,
		"\t"
		"return result;\n}\n\n");

	bufcatbuf(twStr, &ss.text);
}

static inline void TwiddleTableLargeInit(TwiddleTableLarge *table, size_t length)
{
	// Initialize large twiddle table dimensions and allocate storage.
	table->N = length;
	table->X = (size_t) 1 << CLFFT_ARBITRARY_TWIDDLE_DEE;
	table->Y = DivRoundingUpSizeT(CeilPo2(table->N), CLFFT_ARBITRARY_TWIDDLE_DEE);
	table->tableSize = table->X * table->Y;
	table->wc = (double *) clfft_checked_malloc(table->tableSize * sizeof(double));
	table->ws = (double *) clfft_checked_malloc(table->tableSize * sizeof(double));
}

static inline void TwiddleTableLargeFree(TwiddleTableLarge *table)
{
	// Release large twiddle table storage.
	if (!table)
		return;
	free(table->wc);
	free(table->ws);
	table->wc = NULL;
	table->ws = NULL;
}

// FFT butterfly
typedef struct Butterfly
{
	Precision pr; // Floating-point precision used to emit OpenCL source
	size_t radix; // Base radix
	size_t count; // Number of basic butterflies, valid values: 1,2,4
	bool fwd;     // FFT direction
	bool cReg;    // registers are complex numbers, .x (real), .y(imag)
} Butterfly;

static inline size_t ButterflyBitReverse(size_t n, size_t N)
{
	// Reverse the butterfly index recursively for power-of-two radices.
	return (N < 2) ? n : (ButterflyBitReverse(n >> 1, N >> 1) | ((n & 1) != 0 ? (N >> 1) : 0));
}

static void ButterflyGenerateButterflyStr(const Butterfly *butterfly, buffer_t *bflyStr)
{
	// Generate the butterfly source string from explicit state.
	buffer_t regType = butterfly->cReg ? RegBaseType(butterfly->pr, 2) : RegBaseType(butterfly->pr, butterfly->count);

	// Function attribute
	bufcatcstr(bflyStr, "__attribute__((always_inline)) void \n");

	// Function name
	BUFCAT_BUFFER_VALUE(bflyStr, ButterflyName(butterfly->radix, butterfly->count, butterfly->fwd));

	// Function Arguments
	bufcatcstr(bflyStr, "(");
	for (size_t i = 0;; i++)
	{
		if (butterfly->cReg)
		{
			bufcatbuf(bflyStr, &regType);
			bufcatcstr(bflyStr, " *R");
			if (butterfly->radix & (butterfly->radix - 1))
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
			else
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(ButterflyBitReverse(i, butterfly->radix)));
		}
		else
		{
			bufcatbuf(bflyStr, &regType);
			bufcatcstr(bflyStr, " *R");
			BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
			bufcatcstr(bflyStr, ", "); // real arguments
			bufcatbuf(bflyStr, &regType);
			bufcatcstr(bflyStr, " *I");
			BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i)); // imaginary arguments
		}

		if (i == butterfly->radix - 1)
		{
			bufcatcstr(bflyStr, ")");
			break;
		}
		else
		{
			bufcatcstr(bflyStr, ", ");
		}
	}

	bufcatcstr(bflyStr, "\n{\n\n");

	// Temporary variables
	// Allocate temporary variables if we are not using complex registers
	// (butterfly->cReg = 0) or if butterfly->cReg is true, then allocate
	// temporary variables only for non power-of-2 radices
	if (!((butterfly->radix == 7 && butterfly->cReg) || (butterfly->radix == 11 && butterfly->cReg) || (butterfly->radix == 13 && butterfly->cReg)))
	{
		if ((butterfly->radix & (butterfly->radix - 1)) || (!butterfly->cReg))
		{
			bufcatcstr(bflyStr, "\t");
			if (butterfly->cReg)
				BUFCAT_BUFFER_VALUE(bflyStr, RegBaseType(butterfly->pr, 1));
			else
				bufcatbuf(bflyStr, &regType);

			for (size_t i = 0;; i++)
			{
				bufcatcstr(bflyStr, " TR");
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
				bufcatcstr(bflyStr, ","); // real arguments
				bufcatcstr(bflyStr, " TI");
				BUFCAT_BUFFER_VALUE(bflyStr,
					SztToStr(i)); // imaginary arguments

				if (i == butterfly->radix - 1)
				{
					bufcatcstr(bflyStr, ";");
					break;
				}
				else
				{
					bufcatcstr(bflyStr, ",");
				}
			}
		}
		else
		{
			bufcatcstr(bflyStr, "\t");
			BUFCAT_BUFFER_VALUE(bflyStr, RegBaseType(butterfly->pr, 2));
			bufcatcstr(bflyStr, " T;");
		}
	}

	bufcatcstr(bflyStr, "\n\n\t");

	// Butterfly for different radices
	switch (butterfly->radix)
	{
		case 2:
		{
			if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"(*R1) = (*R0) - (*R1);\n\t"
					"(*R0) = 2.0f * (*R0) - (*R1);\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0) + (*R1);\n\t"
					"TI0 = (*I0) + (*I1);\n\t"
					"TR1 = (*R0) - (*R1);\n\t"
					"TI1 = (*I0) - (*I1);\n\t");
			}
		}
		break;
		case 3:
		{
			if (butterfly->fwd)
			{
				if (butterfly->cReg)
				{
					bufcatcstr(bflyStr,
						"TR0 = (*R0).x + (*R1).x + (*R2).x;\n\t"
						"TR1 = ((*R0).x - C3QA*((*R1).x + (*R2).x)) + "
						"C3QB*((*R1).y - (*R2).y);\n\t"
						"TR2 = ((*R0).x - C3QA*((*R1).x + (*R2).x)) - "
						"C3QB*((*R1).y - (*R2).y);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = (*R0).y + (*R1).y + (*R2).y;\n\t"
						"TI1 = ((*R0).y - C3QA*((*R1).y + (*R2).y)) - "
						"C3QB*((*R1).x - (*R2).x);\n\t"
						"TI2 = ((*R0).y - C3QA*((*R1).y + (*R2).y)) + "
						"C3QB*((*R1).x - (*R2).x);\n\t");
				}
				else
				{
					bufcatcstr(bflyStr,
						"TR0 = *R0 + *R1 + *R2;\n\t"
						"TR1 = (*R0 - C3QA*(*R1 + *R2)) + C3QB*(*I1 - *I2);\n\t"
						"TR2 = (*R0 - C3QA*(*R1 + *R2)) - C3QB*(*I1 - *I2);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = *I0 + *I1 + *I2;\n\t"
						"TI1 = (*I0 - C3QA*(*I1 + *I2)) - C3QB*(*R1 - *R2);\n\t"
						"TI2 = (*I0 - C3QA*(*I1 + *I2)) + C3QB*(*R1 - *R2);\n\t");
				}
			}
			else if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0).x + (*R1).x + (*R2).x;\n\t"
					"TR1 = ((*R0).x - C3QA*((*R1).x + (*R2).x)) - "
					"C3QB*((*R1).y - (*R2).y);\n\t"
					"TR2 = ((*R0).x - C3QA*((*R1).x + (*R2).x)) + "
					"C3QB*((*R1).y - (*R2).y);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = (*R0).y + (*R1).y + (*R2).y;\n\t"
					"TI1 = ((*R0).y - C3QA*((*R1).y + (*R2).y)) + "
					"C3QB*((*R1).x - (*R2).x);\n\t"
					"TI2 = ((*R0).y - C3QA*((*R1).y + (*R2).y)) - "
					"C3QB*((*R1).x - (*R2).x);\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = *R0 + *R1 + *R2;\n\t"
					"TR1 = (*R0 - C3QA*(*R1 + *R2)) - C3QB*(*I1 - *I2);\n\t"
					"TR2 = (*R0 - C3QA*(*R1 + *R2)) + C3QB*(*I1 - *I2);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = *I0 + *I1 + *I2;\n\t"
					"TI1 = (*I0 - C3QA*(*I1 + *I2)) + C3QB*(*R1 - *R2);\n\t"
					"TI2 = (*I0 - C3QA*(*I1 + *I2)) - C3QB*(*R1 - *R2);\n\t");
			}
		}
		break;
		case 4:
		{
			if (butterfly->fwd)
			{
				if (butterfly->cReg)
				{
					bufcatcstr(bflyStr,
						"(*R1) = (*R0) - (*R1);\n\t"
						"(*R0) = 2.0f * (*R0) - (*R1);\n\t"
						"(*R3) = (*R2) - (*R3);\n\t"
						"(*R2) = 2.0f * (*R2) - (*R3);\n\t"
						"\n\t"
						"(*R2) = (*R0) - (*R2);\n\t"
						"(*R0) = 2.0f * (*R0) - (*R2);\n\t"
						"(*R3) = (*R1) + (fvect2)(-(*R3).y, (*R3).x);\n\t"
						"(*R1) = 2.0f * (*R1) - (*R3);\n\t");
				}
				else
				{
					bufcatcstr(bflyStr,
						"TR0 = (*R0) + (*R2) + (*R1) + (*R3);\n\t"
						"TR1 = (*R0) - (*R2) + (*I1) - (*I3);\n\t"
						"TR2 = (*R0) + (*R2) - (*R1) - (*R3);\n\t"
						"TR3 = (*R0) - (*R2) - (*I1) + (*I3);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = (*I0) + (*I2) + (*I1) + (*I3);\n\t"
						"TI1 = (*I0) - (*I2) - (*R1) + (*R3);\n\t"
						"TI2 = (*I0) + (*I2) - (*I1) - (*I3);\n\t"
						"TI3 = (*I0) - (*I2) + (*R1) - (*R3);\n\t");
				}
			}
			else if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"(*R1) = (*R0) - (*R1);\n\t"
					"(*R0) = 2.0f * (*R0) - (*R1);\n\t"
					"(*R3) = (*R2) - (*R3);\n\t"
					"(*R2) = 2.0f * (*R2) - (*R3);\n\t"
					"\n\t"
					"(*R2) = (*R0) - (*R2);\n\t"
					"(*R0) = 2.0f * (*R0) - (*R2);\n\t"
					"(*R3) = (*R1) + (fvect2)((*R3).y, -(*R3).x);\n\t"
					"(*R1) = 2.0f * (*R1) - (*R3);\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0) + (*R2) + (*R1) + (*R3);\n\t"
					"TR1 = (*R0) - (*R2) - (*I1) + (*I3);\n\t"
					"TR2 = (*R0) + (*R2) - (*R1) - (*R3);\n\t"
					"TR3 = (*R0) - (*R2) + (*I1) - (*I3);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = (*I0) + (*I2) + (*I1) + (*I3);\n\t"
					"TI1 = (*I0) - (*I2) + (*R1) - (*R3);\n\t"
					"TI2 = (*I0) + (*I2) - (*I1) - (*I3);\n\t"
					"TI3 = (*I0) - (*I2) - (*R1) + (*R3);\n\t");
			}
		}
		break;
		case 5:
		{
			if (butterfly->fwd)
			{
				if (butterfly->cReg)
				{
					bufcatcstr(bflyStr,
						"TR0 = (*R0).x + (*R1).x + (*R2).x + (*R3).x + (*R4).x;\n\t"
						"TR1 = ((*R0).x - C5QC*((*R2).x + (*R3).x)) + "
						"C5QB*((*R1).y - (*R4).y) + C5QD*((*R2).y - (*R3).y) + "
						"C5QA*(((*R1).x - (*R2).x) + ((*R4).x - (*R3).x));\n\t"
						"TR4 = ((*R0).x - C5QC*((*R2).x + (*R3).x)) - "
						"C5QB*((*R1).y - (*R4).y) - C5QD*((*R2).y - (*R3).y) + "
						"C5QA*(((*R1).x - (*R2).x) + ((*R4).x - (*R3).x));\n\t"
						"TR2 = ((*R0).x - C5QC*((*R1).x + (*R4).x)) - "
						"C5QB*((*R2).y - (*R3).y) + C5QD*((*R1).y - (*R4).y) + "
						"C5QA*(((*R2).x - (*R1).x) + ((*R3).x - (*R4).x));\n\t"
						"TR3 = ((*R0).x - C5QC*((*R1).x + (*R4).x)) + "
						"C5QB*((*R2).y - (*R3).y) - C5QD*((*R1).y - (*R4).y) + "
						"C5QA*(((*R2).x - (*R1).x) + ((*R3).x - (*R4).x));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = (*R0).y + (*R1).y + (*R2).y + (*R3).y + (*R4).y;\n\t"
						"TI1 = ((*R0).y - C5QC*((*R2).y + (*R3).y)) - "
						"C5QB*((*R1).x - (*R4).x) - C5QD*((*R2).x - (*R3).x) + "
						"C5QA*(((*R1).y - (*R2).y) + ((*R4).y - (*R3).y));\n\t"
						"TI4 = ((*R0).y - C5QC*((*R2).y + (*R3).y)) + "
						"C5QB*((*R1).x - (*R4).x) + C5QD*((*R2).x - (*R3).x) + "
						"C5QA*(((*R1).y - (*R2).y) + ((*R4).y - (*R3).y));\n\t"
						"TI2 = ((*R0).y - C5QC*((*R1).y + (*R4).y)) + "
						"C5QB*((*R2).x - (*R3).x) - C5QD*((*R1).x - (*R4).x) + "
						"C5QA*(((*R2).y - (*R1).y) + ((*R3).y - (*R4).y));\n\t"
						"TI3 = ((*R0).y - C5QC*((*R1).y + (*R4).y)) - "
						"C5QB*((*R2).x - (*R3).x) + C5QD*((*R1).x - (*R4).x) + "
						"C5QA*(((*R2).y - (*R1).y) + ((*R3).y - (*R4).y));\n\t");
				}
				else
				{
					bufcatcstr(bflyStr,
						"TR0 = *R0 + *R1 + *R2 + *R3 + *R4;\n\t"
						"TR1 = (*R0 - C5QC*(*R2 + *R3)) + C5QB*(*I1 - *I4) + "
						"C5QD*(*I2 - *I3) + C5QA*((*R1 - *R2) + (*R4 - *R3));\n\t"
						"TR4 = (*R0 - C5QC*(*R2 + *R3)) - C5QB*(*I1 - *I4) - "
						"C5QD*(*I2 - *I3) + C5QA*((*R1 - *R2) + (*R4 - *R3));\n\t"
						"TR2 = (*R0 - C5QC*(*R1 + *R4)) - C5QB*(*I2 - *I3) + "
						"C5QD*(*I1 - *I4) + C5QA*((*R2 - *R1) + (*R3 - *R4));\n\t"
						"TR3 = (*R0 - C5QC*(*R1 + *R4)) + C5QB*(*I2 - *I3) - "
						"C5QD*(*I1 - *I4) + C5QA*((*R2 - *R1) + (*R3 - *R4));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = *I0 + *I1 + *I2 + *I3 + *I4;\n\t"
						"TI1 = (*I0 - C5QC*(*I2 + *I3)) - C5QB*(*R1 - *R4) - "
						"C5QD*(*R2 - *R3) + C5QA*((*I1 - *I2) + (*I4 - *I3));\n\t"
						"TI4 = (*I0 - C5QC*(*I2 + *I3)) + C5QB*(*R1 - *R4) + "
						"C5QD*(*R2 - *R3) + C5QA*((*I1 - *I2) + (*I4 - *I3));\n\t"
						"TI2 = (*I0 - C5QC*(*I1 + *I4)) + C5QB*(*R2 - *R3) - "
						"C5QD*(*R1 - *R4) + C5QA*((*I2 - *I1) + (*I3 - *I4));\n\t"
						"TI3 = (*I0 - C5QC*(*I1 + *I4)) - C5QB*(*R2 - *R3) + "
						"C5QD*(*R1 - *R4) + C5QA*((*I2 - *I1) + (*I3 - *I4));\n\t");
				}
			}
			else if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0).x + (*R1).x + (*R2).x + (*R3).x + (*R4).x;\n\t"
					"TR1 = ((*R0).x - C5QC*((*R2).x + (*R3).x)) - C5QB*((*R1).y - "
					"(*R4).y) - C5QD*((*R2).y - (*R3).y) + C5QA*(((*R1).x - "
					"(*R2).x) + ((*R4).x - (*R3).x));\n\t"
					"TR4 = ((*R0).x - C5QC*((*R2).x + (*R3).x)) + C5QB*((*R1).y - "
					"(*R4).y) + C5QD*((*R2).y - (*R3).y) + C5QA*(((*R1).x - "
					"(*R2).x) + ((*R4).x - (*R3).x));\n\t"
					"TR2 = ((*R0).x - C5QC*((*R1).x + (*R4).x)) + C5QB*((*R2).y - "
					"(*R3).y) - C5QD*((*R1).y - (*R4).y) + C5QA*(((*R2).x - "
					"(*R1).x) + ((*R3).x - (*R4).x));\n\t"
					"TR3 = ((*R0).x - C5QC*((*R1).x + (*R4).x)) - C5QB*((*R2).y - "
					"(*R3).y) + C5QD*((*R1).y - (*R4).y) + C5QA*(((*R2).x - "
					"(*R1).x) + ((*R3).x - (*R4).x));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = (*R0).y + (*R1).y + (*R2).y + (*R3).y + (*R4).y;\n\t"
					"TI1 = ((*R0).y - C5QC*((*R2).y + (*R3).y)) + C5QB*((*R1).x - "
					"(*R4).x) + C5QD*((*R2).x - (*R3).x) + C5QA*(((*R1).y - "
					"(*R2).y) + ((*R4).y - (*R3).y));\n\t"
					"TI4 = ((*R0).y - C5QC*((*R2).y + (*R3).y)) - C5QB*((*R1).x - "
					"(*R4).x) - C5QD*((*R2).x - (*R3).x) + C5QA*(((*R1).y - "
					"(*R2).y) + ((*R4).y - (*R3).y));\n\t"
					"TI2 = ((*R0).y - C5QC*((*R1).y + (*R4).y)) - C5QB*((*R2).x - "
					"(*R3).x) + C5QD*((*R1).x - (*R4).x) + C5QA*(((*R2).y - "
					"(*R1).y) + ((*R3).y - (*R4).y));\n\t"
					"TI3 = ((*R0).y - C5QC*((*R1).y + (*R4).y)) + C5QB*((*R2).x - "
					"(*R3).x) - C5QD*((*R1).x - (*R4).x) + C5QA*(((*R2).y - "
					"(*R1).y) + ((*R3).y - (*R4).y));\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = *R0 + *R1 + *R2 + *R3 + *R4;\n\t"
					"TR1 = (*R0 - C5QC*(*R2 + *R3)) - C5QB*(*I1 - *I4) - C5QD*(*I2 "
					"- *I3) + C5QA*((*R1 - *R2) + (*R4 - *R3));\n\t"
					"TR4 = (*R0 - C5QC*(*R2 + *R3)) + C5QB*(*I1 - *I4) + C5QD*(*I2 "
					"- *I3) + C5QA*((*R1 - *R2) + (*R4 - *R3));\n\t"
					"TR2 = (*R0 - C5QC*(*R1 + *R4)) + C5QB*(*I2 - *I3) - C5QD*(*I1 "
					"- *I4) + C5QA*((*R2 - *R1) + (*R3 - *R4));\n\t"
					"TR3 = (*R0 - C5QC*(*R1 + *R4)) - C5QB*(*I2 - *I3) + C5QD*(*I1 "
					"- *I4) + C5QA*((*R2 - *R1) + (*R3 - *R4));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = *I0 + *I1 + *I2 + *I3 + *I4;\n\t"
					"TI1 = (*I0 - C5QC*(*I2 + *I3)) + C5QB*(*R1 - *R4) + C5QD*(*R2 "
					"- *R3) + C5QA*((*I1 - *I2) + (*I4 - *I3));\n\t"
					"TI4 = (*I0 - C5QC*(*I2 + *I3)) - C5QB*(*R1 - *R4) - C5QD*(*R2 "
					"- *R3) + C5QA*((*I1 - *I2) + (*I4 - *I3));\n\t"
					"TI2 = (*I0 - C5QC*(*I1 + *I4)) - C5QB*(*R2 - *R3) + C5QD*(*R1 "
					"- *R4) + C5QA*((*I2 - *I1) + (*I3 - *I4));\n\t"
					"TI3 = (*I0 - C5QC*(*I1 + *I4)) + C5QB*(*R2 - *R3) - C5QD*(*R1 "
					"- *R4) + C5QA*((*I2 - *I1) + (*I3 - *I4));\n\t");
			}
		}
		break;
		case 6:
		{
			if (butterfly->fwd)
			{
				if (butterfly->cReg)
				{
					bufcatcstr(bflyStr,
						"TR0 = (*R0).x + (*R2).x + (*R4).x;\n\t"
						"TR2 = ((*R0).x - C3QA*((*R2).x + (*R4).x)) + "
						"C3QB*((*R2).y - (*R4).y);\n\t"
						"TR4 = ((*R0).x - C3QA*((*R2).x + (*R4).x)) - "
						"C3QB*((*R2).y - (*R4).y);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = (*R0).y + (*R2).y + (*R4).y;\n\t"
						"TI2 = ((*R0).y - C3QA*((*R2).y + (*R4).y)) - "
						"C3QB*((*R2).x - (*R4).x);\n\t"
						"TI4 = ((*R0).y - C3QA*((*R2).y + (*R4).y)) + "
						"C3QB*((*R2).x - (*R4).x);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TR1 = (*R1).x + (*R3).x + (*R5).x;\n\t"
						"TR3 = ((*R1).x - C3QA*((*R3).x + (*R5).x)) + "
						"C3QB*((*R3).y - (*R5).y);\n\t"
						"TR5 = ((*R1).x - C3QA*((*R3).x + (*R5).x)) - "
						"C3QB*((*R3).y - (*R5).y);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI1 = (*R1).y + (*R3).y + (*R5).y;\n\t"
						"TI3 = ((*R1).y - C3QA*((*R3).y + (*R5).y)) - "
						"C3QB*((*R3).x - (*R5).x);\n\t"
						"TI5 = ((*R1).y - C3QA*((*R3).y + (*R5).y)) + "
						"C3QB*((*R3).x - (*R5).x);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R0).x = TR0 + TR1;\n\t"
						"(*R1).x = TR2 + ( C3QA*TR3 + C3QB*TI3);\n\t"
						"(*R2).x = TR4 + (-C3QA*TR5 + C3QB*TI5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R0).y = TI0 + TI1;\n\t"
						"(*R1).y = TI2 + (-C3QB*TR3 + C3QA*TI3);\n\t"
						"(*R2).y = TI4 + (-C3QB*TR5 - C3QA*TI5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R3).x = TR0 - TR1;\n\t"
						"(*R4).x = TR2 - ( C3QA*TR3 + C3QB*TI3);\n\t"
						"(*R5).x = TR4 - (-C3QA*TR5 + C3QB*TI5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R3).y = TI0 - TI1;\n\t"
						"(*R4).y = TI2 - (-C3QB*TR3 + C3QA*TI3);\n\t"
						"(*R5).y = TI4 - (-C3QB*TR5 - C3QA*TI5);\n\t");
				}
				else
				{
					bufcatcstr(bflyStr,
						"TR0 = *R0 + *R2 + *R4;\n\t"
						"TR2 = (*R0 - C3QA*(*R2 + *R4)) + C3QB*(*I2 - *I4);\n\t"
						"TR4 = (*R0 - C3QA*(*R2 + *R4)) - C3QB*(*I2 - *I4);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = *I0 + *I2 + *I4;\n\t"
						"TI2 = (*I0 - C3QA*(*I2 + *I4)) - C3QB*(*R2 - *R4);\n\t"
						"TI4 = (*I0 - C3QA*(*I2 + *I4)) + C3QB*(*R2 - *R4);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TR1 = *R1 + *R3 + *R5;\n\t"
						"TR3 = (*R1 - C3QA*(*R3 + *R5)) + C3QB*(*I3 - *I5);\n\t"
						"TR5 = (*R1 - C3QA*(*R3 + *R5)) - C3QB*(*I3 - *I5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI1 = *I1 + *I3 + *I5;\n\t"
						"TI3 = (*I1 - C3QA*(*I3 + *I5)) - C3QB*(*R3 - *R5);\n\t"
						"TI5 = (*I1 - C3QA*(*I3 + *I5)) + C3QB*(*R3 - *R5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R0) = TR0 + TR1;\n\t"
						"(*R1) = TR2 + ( C3QA*TR3 + C3QB*TI3);\n\t"
						"(*R2) = TR4 + (-C3QA*TR5 + C3QB*TI5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*I0) = TI0 + TI1;\n\t"
						"(*I1) = TI2 + (-C3QB*TR3 + C3QA*TI3);\n\t"
						"(*I2) = TI4 + (-C3QB*TR5 - C3QA*TI5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R3) = TR0 - TR1;\n\t"
						"(*R4) = TR2 - ( C3QA*TR3 + C3QB*TI3);\n\t"
						"(*R5) = TR4 - (-C3QA*TR5 + C3QB*TI5);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*I3) = TI0 - TI1;\n\t"
						"(*I4) = TI2 - (-C3QB*TR3 + C3QA*TI3);\n\t"
						"(*I5) = TI4 - (-C3QB*TR5 - C3QA*TI5);\n\t");
				}
			}
			else if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0).x + (*R2).x + (*R4).x;\n\t"
					"TR2 = ((*R0).x - C3QA*((*R2).x + (*R4).x)) - "
					"C3QB*((*R2).y - (*R4).y);\n\t"
					"TR4 = ((*R0).x - C3QA*((*R2).x + (*R4).x)) + "
					"C3QB*((*R2).y - (*R4).y);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = (*R0).y + (*R2).y + (*R4).y;\n\t"
					"TI2 = ((*R0).y - C3QA*((*R2).y + (*R4).y)) + "
					"C3QB*((*R2).x - (*R4).x);\n\t"
					"TI4 = ((*R0).y - C3QA*((*R2).y + (*R4).y)) - "
					"C3QB*((*R2).x - (*R4).x);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TR1 = (*R1).x + (*R3).x + (*R5).x;\n\t"
					"TR3 = ((*R1).x - C3QA*((*R3).x + (*R5).x)) - "
					"C3QB*((*R3).y - (*R5).y);\n\t"
					"TR5 = ((*R1).x - C3QA*((*R3).x + (*R5).x)) + "
					"C3QB*((*R3).y - (*R5).y);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI1 = (*R1).y + (*R3).y + (*R5).y;\n\t"
					"TI3 = ((*R1).y - C3QA*((*R3).y + (*R5).y)) + "
					"C3QB*((*R3).x - (*R5).x);\n\t"
					"TI5 = ((*R1).y - C3QA*((*R3).y + (*R5).y)) - "
					"C3QB*((*R3).x - (*R5).x);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R0).x = TR0 + TR1;\n\t"
					"(*R1).x = TR2 + ( C3QA*TR3 - C3QB*TI3);\n\t"
					"(*R2).x = TR4 + (-C3QA*TR5 - C3QB*TI5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R0).y = TI0 + TI1;\n\t"
					"(*R1).y = TI2 + ( C3QB*TR3 + C3QA*TI3);\n\t"
					"(*R2).y = TI4 + ( C3QB*TR5 - C3QA*TI5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R3).x = TR0 - TR1;\n\t"
					"(*R4).x = TR2 - ( C3QA*TR3 - C3QB*TI3);\n\t"
					"(*R5).x = TR4 - (-C3QA*TR5 - C3QB*TI5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R3).y = TI0 - TI1;\n\t"
					"(*R4).y = TI2 - ( C3QB*TR3 + C3QA*TI3);\n\t"
					"(*R5).y = TI4 - ( C3QB*TR5 - C3QA*TI5);\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = *R0 + *R2 + *R4;\n\t"
					"TR2 = (*R0 - C3QA*(*R2 + *R4)) - C3QB*(*I2 - *I4);\n\t"
					"TR4 = (*R0 - C3QA*(*R2 + *R4)) + C3QB*(*I2 - *I4);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = *I0 + *I2 + *I4;\n\t"
					"TI2 = (*I0 - C3QA*(*I2 + *I4)) + C3QB*(*R2 - *R4);\n\t"
					"TI4 = (*I0 - C3QA*(*I2 + *I4)) - C3QB*(*R2 - *R4);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TR1 = *R1 + *R3 + *R5;\n\t"
					"TR3 = (*R1 - C3QA*(*R3 + *R5)) - C3QB*(*I3 - *I5);\n\t"
					"TR5 = (*R1 - C3QA*(*R3 + *R5)) + C3QB*(*I3 - *I5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI1 = *I1 + *I3 + *I5;\n\t"
					"TI3 = (*I1 - C3QA*(*I3 + *I5)) + C3QB*(*R3 - *R5);\n\t"
					"TI5 = (*I1 - C3QA*(*I3 + *I5)) - C3QB*(*R3 - *R5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R0) = TR0 + TR1;\n\t"
					"(*R1) = TR2 + ( C3QA*TR3 - C3QB*TI3);\n\t"
					"(*R2) = TR4 + (-C3QA*TR5 - C3QB*TI5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*I0) = TI0 + TI1;\n\t"
					"(*I1) = TI2 + ( C3QB*TR3 + C3QA*TI3);\n\t"
					"(*I2) = TI4 + ( C3QB*TR5 - C3QA*TI5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R3) = TR0 - TR1;\n\t"
					"(*R4) = TR2 - ( C3QA*TR3 - C3QB*TI3);\n\t"
					"(*R5) = TR4 - (-C3QA*TR5 - C3QB*TI5);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*I3) = TI0 - TI1;\n\t"
					"(*I4) = TI2 - ( C3QB*TR3 + C3QA*TI3);\n\t"
					"(*I5) = TI4 - ( C3QB*TR5 - C3QA*TI5);\n\t");
			}
		}
		break;
		case 7:
		{
			static const char *C7SFR = "\
					/*FFT7 Forward Real */ \n\
					\n\
						pr0 = *R1 + *R6; \n\
						pi0 = *I1 + *I6; \n\
						pr1 = *R1 - *R6; \n\
						pi1 = *I1 - *I6; \n\
						pr2 = *R2 + *R5; \n\
						pi2 = *I2 + *I5; \n\
						pr3 = *R2 - *R5; \n\
						pi3 = *I2 - *I5; \n\
						pr4 = *R4 + *R3; \n\
						pi4 = *I4 + *I3; \n\
						pr5 = *R4 - *R3; \n\
						pi5 = *I4 - *I3; \n\
					\n\
						pr6 = pr2 + pr0; \n\
						pi6 = pi2 + pi0; \n\
						qr4 = pr2 - pr0; \n\
						qi4 = pi2 - pi0; \n\
						qr2 = pr0 - pr4; \n\
						qi2 = pi0 - pi4; \n\
						qr3 = pr4 - pr2; \n\
						qi3 = pi4 - pi2; \n\
						pr7 = pr5 + pr3; \n\
						pi7 = pi5 + pi3; \n\
						qr7 = pr5 - pr3; \n\
						qi7 = pi5 - pi3; \n\
						qr6 = pr1 - pr5; \n\
						qi6 = pi1 - pi5; \n\
						qr8 = pr3 - pr1; \n\
						qi8 = pi3 - pi1; \n\
						qr1 = pr6 + pr4; \n\
						qi1 = pi6 + pi4; \n\
						qr5 = pr7 + pr1; \n\
						qi5 = pi7 + pi1; \n\
						qr0 = *R0 + qr1; \n\
						qi0 = *I0 + qi1; \n\
					\n\
						qr1 *= C7Q1; \n\
						qi1 *= C7Q1; \n\
						qr2 *= C7Q2; \n\
						qi2 *= C7Q2; \n\
						qr3 *= C7Q3; \n\
						qi3 *= C7Q3; \n\
						qr4 *= C7Q4; \n\
						qi4 *= C7Q4; \n\
					\n\
						qr5 *= (C7Q5); \n\
						qi5 *= (C7Q5); \n\
						qr6 *= (C7Q6); \n\
						qi6 *= (C7Q6); \n\
						qr7 *= (C7Q7); \n\
						qi7 *= (C7Q7); \n\
						qr8 *= (C7Q8); \n\
						qi8 *= (C7Q8); \n\
					\n\
						pr0 =  qr0 + qr1; \n\
						pi0 =  qi0 + qi1; \n\
						pr1 =  qr2 + qr3; \n\
						pi1 =  qi2 + qi3; \n\
						pr2 =  qr4 - qr3; \n\
						pi2 =  qi4 - qi3; \n\
						pr3 = -qr2 - qr4; \n\
						pi3 = -qi2 - qi4; \n\
						pr4 =  qr6 + qr7; \n\
						pi4 =  qi6 + qi7; \n\
						pr5 =  qr8 - qr7; \n\
						pi5 =  qi8 - qi7; \n\
						pr6 = -qr8 - qr6; \n\
						pi6 = -qi8 - qi6; \n\
						pr7 =  pr0 + pr1; \n\
						pi7 =  pi0 + pi1; \n\
						pr8 =  pr0 + pr2; \n\
						pi8 =  pi0 + pi2; \n\
						pr9 =  pr0 + pr3; \n\
						pi9 =  pi0 + pi3; \n\
						qr6 =  pr4 + qr5; \n\
						qi6 =  pi4 + qi5; \n\
						qr7 =  pr5 + qr5; \n\
						qi7 =  pi5 + qi5; \n\
						qr8 =  pr6 + qr5; \n\
						qi8 =  pi6 + qi5; \n\
					\n\
						TR0 = qr0; TI0 = qi0; \n\
						TR1 = pr7 + qi6; \n\
						TI1 = pi7 - qr6; \n\
						TR2 = pr9 + qi8; \n\
						TI2 = pi9 - qr8; \n\
						TR3 = pr8 - qi7; \n\
						TI3 = pi8 + qr7; \n\
						TR4 = pr8 + qi7; \n\
						TI4 = pi8 - qr7; \n\
						TR5 = pr9 - qi8; \n\
						TI5 = pi9 + qr8; \n\
						TR6 = pr7 - qi6; \n\
						TI6 = pi7 + qr6; \n\
					";

			static const char *C7SBR = "\
					/*FFT7 Backward Real */ \n\
					\n\
						pr0 = *R1 + *R6; \n\
						pi0 = *I1 + *I6; \n\
						pr1 = *R1 - *R6; \n\
						pi1 = *I1 - *I6; \n\
						pr2 = *R2 + *R5; \n\
						pi2 = *I2 + *I5; \n\
						pr3 = *R2 - *R5; \n\
						pi3 = *I2 - *I5; \n\
						pr4 = *R4 + *R3; \n\
						pi4 = *I4 + *I3; \n\
						pr5 = *R4 - *R3; \n\
						pi5 = *I4 - *I3; \n\
					\n\
						pr6 = pr2 + pr0; \n\
						pi6 = pi2 + pi0; \n\
						qr4 = pr2 - pr0; \n\
						qi4 = pi2 - pi0; \n\
						qr2 = pr0 - pr4; \n\
						qi2 = pi0 - pi4; \n\
						qr3 = pr4 - pr2; \n\
						qi3 = pi4 - pi2; \n\
						pr7 = pr5 + pr3; \n\
						pi7 = pi5 + pi3; \n\
						qr7 = pr5 - pr3; \n\
						qi7 = pi5 - pi3; \n\
						qr6 = pr1 - pr5; \n\
						qi6 = pi1 - pi5; \n\
						qr8 = pr3 - pr1; \n\
						qi8 = pi3 - pi1; \n\
						qr1 = pr6 + pr4; \n\
						qi1 = pi6 + pi4; \n\
						qr5 = pr7 + pr1; \n\
						qi5 = pi7 + pi1; \n\
						qr0 = *R0 + qr1; \n\
						qi0 = *I0 + qi1; \n\
					\n\
						qr1 *= C7Q1; \n\
						qi1 *= C7Q1; \n\
						qr2 *= C7Q2; \n\
						qi2 *= C7Q2; \n\
						qr3 *= C7Q3; \n\
						qi3 *= C7Q3; \n\
						qr4 *= C7Q4; \n\
						qi4 *= C7Q4; \n\
					\n\
						qr5 *= -(C7Q5); \n\
						qi5 *= -(C7Q5); \n\
						qr6 *= -(C7Q6); \n\
						qi6 *= -(C7Q6); \n\
						qr7 *= -(C7Q7); \n\
						qi7 *= -(C7Q7); \n\
						qr8 *= -(C7Q8); \n\
						qi8 *= -(C7Q8); \n\
					\n\
						pr0 =  qr0 + qr1; \n\
						pi0 =  qi0 + qi1; \n\
						pr1 =  qr2 + qr3; \n\
						pi1 =  qi2 + qi3; \n\
						pr2 =  qr4 - qr3; \n\
						pi2 =  qi4 - qi3; \n\
						pr3 = -qr2 - qr4; \n\
						pi3 = -qi2 - qi4; \n\
						pr4 =  qr6 + qr7; \n\
						pi4 =  qi6 + qi7; \n\
						pr5 =  qr8 - qr7; \n\
						pi5 =  qi8 - qi7; \n\
						pr6 = -qr8 - qr6; \n\
						pi6 = -qi8 - qi6; \n\
						pr7 =  pr0 + pr1; \n\
						pi7 =  pi0 + pi1; \n\
						pr8 =  pr0 + pr2; \n\
						pi8 =  pi0 + pi2; \n\
						pr9 =  pr0 + pr3; \n\
						pi9 =  pi0 + pi3; \n\
						qr6 =  pr4 + qr5; \n\
						qi6 =  pi4 + qi5; \n\
						qr7 =  pr5 + qr5; \n\
						qi7 =  pi5 + qi5; \n\
						qr8 =  pr6 + qr5; \n\
						qi8 =  pi6 + qi5; \n\
					\n\
						TR0 = qr0; TI0 = qi0; \n\
						TR1 = pr7 + qi6; \n\
						TI1 = pi7 - qr6; \n\
						TR2 = pr9 + qi8; \n\
						TI2 = pi9 - qr8; \n\
						TR3 = pr8 - qi7; \n\
						TI3 = pi8 + qr7; \n\
						TR4 = pr8 + qi7; \n\
						TI4 = pi8 - qr7; \n\
						TR5 = pr9 - qi8; \n\
						TI5 = pi9 + qr8; \n\
						TR6 = pr7 - qi6; \n\
						TI6 = pi7 + qr6; \n\
					";

			static const char *C7SFC = "\
					/*FFT7 Forward Complex */ \n\
					\n\
						p0 = *R1 + *R6; \n\
						p1 = *R1 - *R6; \n\
						p2 = *R2 + *R5; \n\
						p3 = *R2 - *R5; \n\
						p4 = *R4 + *R3; \n\
						p5 = *R4 - *R3; \n\
					\n\
						p6 = p2 + p0; \n\
						q4 = p2 - p0; \n\
						q2 = p0 - p4; \n\
						q3 = p4 - p2; \n\
						p7 = p5 + p3; \n\
						q7 = p5 - p3; \n\
						q6 = p1 - p5; \n\
						q8 = p3 - p1; \n\
						q1 = p6 + p4; \n\
						q5 = p7 + p1; \n\
						q0 = *R0 + q1; \n\
					\n\
						q1 *= C7Q1; \n\
						q2 *= C7Q2; \n\
						q3 *= C7Q3; \n\
						q4 *= C7Q4; \n\
					\n\
						q5 *= (C7Q5); \n\
						q6 *= (C7Q6); \n\
						q7 *= (C7Q7); \n\
						q8 *= (C7Q8); \n\
					\n\
						p0 = q0 + q1; \n\
						p1 = q2 + q3; \n\
						p2 = q4 - q3; \n\
						p3 = -q2 - q4; \n\
						p4 = q6 + q7; \n\
						p5 = q8 - q7; \n\
						p6 = -q8 - q6; \n\
						p7 = p0 + p1; \n\
						p8 = p0 + p2; \n\
						p9 = p0 + p3; \n\
						q6 = p4 + q5; \n\
						q7 = p5 + q5; \n\
						q8 = p6 + q5; \n\
					\n\
						*R0 = q0; \n\
						(*R1).x = p7.x + q6.y; \n\
						(*R1).y = p7.y - q6.x; \n\
						(*R2).x = p9.x + q8.y; \n\
						(*R2).y = p9.y - q8.x; \n\
						(*R3).x = p8.x - q7.y; \n\
						(*R3).y = p8.y + q7.x; \n\
						(*R4).x = p8.x + q7.y; \n\
						(*R4).y = p8.y - q7.x; \n\
						(*R5).x = p9.x - q8.y; \n\
						(*R5).y = p9.y + q8.x; \n\
						(*R6).x = p7.x - q6.y; \n\
						(*R6).y = p7.y + q6.x; \n\
					";

			static const char *C7SBC = "\
					/*FFT7 Backward Complex */ \n\
					\n\
						p0 = *R1 + *R6; \n\
						p1 = *R1 - *R6; \n\
						p2 = *R2 + *R5; \n\
						p3 = *R2 - *R5; \n\
						p4 = *R4 + *R3; \n\
						p5 = *R4 - *R3; \n\
					\n\
						p6 = p2 + p0; \n\
						q4 = p2 - p0; \n\
						q2 = p0 - p4; \n\
						q3 = p4 - p2; \n\
						p7 = p5 + p3; \n\
						q7 = p5 - p3; \n\
						q6 = p1 - p5; \n\
						q8 = p3 - p1; \n\
						q1 = p6 + p4; \n\
						q5 = p7 + p1; \n\
						q0 = *R0 + q1; \n\
					\n\
						q1 *= C7Q1; \n\
						q2 *= C7Q2; \n\
						q3 *= C7Q3; \n\
						q4 *= C7Q4; \n\
					\n\
						q5 *= -(C7Q5); \n\
						q6 *= -(C7Q6); \n\
						q7 *= -(C7Q7); \n\
						q8 *= -(C7Q8); \n\
					\n\
						p0 = q0 + q1; \n\
						p1 = q2 + q3; \n\
						p2 = q4 - q3; \n\
						p3 = -q2 - q4; \n\
						p4 = q6 + q7; \n\
						p5 = q8 - q7; \n\
						p6 = -q8 - q6; \n\
						p7 = p0 + p1; \n\
						p8 = p0 + p2; \n\
						p9 = p0 + p3; \n\
						q6 = p4 + q5; \n\
						q7 = p5 + q5; \n\
						q8 = p6 + q5; \n\
					\n\
						*R0 = q0; \n\
						(*R1).x = p7.x + q6.y; \n\
						(*R1).y = p7.y - q6.x; \n\
						(*R2).x = p9.x + q8.y; \n\
						(*R2).y = p9.y - q8.x; \n\
						(*R3).x = p8.x - q7.y; \n\
						(*R3).y = p8.y + q7.x; \n\
						(*R4).x = p8.x + q7.y; \n\
						(*R4).y = p8.y - q7.x; \n\
						(*R5).x = p9.x - q8.y; \n\
						(*R5).y = p9.y + q8.x; \n\
						(*R6).x = p7.x - q6.y; \n\
						(*R6).y = p7.y + q6.x; \n\
					";

			if (!butterfly->cReg)
			{
				for (size_t i = 0; i < 10; i++)
				{
					// Build the pr/pi register declaration without temporary merge
					// helpers.
					buffer_t index = SztToStr(i);
					buffer_t decl = buffer_empty();
					bufsetbuf(&decl, &regType);
					bufcatcstr(&decl, " pr");
					bufcatbuf(&decl, &index);
					bufcatcstr(&decl, ", pi");
					bufcatbuf(&decl, &index);
					bufcatcstr(&decl, ";\n\t");
					bufcatbuf(bflyStr, &decl);
				}
				for (size_t i = 0; i < 9; i++)
				{
					// Build the qr/qi register declaration without temporary merge
					// helpers.
					buffer_t index = SztToStr(i);
					buffer_t decl = buffer_empty();
					bufsetbuf(&decl, &regType);
					bufcatcstr(&decl, " qr");
					bufcatbuf(&decl, &index);
					bufcatcstr(&decl, ", qi");
					bufcatbuf(&decl, &index);
					bufcatcstr(&decl, ";\n\t");
					bufcatbuf(bflyStr, &decl);
				}

				if (butterfly->fwd)
					bufcatcstr(bflyStr, C7SFR);
				else
					bufcatcstr(bflyStr, C7SBR);
			}
			else
			{
				for (size_t i = 0; i < 10; i++)
				{
					// Build the p register declaration without temporary merge
					// helpers.
					buffer_t index = SztToStr(i);
					buffer_t decl = buffer_empty();
					bufsetbuf(&decl, &regType);
					bufcatcstr(&decl, " p");
					bufcatbuf(&decl, &index);
					bufcatcstr(&decl, ";\n\t");
					bufcatbuf(bflyStr, &decl);
				}
				for (size_t i = 0; i < 9; i++)
				{
					// Build the q register declaration without temporary merge
					// helpers.
					buffer_t index = SztToStr(i);
					buffer_t decl = buffer_empty();
					bufsetbuf(&decl, &regType);
					bufcatcstr(&decl, " q");
					bufcatbuf(&decl, &index);
					bufcatcstr(&decl, ";\n\t");
					bufcatbuf(bflyStr, &decl);
				}
				if (butterfly->fwd)
					bufcatcstr(bflyStr, C7SFC);
				else
					bufcatcstr(bflyStr, C7SBC);
			}
		}
		break;

		case 8:
		{
			if (butterfly->fwd)
			{
				if (butterfly->cReg)
				{
					bufcatcstr(bflyStr,
						"(*R1) = (*R0) - (*R1);\n\t"
						"(*R0) = 2.0f * (*R0) - (*R1);\n\t"
						"(*R3) = (*R2) - (*R3);\n\t"
						"(*R2) = 2.0f * (*R2) - (*R3);\n\t"
						"(*R5) = (*R4) - (*R5);\n\t"
						"(*R4) = 2.0f * (*R4) - (*R5);\n\t"
						"(*R7) = (*R6) - (*R7);\n\t"
						"(*R6) = 2.0f * (*R6) - (*R7);\n\t"
						"\n\t"
						"(*R2) = (*R0) - (*R2);\n\t"
						"(*R0) = 2.0f * (*R0) - (*R2);\n\t"
						"(*R3) = (*R1) + (fvect2)(-(*R3).y, (*R3).x);\n\t"
						"(*R1) = 2.0f * (*R1) - (*R3);\n\t"
						"(*R6) = (*R4) - (*R6);\n\t"
						"(*R4) = 2.0f * (*R4) - (*R6);\n\t"
						"(*R7) = (*R5) + (fvect2)(-(*R7).y, (*R7).x);\n\t"
						"(*R5) = 2.0f * (*R5) - (*R7);\n\t"
						"\n\t"
						"(*R4) = (*R0) - (*R4);\n\t"
						"(*R0) = 2.0f * (*R0) - (*R4);\n\t"
						"(*R5) = ((*R1) - C8Q * (*R5)) - C8Q * "
						"(fvect2)((*R5).y, -(*R5).x);\n\t"
						"(*R1) = 2.0f * (*R1) - (*R5);\n\t"
						"(*R6) = (*R2) + (fvect2)(-(*R6).y, (*R6).x);\n\t"
						"(*R2) = 2.0f * (*R2) - (*R6);\n\t"
						"(*R7) = ((*R3) + C8Q * (*R7)) - C8Q * "
						"(fvect2)((*R7).y, -(*R7).x);\n\t"
						"(*R3) = 2.0f * (*R3) - (*R7);\n\t");
				}
				else
				{
					bufcatcstr(bflyStr,
						"TR0 = (*R0) + (*R4) + (*R2) + (*R6) +     (*R1)    "
						"         +     (*R3)             +     (*R5)       "
						"      +     (*R7)            ;\n\t"
						"TR1 = (*R0) - (*R4) + (*I2) - (*I6) + C8Q*(*R1) + "
						"C8Q*(*I1) - C8Q*(*R3) + C8Q*(*I3) - C8Q*(*R5) - "
						"C8Q*(*I5) + C8Q*(*R7) - C8Q*(*I7);\n\t"
						"TR2 = (*R0) + (*R4) - (*R2) - (*R6)             +  "
						"   (*I1)             -     (*I3)             +     "
						"(*I5)             -     (*I7);\n\t"
						"TR3 = (*R0) - (*R4) - (*I2) + (*I6) - C8Q*(*R1) + "
						"C8Q*(*I1) + C8Q*(*R3) + C8Q*(*I3) + C8Q*(*R5) - "
						"C8Q*(*I5) - C8Q*(*R7) - C8Q*(*I7);\n\t"
						"TR4 = (*R0) + (*R4) + (*R2) + (*R6) -     (*R1)    "
						"         -     (*R3)             -     (*R5)       "
						"      -     (*R7)            ;\n\t"
						"TR5 = (*R0) - (*R4) + (*I2) - (*I6) - C8Q*(*R1) - "
						"C8Q*(*I1) + C8Q*(*R3) - C8Q*(*I3) + C8Q*(*R5) + "
						"C8Q*(*I5) - C8Q*(*R7) + C8Q*(*I7);\n\t"
						"TR6 = (*R0) + (*R4) - (*R2) - (*R6)             -  "
						"  (*I1)              +     (*I3)             -     "
						"(*I5)             +     (*I7);\n\t"
						"TR7 = (*R0) - (*R4) - (*I2) + (*I6) + C8Q*(*R1) - "
						"C8Q*(*I1) - C8Q*(*R3) - C8Q*(*I3) - C8Q*(*R5) + "
						"C8Q*(*I5) + C8Q*(*R7) + C8Q*(*I7);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = (*I0) + (*I4) + (*I2) + (*I6)             +  "
						"   (*I1)             +     (*I3)             +     "
						"(*I5)             +     (*I7);\n\t"
						"TI1 = (*I0) - (*I4) - (*R2) + (*R6) - C8Q*(*R1) + "
						"C8Q*(*I1) - C8Q*(*R3) - C8Q*(*I3) + C8Q*(*R5) - "
						"C8Q*(*I5) + C8Q*(*R7) + C8Q*(*I7);\n\t"
						"TI2 = (*I0) + (*I4) - (*I2) - (*I6) -     (*R1)    "
						"         +     (*R3)             -     (*R5)       "
						"      +     (*R7)            ;\n\t"
						"TI3 = (*I0) - (*I4) + (*R2) - (*R6) - C8Q*(*R1) - "
						"C8Q*(*I1) - C8Q*(*R3) + C8Q*(*I3) + C8Q*(*R5) + "
						"C8Q*(*I5) + C8Q*(*R7) - C8Q*(*I7);\n\t"
						"TI4 = (*I0) + (*I4) + (*I2) + (*I6)             -  "
						"  (*I1)              -     (*I3)             -     "
						"(*I5)             -     (*I7);\n\t"
						"TI5 = (*I0) - (*I4) - (*R2) + (*R6) + C8Q*(*R1) - "
						"C8Q*(*I1) + C8Q*(*R3) + C8Q*(*I3) - C8Q*(*R5) + "
						"C8Q*(*I5) - C8Q*(*R7) - C8Q*(*I7);\n\t"
						"TI6 = (*I0) + (*I4) - (*I2) - (*I6) +     (*R1)    "
						"         -     (*R3)             +     (*R5)       "
						"      -     (*R7)            ;\n\t"
						"TI7 = (*I0) - (*I4) + (*R2) - (*R6) + C8Q*(*R1) + "
						"C8Q*(*I1) + C8Q*(*R3) - C8Q*(*I3) - C8Q*(*R5) - "
						"C8Q*(*I5) - C8Q*(*R7) + C8Q*(*I7);\n\t");
				}
			}
			else if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"(*R1) = (*R0) - (*R1);\n\t"
					"(*R0) = 2.0f * (*R0) - (*R1);\n\t"
					"(*R3) = (*R2) - (*R3);\n\t"
					"(*R2) = 2.0f * (*R2) - (*R3);\n\t"
					"(*R5) = (*R4) - (*R5);\n\t"
					"(*R4) = 2.0f * (*R4) - (*R5);\n\t"
					"(*R7) = (*R6) - (*R7);\n\t"
					"(*R6) = 2.0f * (*R6) - (*R7);\n\t"
					"\n\t"
					"(*R2) = (*R0) - (*R2);\n\t"
					"(*R0) = 2.0f * (*R0) - (*R2);\n\t"
					"(*R3) = (*R1) + (fvect2)((*R3).y, -(*R3).x);\n\t"
					"(*R1) = 2.0f * (*R1) - (*R3);\n\t"
					"(*R6) = (*R4) - (*R6);\n\t"
					"(*R4) = 2.0f * (*R4) - (*R6);\n\t"
					"(*R7) = (*R5) + (fvect2)((*R7).y, -(*R7).x);\n\t"
					"(*R5) = 2.0f * (*R5) - (*R7);\n\t"
					"\n\t"
					"(*R4) = (*R0) - (*R4);\n\t"
					"(*R0) = 2.0f * (*R0) - (*R4);\n\t"
					"(*R5) = ((*R1) - C8Q * (*R5)) + C8Q * "
					"(fvect2)((*R5).y, -(*R5).x);\n\t"
					"(*R1) = 2.0f * (*R1) - (*R5);\n\t"
					"(*R6) = (*R2) + (fvect2)((*R6).y, -(*R6).x);\n\t"
					"(*R2) = 2.0f * (*R2) - (*R6);\n\t"
					"(*R7) = ((*R3) + C8Q * (*R7)) + C8Q * "
					"(fvect2)((*R7).y, -(*R7).x);\n\t"
					"(*R3) = 2.0f * (*R3) - (*R7);\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0) + (*R4) + (*R2) + (*R6) +     (*R1)        "
					"     +     (*R3)             +     (*R5)             + "
					"    (*R7)            ;\n\t"
					"TR1 = (*R0) - (*R4) - (*I2) + (*I6) + C8Q*(*R1) - "
					"C8Q*(*I1) - C8Q*(*R3) - C8Q*(*I3) - C8Q*(*R5) + "
					"C8Q*(*I5) + C8Q*(*R7) + C8Q*(*I7);\n\t"
					"TR2 = (*R0) + (*R4) - (*R2) - (*R6)             -     "
					"(*I1)             +     (*I3)             -     (*I5)  "
					"           +     (*I7);\n\t"
					"TR3 = (*R0) - (*R4) + (*I2) - (*I6) - C8Q*(*R1) - "
					"C8Q*(*I1) + C8Q*(*R3) - C8Q*(*I3) + C8Q*(*R5) + "
					"C8Q*(*I5) - C8Q*(*R7) + C8Q*(*I7);\n\t"
					"TR4 = (*R0) + (*R4) + (*R2) + (*R6) -     (*R1)        "
					"     -    (*R3)              -     (*R5)             - "
					"    (*R7)            ;\n\t"
					"TR5 = (*R0) - (*R4) - (*I2) + (*I6) - C8Q*(*R1) + "
					"C8Q*(*I1) + C8Q*(*R3) + C8Q*(*I3) + C8Q*(*R5) - "
					"C8Q*(*I5) - C8Q*(*R7) - C8Q*(*I7);\n\t"
					"TR6 = (*R0) + (*R4) - (*R2) - (*R6)             +     "
					"(*I1)             -     (*I3)             +     (*I5)  "
					"           -     (*I7);\n\t"
					"TR7 = (*R0) - (*R4) + (*I2) - (*I6) + C8Q*(*R1) + "
					"C8Q*(*I1) - C8Q*(*R3) + C8Q*(*I3) - C8Q*(*R5) - "
					"C8Q*(*I5) + C8Q*(*R7) - C8Q*(*I7);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = (*I0) + (*I4) + (*I2) + (*I6)             +     "
					"(*I1)             +    (*I3)              +     (*I5)  "
					"           +     (*I7);\n\t"
					"TI1 = (*I0) - (*I4) + (*R2) - (*R6) + C8Q*(*R1) + "
					"C8Q*(*I1) + C8Q*(*R3) - C8Q*(*I3) - C8Q*(*R5) - "
					"C8Q*(*I5) - C8Q*(*R7) + C8Q*(*I7);\n\t"
					"TI2 = (*I0) + (*I4) - (*I2) - (*I6) +     (*R1)        "
					"     -     (*R3)             +     (*R5)             - "
					"    (*R7)            ;\n\t"
					"TI3 = (*I0) - (*I4) - (*R2) + (*R6) + C8Q*(*R1) - "
					"C8Q*(*I1) + C8Q*(*R3) + C8Q*(*I3) - C8Q*(*R5) + "
					"C8Q*(*I5) - C8Q*(*R7) - C8Q*(*I7);\n\t"
					"TI4 = (*I0) + (*I4) + (*I2) + (*I6)             -     "
					"(*I1)             -     (*I3)             -     (*I5)  "
					"           -     (*I7);\n\t"
					"TI5 = (*I0) - (*I4) + (*R2) - (*R6) - C8Q*(*R1) - "
					"C8Q*(*I1) - C8Q*(*R3) + C8Q*(*I3) + C8Q*(*R5) + "
					"C8Q*(*I5) + C8Q*(*R7) - C8Q*(*I7);\n\t"
					"TI6 = (*I0) + (*I4) - (*I2) - (*I6) -     (*R1)        "
					"     +     (*R3)             -     (*R5)             + "
					"    (*R7)            ;\n\t"
					"TI7 = (*I0) - (*I4) - (*R2) + (*R6) - C8Q*(*R1) + "
					"C8Q*(*I1) - C8Q*(*R3) - C8Q*(*I3) + C8Q*(*R5) - "
					"C8Q*(*I5) + C8Q*(*R7) + C8Q*(*I7);\n\t");
			}
		}
		break;
		case 10:
		{
			if (butterfly->fwd)
			{
				if (butterfly->cReg)
				{
					bufcatcstr(bflyStr,
						"TR0 = (*R0).x + (*R2).x + (*R4).x + (*R6).x + (*R8).x;\n\t"
						"TR2 = ((*R0).x - C5QC*((*R4).x + (*R6).x)) + "
						"C5QB*((*R2).y - (*R8).y) + C5QD*((*R4).y - (*R6).y) + "
						"C5QA*(((*R2).x - (*R4).x) + ((*R8).x - (*R6).x));\n\t"
						"TR8 = ((*R0).x - C5QC*((*R4).x + (*R6).x)) - "
						"C5QB*((*R2).y - (*R8).y) - C5QD*((*R4).y - (*R6).y) + "
						"C5QA*(((*R2).x - (*R4).x) + ((*R8).x - (*R6).x));\n\t"
						"TR4 = ((*R0).x - C5QC*((*R2).x + (*R8).x)) - "
						"C5QB*((*R4).y - (*R6).y) + C5QD*((*R2).y - (*R8).y) + "
						"C5QA*(((*R4).x - (*R2).x) + ((*R6).x - (*R8).x));\n\t"
						"TR6 = ((*R0).x - C5QC*((*R2).x + (*R8).x)) + "
						"C5QB*((*R4).y - (*R6).y) - C5QD*((*R2).y - (*R8).y) + "
						"C5QA*(((*R4).x - (*R2).x) + ((*R6).x - (*R8).x));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = (*R0).y + (*R2).y + (*R4).y + (*R6).y + (*R8).y;\n\t"
						"TI2 = ((*R0).y - C5QC*((*R4).y + (*R6).y)) - "
						"C5QB*((*R2).x - (*R8).x) - C5QD*((*R4).x - (*R6).x) + "
						"C5QA*(((*R2).y - (*R4).y) + ((*R8).y - (*R6).y));\n\t"
						"TI8 = ((*R0).y - C5QC*((*R4).y + (*R6).y)) + "
						"C5QB*((*R2).x - (*R8).x) + C5QD*((*R4).x - (*R6).x) + "
						"C5QA*(((*R2).y - (*R4).y) + ((*R8).y - (*R6).y));\n\t"
						"TI4 = ((*R0).y - C5QC*((*R2).y + (*R8).y)) + "
						"C5QB*((*R4).x - (*R6).x) - C5QD*((*R2).x - (*R8).x) + "
						"C5QA*(((*R4).y - (*R2).y) + ((*R6).y - (*R8).y));\n\t"
						"TI6 = ((*R0).y - C5QC*((*R2).y + (*R8).y)) - "
						"C5QB*((*R4).x - (*R6).x) + C5QD*((*R2).x - (*R8).x) + "
						"C5QA*(((*R4).y - (*R2).y) + ((*R6).y - (*R8).y));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TR1 = (*R1).x + (*R3).x + (*R5).x + (*R7).x + (*R9).x;\n\t"
						"TR3 = ((*R1).x - C5QC*((*R5).x + (*R7).x)) + "
						"C5QB*((*R3).y - (*R9).y) + C5QD*((*R5).y - (*R7).y) + "
						"C5QA*(((*R3).x - (*R5).x) + ((*R9).x - (*R7).x));\n\t"
						"TR9 = ((*R1).x - C5QC*((*R5).x + (*R7).x)) - "
						"C5QB*((*R3).y - (*R9).y) - C5QD*((*R5).y - (*R7).y) + "
						"C5QA*(((*R3).x - (*R5).x) + ((*R9).x - (*R7).x));\n\t"
						"TR5 = ((*R1).x - C5QC*((*R3).x + (*R9).x)) - "
						"C5QB*((*R5).y - (*R7).y) + C5QD*((*R3).y - (*R9).y) + "
						"C5QA*(((*R5).x - (*R3).x) + ((*R7).x - (*R9).x));\n\t"
						"TR7 = ((*R1).x - C5QC*((*R3).x + (*R9).x)) + "
						"C5QB*((*R5).y - (*R7).y) - C5QD*((*R3).y - (*R9).y) + "
						"C5QA*(((*R5).x - (*R3).x) + ((*R7).x - (*R9).x));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI1 = (*R1).y + (*R3).y + (*R5).y + (*R7).y + (*R9).y;\n\t"
						"TI3 = ((*R1).y - C5QC*((*R5).y + (*R7).y)) - "
						"C5QB*((*R3).x - (*R9).x) - C5QD*((*R5).x - (*R7).x) + "
						"C5QA*(((*R3).y - (*R5).y) + ((*R9).y - (*R7).y));\n\t"
						"TI9 = ((*R1).y - C5QC*((*R5).y + (*R7).y)) + "
						"C5QB*((*R3).x - (*R9).x) + C5QD*((*R5).x - (*R7).x) + "
						"C5QA*(((*R3).y - (*R5).y) + ((*R9).y - (*R7).y));\n\t"
						"TI5 = ((*R1).y - C5QC*((*R3).y + (*R9).y)) + "
						"C5QB*((*R5).x - (*R7).x) - C5QD*((*R3).x - (*R9).x) + "
						"C5QA*(((*R5).y - (*R3).y) + ((*R7).y - (*R9).y));\n\t"
						"TI7 = ((*R1).y - C5QC*((*R3).y + (*R9).y)) - "
						"C5QB*((*R5).x - (*R7).x) + C5QD*((*R3).x - (*R9).x) + "
						"C5QA*(((*R5).y - (*R3).y) + ((*R7).y - (*R9).y));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R0).x = TR0 + TR1;\n\t"
						"(*R1).x = TR2 + ( C5QE*TR3 + C5QD*TI3);\n\t"
						"(*R2).x = TR4 + ( C5QA*TR5 + C5QB*TI5);\n\t"
						"(*R3).x = TR6 + (-C5QA*TR7 + C5QB*TI7);\n\t"
						"(*R4).x = TR8 + (-C5QE*TR9 + C5QD*TI9);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R0).y = TI0 + TI1;\n\t"
						"(*R1).y = TI2 + (-C5QD*TR3 + C5QE*TI3);\n\t"
						"(*R2).y = TI4 + (-C5QB*TR5 + C5QA*TI5);\n\t"
						"(*R3).y = TI6 + (-C5QB*TR7 - C5QA*TI7);\n\t"
						"(*R4).y = TI8 + (-C5QD*TR9 - C5QE*TI9);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R5).x = TR0 - TR1;\n\t"
						"(*R6).x = TR2 - ( C5QE*TR3 + C5QD*TI3);\n\t"
						"(*R7).x = TR4 - ( C5QA*TR5 + C5QB*TI5);\n\t"
						"(*R8).x = TR6 - (-C5QA*TR7 + C5QB*TI7);\n\t"
						"(*R9).x = TR8 - (-C5QE*TR9 + C5QD*TI9);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R5).y = TI0 - TI1;\n\t"
						"(*R6).y = TI2 - (-C5QD*TR3 + C5QE*TI3);\n\t"
						"(*R7).y = TI4 - (-C5QB*TR5 + C5QA*TI5);\n\t"
						"(*R8).y = TI6 - (-C5QB*TR7 - C5QA*TI7);\n\t"
						"(*R9).y = TI8 - (-C5QD*TR9 - C5QE*TI9);\n\t");
				}
				else
				{
					bufcatcstr(bflyStr,
						"TR0 = *R0 + *R2 + *R4 + *R6 + *R8;\n\t"
						"TR2 = (*R0 - C5QC*(*R4 + *R6)) + C5QB*(*I2 - *I8) + "
						"C5QD*(*I4 - *I6) + C5QA*((*R2 - *R4) + (*R8 - *R6));\n\t"
						"TR8 = (*R0 - C5QC*(*R4 + *R6)) - C5QB*(*I2 - *I8) - "
						"C5QD*(*I4 - *I6) + C5QA*((*R2 - *R4) + (*R8 - *R6));\n\t"
						"TR4 = (*R0 - C5QC*(*R2 + *R8)) - C5QB*(*I4 - *I6) + "
						"C5QD*(*I2 - *I8) + C5QA*((*R4 - *R2) + (*R6 - *R8));\n\t"
						"TR6 = (*R0 - C5QC*(*R2 + *R8)) + C5QB*(*I4 - *I6) - "
						"C5QD*(*I2 - *I8) + C5QA*((*R4 - *R2) + (*R6 - *R8));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI0 = *I0 + *I2 + *I4 + *I6 + *I8;\n\t"
						"TI2 = (*I0 - C5QC*(*I4 + *I6)) - C5QB*(*R2 - *R8) - "
						"C5QD*(*R4 - *R6) + C5QA*((*I2 - *I4) + (*I8 - *I6));\n\t"
						"TI8 = (*I0 - C5QC*(*I4 + *I6)) + C5QB*(*R2 - *R8) + "
						"C5QD*(*R4 - *R6) + C5QA*((*I2 - *I4) + (*I8 - *I6));\n\t"
						"TI4 = (*I0 - C5QC*(*I2 + *I8)) + C5QB*(*R4 - *R6) - "
						"C5QD*(*R2 - *R8) + C5QA*((*I4 - *I2) + (*I6 - *I8));\n\t"
						"TI6 = (*I0 - C5QC*(*I2 + *I8)) - C5QB*(*R4 - *R6) + "
						"C5QD*(*R2 - *R8) + C5QA*((*I4 - *I2) + (*I6 - *I8));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TR1 = *R1 + *R3 + *R5 + *R7 + *R9;\n\t"
						"TR3 = (*R1 - C5QC*(*R5 + *R7)) + C5QB*(*I3 - *I9) + "
						"C5QD*(*I5 - *I7) + C5QA*((*R3 - *R5) + (*R9 - *R7));\n\t"
						"TR9 = (*R1 - C5QC*(*R5 + *R7)) - C5QB*(*I3 - *I9) - "
						"C5QD*(*I5 - *I7) + C5QA*((*R3 - *R5) + (*R9 - *R7));\n\t"
						"TR5 = (*R1 - C5QC*(*R3 + *R9)) - C5QB*(*I5 - *I7) + "
						"C5QD*(*I3 - *I9) + C5QA*((*R5 - *R3) + (*R7 - *R9));\n\t"
						"TR7 = (*R1 - C5QC*(*R3 + *R9)) + C5QB*(*I5 - *I7) - "
						"C5QD*(*I3 - *I9) + C5QA*((*R5 - *R3) + (*R7 - *R9));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"TI1 = *I1 + *I3 + *I5 + *I7 + *I9;\n\t"
						"TI3 = (*I1 - C5QC*(*I5 + *I7)) - C5QB*(*R3 - *R9) - "
						"C5QD*(*R5 - *R7) + C5QA*((*I3 - *I5) + (*I9 - *I7));\n\t"
						"TI9 = (*I1 - C5QC*(*I5 + *I7)) + C5QB*(*R3 - *R9) + "
						"C5QD*(*R5 - *R7) + C5QA*((*I3 - *I5) + (*I9 - *I7));\n\t"
						"TI5 = (*I1 - C5QC*(*I3 + *I9)) + C5QB*(*R5 - *R7) - "
						"C5QD*(*R3 - *R9) + C5QA*((*I5 - *I3) + (*I7 - *I9));\n\t"
						"TI7 = (*I1 - C5QC*(*I3 + *I9)) - C5QB*(*R5 - *R7) + "
						"C5QD*(*R3 - *R9) + C5QA*((*I5 - *I3) + (*I7 - *I9));\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R0) = TR0 + TR1;\n\t"
						"(*R1) = TR2 + ( C5QE*TR3 + C5QD*TI3);\n\t"
						"(*R2) = TR4 + ( C5QA*TR5 + C5QB*TI5);\n\t"
						"(*R3) = TR6 + (-C5QA*TR7 + C5QB*TI7);\n\t"
						"(*R4) = TR8 + (-C5QE*TR9 + C5QD*TI9);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*I0) = TI0 + TI1;\n\t"
						"(*I1) = TI2 + (-C5QD*TR3 + C5QE*TI3);\n\t"
						"(*I2) = TI4 + (-C5QB*TR5 + C5QA*TI5);\n\t"
						"(*I3) = TI6 + (-C5QB*TR7 - C5QA*TI7);\n\t"
						"(*I4) = TI8 + (-C5QD*TR9 - C5QE*TI9);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*R5) = TR0 - TR1;\n\t"
						"(*R6) = TR2 - ( C5QE*TR3 + C5QD*TI3);\n\t"
						"(*R7) = TR4 - ( C5QA*TR5 + C5QB*TI5);\n\t"
						"(*R8) = TR6 - (-C5QA*TR7 + C5QB*TI7);\n\t"
						"(*R9) = TR8 - (-C5QE*TR9 + C5QD*TI9);\n\t");

					bufcatcstr(bflyStr, "\n\t");

					bufcatcstr(bflyStr,
						"(*I5) = TI0 - TI1;\n\t"
						"(*I6) = TI2 - (-C5QD*TR3 + C5QE*TI3);\n\t"
						"(*I7) = TI4 - (-C5QB*TR5 + C5QA*TI5);\n\t"
						"(*I8) = TI6 - (-C5QB*TR7 - C5QA*TI7);\n\t"
						"(*I9) = TI8 - (-C5QD*TR9 - C5QE*TI9);\n\t");
				}
			}
			else if (butterfly->cReg)
			{
				bufcatcstr(bflyStr,
					"TR0 = (*R0).x + (*R2).x + (*R4).x + (*R6).x + (*R8).x;\n\t"
					"TR2 = ((*R0).x - C5QC*((*R4).x + (*R6).x)) - C5QB*((*R2).y - "
					"(*R8).y) - C5QD*((*R4).y - (*R6).y) + C5QA*(((*R2).x - "
					"(*R4).x) + ((*R8).x - (*R6).x));\n\t"
					"TR8 = ((*R0).x - C5QC*((*R4).x + (*R6).x)) + C5QB*((*R2).y - "
					"(*R8).y) + C5QD*((*R4).y - (*R6).y) + C5QA*(((*R2).x - "
					"(*R4).x) + ((*R8).x - (*R6).x));\n\t"
					"TR4 = ((*R0).x - C5QC*((*R2).x + (*R8).x)) + C5QB*((*R4).y - "
					"(*R6).y) - C5QD*((*R2).y - (*R8).y) + C5QA*(((*R4).x - "
					"(*R2).x) + ((*R6).x - (*R8).x));\n\t"
					"TR6 = ((*R0).x - C5QC*((*R2).x + (*R8).x)) - C5QB*((*R4).y - "
					"(*R6).y) + C5QD*((*R2).y - (*R8).y) + C5QA*(((*R4).x - "
					"(*R2).x) + ((*R6).x - (*R8).x));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = (*R0).y + (*R2).y + (*R4).y + (*R6).y + (*R8).y;\n\t"
					"TI2 = ((*R0).y - C5QC*((*R4).y + (*R6).y)) + C5QB*((*R2).x - "
					"(*R8).x) + C5QD*((*R4).x - (*R6).x) + C5QA*(((*R2).y - "
					"(*R4).y) + ((*R8).y - (*R6).y));\n\t"
					"TI8 = ((*R0).y - C5QC*((*R4).y + (*R6).y)) - C5QB*((*R2).x - "
					"(*R8).x) - C5QD*((*R4).x - (*R6).x) + C5QA*(((*R2).y - "
					"(*R4).y) + ((*R8).y - (*R6).y));\n\t"
					"TI4 = ((*R0).y - C5QC*((*R2).y + (*R8).y)) - C5QB*((*R4).x - "
					"(*R6).x) + C5QD*((*R2).x - (*R8).x) + C5QA*(((*R4).y - "
					"(*R2).y) + ((*R6).y - (*R8).y));\n\t"
					"TI6 = ((*R0).y - C5QC*((*R2).y + (*R8).y)) + C5QB*((*R4).x - "
					"(*R6).x) - C5QD*((*R2).x - (*R8).x) + C5QA*(((*R4).y - "
					"(*R2).y) + ((*R6).y - (*R8).y));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TR1 = (*R1).x + (*R3).x + (*R5).x + (*R7).x + (*R9).x;\n\t"
					"TR3 = ((*R1).x - C5QC*((*R5).x + (*R7).x)) - C5QB*((*R3).y - "
					"(*R9).y) - C5QD*((*R5).y - (*R7).y) + C5QA*(((*R3).x - "
					"(*R5).x) + ((*R9).x - (*R7).x));\n\t"
					"TR9 = ((*R1).x - C5QC*((*R5).x + (*R7).x)) + C5QB*((*R3).y - "
					"(*R9).y) + C5QD*((*R5).y - (*R7).y) + C5QA*(((*R3).x - "
					"(*R5).x) + ((*R9).x - (*R7).x));\n\t"
					"TR5 = ((*R1).x - C5QC*((*R3).x + (*R9).x)) + C5QB*((*R5).y - "
					"(*R7).y) - C5QD*((*R3).y - (*R9).y) + C5QA*(((*R5).x - "
					"(*R3).x) + ((*R7).x - (*R9).x));\n\t"
					"TR7 = ((*R1).x - C5QC*((*R3).x + (*R9).x)) - C5QB*((*R5).y - "
					"(*R7).y) + C5QD*((*R3).y - (*R9).y) + C5QA*(((*R5).x - "
					"(*R3).x) + ((*R7).x - (*R9).x));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI1 = (*R1).y + (*R3).y + (*R5).y + (*R7).y + (*R9).y;\n\t"
					"TI3 = ((*R1).y - C5QC*((*R5).y + (*R7).y)) + C5QB*((*R3).x - "
					"(*R9).x) + C5QD*((*R5).x - (*R7).x) + C5QA*(((*R3).y - "
					"(*R5).y) + ((*R9).y - (*R7).y));\n\t"
					"TI9 = ((*R1).y - C5QC*((*R5).y + (*R7).y)) - C5QB*((*R3).x - "
					"(*R9).x) - C5QD*((*R5).x - (*R7).x) + C5QA*(((*R3).y - "
					"(*R5).y) + ((*R9).y - (*R7).y));\n\t"
					"TI5 = ((*R1).y - C5QC*((*R3).y + (*R9).y)) - C5QB*((*R5).x - "
					"(*R7).x) + C5QD*((*R3).x - (*R9).x) + C5QA*(((*R5).y - "
					"(*R3).y) + ((*R7).y - (*R9).y));\n\t"
					"TI7 = ((*R1).y - C5QC*((*R3).y + (*R9).y)) + C5QB*((*R5).x - "
					"(*R7).x) - C5QD*((*R3).x - (*R9).x) + C5QA*(((*R5).y - "
					"(*R3).y) + ((*R7).y - (*R9).y));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R0).x = TR0 + TR1;\n\t"
					"(*R1).x = TR2 + ( C5QE*TR3 - C5QD*TI3);\n\t"
					"(*R2).x = TR4 + ( C5QA*TR5 - C5QB*TI5);\n\t"
					"(*R3).x = TR6 + (-C5QA*TR7 - C5QB*TI7);\n\t"
					"(*R4).x = TR8 + (-C5QE*TR9 - C5QD*TI9);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R0).y = TI0 + TI1;\n\t"
					"(*R1).y = TI2 + ( C5QD*TR3 + C5QE*TI3);\n\t"
					"(*R2).y = TI4 + ( C5QB*TR5 + C5QA*TI5);\n\t"
					"(*R3).y = TI6 + ( C5QB*TR7 - C5QA*TI7);\n\t"
					"(*R4).y = TI8 + ( C5QD*TR9 - C5QE*TI9);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R5).x = TR0 - TR1;\n\t"
					"(*R6).x = TR2 - ( C5QE*TR3 - C5QD*TI3);\n\t"
					"(*R7).x = TR4 - ( C5QA*TR5 - C5QB*TI5);\n\t"
					"(*R8).x = TR6 - (-C5QA*TR7 - C5QB*TI7);\n\t"
					"(*R9).x = TR8 - (-C5QE*TR9 - C5QD*TI9);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R5).y = TI0 - TI1;\n\t"
					"(*R6).y = TI2 - ( C5QD*TR3 + C5QE*TI3);\n\t"
					"(*R7).y = TI4 - ( C5QB*TR5 + C5QA*TI5);\n\t"
					"(*R8).y = TI6 - ( C5QB*TR7 - C5QA*TI7);\n\t"
					"(*R9).y = TI8 - ( C5QD*TR9 - C5QE*TI9);\n\t");
			}
			else
			{
				bufcatcstr(bflyStr,
					"TR0 = *R0 + *R2 + *R4 + *R6 + *R8;\n\t"
					"TR2 = (*R0 - C5QC*(*R4 + *R6)) - C5QB*(*I2 - *I8) - C5QD*(*I4 "
					"- *I6) + C5QA*((*R2 - *R4) + (*R8 - *R6));\n\t"
					"TR8 = (*R0 - C5QC*(*R4 + *R6)) + C5QB*(*I2 - *I8) + C5QD*(*I4 "
					"- *I6) + C5QA*((*R2 - *R4) + (*R8 - *R6));\n\t"
					"TR4 = (*R0 - C5QC*(*R2 + *R8)) + C5QB*(*I4 - *I6) - C5QD*(*I2 "
					"- *I8) + C5QA*((*R4 - *R2) + (*R6 - *R8));\n\t"
					"TR6 = (*R0 - C5QC*(*R2 + *R8)) - C5QB*(*I4 - *I6) + C5QD*(*I2 "
					"- *I8) + C5QA*((*R4 - *R2) + (*R6 - *R8));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI0 = *I0 + *I2 + *I4 + *I6 + *I8;\n\t"
					"TI2 = (*I0 - C5QC*(*I4 + *I6)) + C5QB*(*R2 - *R8) + C5QD*(*R4 "
					"- *R6) + C5QA*((*I2 - *I4) + (*I8 - *I6));\n\t"
					"TI8 = (*I0 - C5QC*(*I4 + *I6)) - C5QB*(*R2 - *R8) - C5QD*(*R4 "
					"- *R6) + C5QA*((*I2 - *I4) + (*I8 - *I6));\n\t"
					"TI4 = (*I0 - C5QC*(*I2 + *I8)) - C5QB*(*R4 - *R6) + C5QD*(*R2 "
					"- *R8) + C5QA*((*I4 - *I2) + (*I6 - *I8));\n\t"
					"TI6 = (*I0 - C5QC*(*I2 + *I8)) + C5QB*(*R4 - *R6) - C5QD*(*R2 "
					"- *R8) + C5QA*((*I4 - *I2) + (*I6 - *I8));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TR1 = *R1 + *R3 + *R5 + *R7 + *R9;\n\t"
					"TR3 = (*R1 - C5QC*(*R5 + *R7)) - C5QB*(*I3 - *I9) - C5QD*(*I5 "
					"- *I7) + C5QA*((*R3 - *R5) + (*R9 - *R7));\n\t"
					"TR9 = (*R1 - C5QC*(*R5 + *R7)) + C5QB*(*I3 - *I9) + C5QD*(*I5 "
					"- *I7) + C5QA*((*R3 - *R5) + (*R9 - *R7));\n\t"
					"TR5 = (*R1 - C5QC*(*R3 + *R9)) + C5QB*(*I5 - *I7) - C5QD*(*I3 "
					"- *I9) + C5QA*((*R5 - *R3) + (*R7 - *R9));\n\t"
					"TR7 = (*R1 - C5QC*(*R3 + *R9)) - C5QB*(*I5 - *I7) + C5QD*(*I3 "
					"- *I9) + C5QA*((*R5 - *R3) + (*R7 - *R9));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"TI1 = *I1 + *I3 + *I5 + *I7 + *I9;\n\t"
					"TI3 = (*I1 - C5QC*(*I5 + *I7)) + C5QB*(*R3 - *R9) + C5QD*(*R5 "
					"- *R7) + C5QA*((*I3 - *I5) + (*I9 - *I7));\n\t"
					"TI9 = (*I1 - C5QC*(*I5 + *I7)) - C5QB*(*R3 - *R9) - C5QD*(*R5 "
					"- *R7) + C5QA*((*I3 - *I5) + (*I9 - *I7));\n\t"
					"TI5 = (*I1 - C5QC*(*I3 + *I9)) - C5QB*(*R5 - *R7) + C5QD*(*R3 "
					"- *R9) + C5QA*((*I5 - *I3) + (*I7 - *I9));\n\t"
					"TI7 = (*I1 - C5QC*(*I3 + *I9)) + C5QB*(*R5 - *R7) - C5QD*(*R3 "
					"- *R9) + C5QA*((*I5 - *I3) + (*I7 - *I9));\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R0) = TR0 + TR1;\n\t"
					"(*R1) = TR2 + ( C5QE*TR3 - C5QD*TI3);\n\t"
					"(*R2) = TR4 + ( C5QA*TR5 - C5QB*TI5);\n\t"
					"(*R3) = TR6 + (-C5QA*TR7 - C5QB*TI7);\n\t"
					"(*R4) = TR8 + (-C5QE*TR9 - C5QD*TI9);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*I0) = TI0 + TI1;\n\t"
					"(*I1) = TI2 + ( C5QD*TR3 + C5QE*TI3);\n\t"
					"(*I2) = TI4 + ( C5QB*TR5 + C5QA*TI5);\n\t"
					"(*I3) = TI6 + ( C5QB*TR7 - C5QA*TI7);\n\t"
					"(*I4) = TI8 + ( C5QD*TR9 - C5QE*TI9);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*R5) = TR0 - TR1;\n\t"
					"(*R6) = TR2 - ( C5QE*TR3 - C5QD*TI3);\n\t"
					"(*R7) = TR4 - ( C5QA*TR5 - C5QB*TI5);\n\t"
					"(*R8) = TR6 - (-C5QA*TR7 - C5QB*TI7);\n\t"
					"(*R9) = TR8 - (-C5QE*TR9 - C5QD*TI9);\n\t");

				bufcatcstr(bflyStr, "\n\t");

				bufcatcstr(bflyStr,
					"(*I5) = TI0 - TI1;\n\t"
					"(*I6) = TI2 - ( C5QD*TR3 + C5QE*TI3);\n\t"
					"(*I7) = TI4 - ( C5QB*TR5 + C5QA*TI5);\n\t"
					"(*I8) = TI6 - ( C5QB*TR7 - C5QA*TI7);\n\t"
					"(*I9) = TI8 - ( C5QD*TR9 - C5QE*TI9);\n\t");
			}
		}
		break;
		case 11:
		{
			static const char *radix11str[] = { " \t\t\t\t\t\tfptype p0, p1, p2, p3, p4, p5, p6, p7, p8, p9; \n", "\t\t\t\t\t\tp0 = ((*R1).x - (*R10).x)*dir; \n",
				"\t\t\t\t\t\tp1 = (*R1).x + (*R10).x; \n", "\t\t\t\t\t\tp2 = ((*R5).x - (*R6).x)*dir; \n", "\t\t\t\t\t\tp3 = (*R5).x + (*R6).x; \n",
				"\t\t\t\t\t\tp4 = ((*R2).x - (*R9).x)*dir; \n", "\t\t\t\t\t\tp5 = (*R2).x + (*R9).x; \n", "\t\t\t\t\t\tp6 = ((*R3).x - (*R8).x)*dir; \n",
				"\t\t\t\t\t\tp7 = (*R3).x + (*R8).x; \n", "\t\t\t\t\t\tp8 = (*R4).x + (*R7).x; \n", "\t\t\t\t\t\tp9 = ((*R4).x - (*R7).x)*dir; \n",
				"\t\t\t\t\t\t\n", "\t\t\t\t\t\tfptype r0, r1, r2, r3, r4, r5, r6, r7, r8, r9; \n", "\t\t\t\t\t\tr0 = p4 - p0 * b11_9; \n",
				"\t\t\t\t\t\tr1 = p0 + p2 * b11_9; \n", "\t\t\t\t\t\tr2 = p2 + p6 * b11_9; \n", "\t\t\t\t\t\tr3 = p6 + p9 * b11_9; \n",
				"\t\t\t\t\t\tr4 = p9 - p4 * b11_9; \n", "\t\t\t\t\t\tr5 = p7 - p1 * b11_8; \n", "\t\t\t\t\t\tr6 = p5 - p7 * b11_8; \n",
				"\t\t\t\t\t\tr7 = p1 - p8 * b11_8; \n", "\t\t\t\t\t\tr8 = p3 - p5 * b11_8; \n", "\t\t\t\t\t\tr9 = p8 - p3 * b11_8; \n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype s0, s1, s2, s3, s4, s5, s6, s7, s8, s9; \n", "\t\t\t\t\t\ts0 = p6 - r0 * b11_6; \n", "\t\t\t\t\t\ts1 = p9 + r1 * b11_6; \n",
				"\t\t\t\t\t\ts2 = p4 - r2 * b11_6; \n", "\t\t\t\t\t\ts3 = p0 + r3 * b11_6; \n", "\t\t\t\t\t\ts4 = p2 + r4 * b11_6; \n",
				"\t\t\t\t\t\ts5 = p3 - r5 * b11_7; \n", "\t\t\t\t\t\ts6 = p8 - r6 * b11_7; \n", "\t\t\t\t\t\ts7 = p5 - r7 * b11_7; \n",
				"\t\t\t\t\t\ts8 = p1 - r8 * b11_7; \n", "\t\t\t\t\t\ts9 = p7 - r9 * b11_7; \n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype p10, p11, p12, p13, p14, p15, p16, p17, p18, "
				"p19; \n",
				"\t\t\t\t\t\tp10 = ((*R10).y - (*R1).y)*dir; \n", "\t\t\t\t\t\tp11 = (*R1).y + (*R10).y; \n", "\t\t\t\t\t\tp12 = ((*R9).y - (*R2).y)*dir; \n",
				"\t\t\t\t\t\tp13 = (*R2).y + (*R9).y; \n", "\t\t\t\t\t\tp14 = ((*R8).y - (*R3).y)*dir; \n", "\t\t\t\t\t\tp15 = (*R3).y + (*R8).y; \n",
				"\t\t\t\t\t\tp16 = ((*R7).y - (*R4).y)*dir; \n", "\t\t\t\t\t\tp17 = (*R4).y + (*R7).y; \n", "\t\t\t\t\t\tp18 = ((*R6).y - (*R5).y)*dir; \n",
				"\t\t\t\t\t\tp19 = (*R5).y + (*R6).y; \n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype r10, r11, r12, r13, r14, r15, r16, r17, r18, "
				"r19; \n",
				"\t\t\t\t\t\tr10 = p12 - p10 * b11_9; \n", "\t\t\t\t\t\tr11 = p16 - p12 * b11_9; \n", "\t\t\t\t\t\tr12 = p18 + p14 * b11_9; \n",
				"\t\t\t\t\t\tr13 = p14 + p16 * b11_9; \n", "\t\t\t\t\t\tr14 = p10 + p18 * b11_9; \n", "\t\t\t\t\t\tr15 = p15 - p11 * b11_8; \n",
				"\t\t\t\t\t\tr16 = p19 - p13 * b11_8; \n", "\t\t\t\t\t\tr17 = p13 - p15 * b11_8; \n", "\t\t\t\t\t\tr18 = p11 - p17 * b11_8; \n",
				"\t\t\t\t\t\tr19 = p17 - p19 * b11_8; \n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype s10, s11, s12, s13, s14, s15, s16, s17, s18, "
				"s19; \n",
				"\t\t\t\t\t\ts10 = p14 - r10 * b11_6; \n", "\t\t\t\t\t\ts11 = p18 + r11 * b11_6; \n", "\t\t\t\t\t\ts12 = p12 - r12 * b11_6; \n",
				"\t\t\t\t\t\ts13 = p10 + r13 * b11_6; \n", "\t\t\t\t\t\ts14 = p16 + r14 * b11_6; \n", "\t\t\t\t\t\ts15 = p19 - r15 * b11_7; \n",
				"\t\t\t\t\t\ts16 = p11 - r16 * b11_7; \n", "\t\t\t\t\t\ts17 = p17 - r17 * b11_7; \n", "\t\t\t\t\t\ts18 = p13 - r18 * b11_7; \n",
				"\t\t\t\t\t\ts19 = p15 - r19 * b11_7; \n", "\t\t\t\t\t\t\n", "\t\t\t\t\t\tfptype v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \n",
				"\t\t\t\t\t\tfptype v10, v11, v12, v13, v14, v15, v16, v17, v18, "
				"v19; \n",
				"\t\t\t\t\t\tv0 = p9 - s0 * b11_4; \n", "\t\t\t\t\t\tv1 = p4 + s1 * b11_4; \n", "\t\t\t\t\t\tv2 = p0 + s2 * b11_4; \n",
				"\t\t\t\t\t\tv3 = p2 - s3 * b11_4; \n", "\t\t\t\t\t\tv4 = p6 - s4 * b11_4; \n", "\t\t\t\t\t\tv5 = p8 - s5 * b11_5; \n",
				"\t\t\t\t\t\tv6 = p1 - s6 * b11_5; \n", "\t\t\t\t\t\tv7 = p3 - s7 * b11_5; \n", "\t\t\t\t\t\tv8 = p7 - s8 * b11_5; \n",
				"\t\t\t\t\t\tv9 = p5 - s9 * b11_5; \n", "\t\t\t\t\t\tv10 = p16 - s10 * b11_4; \n", "\t\t\t\t\t\tv11 = p14 - s11 * b11_4; \n",
				"\t\t\t\t\t\tv12 = p10 + s12 * b11_4; \n", "\t\t\t\t\t\tv13 = p18 - s13 * b11_4; \n", "\t\t\t\t\t\tv14 = p12 + s14 * b11_4; \n",
				"\t\t\t\t\t\tv15 = p17 - s15 * b11_5; \n", "\t\t\t\t\t\tv16 = p15 - s16 * b11_5; \n", "\t\t\t\t\t\tv17 = p11 - s17 * b11_5; \n",
				"\t\t\t\t\t\tv18 = p19 - s18 * b11_5; \n", "\t\t\t\t\t\tv19 = p13 - s19 * b11_5; \n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype w0, w1, w2, w3, w4, w5, w6, w7, w8, w9; \n",
				"\t\t\t\t\t\tfptype w10, w11, w12, w13, w14, w15, w16, w17, w18, "
				"w19; \n",
				"\t\t\t\t\t\tw0 = p2 - v0 * b11_2; \n", "\t\t\t\t\t\tw1 = p6 + v1 * b11_2; \n", "\t\t\t\t\t\tw2 = p9 - v2 * b11_2; \n",
				"\t\t\t\t\t\tw3 = p4 + v3 * b11_2; \n", "\t\t\t\t\t\tw4 = p0 - v4 * b11_2; \n", "\t\t\t\t\t\tw5 = p5 - v5 * b11_3; \n",
				"\t\t\t\t\t\tw6 = p3 - v6 * b11_3; \n", "\t\t\t\t\t\tw7 = p7 - v7 * b11_3; \n", "\t\t\t\t\t\tw8 = p8 - v8 * b11_3; \n",
				"\t\t\t\t\t\tw9 = p1 - v9 * b11_3; \n", "\t\t\t\t\t\tw10 = p18 - v10 * b11_2; \n", "\t\t\t\t\t\tw11 = p10 - v11 * b11_2; \n",
				"\t\t\t\t\t\tw12 = p16 - v12 * b11_2; \n", "\t\t\t\t\t\tw13 = p12 + v13 * b11_2; \n", "\t\t\t\t\t\tw14 = p14 + v14 * b11_2; \n",
				"\t\t\t\t\t\tw15 = p13 - v15 * b11_3; \n", "\t\t\t\t\t\tw16 = p17 - v16 * b11_3; \n", "\t\t\t\t\t\tw17 = p19 - v17 * b11_3; \n",
				"\t\t\t\t\t\tw18 = p15 - v18 * b11_3; \n", "\t\t\t\t\t\tw19 = p11 - v19 * b11_3; \n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype z0, z1, z2, z3, z4, z5, z6, z7, z8, z9; \n", "\t\t\t\t\t\tz0 = (*R0).x - w5 * b11_1; \n",
				"\t\t\t\t\t\tz1 = (*R0).x - w6 * b11_1; \n", "\t\t\t\t\t\tz2 = (*R0).x - w7 * b11_1; \n", "\t\t\t\t\t\tz3 = (*R0).x - w8 * b11_1; \n",
				"\t\t\t\t\t\tz4 = (*R0).x - w9 * b11_1; \n", "\t\t\t\t\t\tz5 = (*R0).y - w15 * b11_1; \n", "\t\t\t\t\t\tz6 = (*R0).y - w16 * b11_1; \n",
				"\t\t\t\t\t\tz7 = (*R0).y - w17 * b11_1; \n", "\t\t\t\t\t\tz8 = (*R0).y - w18 * b11_1; \n", "\t\t\t\t\t\tz9 = (*R0).y - w19 * b11_1; \n",
				"\t\t\t\t\t\t\n", "\t\t\t\t\t\t(*R0).x = (*R0).x + p1 + p3 + p5 + p7 + p8; \n", "\t\t\t\t\t\t(*R0).y = (*R0).y + p11 + p13 + p15 + p17 + p19; \n",
				"\t\t\t\t\t\t(*R1).x = z1 + w14* b11_0; \n", "\t\t\t\t\t\t(*R1).y = z7 + w1* b11_0; \n", "\t\t\t\t\t\t(*R2).x = z2 - w12* b11_0; \n",
				"\t\t\t\t\t\t(*R2).y = z8 - w2* b11_0; \n", "\t\t\t\t\t\t(*R3).x = z0 + w11* b11_0; \n", "\t\t\t\t\t\t(*R3).y = z5 + w4* b11_0; \n",
				"\t\t\t\t\t\t(*R4).x = z3 - w13* b11_0; \n", "\t\t\t\t\t\t(*R4).y = z6 - w3* b11_0; \n", "\t\t\t\t\t\t(*R5).x = z4 + w10* b11_0; \n",
				"\t\t\t\t\t\t(*R5).y = z9 + w0* b11_0; \n", "\t\t\t\t\t\t(*R6).x = z4 - w10* b11_0; \n", "\t\t\t\t\t\t(*R6).y = z9 - w0* b11_0; \n",
				"\t\t\t\t\t\t(*R7).x = z3 + w13* b11_0; \n", "\t\t\t\t\t\t(*R7).y = z6 + w3* b11_0; \n", "\t\t\t\t\t\t(*R8).x = z0 - w11* b11_0; \n",
				"\t\t\t\t\t\t(*R8).y = z5 - w4* b11_0; \n", "\t\t\t\t\t\t(*R9).x = z2 + w12* b11_0; \n", "\t\t\t\t\t\t(*R9).y = z8 + w2* b11_0; \n",
				"\t\t\t\t\t\t(*R10).x = z1 - w14* b11_0; \n", "\t\t\t\t\t\t(*R10).y = z7 - w1* b11_0; \n", NULL };

			if (butterfly->fwd)
				bufcatcstr(bflyStr, "fptype dir = -1;\n\n");
			else
				bufcatcstr(bflyStr, "fptype dir = 1;\n\n");

			// Append each radix 11 source fragment without relying on long string
			// literal concatenation.
			for (size_t radix11Index = 0; radix11str[radix11Index] != NULL; ++radix11Index)
				bufcatcstr(bflyStr, radix11str[radix11Index]);
		}
		break;
		case 13:
		{
			static const char *radix13str[] = { " \t\t\t\t\t\tfptype p0, p1, p2, p3, p4, p5, p6, p7, p8, p9;\n", "\t\t\t\t\t\tp0 = (*R7).x - (*R2).x;\n",
				"\t\t\t\t\t\tp1 = (*R7).x + (*R2).x;\n", "\t\t\t\t\t\tp2 = (*R8).x - (*R5).x;\n", "\t\t\t\t\t\tp3 = (*R8).x + (*R5).x;\n",
				"\t\t\t\t\t\tp4 = (*R9).x - (*R3).x;\n", "\t\t\t\t\t\tp5 = (*R3).x + (*R9).x;\n", "\t\t\t\t\t\tp6 = (*R10).x + (*R4).x;\n",
				"\t\t\t\t\t\tp7 = (*R10).x - (*R4).x;\n", "\t\t\t\t\t\tp8 = (*R11).x + (*R6).x;\n", "\t\t\t\t\t\tp9 = (*R11).x - (*R6).x;\n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype p10, p11, p12, p13, p14, p15, p16, p17, p18, "
				"p19;\n",
				"\t\t\t\t\t\tp10 = (*R12).x + p6;\n", "\t\t\t\t\t\tp11 = (*R1).x + p5;\n", "\t\t\t\t\t\tp12 = p8 - p1;\n", "\t\t\t\t\t\tp13 = p8 + p1;\n",
				"\t\t\t\t\t\tp14 = p9 + p0;\n", "\t\t\t\t\t\tp15 = p9 - p0;\n", "\t\t\t\t\t\tp16 = p7 - p4;\n", "\t\t\t\t\t\tp17 = p4 + p7;\n",
				"\t\t\t\t\t\tp18 = p11 + p10;\n", "\t\t\t\t\t\tp19 = p11 - p10;\n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, "
				"s11;\n",
				"\t\t\t\t\t\ts0 = p3 + p13;\n", "\t\t\t\t\t\ts1 = p2 + p14;\n", "\t\t\t\t\t\ts2 = p16 - p15;\n", "\t\t\t\t\t\ts3 = p16 + p15;\n",
				"\t\t\t\t\t\ts4 = -(*R12).x + p6 * b13_17;\n", "\t\t\t\t\t\ts5 =   (*R1).x - p5 * b13_17;\n", "\t\t\t\t\t\ts6 = s5 - s4;\n",
				"\t\t\t\t\t\ts7 = s5 + s4;\n", "\t\t\t\t\t\ts8 = p18 + s0;\n", "\t\t\t\t\t\ts9 = p18 - s0;\n", "\t\t\t\t\t\tfptype c2 = p3 - p13 * b13_17;\n",
				"\t\t\t\t\t\ts10 = s6 - c2;\n", "\t\t\t\t\t\ts11 = s6 + c2;\n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, "
				"r11;\n",
				"\t\t\t\t\t\tr0 = (*R7).y + (*R2).y;\n", "\t\t\t\t\t\tr1 = (*R7).y - (*R2).y;\n", "\t\t\t\t\t\tr2 = (*R8).y + (*R5).y;\n",
				"\t\t\t\t\t\tr3 = (*R8).y - (*R5).y;\n", "\t\t\t\t\t\tr4 = (*R9).y - (*R3).y;\n", "\t\t\t\t\t\tr5 = (*R3).y + (*R9).y;\n",
				"\t\t\t\t\t\tr6 = (*R10).y + (*R4).y;\n", "\t\t\t\t\t\tr7 = (*R10).y - (*R4).y;\n", "\t\t\t\t\t\tr8 = (*R11).y - (*R6).y;\n",
				"\t\t\t\t\t\tr9 = (*R11).y + (*R6).y;\n", "\t\t\t\t\t\tr10 = (*R12).y + r6;\n", "\t\t\t\t\t\tr11 = (*R1).y + r5;\n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10;\n",
				"\t\t\t\t\t\tfptype m11, m12, m13, m14, m15, m16, m17, m18, m19, "
				"m20;\n",
				"\t\t\t\t\t\tm0 = r4 + r7;\n", "\t\t\t\t\t\tm1 = r7 - r4;\n", "\t\t\t\t\t\tm2 = r8 - r1;\n", "\t\t\t\t\t\tm3 = r8 + r1;\n",
				"\t\t\t\t\t\tm4 = r9 + r0;\n", "\t\t\t\t\t\tm5 = r9 - r0;\n", "\t\t\t\t\t\tm6 = r11 + r10;\n", "\t\t\t\t\t\tm7 = r11 - r10;\n",
				"\t\t\t\t\t\tm8 = m1 - m2;\n", "\t\t\t\t\t\tm9 = m1 + m2;\n", "\t\t\t\t\t\tm10 = r3 + m3;\n", "\t\t\t\t\t\tm11 = r2 + m4;\n",
				"\t\t\t\t\t\tm12 = m6 - m11;\n", "\t\t\t\t\t\tm13 = m6 + m11;\n", "\t\t\t\t\t\t\n", "\t\t\t\t\t\tm14 =  (*R1).y - r5 * b13_17;\n",
				"\t\t\t\t\t\tm15 = -(*R12).y + r6 * b13_17;\n", "\t\t\t\t\t\tm16 =  r2      - m4 * b13_17;\n", "\t\t\t\t\t\t\n", "\t\t\t\t\t\tm17 = m14 + m15;\n",
				"\t\t\t\t\t\tm18 = m14 - m15;\n", "\t\t\t\t\t\tm19 = m18 + m16;\n", "\t\t\t\t\t\tm20 = m18 - m16;\n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype c0, c1, c3, c4, c5, c6, c7, c8, c9;\n",
				"\t\t\t\t\t\tfptype c10, c11, c12, c13, c14, c15, c16, c17, c18, "
				"c19;\n",
				"\t\t\t\t\t\tfptype c20, c21, c22, c23, c24;\n", "\t\t\t\t\t\tc0  =  s7 - p12 * b13_3;\n", "\t\t\t\t\t\tc1  =  s7 + p12 * b13_3;\n",
				"\t\t\t\t\t\tc3  =  p2 - p14 * b13_17;\n", "\t\t\t\t\t\tc4  =  s1 - p19 * b13_18;\n", "\t\t\t\t\t\tc5  = p19 + s1 * b13_18;\n",
				"\t\t\t\t\t\tc6  = s10 - s2 * b13_15;\n", "\t\t\t\t\t\tc7  = s11 - s3 * b13_22;\n", "\t\t\t\t\t\tc8  = (*R0).x - s8 * b13_23;\n",
				"\t\t\t\t\t\tc9  =  s2 + s10 * b13_7;\n", "\t\t\t\t\t\tc10 =  s3 + s11 * b13_19;\n", "\t\t\t\t\t\tc11 =  r3 - m3 * b13_17;\n",
				"\t\t\t\t\t\tc12 = m17 - m5 * b13_3;\n", "\t\t\t\t\t\tc13 = m17 + m5 * b13_3;\n", "\t\t\t\t\t\tc14 = m10 - m7 * b13_18;\n",
				"\t\t\t\t\t\tc15 = m20 - m8 * b13_15;\n", "\t\t\t\t\t\tc16 = m19 - m9 * b13_22;\n", "\t\t\t\t\t\tc17 =  m7 + m10 * b13_18;\n",
				"\t\t\t\t\t\tc18 = (*R0).y- m13 * b13_23;\n", "\t\t\t\t\t\tc19 =  m9 + m19 * b13_19;\n", "\t\t\t\t\t\tc20 =  m8 + m20 * b13_7;\n",
				"\t\t\t\t\t\tc21 =  c3 + p17 * b13_3;\n", "\t\t\t\t\t\tc22 =  c3 - p17 * b13_3;\n", "\t\t\t\t\t\tc23 = c11 + m0 * b13_3;\n",
				"\t\t\t\t\t\tc24 = c11 - m0 * b13_3;\n", "\t\t\t\t\t\t\n", "\t\t\t\t\t\tfptype d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;\n",
				"\t\t\t\t\t\tfptype d10, d11, d12, d13, d14, d15, d16, d17, d18, "
				"d19;\n",
				"\t\t\t\t\t\td0  = c22 +  c0 * b13_8;\n", "\t\t\t\t\t\td1  =  c0 - c22 * b13_8;\n", "\t\t\t\t\t\td2  = c21 +  c1 * b13_24;\n",
				"\t\t\t\t\t\td3  =  c1 - c21 * b13_24;\n", "\t\t\t\t\t\td4  =  s9 -  c6 * b13_4;\n", "\t\t\t\t\t\td5  =  c6 +  s9 * b13_10;\n",
				"\t\t\t\t\t\td6  =  c7 +  c9 * b13_6;\n", "\t\t\t\t\t\td7  =  c7 -  c9 * b13_6;\n", "\t\t\t\t\t\td8  =  c8 - c10 * b13_21;\n",
				"\t\t\t\t\t\td9  =  c8 + c10 * b13_16;\n", "\t\t\t\t\t\td10 = c24 + c12 * b13_8;\n", "\t\t\t\t\t\td11 = c12 - c24 * b13_8;\n",
				"\t\t\t\t\t\td12 = c23 + c13 * b13_24;\n", "\t\t\t\t\t\td13 = c13 - c23 * b13_24;\n", "\t\t\t\t\t\td14 = m12 - c15 * b13_4;\n",
				"\t\t\t\t\t\td15 = c15 + m12 * b13_10;\n", "\t\t\t\t\t\td16 = c18 + c19 * b13_16;\n", "\t\t\t\t\t\td17 = c18 - c19 * b13_21;\n",
				"\t\t\t\t\t\td18 = c16 - c20 * b13_6;\n", "\t\t\t\t\t\td19 = c16 + c20 * b13_6;\n", "\t\t\t\t\t\t\n",
				"\t\t\t\t\t\tfptype e0, e1, e2, e3, e4, e5, e6, e7, e8, e9;\n", "\t\t\t\t\t\tfptype e10, e11, e12, e13, e14, e15;\n",
				"\t\t\t\t\t\te0  = d2  +  d0 * b13_5;\n", "\t\t\t\t\t\te1  = d2  -  d0 * b13_5;\n", "\t\t\t\t\t\te2  = d3  -  d1 * b13_5;\n",
				"\t\t\t\t\t\te3  = d3  +  d1 * b13_5;\n", "\t\t\t\t\t\te4  = d8  -  d4 * b13_20;\n", "\t\t\t\t\t\te5  = d8  +  d4 * b13_20;\n",
				"\t\t\t\t\t\te6  = d9  +  d5 * b13_14;\n", "\t\t\t\t\t\te7  = d9  -  d5 * b13_14;\n", "\t\t\t\t\t\te8  = d12 + d10 * b13_5;\n",
				"\t\t\t\t\t\te9  = d12 - d10 * b13_5;\n", "\t\t\t\t\t\te10 = d13 - d11 * b13_5;\n", "\t\t\t\t\t\te11 = d13 + d11 * b13_5;\n",
				"\t\t\t\t\t\te12 = d16 + d15 * b13_14;\n", "\t\t\t\t\t\te13 = d16 - d15 * b13_14;\n", "\t\t\t\t\t\te14 = d17 + d14 * b13_20;\n",
				"\t\t\t\t\t\te15 = d17 - d14 * b13_20;\n", "\t\t\t\t\t\t\n", "\t\t\t\t\t\tfptype f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;\n",
				"\t\t\t\t\t\tfptype f10, f11, f12, f13, f14, f15, f16, f17, f18, "
				"f19;\n",
				"\t\t\t\t\t\tfptype f20, f21, f22, f23;\n", "\t\t\t\t\t\tf0  = c17 - e10 * b13_12;\n", "\t\t\t\t\t\tf1  = e10 + c17 * b13_1;\n",
				"\t\t\t\t\t\tf2  = e9  + c14 * b13_1;\n", "\t\t\t\t\t\tf3  = c14 -  e9 * b13_12;\n", "\t\t\t\t\t\tf4  = e11 + dir * d7 * b13_0;\n",
				"\t\t\t\t\t\tf5  = e11 - dir * d7 * b13_0;\n", "\t\t\t\t\t\tf6  = e5  + dir * f3 * b13_11;\n", "\t\t\t\t\t\tf7  = e5  - dir * f3 * b13_11;\n",
				"\t\t\t\t\t\tf8  = e4  + dir * e8 * b13_13;\n", "\t\t\t\t\t\tf9  = e4  - dir * e8 * b13_13;\n", "\t\t\t\t\t\tf10 = f0  - dir * d6 * b13_2;\n",
				"\t\t\t\t\t\tf11 = f0  + dir * d6 * b13_2;\n", "\t\t\t\t\t\tf12 = e1  +  c4 * b13_1;\n", "\t\t\t\t\t\tf13 = c4  -  e1 * b13_12;\n",
				"\t\t\t\t\t\tf14 = c5  -  e2 * b13_12;\n", "\t\t\t\t\t\tf15 = e2  +  c5 * b13_1;\n", "\t\t\t\t\t\tf16 = f14 + dir * d19 * b13_2;\n",
				"\t\t\t\t\t\tf17 = f14 - dir * d19 * b13_2;\n", "\t\t\t\t\t\tf18 = e15 - dir *  e0 * b13_13;\n", "\t\t\t\t\t\tf19 = e15 + dir *  e0 * b13_13;\n",
				"\t\t\t\t\t\tf20 = e14 - dir * f13 * b13_11;\n", "\t\t\t\t\t\tf21 = e14 + dir * f13 * b13_11;\n", "\t\t\t\t\t\tf22 = e3  - dir * d18 * b13_0;\n",
				"\t\t\t\t\t\tf23 = e3  + dir * d18 * b13_0;\n", "\t\t\t\t\t\t\n", "\t\t\t\t\t\t(*R0).x  = (*R0).x + s8;\n",
				"\t\t\t\t\t\t(*R0).y  = (*R0).y + m13;\n", "\t\t\t\t\t\t(*R1).x  =  e6 +  f2 * dir * b13_9 ;\n",
				"\t\t\t\t\t\t(*R1).y  = e12 - f12 * dir * b13_9 ;\n", "\t\t\t\t\t\t(*R2).x  =  f9 - f10 * dir * b13_11;\n",
				"\t\t\t\t\t\t(*R2).y  = f19 + f16 * dir * b13_11;\n", "\t\t\t\t\t\t(*R3).x  =  f6 -  f5 * dir * b13_13;\n",
				"\t\t\t\t\t\t(*R3).y  = f20 + f23 * dir * b13_13;\n", "\t\t\t\t\t\t(*R4).x  =  f7 -  f4 * dir * b13_13;\n",
				"\t\t\t\t\t\t(*R4).y  = f21 + f22 * dir * b13_13;\n", "\t\t\t\t\t\t(*R5).x  =  e7 -  f1 * dir * b13_9 ;\n",
				"\t\t\t\t\t\t(*R5).y  = e13 + f15 * dir * b13_9 ;\n", "\t\t\t\t\t\t(*R6).x  =  f8 - f11 * dir * b13_11;\n",
				"\t\t\t\t\t\t(*R6).y  = f18 + f17 * dir * b13_11;\n", "\t\t\t\t\t\t(*R7).x  =  f9 + f10 * dir * b13_11;\n",
				"\t\t\t\t\t\t(*R7).y  = f19 - f16 * dir * b13_11;\n", "\t\t\t\t\t\t(*R8).x  =  e7 +  f1 * dir * b13_9 ;\n",
				"\t\t\t\t\t\t(*R8).y  = e13 - f15 * dir * b13_9 ;\n", "\t\t\t\t\t\t(*R9).x  =  f6 +  f5 * dir * b13_13;\n",
				"\t\t\t\t\t\t(*R9).y  = f20 - f23 * dir * b13_13;\n", "\t\t\t\t\t\t(*R10).x =  f7 +  f4 * dir * b13_13;\n",
				"\t\t\t\t\t\t(*R10).y = f21 - f22 * dir * b13_13;\n", "\t\t\t\t\t\t(*R11).x =  f8 + f11 * dir * b13_11;\n",
				"\t\t\t\t\t\t(*R11).y = f18 - f17 * dir * b13_11;\n", "\t\t\t\t\t\t(*R12).x =  e6 -  f2 * dir * b13_9 ;\n",
				"\t\t\t\t\t\t(*R12).y = e12 + f12 * dir * b13_9 ;\n", NULL };

			if (butterfly->fwd)
				bufcatcstr(bflyStr, "fptype dir = -1;\n\n");
			else
				bufcatcstr(bflyStr, "fptype dir = 1;\n\n");

			// Append each radix 13 source fragment without relying on long string
			// literal concatenation.
			for (size_t radix13Index = 0; radix13str[radix13Index] != NULL; ++radix13Index)
				bufcatcstr(bflyStr, radix13str[radix13Index]);
		}
		break;

		default: assert(false);
	}

	bufcatcstr(bflyStr, "\n\t");

	// Assign results
	if ((butterfly->radix & (butterfly->radix - 1)) || (!butterfly->cReg))
	{
		if ((butterfly->radix != 10) && (butterfly->radix != 6))
		{
			for (size_t i = 0; i < butterfly->radix; i++)
			{
				if (butterfly->cReg)
				{
					if ((butterfly->radix != 7) && (butterfly->radix != 11) && (butterfly->radix != 13))
					{
						bufcatcstr(bflyStr, "((*R");
						BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
						bufcatcstr(bflyStr, ").x) = TR");
						BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
						bufcatcstr(bflyStr, "; ");
						bufcatcstr(bflyStr, "((*R");
						BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
						bufcatcstr(bflyStr, ").y) = TI");
						BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
						bufcatcstr(bflyStr, ";\n\t");
					}
				}
				else
				{
					bufcatcstr(bflyStr, "(*R");
					BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
					bufcatcstr(bflyStr, ") = TR");
					BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
					bufcatcstr(bflyStr, "; ");
					bufcatcstr(bflyStr, "(*I");
					BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
					bufcatcstr(bflyStr, ") = TI");
					BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
					bufcatcstr(bflyStr, ";\n\t");
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < butterfly->radix; i++)
		{
			size_t j = ButterflyBitReverse(i, butterfly->radix);

			if (i < j)
			{
				bufcatcstr(bflyStr, "T = (*R");
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
				bufcatcstr(bflyStr, "); (*R");
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(i));
				bufcatcstr(bflyStr, ") = (*R");
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(j));
				bufcatcstr(bflyStr, "); (*R");
				BUFCAT_BUFFER_VALUE(bflyStr, SztToStr(j));
				bufcatcstr(bflyStr, ") = T;\n\t");
			}
		}
	}

	bufcatcstr(bflyStr, "\n}\n");
}

static inline void ButterflyGenerateButterfly(const Butterfly *butterfly, buffer_t *bflyStr)
{
	// Emit a butterfly body only when the requested register group count is
	// valid.
	assert(butterfly->count <= 4);
	if (butterfly->count > 0)
		ButterflyGenerateButterflyStr(butterfly, bflyStr);
}

static inline void ButterflyInit(Butterfly *butterfly, Precision prVal, size_t radixVal, size_t countVal, bool fwdVal, bool cRegVal)
{
	// Initialize butterfly generation parameters explicitly.
	butterfly->pr = prVal;
	butterfly->radix = radixVal;
	butterfly->count = countVal;
	butterfly->fwd = fwdVal;
	butterfly->cReg = cRegVal;
}

/* End inlined header: src\library\generator.stockham.h */

/* Begin inlined header: src\library\generator.transpose.h */

#if !defined(AMD_CLFFT_GENERATOR_TRANSPOSE_HEADER)
  #define AMD_CLFFT_GENERATOR_TRANSPOSE_HEADER

  #define AVAIL_MEM_SIZE 32768

static inline void clKernWrite(buffer_stream_t *rhs, const size_t tabIndex)
{
	// Emit indentation spaces into the kernel source buffer.
	for (size_t i = 0; i < tabIndex; ++i)
		bufstream_cat_char(rhs, ' ');
}

// generate transepose kernel with sqaure 2d matrix of row major with arbitrary
// batch size
/*
Below is a matrix(row major) containing three sqaure sub matrix along column
The transpose will be done within each sub matrix.
[M0
M1
M2]
*/
static clfftStatus clfft_transpose_generator_genTransposeKernelBatched(const FFTKernelGenKeyParams params, buffer_t *strKernel, size_t lwSize, const size_t reShapeFactor);

// generate transpose kernel with square 2d matrix of row major with blocks
// along the leading dimension aka leading dimension batched
/*
Below is a matrix(row major) contaning three square sub matrix along row
[M0 M2 M2]
*/
static clfftStatus clfft_transpose_generator_genTransposeKernelLeadingDimensionBatched(const FFTKernelGenKeyParams params, buffer_t *strKernel, size_t lwSize,
	const size_t reShapeFactor);

// swap lines. a more general kernel generator.
// this function accepts any ratio in theory. But in practice we restrict it to
// 1:2, 1:3, 1:5 and 1:10 ration
static clfftStatus clfft_transpose_generator_genSwapKernelGeneral(const FFTKernelGenKeyParams params, buffer_t *strKernel, buffer_t *KernelFuncName, size_t lwSize,
	const size_t reShapeFactor);

#endif
/* End inlined header: src\library\generator.transpose.h */

#define clfftSetPlanLength amalgamatedSetPlanLength
#define clfftCreateDefaultPlanInternal amalgamatedCreateDefaultPlanInternal
#define clfftInitRequestLibNoMemAlloc amalgamatedInitRequestLibNoMemAlloc
#define clfftGetRequestLibNoMemAlloc amalgamatedGetRequestLibNoMemAlloc
#define getKernelName amalgamatedGetKernelName

static void amalgamatedInitRequestLibNoMemAlloc(void);
static bool amalgamatedGetRequestLibNoMemAlloc(void);
static buffer_t amalgamatedGetKernelName(const clfftGenerators gen, const clfftPlanHandle plHandle, bool withPlHandle);

static bool amalgamatedRequestNoMemAlloc = false;

static void amalgamatedInitRequestLibNoMemAlloc(void)
{
	// Refresh the planner memory-allocation request from the environment.
	const char *val = getenv("CLFFT_REQUEST_LIB_NOMEMALLOC");
	amalgamatedRequestNoMemAlloc = (val != NULL);
}

static bool amalgamatedGetRequestLibNoMemAlloc(void)
{
	// Return the cached environment request used by the planner.
	return amalgamatedRequestNoMemAlloc;
}
/* Begin copied source: src\library\accessors.cpp */

// clfft.accessors.cpp : Defines all the getters/setters for the Plan
//

clfftStatus clfftSetPlanPrecision(clfftPlanHandle plHandle, clfftPrecision precision)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	if (precision >= ENDPRECISION)
		return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

	//	We do not support CLFFT_*_FAST currently
	if (precision == CLFFT_SINGLE_FAST || precision == CLFFT_DOUBLE_FAST)
		return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);

	//	If we modify the state of the plan, we assume that we can't trust any
	// pre-calculated contents anymore
	fftPlan->baked = false;
	fftPlan->precision = precision;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

clfftStatus clfftSetPlanScale(clfftPlanHandle plHandle, clfftDirection dir, cl_float scale)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	if (dir >= ENDDIRECTION)
		return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

	//	If we modify the state of the plan, we assume that we can't trust any
	// pre-calculated contents anymore
	fftPlan->baked = false;

	if (dir == CLFFT_FORWARD || dir == CLFFT_MINUS)
		fftPlan->forwardScale = scale;
	else
		fftPlan->backwardScale = scale;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus clfftSetPlanLength(clfftPlanHandle plHandle, const clfftDim dim, const size_t *clLengths)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	if (clLengths == NULL)
		return clfftReturnLocked(sLock, CLFFT_INVALID_HOST_PTR);

	//	Simplest to clear any previous contents, because it's valid for user to
	// shrink dimension
	array_clear(&fftPlan->length);
	switch (dim)
	{
		case CLFFT_1D:
		{
			//	Minimum length size is 1
			if (clLengths[DimX] == 0)
				return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

			if (!IsASupportedLength(clLengths[DimX]))
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);

			array_push_back(&fftPlan->length, clLengths[DimX]);
		}
		break;
		case CLFFT_2D:
		{
			//	Minimum length size is 1
			if (clLengths[DimX] == 0 || clLengths[DimY] == 0)
				return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

			if (!fftPlan->transflag)
			{
				if (!IsASupportedLength(clLengths[DimX]) || !IsASupportedLength(clLengths[DimY]))
				{
					return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
				}
			}

			array_push_back(&fftPlan->length, clLengths[DimX]);
			array_push_back(&fftPlan->length, clLengths[DimY]);
		}
		break;
		case CLFFT_3D:
		{
			//	Minimum length size is 1
			if (clLengths[DimX] == 0 || clLengths[DimY] == 0 || clLengths[DimZ] == 0)
				return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

			if (!IsASupportedLength(clLengths[DimX]) || !IsASupportedLength(clLengths[DimY]) || !IsASupportedLength(clLengths[DimZ]))
			{
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
			}

			array_push_back(&fftPlan->length, clLengths[DimX]);
			array_push_back(&fftPlan->length, clLengths[DimY]);
			array_push_back(&fftPlan->length, clLengths[DimZ]);
		}
		break;
		default: return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED); break;
	}

	fftPlan->dim = dim;

	//	If we modify the state of the plan, we assume that we can't trust any
	// pre-calculated contents anymore
	fftPlan->baked = false;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

clfftStatus clfftSetLayout(clfftPlanHandle plHandle, clfftLayout iLayout, clfftLayout oLayout)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	//	Basic error checking on parameter
	if ((iLayout >= ENDLAYOUT) || (oLayout >= ENDLAYOUT))
		return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

	//	We currently only support a subset of formats
	switch (iLayout)
	{
		case CLFFT_COMPLEX_INTERLEAVED:
		{
			if ((oLayout == CLFFT_HERMITIAN_INTERLEAVED) || (oLayout == CLFFT_HERMITIAN_PLANAR) || (oLayout == CLFFT_REAL))
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
		}
		break;
		case CLFFT_COMPLEX_PLANAR:
		{
			if ((oLayout == CLFFT_HERMITIAN_INTERLEAVED) || (oLayout == CLFFT_HERMITIAN_PLANAR) || (oLayout == CLFFT_REAL))
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
		}
		break;
		case CLFFT_HERMITIAN_INTERLEAVED:
		{
			if (oLayout != CLFFT_REAL)
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
		}
		break;
		case CLFFT_HERMITIAN_PLANAR:
		{
			if (oLayout != CLFFT_REAL)
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
		}
		break;
		case CLFFT_REAL:
		{
			if ((oLayout == CLFFT_REAL) || (oLayout == CLFFT_COMPLEX_INTERLEAVED) || (oLayout == CLFFT_COMPLEX_PLANAR))
				return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
		}
		break;
		default: return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED); break;
	}

	//	We currently only support a subset of formats
	switch (oLayout)
	{
		case CLFFT_COMPLEX_PLANAR:
		case CLFFT_COMPLEX_INTERLEAVED:
		case CLFFT_HERMITIAN_INTERLEAVED:
		case CLFFT_HERMITIAN_PLANAR:
		case CLFFT_REAL: break;
		default: return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED); break;
	}

	//	If we modify the state of the plan, we assume that we can't trust any
	// pre-calculated contents anymore
	fftPlan->baked = false;
	fftPlan->inputLayout = iLayout;
	fftPlan->outputLayout = oLayout;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

clfftStatus clfftSetResultLocation(clfftPlanHandle plHandle, clfftResultLocation placeness)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	//	Basic error checking on parameter
	if (placeness >= ENDPLACE)
		return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);

	//	If we modify the state of the plan, we assume that we can't trust any
	// pre-calculated contents anymore
	fftPlan->baked = false;
	fftPlan->placeness = placeness;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

clfftStatus clfftGetTmpBufSize(const clfftPlanHandle plHandle, size_t *buffersize)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	if (fftPlan->baked == true)
	{
		*buffersize = fftPlan->tmpBufSize;
		return clfftReturnLocked(sLock, CLFFT_SUCCESS);
	}

	return clfftReturnLocked(sLock, CLFFT_INVALID_OPERATION);
}

/* End copied source: src\library\accessors.cpp */

/* Begin copied source: src\library\repo.cpp */

// clfft.repo.cpp : Defines the entry point for the console application.
//

//	Static initialization of the plan count variable

static clfftStatus FFTRepoReleaseResources(FFTRepo *repo)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	//	Release all handles to Kernels
	//
	for (KernelEntryPtr iKern = ARRAY_MAP_BEGIN(repo->mapKernels); iKern != ARRAY_MAP_END(repo->mapKernels); ++iKern)
	{
		cl_kernel k = iKern->second.kernel_fwd;
		iKern->second.kernel_fwd = NULL;
		if (NULL != k)
			clReleaseKernel(k);
		k = iKern->second.kernel_back;
		iKern->second.kernel_back = NULL;
		if (NULL != k)
			clReleaseKernel(k);

		if (NULL != iKern->second.kernel_fwd_lock)
		{
			lockRAIIDestroy(iKern->second.kernel_fwd_lock);
			iKern->second.kernel_fwd_lock = NULL;
		}

		if (NULL != iKern->second.kernel_back_lock)
		{
			lockRAIIDestroy(iKern->second.kernel_back_lock);
			iKern->second.kernel_back_lock = NULL;
		}
	}
	array_clear(&repo->mapKernels.entries);

	//	Release all handles to programs
	//
	for (fftRepoEntryPtr iProg = ARRAY_MAP_BEGIN(repo->mapFFTs); iProg != ARRAY_MAP_END(repo->mapFFTs); ++iProg)
	{
		if (iProg->first.data != NULL)
			FFTRepoKeyDeleteData((FFTRepoKey *) (&iProg->first));

		cl_program p = iProg->second.clProgram;
		iProg->second.clProgram = NULL;
		if (NULL != p)
			clReleaseProgram(p);

		// Release owned cached kernel strings before clearing the entry.
		FFTRepoValueFree(&iProg->second);
	}

	//	Free all memory allocated in the repoPlans; represents cached plans that
	// were not destroyed by the client
	//
	// Walk repository plans with raw map-entry pointers.
	repoPlansEntryPtr repoPlans_end = ARRAY_MAP_END(repo->repoPlans);
	for (repoPlansEntryPtr iter = repo->repoPlans.entries.buf; iter != repoPlans_end; ++iter)
	{
		FFTPlan *plan = iter->second.first;
		lockRAII *lock = iter->second.second;
		if (plan != NULL)
			FFTPlanDestroy(plan);
		if (lock != NULL)
		{
			// Destroy the heap-owned plan lock through the explicit helper.
			lockRAIIDestroy(lock);
		}
	}

	//	Reset the plan count to zero because we are guaranteed to have destroyed
	// all plans
	FFTRepoPlanCount = 1;

	//	Release all strings
	array_clear(&repo->mapFFTs.entries);

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoSetProgramCode(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, const buffer_t *kernel, cl_device_id device,
	cl_context planContext)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	FFTRepoKey key = { gen, data, planContext, device, false };

	FFTRepoKeyPrivatizeData(&key);

	// Prefix copyright statement at the top of generated kernels
	buffer_stream_t ss;
	// Initialize stream formatting state explicitly.
	bufstream_init(&ss);
	bufstream_cat_cstr(&ss,
		"/* "
		"**********************************************************************"
		"**\n"
		" * Copyright 2013 Advanced Micro Devices, Inc.\n"
		" *\n"
		" * Licensed under the Apache License, Version 2.0 (the \"License\");\n"
		" * you may not use this file except in compliance with the License.\n"
		" * You may obtain a copy of the License at\n"
		" *\n"
		" * http://www.apache.org/licenses/LICENSE-2.0\n"
		" *\n"
		" * Unless required by applicable law or agreed to in writing, "
		"software\n"
		" * distributed under the License is distributed on an \"AS IS\" "
		"BASIS,\n"
		" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
		"implied.\n"
		" * See the License for the specific language governing permissions "
		"and\n"
		" * limitations under the License.\n"
		" * "
		"**********************************************************************"
		"**/");
	bufprintf(&ss.text, "\n\n");

	buffer_t prefixCopyright = buffer_copy(&ss.text);

	fftRepoEntryPtr it = fftRepoFind(&repo->mapFFTs, key);
	if (it == ARRAY_MAP_END(repo->mapFFTs))
	{
		// Prefix the generated kernel source with the embedded license text.
		buffer_t programString = buffer_empty();
		bufsetbuf(&programString, &prefixCopyright);
		bufcatbuf(&programString, kernel);
		bufsetbuf(&fftRepoGet(&repo->mapFFTs, key)->ProgramString, &programString);
	}
	else
		FFTRepoKeyDeleteData(&key);

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoGetProgramCode(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, buffer_t *kernel, cl_device_id device, cl_context planContext)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	FFTRepoKey key = { gen, data, planContext, device, false };

	fftRepoEntryPtr pos = fftRepoFind(&repo->mapFFTs, key);
	if (pos == ARRAY_MAP_END(repo->mapFFTs))
		return clfftReturnLocked(sLock, CLFFT_FILE_NOT_FOUND);

	bufsetbuf(kernel, &pos->second.ProgramString);
	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoSetProgramEntryPoints(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, const char *kernel_fwd, const char *kernel_back,
	cl_device_id device, cl_context planContext)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	FFTRepoKey key = { gen, data, planContext, device, false };

	fftRepoValue *fft = fftRepoGet(&repo->mapFFTs, key);
	bufsetcstr(&fft->EntryPoint_fwd, kernel_fwd);
	bufsetcstr(&fft->EntryPoint_back, kernel_back);

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoGetProgramEntryPoint(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, clfftDirection dir, buffer_t *kernel, cl_device_id device,
	cl_context planContext)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	FFTRepoKey key = { gen, data, planContext, device, false };

	fftRepoEntryPtr pos = fftRepoFind(&repo->mapFFTs, key);
	if (pos == ARRAY_MAP_END(repo->mapFFTs))
		return clfftReturnLocked(sLock, CLFFT_FILE_NOT_FOUND);

	switch (dir)
	{
		case CLFFT_FORWARD: bufsetbuf(kernel, &pos->second.EntryPoint_fwd); break;
		case CLFFT_BACKWARD: bufsetbuf(kernel, &pos->second.EntryPoint_back); break;
		default: assert(false); return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);
	}

	if (kernel == NULL || 0 == kernel->len)
		return clfftReturnLocked(sLock, CLFFT_FILE_NOT_FOUND);

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoSetclProgram(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, cl_program prog, cl_device_id device, cl_context planContext)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	FFTRepoKey key = { gen, data, planContext, device, false };

	fftRepoEntryPtr pos = fftRepoFind(&repo->mapFFTs, key);
	if (pos == ARRAY_MAP_END(repo->mapFFTs))
	{
		FFTRepoKeyPrivatizeData(&key); // the key owns the data
		fftRepoGet(&repo->mapFFTs, key)->clProgram = prog;
	}
	else
	{
		cl_program p = pos->second.clProgram;
		assert(NULL == p);
		if (NULL != p)
			clReleaseProgram(p);
		pos->second.clProgram = prog;
	}

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoGetclProgram(FFTRepo *repo, clfftGenerators gen, const FFTKernelSignatureHeader *data, cl_program *prog, cl_device_id device, cl_context planContext)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	FFTRepoKey key = { gen, data, planContext, device, false };

	fftRepoEntryPtr pos = fftRepoFind(&repo->mapFFTs, key);
	if (pos == ARRAY_MAP_END(repo->mapFFTs))
		return clfftReturnLocked(sLock, CLFFT_INVALID_PROGRAM);
	*prog = pos->second.clProgram;
	if (NULL == *prog)
		return clfftReturnLocked(sLock, CLFFT_INVALID_PROGRAM);

	cl_context progContext;
	clGetProgramInfo(*prog, CL_PROGRAM_CONTEXT, sizeof(cl_context), &progContext, NULL);
	if (planContext != progContext)
		return clfftReturnLocked(sLock, CLFFT_INVALID_PROGRAM);

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoSetclKernel(FFTRepo *repo, cl_program prog, clfftDirection dir, cl_kernel kernel)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	fftKernels *Kernels = mapKernelGet(&repo->mapKernels, prog);

	cl_kernel *pk;
	lockRAII **kernelLock;

	switch (dir)
	{
		case CLFFT_FORWARD:
			pk = &Kernels->kernel_fwd;
			kernelLock = &Kernels->kernel_fwd_lock;
			break;
		case CLFFT_BACKWARD:
			pk = &Kernels->kernel_back;
			kernelLock = &Kernels->kernel_back_lock;
			break;
		default: assert(false); return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);
	}

	assert(NULL == *pk);
	if (NULL != *pk)
		clReleaseKernel(*pk);

	*pk = kernel;

	if (NULL != *kernelLock)
		lockRAIIDestroy(*kernelLock);

	*kernelLock = lockRAIICreate();

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoGetclKernel(FFTRepo *repo, cl_program prog, clfftDirection dir, cl_kernel *kernel, lockRAII **kernelLock)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	KernelEntryPtr pos = mapKernelFind(&repo->mapKernels, prog);
	if (pos == ARRAY_MAP_END(repo->mapKernels))
		return clfftReturnLocked(sLock, CLFFT_INVALID_KERNEL);

	switch (dir)
	{
		case CLFFT_FORWARD:
			*kernel = pos->second.kernel_fwd;
			*kernelLock = pos->second.kernel_fwd_lock;
			break;
		case CLFFT_BACKWARD:
			*kernel = pos->second.kernel_back;
			*kernelLock = pos->second.kernel_back_lock;
			break;
		default: assert(false); return clfftReturnLocked(sLock, CLFFT_INVALID_ARG_VALUE);
	}

	if (kernel == NULL || NULL == *kernel)
		return clfftReturnLocked(sLock, CLFFT_INVALID_KERNEL);

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoCreatePlan(FFTRepo *repo, clfftPlanHandle *plHandle, FFTPlan **fftPlan)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	//	We keep track of this memory in our own collection struct, to make sure
	// it's freed in releaseResources 	The lifetime of a plan is tracked by the
	// client and is freed when the client calls clfftDestroyPlan()
	// Store the created plan through the explicit output pointer.
	*fftPlan = FFTPlanCreate();

	//	We allocate a new lock here, and expect it to be freed in
	// clfftDestroyPlan(); 	The lifetime of the lock is the same as the lifetime
	// of the plan
	lockRAII *lockPlan = lockRAIICreate();

	//	Add and remember the fftPlan in our map
	// Store the plan and its lock with explicit aggregate initialization.
	repoPlansValue repoValue = { *fftPlan, lockPlan };
	*repoPlansGet(&repo->repoPlans, FFTRepoPlanCount) = repoValue;

	//	Assign the user handle the plan count (unique identifier), and bump the
	// count for the next plan
	*plHandle = FFTRepoPlanCount++;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoGetPlan(FFTRepo *repo, clfftPlanHandle plHandle, FFTPlan **fftPlan, lockRAII **planLock)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	//	First, check if we have already created a plan with this exact same
	// FFTPlan
	repoPlansEntryPtr iter = repoPlansFind(&repo->repoPlans, plHandle);
	if (iter == ARRAY_MAP_END(repo->repoPlans))
		return clfftReturnLocked(sLock, CLFFT_INVALID_PLAN);

	//	If plan is valid, fill out the output pointers
	*fftPlan = iter->second.first;
	*planLock = iter->second.second;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTRepoDeletePlan(FFTRepo *repo, clfftPlanHandle *plHandle)
{
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	//	First, check if we have already created a plan with this exact same
	// FFTPlan
	repoPlansEntryPtr iter = repoPlansFind(&repo->repoPlans, *plHandle);
	if (iter == ARRAY_MAP_END(repo->repoPlans))
		return clfftReturnLocked(sLock, CLFFT_INVALID_PLAN);

	//	We lock the plan object while we are in the process of deleting it
	{
		// Enter the plan lock while destroying the plan object.
		lockRAIIEnter(iter->second.second);
		clReleaseContext(iter->second.first->context);

		//	Release and delete the FFTPlan
		FFTPlanDestroy(iter->second.first);
		lockRAIILeave(iter->second.second);
	}

	//	Delete the lockRAII
	lockRAIIDestroy(iter->second.second);

	//	Remove entry from our map object
	array_erase(&repo->repoPlans.entries, iter);

	//	Clear the client's handle to signify that the plan is gone
	*plHandle = 0;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}
/* End copied source: src\library\repo.cpp */

/* Begin copied source: src\library\generator.stockham.cpp */

static FFTGeneratedStockhamAction *FFTGeneratedStockhamActionCreate(clfftPlanHandle plHandle, FFTPlan *plan, cl_command_queue queue, clfftStatus *err)
{
	// Allocate action storage and clear it before explicit initialization.
	FFTGeneratedStockhamAction *action = (FFTGeneratedStockhamAction *) clfft_checked_malloc(sizeof(FFTGeneratedStockhamAction));
	memset((void *) action, 0, sizeof(*action));

	// Initialize the common action base and signature payload.
	FFTActionInit(&action->base, plan, Stockham, action, err);
	FFTKernelSignatureStockhamInit(&action->signature);
	if (*err != CLFFT_SUCCESS)
	{
		// Report base initialization failure and leave cleanup to the caller.
		fprintf(stderr, "FFTGeneratedStockhamActionCreate: FFTActionInit failed!\n");
		return action;
	}

	// Initialize the FFTAction kernel-generation parameter member.
	*err = FFTGeneratedStockhamActionInitParams(action);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedStockhamActionInitParams failed!\n");
		return action;
	}

	// Generate the kernel source through the shared repository.
	FFTRepo *fftRepo = FFTRepoGetInstance();
	*err = FFTActionGenerateKernel(&action->base, fftRepo, queue);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedStockhamActionCreate: "
			"FFTActionGenerateKernel failed\n");
		return action;
	}

	// Compile generated kernels for the requested queue and plan.
	*err = FFTActionCompileKernels(&action->base, queue, plHandle, plan);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedStockhamActionCreate: "
			"FFTActionCompileKernels failed\n");
		return action;
	}

	// Mark action creation as successful.
	*err = CLFFT_SUCCESS;
	return action;
}

static bool FFTGeneratedStockhamActionBuildForwardKernel(FFTGeneratedStockhamAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || r2c_transform;
}

static bool FFTGeneratedStockhamActionBuildBackwardKernel(FFTGeneratedStockhamAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || c2r_transform;
}

// FFT Stockham Autosort Method
//
//   Each pass does one digit reverse in essence. Hence by the time all passes
//   are done, complete digit reversal is done and output FFT is in correct
//   order. Intermediate FFTs are stored in natural order, which is not the case
//   with basic Cooley-Tukey algorithm. Natural order in intermediate data makes
//   it convenient for stitching together passes with different radices.
//
//  Basic FFT algorithm:
//
//        Pass loop
//        {
//            Outer loop
//            {
//                Inner loop
//                {
//                }
//            }
//        }
//
//  The sweeps of the outer and inner loop resemble matrix indexing, this matrix
//  changes shape with every pass as noted below
//
//   FFT pass diagram (radix 2)
//
//                k            k+R                                    k
//            * * * * * * * * * * * * * * * *                     * * * * * * *
//            *
//            *   |             |           *                     *   | *
//            *   |             |           *                     *   | *
//            *   |             |           * LS        -->       *   | *
//            *   |             |           *                     *   | *
//            *   |             |           *                     *   | *
//            * * * * * * * * * * * * * * * *                     *   | *
//                         RS                                     *   | * L
//                                                                *   | *
//                                                                *   | *
//                                                                *   | *
//                                                                *   | *
//                                                                *   | *
//                                                                *   | *
//                                                                *   | *
//                                                                * * * * * * *
//                                                                *
//                                                                       R
//
//
//    With every pass, the matrix doubles in height and halves in length
//
//
//  N = 2^T = Length of FFT
//  q = pass loop index
//  k = outer loop index = (0 ... R-1)
//  j = inner loop index = (0 ... LS-1)
//
//  Tables shows how values change as we go through the passes
//
//    q | LS   |  R   |  L  | RS
//   ___|______|______|_____|___
//    0 |  1   | N/2  |  2  | N
//    1 |  2   | N/4  |  4  | N/2
//    2 |  4   | N/8  |  8  | N/4
//    . |  .   | .    |  .  | .
//  T-1 |  N/2 | 1    |  N  | 2
//
//
//   Data Read Order
//     Radix 2: k*LS + j, (k+R)*LS + j
//     Radix 3: k*LS + j, (k+R)*LS + j, (k+2R)*LS + j
//     Radix 4: k*LS + j, (k+R)*LS + j, (k+2R)*LS + j, (k+3R)*LS + j
//     Radix 5: k*LS + j, (k+R)*LS + j, (k+2R)*LS + j, (k+3R)*LS + j, (k+4R)*LS
//     + j
//
//   Data Write Order
//       Radix 2: k*L + j, k*L + j + LS
//       Radix 3: k*L + j, k*L + j + LS, k*L + j + 2*LS
//       Radix 4: k*L + j, k*L + j + LS, k*L + j + 2*LS, k*L + j + 3*LS
//       Radix 5: k*L + j, k*L + j + LS, k*L + j + 2*LS, k*L + j + 3*LS, k*L + j
//       + 4*LS
//

// Experimnetal Start =========================================
// Kernel Generator Parameterization ==========================

// Uncomment this directive to activate parameter reads from file

// Parameters to read

#define RADIX_TABLE_COMMON \
	{ 2048, 256, 1, 4, 8, 8, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, { 512, 64, 1, 3, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 256, 64, 1, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, \
		{ 64, 64, 4, 3, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 32, 64, 16, 2, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, \
		{ 16, 64, 16, 2, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 4, 64, 32, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 2, 64, 64, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

typedef struct KernelCoreSpecRecord
{
	size_t length;
	size_t workGroupSize;
	size_t numTransforms;
	size_t numPasses;
	size_t radices[12];
} KernelCoreSpecRecord;

static const KernelCoreSpecRecord kernelCoreSpecSingle[] = {
	RADIX_TABLE_COMMON

	//  Length, WorkGroupSize, NumTransforms, NumPasses,  Radices
	{ 4096, 256, 1, 4, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1024, 128, 1, 4, 8, 8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 128, 64, 4, 3, 8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 64, 32, 2, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

static const KernelCoreSpecRecord kernelCoreSpecDouble[] = {
	RADIX_TABLE_COMMON

	//  Length, WorkGroupSize, NumTransforms, NumPasses,  Radices
	{ 1024, 128, 1, 4, 8, 8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 128, 64, 4, 3, 8, 8, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 8, 64, 16, 3, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

static const KernelCoreSpecRecord *KernelCoreSpecsFind(Precision pr, size_t length)
{
	// Select the static specification table for the requested precision.
	const KernelCoreSpecRecord *table = NULL;
	size_t tableLength = 0;
	switch (pr)
	{
		case P_SINGLE:
			table = kernelCoreSpecSingle;
			tableLength = sizeof(kernelCoreSpecSingle) / sizeof(kernelCoreSpecSingle[0]);
			break;
		case P_DOUBLE:
			table = kernelCoreSpecDouble;
			tableLength = sizeof(kernelCoreSpecDouble) / sizeof(kernelCoreSpecDouble[0]);
			break;
		default: assert(false); return NULL;
	}

	// Search the table linearly; it is tiny and static.
	for (size_t i = 0; i < tableLength; ++i)
		if (table[i].length == length)
			return table + i;
	return NULL;
}

static void KernelCoreSpecsGetRadices(Precision pr, size_t length, size_t *numPasses, const size_t **pRadices)
{
	// Clear output values before looking up optional table data.
	*pRadices = NULL;
	*numPasses = 0;

	// Copy radix metadata from a matching static record.
	const KernelCoreSpecRecord *spec = KernelCoreSpecsFind(pr, length);
	if (spec != NULL)
	{
		*pRadices = spec->radices;
		*numPasses = spec->numPasses;
	}
}

static void KernelCoreSpecsGetWGSAndNT(Precision pr, size_t length, size_t *workGroupSize, size_t *numTransforms)
{
	// Clear output values before looking up optional table data.
	*workGroupSize = 0;
	*numTransforms = 0;

	// Copy work-size metadata from a matching static record.
	const KernelCoreSpecRecord *spec = KernelCoreSpecsFind(pr, length);
	if (spec != NULL)
	{
		*workGroupSize = spec->workGroupSize;
		*numTransforms = spec->numTransforms;
	}
}

// Given the length of 1d fft, this function determines the appropriate work
// group size and the number of transforms per work group
// TODO for optimizations - experiment with different possibilities for work
// group sizes and num transforms for improving performance
static void DetermineSizes(size_t MAX_WGS, size_t length, size_t *workGroupSize, size_t *numTrans, const Precision *pr)
{
	assert(MAX_WGS >= 64);
	assert(workGroupSize != NULL);
	assert(numTrans != NULL);
	assert(pr != NULL);

	if (length == 1) // special case
	{
		*workGroupSize = 64;
		*numTrans = 64;
		return;
	}

	size_t baseRadix[] = { 13, 11, 7, 5, 3, 2 }; // list only supported primes
	size_t baseRadixSize = sizeof(baseRadix) / sizeof(baseRadix[0]);

	size_t l = length;
	// Store expanded prime factors by radix using direct C-style indexing.
	size_t primeFactorsExpanded[14] = { 0 };
	for (size_t r = 0; r < baseRadixSize; r++)
	{
		size_t rad = baseRadix[r];
		size_t e = 1;
		while (!(l % rad))
		{
			l /= rad;
			e *= rad;
		}

		primeFactorsExpanded[rad] = e;
	}

	assert(l == 1); // Makes sure the number is composed of only supported primes

	if (primeFactorsExpanded[2] == length) // Length is pure power of 2
	{
		if (length >= 1024)
		{
			*workGroupSize = (MAX_WGS >= 256) ? 256 : MAX_WGS;
			*numTrans = 1;
		}
		else if (length == 512)
		{
			*workGroupSize = 64;
			*numTrans = 1;
		}
		else if (length >= 16)
		{
			*workGroupSize = 64;
			*numTrans = 256 / length;
		}
		else
		{
			*workGroupSize = 64;
			*numTrans = 128 / length;
		}
	}
	else if (primeFactorsExpanded[3] == length) // Length is pure power of 3
	{
		*workGroupSize = (MAX_WGS >= 256) ? 243 : 27;
		*numTrans = length >= 3 * (*workGroupSize) ? 1 : (3 * (*workGroupSize)) / length;
	}
	else if (primeFactorsExpanded[5] == length) // Length is pure power of 5
	{
		*workGroupSize = (MAX_WGS >= 128) ? 125 : 25;
		*numTrans = length >= 5 * (*workGroupSize) ? 1 : (5 * (*workGroupSize)) / length;
	}
	else if (primeFactorsExpanded[7] == length) // Length is pure power of 7
	{
		*workGroupSize = 49;
		*numTrans = length >= 7 * (*workGroupSize) ? 1 : (7 * (*workGroupSize)) / length;
	}
	else if (primeFactorsExpanded[11] == length) // Length is pure power of 11
	{
		*workGroupSize = 121;
		*numTrans = length >= 11 * (*workGroupSize) ? 1 : (11 * (*workGroupSize)) / length;
	}
	else if (primeFactorsExpanded[13] == length) // Length is pure power of 13
	{
		*workGroupSize = 169;
		*numTrans = length >= 13 * (*workGroupSize) ? 1 : (13 * (*workGroupSize)) / length;
	}
	else
	{
		size_t leastNumPerWI = 1;	   // least number of elements in one work item
		size_t maxWorkGroupSize = MAX_WGS; // maximum work group size desired

		if (primeFactorsExpanded[2] * primeFactorsExpanded[3] == length)
		{
			if (length % 12 == 0)
			{
				leastNumPerWI = 12;
				maxWorkGroupSize = 128;
			}
			else
			{
				leastNumPerWI = 6;
				maxWorkGroupSize = 256;
			}
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[5] == length)
		{
			if (length % 20 == 0)
			{
				leastNumPerWI = 20;
				maxWorkGroupSize = 64;
			}
			else
			{
				leastNumPerWI = 10;
				maxWorkGroupSize = 128;
			}
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[7] == length)
		{
			leastNumPerWI = 14;
			maxWorkGroupSize = 64;
		}
		else if (primeFactorsExpanded[3] * primeFactorsExpanded[5] == length)
		{
			leastNumPerWI = 15;
			maxWorkGroupSize = 128;
		}
		else if (primeFactorsExpanded[3] * primeFactorsExpanded[7] == length)
		{
			leastNumPerWI = 21;
			maxWorkGroupSize = 128;
		}
		else if (primeFactorsExpanded[5] * primeFactorsExpanded[7] == length)
		{
			leastNumPerWI = 35;
			maxWorkGroupSize = 64;
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[3] * primeFactorsExpanded[5] == length)
		{
			leastNumPerWI = 30;
			maxWorkGroupSize = 64;
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[3] * primeFactorsExpanded[7] == length)
		{
			leastNumPerWI = 42;
			maxWorkGroupSize = 60;
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[5] * primeFactorsExpanded[7] == length)
		{
			leastNumPerWI = 70;
			maxWorkGroupSize = 36;
		}
		else if (primeFactorsExpanded[3] * primeFactorsExpanded[5] * primeFactorsExpanded[7] == length)
		{
			leastNumPerWI = 105;
			maxWorkGroupSize = 24;
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[11] == length)
		{
			leastNumPerWI = 22;
			maxWorkGroupSize = 128;
		}
		else if (primeFactorsExpanded[2] * primeFactorsExpanded[13] == length)
		{
			leastNumPerWI = 26;
			maxWorkGroupSize = 128;
		}
		else
		{
			leastNumPerWI = 210;
			maxWorkGroupSize = 12;
		}
		if (*pr == P_DOUBLE)
		{
			// leastNumPerWI /= 2;
			maxWorkGroupSize /= 2;
		}

		if (maxWorkGroupSize > MAX_WGS)
			maxWorkGroupSize = MAX_WGS;
		assert(leastNumPerWI > 0 && length % leastNumPerWI == 0);

		for (size_t lnpi = leastNumPerWI; lnpi <= length; lnpi += leastNumPerWI)
		{
			if (length % lnpi != 0)
				continue;

			if (length / lnpi <= MAX_WGS)
			{
				leastNumPerWI = lnpi;
				break;
			}
		}

		*numTrans = maxWorkGroupSize / (length / leastNumPerWI);
		*numTrans = *numTrans < 1 ? 1 : *numTrans;
		*workGroupSize = *numTrans * (length / leastNumPerWI);
	}

	assert(*workGroupSize <= MAX_WGS);
}

// Twiddle factors table
typedef struct TwiddleTable
{
	size_t N;	 // length
	double *wc, *ws; // cosine, sine arrays

} TwiddleTable;

static void TwiddleTableGenerateTwiddleTable(TwiddleTable *table, Precision pr, const array_size_t radices, buffer_t *twStr)
{
	// Generate twiddle table source from explicit table state.
	const double TWO_PI = -6.283185307179586476925286766559;

	// Make sure the radices vector sums up to table->N
	size_t sz = 1;
	const size_t *radices_end = radices.buf ? radices.buf + radices.len : NULL;
	for (const size_t *i = radices.buf; i != radices_end; i++)
		sz *= (*i);
	assert(sz == table->N);

	// Generate the table
	size_t L = 1;
	size_t nt = 0;
	for (const size_t *i = radices.buf; i != radices_end; i++)
	{
		size_t radix = *i;

		L *= radix;

		// Twiddle factors
		for (size_t k = 0; k < (L / radix); k++)
		{
			double theta = TWO_PI * ((double) k) / ((double) L);

			for (size_t j = 1; j < radix; j++)
			{
				double c = cos(((double) j) * theta);
				double s = sin(((double) j) * theta);

				// if (fabs(c) < 1.0E-12)	c = 0.0;
				// if (fabs(s) < 1.0E-12)	s = 0.0;

				table->wc[nt] = c;
				table->ws[nt++] = s;
			}
		}
	}

	buffer_t sfx = FloatSuffix(pr);

	// Cache generated names so formatting uses explicit buffer pointers.
	buffer_t regBase2 = RegBaseType(pr, 2);

	// Stringize the table
	buffer_stream_t ss;
	// Initialize stream formatting state explicitly.
	bufstream_init(&ss);
	ss.precision_value = 34;
	bufstream_scientific(&ss);
	for (size_t i = 0; i < (table->N - 1); i++)
	{
		bufprintf(&ss.text, "(%s)(%.*e%s, %.*e%s),\n", bufcstr(&regBase2), ss.precision_value, table->wc[i], bufcstr(&sfx), ss.precision_value, table->ws[i],
			bufcstr(&sfx));
	}
	bufcatbuf(twStr, &ss.text);
}

static inline void TwiddleTableInit(TwiddleTable *table, size_t length)
{
	// Initialize twiddle table dimensions and allocate storage.
	table->N = length;
	table->wc = (double *) clfft_checked_malloc(table->N * sizeof(double));
	table->ws = (double *) clfft_checked_malloc(table->N * sizeof(double));
}

static inline void TwiddleTableFree(TwiddleTable *table)
{
	// Release twiddle table storage.
	if (!table)
		return;
	free(table->wc);
	free(table->ws);
	table->wc = NULL;
	table->ws = NULL;
}

// A pass inside an FFT kernel
typedef struct Pass
{
	Precision pr;
	size_t position; // Position in the kernel

	size_t algL;  // 'L' value from fft algorithm
	size_t algLS; // 'LS' value
	size_t algR;  // 'R' value

	size_t length;	// Length of FFT
	size_t radix;	// Base radix
	size_t cnPerWI; // Complex numbers per work-item

	size_t workGroupSize;	    // size of the workgroup = (length / cnPerWI)
	size_t numButterfly;	    // Number of basic FFT butterflies = (cnPerWI / radix)
	size_t numB1, numB2, numB4; // number of different types of butterflies

	bool r2c; // real to complex transform
	bool c2r; // complex to real transform
	bool rcFull;
	bool rcSimple;
	bool realSpecial;

	bool enableGrouping;
	bool linearRegs; // scalar registers (non-vectorized registers) to be used
	bool halfLds;	 // only half the LDS of a complex length need to be used
	struct Pass *nextPass;

	bool fft_doPreCallback;
	clfftCallbackParam fft_preCallback;

	bool fft_doPostCallback;
	clfftCallbackParam fft_postCallback;
} Pass;

typedef enum PassSweepFlag
{
	SR_READ,
	SR_TWMUL,
	SR_TWMUL_3STEP,
	SR_WRITE,
} PassSweepFlag;

typedef enum PassSweepComponent
{
	SR_COMP_REAL,
	SR_COMP_IMAG,
	SR_COMP_BOTH,
} PassSweepComponent;

static inline void PassRegBase(const Pass *pass, size_t regC, buffer_t *str)
{
	// Append the base register name for a pass register group.
	bufcatcstr(str, "B");
	BUFCAT_BUFFER_VALUE(str, SztToStr(regC));
}

static inline void PassRegBaseAndCount(const Pass *pass, size_t num, buffer_t *str)
{
	// Append the register group count suffix.
	bufcatcstr(str, "C");
	BUFCAT_BUFFER_VALUE(str, SztToStr(num));
}

static inline void PassRegBaseAndCountAndPos(const Pass *pass, const char *RealImag, size_t radPos, buffer_t *str)
{
	// Append the component and radix-position suffix.
	bufcatcstr(str, RealImag);
	BUFCAT_BUFFER_VALUE(str, SztToStr(radPos));
}

static void PassDeclareRegs(const Pass *pass, const buffer_t regType, size_t regC, size_t numB, buffer_t *passStr)
{
	// Declare all registers used by one pass register group.
	buffer_t regBase = buffer_empty();
	PassRegBase(pass, regC, &regBase);

	if (pass->linearRegs)
	{
		assert(regC == 1);
		assert(numB == pass->numButterfly);
	}

	for (size_t i = 0; i < numB; i++)
	{
		bufcatcstr(passStr, "\n\t");
		bufcatbuf(passStr, &regType);
		bufcatcstr(passStr, " ");

		buffer_t regBaseCount = buffer_copy(&regBase);
		PassRegBaseAndCount(pass, i, &regBaseCount);

		for (size_t r = 0;; r++)
		{
			if (pass->linearRegs)
			{
				buffer_t regIndex = buffer_from_cstr("R");
				PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);

				bufcatbuf(passStr, &regIndex);
			}
			else
			{
				buffer_t regRealIndex = buffer_copy(&regBaseCount);
				buffer_t regImagIndex = buffer_copy(&regBaseCount);

				PassRegBaseAndCountAndPos(pass, "R", r, &regRealIndex); // real
				PassRegBaseAndCountAndPos(pass, "I", r,
					&regImagIndex); // imaginary

				bufcatbuf(passStr, &regRealIndex);
				bufcatcstr(passStr, ", ");
				bufcatbuf(passStr, &regImagIndex);
			}

			if (r == pass->radix - 1)
			{
				bufcatcstr(passStr, ";");
				break;
			}
			else
			{
				bufcatcstr(passStr, ", ");
			}
		}
	}
}

static inline buffer_t PassIterRegArgs(const Pass *pass)
{
	// Build the linear register argument list for a pass.
	buffer_t str = buffer_from_cstr("");

	if (pass->linearRegs)
	{
		buffer_t regType = RegBaseType(pass->pr, 2);

		for (size_t i = 0; i < pass->cnPerWI; i++)
		{
			if (i != 0)
				bufcatcstr(&str, ", ");
			bufcatbuf(&str, &regType);
			bufcatcstr(&str, " *R");
			BUFCAT_BUFFER_VALUE(&str, SztToStr(i));
		}
	}

	return str;
}

static void PassSweepRegs(const Pass *pass, size_t flag, bool fwd, bool interleaved, size_t stride, size_t component, double scale, bool frontTwiddle, const buffer_t bufferRe,
	const buffer_t bufferIm, const char *offset, size_t regC, size_t numB, size_t numPrev, buffer_t *passStr, bool initZero, bool isPrecallVector, bool oddt)
{
	// Emit register sweep source for one pass operation.
	assert((flag == SR_READ) || (flag == SR_TWMUL) || (flag == SR_TWMUL_3STEP) || (flag == SR_WRITE));

	const buffer_t twTable = TwTableName();
	const buffer_t tw3StepFunc = TwTableLargeFunc();

	// component: 0 - real, 1 - imaginary, 2 - both
	size_t cStart, cEnd;
	switch (component)
	{
		case SR_COMP_REAL:
			cStart = 0;
			cEnd = 1;
			break;
		case SR_COMP_IMAG:
			cStart = 1;
			cEnd = 2;
			break;
		case SR_COMP_BOTH:
			cStart = 0;
			cEnd = 2;
			break;
		default: assert(false);
	}

	// Read/Write logic:
	// The double loop inside pass loop of FFT algorithm is mapped into the
	// workGroupSize work items with each work item handling cnPerWI numbers

	// Read logic:
	// Reads for any pass appear the same with the stockham algorithm when
	// mapped to the work items. The buffer is divided into (L/radix) sized
	// blocks and the values are read in linear order inside each block.

	// Vector reads are possible if we have unit strides
	// since read pattern remains the same for all passes and they are
	// contiguous Writes are not contiguous

	// TODO : twiddle multiplies can be combined with read
	// TODO : twiddle factors can be reordered in the table to do vector reads
	// of them

	// Write logic:
	// outer loop index k and the inner loop index j map to 'me' as follows:
	// In one work-item (1 'me'), there are 'numButterfly' fft butterflies. They
	// are indexed as numButterfly*me + butterflyIndex, where butterflyIndex's
	// range is 0 ... numButterfly-1. The total number of butterflies needed is
	// covered over all the work-items. So essentially the double loop k,j is
	// flattened to fit this linearly increasing 'me'. j = (numButterfly*me +
	// butterflyIndex)%LS k = (numButterfly*me + butterflyIndex)/LS

	buffer_t twType = RegBaseType(pass->pr, 2);
	buffer_t rType = RegBaseType(pass->pr, 1);

	size_t butterflyIndex = numPrev;
	buffer_t bufOffset = buffer_empty();

	buffer_t regBase = buffer_empty();
	PassRegBase(pass, regC, &regBase);

	// special write back to global memory with float4 grouping, writing 2
	// complex numbers at once
	if (numB && (numB % 2 == 0) && (regC == 1) && (stride == 1) && (pass->numButterfly % 2 == 0) && (pass->algLS % 2 == 0) && (flag == SR_WRITE) && (pass->nextPass == NULL) &&
		interleaved && (component == SR_COMP_BOTH) && pass->linearRegs && pass->enableGrouping && !pass->fft_doPostCallback)
	{
		assert((pass->numButterfly * pass->workGroupSize) == pass->algLS);
		assert(bufcmp(&bufferRe, &bufferIm) == 0); // Make sure Real & Imag buffer strings are same for
		// interleaved data

		bufcatcstr(passStr, "\n\t");
		bufcatcstr(passStr, "__global ");
		BUFCAT_BUFFER_VALUE(passStr, RegBaseType(pass->pr, 4));
		bufcatcstr(passStr, " *buff4g = (__global ");
		BUFCAT_BUFFER_VALUE(passStr, RegBaseType(pass->pr, 4));
		bufcatcstr(passStr, " *)");
		bufcatbuf(passStr, &bufferRe);
		bufcatcstr(passStr,
			";\n\t"); // Assuming 'outOffset' is 0, so not adding it here

		for (size_t r = 0; r < pass->radix; r++) // setting the radix loop outside to facilitate grouped writing
		{
			butterflyIndex = numPrev;

			for (size_t i = 0; i < (numB / 2); i++)
			{
				buffer_t regIndexA = buffer_from_cstr("(*R");
				buffer_t regIndexB = buffer_from_cstr("(*R");

				PassRegBaseAndCountAndPos(pass, "", (2 * i + 0) * pass->radix + r, &regIndexA);
				bufcatcstr(&regIndexA, ")");
				PassRegBaseAndCountAndPos(pass, "", (2 * i + 1) * pass->radix + r, &regIndexB);
				bufcatcstr(&regIndexB, ")");

				bufcatcstr(passStr, "\n\t");
				bufcatcstr(passStr, "buff4g");
				bufcatcstr(passStr, "[ ");
				BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->numButterfly / 2));
				bufcatcstr(passStr, "*me + ");
				BUFCAT_BUFFER_VALUE(passStr, SztToStr(butterflyIndex));
				bufcatcstr(passStr, " + ");
				BUFCAT_BUFFER_VALUE(passStr, SztToStr(r * (pass->algLS / 2)));
				bufcatcstr(passStr, " ]");
				bufcatcstr(passStr, " = ");
				bufcatcstr(passStr, "(");
				BUFCAT_BUFFER_VALUE(passStr, RegBaseType(pass->pr, 4));
				bufcatcstr(passStr, ")(");
				bufcatbuf(passStr, &regIndexA);
				bufcatcstr(passStr, ".x, ");
				bufcatbuf(passStr, &regIndexA);
				bufcatcstr(passStr, ".y, ");
				bufcatbuf(passStr, &regIndexB);
				bufcatcstr(passStr, ".x, ");
				bufcatbuf(passStr, &regIndexB);
				bufcatcstr(passStr, ".y) ");
				if (scale != 1.0f)
				{
					bufcatcstr(passStr, " * ");
					BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
					BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
				}
				bufcatcstr(passStr, ";");

				butterflyIndex++;
			}
		}

		return;
	}

	size_t hid = 0;
	bool swapElement = false;
	size_t tIter = numB * pass->radix;

	// block to rearrange reads of adjacent memory locations together
	if (pass->linearRegs && (flag == SR_READ))
	{
		for (size_t r = 0; r < pass->radix; r++)
		{
			for (size_t i = 0; i < numB; i++)
			{
				for (size_t c = cStart; c < cEnd; c++) // component loop: 0 - real, 1 - imaginary
				{
					swapElement = (pass->fft_doPreCallback && pass->c2r && component == SR_COMP_REAL); // reset at start of loop

					buffer_t tail = buffer_empty();
					buffer_t regIndex = buffer_empty();
					buffer_t regIndexC = buffer_empty();
					bufsetcstr(&regIndex, "(*R");
					buffer_t buffer = buffer_empty();

					// Read real & imag at once
					if (interleaved && (component == SR_COMP_BOTH))
					{
						assert(bufcmp(&bufferRe, &bufferIm) == 0); // Make sure Real & Imag buffer strings are
						// same for interleaved data
						bufsetbuf(&buffer, &bufferRe);
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
						bufcatcstr(&regIndex, ")");
						bufsetcstr(&tail, ";");
					}
					else if (c == 0)
					{
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);

						hid = (i * pass->radix + r) / (tIter > 1 ? (tIter / 2) : 1);
						swapElement = swapElement && hid != 0;
						swapElement = (oddt && ((i * pass->radix + r) >= (tIter - 1))) ? false : swapElement; // for c2r odd size don't swap
						// for last register
						if (swapElement)
						{
							bufsetbuf(&regIndexC, &regIndex);
							bufcatcstr(&regIndexC, ").y");
						}

						bufcatcstr(&regIndex, ").x");
						bufsetbuf(&buffer, &bufferRe);
						bufsetcstr(&tail, interleaved ? ".x;" : ";");
					}
					else
					{
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
						bufcatcstr(&regIndex, ").y");
						bufsetbuf(&buffer, &bufferIm);
						bufsetcstr(&tail, interleaved ? ".y;" : ";");
					}

					// get offset
					bufclear(&bufOffset);
					bufcatcstr(&bufOffset, offset);
					bufcatcstr(&bufOffset, " + ( ");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(numPrev));
					bufcatcstr(&bufOffset, " + ");
					bufcatcstr(&bufOffset, "me*");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
					bufcatcstr(&bufOffset, " + ");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(i));
					bufcatcstr(&bufOffset, " + ");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(r * pass->length / pass->radix));
					bufcatcstr(&bufOffset, " )*");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(stride));

					// If precallback is set invoke callback function
					// Invoke callback only once in Planar data layout
					// (i.e.c==0)
					if (pass->fft_doPreCallback && c == 0 && component == SR_COMP_BOTH)
					{
						bufcatcstr(passStr, "\n\t");
						bufcatcstr(passStr, "retPrecallback = ");
						bufcatcstr(passStr, pass->fft_preCallback.funcname);
						bufcatcstr(passStr, "(");
						if (interleaved)
						{
							bufcatbuf(passStr, &buffer);
							bufcatcstr(passStr, ", ");
						}
						else
						{
							bufcatbuf(passStr, &bufferRe);
							bufcatcstr(passStr, ", ");
							bufcatbuf(passStr, &bufferIm);
							bufcatcstr(passStr, ", ");
						}
						bufcatbuf(passStr, &bufOffset);
						bufcatcstr(passStr, ", pre_userdata");
						if (pass->fft_preCallback.localMemSize > 0)
							bufcatcstr(passStr, ", localmem");
						bufcatcstr(passStr, ");");
					}

					if (swapElement)
					{
						bufcatcstr(passStr, "\n\t");
						bufcatbuf(passStr, &regIndexC);
						bufcatcstr(passStr, " = ");
						bufcatbuf(passStr, &regIndex);
						bufcatcstr(passStr, ";");
					}

					bufcatcstr(passStr, "\n\t");
					bufcatbuf(passStr, &regIndex);
					bufcatcstr(passStr, " = ");

					if (initZero)
					{
						if (interleaved && (component == SR_COMP_BOTH))
							bufcatcstr(passStr, "(fvect2)(0, 0);");
						else
							bufcatcstr(passStr, "0;");
					}
					else
					{
						// Use the return value from precallback if set
						if (pass->fft_doPreCallback && (component == SR_COMP_BOTH || pass->r2c))
						{
							if (component == SR_COMP_BOTH)
							{
								bufcatcstr(passStr, "retPrecallback");
								// Append the vector tail when callback data is
								// interleaved.
								if (interleaved)
									bufcatbuf(passStr, &tail);
								else
									bufcatcstr(passStr, (c == 0) ? ".x;" : ".y;");
							}
							else if (pass->r2c)
							{
								bufcatcstr(passStr, pass->fft_preCallback.funcname);
								bufcatcstr(passStr, "(");
								bufcatbuf(passStr, &buffer);
								bufcatcstr(passStr, ", ");
								bufcatbuf(passStr, &bufOffset);
								bufcatcstr(passStr, ", pre_userdata");

								if (pass->fft_preCallback.localMemSize > 0)
									bufcatcstr(passStr, ", localmem");
								bufcatcstr(passStr, ");");
							}
						}
						else
						{
							bufcatbuf(passStr, &buffer);
							bufcatcstr(passStr, "[");
							bufcatbuf(passStr, &bufOffset);
							bufcatcstr(passStr, "]");
							bufcatbuf(passStr, &tail);
						}
					}

					// Since we read real & imag at once, we break the loop
					if (interleaved && (component == SR_COMP_BOTH))
						break;
				}
			}
		}
		return;
	}

	// block to rearrange writes of adjacent memory locations together
	if (pass->linearRegs && (flag == SR_WRITE) && (pass->nextPass == NULL))
	{
		for (size_t r = 0; r < pass->radix; r++)
		{
			butterflyIndex = numPrev;

			for (size_t i = 0; i < numB; i++)
			{
				if (pass->realSpecial && (pass->nextPass == NULL) && (r > (pass->radix / 2)))
					break;

				if (pass->realSpecial && (pass->nextPass == NULL) && (r == pass->radix / 2) && (i != 0))
					break;

				if (pass->realSpecial && (pass->nextPass == NULL) && (r == pass->radix / 2) && (i == 0))
					bufcatcstr(passStr, "\n\t}\n\tif( rw && !me)\n\t{");

				buffer_t regIndexC0 = buffer_empty();
				for (size_t c = cStart; c < cEnd; c++) // component loop: 0 - real, 1 - imaginary
				{
					buffer_t tail = buffer_empty();
					buffer_t regIndex = buffer_empty();
					bufsetcstr(&regIndex, "(*R");
					buffer_t buffer = buffer_empty();

					// Write real & imag at once
					if (interleaved && (component == SR_COMP_BOTH))
					{
						assert(bufcmp(&bufferRe, &bufferIm) == 0); // Make sure Real & Imag buffer strings are
						// same for interleaved data
						bufsetbuf(&buffer, &bufferRe);
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
						bufcatcstr(&regIndex, ")");
						bufsetcstr(&tail, "");
					}
					else if (c == 0)
					{
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
						bufcatcstr(&regIndex, ").x");
						bufsetbuf(&buffer, &bufferRe);
						bufsetcstr(&tail, interleaved ? ".x" : "");
					}
					else
					{
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
						bufcatcstr(&regIndex, ").y");
						bufsetbuf(&buffer, &bufferIm);
						bufsetcstr(&tail, interleaved ? ".y" : "");
					}

					bufclear(&bufOffset);
					bufcatcstr(&bufOffset, offset);
					bufcatcstr(&bufOffset, " + ( ");
					if ((pass->numButterfly * pass->workGroupSize) > pass->algLS)
					{
						bufcatcstr(&bufOffset, "((");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
						bufcatcstr(&bufOffset, "*me + ");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(butterflyIndex));
						bufcatcstr(&bufOffset, ")/");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->algLS));
						bufcatcstr(&bufOffset, ")*");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->algL));
						bufcatcstr(&bufOffset, " + (");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
						bufcatcstr(&bufOffset, "*me + ");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(butterflyIndex));
						bufcatcstr(&bufOffset, ")%");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->algLS));
						bufcatcstr(&bufOffset, " + ");
					}
					else
					{
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
						bufcatcstr(&bufOffset, "*me + ");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(butterflyIndex));
						bufcatcstr(&bufOffset, " + ");
					}
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(r * pass->algLS));
					bufcatcstr(&bufOffset, " )*");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(stride));

					if (scale != 1.0f)
					{
						bufcatcstr(&regIndex, " * ");
						BUFCAT_BUFFER_VALUE(&regIndex, FloatToStr(scale));
						BUFCAT_BUFFER_VALUE(&regIndex, FloatSuffix(pass->pr));
					}
					if (c == cStart)
						bufsetbuf(&regIndexC0, &regIndex);

					if (pass->fft_doPostCallback && !pass->r2c)
					{
						if (interleaved || c == (cEnd - 1))
						{
							bufcatcstr(passStr, "\n\t");
							bufcatcstr(passStr, pass->fft_postCallback.funcname);
							bufcatcstr(passStr, "(");

							if (interleaved || (pass->c2r && bufcmp(&bufferRe, &bufferIm) == 0))
							{
								bufcatbuf(passStr, &buffer);
							}
							else
							{
								bufcatbuf(passStr, &bufferRe);
								bufcatcstr(passStr, ", ");
								bufcatbuf(passStr, &bufferIm);
							}
							bufcatcstr(passStr, ", ");
							bufcatbuf(passStr, &bufOffset);
							bufcatcstr(passStr, ", post_userdata, (");
							bufcatbuf(passStr, &regIndexC0);
							bufcatcstr(passStr, ")");
							if (!(interleaved || (pass->c2r && bufcmp(&bufferRe, &bufferIm) == 0)))
							{
								bufcatcstr(passStr, ", (");
								bufcatbuf(passStr, &regIndex);
								bufcatcstr(passStr, ")");
							}

							if (pass->fft_postCallback.localMemSize > 0)
								bufcatcstr(passStr, ", post_localmem");
							bufcatcstr(passStr, ");");
						}
					}
					else
					{
						bufcatcstr(passStr, "\n\t");
						bufcatbuf(passStr, &buffer);
						bufcatcstr(passStr, "[");
						bufcatbuf(passStr, &bufOffset);
						bufcatcstr(passStr, "]");
						bufcatbuf(passStr, &tail);
						bufcatcstr(passStr, " = ");
						bufcatbuf(passStr, &regIndex);
						bufcatcstr(passStr, ";");
					}

					// Since we write real & imag at once, we break the loop
					if (interleaved && (component == SR_COMP_BOTH))
						break;
				}

				if (pass->realSpecial && (pass->nextPass == NULL) && (r == pass->radix / 2) && (i == 0))
					bufcatcstr(passStr, "\n\t}\n\tif(rw)\n\t{");

				butterflyIndex++;
			}
		}

		return;
	}

	for (size_t i = 0; i < numB; i++)
	{
		buffer_t regBaseCount = buffer_copy(&regBase);
		PassRegBaseAndCount(pass, i, &regBaseCount);

		if (flag == SR_READ) // read operation
		{
			// the 'r' (radix index) loop is placed outer to the
			// 'v' (vector index) loop to make possible vectorized reads

			for (size_t r = 0; r < pass->radix; r++)
			{
				for (size_t c = cStart; c < cEnd; c++) // component loop: 0 - real, 1 - imaginary
				{
					buffer_t tail = buffer_empty();
					buffer_t regIndex = buffer_empty();
					buffer_t regIndexC = buffer_empty();
					bufset_linear_reg(&regIndex, pass->linearRegs, &regBaseCount);
					buffer_t buffer = buffer_empty();

					// Read real & imag at once
					if (interleaved && (component == SR_COMP_BOTH) && pass->linearRegs)
					{
						assert(bufcmp(&bufferRe, &bufferIm) == 0); // Make sure Real & Imag buffer strings are
						// same for interleaved data
						bufsetbuf(&buffer, &bufferRe);
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
						bufcatcstr(&regIndex, ")");
						bufsetcstr(&tail, ";");
					}
					else if (c == 0)
					{
						if (pass->linearRegs)
						{
							PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);

							hid = (i * pass->radix + r) / (numB * pass->radix / 2);
							if (pass->fft_doPreCallback && pass->c2r && component == SR_COMP_REAL && hid != 0)
							{
								bufsetbuf(&regIndexC, &regIndex);
								bufcatcstr(&regIndexC, ").y");
							}
							bufcatcstr(&regIndex, ").x");
						}
						else
						{
							PassRegBaseAndCountAndPos(pass, "R", r, &regIndex);
						}
						bufsetbuf(&buffer, &bufferRe);
						bufsetcstr(&tail, interleaved ? ".x;" : ";");
					}
					else
					{
						if (pass->linearRegs)
						{
							PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
							bufcatcstr(&regIndex, ").y");
						}
						else
						{
							PassRegBaseAndCountAndPos(pass, "I", r, &regIndex);
						}
						bufsetbuf(&buffer, &bufferIm);
						bufsetcstr(&tail, interleaved ? ".y;" : ";");
					}

					for (size_t v = 0; v < regC; v++) // TODO: vectorize the reads; instead of reading
									  // individually for consecutive reads of vector
									  // elements
					{
						buffer_t regIndexSub = buffer_copy(&regIndex);
						if (regC != 1)
						{
							bufcatcstr(&regIndexSub, ".s");
							BUFCAT_BUFFER_VALUE(&regIndexSub, SztToStr(v));
						}

						// get offset
						bufclear(&bufOffset);
						bufcatcstr(&bufOffset, offset);
						bufcatcstr(&bufOffset, " + ( ");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(numPrev));
						bufcatcstr(&bufOffset, " + ");
						bufcatcstr(&bufOffset, "me*");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
						bufcatcstr(&bufOffset, " + ");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(i * regC + v));
						bufcatcstr(&bufOffset, " + ");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(r * pass->length / pass->radix));
						bufcatcstr(&bufOffset, " )*");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(stride));

						// If precallback is set invoke callback function
						// Invoke callback only once in Planar data layout
						// (i.e.c==0)
						if (pass->fft_doPreCallback && c == 0 && component == SR_COMP_BOTH)
						{
							bufcatcstr(passStr, "\n\t");
							bufcatcstr(passStr, "retPrecallback");

							if (isPrecallVector)
							{
								bufcatcstr(passStr, "[");
								BUFCAT_BUFFER_VALUE(passStr, SztToStr(v));
								bufcatcstr(passStr, "]");
							}

							bufcatcstr(passStr, " = ");
							bufcatcstr(passStr, pass->fft_preCallback.funcname);
							bufcatcstr(passStr, "(");
							if (interleaved)
							{
								bufcatbuf(passStr, &buffer);
								bufcatcstr(passStr, ", ");
							}
							else
							{
								bufcatbuf(passStr, &bufferRe);
								bufcatcstr(passStr, ", ");
								bufcatbuf(passStr, &bufferIm);
								bufcatcstr(passStr, ", ");
							}
							bufcatbuf(passStr, &bufOffset);
							bufcatcstr(passStr, ", pre_userdata");
							if (pass->fft_preCallback.localMemSize > 0)
								bufcatcstr(passStr, ", localmem");
							bufcatcstr(passStr, ");");
						}

						if (pass->fft_doPreCallback && pass->c2r && component == SR_COMP_REAL && hid != 0)
						{
							bufcatcstr(passStr, "\n\t");
							bufcatbuf(passStr, &regIndexC);
							bufcatcstr(passStr, " = ");
							bufcatbuf(passStr, &regIndexSub);
							bufcatcstr(passStr, ";");
						}

						bufcatcstr(passStr, "\n\t");
						bufcatbuf(passStr, &regIndexSub);
						bufcatcstr(passStr, " = ");

						// Use the return value from precallback if set
						if (pass->fft_doPreCallback && (component == SR_COMP_BOTH || pass->r2c))
						{
							if (component == SR_COMP_BOTH)
							{
								bufcatcstr(passStr, "retPrecallback");

								if (isPrecallVector)
								{
									bufcatcstr(passStr, "[");
									BUFCAT_BUFFER_VALUE(passStr, SztToStr(v));
									bufcatcstr(passStr, "]");
								}
								// Append the vector tail when callback data is
								// interleaved.
								if (interleaved)
									bufcatbuf(passStr, &tail);
								else
									bufcatcstr(passStr, (c == 0) ? ".x;" : ".y;");
							}
							else if (pass->r2c)
							{
								bufcatcstr(passStr, pass->fft_preCallback.funcname);
								bufcatcstr(passStr, "(");
								bufcatbuf(passStr, &buffer);
								bufcatcstr(passStr, ", ");
								bufcatbuf(passStr, &bufOffset);
								bufcatcstr(passStr, ", pre_userdata");

								if (pass->fft_preCallback.localMemSize > 0)
									bufcatcstr(passStr, ", localmem");
								bufcatcstr(passStr, ");");
							}
						}
						else
						{
							bufcatbuf(passStr, &buffer);
							bufcatcstr(passStr, "[");
							bufcatbuf(passStr, &bufOffset);
							bufcatcstr(passStr, "]");
							bufcatbuf(passStr, &tail);
						}
					}

					// Since we read real & imag at once, we break the loop
					if (interleaved && (component == SR_COMP_BOTH) && pass->linearRegs)
						break;
				}
			}
		}
		else if ((flag == SR_TWMUL) || (flag == SR_TWMUL_3STEP)) // twiddle multiplies and writes
		// require that 'r' loop be innermost
		{
			for (size_t v = 0; v < regC; v++)
			{
				for (size_t r = 0; r < pass->radix; r++)
				{
					buffer_t regRealIndex = buffer_empty();
					buffer_t regImagIndex = buffer_empty();
					bufset_linear_reg(&regRealIndex, pass->linearRegs, &regBaseCount);
					bufset_linear_reg(&regImagIndex, pass->linearRegs, &regBaseCount);

					if (pass->linearRegs)
					{
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regRealIndex);
						bufcatcstr(&regRealIndex, ").x");
						PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regImagIndex);
						bufcatcstr(&regImagIndex, ").y");
					}
					else
					{
						PassRegBaseAndCountAndPos(pass, "R", r, &regRealIndex);
						PassRegBaseAndCountAndPos(pass, "I", r, &regImagIndex);
					}

					if (regC != 1)
					{
						bufcatcstr(&regRealIndex, ".s");
						BUFCAT_BUFFER_VALUE(&regRealIndex, SztToStr(v));
						bufcatcstr(&regImagIndex, ".s");
						BUFCAT_BUFFER_VALUE(&regImagIndex, SztToStr(v));
					}

					if (flag == SR_TWMUL) // twiddle multiply operation
					{
						if (r == 0) // no twiddle muls needed
							continue;

						bufcatcstr(passStr, "\n\t{\n\t\t");
						bufcatbuf(passStr, &twType);
						bufcatcstr(passStr, " W = ");
						bufcatbuf(passStr, &twTable);
						bufcatcstr(passStr, "[");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->algLS - 1));
						bufcatcstr(passStr, " + ");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->radix - 1));
						bufcatcstr(passStr, "*((");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->numButterfly));
						bufcatcstr(passStr, "*me + ");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(butterflyIndex));
						bufcatcstr(passStr, ")%");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->algLS));
						bufcatcstr(passStr, ") + ");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(r - 1));
						bufcatcstr(passStr, "];\n\t\t");
					}
					else // 3-step twiddle
					{
						bufcatcstr(passStr, "\n\t{\n\t\t");
						bufcatbuf(passStr, &twType);
						bufcatcstr(passStr, " W = ");
						bufcatbuf(passStr, &tw3StepFunc);
						bufcatcstr(passStr, "( ");

						if (frontTwiddle)
						{
							assert(pass->linearRegs);
							bufcatcstr(passStr, "(");
							bufcatcstr(passStr, "me*");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->numButterfly));
							bufcatcstr(passStr, " + ");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(i));
							bufcatcstr(passStr, " + ");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(r * pass->length / pass->radix));
							bufcatcstr(passStr, ") * b");
						}
						else
						{
							bufcatcstr(passStr, "((");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->numButterfly));
							bufcatcstr(passStr, "*me + ");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(butterflyIndex));
							bufcatcstr(passStr, ")%");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->algLS));
							bufcatcstr(passStr, " + ");
							BUFCAT_BUFFER_VALUE(passStr, SztToStr(r * pass->algLS));
							bufcatcstr(passStr, ") * b");
						}

						bufcatcstr(passStr, " );\n\t\t");
					}

					bufcatbuf(passStr, &rType);
					bufcatcstr(passStr, " TR, TI;\n\t\t");

					if (pass->realSpecial && (flag == SR_TWMUL_3STEP))
					{
						if (fwd)
						{
							bufcatcstr(passStr, "if(t==0)\n\t\t{\n\t\t");

							bufcatcstr(passStr, "TR = (W.x * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") - (W.y * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");
							bufcatcstr(passStr, "TI = (W.y * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") + (W.x * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");

							bufcatcstr(passStr, "}\n\t\telse\n\t\t{\n\t\t");

							bufcatcstr(passStr, "TR = (W.x * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") + (W.y * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");
							bufcatcstr(passStr, "TI = (W.y * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") - (W.x * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");

							bufcatcstr(passStr, "}\n\t\t");
						}
						else
						{
							bufcatcstr(passStr, "if(t==0)\n\t\t{\n\t\t");

							bufcatcstr(passStr, "TR = (W.x * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") + (W.y * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");
							bufcatcstr(passStr, "TI = (W.y * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") - (W.x * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");

							bufcatcstr(passStr, "}\n\t\telse\n\t\t{\n\t\t");

							bufcatcstr(passStr, "TR = (W.x * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") - (W.y * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");
							bufcatcstr(passStr, "TI = (W.y * ");
							bufcatbuf(passStr, &regRealIndex);
							bufcatcstr(passStr, ") + (W.x * ");
							bufcatbuf(passStr, &regImagIndex);
							bufcatcstr(passStr, ");\n\t\t");

							bufcatcstr(passStr, "}\n\t\t");
						}
					}
					else if (fwd)
					{
						bufcatcstr(passStr, "TR = (W.x * ");
						bufcatbuf(passStr, &regRealIndex);
						bufcatcstr(passStr, ") - (W.y * ");
						bufcatbuf(passStr, &regImagIndex);
						bufcatcstr(passStr, ");\n\t\t");
						bufcatcstr(passStr, "TI = (W.y * ");
						bufcatbuf(passStr, &regRealIndex);
						bufcatcstr(passStr, ") + (W.x * ");
						bufcatbuf(passStr, &regImagIndex);
						bufcatcstr(passStr, ");\n\t\t");
					}
					else
					{
						bufcatcstr(passStr, "TR =  (W.x * ");
						bufcatbuf(passStr, &regRealIndex);
						bufcatcstr(passStr, ") + (W.y * ");
						bufcatbuf(passStr, &regImagIndex);
						bufcatcstr(passStr, ");\n\t\t");
						bufcatcstr(passStr, "TI = -(W.y * ");
						bufcatbuf(passStr, &regRealIndex);
						bufcatcstr(passStr, ") + (W.x * ");
						bufcatbuf(passStr, &regImagIndex);
						bufcatcstr(passStr, ");\n\t\t");
					}

					bufcatbuf(passStr, &regRealIndex);
					bufcatcstr(passStr, " = TR;\n\t\t");
					bufcatbuf(passStr, &regImagIndex);
					bufcatcstr(passStr, " = TI;\n\t}\n");
				}

				butterflyIndex++;
			}
		}
		else // write operation
		{
			for (size_t v = 0; v < regC; v++)
			{
				for (size_t r = 0; r < pass->radix; r++)
				{
					if (pass->realSpecial && (pass->nextPass == NULL) && (r > (pass->radix / 2)))
						break;

					if (pass->realSpecial && (pass->nextPass == NULL) && (r == pass->radix / 2) && (i != 0))
						break;

					if (pass->realSpecial && (pass->nextPass == NULL) && (r == pass->radix / 2) && (i == 0))
						bufcatcstr(passStr, "\n\t}\n\tif( rw && !me)\n\t{");

					buffer_t regIndexC0 = buffer_empty();

					for (size_t c = cStart; c < cEnd; c++) // component loop: 0 - real, 1 - imaginary
					{
						buffer_t tail = buffer_empty();
						buffer_t regIndex = buffer_empty();
						bufset_linear_reg(&regIndex, pass->linearRegs, &regBaseCount);
						buffer_t buffer = buffer_empty();

						// Write real & imag at once
						if (interleaved && (component == SR_COMP_BOTH) && pass->linearRegs)
						{
							assert(bufcmp(&bufferRe, &bufferIm) == 0); // Make sure Real & Imag buffer strings
							// are same for interleaved data
							bufsetbuf(&buffer, &bufferRe);
							PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
							bufcatcstr(&regIndex, ")");
							bufsetcstr(&tail, "");
						}
						else if (c == 0)
						{
							if (pass->linearRegs)
							{
								PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
								bufcatcstr(&regIndex, ").x");
							}
							else
							{
								PassRegBaseAndCountAndPos(pass, "R", r, &regIndex);
							}
							bufsetbuf(&buffer, &bufferRe);
							bufsetcstr(&tail, interleaved ? ".x" : "");
						}
						else
						{
							if (pass->linearRegs)
							{
								PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);
								bufcatcstr(&regIndex, ").y");
							}
							else
							{
								PassRegBaseAndCountAndPos(pass, "I", r, &regIndex);
							}
							bufsetbuf(&buffer, &bufferIm);
							bufsetcstr(&tail, interleaved ? ".y" : "");
						}

						if (regC != 1)
						{
							bufcatcstr(&regIndex, ".s");
							BUFCAT_BUFFER_VALUE(&regIndex, SztToStr(v));
						}

						bufcatcstr(passStr, "\n\t");

						if (scale != 1.0f)
						{
							bufcatcstr(&regIndex, " * ");
							BUFCAT_BUFFER_VALUE(&regIndex, FloatToStr(scale));
							BUFCAT_BUFFER_VALUE(&regIndex, FloatSuffix(pass->pr));
						}
						if (c == 0)
							bufcatbuf(&regIndexC0, &regIndex);

						bufclear(&bufOffset);
						bufcatcstr(&bufOffset, offset);
						bufcatcstr(&bufOffset, " + ( ");
						if ((pass->numButterfly * pass->workGroupSize) > pass->algLS)
						{
							bufcatcstr(&bufOffset, "((");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
							bufcatcstr(&bufOffset, "*me + ");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(butterflyIndex));
							bufcatcstr(&bufOffset, ")/");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->algLS));
							bufcatcstr(&bufOffset, ")*");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->algL));
							bufcatcstr(&bufOffset, " + (");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
							bufcatcstr(&bufOffset, "*me + ");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(butterflyIndex));
							bufcatcstr(&bufOffset, ")%");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->algLS));
							bufcatcstr(&bufOffset, " + ");
						}
						else
						{
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(pass->numButterfly));
							bufcatcstr(&bufOffset, "*me + ");
							BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(butterflyIndex));
							bufcatcstr(&bufOffset, " + ");
						}

						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(r * pass->algLS));
						bufcatcstr(&bufOffset, " )*");
						BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(stride));

						if (pass->fft_doPostCallback)
						{
							if (interleaved && (component == SR_COMP_BOTH))
							{
								if (c == (cEnd - 1))
								{
									bufcatcstr(passStr, "tempC.x = ");
									bufcatbuf(passStr, &regIndexC0);
									bufcatcstr(passStr, ";\n\t");
									bufcatcstr(passStr, "tempC.y = ");
									bufcatbuf(passStr, &regIndex);
									bufcatcstr(passStr, ";\n\t");

									bufcatcstr(passStr, pass->fft_postCallback.funcname);
									bufcatcstr(passStr, "(");
									bufcatbuf(passStr, &buffer);
									bufcatcstr(passStr, ", (");
									bufcatbuf(passStr, &bufOffset);
									bufcatcstr(passStr, "), post_userdata, tempC");
									if (pass->fft_postCallback.localMemSize > 0)
										bufcatcstr(passStr, ", post_localmem");
									bufcatcstr(passStr, ");");
								}
							}
							else if (c == (cEnd - 1))
							{
								bufcatcstr(passStr, pass->fft_postCallback.funcname);
								bufcatcstr(passStr, "(");
								bufcatbuf(passStr, &bufferRe);
								bufcatcstr(passStr, ", ");
								bufcatbuf(passStr, &bufferIm);
								bufcatcstr(passStr, ", (");
								bufcatbuf(passStr, &bufOffset);
								bufcatcstr(passStr, "), post_userdata, (");
								bufcatbuf(passStr, &regIndexC0);
								bufcatcstr(passStr, "), (");
								bufcatbuf(passStr, &regIndex);
								bufcatcstr(passStr, ")");
								if (pass->fft_postCallback.localMemSize > 0)
									bufcatcstr(passStr, ", post_localmem");
								bufcatcstr(passStr, ");");
							}
						}
						else
						{
							bufcatbuf(passStr, &buffer);
							bufcatcstr(passStr, "[");
							bufcatbuf(passStr, &bufOffset);
							bufcatcstr(passStr, "]");
							bufcatbuf(passStr, &tail);
							bufcatcstr(passStr, " = ");
							bufcatbuf(passStr, &regIndex);
							bufcatcstr(passStr, ";");
						}

						// Since we write real & imag at once, we break the loop
						if (interleaved && (component == SR_COMP_BOTH) && pass->linearRegs)
							break;
					}

					if (pass->realSpecial && (pass->nextPass == NULL) && (r == pass->radix / 2) && (i == 0))
						bufcatcstr(passStr, "\n\t}\n\tif(rw)\n\t{");
				}

				butterflyIndex++;
			}
		}
	}

	assert(butterflyIndex <= pass->numButterfly);
}

static void PassSweepRegsRC(const Pass *pass, size_t flag, bool fwd, bool interleaved, size_t stride, size_t component, double scale, bool setZero, bool batch2, bool oddt,
	const buffer_t bufferRe, const buffer_t bufferIm, const char *offset, buffer_t *passStr)
{
	// Emit real-complex special register sweep source.
	assert((flag == SR_READ) || (flag == SR_WRITE));

	// component: 0 - real, 1 - imaginary, 2 - both
	size_t cStart, cEnd;
	switch (component)
	{
		case SR_COMP_REAL:
			cStart = 0;
			cEnd = 1;
			break;
		case SR_COMP_IMAG:
			cStart = 1;
			cEnd = 2;
			break;
		case SR_COMP_BOTH:
			cStart = 0;
			cEnd = 2;
			break;
		default: assert(false);
	}

	buffer_t rType = RegBaseType(pass->pr, 1);

	assert(pass->r2c || pass->c2r);
	assert(pass->linearRegs);
	bool singlePass = ((pass->position == 0) && (pass->nextPass == NULL));

	size_t numCR = pass->numButterfly * pass->radix;
	if (!(numCR % 2))
		assert(!oddt);

	size_t rStart = 0;
	size_t rEnd = numCR;

	bool oddp = ((numCR % 2) && (numCR > 1) && !setZero);
	if (oddp)
	{
		if (oddt)
		{
			rStart = numCR - 1;
			rEnd = numCR + 1;
		}
		else
		{
			rStart = 0;
			rEnd = numCR - 1;
		}
	}

	if (!oddp)
		assert(!oddt);

	for (size_t r = rStart; r < rEnd; r++)
	{
		buffer_t val1StrExt = buffer_empty();

		for (size_t c = cStart; c < cEnd; c++) // component loop: 0 - real, 1 - imaginary
		{
			if (flag == SR_READ) // read operation
			{
				buffer_t tail = buffer_empty();
				buffer_t tail2 = buffer_empty();
				buffer_t regIndex = buffer_from_cstr("(*R");
				buffer_t buffer = buffer_empty();

				PassRegBaseAndCountAndPos(pass, "", r, &regIndex);
				if (pass->fft_doPreCallback && pass->c2r)
				{
					bufcatcstr(&regIndex, ")");
					if (interleaved)
					{
						bufsetbuf(&buffer, (c == 0) ? &bufferRe : &bufferIm);
					}
					else
					{
						bufcatbuf(&buffer, &bufferRe);
						bufcatcstr(&buffer, ", ");
						bufcatbuf(&buffer, &bufferIm);
					}
				}
				else if (c == 0)
				{
					bufcatcstr(&regIndex, ").x");
					bufsetbuf(&buffer, &bufferRe);
					bufsetcstr(&tail, interleaved ? ".x;" : ";");
					bufsetcstr(&tail2, interleaved ? ".y;" : ";");
				}
				else
				{
					bufcatcstr(&regIndex, ").y");
					bufsetbuf(&buffer, &bufferIm);
					bufsetcstr(&tail, interleaved ? ".y;" : ";");
					bufsetcstr(&tail2, interleaved ? ".x;" : ";");
				}

				size_t bid = numCR / 2;
				bid = bid ? bid : 1;
				size_t cid, lid;

				if (oddt)
				{
					cid = r % 2;
					lid = 1 + (numCR / 2);
				}
				else
				{
					cid = r / bid;
					lid = 1 + r % bid;
				}

				buffer_t oddpadd = buffer_from_cstr(oddp ? " (me/2) + " : " ");

				buffer_t idxStr = buffer_empty();
				buffer_t idxStrRev = buffer_empty();
				if ((pass->length <= 2) || ((pass->length & (pass->length - 1)) != 0))
				{
					BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(bid));
					bufcatcstr(&idxStr, "*me +");
					bufcatbuf(&idxStr, &oddpadd);
					BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(lid));
				}
				else
				{
					bufcatcstr(&idxStr, "me + ");
					BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(1 + pass->length * (r % bid) / numCR));
					bufcatbuf(&idxStr, &oddpadd);
				}
				BUFCAT_BUFFER_VALUE(&idxStrRev, SztToStr(pass->length));
				bufcatcstr(&idxStrRev, " - (");
				bufcatbuf(&idxStrRev, &idxStr);
				bufcatcstr(&idxStrRev, " )");

				bool act = (fwd || ((cid == 0) && (!batch2)) || ((cid != 0) && batch2));
				if (act)
				{
					bufcatcstr(passStr, "\n\t");
					bufcatbuf(passStr, &regIndex);
					bufcatcstr(passStr, " = ");
				}

				if (setZero)
				{
					if (act)
						bufcatcstr(passStr, "0;");
				}
				else
				{
					if (act)
					{
						if (pass->fft_doPreCallback)
						{
							bufcatcstr(passStr, pass->fft_preCallback.funcname);
							bufcatcstr(passStr, "(");
							bufcatbuf(passStr, &buffer);
							bufcatcstr(passStr, ", ");
						}
						else
						{
							bufcatbuf(passStr, &buffer);
							bufcatcstr(passStr, "[");
						}
						bufcatcstr(passStr, offset);
						bufcatcstr(passStr, " + ( ");
					}

					if (fwd)
					{
						if (cid == 0)
							bufcatbuf(passStr, &idxStr);
						else
							bufcatbuf(passStr, &idxStrRev);
					}
					else if (cid == 0)
					{
						if (!batch2)
							bufcatbuf(passStr, &idxStr);
					}
					else
					{
						if (batch2)
							bufcatbuf(passStr, &idxStr);
					}

					if (act)
					{
						bufcatcstr(passStr, " )*");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(stride));

						if (pass->fft_doPreCallback)
						{
							bufcatcstr(passStr, ", pre_userdata");
							bufcatcstr(passStr, (pass->fft_preCallback.localMemSize > 0) ? ", localmem);" : ");");
						}
						else
						{
							bufcatcstr(passStr, "]");

							if (fwd)
								bufcatbuf(passStr, &tail);
							else if (!batch2)
								bufcatbuf(passStr, &tail);
							else
								bufcatbuf(passStr, &tail2);
						}
					}
				}
			}
			else // write operation
			{
				buffer_t tail = buffer_empty();
				buffer_t regIndex = buffer_from_cstr("(*R");
				buffer_t regIndexPair = buffer_from_cstr("(*R");
				buffer_t buffer = buffer_empty();

				// Write real & imag at once
				if (interleaved && (component == SR_COMP_BOTH))
				{
					assert(bufcmp(&bufferRe, &bufferIm) == 0); // Make sure Real & Imag buffer strings are same
					// for interleaved data
					bufsetbuf(&buffer, &bufferRe);
				}
				else if (c == 0)
				{
					bufsetbuf(&buffer, &bufferRe);
					bufsetcstr(&tail, interleaved ? ".x" : "");
				}
				else
				{
					bufsetbuf(&buffer, &bufferIm);
					bufsetcstr(&tail, interleaved ? ".y" : "");
				}

				size_t bid, cid, lid;
				if (singlePass && fwd)
				{
					bid = 1 + pass->radix / 2;
					lid = r;
					cid = r / bid;

					PassRegBaseAndCountAndPos(pass, "", r, &regIndex);
					bufcatcstr(&regIndex, ")");
					PassRegBaseAndCountAndPos(pass, "", (pass->radix - r) % pass->radix, &regIndexPair);
					bufcatcstr(&regIndexPair, ")");
				}
				else
				{
					bid = numCR / 2;

					if (oddt)
					{
						cid = r % 2;
						lid = 1 + (numCR / 2);

						PassRegBaseAndCountAndPos(pass, "", r, &regIndex);
						bufcatcstr(&regIndex, ")");
						PassRegBaseAndCountAndPos(pass, "", r + 1, &regIndexPair);
						bufcatcstr(&regIndexPair, ")");
					}
					else
					{
						cid = r / bid;
						lid = 1 + r % bid;

						PassRegBaseAndCountAndPos(pass, "", r, &regIndex);
						bufcatcstr(&regIndex, ")");
						PassRegBaseAndCountAndPos(pass, "", r + bid, &regIndexPair);
						bufcatcstr(&regIndexPair, ")");
					}
				}

				if (!cid)
				{
					buffer_t oddpadd = buffer_from_cstr(oddp ? " (me/2) + " : " ");

					buffer_t sclStr = buffer_from_cstr("");
					if (scale != 1.0f)
					{
						bufcatcstr(&sclStr, " * ");
						BUFCAT_BUFFER_VALUE(&sclStr, FloatToStr(scale));
						BUFCAT_BUFFER_VALUE(&sclStr, FloatSuffix(pass->pr));
					}

					if (fwd)
					{
						buffer_t idxStr = buffer_empty();
						buffer_t idxStrRev = buffer_empty();
						if ((pass->length <= 2) || ((pass->length & (pass->length - 1)) != 0))
						{
							BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(pass->length / (2 * pass->workGroupSize)));
							bufcatcstr(&idxStr, "*me +");
							bufcatbuf(&idxStr, &oddpadd);
							BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(lid));
						}
						else
						{
							bufcatcstr(&idxStr, "me + ");
							BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(1 + pass->length * (r % bid) / numCR));
							bufcatbuf(&idxStr, &oddpadd);
						}
						BUFCAT_BUFFER_VALUE(&idxStrRev, SztToStr(pass->length));
						bufcatcstr(&idxStrRev, " - (");
						bufcatbuf(&idxStrRev, &idxStr);
						bufcatcstr(&idxStrRev, " )");

						buffer_t val1Str = buffer_empty();
						buffer_t val2Str = buffer_empty();

						if (pass->fft_doPostCallback && !pass->rcFull)
						{
							if (interleaved)
							{
								bufcatcstr(&val1Str, "\n\t");
								bufcatcstr(&val1Str, pass->fft_postCallback.funcname);
								bufcatcstr(&val1Str, "(");
								bufcatbuf(&val1Str, &buffer);
								bufcatcstr(&val1Str, ", ");
								bufcatcstr(&val1Str, offset);
								bufcatcstr(&val1Str, " + ( ");
								bufcatbuf(&val1Str, &idxStr);
								bufcatcstr(&val1Str, " )*");
								BUFCAT_BUFFER_VALUE(&val1Str, SztToStr(stride));
								bufcatcstr(&val1Str, ", post_userdata, ");
							}
							else if (c == 0)
							{
								bufcatcstr(&val1StrExt, "\n\t");
								bufcatcstr(&val1StrExt, pass->fft_postCallback.funcname);
								bufcatcstr(&val1StrExt, "(");
								bufcatbuf(&val1StrExt, &bufferRe);
								bufcatcstr(&val1StrExt, ", ");
								bufcatbuf(&val1StrExt, &bufferIm);
								bufcatcstr(&val1StrExt, ", ");
								bufcatcstr(&val1StrExt, offset);
								bufcatcstr(&val1StrExt, " + ( ");
								bufcatbuf(&val1StrExt, &idxStr);
								bufcatcstr(&val1StrExt, " )*");
								BUFCAT_BUFFER_VALUE(&val1StrExt, SztToStr(stride));
								bufcatcstr(&val1StrExt, ", post_userdata, ");
							}
						}
						else
						{
							bufcatcstr(&val1Str, "\n\t");
							bufcatbuf(&val1Str, &buffer);
							bufcatcstr(&val1Str, "[");
							bufcatcstr(&val1Str, offset);
							bufcatcstr(&val1Str, " + ( ");
							bufcatbuf(&val1Str, &idxStr);
							bufcatcstr(&val1Str, " )*");
							BUFCAT_BUFFER_VALUE(&val1Str, SztToStr(stride));
							bufcatcstr(&val1Str, "]");
							bufcatbuf(&val1Str, &tail);
							bufcatcstr(&val1Str, " = ");
						}

						bufcatcstr(&val2Str, "\n\t");
						bufcatbuf(&val2Str, &buffer);
						bufcatcstr(&val2Str, "[");
						bufcatcstr(&val2Str, offset);
						bufcatcstr(&val2Str, " + ( ");
						bufcatbuf(&val2Str, &idxStrRev);
						bufcatcstr(&val2Str, " )*");
						BUFCAT_BUFFER_VALUE(&val2Str, SztToStr(stride));
						bufcatcstr(&val2Str, "]");
						bufcatbuf(&val2Str, &tail);
						bufcatcstr(&val2Str, " = ");

						buffer_t real1 = buffer_empty();
						buffer_t imag1 = buffer_empty();
						buffer_t real2 = buffer_empty();
						buffer_t imag2 = buffer_empty();

						bufcatcstr(&real1, "(");
						bufcatbuf(&real1, &regIndex);
						bufcatcstr(&real1, ".x + ");
						bufcatbuf(&real1, &regIndexPair);
						bufcatcstr(&real1, ".x)*0.5");
						bufcatcstr(&imag1, "(");
						bufcatbuf(&imag1, &regIndex);
						bufcatcstr(&imag1, ".y - ");
						bufcatbuf(&imag1, &regIndexPair);
						bufcatcstr(&imag1, ".y)*0.5");
						bufcatcstr(&real2, "(");
						bufcatbuf(&real2, &regIndex);
						bufcatcstr(&real2, ".y + ");
						bufcatbuf(&real2, &regIndexPair);
						bufcatcstr(&real2, ".y)*0.5");
						bufcatcstr(&imag2, "(-");
						bufcatbuf(&imag2, &regIndex);
						bufcatcstr(&imag2, ".x + ");
						bufcatbuf(&imag2, &regIndexPair);
						bufcatcstr(&imag2, ".x)*0.5");

						if (interleaved && (component == SR_COMP_BOTH))
						{
							bufcatcstr(&val1Str, "(");
							BUFCAT_BUFFER_VALUE(&val1Str, RegBaseType(pass->pr, 2));
							bufcatcstr(&val1Str, ")( ");
							bufcatcstr(&val2Str, "(");
							BUFCAT_BUFFER_VALUE(&val2Str, RegBaseType(pass->pr, 2));
							bufcatcstr(&val2Str, ")( ");

							if (!batch2)
							{
								bufcatbuf(&val1Str, &real1);
								bufcatcstr(&val1Str, ", ");
								bufcatcstr(&val1Str, "+");
								bufcatbuf(&val1Str, &imag1);
								bufcatbuf(&val2Str, &real1);
								bufcatcstr(&val2Str, ", ");
								bufcatcstr(&val2Str, "-");
								bufcatbuf(&val2Str, &imag1);
							}
							else
							{
								bufcatbuf(&val1Str, &real2);
								bufcatcstr(&val1Str, ", ");
								bufcatcstr(&val1Str, "+");
								bufcatbuf(&val1Str, &imag2);
								bufcatbuf(&val2Str, &real2);
								bufcatcstr(&val2Str, ", ");
								bufcatcstr(&val2Str, "-");
								bufcatbuf(&val2Str, &imag2);
							}

							bufcatcstr(&val1Str, " )");
							bufcatcstr(&val2Str, " )");
						}
						else
						{
							bufcatcstr(&val1Str, " (");
							bufcatcstr(&val2Str, " (");
							if (c == 0)
							{
								if (!batch2)
								{
									bufcatbuf(&val1Str, &real1);
									bufcatbuf(&val2Str, &real1);
								}
								else
								{
									bufcatbuf(&val1Str, &real2);
									bufcatbuf(&val2Str, &real2);
								}
							}
							else if (!batch2)
							{
								bufcatcstr(&val1Str, "+");
								bufcatbuf(&val1Str, &imag1);
								bufcatcstr(&val2Str, "-");
								bufcatbuf(&val2Str, &imag1);
							}
							else
							{
								bufcatcstr(&val1Str, "+");
								bufcatbuf(&val1Str, &imag2);
								bufcatcstr(&val2Str, "-");
								bufcatbuf(&val2Str, &imag2);
							}
							bufcatcstr(&val1Str, " )");
							bufcatcstr(&val2Str, " )");
						}

						bufcatbuf(&val1Str, &sclStr);
						bufcatbuf(&val2Str, &sclStr);

						if (pass->fft_doPostCallback && !pass->rcFull)
						{
							if (!interleaved)
							{
								bufcatbuf(&val1StrExt, &val1Str);
								bufclear(&val1Str);

								if (c == 0)
									bufcatcstr(&val1StrExt, ", ");
								else
									bufcatbuf(&val1Str, &val1StrExt);
							}

							if (interleaved || c == (cEnd - 1))
							{
								if (pass->fft_postCallback.localMemSize > 0)
									bufcatcstr(&val1Str, ", localmem");
								bufcatcstr(&val1Str, ");");
							}
						}
						else
						{
							bufcatcstr(&val1Str, ";");
						}

						bufcatbuf(passStr, &val1Str);
						if (pass->rcFull)
						{
							bufcatbuf(passStr, &val2Str);
							bufcatcstr(passStr, ";");
						}
					}
					else
					{
						buffer_t idxStr = buffer_empty();
						buffer_t idxStrRev = buffer_empty();
						if ((pass->length <= 2) || ((pass->length & (pass->length - 1)) != 0))
						{
							BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(bid));
							bufcatcstr(&idxStr, "*me +");
							bufcatbuf(&idxStr, &oddpadd);
							BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(lid));
						}
						else
						{
							bufcatcstr(&idxStr, "me + ");
							BUFCAT_BUFFER_VALUE(&idxStr, SztToStr(1 + pass->length * (r % bid) / numCR));
							bufcatbuf(&idxStr, &oddpadd);
						}
						BUFCAT_BUFFER_VALUE(&idxStrRev, SztToStr(pass->length));
						bufcatcstr(&idxStrRev, " - (");
						bufcatbuf(&idxStrRev, &idxStr);
						bufcatcstr(&idxStrRev, " )");

						bufcatcstr(passStr, "\n\t");
						bufcatbuf(passStr, &buffer);
						bufcatcstr(passStr, "[");
						bufcatcstr(passStr, offset);
						bufcatcstr(passStr, " + ( ");

						if (!batch2)
							bufcatbuf(passStr, &idxStr);
						else
							bufcatbuf(passStr, &idxStrRev);

						bufcatcstr(passStr, " )*");
						BUFCAT_BUFFER_VALUE(passStr, SztToStr(stride));
						bufcatcstr(passStr, "]");
						bufcatbuf(passStr, &tail);
						bufcatcstr(passStr, " = ");

						bufcatcstr(passStr, "( ");
						if (c == 0)
						{
							bufcatcstr(&regIndex, ".x");
							bufcatcstr(&regIndexPair, pass->fft_doPreCallback ? ".y" : ".x");

							if (!batch2)
							{
								bufcatbuf(passStr, &regIndex);
								bufcatcstr(passStr, " - ");
								bufcatbuf(passStr, &regIndexPair);
							}
							else
							{
								bufcatbuf(passStr, &regIndex);
								bufcatcstr(passStr, " + ");
								bufcatbuf(passStr, &regIndexPair);
							}
						}
						else
						{
							bufcatcstr(&regIndex, ".y");
							bufcatcstr(&regIndexPair, (pass->fft_doPreCallback && oddt) ? ".x" : ".y");

							if (!batch2)
							{
								bufcatbuf(passStr, &regIndex);
								bufcatcstr(passStr, " + ");
								bufcatbuf(passStr, &regIndexPair);
							}
							else
							{
								bufcatcstr(passStr, " - ");
								bufcatbuf(passStr, &regIndex);
								bufcatcstr(passStr, " + ");
								bufcatbuf(passStr, &regIndexPair);
							}
						}
						bufcatcstr(passStr, " )");
						bufcatbuf(passStr, &sclStr);
						bufcatcstr(passStr, ";");
					}

					// Since we write real & imag at once, we break the loop
					if (interleaved && (component == SR_COMP_BOTH))
						break;
				}
			}
		}
	}
}

static void PassCallButterfly(const Pass *pass, const buffer_t bflyName, size_t regC, size_t numB, buffer_t *passStr)
{
	// Emit the butterfly call sequence for a pass.
	buffer_t regBase = buffer_empty();
	PassRegBase(pass, regC, &regBase);

	for (size_t i = 0; i < numB; i++)
	{
		buffer_t regBaseCount = buffer_copy(&regBase);
		PassRegBaseAndCount(pass, i, &regBaseCount);

		bufcatcstr(passStr, "\n\t");
		bufcatbuf(passStr, &bflyName);
		bufcatcstr(passStr, "(");

		for (size_t r = 0;; r++)
		{
			if (pass->linearRegs)
			{
				buffer_t regIndex = buffer_from_cstr("R");
				PassRegBaseAndCountAndPos(pass, "", i * pass->radix + r, &regIndex);

				bufcatbuf(passStr, &regIndex);
			}
			else
			{
				buffer_t regRealIndex = buffer_copy(&regBaseCount);
				buffer_t regImagIndex = buffer_copy(&regBaseCount);
				PassRegBaseAndCountAndPos(pass, "R", r, &regRealIndex);
				PassRegBaseAndCountAndPos(pass, "I", r, &regImagIndex);

				bufcatcstr(passStr, "&");
				bufcatbuf(passStr, &regRealIndex);
				bufcatcstr(passStr, ", ");
				bufcatcstr(passStr, "&");
				bufcatbuf(passStr, &regImagIndex);
			}

			if (r == pass->radix - 1)
			{
				bufcatcstr(passStr, ");");
				break;
			}
			else
			{
				bufcatcstr(passStr, ", ");
			}
		}
	}
}

static void PassInit(Pass *pass, Precision prVal, size_t positionVal, size_t lengthVal, size_t radixVal, size_t cnPerWIVal, size_t L, size_t LS, size_t R, bool linearRegsVal,
	bool halfLdsVal, bool r2cVal, bool c2rVal, bool rcFullVal, bool rcSimpleVal, bool realSpecialVal)
{
	// Initialize pass state explicitly from the former constructor arguments.
	pass->pr = prVal;
	pass->position = positionVal;
	pass->length = lengthVal;
	pass->radix = radixVal;
	pass->cnPerWI = cnPerWIVal;
	pass->algL = L;
	pass->algLS = LS;
	pass->algR = R;
	pass->linearRegs = linearRegsVal;
	pass->halfLds = halfLdsVal;
	pass->r2c = r2cVal;
	pass->c2r = c2rVal;
	pass->rcFull = rcFullVal;
	pass->rcSimple = rcSimpleVal;
	pass->realSpecial = realSpecialVal;
	pass->enableGrouping = true;
	pass->numB1 = 0;
	pass->numB2 = 0;
	pass->numB4 = 0;
	pass->nextPass = NULL;
	pass->fft_doPreCallback = false;
	pass->fft_doPostCallback = false;
	assert(pass->radix <= pass->length);
	assert(pass->length % pass->radix == 0);

	pass->numButterfly = pass->cnPerWI / pass->radix;
	pass->workGroupSize = pass->length / pass->cnPerWI;

	// Total number of butterflies (over all work-tems) must be divisible by LS
	assert(((pass->numButterfly * pass->workGroupSize) % pass->algLS) == 0);

	// All butterflies in one work-item should always be part of no more than 1
	// FFT transform. In other words, there should not be more than 1 FFT
	// transform per work-item.
	assert(pass->cnPerWI <= pass->length);

	// Calculate the different types of Butterflies needed
	if (pass->linearRegs || pass->r2c || pass->c2r)
	{
		pass->numB1 = pass->numButterfly;
	}
	else
	{
		pass->numB4 = pass->numButterfly / 4;
		pass->numB2 = (pass->numButterfly % 4) / 2; // can be 0 or 1
		pass->numB1 = (pass->numButterfly % 2);	    // can be 0 or 1

		assert(pass->numButterfly == (pass->numB4 * 4 + pass->numB2 * 2 + pass->numB1));
	}

	// if only half LDS can be used, we need the passes to share registers
	// and hence they need to be linear registers
	if (pass->halfLds)
		assert(pass->linearRegs);
}

static inline void PassSetPrecallback(Pass *pass, bool hasPrecallback, clfftCallbackParam precallbackParam)
{
	// Store pre-callback information for single-kernel transforms.
	pass->fft_doPreCallback = hasPrecallback;
	pass->fft_preCallback = precallbackParam;
}

static inline void PassSetPostcallback(Pass *pass, bool hasPostcallback, clfftCallbackParam postcallbackParam)
{
	// Store post-callback information for single-kernel transforms.
	pass->fft_doPostCallback = hasPostcallback;
	pass->fft_postCallback = postcallbackParam;
}

static void PassGeneratePass(const Pass *pass, bool fwd, buffer_t *passStr, bool fft_3StepTwiddle, bool twiddleFront, bool inInterleaved, bool outInterleaved, bool inReal,
	bool outReal, size_t inStride, size_t outStride, double scale, bool gIn, bool gOut)
{
	// Generate the OpenCL source for one Stockham pass.
	const buffer_t bufferInRe = buffer_from_cstr((inReal || inInterleaved) ? "bufIn" : "bufInRe");
	const buffer_t bufferInIm = buffer_from_cstr((inReal || inInterleaved) ? "bufIn" : "bufInIm");
	const buffer_t bufferOutRe = buffer_from_cstr((outReal || outInterleaved) ? "bufOut" : "bufOutRe");
	const buffer_t bufferOutIm = buffer_from_cstr((outReal || outInterleaved) ? "bufOut" : "bufOutIm");

	const buffer_t bufferInRe2 = buffer_from_cstr((inReal || inInterleaved) ? "bufIn2" : "bufInRe2");
	const buffer_t bufferInIm2 = buffer_from_cstr((inReal || inInterleaved) ? "bufIn2" : "bufInIm2");
	const buffer_t bufferOutRe2 = buffer_from_cstr((outReal || outInterleaved) ? "bufOut2" : "bufOutRe2");
	const buffer_t bufferOutIm2 = buffer_from_cstr((outReal || outInterleaved) ? "bufOut2" : "bufOutIm2");

	// for real transforms we use only B1 butteflies (regC = 1)
	if (pass->r2c || pass->c2r)
	{
		assert(pass->numB1 == pass->numButterfly);
		assert(pass->linearRegs);
	}

	// Check if it is single pass transform
	bool singlePass = ((pass->position == 0) && (pass->nextPass == NULL));
	if (singlePass)
		assert(pass->numButterfly == 1); // for single pass transforms, there can be only 1 butterfly
						 // per transform
	if (singlePass)
		assert(pass->workGroupSize == 1);

	// Register types
	buffer_t regB1Type = RegBaseType(pass->pr, 1);
	buffer_t regB2Type = RegBaseType(pass->pr, 2);
	buffer_t regB4Type = RegBaseType(pass->pr, 4);

	// Function attribute
	bufcatcstr(passStr, "__attribute__((always_inline)) void\n");

	// Function name
	BUFCAT_BUFFER_VALUE(passStr, PassName(pass->position, fwd));

	// Function arguments
	bufcatcstr(passStr, "(");
	bufcatcstr(passStr, "uint rw, uint b, ");
	if (pass->realSpecial)
		bufcatcstr(passStr, "uint t, ");
	bufcatcstr(passStr, "uint me, uint inOffset, uint outOffset, ");

	if (pass->r2c || pass->c2r)
	{
		assert(pass->halfLds);

		if (gIn)
		{
			if (inInterleaved)
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB2Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInRe);
				bufcatcstr(passStr, ", ");
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB2Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferInRe2);
					bufcatcstr(passStr, ", ");
				}
			}
			else if (inReal)
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInRe);
				bufcatcstr(passStr, ", ");
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB1Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferInRe2);
					bufcatcstr(passStr, ", ");
				}
			}
			else
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInRe);
				bufcatcstr(passStr, ", ");
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB1Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferInRe2);
					bufcatcstr(passStr, ", ");
				}
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInIm);
				bufcatcstr(passStr, ", ");
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB1Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferInIm2);
					bufcatcstr(passStr, ", ");
				}
			}
		}
		else
		{
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferInRe);
			bufcatcstr(passStr, ", ");
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferInIm);
			bufcatcstr(passStr, ", ");
		}

		if (gOut)
		{
			if (outInterleaved)
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB2Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutRe);
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, ", ");
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB2Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferOutRe2);
				}
			}
			else if (outReal)
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutRe);
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, ", ");
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB1Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferOutRe2);
				}
			}
			else
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutRe);
				bufcatcstr(passStr, ", ");
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB1Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferOutRe2);
					bufcatcstr(passStr, ", ");
				}
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutIm);
				if (!pass->rcSimple)
				{
					bufcatcstr(passStr, ", ");
					bufcatcstr(passStr, "__global ");
					bufcatbuf(passStr, &regB1Type);
					bufcatcstr(passStr, " *");
					bufcatbuf(passStr, &bufferOutIm2);
				}
			}
		}
		else
		{
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferOutRe);
			bufcatcstr(passStr, ", ");
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferOutIm);
		}
	}
	else
	{
		if (gIn)
		{
			if (inInterleaved)
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB2Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInRe);
				bufcatcstr(passStr, ", ");
			}
			else
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInRe);
				bufcatcstr(passStr, ", ");
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferInIm);
				bufcatcstr(passStr, ", ");
			}
		}
		else if (inInterleaved)
		{
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB2Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferInRe);
			bufcatcstr(passStr, ", ");
		}
		else
		{
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferInRe);
			bufcatcstr(passStr, ", ");
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferInIm);
			bufcatcstr(passStr, ", ");
		}

		if (gOut)
		{
			if (outInterleaved)
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB2Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutRe);
			}
			else
			{
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutRe);
				bufcatcstr(passStr, ", ");
				bufcatcstr(passStr, "__global ");
				bufcatbuf(passStr, &regB1Type);
				bufcatcstr(passStr, " *");
				bufcatbuf(passStr, &bufferOutIm);
			}
		}
		else if (outInterleaved)
		{
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB2Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferOutRe);
		}
		else
		{
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferOutRe);
			bufcatcstr(passStr, ", ");
			bufcatcstr(passStr, "__local ");
			bufcatbuf(passStr, &regB1Type);
			bufcatcstr(passStr, " *");
			bufcatbuf(passStr, &bufferOutIm);
		}
	}

	// Register arguments
	if (pass->linearRegs)
	{
		bufcatcstr(passStr, ", ");
		BUFCAT_BUFFER_VALUE(passStr, PassIterRegArgs(pass));
	}

	if (pass->fft_doPreCallback || pass->fft_doPostCallback)
	{
		// Include pre-callback parameters if pre-callback is set
		if (pass->fft_doPreCallback)
		{
			if ((pass->r2c && !pass->rcSimple) || pass->c2r)
				bufcatcstr(passStr, ", uint inOffset2");

			bufcatcstr(passStr, ", __global void* pre_userdata");
		}

		// Include post-callback parameters if post-callback is set
		if (pass->fft_doPostCallback)
		{
			if (pass->r2c || (pass->c2r && !pass->rcSimple))
				bufcatcstr(passStr, ", uint outOffset2");
			bufcatcstr(passStr, ", __global void* post_userdata");
		}

		if (pass->fft_doPreCallback && pass->fft_preCallback.localMemSize > 0)
			bufcatcstr(passStr, ", __local void* localmem");
		if (pass->fft_doPostCallback && pass->fft_postCallback.localMemSize > 0)
			bufcatcstr(passStr, ", __local void* post_localmem");
	}

	bufcatcstr(passStr, ")\n{\n");

	// Register Declarations
	if (!pass->linearRegs)
	{
		PassDeclareRegs(pass, regB1Type, 1, pass->numB1, passStr);
		PassDeclareRegs(pass, regB2Type, 2, pass->numB2, passStr);
		PassDeclareRegs(pass, regB4Type, 4, pass->numB4, passStr);
	}

	// odd cnPerWI processing
	bool oddp = false;
	oddp = ((pass->cnPerWI % 2) && (pass->length > 1) && (!singlePass));

	// additional register for odd
	if (!pass->rcSimple && oddp && ((pass->r2c && (pass->nextPass == NULL)) || (pass->c2r && (pass->position == 0))))
	{
		bufcatcstr(passStr, "\n\t");
		bufcatcstr(passStr, "uint brv = 0;\n\t");
		bufcatcstr(passStr, "\n\t");
		bufcatbuf(passStr, &regB2Type);
		bufcatcstr(passStr, " R");
		BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->cnPerWI));
		bufcatcstr(passStr, "[1];\n\t");
		bufcatcstr(passStr, "(*R");
		BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->cnPerWI));
		bufcatcstr(passStr, ").x = 0; ");
		bufcatcstr(passStr, "(*R");
		BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->cnPerWI));
		bufcatcstr(passStr, ").y = 0;\n");
	}

	// Special private memory for c-r 1 pass transforms
	if (!pass->rcSimple && (pass->c2r && (pass->position == 0)) && singlePass)
	{
		assert(pass->radix == pass->length);

		bufcatcstr(passStr, "\n\t");
		bufcatbuf(passStr, &regB1Type);
		bufcatcstr(passStr, " mpvt[");
		BUFCAT_BUFFER_VALUE(passStr, SztToStr(pass->length));
		bufcatcstr(passStr, "];\n");
	}

	bufcatcstr(passStr, "\n");

	// Read into registers
	if (pass->r2c)
	{
		if (pass->position == 0)
		{
			bufcatcstr(passStr, "\n\tif(rw)\n\t{");
			PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 1, pass->numB1, 0, passStr, false,
				false, false);
			bufcatcstr(passStr, "\n\t}\n");

			if (pass->rcSimple)
			{
				bufcatcstr(passStr, "\n");
				PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, true, true, false, bufferInRe2, bufferInIm2, "inOffset", passStr);
				bufcatcstr(passStr, "\n");
			}
			else
			{
				bufcatcstr(passStr, "\n\tif(rw > 1)\n\t{");
				if (pass->fft_doPreCallback)
				{
					PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, bufferInRe2, bufferInIm2, "inOffset2", 1, pass->numB1,
						0, passStr, false, false, false);
				}
				else
				{
					PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, bufferInRe2, bufferInIm2, "inOffset", 1, pass->numB1,
						0, passStr, false, false, false);
				}
				bufcatcstr(passStr, "\n\t}\n");

				bufcatcstr(passStr, "\telse\n\t{");
				PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, true, true, false, bufferInRe2, bufferInIm2, "inOffset", passStr);
				bufcatcstr(passStr, "\n\t}\n");
			}
		}
	}
	else if (pass->c2r && !pass->rcSimple)
	{
		if (pass->position == 0)
		{
			buffer_t processBufRe = buffer_copy(&bufferOutRe);
			buffer_t processBufIm = buffer_copy(&bufferOutIm);
			buffer_t processBufOffset = buffer_from_cstr("outOffset");
			size_t processBufStride = outStride;

			if (singlePass)
			{
				bufsetcstr(&processBufRe, "mpvt");
				bufsetcstr(&processBufIm, "mpvt");
				bufsetcstr(&processBufOffset, "0");
				processBufStride = 1;
			}

			bufcatcstr(passStr, "\n\tif(rw && !me)\n\t{\n\t");
			bufcatbuf(passStr, &processBufRe);
			bufcatcstr(passStr, "[");
			bufcatbuf(passStr, &processBufOffset);
			bufcatcstr(passStr, "] = ");

			if (pass->fft_doPreCallback)
			{
				bufcatcstr(passStr, pass->fft_preCallback.funcname);
				bufcatcstr(passStr, "(");
				bufcatbuf(passStr, &bufferInRe);
				if (!inInterleaved)
				{
					bufcatcstr(passStr, ", ");
					bufcatbuf(passStr, &bufferInIm);
				}
				bufcatcstr(passStr, ", inOffset, pre_userdata");
				bufcatcstr(passStr, pass->fft_preCallback.localMemSize > 0 ? ", localmem)" : ")");
			}
			else
			{
				bufcatbuf(passStr, &bufferInRe);
				bufcatcstr(passStr, "[inOffset]");
			}

			if (inInterleaved || pass->fft_doPreCallback)
				bufcatcstr(passStr, ".x;\n\t}");
			else
				bufcatcstr(passStr, ";\n\t}");

			if (pass->length > 1)
			{
				bufcatcstr(passStr, "\n\n\tif(rw)\n\t{");
				if (pass->fft_doPreCallback && !inInterleaved)
				{
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, false, false, bufferInRe, bufferInIm, "inOffset",
						passStr);
				}
				else
				{
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, false, false, bufferInRe, bufferInRe, "inOffset",
						passStr);
				}
				bufcatcstr(passStr, "\n\t}\n");

				bufcatcstr(passStr, "\n\tif(rw > 1)\n\t{");
				if (pass->fft_doPreCallback)
				{
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, true, false, bufferInRe2, bufferInIm2, "inOffset2",
						passStr);
				}
				else
				{
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, true, false, bufferInIm2, bufferInIm2, "inOffset",
						passStr);
				}

				bufcatcstr(passStr, "\n\t}\n\telse\n\t{");
				PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, true, true, false, bufferInIm2, bufferInIm2, "inOffset", passStr);
				bufcatcstr(passStr, "\n\t}\n");

				if (oddp)
				{
					bufcatcstr(passStr, "\n\tif(rw && (me%2))\n\t{");
					if (pass->fft_doPreCallback)
					{
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, false, true, bufferInRe, bufferInIm,
							"inOffset", passStr);
					}
					else
					{
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, false, true, bufferInRe, bufferInRe,
							"inOffset", passStr);
					}
					bufcatcstr(passStr, "\n\t}");
					bufcatcstr(passStr, "\n\tif((rw > 1) && (me%2))\n\t{");
					if (pass->fft_doPreCallback)
					{
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, true, true, bufferInRe2, bufferInIm2,
							"inOffset2", passStr);
					}
					else
					{
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, true, true, bufferInIm2, bufferInIm2,
							"inOffset", passStr);
					}
					bufcatcstr(passStr, "\n\t}\n");
				}

				PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_REAL, 1.0f, false, true, false, processBufRe, processBufIm,
					bufcstr(&processBufOffset), passStr);
				if (oddp)
				{
					bufcatcstr(passStr, "\n\tif(me%2)\n\t{");
					PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_REAL, 1.0f, false, true, true, processBufRe, processBufIm,
						bufcstr(&processBufOffset), passStr);
					bufcatcstr(passStr, "\n\t}\n");
				}
				PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_REAL, 1.0f, false, false, false, processBufRe, processBufIm,
					bufcstr(&processBufOffset), passStr);
				if (oddp)
				{
					bufcatcstr(passStr, "\n\tif(me%2)\n\t{");
					PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_REAL, 1.0f, false, false, true, processBufRe, processBufIm,
						bufcstr(&processBufOffset), passStr);
					bufcatcstr(passStr, "\n\t}\n");
				}
			}

			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
			PassSweepRegs(pass, SR_READ, fwd, outInterleaved, processBufStride, SR_COMP_REAL, 1.0f, false, processBufRe, processBufIm, bufcstr(&processBufOffset), 1,
				pass->numB1, 0, passStr, false, false, oddp);
			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");

			bufcatcstr(passStr, "\n\tif((rw > 1) && !me)\n\t{\n\t");
			bufcatbuf(passStr, &processBufIm);
			bufcatcstr(passStr, "[");
			bufcatbuf(passStr, &processBufOffset);
			bufcatcstr(passStr, "] = ");

			if (pass->fft_doPreCallback)
			{
				bufcatcstr(passStr, pass->fft_preCallback.funcname);
				bufcatcstr(passStr, "(");
				bufcatbuf(passStr, &bufferInRe2);
				if (!inInterleaved)
				{
					bufcatcstr(passStr, ", ");
					bufcatbuf(passStr, &bufferInIm2);
				}
				bufcatcstr(passStr, ", inOffset2, pre_userdata");
				bufcatcstr(passStr, pass->fft_preCallback.localMemSize > 0 ? ", localmem)" : ")");
			}
			else
			{
				bufcatbuf(passStr, &bufferInRe2);
				bufcatcstr(passStr, "[inOffset]");
			}
			if (inInterleaved || pass->fft_doPreCallback)
				bufcatcstr(passStr, ".x;\n\t}");
			else
				bufcatcstr(passStr, ";\n\t}");
			bufcatcstr(passStr, "\n\tif((rw == 1) && !me)\n\t{\n\t");
			bufcatbuf(passStr, &processBufIm);
			bufcatcstr(passStr, "[");
			bufcatbuf(passStr, &processBufOffset);
			bufcatcstr(passStr, "] = 0;\n\t}");

			if (pass->length > 1)
			{
				if (!pass->fft_doPreCallback)
				{
					bufcatcstr(passStr, "\n\n\tif(rw)\n\t{");
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, false, false, bufferInIm, bufferInIm, "inOffset",
						passStr);
					bufcatcstr(passStr, "\n\t}\n");

					bufcatcstr(passStr, "\n\tif(rw > 1)\n\t{");
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, true, false, bufferInRe2, bufferInRe2, "inOffset",
						passStr);
					bufcatcstr(passStr, "\n\t}\n\telse\n\t{");
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, true, true, false, bufferInRe2, bufferInRe2, "inOffset",
						passStr);
					bufcatcstr(passStr, "\n\t}");

					if (oddp)
					{
						bufcatcstr(passStr, "\n\tif(rw && (me%2))\n\t{");
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, false, true, bufferInIm, bufferInIm,
							"inOffset", passStr);
						bufcatcstr(passStr, "\n\t}");
						bufcatcstr(passStr, "\n\tif((rw > 1) && (me%2))\n\t{");
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, true, true, bufferInRe2, bufferInRe2,
							"inOffset", passStr);
						bufcatcstr(passStr, "\n\t}");
					}
				}
				bufcatcstr(passStr, "\n");

				PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_IMAG, 1.0f, false, true, false, processBufRe, processBufIm,
					bufcstr(&processBufOffset), passStr);
				if (oddp)
				{
					bufcatcstr(passStr, "\n\tif(me%2)\n\t{");
					PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_IMAG, 1.0f, false, true, true, processBufRe, processBufIm,
						bufcstr(&processBufOffset), passStr);
					bufcatcstr(passStr, "\n\t}\n");
				}
				PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_IMAG, 1.0f, false, false, false, processBufRe, processBufIm,
					bufcstr(&processBufOffset), passStr);
				if (oddp)
				{
					bufcatcstr(passStr, "\n\tif(me%2)\n\t{");
					PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, processBufStride, SR_COMP_IMAG, 1.0f, false, false, true, processBufRe, processBufIm,
						bufcstr(&processBufOffset), passStr);
					bufcatcstr(passStr, "\n\t}\n");
				}
			}

			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
			PassSweepRegs(pass, SR_READ, fwd, outInterleaved, processBufStride, SR_COMP_IMAG, 1.0f, false, processBufRe, processBufIm, bufcstr(&processBufOffset), 1,
				pass->numB1, 0, passStr, false, false, false);
			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
		}
	}
	else if ((!pass->halfLds) || (pass->halfLds && (pass->position == 0)))
	{
		bool isPrecallVector = false;
		// If precallback is set
		if (pass->fft_doPreCallback)
		{
			bufcatcstr(passStr, "\n\t");
			bufcatbuf(passStr, &regB2Type);
			bufcatcstr(passStr, " retPrecallback");

			if (pass->numB4 > 0 || pass->numB2 > 0)
			{
				bufcatcstr(passStr, "[");
				bufcatcstr(passStr, (pass->numB4 > 0) ? "4" : (pass->numB2 > 0) ? "2" : "1");
				bufcatcstr(passStr, "]");

				isPrecallVector = true;
			}
			bufcatcstr(passStr, ";");
		}
		bufcatcstr(passStr, "\n\tif(rw)\n\t{");
		PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 1, pass->numB1, 0, passStr, false,
			isPrecallVector, false);
		PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 2, pass->numB2, pass->numB1, passStr,
			false, isPrecallVector, false);
		PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 4, pass->numB4,
			2 * pass->numB2 + pass->numB1, passStr, false, isPrecallVector, false);
		bufcatcstr(passStr, "\n\t}\n");
		bufcatcstr(passStr, "\n\telse\n\t{");
		PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 1, pass->numB1, 0, passStr, true,
			isPrecallVector, false);
		PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 2, pass->numB2, pass->numB1, passStr,
			true, isPrecallVector, false);
		PassSweepRegs(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 4, pass->numB4,
			2 * pass->numB2 + pass->numB1, passStr, true, isPrecallVector, false);
		bufcatcstr(passStr, "\n\t}\n");
	}

	bufcatcstr(passStr, "\n");

	// 3-step twiddle multiplies done in the front
	bool tw3Done = false;
	if (fft_3StepTwiddle && twiddleFront)
	{
		tw3Done = true;
		if (pass->linearRegs)
		{
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, true, bufferInRe, bufferInIm, "", 1, pass->numB1, 0, passStr, false, false, false);
		}
		else
		{
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, true, bufferInRe, bufferInIm, "", 1, pass->numB1, 0, passStr, false, false, false);
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, true, bufferInRe, bufferInIm, "", 2, pass->numB2, pass->numB1, passStr, false, false,
				false);
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, true, bufferInRe, bufferInIm, "", 4, pass->numB4, 2 * pass->numB2 + pass->numB1,
				passStr, false, false, false);
		}
	}

	bufcatcstr(passStr, "\n");

	// Twiddle multiply
	if ((pass->position > 0) && (pass->radix > 1))
	{
		PassSweepRegs(pass, SR_TWMUL, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 1, pass->numB1, 0, passStr, false, false, false);
		PassSweepRegs(pass, SR_TWMUL, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 2, pass->numB2, pass->numB1, passStr, false, false, false);
		PassSweepRegs(pass, SR_TWMUL, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 4, pass->numB4, 2 * pass->numB2 + pass->numB1, passStr, false,
			false, false);
	}

	// Butterfly calls
	if (pass->radix > 1)
	{
		if (pass->numB1)
			PassCallButterfly(pass, ButterflyName(pass->radix, 1, fwd), 1, pass->numB1, passStr);
		if (pass->numB2)
			PassCallButterfly(pass, ButterflyName(pass->radix, 2, fwd), 2, pass->numB2, passStr);
		if (pass->numB4)
			PassCallButterfly(pass, ButterflyName(pass->radix, 4, fwd), 4, pass->numB4, passStr);
	}

	if (!pass->halfLds)
		bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
	bufcatcstr(passStr, "\n\n");

	// 3-step twiddle multiplies
	if (fft_3StepTwiddle && !tw3Done)
	{
		assert(pass->nextPass == NULL);
		if (pass->linearRegs)
		{
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 1, pass->numB1, 0, passStr, false, false, false);
		}
		else
		{
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 1, pass->numB1, 0, passStr, false, false, false);
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 2, pass->numB2, pass->numB1, passStr, false,
				false, false);
			PassSweepRegs(pass, SR_TWMUL_3STEP, fwd, false, 1, SR_COMP_BOTH, 1.0f, false, bufferInRe, bufferInIm, "", 4, pass->numB4, 2 * pass->numB2 + pass->numB1,
				passStr, false, false, false);
		}
	}

	// Write back from registers
	if (pass->halfLds)
	{
		// In this case, we have to write & again read back for the next pass
		// since we are using only half the lds. Number of barriers will
		// increase at the cost of halving the lds.

		if (pass->nextPass == NULL) // last pass
		{
			if (pass->r2c && !pass->rcSimple)
			{
				if (!singlePass)
				{
					PassSweepRegs(pass, SR_WRITE, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 1, pass->numB1,
						0, passStr, false, false, false);
					bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, false, false, bufferInRe, bufferInIm, "inOffset",
						passStr);
					if (oddp)
					{
						bufcatcstr(passStr, "\n\tif(me%2)\n\t{");
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_REAL, 1.0f, false, false, true, bufferInRe, bufferInIm,
							"inOffset", passStr);
						bufcatcstr(passStr, "\n\t}\n");
					}

					bufcatcstr(passStr, "\n\tif(rw && !me)\n\t{\n\t");
					if (outInterleaved)
					{
						if (pass->fft_doPostCallback)
						{
							bufcatcstr(passStr, pass->fft_postCallback.funcname);
							bufcatcstr(passStr, "(bufOut, outOffset, post_userdata, ");
							bufcatcstr(passStr, "(");
							BUFCAT_BUFFER_VALUE(passStr, RegBaseType(pass->pr, 2));
							bufcatcstr(passStr, ") ( (");
							bufcatbuf(passStr, &bufferInRe);
							bufcatcstr(passStr, "[inOffset]");
							if (scale != 1.0)
							{
								bufcatcstr(passStr, " * ");
								BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
								BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
							}
							bufcatcstr(passStr, ") , 0 )");
							if (pass->fft_postCallback.localMemSize > 0)
								bufcatcstr(passStr, ", localmem");
							bufcatcstr(passStr, ");\n\t}");
						}
						else
						{
							bufcatbuf(passStr, &bufferOutRe);
							bufcatcstr(passStr, "[outOffset].x = ");
							bufcatbuf(passStr, &bufferInRe);
							bufcatcstr(passStr, "[inOffset]");
							if (scale != 1.0)
							{
								bufcatcstr(passStr, " * ");
								BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
								BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
							}
							bufcatcstr(passStr, ";\n\t");
							bufcatbuf(passStr, &bufferOutIm);
							bufcatcstr(passStr, "[outOffset].y = ");
							bufcatcstr(passStr, "0;\n\t}");
						}
					}
					else if (pass->fft_doPostCallback)
					{
						bufcatcstr(passStr, pass->fft_postCallback.funcname);
						bufcatcstr(passStr, "(");
						bufcatbuf(passStr, &bufferOutRe);
						bufcatcstr(passStr, ", ");
						bufcatbuf(passStr, &bufferOutIm);
						bufcatcstr(passStr, ", outOffset, post_userdata, ");
						bufcatbuf(passStr, &bufferInRe);
						bufcatcstr(passStr, "[inOffset]");
						if (scale != 1.0)
						{
							bufcatcstr(passStr, " * ");
							BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
							BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
						}
						bufcatcstr(passStr, ", 0");
						if (pass->fft_postCallback.localMemSize > 0)
							bufcatcstr(passStr, ", localmem");
						bufcatcstr(passStr, ");\n\t}");
					}
					else
					{
						bufcatbuf(passStr, &bufferOutRe);
						bufcatcstr(passStr, "[outOffset] = ");
						bufcatbuf(passStr, &bufferInRe);
						bufcatcstr(passStr, "[inOffset]");
						if (scale != 1.0)
						{
							bufcatcstr(passStr, " * ");
							BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
							BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
						}
						bufcatcstr(passStr, ";\n\t");
						bufcatbuf(passStr, &bufferOutIm);
						bufcatcstr(passStr, "[outOffset] = ");
						bufcatcstr(passStr, "0;\n\t}");
					}
					bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");

					PassSweepRegs(pass, SR_WRITE, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, bufferInRe, bufferInIm, "inOffset", 1, pass->numB1,
						0, passStr, false, false, false);
					bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
					PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, false, false, bufferInRe, bufferInIm, "inOffset",
						passStr);
					if (oddp)
					{
						bufcatcstr(passStr, "\n\tif(me%2)\n\t{");
						PassSweepRegsRC(pass, SR_READ, fwd, inInterleaved, inStride, SR_COMP_IMAG, 1.0f, false, false, true, bufferInRe, bufferInIm,
							"inOffset", passStr);
						bufcatcstr(passStr, "\n\t}\n");
					}

					bufcatcstr(passStr, "\n\tif((rw > 1) && !me)\n\t{\n\t");
					if (outInterleaved)
					{
						if (pass->fft_doPostCallback)
						{
							bufcatcstr(passStr, pass->fft_postCallback.funcname);
							bufcatcstr(passStr, "(bufOut2, outOffset2, post_userdata, ");
							bufcatcstr(passStr, "(");
							BUFCAT_BUFFER_VALUE(passStr, RegBaseType(pass->pr, 2));
							bufcatcstr(passStr, ") ( (");
							bufcatbuf(passStr, &bufferInIm);
							bufcatcstr(passStr, "[inOffset]");
							if (scale != 1.0)
							{
								bufcatcstr(passStr, " * ");
								BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
								BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
							}
							bufcatcstr(passStr, ") , 0 )");
							if (pass->fft_postCallback.localMemSize > 0)
								bufcatcstr(passStr, ", localmem");
							bufcatcstr(passStr, ");\n\t}");
						}
						else
						{
							bufcatbuf(passStr, &bufferOutRe2);
							bufcatcstr(passStr, "[outOffset].x = ");
							bufcatbuf(passStr, &bufferInIm);
							bufcatcstr(passStr, "[inOffset]");
							if (scale != 1.0)
							{
								bufcatcstr(passStr, " * ");
								BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
								BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
							}
							bufcatcstr(passStr, ";\n\t");
							bufcatbuf(passStr, &bufferOutIm2);
							bufcatcstr(passStr, "[outOffset].y = ");
							bufcatcstr(passStr, "0;\n\t}");
						}
					}
					else if (pass->fft_doPostCallback)
					{
						bufcatcstr(passStr, pass->fft_postCallback.funcname);
						bufcatcstr(passStr, "(");
						bufcatbuf(passStr, &bufferOutRe2);
						bufcatcstr(passStr, ", ");
						bufcatbuf(passStr, &bufferOutIm2);
						bufcatcstr(passStr, ", outOffset2, post_userdata, ");
						bufcatbuf(passStr, &bufferInIm);
						bufcatcstr(passStr, "[inOffset]");
						if (scale != 1.0)
						{
							bufcatcstr(passStr, " * ");
							BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
							BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
						}
						bufcatcstr(passStr, ", 0");
						if (pass->fft_postCallback.localMemSize > 0)
							bufcatcstr(passStr, ", localmem");
						bufcatcstr(passStr, ");\n\t}");
					}
					else
					{
						bufcatbuf(passStr, &bufferOutRe2);
						bufcatcstr(passStr, "[outOffset] = ");
						bufcatbuf(passStr, &bufferInIm);
						bufcatcstr(passStr, "[inOffset]");
						if (scale != 1.0)
						{
							bufcatcstr(passStr, " * ");
							BUFCAT_BUFFER_VALUE(passStr, FloatToStr(scale));
							BUFCAT_BUFFER_VALUE(passStr, FloatSuffix(pass->pr));
						}
						bufcatcstr(passStr, ";\n\t");
						bufcatbuf(passStr, &bufferOutIm2);
						bufcatcstr(passStr, "[outOffset] = ");
						bufcatcstr(passStr, "0;\n\t}");
					}
					bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
				}

				bufcatcstr(passStr, "\n\n\tif(rw)\n\t{");
				PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, false, false, bufferOutRe, bufferOutIm, "outOffset",
					passStr);
				bufcatcstr(passStr, "\n\t}\n");
				if (oddp)
				{
					bufcatcstr(passStr, "\n\n\tbrv = ((rw != 0) & (me%2 == 1));\n\t");
					bufcatcstr(passStr, "if(brv)\n\t{");
					PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, false, true, bufferOutRe, bufferOutIm,
						"outOffset", passStr);
					bufcatcstr(passStr, "\n\t}\n");
				}

				bufcatcstr(passStr, "\n\n\tif(rw > 1)\n\t{");

				buffer_t outOffset = buffer_empty();
				bufcatcstr(&outOffset, "outOffset");
				if (pass->fft_doPostCallback)
					bufcatcstr(&outOffset, "2");

				PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, true, false, bufferOutRe2, bufferOutIm2,
					bufcstr(&outOffset), passStr);
				bufcatcstr(passStr, "\n\t}\n");
				if (oddp)
				{
					bufcatcstr(passStr, "\n\n\tbrv = ((rw > 1) & (me%2 == 1));\n\t");
					bufcatcstr(passStr, "if(brv)\n\t{");
					PassSweepRegsRC(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, true, true, bufferOutRe2, bufferOutIm2,
						bufcstr(&outOffset), passStr);
					bufcatcstr(passStr, "\n\t}\n");
				}
			}
			else if (pass->c2r)
			{
				bufcatcstr(passStr, "\n\tif(rw)\n\t{");
				PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_REAL, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1, pass->numB1, 0,
					passStr, false, false, false);
				bufcatcstr(passStr, "\n\t}\n");

				if (!pass->rcSimple)
				{
					buffer_t outOffset = buffer_empty();
					bufcatcstr(&outOffset, "outOffset");
					if (pass->fft_doPostCallback)
						bufcatcstr(&outOffset, "2");

					bufcatcstr(passStr, "\n\tif(rw > 1)\n\t{");
					PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_IMAG, scale, false, bufferOutRe2, bufferOutIm2, bufcstr(&outOffset),
						1, pass->numB1, 0, passStr, false, false, false);
					bufcatcstr(passStr, "\n\t}\n");
				}
			}
			else
			{
				bufcatcstr(passStr, "\n\tif(rw)\n\t{");
				PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1, pass->numB1, 0,
					passStr, false, false, false);
				bufcatcstr(passStr, "\n\t}\n");
			}
		}
		else
		{
			bufcatcstr(passStr, "\n\tif(rw)\n\t{");
			PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_REAL, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1, pass->numB1, 0, passStr,
				false, false, false);
			bufcatcstr(passStr, "\n\t}\n");
			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
			bufcatcstr(passStr, "\n\tif(rw)\n\t{");
			PassSweepRegs(pass->nextPass, SR_READ, fwd, outInterleaved, outStride, SR_COMP_REAL, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1,
				pass->nextPass->numB1, 0, passStr, false, false, false);
			bufcatcstr(passStr, "\n\t}\n");
			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
			bufcatcstr(passStr, "\n\tif(rw)\n\t{");
			PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_IMAG, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1, pass->numB1, 0, passStr,
				false, false, false);
			bufcatcstr(passStr, "\n\t}\n");
			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
			bufcatcstr(passStr, "\n\tif(rw)\n\t{");
			PassSweepRegs(pass->nextPass, SR_READ, fwd, outInterleaved, outStride, SR_COMP_IMAG, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1,
				pass->nextPass->numB1, 0, passStr, false, false, false);
			bufcatcstr(passStr, "\n\t}\n");
			bufcatcstr(passStr, "\n\n\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
		}
	}
	else
	{
		if (pass->fft_doPostCallback && outInterleaved)
		{
			bufcatcstr(passStr, "\n\t");
			bufcatbuf(passStr, &regB2Type);
			bufcatcstr(passStr, " tempC;");
		}
		bufcatcstr(passStr, "\n\tif(rw)\n\t{");
		PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, bufferOutRe, bufferOutIm, "outOffset", 1, pass->numB1, 0, passStr, false,
			false, false);
		PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, bufferOutRe, bufferOutIm, "outOffset", 2, pass->numB2, pass->numB1,
			passStr, false, false, false);
		PassSweepRegs(pass, SR_WRITE, fwd, outInterleaved, outStride, SR_COMP_BOTH, scale, false, bufferOutRe, bufferOutIm, "outOffset", 4, pass->numB4,
			2 * pass->numB2 + pass->numB1, passStr, false, false, false);
		bufcatcstr(passStr, "\n\t}\n");
	}

	bufcatcstr(passStr, "\n}\n\n");
}

// FFT kernel

typedef enum KernelBlockSizeValueType
{
	BS_VT_WGS,
	BS_VT_BWD,
	BS_VT_LDS,
} KernelBlockSizeValueType;

static size_t KernelBlockSizesGetValue(Precision pr, size_t N, KernelBlockSizeValueType vt)
{
	// Calculate the requested block-size parameter for the Stockham block
	// kernel.
	size_t wgs; // preferred work group size
	size_t bwd; // block width to be used
	size_t lds; // LDS size to be used for the block

	size_t t_wgs, t_nt;
	KernelCoreSpecsGetWGSAndNT(pr, N, &t_wgs, &t_nt);

	switch (N)
	{
		case 256:
			bwd = 8 / StockhamPrecisionWidth(pr);
			wgs = (bwd > t_nt) ? 256 : t_wgs;
			break;
		case 128:
			bwd = 8 / StockhamPrecisionWidth(pr);
			wgs = (bwd > t_nt) ? 128 : t_wgs;
			break;
		case 64:
			bwd = 16 / StockhamPrecisionWidth(pr);
			wgs = (bwd > t_nt) ? 128 : t_wgs;
			break;
		case 32:
			bwd = 32 / StockhamPrecisionWidth(pr);
			wgs = (bwd > t_nt) ? 64 : t_wgs;
			break;
		case 16:
			bwd = 64 / StockhamPrecisionWidth(pr);
			wgs = (bwd > t_nt) ? 64 : t_wgs;
			break;
		case 8:
			bwd = 128 / StockhamPrecisionWidth(pr);
			wgs = (bwd > t_nt) ? 64 : t_wgs;
			break;
		default: assert(false);
	}

	// block width cannot be less than numTrans, math in other parts of code
	// depend on this assumption
	assert(bwd >= t_nt);

	lds = N * bwd;

	switch (vt)
	{
		case BS_VT_WGS: return wgs;
		case BS_VT_BWD: return bwd;
		case BS_VT_LDS: return lds;
		default: assert(false); return 0;
	}
}

static inline size_t KernelBlockLdsSize(Precision pr, size_t N)
{
	// Return the local-memory footprint for a Stockham block kernel.
	return KernelBlockSizesGetValue(pr, N, BS_VT_LDS);
}

static inline size_t KernelBlockWidth(Precision pr, size_t N)
{
	// Return the block width for a Stockham block kernel.
	return KernelBlockSizesGetValue(pr, N, BS_VT_BWD);
}

static inline size_t KernelBlockWorkGroupSize(Precision pr, size_t N)
{
	// Return the work-group size for a Stockham block kernel.
	return KernelBlockSizesGetValue(pr, N, BS_VT_WGS);
}

typedef struct KernelPassArray
{
	Pass *buf;
	size_t len;
	size_t as;
} KernelPassArray;

typedef struct Kernel
{
	Precision precision;
	size_t length;		      // Length of FFT
	size_t workGroupSize;	      // Work group size
	size_t cnPerWI;		      // complex numbers per work-item
	size_t numTrans;	      // Number of transforms per work-group
	size_t workGroupSizePerTrans; // Work group subdivision per transform
	size_t numPasses;	      // Number of FFT passes
	array_size_t radices;	      // Base radix at each pass
	KernelPassArray passes;	      // Array of pass objects
	bool halfLds;		      // LDS used to store one component at a time
	bool linearRegs;	      // scalar registers
	bool r2c2r;		      // real to complex or complex to real transform
	bool r2c, c2r;
	bool rcFull;
	bool rcSimple;
	bool blockCompute; // Column/block compute mode
	BlockComputeType blockComputeType;
	size_t blockWidth, blockWGS, blockLDS;
	bool realSpecial;
	FFTKernelGenKeyParams params; // key params
} Kernel;

static inline buffer_t KernelIterRegs(const Kernel *kernel, const char *pfx, bool initComma)
{
	// Build the linear register list for a Stockham kernel.
	buffer_t str = buffer_from_cstr("");

	if (kernel->linearRegs)
	{
		if (initComma)
			bufcatcstr(&str, ", ");

		for (size_t i = 0; i < kernel->cnPerWI; i++)
		{
			if (i != 0)
				bufcatcstr(&str, ", ");
			bufcatcstr(&str, pfx);
			bufcatcstr(&str, "R");
			BUFCAT_BUFFER_VALUE(&str, SztToStr(i));
		}
	}

	return str;
}

static inline bool KernelIsGroupedReadWritePossible(const Kernel *kernel)
{
	// Check whether grouped reads and writes are valid for this kernel.
	bool possible = true;
	const size_t *iStride, *oStride;

	if (kernel->r2c2r)
		return false;

	if (kernel->realSpecial)
		return false;

	if (kernel->params.fft_placeness == CLFFT_INPLACE)
	{
		iStride = oStride = kernel->params.fft_inStride;
	}
	else
	{
		iStride = kernel->params.fft_inStride;
		oStride = kernel->params.fft_outStride;
	}

	for (size_t i = 1; i < kernel->params.fft_DataDim; i++)
	{
		if (iStride[i] % 2)
		{
			possible = false;
			break;
		}
		if (oStride[i] % 2)
		{
			possible = false;
			break;
		}
	}

	return possible;
}

static inline buffer_t KernelOffsetCalcBlock(const Kernel *kernel, const char *off, bool input)
{
	// Generate offset source for block-compute kernels.
	buffer_t str = buffer_empty();

	const size_t *pStride = input ? kernel->params.fft_inStride : kernel->params.fft_outStride;

	bufcatcstr(&str, "\t");
	bufcatcstr(&str, off);
	bufcatcstr(&str, " = ");
	buffer_t nextBatch = buffer_from_cstr("batch");
	for (size_t i = (kernel->params.fft_DataDim - 1); i > 2; i--)
	{
		size_t currentLength = 1;
		for (int j = 2; j < i; j++)
			currentLength *= kernel->params.fft_N[j];
		currentLength *= (kernel->params.fft_N[1] / kernel->blockWidth);

		bufcatcstr(&str, "(");
		bufcatbuf(&str, &nextBatch);
		bufcatcstr(&str, "/");
		BUFCAT_BUFFER_VALUE(&str, SztToStr(currentLength));
		bufcatcstr(&str, ")*");
		BUFCAT_BUFFER_VALUE(&str, SztToStr(pStride[i]));
		bufcatcstr(&str, " + ");

		// Rebuild the next batch expression without temporary merge helpers.
		buffer_t nextBatchOld = buffer_empty();
		buffer_t currentLengthStr = SztToStr(currentLength);
		bufsetbuf(&nextBatchOld, &nextBatch);
		bufsetcstr(&nextBatch, "(");
		bufcatbuf(&nextBatch, &nextBatchOld);
		bufcatcstr(&nextBatch, "%");
		bufcatbuf(&nextBatch, &currentLengthStr);
		bufcatcstr(&nextBatch, ")");
	}

	bufcatcstr(&str, "(");
	bufcatbuf(&str, &nextBatch);
	bufcatcstr(&str, "/");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(kernel->params.fft_N[1] / kernel->blockWidth));
	bufcatcstr(&str, ")*");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(pStride[2]));
	bufcatcstr(&str, " + (");
	bufcatbuf(&str, &nextBatch);
	bufcatcstr(&str, "%");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(kernel->params.fft_N[1] / kernel->blockWidth));
	bufcatcstr(&str, ")*");
	if ((input && (kernel->blockComputeType == BCT_R2C)) || (!input && (kernel->blockComputeType == BCT_C2R)))
		BUFCAT_BUFFER_VALUE(&str, SztToStr(kernel->blockWidth * kernel->length));
	else
		BUFCAT_BUFFER_VALUE(&str, SztToStr(kernel->blockWidth));
	bufcatcstr(&str, ";\n");

	return str;
}

static inline buffer_t KernelOffsetCalc(const Kernel *kernel, const char *off, bool input, bool rc_second_index)
{
	// Generate offset source for normal Stockham kernels.
	buffer_t str = buffer_empty();

	const size_t *pStride = input ? kernel->params.fft_inStride : kernel->params.fft_outStride;

	buffer_t batch = buffer_empty();
	if (kernel->r2c2r && !kernel->rcSimple)
	{
		bufcatcstr(&batch, "(batch*");
		BUFCAT_BUFFER_VALUE(&batch, SztToStr(2 * kernel->numTrans));
		if (rc_second_index)
			bufcatcstr(&batch, " + 1");
		else
			bufcatcstr(&batch, " + 0");

		if (kernel->numTrans != 1)
		{
			bufcatcstr(&batch, " + 2*(me/");
			BUFCAT_BUFFER_VALUE(&batch, SztToStr(kernel->workGroupSizePerTrans));
			bufcatcstr(&batch, "))");
		}
		else
		{
			bufcatcstr(&batch, ")");
		}
	}
	else if (kernel->numTrans == 1)
	{
		bufcatcstr(&batch, "batch");
	}
	else
	{
		bufcatcstr(&batch, "(batch*");
		BUFCAT_BUFFER_VALUE(&batch, SztToStr(kernel->numTrans));
		bufcatcstr(&batch, " + (me/");
		BUFCAT_BUFFER_VALUE(&batch, SztToStr(kernel->workGroupSizePerTrans));
		bufcatcstr(&batch, "))");
	}

	bufcatcstr(&str, "\t");
	bufcatcstr(&str, off);
	bufcatcstr(&str, " = ");
	buffer_t nextBatch = buffer_copy(&batch);
	for (size_t i = (kernel->params.fft_DataDim - 1); i > 1; i--)
	{
		size_t currentLength = 1;
		for (int j = 1; j < i; j++)
			currentLength *= kernel->params.fft_N[j];

		bufcatcstr(&str, "(");
		bufcatbuf(&str, &nextBatch);
		bufcatcstr(&str, "/");
		BUFCAT_BUFFER_VALUE(&str, SztToStr(currentLength));
		bufcatcstr(&str, ")*");
		BUFCAT_BUFFER_VALUE(&str, SztToStr(pStride[i]));
		bufcatcstr(&str, " + ");

		// Rebuild the next batch expression without temporary merge helpers.
		buffer_t nextBatchOld = buffer_empty();
		buffer_t currentLengthStr = SztToStr(currentLength);
		bufsetbuf(&nextBatchOld, &nextBatch);
		bufsetcstr(&nextBatch, "(");
		bufcatbuf(&nextBatch, &nextBatchOld);
		bufcatcstr(&nextBatch, "%");
		bufcatbuf(&nextBatch, &currentLengthStr);
		bufcatcstr(&nextBatch, ")");
	}

	bufcatbuf(&str, &nextBatch);
	bufcatcstr(&str, "*");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(pStride[1]));
	bufcatcstr(&str, ";\n");

	return str;
}

static void KernelInit(Kernel *kernel, Precision precisionVal, const FFTKernelGenKeyParams paramsVal)
{
	// Initialize kernel state explicitly from the former constructor arguments.
	kernel->precision = precisionVal;
	kernel->params = paramsVal;
	kernel->r2c2r = false;
	// Initialize member dynamic arrays before appending generated pass data.
	array_init(&kernel->radices);
	array_init(&kernel->passes);

	kernel->length = kernel->params.fft_N[0];
	kernel->workGroupSize = kernel->params.fft_SIMD;
	kernel->numTrans = (kernel->workGroupSize * kernel->params.fft_R) / kernel->length;

	kernel->r2c = false;
	kernel->c2r = false;
	// Check if it is R2C or C2R transform
	if (kernel->params.fft_inputLayout == CLFFT_REAL)
		kernel->r2c = true;
	if (kernel->params.fft_outputLayout == CLFFT_REAL)
		kernel->c2r = true;
	kernel->r2c2r = (kernel->r2c || kernel->c2r);

	if (kernel->r2c)
	{
		kernel->rcFull = ((kernel->params.fft_outputLayout == CLFFT_COMPLEX_INTERLEAVED) || (kernel->params.fft_outputLayout == CLFFT_COMPLEX_PLANAR)) ? true : false;
	}
	if (kernel->c2r)
	{
		kernel->rcFull = ((kernel->params.fft_inputLayout == CLFFT_COMPLEX_INTERLEAVED) || (kernel->params.fft_inputLayout == CLFFT_COMPLEX_PLANAR)) ? true : false;
	}

	kernel->rcSimple = kernel->params.fft_RCsimple;

	kernel->halfLds = true;
	kernel->linearRegs = true;

	kernel->realSpecial = kernel->params.fft_realSpecial;

	kernel->blockCompute = kernel->params.blockCompute;
	kernel->blockComputeType = kernel->params.blockComputeType;
	// Make sure we can utilize all Lds if we are going to
	// use blocked columns to compute FFTs
	if (kernel->blockCompute)
	{
		assert(kernel->length <= 256); // 256 parameter comes from prototype experiments
					       // largest length at which block column possible given 32KB
					       // LDS limit if LDS limit is different this number need to
					       // be changed appropriately
		kernel->halfLds = false;
		kernel->linearRegs = true;
	}

	assert(((kernel->length * kernel->numTrans) % kernel->workGroupSize) == 0);
	kernel->cnPerWI = (kernel->numTrans * kernel->length) / kernel->workGroupSize;
	kernel->workGroupSizePerTrans = kernel->workGroupSize / kernel->numTrans;

	// !!!! IMPORTANT !!!! Keep these assertions unchanged, algorithm depend on
	// these to be true
	assert((kernel->cnPerWI * kernel->workGroupSize) == (kernel->numTrans * kernel->length));
	assert(kernel->cnPerWI <= kernel->length); // Don't do more than 1 fft per work-item

	// Breakdown into passes

	size_t LS = 1;
	size_t L;
	size_t R = kernel->length;
	size_t pid = 0;

	// See if we can get radices from the lookup table
	const size_t *pRadices = NULL;
	size_t nPasses;
	KernelCoreSpecsGetRadices(kernel->precision, kernel->length, &nPasses, &pRadices);
	if ((kernel->params.fft_MaxWorkGroupSize >= 256) && (pRadices != NULL))
	{
		for (size_t i = 0; i < nPasses; i++)
		{
			size_t rad = pRadices[i];
			L = LS * rad;
			R /= rad;

			array_push_back(&kernel->radices, rad);
			// Initialize and append a pass object explicitly.
			Pass passValue0;
			PassInit(&passValue0, kernel->precision, i, kernel->length, rad, kernel->cnPerWI, L, LS, R, kernel->linearRegs, kernel->halfLds, kernel->r2c, kernel->c2r,
				kernel->rcFull, kernel->rcSimple, kernel->realSpecial);
			array_push_back(&kernel->passes, passValue0);

			// Pass precallback information to Pass object if its the first
			// pass. This will be used in single kernel transforms
			if (kernel->params.fft_hasPreCallback && i == 0 && !kernel->params.blockCompute)
			{
				PassSetPrecallback(&kernel->passes.buf[0], kernel->params.fft_hasPreCallback, kernel->params.fft_preCallback);
			}

			// Pass post-callback information to Pass object if its the last
			// pass. This will be used in single kernel transforms
			if (kernel->params.fft_hasPostCallback && i == (nPasses - 1) && !kernel->params.blockCompute)
			{
				PassSetPostcallback(&kernel->passes.buf[i], kernel->params.fft_hasPostCallback, kernel->params.fft_postCallback);
			}

			LS *= rad;
		}
		assert(R == 1); // this has to be true for correct radix composition of
				// the length
		kernel->numPasses = nPasses;
	}
	else
	{
		// Possible radices
		size_t cRad[] = { 13, 11, 10, 8, 7, 6, 5, 4, 3, 2, 1 }; // Must be in descending order
		size_t cRadSize = (sizeof(cRad) / sizeof(cRad[0]));

		// Generate the radix and pass objects
		while (true)
		{
			size_t rad;

			assert(cRadSize >= 1);

			// Picks the radices in descending order (biggest radix first)
			for (size_t r = 0; r < cRadSize; r++)
			{
				rad = cRad[r];

				if ((rad > kernel->cnPerWI) || (kernel->cnPerWI % rad))
					continue;

				if (!(R % rad))
					break;
			}

			assert((kernel->cnPerWI % rad) == 0);

			L = LS * rad;
			R /= rad;

			array_push_back(&kernel->radices, rad);
			// Initialize and append a pass object explicitly.
			Pass passValue1;
			PassInit(&passValue1, kernel->precision, pid, kernel->length, rad, kernel->cnPerWI, L, LS, R, kernel->linearRegs, kernel->halfLds, kernel->r2c, kernel->c2r,
				kernel->rcFull, kernel->rcSimple, kernel->realSpecial);
			array_push_back(&kernel->passes, passValue1);

			// Pass precallback information to Pass object if its the first
			// pass. This will be used in single kernel transforms
			if (pid == 0 && kernel->params.fft_hasPreCallback)
			{
				PassSetPrecallback(&kernel->passes.buf[0], kernel->params.fft_hasPreCallback, kernel->params.fft_preCallback);
			}

			pid++;
			LS *= rad;

			assert(R >= 1);
			if (R == 1)
				break;
		}
		kernel->numPasses = pid;

		// Pass post-callback information to Pass object if its the last pass.
		// This will be used in single kernel transforms
		if (kernel->params.fft_hasPostCallback)
		{
			PassSetPostcallback(&kernel->passes.buf[kernel->numPasses - 1], kernel->params.fft_hasPostCallback, kernel->params.fft_postCallback);
		}
	}

	assert(kernel->numPasses == array_size(&kernel->passes));
	assert(kernel->numPasses == array_size(&kernel->radices));

	// Grouping read/writes ok?
	bool grp = KernelIsGroupedReadWritePossible(kernel);
	for (size_t i = 0; i < kernel->numPasses; i++)
		kernel->passes.buf[i].enableGrouping = grp;

	// Store the next pass-object pointers
	if (kernel->numPasses > 1)
		for (size_t i = 0; i < (kernel->numPasses - 1); i++)
			kernel->passes.buf[i].nextPass = &kernel->passes.buf[i + 1];

	if (kernel->blockCompute)
	{
		kernel->blockWidth = KernelBlockWidth(kernel->precision, kernel->length);
		kernel->blockWGS = KernelBlockWorkGroupSize(kernel->precision, kernel->length);
		kernel->blockLDS = KernelBlockLdsSize(kernel->precision, kernel->length);
	}
	else
	{
		kernel->blockWidth = kernel->blockWGS = kernel->blockLDS = 0;
	}
}

static void KernelGenerateKernel(Kernel *kernel, buffer_t *str, cl_device_id Dev_ID)
{
	// Generate the OpenCL source for one Stockham kernel.
	buffer_t twType = RegBaseType(kernel->precision, 2);
	buffer_t rType = RegBaseType(kernel->precision, 1);
	buffer_t r2Type = RegBaseType(kernel->precision, 2);

	bool inInterleaved;  // Input is interleaved format
	bool outInterleaved; // Output is interleaved format
	inInterleaved = ((kernel->params.fft_inputLayout == CLFFT_COMPLEX_INTERLEAVED) || (kernel->params.fft_inputLayout == CLFFT_HERMITIAN_INTERLEAVED)) ? true : false;
	outInterleaved = ((kernel->params.fft_outputLayout == CLFFT_COMPLEX_INTERLEAVED) || (kernel->params.fft_outputLayout == CLFFT_HERMITIAN_INTERLEAVED)) ? true : false;

	// use interleaved LDS when halfLds constraint absent
	bool ldsInterleaved = inInterleaved || outInterleaved;
	ldsInterleaved = kernel->halfLds ? false : ldsInterleaved;
	ldsInterleaved = kernel->blockCompute ? true : ldsInterleaved;

	bool inReal;  // Input is real format
	bool outReal; // Output is real format
	inReal = (kernel->params.fft_inputLayout == CLFFT_REAL) ? true : false;
	outReal = (kernel->params.fft_outputLayout == CLFFT_REAL) ? true : false;

	size_t large1D = 0;
	if (kernel->params.fft_realSpecial)
		large1D = kernel->params.fft_N[0] * kernel->params.fft_realSpecial_Nr;
	else
		large1D = kernel->params.fft_N[0] * kernel->params.fft_N[1];

	// Pragma
	BUFCAT_BUFFER_VALUE(str, ClPragma(kernel->precision));

	// Twiddle table
	if (kernel->length > 1)
	{
		TwiddleTable twTable;
		// Initialize twiddle table storage explicitly.
		TwiddleTableInit(&twTable, kernel->length);

		bufcatcstr(str, "\n__constant ");
		bufcatbuf(str, &twType);
		bufcatcstr(str, " ");
		BUFCAT_BUFFER_VALUE(str, TwTableName());
		bufcatcstr(str, "[");
		BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->length - 1));
		bufcatcstr(str, "] = {\n");
		TwiddleTableGenerateTwiddleTable(&twTable, kernel->precision, kernel->radices, str);
		// Release twiddle table storage after emitting source.
		TwiddleTableFree(&twTable);
		bufcatcstr(str, "};\n\n");
	}
	bufcatcstr(str, "\n");

	// twiddle factors for 1d-large 3-step algorithm
	if (kernel->params.fft_3StepTwiddle)
	{
		TwiddleTableLarge twLarge;
		// Initialize large twiddle table storage explicitly.
		TwiddleTableLargeInit(&twLarge, large1D);
		TwiddleTableLargeGenerateTwiddleTable(&twLarge, kernel->precision, str);
		// Release large twiddle table storage after emitting source.
		TwiddleTableLargeFree(&twLarge);
	}

	buffer_t sfx = FloatSuffix(kernel->precision);

	// Base type
	bufcatcstr(str, "#define fptype ");
	BUFCAT_BUFFER_VALUE(str, RegBaseType(kernel->precision, 1));
	bufcatcstr(str, "\n\n");

	// Vector type
	bufcatcstr(str, "#define fvect2 ");
	BUFCAT_BUFFER_VALUE(str, RegBaseType(kernel->precision, 2));
	bufcatcstr(str, "\n\n");

	bool cReg = kernel->linearRegs ? true : false;

	// Generate butterflies for all unique radices.
	array_size_t uradices;
	// Initialize dynamic array storage explicitly.
	array_init(&uradices);
	const size_t *radices_end = kernel->radices.buf ? kernel->radices.buf + kernel->radices.len : NULL;
	for (const size_t *r = kernel->radices.buf; r != radices_end; r++)
		array_push_back(&uradices, *r);
	array_sort_unique_size_t(&uradices);

	// constants
	if (kernel->length % 8 == 0)
	{
		bufcatcstr(str, "#define C8Q  0.70710678118654752440084436210485");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
	}

	if (kernel->length % 5 == 0)
	{
		bufcatcstr(str, "#define C5QA 0.30901699437494742410229341718282");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C5QB 0.95105651629515357211643933337938");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C5QC 0.50000000000000000000000000000000");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C5QD 0.58778525229247312916870595463907");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C5QE 0.80901699437494742410229341718282");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
	}

	if (kernel->length % 3 == 0)
	{
		bufcatcstr(str, "#define C3QA 0.50000000000000000000000000000000");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C3QB 0.86602540378443864676372317075294");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
	}

	if (kernel->length % 7 == 0)
	{
		bufcatcstr(str, "#define C7Q1 -1.16666666666666651863693004997913");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q2  0.79015646852540022404554065360571");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q3  0.05585426728964774240049351305970");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q4  0.73430220123575240531721419756650");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q5  0.44095855184409837868031445395900");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q6  0.34087293062393136944265847887436");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q7 -0.53396936033772524066165487965918");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define C7Q8  0.87484229096165666561546458979137");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
	}

	if (kernel->length % 11 == 0)
	{
		bufcatcstr(str, "#define b11_0 0.9898214418809327");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_1 0.9594929736144973");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_2 0.9189859472289947");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_3 0.8767688310025893");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_4 0.8308300260037728");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_5 0.7784344533346518");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_6 0.7153703234534297");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_7 0.6343562706824244");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_8 0.3425847256816375");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b11_9 0.5211085581132027");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
	}

	if (kernel->length % 13 == 0)
	{
		bufcatcstr(str, "#define b13_0  0.9682872443619840");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_1  0.9578059925946651");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_2  0.8755023024091479");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_3  0.8660254037844386");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_4  0.8595425350987748");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_5  0.8534800018598239");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_6  0.7693388175729806");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_7  0.6865583707817543");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_8  0.6122646503767565");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_9  0.6004772719326652");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_10 0.5817047785105157");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_11 0.5751407294740031");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_12 0.5220263851612750");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_13 0.5200285718888646");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_14 0.5165207806234897");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_15 0.5149187780863157");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_16 0.5035370328637666");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_17 0.5000000000000000");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_18 0.3027756377319946");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_19 0.3014792600477098");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_20 0.3004626062886657");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_21 0.2517685164318833");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_22 0.2261094450357824");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_23 0.0833333333333333");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
		bufcatcstr(str, "#define b13_24 0.0386329546443481");
		bufcatbuf(str, &sfx);
		bufcatcstr(str, "\n");
	}

	bufcatcstr(str, "\n");

	// If pre-callback is set for the plan
	buffer_t callbackstr = buffer_empty();
	if (kernel->params.fft_hasPreCallback)
	{
		// Insert pre-callback function code at the beginning
		bufcatcstr(&callbackstr, kernel->params.fft_preCallback.funcstring);
		bufcatcstr(&callbackstr, "\n\n");

		bufcatbuf(str, &callbackstr);
	}

	// If post-callback is set for the plan
	if (kernel->params.fft_hasPostCallback)
	{
		// Insert post-callback function code
		bufcatcstr(str, kernel->params.fft_postCallback.funcstring);
		bufcatcstr(str, "\n\n");
	}

	// Walk the compact pass array with raw pointers.
	const Pass *p;
	const Pass *passes_begin = kernel->passes.buf;
	const Pass *passes_end = kernel->passes.buf ? kernel->passes.buf + kernel->passes.len : NULL;
	if (kernel->length > 1)
	{
		// Walk the compact radix array with raw pointers.
		const size_t *uradices_end = uradices.buf ? uradices.buf + uradices.len : NULL;
		for (const size_t *r = uradices.buf; r != uradices_end; r++)
		{
			size_t rad = *r;
			p = passes_begin;
			while (p->radix != rad)
				p++;

			for (size_t d = 0; d < 2; d++)
			{
				bool fwd = d ? false : true;

				if (p->numB1)
				{
					// Initialize and emit the radix butterfly for one register
					// group.
					Butterfly bfly;
					ButterflyInit(&bfly, kernel->precision, rad, 1, fwd, cReg);
					ButterflyGenerateButterfly(&bfly, str);
					bufcatcstr(str, "\n");
				}
				if (p->numB2)
				{
					// Initialize and emit the radix butterfly for two register
					// groups.
					Butterfly bfly;
					ButterflyInit(&bfly, kernel->precision, rad, 2, fwd, cReg);
					ButterflyGenerateButterfly(&bfly, str);
					bufcatcstr(str, "\n");
				}
				if (p->numB4)
				{
					// Initialize and emit the radix butterfly for four register
					// groups.
					Butterfly bfly;
					ButterflyInit(&bfly, kernel->precision, rad, 4, fwd, cReg);
					ButterflyGenerateButterfly(&bfly, str);
					bufcatcstr(str, "\n");
				}
			}
		}
	}

	// Generate passes
	for (size_t d = 0; d < 2; d++)
	{
		bool fwd;

		if (kernel->r2c2r)
			fwd = kernel->r2c;
		else
			fwd = d ? false : true;

		double scale = fwd ? kernel->params.fft_fwdScale : kernel->params.fft_backScale;

		for (p = passes_begin; p != passes_end; p++)
		{
			double s = 1.0;
			size_t ins = 1, outs = 1;
			bool gIn = false, gOut = false;
			bool inIlvd = false, outIlvd = false;
			bool inRl = false, outRl = false;
			bool tw3Step = false;

			if (p == passes_begin && kernel->params.fft_twiddleFront)
				tw3Step = kernel->params.fft_3StepTwiddle;
			if ((p + 1) == passes_end)
			{
				s = scale;
				if (!kernel->params.fft_twiddleFront)
					tw3Step = kernel->params.fft_3StepTwiddle;
			}

			if (kernel->blockCompute && !kernel->r2c2r)
			{
				inIlvd = ldsInterleaved;
				outIlvd = ldsInterleaved;
			}
			else
			{
				if (p == passes_begin)
				{
					inIlvd = inInterleaved;
					inRl = inReal;
					gIn = true;
					ins = kernel->params.fft_inStride[0];
				}
				if ((p + 1) == passes_end)
				{
					outIlvd = outInterleaved;
					outRl = outReal;
					gOut = true;
					outs = kernel->params.fft_outStride[0];
				}

				if (p != passes_begin)
					inIlvd = ldsInterleaved;
				if ((p + 1) != passes_end)
					outIlvd = ldsInterleaved;
			}

			PassGeneratePass(p, fwd, str, tw3Step, kernel->params.fft_twiddleFront, inIlvd, outIlvd, inRl, outRl, ins, outs, s, gIn, gOut);
		}

		// if real transform we do only 1 direction
		if (kernel->r2c2r)
			break;
	}

	// TODO : address this kludge
	bufcatcstr(str, " typedef union  { uint u; int i; } cb_t;\n\n");

	for (size_t d = 0; d < 2; d++)
	{
		bool fwd;

		if (kernel->r2c2r)
			fwd = inReal ? true : false;
		else
			fwd = d ? false : true;

		// FFT kernel begin
		// Function attribute
		bufcatcstr(str, "__kernel __attribute__((reqd_work_group_size (");
		if (kernel->blockCompute)
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS));
		else
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->workGroupSize));
		bufcatcstr(str, ",1,1)))\nvoid ");

		// Function name
		if (fwd)
			bufcatcstr(str, "fft_fwd");
		else
			bufcatcstr(str, "fft_back");
		bufcatcstr(str, "(");

		// TODO : address this kludge
		size_t SizeParam_ret = 0;
		clGetDeviceInfo(Dev_ID, CL_DEVICE_VENDOR, 0, NULL, &SizeParam_ret);
		char *nameVendor = (char *) clfft_checked_malloc(SizeParam_ret);
		clGetDeviceInfo(Dev_ID, CL_DEVICE_VENDOR, SizeParam_ret, nameVendor, NULL);

		// nv compiler doesn't support __constant kernel argument
		if (strncmp(nameVendor, "NVIDIA", 6) != 0)
			bufcatcstr(str, "__constant cb_t *cb __attribute__((max_constant_size(32))), ");
		else
			bufcatcstr(str, "__global cb_t *cb, ");

		free(nameVendor);

		// If plan has pre/post callback
		bufclear(&callbackstr);
		bool hasCallback = kernel->params.fft_hasPreCallback || kernel->params.fft_hasPostCallback;

		if (hasCallback)
		{
			if (kernel->params.fft_hasPreCallback)
				bufcatcstr(&callbackstr, ", __global void* pre_userdata");
			if (kernel->params.fft_hasPostCallback)
				bufcatcstr(&callbackstr, ", __global void* post_userdata");

			if (kernel->params.fft_preCallback.localMemSize > 0 || kernel->params.fft_postCallback.localMemSize > 0)
			{
				bufcatcstr(&callbackstr, ", __local void* localmem");
			}
		}

		// Function attributes
		if (kernel->params.fft_placeness == CLFFT_INPLACE)
		{
			if (kernel->r2c2r)
			{
				if (outInterleaved)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " * restrict gb");
				}
				else
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " * restrict gb");
				}

				// If plan has callback
				if (hasCallback)
					bufcatbuf(str, &callbackstr);

				bufcatcstr(str, ")\n");
			}
			else
			{
				assert(inInterleaved == outInterleaved);
				assert(kernel->params.fft_inStride[1] == kernel->params.fft_outStride[1]);
				assert(kernel->params.fft_inStride[0] == kernel->params.fft_outStride[0]);

				if (inInterleaved)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " * restrict gb");

					// If plan has callback
					if (hasCallback)
						bufcatbuf(str, &callbackstr);

					bufcatcstr(str, ")\n");
				}
				else
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " * restrict gbRe, ");
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " * restrict gbIm");

					// If plan has callback
					if (hasCallback)
						bufcatbuf(str, &callbackstr);

					bufcatcstr(str, ")\n");
				}
			}
		}
		else if (kernel->r2c2r)
		{
			if (inInterleaved)
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &r2Type);
				bufcatcstr(str, " * restrict gbIn, ");
			}
			else if (inReal)
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbIn, ");
			}
			else
			{
				bufcatcstr(str, "__global const ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbInRe, ");
				bufcatcstr(str, "__global const ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbInIm, ");
			}

			if (outInterleaved)
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &r2Type);
				bufcatcstr(str, " * restrict gbOut");
			}
			else if (outReal)
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbOut");
			}
			else
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbOutRe, ");
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbOutIm");
			}

			// If plan has callback
			if (hasCallback)
				bufcatbuf(str, &callbackstr);

			bufcatcstr(str, ")\n");
		}
		else
		{
			if (inInterleaved)
			{
				bufcatcstr(str, "__global const ");
				bufcatbuf(str, &r2Type);
				bufcatcstr(str, " * restrict gbIn, ");
			}
			else
			{
				bufcatcstr(str, "__global const ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbInRe, ");
				bufcatcstr(str, "__global const ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbInIm, ");
			}

			if (outInterleaved)
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &r2Type);
				bufcatcstr(str, " * restrict gbOut");
			}
			else
			{
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbOutRe, ");
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " * restrict gbOutIm");
			}

			// If plan has callback
			if (hasCallback)
				bufcatbuf(str, &callbackstr);

			bufcatcstr(str, ")\n");
		}

		bufcatcstr(str, "{\n");

		// Initialize
		bufcatcstr(str, "\t");
		bufcatcstr(str, "uint me = get_local_id(0);\n\t");
		bufcatcstr(str, "uint batch = get_group_id(0);");
		bufcatcstr(str, "\n");

		// Allocate LDS
		if (kernel->blockCompute)
		{
			bufcatcstr(str, "\n\t");
			bufcatcstr(str, "__local ");
			bufcatbuf(str, &r2Type);
			bufcatcstr(str, " lds[");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockLDS));
			bufcatcstr(str, "];\n");
		}
		else
		{
			size_t ldsSize = kernel->halfLds ? kernel->length * kernel->numTrans : 2 * kernel->length * kernel->numTrans;
			ldsSize = ldsInterleaved ? ldsSize / 2 : ldsSize;

			if (kernel->numPasses > 1)
			{
				bufcatcstr(str, "\n\t");
				bufcatcstr(str, "__local ");
				bufcatbuf(str, ldsInterleaved ? &r2Type : &rType);
				bufcatcstr(str, " lds[");
				BUFCAT_BUFFER_VALUE(str, SztToStr(ldsSize));
				bufcatcstr(str, "];\n");
			}
		}

		// Declare memory pointers
		bufcatcstr(str, "\n\t");
		if (kernel->r2c2r)
		{
			bufcatcstr(str, "uint iOffset;\n\t");
			bufcatcstr(str, "uint oOffset;\n\n\t");
			if (!kernel->rcSimple)
			{
				bufcatcstr(str, "uint iOffset2;\n\t");
				bufcatcstr(str, "uint oOffset2;\n\n\t");
			}

			if (!kernel->params.fft_hasPreCallback)
			{
				if (inInterleaved)
				{
					if (!kernel->rcSimple)
					{
						bufcatcstr(str, "__global ");
						bufcatbuf(str, &r2Type);
						bufcatcstr(str, " *lwbIn2;\n\t");
					}
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " *lwbIn;\n\t");
				}
				else if (inReal)
				{
					if (!kernel->rcSimple)
					{
						bufcatcstr(str, "__global ");
						bufcatbuf(str, &rType);
						bufcatcstr(str, " *lwbIn2;\n\t");
					}
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbIn;\n\t");
				}
				else
				{
					if (!kernel->rcSimple)
					{
						bufcatcstr(str, "__global ");
						bufcatbuf(str, &rType);
						bufcatcstr(str, " *lwbInRe2;\n\t");
					}
					if (!kernel->rcSimple)
					{
						bufcatcstr(str, "__global ");
						bufcatbuf(str, &rType);
						bufcatcstr(str, " *lwbInIm2;\n\t");
					}

					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbInRe;\n\t");
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbInIm;\n\t");
				}
			}

			if (outInterleaved)
			{
				if (!kernel->params.fft_hasPostCallback)
				{
					if (!kernel->rcSimple)
					{
						bufcatcstr(str, "__global ");
						bufcatbuf(str, &r2Type);
						bufcatcstr(str, " *lwbOut2;\n\t");
					}
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " *lwbOut;\n");
				}
			}
			else if (outReal)
			{
				if (!kernel->params.fft_hasPostCallback)
				{
					if (!kernel->rcSimple)
					{
						bufcatcstr(str, "__global ");
						bufcatbuf(str, &rType);
						bufcatcstr(str, " *lwbOut2;\n\t");
					}
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbOut;\n");
				}
			}
			else if (!kernel->params.fft_hasPostCallback)
			{
				if (!kernel->rcSimple)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbOutRe2;\n\t");
				}
				if (!kernel->rcSimple)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbOutIm2;\n\t");
				}
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " *lwbOutRe;\n\t");
				bufcatcstr(str, "__global ");
				bufcatbuf(str, &rType);
				bufcatcstr(str, " *lwbOutIm;\n");
			}
			bufcatcstr(str, "\n");
		}
		else if (kernel->params.fft_placeness == CLFFT_INPLACE)
		{
			bufcatcstr(str, "uint ioOffset;\n\t");

			// Skip if callback is set
			if (!kernel->params.fft_hasPreCallback || !kernel->params.fft_hasPostCallback)
			{
				if (inInterleaved)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " *lwb;\n");
				}
				else
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbRe;\n\t");
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbIm;\n");
				}
			}
			bufcatcstr(str, "\n");
		}
		else
		{
			bufcatcstr(str, "uint iOffset;\n\t");
			bufcatcstr(str, "uint oOffset;\n\t");

			// Skip if precallback is set
			if (!(kernel->params.fft_hasPreCallback))
			{
				if (inInterleaved)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " *lwbIn;\n\t");
				}
				else
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbInRe;\n\t");
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbInIm;\n\t");
				}
			}

			// Skip if postcallback is set
			if (!kernel->params.fft_hasPostCallback)
			{
				if (outInterleaved)
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &r2Type);
					bufcatcstr(str, " *lwbOut;\n");
				}
				else
				{
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbOutRe;\n\t");
					bufcatcstr(str, "__global ");
					bufcatbuf(str, &rType);
					bufcatcstr(str, " *lwbOutIm;\n");
				}
			}
			bufcatcstr(str, "\n");
		}

		// Setup registers if needed
		if (kernel->linearRegs)
		{
			bufcatcstr(str, "\t");
			BUFCAT_BUFFER_VALUE(str, RegBaseType(kernel->precision, 2));
			bufcatcstr(str, " ");
			BUFCAT_BUFFER_VALUE(str, KernelIterRegs(kernel, "", false));
			bufcatcstr(str, ";\n\n");
		}

		// Calculate total transform count
		buffer_t totalBatch = buffer_from_cstr("(");
		size_t i = 0;
		while (i < (kernel->params.fft_DataDim - 2))
		{
			BUFCAT_BUFFER_VALUE(&totalBatch, SztToStr(kernel->params.fft_N[i + 1]));
			bufcatcstr(&totalBatch, " * ");
			i++;
		}
		bufcatcstr(&totalBatch, "cb[0].u)");

		// Conditional read-write ('rw') for arbitrary batch number
		if (kernel->r2c2r && !kernel->rcSimple)
		{
			bufcatcstr(str, "\tuint this = ");
			bufcatbuf(str, &totalBatch);
			bufcatcstr(str, " - batch*");
			BUFCAT_BUFFER_VALUE(str, SztToStr(2 * kernel->numTrans));
			bufcatcstr(str, ";\n");
			bufcatcstr(str, "\tuint rw = (me < ((this+1)/2)*");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->workGroupSizePerTrans));
			bufcatcstr(str, ") ? (this - 2*(me/");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->workGroupSizePerTrans));
			bufcatcstr(str, ")) : 0;\n\n");
		}
		else if ((kernel->numTrans > 1) && !kernel->blockCompute)
		{
			bufcatcstr(str, "\tuint rw = (me < (");
			bufcatbuf(str, &totalBatch);
			bufcatcstr(str, " - batch*");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->numTrans));
			bufcatcstr(str, ")*");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->workGroupSizePerTrans));
			bufcatcstr(str, ") ? 1 : 0;\n\n");
		}
		else
		{
			bufcatcstr(str, "\tuint rw = 1;\n\n");
		}

		// Transform index for 3-step twiddles
		if (kernel->params.fft_3StepTwiddle && !kernel->blockCompute)
		{
			if (kernel->numTrans == 1)
			{
				bufcatcstr(str, "\tuint b = batch%");
			}
			else
			{
				bufcatcstr(str, "\tuint b = (batch*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->numTrans));
				bufcatcstr(str, " + (me/");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->workGroupSizePerTrans));
				bufcatcstr(str, "))%");
			}

			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_N[1]));
			bufcatcstr(str, ";\n\n");

			if (kernel->params.fft_realSpecial)
				bufcatcstr(str, "\tuint bt = b;\n\n");
		}
		else
		{
			bufcatcstr(str, "\tuint b = 0;\n\n");
		}

		// Setup memory pointers
		if (kernel->r2c2r)
		{
			BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "iOffset", true, false));
			BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "oOffset", false, false));
			if (!kernel->rcSimple)
			{
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "iOffset2", true, true));
			}
			if (!kernel->rcSimple)
			{
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "oOffset2", false, true));
			}

			bufcatcstr(str, "\n\t");
			if (kernel->params.fft_placeness == CLFFT_INPLACE)
			{
				if (!kernel->params.fft_hasPreCallback)
				{
					if (inInterleaved)
					{
						if (!kernel->rcSimple)
						{
							bufcatcstr(str, "lwbIn2 = (__global ");
							bufcatbuf(str, &r2Type);
							bufcatcstr(str, " *)gb + iOffset2;\n\t");
						}
						bufcatcstr(str, "lwbIn  = (__global ");
						bufcatbuf(str, &r2Type);
						bufcatcstr(str, " *)gb + iOffset;\n\t");
					}
					else
					{
						if (!kernel->rcSimple)
						{
							bufcatcstr(str, "lwbIn2 = (__global ");
							bufcatbuf(str, &rType);
							bufcatcstr(str, " *)gb + iOffset2;\n\t");
						}
						bufcatcstr(str, "lwbIn  = (__global ");
						bufcatbuf(str, &rType);
						bufcatcstr(str, " *)gb + iOffset;\n\t");
					}
				}

				if (!kernel->params.fft_hasPostCallback)
				{
					if (!kernel->rcSimple)
						bufcatcstr(str, "lwbOut2 = gb + oOffset2;\n\t");
					bufcatcstr(str, "lwbOut = gb + oOffset;\n");
				}
				bufcatcstr(str, "\n");
			}
			else
			{
				if (!kernel->params.fft_hasPreCallback)
				{
					if (inInterleaved || inReal)
					{
						if (!kernel->rcSimple)
							bufcatcstr(str, "lwbIn2 = gbIn + iOffset2;\n\t");
						bufcatcstr(str, "lwbIn = gbIn + iOffset;\n\t");
					}
					else
					{
						if (!kernel->rcSimple)
						{
							bufcatcstr(str, "lwbInRe2 = gbInRe + iOffset2;\n\t");
						}
						if (!kernel->rcSimple)
						{
							bufcatcstr(str, "lwbInIm2 = gbInIm + iOffset2;\n\t");
						}
						bufcatcstr(str, "lwbInRe = gbInRe + iOffset;\n\t");
						bufcatcstr(str, "lwbInIm = gbInIm + iOffset;\n\t");
					}
				}

				if (!kernel->params.fft_hasPostCallback)
				{
					if (outInterleaved || outReal)
					{
						if (!kernel->rcSimple)
							bufcatcstr(str, "lwbOut2 = gbOut + oOffset2;\n\t");
						bufcatcstr(str, "lwbOut = gbOut + oOffset;\n");
					}
					else
					{
						if (!kernel->rcSimple)
						{
							bufcatcstr(str, "lwbOutRe2 = gbOutRe + oOffset2;\n\t");
						}
						if (!kernel->rcSimple)
						{
							bufcatcstr(str, "lwbOutIm2 = gbOutIm + oOffset2;\n\t");
						}
						bufcatcstr(str, "lwbOutRe = gbOutRe + oOffset;\n\t");
						bufcatcstr(str, "lwbOutIm = gbOutIm + oOffset;\n");
					}
				}
				bufcatcstr(str, "\n");
			}
		}
		else if (kernel->params.fft_placeness == CLFFT_INPLACE)
		{
			if (kernel->blockCompute)
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalcBlock(kernel, "ioOffset", true));
			else
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "ioOffset", true, false));

			bufcatcstr(str, "\t");

			// Skip if callback is set
			if (!kernel->params.fft_hasPreCallback || !kernel->params.fft_hasPostCallback)
			{
				if (inInterleaved)
				{
					bufcatcstr(str, "lwb = gb + ioOffset;\n");
				}
				else
				{
					bufcatcstr(str, "lwbRe = gbRe + ioOffset;\n\t");
					bufcatcstr(str, "lwbIm = gbIm + ioOffset;\n");
				}
			}
			bufcatcstr(str, "\n");
		}
		else
		{
			if (kernel->blockCompute)
			{
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalcBlock(kernel, "iOffset", true));
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalcBlock(kernel, "oOffset", false));
			}
			else
			{
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "iOffset", true, false));
				BUFCAT_BUFFER_VALUE(str, KernelOffsetCalc(kernel, "oOffset", false, false));
			}

			bufcatcstr(str, "\t");

			// Skip if precallback is set
			if (!(kernel->params.fft_hasPreCallback))
			{
				if (inInterleaved)
				{
					bufcatcstr(str, "lwbIn = gbIn + iOffset;\n\t");
				}
				else
				{
					bufcatcstr(str, "lwbInRe = gbInRe + iOffset;\n\t");
					bufcatcstr(str, "lwbInIm = gbInIm + iOffset;\n\t");
				}
			}

			// Skip if postcallback is set
			if (!kernel->params.fft_hasPostCallback)
			{
				if (outInterleaved)
				{
					bufcatcstr(str, "lwbOut = gbOut + oOffset;\n");
				}
				else
				{
					bufcatcstr(str, "lwbOutRe = gbOutRe + oOffset;\n\t");
					bufcatcstr(str, "lwbOutIm = gbOutIm + oOffset;\n");
				}
			}
			bufcatcstr(str, "\n");
		}

		buffer_t inOffset = buffer_empty();
		buffer_t outOffset = buffer_empty();
		if (kernel->params.fft_placeness == CLFFT_INPLACE && !kernel->r2c2r)
		{
			bufcatcstr(&inOffset, "ioOffset");
			bufcatcstr(&outOffset, "ioOffset");
		}
		else
		{
			bufcatcstr(&inOffset, "iOffset");
			bufcatcstr(&outOffset, "oOffset");
		}

		// Read data into LDS for blocked access
		if (kernel->blockCompute)
		{
			size_t loopCount = (kernel->length * kernel->blockWidth) / kernel->blockWGS;

			if ((kernel->blockComputeType == BCT_C2C) && kernel->params.fft_hasPreCallback)
			{
				bufcatcstr(str, "\n\t");
				bufcatbuf(str, &r2Type);
				bufcatcstr(str, " retCallback;");
			}

			bufcatcstr(str, "\n\tfor(uint t=0; t<");
			BUFCAT_BUFFER_VALUE(str, SztToStr(loopCount));
			bufcatcstr(str, "; t++)\n\t{\n");

			// get offset
			buffer_t bufOffset = buffer_empty();

			for (size_t c = 0; c < 2; c++)
			{
				buffer_t comp = buffer_from_cstr("");
				buffer_t readBuf = buffer_from_cstr((kernel->params.fft_placeness == CLFFT_INPLACE) ? "lwb" : "lwbIn");
				if (!inInterleaved)
					bufsetcstr(&comp, c ? ".y" : ".x");
				if (!inInterleaved)
					bufsetcstr(&readBuf, (kernel->params.fft_placeness == CLFFT_INPLACE) ? (c ? "lwbIm" : "lwbRe") : (c ? "lwbInIm" : "lwbInRe"));

				if ((kernel->blockComputeType == BCT_C2C) || (kernel->blockComputeType == BCT_C2R))
				{
					bufclear(&bufOffset);
					bufcatcstr(&bufOffset, "(me%");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(kernel->blockWidth));
					bufcatcstr(&bufOffset, ") + ");
					bufcatcstr(&bufOffset, "(me/");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(kernel->blockWidth));
					bufcatcstr(&bufOffset, ")*");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(kernel->params.fft_inStride[0]));
					bufcatcstr(&bufOffset, " + t*");
					BUFCAT_BUFFER_VALUE(&bufOffset, SztToStr(kernel->params.fft_inStride[0] * kernel->blockWGS / kernel->blockWidth));

					if ((kernel->blockComputeType == BCT_C2C) && kernel->params.fft_hasPreCallback)
					{
						if (c == 0)
						{
							bufcatcstr(str, "\t\tretCallback = ");
							bufcatcstr(str, kernel->params.fft_preCallback.funcname);
							bufcatcstr(str, "(");

							if (inInterleaved)
							{
								bufcatcstr(str, (kernel->params.fft_placeness == CLFFT_INPLACE) ? "gb, " : "gbIn, ");
							}
							else
							{
								bufcatcstr(str, (kernel->params.fft_placeness == CLFFT_INPLACE) ? "gbRe, gbIm, " : "gbInRe, gbInIm, ");
							}

							bufcatbuf(str, &inOffset);
							bufcatcstr(str, " + ");
							bufcatbuf(str, &bufOffset);
							bufcatcstr(str, ", pre_userdata");
							if (kernel->params.fft_preCallback.localMemSize > 0)
								bufcatcstr(str, ", localmem);\n");
							else
								bufcatcstr(str, ");\n");
						}

						bufcatcstr(str, "\t\tR0");
						bufcatbuf(str, &comp);
						bufcatcstr(str, " = retCallback");
						bufcatbuf(str, &comp);
						bufcatcstr(str, ";\n");
					}
					else
					{
						bufcatcstr(str, "\t\tR0");
						bufcatbuf(str, &comp);
						bufcatcstr(str, " = ");
						bufcatbuf(str, &readBuf);
						bufcatcstr(str, "[");
						bufcatbuf(str, &bufOffset);
						bufcatcstr(str, "];\n");
					}
				}
				else
				{
					bufcatcstr(str, "\t\tR0");
					bufcatbuf(str, &comp);
					bufcatcstr(str, " = ");
					bufcatbuf(str, &readBuf);
					bufcatcstr(str, "[me + t*");
					BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS));
					bufcatcstr(str, "];\n");
				}

				if (inInterleaved)
					break;
			}

			if ((kernel->blockComputeType == BCT_C2C) || (kernel->blockComputeType == BCT_C2R))
			{
				bufcatcstr(str, "\t\tlds[t*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS / kernel->blockWidth));
				bufcatcstr(str, " + ");
				bufcatcstr(str, "(me%");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
				bufcatcstr(str, ")*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->length));
				bufcatcstr(str, " + ");
				bufcatcstr(str, "(me/");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
				bufcatcstr(str, ")] = R0;");
				bufcatcstr(str, "\n");
			}
			else
			{
				bufcatcstr(str, "\t\tlds[t*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS));
				bufcatcstr(str, " + me] = R0;");
				bufcatcstr(str, "\n");
			}

			bufcatcstr(str, "\t}\n\n");
			bufcatcstr(str, "\tbarrier(CLK_LOCAL_MEM_FENCE);\n\n");
		}

		// Set rw and 'me' per transform
		// rw string also contains 'b'
		buffer_t rw = buffer_empty();
		buffer_t me = buffer_empty();

		if (kernel->r2c2r && !kernel->rcSimple)
			bufsetcstr(&rw, "rw, b, ");
		else
			bufsetcstr(&rw, ((kernel->numTrans > 1) || kernel->realSpecial) ? "rw, b, " : "1, b, ");

		if (kernel->numTrans > 1)
		{
			bufcatcstr(&me, "me%");
			BUFCAT_BUFFER_VALUE(&me, SztToStr(kernel->workGroupSizePerTrans));
			bufcatcstr(&me, ", ");
		}
		else
		{
			bufcatcstr(&me, "me, ");
		}

		if (kernel->blockCompute)
		{
			bufsetcstr(&me, "me%");
			BUFCAT_BUFFER_VALUE(&me, SztToStr(kernel->workGroupSizePerTrans));
			bufcatcstr(&me, ", ");
		}

		// Buffer strings
		buffer_t inBuf = buffer_empty();
		buffer_t outBuf = buffer_empty();
		if (kernel->r2c2r)
		{
			if (kernel->rcSimple)
			{
				if (inInterleaved || inReal)
					bufsetcstr(&inBuf, kernel->params.fft_hasPreCallback ? "gbIn, " : "lwbIn, ");
				else
					bufsetcstr(&inBuf, "lwbInRe, lwbInIm, ");
				if (outInterleaved || outReal)
					bufsetcstr(&outBuf, kernel->params.fft_hasPostCallback ? "gbOut" : "lwbOut");
				else
					bufsetcstr(&outBuf, "lwbOutRe, lwbOutIm");
			}
			else
			{
				if (inInterleaved || inReal)
				{
					if (!kernel->params.fft_hasPreCallback)
					{
						bufsetcstr(&inBuf, "lwbIn, lwbIn2, ");
					}
					else if (kernel->params.fft_placeness == CLFFT_INPLACE)
					{
						bufsetcstr(&inBuf, "(__global ");
						bufcatbuf(&inBuf, kernel->r2c ? &rType : &r2Type);
						bufcatcstr(&inBuf, "*) gb, ");
						bufcatcstr(&inBuf, "(__global ");
						bufcatbuf(&inBuf, kernel->r2c ? &rType : &r2Type);
						bufcatcstr(&inBuf, "*) gb, ");
					}
					else
					{
						bufsetcstr(&inBuf, "gbIn, gbIn, ");
					}
				}
				else
					bufsetcstr(&inBuf, (kernel->params.fft_hasPreCallback) ? "gbInRe, gbInRe, gbInIm, gbInIm, " : "lwbInRe, lwbInRe2, lwbInIm, lwbInIm2, ");

				if (outInterleaved || outReal)
					bufsetcstr(&outBuf,
						kernel->params.fft_hasPostCallback ? ((kernel->params.fft_placeness == CLFFT_INPLACE) ? "gb, gb" : "gbOut, gbOut")
										   : "lwbOut, lwbOut2");
				else
					bufsetcstr(&outBuf, kernel->params.fft_hasPostCallback ? "gbOutRe, gbOutRe, gbOutIm, gbOutIm" : "lwbOutRe, lwbOutRe2, lwbOutIm, lwbOutIm2");
			}
		}
		else if (kernel->params.fft_placeness == CLFFT_INPLACE)
		{
			if (inInterleaved)
			{
				bufsetcstr(&inBuf, kernel->params.fft_hasPreCallback ? "gb, " : "lwb, ");
				bufsetcstr(&outBuf, kernel->params.fft_hasPostCallback ? "gb" : "lwb");
			}
			else
			{
				bufsetcstr(&inBuf, kernel->params.fft_hasPreCallback ? "gbRe, gbIm, " : "lwbRe, lwbIm, ");
				bufsetcstr(&outBuf, kernel->params.fft_hasPostCallback ? "gbRe, gbIm" : "lwbRe, lwbIm");
			}
		}
		else
		{
			if (inInterleaved)
				bufsetcstr(&inBuf, kernel->params.fft_hasPreCallback ? "gbIn, " : "lwbIn, ");
			else
				bufsetcstr(&inBuf, kernel->params.fft_hasPreCallback ? "gbInRe, gbInIm, " : "lwbInRe, lwbInIm, ");
			if (outInterleaved)
				bufsetcstr(&outBuf, kernel->params.fft_hasPostCallback ? "gbOut" : "lwbOut");
			else
				bufsetcstr(&outBuf, kernel->params.fft_hasPostCallback ? "gbOutRe, gbOutIm" : "lwbOutRe, lwbOutIm");
		}

		if (kernel->blockCompute)
		{
			bufcatcstr(str, "\n\tfor(uint t=0; t<");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth / (kernel->blockWGS / kernel->workGroupSizePerTrans)));
			bufcatcstr(str, "; t++)\n\t{\n\n");

			bufsetcstr(&inBuf, "lds, ");
			bufsetcstr(&outBuf, "lds");

			if (kernel->params.fft_3StepTwiddle)
			{
				bufcatcstr(str, "\t\tb = (batch%");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_N[1] / kernel->blockWidth));
				bufcatcstr(str, ")*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
				bufcatcstr(str, " + t*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS / kernel->workGroupSizePerTrans));
				bufcatcstr(str, " + (me/");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->workGroupSizePerTrans));
				bufcatcstr(str, ");\n\n");
			}
		}

		if (kernel->realSpecial)
			bufcatcstr(str, "\n\tfor(uint t=0; t<2; t++)\n\t{\n\n");

		// Call passes
		if (kernel->numPasses == 1)
		{
			bufcatcstr(str, "\t");
			BUFCAT_BUFFER_VALUE(str, PassName(0, fwd));
			bufcatcstr(str, "(");
			bufcatbuf(str, &rw);
			bufcatbuf(str, &me);

			// Append the pre-callback input offset when it is present.

			if (kernel->params.fft_hasPreCallback)
				bufcatbuf(str, &inOffset);

			else
				bufcatcstr(str, "0");

			if (kernel->params.fft_hasPostCallback)
			{
				bufcatcstr(str, ", ");
				bufcatbuf(str, &outOffset);
				bufcatcstr(str, ", ");
			}
			else
			{
				bufcatcstr(str, ", 0, ");
			}

			bufcatbuf(str, &inBuf);
			bufcatbuf(str, &outBuf);
			BUFCAT_BUFFER_VALUE(str, KernelIterRegs(kernel, "&", true));

			// If callback is set
			if (hasCallback)
			{
				// if pre-calback set
				if (kernel->params.fft_hasPreCallback)
				{
					bufcatcstr(str, (kernel->r2c2r && !kernel->rcSimple) ? ", iOffset2, pre_userdata" : ", pre_userdata");
				}

				// if post-calback set
				if (kernel->params.fft_hasPostCallback)
				{
					if ((kernel->r2c || kernel->c2r) && !kernel->rcSimple)
					{
						bufcatcstr(str, ", ");
						bufcatbuf(str, &outOffset);
						bufcatcstr(str, "2");
					}

					bufcatcstr(str, ", post_userdata");
				}

				if (kernel->params.fft_preCallback.localMemSize > 0)
					bufcatcstr(str, ", localmem");
				if (kernel->params.fft_postCallback.localMemSize > 0)
				{
					// if precallback localmem also requested, send the localmem
					// with the right offset
					if (kernel->params.fft_hasPreCallback && kernel->params.fft_preCallback.localMemSize > 0)
					{
						bufcatcstr(str, ", ((__local char *)localmem + ");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_preCallback.localMemSize));
						bufcatcstr(str, ")");
					}
					else
					{
						bufcatcstr(str, ", localmem");
					}
				}
			}

			bufcatcstr(str, ");\n");
		}
		else
		{
			// Walk the pass array with raw pointers.
			const Pass *passes_begin = kernel->passes.buf;
			const Pass *passes_end = kernel->passes.buf ? kernel->passes.buf + kernel->passes.len : NULL;
			for (const Pass *p = passes_begin; p != passes_end; p++)
			{
				buffer_t exTab = buffer_from_cstr("");
				if (kernel->blockCompute || kernel->realSpecial)
					bufsetcstr(&exTab, "\t");

				bufcatbuf(str, &exTab);
				bufcatcstr(str, "\t");
				BUFCAT_BUFFER_VALUE(str, PassName(p->position, fwd));
				bufcatcstr(str, "(");

				buffer_t ldsOff = buffer_empty();
				if (kernel->blockCompute)
				{
					bufcatcstr(&ldsOff, "t*");
					BUFCAT_BUFFER_VALUE(&ldsOff, SztToStr(kernel->length * (kernel->blockWGS / kernel->workGroupSizePerTrans)));
					bufcatcstr(&ldsOff, " + (me/");
					BUFCAT_BUFFER_VALUE(&ldsOff, SztToStr(kernel->workGroupSizePerTrans));
					bufcatcstr(&ldsOff, ")*");
					BUFCAT_BUFFER_VALUE(&ldsOff, SztToStr(kernel->length));
				}
				else if (kernel->numTrans > 1)
				{
					bufcatcstr(&ldsOff, "(me/");
					BUFCAT_BUFFER_VALUE(&ldsOff, SztToStr(kernel->workGroupSizePerTrans));
					bufcatcstr(&ldsOff, ")*");
					BUFCAT_BUFFER_VALUE(&ldsOff, SztToStr(kernel->length));
				}
				else
				{
					bufcatcstr(&ldsOff, "0");
				}

				buffer_t ldsArgs = buffer_empty();
				if (kernel->halfLds)
				{
					bufcatcstr(&ldsArgs, "lds, lds");
				}
				else if (ldsInterleaved)
				{
					bufcatcstr(&ldsArgs, "lds");
				}
				else
				{
					bufcatcstr(&ldsArgs, "lds, lds + ");
					BUFCAT_BUFFER_VALUE(&ldsArgs, SztToStr(kernel->length * kernel->numTrans));
				}

				bufcatbuf(str, &rw);
				if (kernel->params.fft_realSpecial)
					bufcatcstr(str, "t, ");
				bufcatbuf(str, &me);
				if (p == passes_begin) // beginning pass
				{
					if (kernel->blockCompute)
					{
						bufcatbuf(str, &ldsOff);
					}
					else
					{
						// Append the pre-callback input offset when it is
						// present.
						if (kernel->params.fft_hasPreCallback)
							bufcatbuf(str, &inOffset);
						else
							bufcatcstr(str, "0");
					}
					bufcatcstr(str, ", ");
					bufcatbuf(str, &ldsOff);
					bufcatcstr(str, ", ");
					bufcatbuf(str, &inBuf);
					bufcatbuf(str, &ldsArgs);
					BUFCAT_BUFFER_VALUE(str, KernelIterRegs(kernel, "&", true));

					// if precalback set, append additional arguments
					if (!kernel->blockCompute && kernel->params.fft_hasPreCallback)
					{
						bufcatcstr(str, (kernel->r2c2r && !kernel->rcSimple) ? ", iOffset2, pre_userdata" : ", pre_userdata");

						if (kernel->params.fft_preCallback.localMemSize > 0)
							bufcatcstr(str, ", localmem");
					}

					bufcatcstr(str, ");\n");
					if (!kernel->halfLds)
					{
						bufcatbuf(str, &exTab);
						bufcatcstr(str, "\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
					}
				}
				else if ((p + 1) == passes_end) // ending pass
				{
					bufcatbuf(str, &ldsOff);
					bufcatcstr(str, ", ");
					if (kernel->blockCompute)
					{
						bufcatbuf(str, &ldsOff);
					}
					else
					{
						// Append the post-callback output offset when it is
						// present.
						if (kernel->params.fft_hasPostCallback)
							bufcatbuf(str, &outOffset);
						else
							bufcatcstr(str, "0");
					}
					bufcatcstr(str, ", ");
					bufcatbuf(str, &ldsArgs);
					bufcatcstr(str, ", ");
					bufcatbuf(str, &outBuf);

					BUFCAT_BUFFER_VALUE(str, KernelIterRegs(kernel, "&", true));

					if (!kernel->blockCompute && kernel->params.fft_hasPostCallback)
					{
						if ((kernel->c2r || kernel->r2c) && !kernel->rcSimple)
						{
							bufcatcstr(str, ", ");
							bufcatbuf(str, &outOffset);
							bufcatcstr(str, "2");
						}

						bufcatcstr(str, ", post_userdata");

						if (kernel->params.fft_postCallback.localMemSize > 0)
						{
							// if precallback localmem also requested, send the
							// localmem with the right offset
							if (kernel->params.fft_hasPreCallback && kernel->params.fft_preCallback.localMemSize > 0)
							{
								bufcatcstr(str, ", ((__local char *)localmem + ");
								BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_preCallback.localMemSize));
								bufcatcstr(str, ")");
							}
							else
							{
								bufcatcstr(str, ", localmem");
							}
						}
					}
					bufcatcstr(str, ");\n");

					if (!kernel->halfLds)
					{
						bufcatbuf(str, &exTab);
						bufcatcstr(str, "\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
					}
				}
				else // intermediate pass
				{
					bufcatbuf(str, &ldsOff);
					bufcatcstr(str, ", ");
					bufcatbuf(str, &ldsOff);
					bufcatcstr(str, ", ");
					bufcatbuf(str, &ldsArgs);
					bufcatcstr(str, ", ");
					bufcatbuf(str, &ldsArgs);
					BUFCAT_BUFFER_VALUE(str, KernelIterRegs(kernel, "&", true));
					bufcatcstr(str, ");\n");
					if (!kernel->halfLds)
					{
						bufcatbuf(str, &exTab);
						bufcatcstr(str, "\tbarrier(CLK_LOCAL_MEM_FENCE);\n");
					}
				}
			}
		}

		if (kernel->realSpecial)
		{
			size_t Nt = 1 + kernel->length / 2;
			bufcatcstr(str, "\n\t\tif( (bt == 0) || (2*bt == ");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_realSpecial_Nr));
			bufcatcstr(str, ") ) { rw = 0; }\n");

			bufcatcstr(str, "\t\tlwbOut += (");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_realSpecial_Nr));
			bufcatcstr(str, " - 2*bt)*");
			BUFCAT_BUFFER_VALUE(str, SztToStr(Nt));
			bufcatcstr(str, ";\n");
			bufcatcstr(str, "\t\tb = ");
			BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_realSpecial_Nr));
			bufcatcstr(str, " - b;\n\n");
		}

		if (kernel->blockCompute || kernel->realSpecial)
			bufcatcstr(str, "\n\t}\n\n");

		// Write data from LDS for blocked access
		if (kernel->blockCompute)
		{
			size_t loopCount = (kernel->length * kernel->blockWidth) / kernel->blockWGS;

			bufcatcstr(str, "\tbarrier(CLK_LOCAL_MEM_FENCE);\n\n");
			bufcatcstr(str, "\n\tfor(uint t=0; t<");
			BUFCAT_BUFFER_VALUE(str, SztToStr(loopCount));
			bufcatcstr(str, "; t++)\n\t{\n");

			if ((kernel->blockComputeType == BCT_C2C) || (kernel->blockComputeType == BCT_R2C))
			{
				bufcatcstr(str, "\t\tR0 = lds[t*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS / kernel->blockWidth));
				bufcatcstr(str, " + ");
				bufcatcstr(str, "(me%");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
				bufcatcstr(str, ")*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->length));
				bufcatcstr(str, " + ");
				bufcatcstr(str, "(me/");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
				bufcatcstr(str, ")];");
				bufcatcstr(str, "\n");
			}
			else
			{
				bufcatcstr(str, "\t\tR0 = lds[t*");
				BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS));
				bufcatcstr(str, " + me];");
				bufcatcstr(str, "\n");
			}

			for (size_t c = 0; c < 2; c++)
			{
				buffer_t comp = buffer_from_cstr("");
				buffer_t writeBuf = buffer_from_cstr((kernel->params.fft_placeness == CLFFT_INPLACE) ? "lwb" : "lwbOut");
				if (!outInterleaved)
					bufsetcstr(&comp, c ? ".y" : ".x");
				if (!outInterleaved)
					bufsetcstr(&writeBuf, (kernel->params.fft_placeness == CLFFT_INPLACE) ? (c ? "lwbIm" : "lwbRe") : (c ? "lwbOutIm" : "lwbOutRe"));

				if ((kernel->blockComputeType == BCT_C2C) || (kernel->blockComputeType == BCT_R2C))
				{
					if (kernel->blockComputeType == BCT_R2C && kernel->params.fft_hasPostCallback)
					{
						if (outInterleaved)
							bufsetcstr(&writeBuf, (kernel->params.fft_placeness == CLFFT_INPLACE) ? "gb" : "gbOut");
						else
							bufsetcstr(&writeBuf, (kernel->params.fft_placeness == CLFFT_INPLACE) ? "gbRe, gbIm" : "gbOutRe, gbOutIm");

						bufcatcstr(str, "\t\t");
						bufcatcstr(str, kernel->params.fft_postCallback.funcname);
						bufcatcstr(str, "(");
						bufcatbuf(str, &writeBuf);
						bufcatcstr(str, ", (");
						bufcatbuf(str, &outOffset);
						bufcatcstr(str, " + (me%");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
						bufcatcstr(str, ") + ");
						bufcatcstr(str, "(me/");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
						bufcatcstr(str, ")*");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_outStride[0]));
						bufcatcstr(str, " + t*");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_outStride[0] * kernel->blockWGS / kernel->blockWidth));
						bufcatcstr(str, "), post_userdata, R0");
						if (!outInterleaved)
							bufcatcstr(str, ".x, R0.y");

						if (kernel->params.fft_postCallback.localMemSize > 0)
						{
							if (kernel->params.fft_hasPreCallback && kernel->params.fft_preCallback.localMemSize > 0)
							{
								bufcatcstr(str, ", (char *)(localmem + ");
								BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_preCallback.localMemSize));
								bufcatcstr(str, ")");
							}
							else
							{
								bufcatcstr(str, ", localmem");
							}
						}
						bufcatcstr(str, ");\n");

						// in the planar case, break from for loop since both
						// real and imag components are handled together in
						// post-callback
						if (!outInterleaved)
							break;
					}
					else
					{
						bufcatcstr(str, "\t\t");
						bufcatbuf(str, &writeBuf);
						bufcatcstr(str, "[(me%");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
						bufcatcstr(str, ") + ");
						bufcatcstr(str, "(me/");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWidth));
						bufcatcstr(str, ")*");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_outStride[0]));
						bufcatcstr(str, " + t*");
						BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->params.fft_outStride[0] * kernel->blockWGS / kernel->blockWidth));
						bufcatcstr(str, "] = R0");
						bufcatbuf(str, &comp);
						bufcatcstr(str, ";\n");
					}
				}
				else
				{
					bufcatcstr(str, "\t\t");
					bufcatbuf(str, &writeBuf);
					bufcatcstr(str, "[me + t*");
					BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->blockWGS));
					bufcatcstr(str, "] = R0");
					bufcatbuf(str, &comp);
					bufcatcstr(str, ";\n");
				}

				if (outInterleaved)
					break;
			}

			bufcatcstr(str, "\t}\n\n");
		}

		bufcatcstr(str, "}\n\n");

		if (kernel->r2c2r)
			break;
	}
}

static clfftStatus FFTGeneratedStockhamActionInitParams(FFTGeneratedStockhamAction *action)
{
	//    Query the devices in this context for their local memory sizes
	//    How we generate a kernel depends on the *minimum* LDS size for all
	//    devices.
	//
	const FFTEnvelope *pEnvelope = NULL;
	OPENCL_V(FFTPlanGetEnvelope(action->base.plan, &pEnvelope), "GetEnvelope failed");
	BUG_CHECK(NULL != pEnvelope);

	// Remainder: params was properly cleared by its constructor
	//            clearing it again would destroy datasize and id!!
	action->signature.data.fft_precision = action->base.plan->precision;
	action->signature.data.fft_placeness = action->base.plan->placeness;
	action->signature.data.fft_inputLayout = action->base.plan->inputLayout;
	action->signature.data.fft_MaxWorkGroupSize = action->base.plan->envelope.limit_WorkGroupSize;

	ARG_CHECK(array_size(&action->base.plan->length) > 0);
	ARG_CHECK(array_size(&action->base.plan->inStride) > 0);
	ARG_CHECK(array_size(&action->base.plan->outStride) > 0);

	ARG_CHECK(array_size(&action->base.plan->inStride) == array_size(&action->base.plan->outStride))

	bool real_transform = ((action->base.plan->inputLayout == CLFFT_REAL) || (action->base.plan->outputLayout == CLFFT_REAL));

	if ((CLFFT_INPLACE == action->base.plan->placeness) && (!real_transform))
	{
		//    If this is an in-place transform the
		//    input and output layout, dimensions and strides
		//    *MUST* be the same.
		//
		ARG_CHECK(action->base.plan->inputLayout == action->base.plan->outputLayout)
		action->signature.data.fft_outputLayout = action->base.plan->inputLayout;
		for (size_t u = array_size(&action->base.plan->inStride); u-- > 0;)
		{
			ARG_CHECK(action->base.plan->inStride.buf[u] == action->base.plan->outStride.buf[u]);
		}
	}
	else
	{
		action->signature.data.fft_outputLayout = action->base.plan->outputLayout;
	}

	action->signature.data.fft_DataDim = array_size(&action->base.plan->length) + 1;
	int i = 0;
	for (i = 0; i < (action->signature.data.fft_DataDim - 1); i++)
	{
		action->signature.data.fft_N[i] = action->base.plan->length.buf[i];
		action->signature.data.fft_inStride[i] = action->base.plan->inStride.buf[i];
		action->signature.data.fft_outStride[i] = action->base.plan->outStride.buf[i];
	}
	action->signature.data.fft_inStride[i] = action->base.plan->iDist;
	action->signature.data.fft_outStride[i] = action->base.plan->oDist;

	action->signature.data.fft_RCsimple = action->base.plan->RCsimple;

	action->signature.data.fft_realSpecial = action->base.plan->realSpecial;
	action->signature.data.fft_realSpecial_Nr = action->base.plan->realSpecial_Nr;

	action->signature.data.blockCompute = action->base.plan->blockCompute;
	action->signature.data.blockComputeType = action->base.plan->blockComputeType;

	action->signature.data.fft_twiddleFront = action->base.plan->twiddleFront;

	size_t wgs, nt;
	size_t t_wgs, t_nt;
	Precision pr = (action->signature.data.fft_precision == CLFFT_SINGLE) ? P_SINGLE : P_DOUBLE;
	switch (pr)
	{
		case P_SINGLE:
		{
			KernelCoreSpecsGetWGSAndNT(P_SINGLE, action->signature.data.fft_N[0], &t_wgs, &t_nt);
			if (action->signature.data.blockCompute)
			{
				action->signature.data.blockSIMD = KernelBlockWorkGroupSize(P_SINGLE, action->signature.data.fft_N[0]);
				action->signature.data.blockLDS = KernelBlockLdsSize(P_SINGLE, action->signature.data.fft_N[0]);
			}
		}
		break;
		case P_DOUBLE:
		{
			KernelCoreSpecsGetWGSAndNT(P_DOUBLE, action->signature.data.fft_N[0], &t_wgs, &t_nt);
			if (action->signature.data.blockCompute)
			{
				action->signature.data.blockSIMD = KernelBlockWorkGroupSize(P_DOUBLE, action->signature.data.fft_N[0]);
				action->signature.data.blockLDS = KernelBlockLdsSize(P_DOUBLE, action->signature.data.fft_N[0]);
			}
		}
		break;
	}

	if ((t_wgs != 0) && (t_nt != 0) && (action->base.plan->envelope.limit_WorkGroupSize >= 256))
	{
		wgs = t_wgs;
		nt = t_nt;
	}
	else
		DetermineSizes(action->base.plan->envelope.limit_WorkGroupSize, action->signature.data.fft_N[0], &wgs, &nt, &pr);

	assert((nt * action->signature.data.fft_N[0]) >= wgs);
	assert((nt * action->signature.data.fft_N[0]) % wgs == 0);

	action->signature.data.fft_R = (nt * action->signature.data.fft_N[0]) / wgs;
	action->signature.data.fft_SIMD = wgs;

	// Set pre-callback if specified
	if (action->base.plan->hasPreCallback)
	{
		action->signature.data.fft_hasPreCallback = true;
		action->signature.data.fft_preCallback = action->base.plan->preCallback;
	}

	// Set post-callback if specified
	if (action->base.plan->hasPostCallback)
	{
		action->signature.data.fft_hasPostCallback = true;
		action->signature.data.fft_postCallback = action->base.plan->postCallbackParam;
	}
	action->signature.data.limit_LocalMemSize = action->base.plan->envelope.limit_LocalMemSize;

	if (action->base.plan->large1D != 0)
	{
		ARG_CHECK(action->signature.data.fft_N[0] != 0)
		ARG_CHECK((action->base.plan->large1D % action->signature.data.fft_N[0]) == 0)
		action->signature.data.fft_3StepTwiddle = true;
		if (!(action->base.plan->realSpecial))
			ARG_CHECK(action->base.plan->large1D == (action->signature.data.fft_N[1] * action->signature.data.fft_N[0]));
	}

	action->signature.data.fft_fwdScale = action->base.plan->forwardScale;
	action->signature.data.fft_backScale = action->base.plan->backwardScale;

	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedStockhamActionGetWorkSizes(FFTGeneratedStockhamAction *action, array_size_t *globalWS, array_size_t *localWS)
{
	//    How many complex numbers in the input mutl-dimensional array?
	//
	unsigned long long count = 1;
	for (unsigned u = 0; u < array_size(&action->base.plan->length); ++u)
		count *= clfft_max_size_t((size_t) 1, action->base.plan->length.buf[u]);
	count *= action->base.plan->batchsize;

	if (action->signature.data.blockCompute)
	{
		count = DivRoundingUpULL(count, action->signature.data.blockLDS);
		count = count * action->signature.data.blockSIMD;

		array_push_back(globalWS, ((size_t) (count)));
		array_push_back(localWS, action->signature.data.blockSIMD);

		return CLFFT_SUCCESS;
	}

	count = DivRoundingUpULL(count, action->signature.data.fft_R);	  // count of WorkItems
	count = DivRoundingUpULL(count, action->signature.data.fft_SIMD); // count of WorkGroups

	// for real transforms we only need half the work groups since we do twice
	// the work in 1 work group
	if (!(action->signature.data.fft_RCsimple) && ((action->signature.data.fft_inputLayout == CLFFT_REAL) || (action->signature.data.fft_outputLayout == CLFFT_REAL)))
		count = DivRoundingUpULL(count, 2);

	count = clfft_max_ull(count, 1) * action->signature.data.fft_SIMD;
	// .. count of WorkItems, rounded up to next multiple of fft_SIMD.

	// 1 dimension work group size
	array_push_back(globalWS, ((size_t) (count)));

	array_push_back(localWS, action->signature.data.fft_SIMD);

	return CLFFT_SUCCESS;
}

static clfftStatus FFTPlanGetMax1DLengthStockham(const FFTPlan *fftPlan, size_t *longest)
{
	// TODO  The caller has already acquired the lock on *this
	//	However, we shouldn't depend on it.

	//	Query the devices in this context for their local memory sizes
	//	How large a kernel we can generate depends on the *minimum* LDS
	//	size for all devices.
	//
	const FFTEnvelope *pEnvelope = NULL;
	OPENCL_V(FFTPlanGetEnvelope(fftPlan, &pEnvelope), "GetEnvelope failed");
	BUG_CHECK(NULL != pEnvelope);

	ARG_CHECK(NULL != longest)
	size_t LdsperElement = FFTPlanElementSize(fftPlan);
	size_t result = pEnvelope->limit_LocalMemSize / (1 * LdsperElement);
	result = FloorPo2(result);
	*longest = result;
	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedStockhamActionGenerateKernel(FFTGeneratedStockhamAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT)
{
	cl_int status = CL_SUCCESS;
	cl_device_id Device = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_DEVICE, sizeof(cl_device_id), &Device, NULL);
	OPENCL_V(status, _T( "clGetCommandQueueInfo failed" ));

	cl_context QueueContext = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_CONTEXT, sizeof(cl_context), &QueueContext, NULL);
	OPENCL_V(status, _T( "clGetCommandQueueInfo failed" ));

	buffer_t programCode = buffer_empty();
	Precision pr = (action->signature.data.fft_precision == CLFFT_SINGLE) ? P_SINGLE : P_DOUBLE;
	switch (pr)
	{
		case P_SINGLE:
		{
			// Initialize and generate the single-precision Stockham kernel
			// explicitly.
			Kernel kernel;
			KernelInit(&kernel, P_SINGLE, action->signature.data);
			KernelGenerateKernel(&kernel, &programCode, Device);
		}
		break;
		case P_DOUBLE:
		{
			// Initialize and generate the double-precision Stockham kernel
			// explicitly.
			Kernel kernel;
			KernelInit(&kernel, P_DOUBLE, action->signature.data);
			KernelGenerateKernel(&kernel, &programCode, Device);
		}
		break;
	}

	// Requested local memory size by callback must not exceed the device LDS
	// limits after factoring the LDS size required by main FFT kernel
	if ((action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0) ||
		(action->signature.data.fft_hasPostCallback && action->signature.data.fft_postCallback.localMemSize > 0))
	{
		bool validLDSSize = false;
		size_t requestedCallbackLDS = 0;

		if (action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0)
			requestedCallbackLDS = action->signature.data.fft_preCallback.localMemSize;
		if (action->signature.data.fft_hasPostCallback && action->signature.data.fft_postCallback.localMemSize > 0)
			requestedCallbackLDS += action->signature.data.fft_postCallback.localMemSize;

		if (action->base.plan->blockCompute)
		{
			validLDSSize =
				((action->signature.data.blockLDS * FFTPlanElementSize(action->base.plan)) + requestedCallbackLDS) < action->base.plan->envelope.limit_LocalMemSize;
		}
		else
		{
			size_t length = action->signature.data.fft_N[0];
			size_t workGroupSize = action->signature.data.fft_SIMD;
			size_t numTrans = (workGroupSize * action->signature.data.fft_R) / length;

			// TODO - Need to abstract this out. Repeating the same compute as
			// in GenerateKernel.
			//  Set half lds only for power-of-2 problem sizes & interleaved
			//  data
			bool halfLds =
				((action->signature.data.fft_inputLayout == CLFFT_COMPLEX_INTERLEAVED) && (action->signature.data.fft_outputLayout == CLFFT_COMPLEX_INTERLEAVED))
				? true
				: false;
			halfLds = halfLds ? ((length & (length - 1)) ? false : true) : false;

			// Set half lds for real transforms
			halfLds = ((action->signature.data.fft_inputLayout == CLFFT_REAL) && (action->signature.data.fft_outputLayout == CLFFT_REAL)) ? true : halfLds;

			size_t ldsSize = halfLds ? length * numTrans : 2 * length * numTrans;
			size_t elementSize = ((action->signature.data.fft_precision == CLFFT_DOUBLE) || (action->signature.data.fft_precision == CLFFT_DOUBLE_FAST))
				? sizeof(double)
				: sizeof(float);

			validLDSSize = ((ldsSize * elementSize) + requestedCallbackLDS) < action->base.plan->envelope.limit_LocalMemSize;
		}

		if (!validLDSSize)
		{
			fprintf(stderr, "Requested local memory size not available\n");
			return CLFFT_INVALID_ARG_VALUE;
		}
	}

	OPENCL_V(FFTRepoSetProgramCode(fftRepo, FFTActionGetGenerator(&action->base), &action->signature.header, &programCode, Device, QueueContext),
		_T( "FFTRepoSetProgramCode failed!" ));
	OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, FFTActionGetGenerator(&action->base), &action->signature.header, "fft_fwd", "fft_back", Device, QueueContext),
		_T( "FFTRepoSetProgramEntryPoints failed!" ));

	return CLFFT_SUCCESS;
}
/* End copied source: src\library\generator.stockham.cpp */

/* Begin copied source: src\library\generator.transpose.cpp */

/*
This file contains the implementation of inplace transpose kernel string
generation. This includes both square and non square, twiddle and non twiddle,
as well as the kernels that swap lines following permutation algorithm.
*/

// generating string for calculating offset within sqaure transpose kernels
// (clfft_transpose_generator_genTransposeKernelBatched)
static void clfft_transpose_generator_OffsetCalc(buffer_stream_t *transKernel, const FFTKernelGenKeyParams *params, bool input)
{
	const size_t *stride = input ? params->fft_inStride : params->fft_outStride;
	buffer_t offset = buffer_from_cstr(input ? "iOffset" : "oOffset");

	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "size_t %s = 0;\n", bufcstr(&offset));
	} while (0);
	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "g_index = get_group_id(0);\n");
	} while (0);

	for (size_t i = params->fft_DataDim - 2; i > 0; i--)
	{
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "%s += (g_index/numGroupsY_%llu)*%llu;\n", bufcstr(&offset), (unsigned long long) (i), (unsigned long long) (stride[i + 1]));
		} while (0);
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "g_index = g_index %% numGroupsY_%llu;\n", (unsigned long long) (i));
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 3);
		bufstream_endline(transKernel);
	} while (0);
}

// generating string for calculating offset within sqaure transpose kernels
// (clfft_transpose_generator_genTransposeKernelLeadingDimensionBatched)
static void clfft_transpose_generator_OffsetCalcLeadingDimensionBatched(buffer_stream_t *transKernel, const FFTKernelGenKeyParams *params)
{
	const size_t *stride = params->fft_inStride;
	buffer_t offset = buffer_from_cstr("iOffset");

	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "size_t %s = 0;\n", bufcstr(&offset));
	} while (0);
	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "g_index = get_group_id(0);\n");
	} while (0);

	for (size_t i = params->fft_DataDim - 2; i > 0; i--)
	{
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "%s += (g_index/numGroupsY_%llu)*%llu;\n", bufcstr(&offset), (unsigned long long) (i), (unsigned long long) (stride[i + 1]));
		} while (0);
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "g_index = g_index %% numGroupsY_%llu;\n", (unsigned long long) (i));
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 3);
		bufstream_endline(transKernel);
	} while (0);
}

// Small snippet of code that multiplies the twiddle factors into the
// butterfiles.  It is only emitted if the plan tells the generator that it
// wants the twiddle factors generated inside of the transpose
static clfftStatus clfft_transpose_generator_genTwiddleMath(const FFTKernelGenKeyParams *params, buffer_stream_t *transKernel, const buffer_t *dtComplex, bool fwd)
{
	do
	{
		clKernWrite(transKernel, 9);
		bufstream_endline(transKernel);
	} while (0);

	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text,
			"%s Wm = TW3step( (t_gx_p*32 + lidx) * (t_gy_p*32 + lidy + "
			"loop*8) );\n",
			bufcstr(dtComplex));
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text,
			"%s Wt = TW3step( (t_gy_p*32 + lidx) * (t_gx_p*32 + lidy + "
			"loop*8) );\n",
			bufcstr(dtComplex));
	} while (0);

	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "%s Tm, Tt;\n", bufcstr(dtComplex));
	} while (0);

	if (fwd)
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.x = ( Wm.x * tmpm.x ) - ( Wm.y * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.y = ( Wm.y * tmpm.x ) + ( Wm.x * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.x = ( Wt.x * tmpt.x ) - ( Wt.y * tmpt.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.y = ( Wt.y * tmpt.x ) + ( Wt.x * tmpt.y );\n");
		} while (0);
	}
	else
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.x =  ( Wm.x * tmpm.x ) + ( Wm.y * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.y = -( Wm.y * tmpm.x ) + ( Wm.x * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.x =  ( Wt.x * tmpt.x ) + ( Wt.y * tmpt.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.y = -( Wt.y * tmpt.x ) + ( Wt.x * tmpt.y );\n");
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpm.x = Tm.x;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpm.y = Tm.y;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpt.x = Tt.x;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpt.y = Tt.y;\n");
	} while (0);

	do
	{
		clKernWrite(transKernel, 9);
		bufstream_endline(transKernel);
	} while (0);

	return CLFFT_SUCCESS;
}

// Small snippet of code that multiplies the twiddle factors into the
// butterfiles.  It is only emitted if the plan tells the generator that it
// wants the twiddle factors generated inside of the transpose
static clfftStatus clfft_transpose_generator_genTwiddleMathLeadingDimensionBatched(const FFTKernelGenKeyParams *params, buffer_stream_t *transKernel, const buffer_t *dtComplex,
	bool fwd)
{
	do
	{
		clKernWrite(transKernel, 9);
		bufstream_endline(transKernel);
	} while (0);
	if (params->fft_N[0] > params->fft_N[1])
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text,
				"%s Wm = TW3step( (%llu * square_matrix_index + "
				"t_gx_p*32 + lidx) * (t_gy_p*32 + lidy + loop*8) );\n",
				bufcstr(dtComplex), (unsigned long long) (params->fft_N[1]));
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text,
				"%s Wt = TW3step( (%llu * square_matrix_index + "
				"t_gy_p*32 + lidx) * (t_gx_p*32 + lidy + loop*8) );\n",
				bufcstr(dtComplex), (unsigned long long) (params->fft_N[1]));
		} while (0);
	}
	else
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text,
				"%s Wm = TW3step( (t_gx_p*32 + lidx) * (%llu * "
				"square_matrix_index + t_gy_p*32 + lidy + loop*8) );\n",
				bufcstr(dtComplex), (unsigned long long) (params->fft_N[0]));
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text,
				"%s Wt = TW3step( (t_gy_p*32 + lidx) * (%llu * "
				"square_matrix_index + t_gx_p*32 + lidy + loop*8) );\n",
				bufcstr(dtComplex), (unsigned long long) (params->fft_N[0]));
		} while (0);
	}
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "%s Tm, Tt;\n", bufcstr(dtComplex));
	} while (0);

	if (fwd)
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.x = ( Wm.x * tmpm.x ) - ( Wm.y * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.y = ( Wm.y * tmpm.x ) + ( Wm.x * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.x = ( Wt.x * tmpt.x ) - ( Wt.y * tmpt.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.y = ( Wt.y * tmpt.x ) + ( Wt.x * tmpt.y );\n");
		} while (0);
	}
	else
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.x =  ( Wm.x * tmpm.x ) + ( Wm.y * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tm.y = -( Wm.y * tmpm.x ) + ( Wm.x * tmpm.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.x =  ( Wt.x * tmpt.x ) + ( Wt.y * tmpt.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "Tt.y = -( Wt.y * tmpt.x ) + ( Wt.x * tmpt.y );\n");
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpm.x = Tm.x;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpm.y = Tm.y;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpt.x = Tt.x;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmpt.y = Tt.y;\n");
	} while (0);

	do
	{
		clKernWrite(transKernel, 9);
		bufstream_endline(transKernel);
	} while (0);

	return CLFFT_SUCCESS;
}

static clfftStatus clfft_transpose_generator_genTransposePrototype(const FFTKernelGenKeyParams *params, size_t lwSize, const buffer_t *dtPlanar, const buffer_t *dtComplex,
	const buffer_t *funcName, buffer_stream_t *transKernel, buffer_t *dtInput, buffer_t *dtOutput)
{
	// Declare and define the function
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "__attribute__(( reqd_work_group_size( %llu, 1, 1 ) ))\n", (unsigned long long) (lwSize));
	} while (0);
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "kernel void\n");
	} while (0);

	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "%s( ", bufcstr(funcName));
	} while (0);

	switch (params->fft_inputLayout)
	{
		case CLFFT_COMPLEX_INTERLEAVED:
			bufsetbuf(dtInput, dtComplex);
			bufsetbuf(dtOutput, dtComplex);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict inputA", bufcstr(dtInput));
			} while (0);
			break;
		case CLFFT_COMPLEX_PLANAR:
			bufsetbuf(dtInput, dtPlanar);
			bufsetbuf(dtOutput, dtPlanar);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict inputA_R, global %s* restrict inputA_I", bufcstr(dtInput), bufcstr(dtInput));
			} while (0);
			break;
		case CLFFT_HERMITIAN_INTERLEAVED:
		case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		case CLFFT_REAL:
			bufsetbuf(dtInput, dtPlanar);
			bufsetbuf(dtOutput, dtPlanar);

			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict inputA", bufcstr(dtInput));
			} while (0);
			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}

	if (params->fft_placeness == CLFFT_OUTOFPLACE)
		switch (params->fft_outputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
				bufsetbuf(dtInput, dtComplex);
				bufsetbuf(dtOutput, dtComplex);
				do
				{
					clKernWrite(transKernel, 0);
					bufprintf(&transKernel->text, ", global %s* restrict outputA", bufcstr(dtOutput));
				} while (0);
				break;
			case CLFFT_COMPLEX_PLANAR:
				bufsetbuf(dtInput, dtPlanar);
				bufsetbuf(dtOutput, dtPlanar);
				do
				{
					clKernWrite(transKernel, 0);
					bufprintf(&transKernel->text,
						", global %s* restrict outputA_R, global %s* "
						"restrict outputA_I",
						bufcstr(dtOutput), bufcstr(dtOutput));
				} while (0);
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_REAL:
				bufsetbuf(dtInput, dtPlanar);
				bufsetbuf(dtOutput, dtPlanar);
				do
				{
					clKernWrite(transKernel, 0);
					bufprintf(&transKernel->text, ", global %s* restrict outputA", bufcstr(dtOutput));
				} while (0);
				break;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

	if (params->fft_hasPreCallback)
	{
		assert(!params->fft_hasPostCallback);

		if (params->fft_preCallback.localMemSize > 0)
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* pre_userdata, __local void* localmem");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* pre_userdata");
			} while (0);
		}
	}
	if (params->fft_hasPostCallback)
	{
		assert(!params->fft_hasPreCallback);

		if (params->fft_postCallback.localMemSize > 0)
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* post_userdata, __local void* localmem");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* post_userdata");
			} while (0);
		}
	}

	// Close the method signature
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, " )\n{\n");
	} while (0);
	return CLFFT_SUCCESS;
}

static clfftStatus clfft_transpose_generator_genTransposePrototypeLeadingDimensionBatched(const FFTKernelGenKeyParams *params, size_t lwSize, const buffer_t *dtPlanar,
	const buffer_t *dtComplex, const buffer_t *funcName, buffer_stream_t *transKernel, buffer_t *dtInput, buffer_t *dtOutput)
{
	// Declare and define the function
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "__attribute__(( reqd_work_group_size( %llu, 1, 1 ) ))\n", (unsigned long long) (lwSize));
	} while (0);
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "kernel void\n");
	} while (0);

	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "%s( ", bufcstr(funcName));
	} while (0);

	switch (params->fft_inputLayout)
	{
		case CLFFT_COMPLEX_INTERLEAVED:
			bufsetbuf(dtInput, dtComplex);
			bufsetbuf(dtOutput, dtComplex);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict inputA", bufcstr(dtInput));
			} while (0);
			break;
		case CLFFT_COMPLEX_PLANAR:
			bufsetbuf(dtInput, dtPlanar);
			bufsetbuf(dtOutput, dtPlanar);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict inputA_R, global %s* restrict inputA_I", bufcstr(dtInput), bufcstr(dtInput));
			} while (0);
			break;
		case CLFFT_HERMITIAN_INTERLEAVED:
		case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		case CLFFT_REAL:
			bufsetbuf(dtInput, dtPlanar);
			bufsetbuf(dtOutput, dtPlanar);

			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict inputA", bufcstr(dtInput));
			} while (0);
			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}

	if (params->fft_hasPreCallback)
	{
		assert(!params->fft_hasPostCallback);
		if (params->fft_preCallback.localMemSize > 0)
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* pre_userdata, __local void* localmem");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* pre_userdata");
			} while (0);
		}
	}
	if (params->fft_hasPostCallback)
	{
		assert(!params->fft_hasPreCallback);

		if (params->fft_postCallback.localMemSize > 0)
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* post_userdata, __local void* localmem");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* post_userdata");
			} while (0);
		}
	}

	// Close the method signature
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, " )\n{\n");
	} while (0);
	return CLFFT_SUCCESS;
}

/* -> clfft_transpose_generator_get_cycles function gets the swapping logic
required for given row x col matrix.
-> cycle_map[0] holds the total number of cycles required.
-> cycles start and end with the same index, hence we can identify individual
cycles, though we tend to store the cycle index contiguously*/
static void clfft_transpose_generator_get_cycles(size_t *cycle_map, size_t num_reduced_row, size_t num_reduced_col)
{
	int *is_swapped = (int *) clfft_checked_malloc(num_reduced_row * num_reduced_col * sizeof(int));
	int i, map_index = 1, num_cycles = 0;
	size_t swap_id;
	/*initialize swap map*/
	is_swapped[0] = 1;
	is_swapped[num_reduced_row * num_reduced_col - 1] = 1;
	for (i = 1; i < (num_reduced_row * num_reduced_col - 1); i++)
		is_swapped[i] = 0;

	for (i = 1; i < (num_reduced_row * num_reduced_col - 1); i++)
	{
		swap_id = i;
		while (!is_swapped[swap_id])
		{
			is_swapped[swap_id] = 1;
			cycle_map[map_index++] = swap_id;
			swap_id = (num_reduced_row * swap_id) % (num_reduced_row * num_reduced_col - 1);
			if (swap_id == i)
			{
				cycle_map[map_index++] = swap_id;
				num_cycles++;
			}
		}
	}
	cycle_map[0] = num_cycles;
	free(is_swapped);
}

/*
calculate the permutation cycles consumed in swap kernels.
each cycle is strored in a vecotor. hopfully there are mutliple independent
vectors thus we use a vector of vecotor
*/
static void clfft_transpose_generator_permutation_calculation(size_t m, size_t n, array_size_t_array *permutationVec)
{
	/*
	calculate inplace transpose permutation lists
	reference:
	https://en.wikipedia.org/wiki/In-place_matrix_transposition
	and
	http://www.netlib.org/utk/people/JackDongarra/CCDSC-2014/talk35.pdf
	row major matrix of size n x m
	p(k) = (k*n)mod(m*n-1), if 0 < k < m*n-1
	when k = 0 or m*n-1, it does not require movement
	*/
	if (m < 1 || n < 1)
		return;

	size_t mn_minus_one = m * n - 1;
	// maintain a table so check is faster
	size_t *table = (size_t *) clfft_checked_calloc(mn_minus_one + 1, sizeof(size_t));
	table[0] = 1;

	for (size_t i = 1; i < mn_minus_one; i++)
	{
		// first check if i is already stored in somewhere in vector of vectors
		bool already_checked = false;
		if (table[i] >= 1)
			already_checked = true;
		if (already_checked == true)
			continue;

		// if not checked yet
		array_size_t vec;
		// Initialize dynamic array storage explicitly.
		array_init(&vec);
		array_push_back(&vec, i);
		table[i] += 1;
		size_t temp = i;

		while (1)
		{
			temp = (temp * n);
			temp = temp % (mn_minus_one);
			if (array_contains_size_t(&vec, temp))
			{
				// what goes around comes around and it should
				break;
			}
			if (table[temp] >= 1)
			{
				already_checked = true;
				break;
			}
			array_push_back(&vec, temp);
			table[temp] += 1;
		}
		if (already_checked == true)
		{
			// Release the partial cycle vector before skipping it.
			array_free(&vec);
			continue;
		}
		array_push_back_size_t_array(permutationVec, &vec);
		// Release the temporary cycle vector after copying it into the output
		// table.
		array_free(&vec);
	}
	free(table);
}

// swap lines. a more general kernel generator.
// this function accepts any ratio in theory. But in practice we restrict it to
// 1:2, 1:3, 1:5 and 1:10 ration
static clfftStatus clfft_transpose_generator_genSwapKernelGeneral(const FFTKernelGenKeyParams params, buffer_t *strKernel, buffer_t *KernelFuncName, size_t lwSize,
	const size_t reShapeFactor)
{
	if (params.fft_placeness == CLFFT_OUTOFPLACE)
		return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	size_t smaller_dim = (params.fft_N[0] < params.fft_N[1]) ? params.fft_N[0] : params.fft_N[1];
	size_t bigger_dim = (params.fft_N[0] >= params.fft_N[1]) ? params.fft_N[0] : params.fft_N[1];
	size_t dim_ratio = bigger_dim / smaller_dim;
	/*
	if ( (params.fft_N[0] != 2 * params.fft_N[1]) && (params.fft_N[1] != 2 *
	params.fft_N[0]) && (params.fft_N[0] != 3 * params.fft_N[1]) &&
	(params.fft_N[1] != 3 * params.fft_N[0]) && (params.fft_N[0] != 5 *
	params.fft_N[1]) && (params.fft_N[1] != 5 * params.fft_N[0]) &&
		 (params.fft_N[0] != 10 * params.fft_N[1]) && (params.fft_N[1] != 10 *
	params.fft_N[0]) )
	*/
	if (dim_ratio % 2 != 0 && dim_ratio % 3 != 0 && dim_ratio % 5 != 0 && dim_ratio % 10 != 0)
	{
		return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}

	bufreserve(strKernel, 4096);
	buffer_stream_t transKernel;
	// Initialize stream formatting state explicitly.
	bufstream_init(&transKernel);
	// These strings represent the various data types we read or write in the
	// kernel, depending on how the plan is configured
	buffer_t dtInput = buffer_empty();  // The type read as input into kernel
	buffer_t dtOutput = buffer_empty(); // The type written as output from kernel
	buffer_t dtPlanar = buffer_empty(); // Fundamental type for planar arrays
	buffer_t tmpBuffType = buffer_empty();
	buffer_t dtComplex = buffer_empty(); // Fundamental type for complex arrays

	// NOTE:  Enable only for debug

	// if (params.fft_inputLayout != params.fft_outputLayout)
	//	return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	switch (params.fft_precision)
	{
		case CLFFT_SINGLE:
		case CLFFT_SINGLE_FAST:
			bufsetcstr(&dtPlanar, "float");
			bufsetcstr(&dtComplex, "float2");
			break;
		case CLFFT_DOUBLE:
		case CLFFT_DOUBLE_FAST:
			bufsetcstr(&dtPlanar, "double");
			bufsetcstr(&dtComplex, "double2");

			// Emit code that enables double precision in the kernel
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#ifdef cl_khr_fp64\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#else\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#endif\n\n");
			} while (0);

			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED; break;
	}

	size_t LDS_per_WG = smaller_dim;
	while (LDS_per_WG > 1024) // avoiding using too much lds memory. the biggest LDS memory
				  // we will allocate would be 1024*sizeof(float2/double2)*2
	{
		if (LDS_per_WG % 2 == 0)
		{
			LDS_per_WG /= 2;
			continue;
		}
		if (LDS_per_WG % 3 == 0)
		{
			LDS_per_WG /= 3;
			continue;
		}
		if (LDS_per_WG % 5 == 0)
		{
			LDS_per_WG /= 5;
			continue;
		}
		return CLFFT_NOTIMPLEMENTED;
	}
	size_t WG_per_line = smaller_dim / LDS_per_WG;

	size_t input_elm_size_in_bytes;
	switch (params.fft_precision)
	{
		case CLFFT_SINGLE:
		case CLFFT_SINGLE_FAST: input_elm_size_in_bytes = 4; break;
		case CLFFT_DOUBLE:
		case CLFFT_DOUBLE_FAST: input_elm_size_in_bytes = 8; break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}

	switch (params.fft_outputLayout)
	{
		case CLFFT_COMPLEX_INTERLEAVED:
		case CLFFT_COMPLEX_PLANAR: input_elm_size_in_bytes *= 2; break;
		case CLFFT_REAL: break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}
	/* not entirely clearly why do i need this yet
	size_t max_elements_loaded = AVAIL_MEM_SIZE / input_elm_size_in_bytes;
	size_t num_elements_loaded;
	size_t local_work_size_swap, num_grps_pro_row;
	*/

	// if pre-callback is set for the plan
	if (params.fft_hasPreCallback)
	{
		// we have already checked available LDS for pre callback
		// Insert callback function code at the beginning
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", params.fft_preCallback.funcstring);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}
	// if post-callback is set for the plan
	// rarely do we need post callback in swap kernel. But it is possible.
	if (params.fft_hasPostCallback)
	{
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", params.fft_postCallback.funcstring);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	// twiddle in swap kernel (for now, swap with twiddle seems to always be the
	// second kernel after transpose)
	bool twiddleSwapKernel = params.fft_3StepTwiddle && (dim_ratio > 1);
	// twiddle factors applied to the output of swap kernels if swap kernels are
	// the last kernel in transpose order
	bool twiddleSwapKernelOut = twiddleSwapKernel && (params.nonSquareKernelOrder == TRANSPOSE_AND_SWAP || params.nonSquareKernelOrder == TRANSPOSE_LEADING_AND_SWAP);
	// twiddle factors applied to the input of swap kernels if swap kernels are
	// the first kernel in transpose order
	bool twiddleSwapKernelIn = twiddleSwapKernel && (params.nonSquareKernelOrder == SWAP_AND_TRANSPOSE);

	// generate the swap_table
	array_size_t_array permutationTable;
	// Initialize dynamic array storage explicitly.
	array_init(&permutationTable);
	clfft_transpose_generator_permutation_calculation(dim_ratio, smaller_dim, &permutationTable);

	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "__constant size_t swap_table[%llu][1] = {\n", (unsigned long long) (array_size(&permutationTable) + 2));
	} while (0);
	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "{0},\n");
	} while (0);
	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "{%llu},", (unsigned long long) (smaller_dim * dim_ratio - 1));
		bufstream_endline(&transKernel); // add the first and last row to the
						 // swap table. needed for twiddling
	} while (0);
	// Walk the permutation rows with raw array pointers.
	array_size_t *permutationTable_end = permutationTable.buf ? permutationTable.buf + permutationTable.len : NULL;
	for (array_size_t *itor = permutationTable.buf; itor != permutationTable_end; itor++)
	{
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "{%llu}", (unsigned long long) (itor->buf[0]));
		} while (0);
		if (itor == (permutationTable_end - 1)) // last vector
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "\n};\n");
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, ",\n");
			} while (0);
	}

	// twiddle in swap kernel
	// twiddle in or out should be using the same twiddling table
	if (twiddleSwapKernel)
	{
		buffer_t str = buffer_empty();
		TwiddleTableLarge twLarge;
		// Initialize large twiddle table storage explicitly.
		TwiddleTableLargeInit(&twLarge, smaller_dim * smaller_dim * dim_ratio);
		if ((params.fft_precision == CLFFT_SINGLE) || (params.fft_precision == CLFFT_SINGLE_FAST))
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_SINGLE, &str);
		else
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_DOUBLE, &str);
		// Release large twiddle table storage after emitting source.
		TwiddleTableLargeFree(&twLarge);
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", bufcstr(&str));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	buffer_t funcName = buffer_from_cstr("swap_nonsquare_");
	buffer_t smaller_dim_str = SztToStr(smaller_dim);
	buffer_t dim_ratio_str = SztToStr(dim_ratio);
	if (params.fft_N[0] > params.fft_N[1])
	{
		// Append the smaller dimension and ratio to the swap kernel name.
		bufcatbuf(&funcName, &smaller_dim_str);
		bufcatcstr(&funcName, "_");
		bufcatbuf(&funcName, &dim_ratio_str);
	}
	else
	{
		// Append the ratio and smaller dimension to the swap kernel name.
		bufcatbuf(&funcName, &dim_ratio_str);
		bufcatcstr(&funcName, "_");
		bufcatbuf(&funcName, &smaller_dim_str);
	}

	bufsetbuf(KernelFuncName, &funcName);
	size_t local_work_size_swap = 256;

	for (size_t bothDir = 0; bothDir < 2; bothDir++)
	{
		bool fwd = bothDir ? false : true;
		// Generate kernel API

		/*when swap can be performed in LDS itself then, same prototype of
		 * transpose can be used for swap function too*/
		buffer_t funcNameTW = buffer_empty();
		if (twiddleSwapKernel)
		{
			if (fwd)
			{
				// Append the forward twiddle suffix to the swap kernel name.
				bufsetbuf(&funcNameTW, &funcName);
				bufcatcstr(&funcNameTW, "_tw_fwd");
			}
			else
			{
				// Append the backward twiddle suffix to the swap kernel name.
				bufsetbuf(&funcNameTW, &funcName);
				bufcatcstr(&funcNameTW, "_tw_back");
			}
		}
		else
			bufsetbuf(&funcNameTW, &funcName);

		clfft_transpose_generator_genTransposePrototypeLeadingDimensionBatched(&params, local_work_size_swap, &dtPlanar, &dtComplex, &funcNameTW, &transKernel, &dtInput,
			&dtOutput);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "//each wg handles 1/%llu row of %llu in memory\n", (unsigned long long) (WG_per_line), (unsigned long long) (LDS_per_WG));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t num_wg_per_batch = %llu;", (unsigned long long) ((array_size(&permutationTable) + 2) * WG_per_line));
			bufstream_endline(&transKernel); // number of wg per batch = number
							 // of independent cycles
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t group_id = get_group_id(0);\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t idx = get_local_id(0);\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t batch_offset = group_id / num_wg_per_batch;\n");
		} while (0);
		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL:
			case CLFFT_COMPLEX_INTERLEAVED:
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "inputA += batch_offset*%llu;\n", (unsigned long long) (smaller_dim * bigger_dim));
				} while (0);
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_COMPLEX_PLANAR:
			{
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "inputA_R += batch_offset*%llu;\n", (unsigned long long) (smaller_dim * bigger_dim));
				} while (0);
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "inputA_I += batch_offset*%llu;\n", (unsigned long long) (smaller_dim * bigger_dim));
				} while (0);
				break;
			}
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "group_id -= batch_offset*%llu;\n", (unsigned long long) ((array_size(&permutationTable) + 2) * WG_per_line));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		if (WG_per_line == 1)
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "size_t prev = swap_table[group_id][0];\n");
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "size_t prev = swap_table[group_id/%llu][0];\n", (unsigned long long) (WG_per_line));
			} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t next = 0;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL:
			case CLFFT_COMPLEX_INTERLEAVED:
			{
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s prevValue[%llu];", bufcstr(&dtInput), (unsigned long long) (LDS_per_WG));
					bufstream_endline(&transKernel); // lds within each wg should be able to store
									 // a row block (smaller_dim) of element
				} while (0);
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s nextValue[%llu];\n", bufcstr(&dtInput), (unsigned long long) (LDS_per_WG));
				} while (0);
				break;
			}
			case CLFFT_COMPLEX_PLANAR:
			{
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s prevValue[%llu];", bufcstr(&dtComplex), (unsigned long long) (LDS_per_WG));
					bufstream_endline(&transKernel); // lds within each wg should be able to store
									 // a row block (smaller_dim) of element
				} while (0);
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s nextValue[%llu];\n", bufcstr(&dtComplex), (unsigned long long) (LDS_per_WG));
				} while (0);
				break;
			}
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall or wide rectangle
		{
			if (WG_per_line == 1)
			{
				// might look like: size_t group_offset = (prev/3)*729*3 +
				// (prev%3)*729;
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text,
						"size_t group_offset = (prev/%llu)*%llu*%llu + "
						"(prev%%%llu)*%llu;\n",
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim), (unsigned long long) (dim_ratio),
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim));
				} while (0);
			}
			else
			{
				// if smaller_dim is 2187 > 1024 this should look like size_t
				// group_offset = (prev/3)*2187*3 + (prev%3)*2187 + (group_id %
				// 3)*729;
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text,
						"size_t group_offset = (prev/%llu)*%llu*%llu + "
						"(prev%%%llu)*%llu + (group_id %% %llu)*%llu;\n",
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim), (unsigned long long) (dim_ratio),
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim), (unsigned long long) (WG_per_line),
						(unsigned long long) (LDS_per_WG));
				} while (0);
			}
		}
		else
		{
			if (WG_per_line == 1) // might look like: size_t group_offset = prev*729;
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "size_t group_offset = (prev*%llu);\n", (unsigned long long) (smaller_dim));
				} while (0);
			else // if smaller_dim is 2187 > 1024 this should look like size_t
				// group_offset = prev*2187 + (group_id % 3)*729;
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text,
						"size_t group_offset = (prev*%llu) + (group_id "
						"%% %llu)*%llu;\n",
						(unsigned long long) (smaller_dim), (unsigned long long) (WG_per_line), (unsigned long long) (LDS_per_WG));
				} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		// move to that row block and load that row block to LDS
		if (twiddleSwapKernelIn)
		{
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "size_t p;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "size_t q;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "%s twiddle_factor;\n", bufcstr(&dtComplex));
			} while (0);
		}
		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL:
			case CLFFT_COMPLEX_INTERLEAVED:
			{
				for (size_t i = 0; i < LDS_per_WG; i = i + 256)
				{
					if (i + 256 < LDS_per_WG)
					{
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu] = "
									"%s(inputA-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.x - "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.y + "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.x + "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.x - "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu] = "
									"inputA[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
					}
					else
					{
						// need to handle boundary
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
						} while (0);
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu] = "
									"%s(inputA-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.x - "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.y + "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.x + "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.x - "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu] = "
									"inputA[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "}\n");
						} while (0);
					}
				}
				break;
			}
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_COMPLEX_PLANAR:
			{
				for (size_t i = 0; i < LDS_per_WG; i = i + 256)
				{
					if (i + 256 < LDS_per_WG)
					{
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu] = "
									"%s(inputA_R-batch_offset*%llu, "
									"inputA_I-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu].x = "
									"inputA_R[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu].y = "
									"inputA_I[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						}
					}
					else
					{
						// need to handle boundary
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
						} while (0);
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu] = "
									"%s(inputA_R-batch_offset*%llu, "
									"inputA_I-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"prevValue[idx+%llu].y = "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu].x = "
									"inputA_R[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 3);
								bufprintf(&transKernel.text,
									"prevValue[idx+%llu].y = "
									"inputA_I[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "}\n");
						} while (0);
					}
				}
				break;
			}
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_cat_cstr(&transKernel, "do{");
			bufstream_endline(&transKernel); // begining of do-while
		} while (0);
		// calculate the next location p(k) = (k*n)mod(m*n-1), if 0 < k < m*n-1
		if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall or wide rectangle
		{
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "next = (prev*%llu)%%%llu;\n", (unsigned long long) (smaller_dim), (unsigned long long) (smaller_dim * dim_ratio - 1));
			} while (0);
			// takes care the last row
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "if (prev == %llu)\n", (unsigned long long) (smaller_dim * dim_ratio - 1));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "next = %llu;\n", (unsigned long long) (smaller_dim * dim_ratio - 1));
			} while (0);
			if (WG_per_line == 1)
			{
				do
				{
					clKernWrite(&transKernel, 6);
					bufprintf(&transKernel.text,
						"group_offset = (next/%llu)*%llu*%llu + "
						"(next%%%llu)*%llu;",
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim), (unsigned long long) (dim_ratio),
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim));
					bufstream_endline(&transKernel); // might look like: group_offset =
									 // (next/3)*729*3 + (next%3)*729;
				} while (0);
			}
			else
			{
				// if smaller_dim is 2187 > 1024 this should look like size_t
				// group_offset = (next/3)*2187*3 + (next%3)*2187 + (group_id %
				// 3)*729;
				do
				{
					clKernWrite(&transKernel, 6);
					bufprintf(&transKernel.text,
						"group_offset = (next/%llu)*%llu*%llu + "
						"(next%%%llu)*%llu + (group_id %% %llu)*%llu;\n",
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim), (unsigned long long) (dim_ratio),
						(unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim), (unsigned long long) (WG_per_line),
						(unsigned long long) (LDS_per_WG));
				} while (0);
			}
		}
		else
		{
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "next = (prev*%llu)%%%llu;\n", (unsigned long long) (dim_ratio), (unsigned long long) (smaller_dim * dim_ratio - 1));
			} while (0);
			// takes care the last row
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "if (prev == %llu)\n", (unsigned long long) (smaller_dim * dim_ratio - 1));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "next = %llu;\n", (unsigned long long) (smaller_dim * dim_ratio - 1));
			} while (0);
			if (WG_per_line == 1) // might look like: size_t group_offset = prev*729;
				do
				{
					clKernWrite(&transKernel, 6);
					bufprintf(&transKernel.text, "group_offset = (next*%llu);\n", (unsigned long long) (smaller_dim));
				} while (0);
			else // if smaller_dim is 2187 > 1024 this should look like size_t
				// group_offset = next*2187 + (group_id % 3)*729;
				do
				{
					clKernWrite(&transKernel, 6);
					bufprintf(&transKernel.text,
						"group_offset = (next*%llu) + (group_id %% "
						"%llu)*%llu;\n",
						(unsigned long long) (smaller_dim), (unsigned long long) (WG_per_line), (unsigned long long) (LDS_per_WG));
				} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL:
			case CLFFT_COMPLEX_INTERLEAVED:
			{
				for (size_t i = 0; i < LDS_per_WG; i = i + 256)
				{
					if (i + 256 < LDS_per_WG)
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu] = "
									"%s(inputA-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (i));
							} while (0);
						}
						else
						{
							if (twiddleSwapKernelIn)
							{
								if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a
												       // tall or wide rectangle
								{
									// input is wide; output is tall; read input
									// index realted
									do
									{
										clKernWrite(&transKernel, 6);
										bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
											(unsigned long long) (bigger_dim));
									} while (0);
									do
									{
										clKernWrite(&transKernel, 6);
										bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
											(unsigned long long) (bigger_dim));
									} while (0);
								}
								else
								{
									// input is tall; output is wide; read input
									// index realted
									do
									{
										clKernWrite(&transKernel, 6);
										bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
											(unsigned long long) (smaller_dim));
									} while (0);
									do
									{
										clKernWrite(&transKernel, 6);
										bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
											(unsigned long long) (smaller_dim));
									} while (0);
								}
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
								} while (0);
								if (fwd)
								{
									// forward
									do
									{
										clKernWrite(&transKernel, 3);
										bufprintf(&transKernel.text,
											"nextValue[idx+%llu].x = "
											"inputA[group_offset+idx+%llu].x "
											"* twiddle_factor.x - "
											"inputA[group_offset+idx+%llu].y "
											"* twiddle_factor.y;\n",
											(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
									} while (0);
									do
									{
										clKernWrite(&transKernel, 3);
										bufprintf(&transKernel.text,
											"nextValue[idx+%llu].y = "
											"inputA[group_offset+idx+%llu].x "
											"* twiddle_factor.y + "
											"inputA[group_offset+idx+%llu].y "
											"* twiddle_factor.x;\n",
											(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
									} while (0);
								}
								else
								{
									// backward
									do
									{
										clKernWrite(&transKernel, 3);
										bufprintf(&transKernel.text,
											"nextValue[idx+%llu].x = "
											"inputA[group_offset+idx+%llu].x "
											"* twiddle_factor.x + "
											"inputA[group_offset+idx+%llu].y "
											"* twiddle_factor.y;\n",
											(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
									} while (0);
									do
									{
										clKernWrite(&transKernel, 3);
										bufprintf(&transKernel.text,
											"nextValue[idx+%llu].y = "
											"inputA[group_offset+idx+%llu].y "
											"* twiddle_factor.x - "
											"inputA[group_offset+idx+%llu].x "
											"* twiddle_factor.y;\n",
											(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
									} while (0);
								}
							}
							else
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu] = "
										"inputA[group_offset+idx+%llu];\n",
										(unsigned long long) (i), (unsigned long long) (i));
								} while (0);
						}
					else
					{
						// need to handle boundary
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
						} while (0);
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu] = "
									"%s(inputA-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].x = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.x - "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].y = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.y + "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].x = "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.x + "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].y = "
										"inputA[group_offset+idx+%llu].y * "
										"twiddle_factor.x - "
										"inputA[group_offset+idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu] = "
									"inputA[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "}\n");
						} while (0);
					}
				}
				break;
			}
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_COMPLEX_PLANAR:
			{
				for (size_t i = 0; i < LDS_per_WG; i = i + 256)
				{
					if (i + 256 < LDS_per_WG)
					{
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu] = "
									"%s(inputA_R-batch_offset*%llu, "
									"inputA_I-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].y = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].y = "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu].x = "
									"inputA_R[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu].y = "
									"inputA_I[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						}
					}
					else
					{
						// need to handle boundary
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
						} while (0);
						if (params.fft_hasPreCallback)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu] = "
									"%s(inputA_R-batch_offset*%llu, "
									"inputA_I-batch_offset*%llu, "
									"batch_offset*%llu+group_offset+idx+%llu, "
									"pre_userdata);\n",
									(unsigned long long) (i), params.fft_preCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (smaller_dim * bigger_dim),
									(unsigned long long) (i));
							} while (0);
						}
						else if (twiddleSwapKernelIn)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide; read input index
								// realted
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].y = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].x = "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.x + "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 3);
									bufprintf(&transKernel.text,
										"nextValue[idx+%llu].y = "
										"inputA_I[group_offset+idx+%llu] * "
										"twiddle_factor.x - "
										"inputA_R[group_offset+idx+%llu] * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu].x = "
									"inputA_R[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"nextValue[idx+%llu].y = "
									"inputA_I[group_offset+idx+%llu];\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "}\n");
						} while (0);
					}
				}
				break;
			}
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

		do
		{
			clKernWrite(&transKernel, 6);
			bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL: // for real case this is different
			case CLFFT_COMPLEX_INTERLEAVED:
			{
				if (twiddleSwapKernelOut)
				{
					do
					{
						clKernWrite(&transKernel, 6);
						bufprintf(&transKernel.text, "size_t p;\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 6);
						bufprintf(&transKernel.text, "size_t q;\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 6);
						bufprintf(&transKernel.text, "%s twiddle_factor;\n", bufcstr(&dtComplex));
					} while (0);

					for (size_t i = 0; i < LDS_per_WG; i = i + 256)
					{
						if (i + 256 < LDS_per_WG)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].x = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"- prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].y = "
										"prevValue[idx+%llu].x * twiddle_factor.y "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].x = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].y = "
										"prevValue[idx+%llu].y * twiddle_factor.x "
										"- prevValue[idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
						{
							// need to handle boundary
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
							} while (0);
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].x = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"- prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].y = "
										"prevValue[idx+%llu].x * twiddle_factor.y "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].x = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu].y = "
										"prevValue[idx+%llu].y * twiddle_factor.x "
										"- prevValue[idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "}\n");
							} while (0);
						}
					}
				}
				else if (!twiddleSwapKernelOut) // could be twiddleSwapKernelIn
				{
					for (size_t i = 0; i < LDS_per_WG; i = i + 256)
					{
						// twiddling and callback do not coexist
						if (params.fft_hasPostCallback)
						{
							if (i + 256 < LDS_per_WG)
							{
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"%s(inputA - batch_offset*%llu, "
										"batch_offset*%llu+group_offset+idx+%llu, "
										"post_userdata, prevValue[idx+%llu]);\n",
										params.fft_postCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
										(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (i),
										(unsigned long long) (i));
								} while (0);
							}
							else
							{
								// need to handle boundary
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i),
										(unsigned long long) (LDS_per_WG));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 9);
									bufprintf(&transKernel.text,
										"%s(inputA - batch_offset*%llu, "
										"batch_offset*%llu+group_offset+idx+%llu, "
										"post_userdata, prevValue[idx+%llu]);\n",
										params.fft_postCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
										(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (i),
										(unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "}\n");
								} while (0);
							}
						}
						else
						{
							if (i + 256 < LDS_per_WG)
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu] = "
										"prevValue[idx+%llu];\n",
										(unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							else
							{
								// need to handle boundary
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i),
										(unsigned long long) (LDS_per_WG));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 9);
									bufprintf(&transKernel.text,
										"inputA[group_offset+idx+%llu] = "
										"prevValue[idx+%llu];\n",
										(unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "}\n");
								} while (0);
							}
						}
					}
				}
				break;
			}
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_COMPLEX_PLANAR:
			{
				if (twiddleSwapKernelOut)
				{
					do
					{
						clKernWrite(&transKernel, 6);
						bufprintf(&transKernel.text, "size_t p;\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 6);
						bufprintf(&transKernel.text, "size_t q;\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 6);
						bufprintf(&transKernel.text, "%s twiddle_factor;\n", bufcstr(&dtComplex));
					} while (0);
					for (size_t i = 0; i < LDS_per_WG; i = i + 256)
					{
						if (i + 256 < LDS_per_WG)
						{
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_R[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"- prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_I[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].x * twiddle_factor.y "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_R[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_I[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].y * twiddle_factor.x "
										"- prevValue[idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
						}
						else
						{
							// need to handle boundary
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
							} while (0);
							if (params.fft_N[0] > params.fft_N[1]) // decides whether we have a tall
											       // or wide rectangle
							{
								// input is wide; output is tall
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (smaller_dim));
								} while (0);
							}
							else
							{
								// input is tall; output is wide
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "p = (group_offset+idx+%llu)/%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "q = (group_offset+idx+%llu)%%%llu;\n", (unsigned long long) (i),
										(unsigned long long) (bigger_dim));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "twiddle_factor = TW3step(p*q);\n");
							} while (0);
							if (fwd)
							{
								// forward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_R[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"- prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_I[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].x * twiddle_factor.y "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.x;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// backward
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_R[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].x * twiddle_factor.x "
										"+ prevValue[idx+%llu].y * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"inputA_I[group_offset+idx+%llu] = "
										"prevValue[idx+%llu].y * twiddle_factor.x "
										"- prevValue[idx+%llu].x * "
										"twiddle_factor.y;\n",
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "}\n");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 3);
							bufstream_endline(&transKernel);
						} while (0);
					}
				}
				else if (!twiddleSwapKernelOut) // could be twiddleSwapKernelIn
				{
					for (size_t i = 0; i < LDS_per_WG; i = i + 256)
					{
						// twiddling and callback do not coexist
						if (params.fft_hasPostCallback)
						{
							if (i + 256 < LDS_per_WG)
							{
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"%s(inputA_R - batch_offset*%llu, inputA_I "
										"- batch_offset*%llu, "
										"batch_offset*%llu+group_offset+idx+%llu, "
										"post_userdata, prevValue[idx+%llu].x, "
										"prevValue[idx+%llu].y);\n",
										params.fft_postCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
										(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (smaller_dim * bigger_dim),
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
							}
							else
							{
								// need to handle boundary
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i),
										(unsigned long long) (LDS_per_WG));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text,
										"%s(inputA_R - batch_offset*%llu, inputA_I "
										"- batch_offset*%llu, "
										"batch_offset*%llu+group_offset+idx+%llu, "
										"post_userdata, prevValue[idx+%llu].x, "
										"prevValue[idx+%llu].y);\n",
										params.fft_postCallback.funcname, (unsigned long long) (smaller_dim * bigger_dim),
										(unsigned long long) (smaller_dim * bigger_dim), (unsigned long long) (smaller_dim * bigger_dim),
										(unsigned long long) (i), (unsigned long long) (i), (unsigned long long) (i));
								} while (0);
								do
								{
									clKernWrite(&transKernel, 6);
									bufprintf(&transKernel.text, "}\n");
								} while (0);
							}
						}
						else if (i + 256 < LDS_per_WG)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"inputA_R[group_offset+idx+%llu] = "
									"prevValue[idx+%llu].x;\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"inputA_I[group_offset+idx+%llu] = "
									"prevValue[idx+%llu].y;\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
						}
						else
						{
							// need to handle boundary
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"inputA_R[group_offset+idx+%llu] = "
									"prevValue[idx+%llu].x;\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"inputA_I[group_offset+idx+%llu] = "
									"prevValue[idx+%llu].y;\n",
									(unsigned long long) (i), (unsigned long long) (i));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text, "}\n");
							} while (0);
						}
					}
				}
				break;
			}
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}
		do
		{
			clKernWrite(&transKernel, 6);
			bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL:
			case CLFFT_COMPLEX_INTERLEAVED:
			case CLFFT_COMPLEX_PLANAR:
			{
				for (size_t i = 0; i < LDS_per_WG; i = i + 256)
				{
					if (i + 256 < LDS_per_WG)
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "prevValue[idx+%llu] = nextValue[idx+%llu];\n", (unsigned long long) (i),
								(unsigned long long) (i));
						} while (0);
					else
					{
						// need to handle boundary
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "if(idx+%llu<%llu){\n", (unsigned long long) (i), (unsigned long long) (LDS_per_WG));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "prevValue[idx + %llu] = nextValue[idx + %llu]; \n", (unsigned long long) (i),
								(unsigned long long) (i));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "}\n");
						} while (0);
					}
				}
				break;
			}
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

		do
		{
			clKernWrite(&transKernel, 6);
			bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "prev = next;\n");
		} while (0);
		if (WG_per_line == 1)
			do
			{
				clKernWrite(&transKernel, 3);
				bufstream_cat_cstr(&transKernel, "}while(next!=swap_table[group_id][0]);");
				bufstream_endline(&transKernel); // end of do-while
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}while(next!=swap_table[group_id/%llu][0]);", (unsigned long long) (WG_per_line));
				bufstream_endline(&transKernel); // end of do-while
			} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_cat_cstr(&transKernel, "}");
			bufstream_endline(&transKernel); // end of kernel
		} while (0);

		if (!twiddleSwapKernel)
			break; // break for bothDir only need one kernel if twiddle is not
			       // done here

	} // end of for (size_t bothDir = 0; bothDir < 2; bothDir++)

	// by now the kernel string is generated
	//  Release permutation table after generated swap source has been emitted.
	array_free_size_t_array(&permutationTable);
	// Copy generated source into the caller-owned buffer explicitly.
	bufsetbuf(strKernel, &transKernel.text);
	return CLFFT_SUCCESS;
}

// generate transepose kernel with sqaure 2d matrix of row major with arbitrary
// batch size
/*
Below is a matrix(row major) containing three sqaure sub matrix along column
The transpose will be done within each sub matrix.
[M0
 M1
 M2]
*/
static clfftStatus clfft_transpose_generator_genTransposeKernelBatched(const FFTKernelGenKeyParams params, buffer_t *strKernel, size_t lwSize, const size_t reShapeFactor)
{
	bufreserve(strKernel, 4096);
	buffer_stream_t transKernel;
	// Initialize stream formatting state explicitly.
	bufstream_init(&transKernel);
	// These strings represent the various data types we read or write in the
	// kernel, depending on how the plan is configured
	buffer_t dtInput = buffer_empty();   // The type read as input into kernel
	buffer_t dtOutput = buffer_empty();  // The type written as output from kernel
	buffer_t dtPlanar = buffer_empty();  // Fundamental type for planar arrays
	buffer_t dtComplex = buffer_empty(); // Fundamental type for complex arrays

	// NOTE:  Enable only for debug

	// if (params.fft_inputLayout != params.fft_outputLayout)
	//	return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	switch (params.fft_precision)
	{
		case CLFFT_SINGLE:
		case CLFFT_SINGLE_FAST:
			bufsetcstr(&dtPlanar, "float");
			bufsetcstr(&dtComplex, "float2");
			break;
		case CLFFT_DOUBLE:
		case CLFFT_DOUBLE_FAST:
			bufsetcstr(&dtPlanar, "double");
			bufsetcstr(&dtComplex, "double2");

			// Emit code that enables double precision in the kernel
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#ifdef cl_khr_fp64\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#else\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#endif\n\n");
			} while (0);

			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED; break;
	}

	//  it is a better idea to do twiddle in swap kernel if we will have a swap
	//  kernel. for pure square transpose, twiddle will be done in transpose
	//  kernel
	bool twiddleTransposeKernel = params.fft_3StepTwiddle && (params.transposeMiniBatchSize == 1); // when transposeMiniBatchSize == 1 it is guaranteed to be a sqaure
												       // matrix transpose
	//	If twiddle computation has been requested, generate the lookup function

	if (twiddleTransposeKernel)
	{
		buffer_t str = buffer_empty();
		TwiddleTableLarge twLarge;
		// Initialize large twiddle table storage explicitly.
		TwiddleTableLargeInit(&twLarge, params.fft_N[0] * params.fft_N[1]);
		if ((params.fft_precision == CLFFT_SINGLE) || (params.fft_precision == CLFFT_SINGLE_FAST))
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_SINGLE, &str);
		else
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_DOUBLE, &str);
		// Release large twiddle table storage after emitting source.
		TwiddleTableLargeFree(&twLarge);
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", bufcstr(&str));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	// This detects whether the input matrix is square
	bool notSquare = (params.fft_N[0] == params.fft_N[1]) ? false : true;

	if (notSquare && (params.fft_placeness == CLFFT_INPLACE))
		return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	// This detects whether the input matrix is a multiple of 16*reshapefactor
	// or not

	bool mult_of_16 = (params.fft_N[0] % (reShapeFactor * 16) == 0) ? true : false;

	for (size_t bothDir = 0; bothDir < 2; bothDir++)
	{
		bool fwd = bothDir ? false : true;

		// If pre-callback is set for the plan
		if (params.fft_hasPreCallback)
		{
			// Insert callback function code at the beginning
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "%s\n", params.fft_preCallback.funcstring);
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufstream_endline(&transKernel);
			} while (0);
		}
		// If post-callback is set for the plan
		if (params.fft_hasPostCallback)
		{
			// Insert callback function code at the beginning
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "%s\n", params.fft_postCallback.funcstring);
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufstream_endline(&transKernel);
			} while (0);
		}

		buffer_t funcName = buffer_empty();
		if (twiddleTransposeKernel) // it makes more sense to do twiddling in
					    // swap kernel
			bufsetcstr(&funcName, fwd ? "transpose_square_tw_fwd" : "transpose_square_tw_back");
		else
			bufsetcstr(&funcName, "transpose_square");

		// Generate kernel API
		clfft_transpose_generator_genTransposePrototype(&params, lwSize, &dtPlanar, &dtComplex, &funcName, &transKernel, &dtInput, &dtOutput);
		size_t wgPerBatch;
		if (mult_of_16)
			wgPerBatch = (params.fft_N[0] / 16 / reShapeFactor) * (params.fft_N[0] / 16 / reShapeFactor + 1) / 2;
		else
			wgPerBatch = (params.fft_N[0] / (16 * reShapeFactor) + 1) * (params.fft_N[0] / (16 * reShapeFactor) + 1 + 1) / 2;
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t numGroupsY_1 = %llu;\n", (unsigned long long) (wgPerBatch));
		} while (0);

		for (size_t i = 2; i < params.fft_DataDim - 1; i++)
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "const size_t numGroupsY_%llu = numGroupsY_%llu * %llu;\n", (unsigned long long) (i), (unsigned long long) (i - 1),
					(unsigned long long) (params.fft_N[i]));
			} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t g_index;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		clfft_transpose_generator_OffsetCalc(&transKernel, &params, true);

		if (params.fft_placeness == CLFFT_OUTOFPLACE)
			clfft_transpose_generator_OffsetCalc(&transKernel, &params, false);

		// Handle planar and interleaved right here
		switch (params.fft_inputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
				// Do not advance offset when precallback is set as the starting
				// address of global buffer is needed
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "inputA += iOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
				}
				break;
			case CLFFT_COMPLEX_PLANAR:
				// Do not advance offset when precallback is set as the starting
				// address of global buffer is needed
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "inputA_R += iOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "inputA_I += iOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
				}
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_REAL: break;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

		if (params.fft_placeness == CLFFT_OUTOFPLACE)
		{
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "outputA += oOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);

					break;
				case CLFFT_COMPLEX_PLANAR:

					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "outputA_R += oOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "outputA_I += oOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}
		}
		else
		{
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPreCallback)
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "global %s *outputA = inputA + iOffset;\n", bufcstr(&dtInput));
						} while (0);
					else
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "global %s *outputA = inputA;\n", bufcstr(&dtInput));
						} while (0);
					break;
				case CLFFT_COMPLEX_PLANAR:
					if (params.fft_hasPreCallback)
					{
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "global %s *outputA_R = inputA_R + iOffset;\n", bufcstr(&dtInput));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "global %s *outputA_I = inputA_I + iOffset;\n", bufcstr(&dtInput));
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "global %s *outputA_R = inputA_R;\n", bufcstr(&dtInput));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "global %s *outputA_I = inputA_I;\n", bufcstr(&dtInput));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		// Now compute the corresponding y,x coordinates
		// for a triangular indexing
		if (mult_of_16)
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"float row = (%llu+sqrt((%llu-8.0f*g_index- 7)))/ "
					"(-2.0f);\n",
					(unsigned long long) (-2.0f * params.fft_N[0] / 16 / reShapeFactor - 1),
					(unsigned long long) (4.0f * params.fft_N[0] / 16 / reShapeFactor * (params.fft_N[0] / 16 / reShapeFactor + 1)));
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"float row = (%llu+sqrt((%llu-8.0f*g_index- 7)))/ "
					"(-2.0f);\n",
					(unsigned long long) (-2.0f * (params.fft_N[0] / (16 * reShapeFactor) + 1) - 1),
					(unsigned long long) (4.0f * (params.fft_N[0] / (16 * reShapeFactor) + 1) * (params.fft_N[0] / (16 * reShapeFactor) + 1 + 1)));
			} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "if (row == (float)(size_t)row) row -= 1; \n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t t_gy = (size_t)row;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		if (mult_of_16)
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"const long t_gx_p = g_index - %llu*t_gy + "
					"t_gy*(t_gy + 1) / 2;\n",
					(unsigned long long) ((params.fft_N[0] / 16 / reShapeFactor)));
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"const long t_gx_p = g_index - %llu*t_gy + "
					"t_gy*(t_gy + 1) / 2;\n",
					(unsigned long long) ((params.fft_N[0] / (16 * reShapeFactor) + 1)));
			} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const long t_gy_p = t_gx_p - t_gy;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t d_lidx = get_local_id(0) %% 16;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t d_lidy = get_local_id(0) / 16;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t lidy = (d_lidy * 16 + d_lidx) /%llu;\n", (unsigned long long) ((16 * reShapeFactor)));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t lidx = (d_lidy * 16 + d_lidx) %%%llu;\n", (unsigned long long) ((16 * reShapeFactor)));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t idx = lidx + t_gx_p*%llu;\n", (unsigned long long) (16 * reShapeFactor));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t idy = lidy + t_gy_p*%llu;\n", (unsigned long long) (16 * reShapeFactor));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t starting_index_yx = t_gy_p*%llu + t_gx_p*%llu;\n", (unsigned long long) (16 * reShapeFactor),
				(unsigned long long) (16 * reShapeFactor * params.fft_N[0]));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "__local %s xy_s[%llu];\n", bufcstr(&dtComplex), (unsigned long long) (16 * reShapeFactor * 16 * reShapeFactor));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "__local %s yx_s[%llu];\n", bufcstr(&dtComplex), (unsigned long long) (16 * reShapeFactor * 16 * reShapeFactor));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "%s tmpm, tmpt;\n", bufcstr(&dtComplex));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		// Step 1: Load both blocks into local memory
		// Here I load inputA for both blocks contiguously and write it
		// contigously into the corresponding shared memories. Afterwards I use
		// non-contiguous access from local memory and write contiguously back
		// into the arrays

		if (mult_of_16)
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "size_t index;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "index = lidy*%llu + lidx + loop*256;\n", (unsigned long long) (16 * reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + loop * "
									"%llu)*%llu + idx, pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + loop * "
									"%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + loop "
									"* %llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop * %llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "tmpm = inputA[(idy + loop *%llu)*%llu + idx];\n", (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpt = inputA[(lidy + loop *%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
				}
				break;
				case CLFFT_COMPLEX_PLANAR:
					bufsetbuf(&dtInput, &dtPlanar);
					bufsetbuf(&dtOutput, &dtPlanar);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + "
									"(idy + loop *%llu)*%llu + idx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + (lidy "
									"+ loop *%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + (idy "
									"+ loop *%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + "
									"(lidy + loop *%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpm.x = inputA_R[(idy + loop *%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpm.y = inputA_I[(idy + loop *%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpt.x = inputA_R[(lidy + loop *%llu)*%llu "
								"+ lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpt.y = inputA_I[(lidy + loop *%llu)*%llu "
								"+ lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			// it makes more sense to do twiddling in swap kernel
			// If requested, generate the Twiddle math to multiply constant
			// values
			if (twiddleTransposeKernel)
				clfft_transpose_generator_genTwiddleMath(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "xy_s[index] = tmpm; \n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "yx_s[index] = tmpt; \n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);

			// Step2: Write from shared to global
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "index = lidx*%llu + lidy + %llu*loop;\n", (unsigned long long) (16 * reShapeFactor),
					(unsigned long long) (16 / reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPostCallback)
					{
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA, ((idy + loop*%llu)*%llu + "
									"idx), post_userdata, yx_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
						{
							// assume tranpose is only two dimensional for now
							// size_t actualBatchSize = params.transposeBatchSize /
							// params.transposeMiniBatchSize;
							size_t blockOffset = params.fft_inStride[2];
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA-%llu*((get_group_id(0)/"
									"numGroupsY_1)%%%llu), ((idy + loop*%llu)*%llu "
									"+ idx + %llu*( (get_group_id(0)/numGroupsY_1 "
									")%%%llu) ), post_userdata, yx_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]), (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize));
							} while (0);
						}
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						if (params.transposeMiniBatchSize < 2)
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA, ((lidy + loop*%llu)*%llu + "
									"lidx + starting_index_yx), "
									"post_userdata, xy_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
						{
							size_t blockOffset = params.fft_inStride[2];
							// clKernWrite(&transKernel, 6) <<
							// params.fft_postCallback.funcname <<
							// "(outputA-iOffset, ((lidy + loop*" << 16 /
							// reShapeFactor << ")*" << params.fft_N[0] << " + lidx
							// + starting_index_yx +iOffset), post_userdata,
							// xy_s[index]";
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA-%llu*((get_group_id(0)/"
									"numGroupsY_1)%%%llu), ((lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx + "
									"%llu*( (get_group_id(0)/numGroupsY_1 )%%%llu) "
									"), post_userdata, xy_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]), (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize));
							} while (0);
						}
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA[(lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx] = xy_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					if (params.fft_hasPostCallback)
					{
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA_R, outputA_I, ((idy + "
									"loop*%llu)*%llu + idx), post_userdata, "
									"yx_s[index].x, yx_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
						{
							size_t blockOffset = params.fft_inStride[2];
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA_R - "
									"%llu*((get_group_id(0)/numGroupsY_1)%%%llu), "
									"outputA_I "
									"-%llu*((get_group_id(0)/numGroupsY_1)%%%llu), "
									"((idy + loop*%llu)*%llu + idx "
									"+%llu*((get_group_id(0)/"
									"numGroupsY_1)%%%llu)), post_userdata, "
									"yx_s[index].x, yx_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize), (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]), (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize));
							} while (0);
						}
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA_R, outputA_I, ((lidy + "
									"loop*%llu)*%llu + lidx+ starting_index_yx), "
									"post_userdata, xy_s[index].x, xy_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
						{
							size_t blockOffset = params.fft_inStride[2];
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"%s(outputA_R - "
									"%llu*((get_group_id(0)/numGroupsY_1)%%%llu), "
									"outputA_I "
									"-%llu*((get_group_id(0)/numGroupsY_1)%%%llu), "
									"((lidy + loop*%llu)*%llu + lidx+ "
									"starting_index_yx "
									"+%llu*((get_group_id(0)/"
									"numGroupsY_1)%%%llu)), post_userdata, "
									"xy_s[index].x, xy_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize), (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]), (unsigned long long) (blockOffset),
									(unsigned long long) (params.transposeMiniBatchSize));
							} while (0);
						}
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_R[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_I[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].y;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_R[(lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx] = xy_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_I[(lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx] = xy_s[index].y;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
		}
		else
		{ // mult_of_16

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "size_t index;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "if (%llu - (t_gx_p + 1) *%llu>0){\n", (unsigned long long) (params.fft_N[0]),
					(unsigned long long) (16 * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidy*%llu + lidx + loop*256;\n", (unsigned long long) (16 * reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata, "
									"localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmpm = inputA[(idy + loop*%llu)*%llu + idx];\n", (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpt = inputA[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					bufsetbuf(&dtInput, &dtPlanar);
					bufsetbuf(&dtOutput, &dtPlanar);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + "
									"(idy + loop*%llu)*%llu + idx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + (lidy "
									"+ loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + (idy "
									"+ loop*%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + "
									"(lidy + loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpm.x = inputA_R[(idy + loop*%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpm.y = inputA_I[(idy + loop*%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpt.x = inputA_R[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpt.y = inputA_I[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			// it makes more sense to do twiddling in swap kernel
			// If requested, generate the Twiddle math to multiply constant
			// values
			if (twiddleTransposeKernel)
				clfft_transpose_generator_genTwiddleMath(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "xy_s[index] = tmpm;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "yx_s[index] = tmpt;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "else{\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidy*%llu + lidx + loop*256;\n", (unsigned long long) (16 * reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu&& idx<%llu)\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (params.fft_N[0]), (unsigned long long) (params.fft_N[0]));
					} while (0);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata, "
									"localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) \n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) \n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text, "tmpm = inputA[(idy + loop*%llu)*%llu + idx];\n", (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
								"%llu + lidy + loop*%llu)<%llu) \n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpt = inputA[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					bufsetbuf(&dtInput, &dtPlanar);
					bufsetbuf(&dtOutput, &dtPlanar);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu&& idx<%llu) {\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (params.fft_N[0]), (unsigned long long) (params.fft_N[0]));
					} while (0);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + "
									"(idy + loop*%llu)*%llu + idx, "
									"pre_userdata, localmem); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) {\n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + (lidy "
									"+ loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + (idy "
									"+ loop*%llu)*%llu + idx, pre_userdata); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) {\n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + "
									"(lidy + loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpm.x = inputA_R[(idy + loop*%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpm.y = inputA_I[(idy + loop*%llu)*%llu + "
								"idx]; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
								"%llu + lidy + loop*%llu)<%llu) {\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpt.x = inputA_R[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpt.y = inputA_I[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx]; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			// If requested, generate the Twiddle math to multiply constant
			// values
			if (twiddleTransposeKernel)
				clfft_transpose_generator_genTwiddleMath(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "xy_s[index] = tmpm;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "yx_s[index] = tmpt;\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);

			// Step2: Write from shared to global

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "if (%llu - (t_gx_p + 1) *%llu>0){\n", (unsigned long long) (params.fft_N[0]),
					(unsigned long long) (16 * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidx*%llu + lidy + %llu*loop ;\n", (unsigned long long) (16 * reShapeFactor),
					(unsigned long long) (16 / reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPostCallback)
					{
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA, ((idy + loop*%llu)*%llu + "
									"idx), post_userdata, yx_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA - iOffset, iOffset + ((idy + "
									"loop*%llu)*%llu + idx), post_userdata, "
									"yx_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA, ((lidy + loop*%llu)*%llu + "
									"lidx + starting_index_yx), "
									"post_userdata, xy_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA - iOffset, iOffset + ((lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx), "
									"post_userdata, xy_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index]; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					if (params.fft_hasPostCallback)
					{
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA_R, outputA_I, ((idy + "
									"loop*%llu)*%llu + idx), post_userdata, "
									"yx_s[index].x, yx_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA_R-iOffset, outputA_I-iOffset, "
									"iOffset+((idy + loop*%llu)*%llu + idx), "
									"post_userdata, yx_s[index].x, yx_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA_R, outputA_I, ((lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx), "
									"post_userdata, xy_s[index].x, xy_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"%s(outputA_R-iOffset, outputA_I-iOffset, "
									"iOffset+((lidy + loop*%llu)*%llu + lidx + "
									"starting_index_yx), post_userdata, "
									"xy_s[index].x, xy_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_R[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_I[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].y;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_R[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].x; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_I[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].y; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "else{\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidx*%llu + lidy + %llu*loop;\n", (unsigned long long) (16 * reShapeFactor),
					(unsigned long long) (16 / reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu && idx<%llu)\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (params.fft_N[0]), (unsigned long long) (params.fft_N[0]));
					} while (0);
					if (params.fft_hasPostCallback)
					{
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA, ((idy + loop*%llu)*%llu + "
									"idx), post_userdata, yx_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA - iOffset, iOffset + ((idy + "
									"loop*%llu)*%llu + idx), post_userdata, "
									"yx_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu)\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA, ((lidy + loop*%llu)*%llu + "
									"lidx + starting_index_yx), "
									"post_userdata, xy_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						else
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA - iOffset, iOffset + ((lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx), "
									"post_userdata, xy_s[index]",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);

						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index]; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu)\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu && idx<%llu) {\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (params.fft_N[0]), (unsigned long long) (params.fft_N[0]));
					} while (0);

					if (params.fft_hasPostCallback)
					{
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA_R, outputA_I, ((idy + "
									"loop*%llu)*%llu + idx), post_userdata, "
									"yx_s[index].x, yx_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA_R-iOffset, outputA_I-iOffset, "
									"iOffset+((idy + loop*%llu)*%llu + idx), "
									"post_userdata, yx_s[index].x, yx_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, "); }\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu) {\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.transposeMiniBatchSize < 2) // which means the matrix was not broken down into
										       // sub square matrics
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA_R, outputA_I, ((lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx), "
									"post_userdata, xy_s[index].x, xy_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"%s(outputA_R-iOffset, outputA_I-iOffset, "
									"iOffset+((lidy + loop*%llu)*%llu + lidx + "
									"starting_index_yx), post_userdata, "
									"xy_s[index].x, xy_s[index].y",
									params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, "); }\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_R[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].x; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_I[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].y; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu) {\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (params.fft_N[0]),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_R[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_I[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].y; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}

					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			do
			{
				clKernWrite(&transKernel, 6);
				bufstream_cat_cstr(&transKernel, "}");
				bufstream_endline(&transKernel); // end for
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufstream_cat_cstr(&transKernel, "}");
				bufstream_endline(&transKernel); // end else
			} while (0);
		}
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "}\n");
		} while (0);

		// Copy generated source into the caller-owned buffer explicitly.
		bufsetbuf(strKernel, &transKernel.text);

		if (!twiddleTransposeKernel)
			break; // break for bothDir
	}

	return CLFFT_SUCCESS;
}

// generate transpose kernel with square 2d matrix of row major with blocks
// along the leading dimension aka leading dimension batched
/*
Below is a matrix(row major) contaning three square sub matrix along row
[M0 M2 M2]
*/
static clfftStatus clfft_transpose_generator_genTransposeKernelLeadingDimensionBatched(const FFTKernelGenKeyParams params, buffer_t *strKernel, size_t lwSize,
	const size_t reShapeFactor)
{
	bufreserve(strKernel, 4096);
	buffer_stream_t transKernel;
	// Initialize stream formatting state explicitly.
	bufstream_init(&transKernel);
	// These strings represent the various data types we read or write in the
	// kernel, depending on how the plan is configured
	buffer_t dtInput = buffer_empty();   // The type read as input into kernel
	buffer_t dtOutput = buffer_empty();  // The type written as output from kernel
	buffer_t dtPlanar = buffer_empty();  // Fundamental type for planar arrays
	buffer_t dtComplex = buffer_empty(); // Fundamental type for complex arrays

	// NOTE:  Enable only for debug

	// if (params.fft_inputLayout != params.fft_outputLayout)
	//	return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	switch (params.fft_precision)
	{
		case CLFFT_SINGLE:
		case CLFFT_SINGLE_FAST:
			bufsetcstr(&dtPlanar, "float");
			bufsetcstr(&dtComplex, "float2");
			break;
		case CLFFT_DOUBLE:
		case CLFFT_DOUBLE_FAST:
			bufsetcstr(&dtPlanar, "double");
			bufsetcstr(&dtComplex, "double2");

			// Emit code that enables double precision in the kernel
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#ifdef cl_khr_fp64\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#else\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#endif\n\n");
			} while (0);

			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED; break;
	}

	//	If twiddle computation has been requested, generate the lookup function
	if (params.fft_3StepTwiddle)
	{
		buffer_t str = buffer_empty();
		TwiddleTableLarge twLarge;
		// Initialize large twiddle table storage explicitly.
		TwiddleTableLargeInit(&twLarge, params.fft_N[0] * params.fft_N[1]);
		if ((params.fft_precision == CLFFT_SINGLE) || (params.fft_precision == CLFFT_SINGLE_FAST))
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_SINGLE, &str);
		else
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_DOUBLE, &str);
		// Release large twiddle table storage after emitting source.
		TwiddleTableLargeFree(&twLarge);
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", bufcstr(&str));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	size_t smaller_dim = (params.fft_N[0] < params.fft_N[1]) ? params.fft_N[0] : params.fft_N[1];
	size_t bigger_dim = (params.fft_N[0] >= params.fft_N[1]) ? params.fft_N[0] : params.fft_N[1];
	size_t dim_ratio = bigger_dim / smaller_dim;

	// This detects whether the input matrix is rectangle of ratio 1:2

	if ((params.fft_N[0] != 2 * params.fft_N[1]) && (params.fft_N[1] != 2 * params.fft_N[0]) && (params.fft_N[0] != 3 * params.fft_N[1]) &&
		(params.fft_N[1] != 3 * params.fft_N[0]) && (params.fft_N[0] != 5 * params.fft_N[1]) && (params.fft_N[1] != 5 * params.fft_N[0]) &&
		(params.fft_N[0] != 10 * params.fft_N[1]) && (params.fft_N[1] != 10 * params.fft_N[0]))
	{
		return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}

	if (params.fft_placeness == CLFFT_OUTOFPLACE)
		return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	// This detects whether the input matrix is a multiple of 16*reshapefactor
	// or not

	bool mult_of_16 = (smaller_dim % (reShapeFactor * 16) == 0) ? true : false;

	for (size_t bothDir = 0; bothDir < 2; bothDir++)
	{
		bool fwd = bothDir ? false : true;

		// If pre-callback is set for the plan
		if (params.fft_hasPreCallback)
		{
			// Insert callback function code at the beginning
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "%s\n", params.fft_preCallback.funcstring);
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufstream_endline(&transKernel);
			} while (0);
		}
		// If post-callback is set for the plan
		if (params.fft_hasPostCallback)
		{
			// Insert callback function code at the beginning
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "%s\n", params.fft_postCallback.funcstring);
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufstream_endline(&transKernel);
			} while (0);
		}

		buffer_t funcName = buffer_empty();
		if (params.fft_3StepTwiddle) // TODO
			bufsetcstr(&funcName, fwd ? "transpose_nonsquare_tw_fwd" : "transpose_nonsquare_tw_back");
		else
			bufsetcstr(&funcName, "transpose_nonsquare");

		// Generate kernel API
		clfft_transpose_generator_genTransposePrototypeLeadingDimensionBatched(&params, lwSize, &dtPlanar, &dtComplex, &funcName, &transKernel, &dtInput, &dtOutput);

		if (mult_of_16) // number of WG per sub square block
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "const size_t  numGroups_square_matrix_Y_1 = %llu;\n",
					(unsigned long long) ((smaller_dim / 16 / reShapeFactor) * (smaller_dim / 16 / reShapeFactor + 1) / 2));
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "const size_t  numGroups_square_matrix_Y_1 = %llu;\n",
					(unsigned long long) ((smaller_dim / (16 * reShapeFactor) + 1) * (smaller_dim / (16 * reShapeFactor) + 1 + 1) / 2));
			} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"const size_t  numGroupsY_1 =  "
				"numGroups_square_matrix_Y_1 * %llu;\n",
				(unsigned long long) (dim_ratio));
		} while (0);

		for (size_t i = 2; i < params.fft_DataDim - 1; i++)
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "const size_t numGroupsY_%llu = numGroupsY_%llu * %llu;\n", (unsigned long long) (i), (unsigned long long) (i - 1),
					(unsigned long long) (params.fft_N[i]));
			} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t g_index;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t square_matrix_index;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t square_matrix_offset;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		clfft_transpose_generator_OffsetCalcLeadingDimensionBatched(&transKernel, &params);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"square_matrix_index = (g_index / "
				"numGroups_square_matrix_Y_1) ;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "g_index = g_index %% numGroups_square_matrix_Y_1;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		if (smaller_dim == params.fft_N[1])
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "square_matrix_offset = square_matrix_index * %llu;\n", (unsigned long long) (smaller_dim));
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "square_matrix_offset = square_matrix_index *%llu;\n", (unsigned long long) (smaller_dim * smaller_dim));
			} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "iOffset += square_matrix_offset ;\n");
		} while (0);

		// Handle planar and interleaved right here
		switch (params.fft_inputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
			case CLFFT_REAL:
				// Do not advance offset when precallback is set as the starting
				// address of global buffer is needed
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "inputA += iOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
				}
				break;
			case CLFFT_COMPLEX_PLANAR:
				// Do not advance offset when precallback is set as the starting
				// address of global buffer is needed
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "inputA_R += iOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufstream_cat_cstr(&transKernel, "inputA_I += iOffset;");
						bufstream_endline(&transKernel); // Set A ptr to the start of each slice
					} while (0);
				}
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

		switch (params.fft_inputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
			case CLFFT_REAL:
				if (params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s *outputA = inputA + iOffset;\n", bufcstr(&dtInput));
					} while (0);
				}
				else
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s *outputA = inputA;\n", bufcstr(&dtInput));
					} while (0);
				}
				break;
			case CLFFT_COMPLEX_PLANAR:
				if (params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s *outputA_R = inputA_R + iOffset;\n", bufcstr(&dtInput));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s *outputA_I = inputA_I + iOffset;\n", bufcstr(&dtInput));
					} while (0);
				}
				else
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s *outputA_R = inputA_R;\n", bufcstr(&dtInput));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s *outputA_I = inputA_I;\n", bufcstr(&dtInput));
					} while (0);
				}
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		// Now compute the corresponding y,x coordinates
		// for a triangular indexing
		if (mult_of_16)
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"float row = (%llu+sqrt((%llu-8.0f*g_index- 7)))/ "
					"(-2.0f);\n",
					(unsigned long long) (-2.0f * smaller_dim / 16 / reShapeFactor - 1),
					(unsigned long long) (4.0f * smaller_dim / 16 / reShapeFactor * (smaller_dim / 16 / reShapeFactor + 1)));
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"float row = (%llu+sqrt((%llu-8.0f*g_index- 7)))/ "
					"(-2.0f);\n",
					(unsigned long long) (-2.0f * (smaller_dim / (16 * reShapeFactor) + 1) - 1),
					(unsigned long long) (4.0f * (smaller_dim / (16 * reShapeFactor) + 1) * (smaller_dim / (16 * reShapeFactor) + 1 + 1)));
			} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "if (row == (float)(int)row) row -= 1; \n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t t_gy = (int)row;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		if (mult_of_16)
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"const long t_gx_p = g_index - %llu*t_gy + t_gy*(t_gy + 1) "
					"/ 2;\n",
					(unsigned long long) ((smaller_dim / 16 / reShapeFactor)));
			} while (0);
		else
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text,
					"const long t_gx_p = g_index - %llu*t_gy + "
					"t_gy*(t_gy + 1) / 2;\n",
					(unsigned long long) ((smaller_dim / (16 * reShapeFactor) + 1)));
			} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const long t_gy_p = t_gx_p - t_gy;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t d_lidx = get_local_id(0) %% 16;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t d_lidy = get_local_id(0) / 16;\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t lidy = (d_lidy * 16 + d_lidx) /%llu;\n", (unsigned long long) ((16 * reShapeFactor)));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t lidx = (d_lidy * 16 + d_lidx) %%%llu;\n", (unsigned long long) ((16 * reShapeFactor)));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t idx = lidx + t_gx_p*%llu;\n", (unsigned long long) (16 * reShapeFactor));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t idy = lidy + t_gy_p*%llu;\n", (unsigned long long) (16 * reShapeFactor));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t starting_index_yx = t_gy_p*%llu + t_gx_p*%llu;\n", (unsigned long long) (16 * reShapeFactor),
				(unsigned long long) (16 * reShapeFactor * params.fft_N[0]));
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		switch (params.fft_inputLayout)
		{
			case CLFFT_REAL:
			case CLFFT_COMPLEX_INTERLEAVED:
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s xy_s[%llu];\n", bufcstr(&dtInput), (unsigned long long) (16 * reShapeFactor * 16 * reShapeFactor));
				} while (0);
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s yx_s[%llu];\n", bufcstr(&dtInput), (unsigned long long) (16 * reShapeFactor * 16 * reShapeFactor));
				} while (0);

				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "%s tmpm, tmpt;\n", bufcstr(&dtInput));
				} while (0);
				break;
			case CLFFT_COMPLEX_PLANAR:
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s xy_s[%llu];\n", bufcstr(&dtComplex),
						(unsigned long long) (16 * reShapeFactor * 16 * reShapeFactor));
				} while (0);
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "__local %s yx_s[%llu];\n", bufcstr(&dtComplex),
						(unsigned long long) (16 * reShapeFactor * 16 * reShapeFactor));
				} while (0);

				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "%s tmpm, tmpt;\n", bufcstr(&dtComplex));
				} while (0);
				break;
			default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		}
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n");
		} while (0);

		// Step 1: Load both blocks into local memory
		// Here I load inputA for both blocks contiguously and write it
		// contigously into the corresponding shared memories. Afterwards I use
		// non-contiguous access from local memory and write contiguously back
		// into the arrays

		if (mult_of_16)
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "size_t index;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "index = lidy*%llu + lidx + loop*256;\n", (unsigned long long) (16 * reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				case CLFFT_REAL:
				{
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + loop * "
									"%llu)*%llu + idx, pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + loop * "
									"%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + loop "
									"* %llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop * %llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text, "tmpm = inputA[(idy + loop *%llu)*%llu + idx];\n", (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpt = inputA[(lidy + loop *%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
				}
				break;
				case CLFFT_COMPLEX_PLANAR:
					bufsetbuf(&dtInput, &dtPlanar);
					bufsetbuf(&dtOutput, &dtPlanar);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + "
									"(idy + loop *%llu)*%llu + idx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + (lidy "
									"+ loop *%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + (idy "
									"+ loop *%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 6);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + "
									"(lidy + loop *%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpm.x = inputA_R[(idy + loop *%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpm.y = inputA_I[(idy + loop *%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpt.x = inputA_R[(lidy + loop *%llu)*%llu "
								"+ lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"tmpt.y = inputA_I[(lidy + loop *%llu)*%llu "
								"+ lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			// If requested, generate the Twiddle math to multiply constant
			// values
			if (params.fft_3StepTwiddle)
				clfft_transpose_generator_genTwiddleMathLeadingDimensionBatched(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "xy_s[index] = tmpm; \n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "yx_s[index] = tmpt; \n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);

			// Step2: Write from shared to global
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "index = lidx*%llu + lidy + %llu*loop;\n", (unsigned long long) (16 * reShapeFactor),
					(unsigned long long) (16 / reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"%s(outputA, ((idy + loop*%llu)*%llu + idx), "
								"post_userdata, yx_s[index]",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"%s(outputA, ((lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx), post_userdata, xy_s[index]",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA[(lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx] = xy_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}

					break;
				case CLFFT_COMPLEX_PLANAR:
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"%s(outputA_R, outputA_I, ((idy + "
								"loop*%llu)*%llu + idx), post_userdata, "
								"yx_s[index].x, yx_s[index].y",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"%s(outputA_R, outputA_I, ((lidy + "
								"loop*%llu)*%llu + lidx+ starting_index_yx), "
								"post_userdata, xy_s[index].x, xy_s[index].y",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_R[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_I[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].y;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_R[(lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx] = xy_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 6);
							bufprintf(&transKernel.text,
								"outputA_I[(lidy + loop*%llu)*%llu + lidx+ "
								"starting_index_yx] = xy_s[index].y;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "size_t index;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "if (%llu - (t_gx_p + 1) *%llu>0){\n", (unsigned long long) (smaller_dim), (unsigned long long) (16 * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidy*%llu + lidx + loop*256;\n", (unsigned long long) (16 * reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				case CLFFT_REAL:
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata, "
									"localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmpm = inputA[(idy + loop*%llu)*%llu + idx];\n", (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpt = inputA[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					bufsetbuf(&dtInput, &dtPlanar);
					bufsetbuf(&dtOutput, &dtPlanar);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + "
									"(idy + loop*%llu)*%llu + idx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + (lidy "
									"+ loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + (idy "
									"+ loop*%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + "
									"(lidy + loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpm.x = inputA_R[(idy + loop*%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpm.y = inputA_I[(idy + loop*%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpt.x = inputA_R[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"tmpt.y = inputA_I[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			// If requested, generate the Twiddle math to multiply constant
			// values
			if (params.fft_3StepTwiddle)
				clfft_transpose_generator_genTwiddleMathLeadingDimensionBatched(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "xy_s[index] = tmpm;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "yx_s[index] = tmpt;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "else{\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidy*%llu + lidx + loop*256;\n", (unsigned long long) (16 * reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				case CLFFT_REAL:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu&& idx<%llu)\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (smaller_dim), (unsigned long long) (smaller_dim));
					} while (0);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata, "
									"localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) \n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (smaller_dim));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA, iOffset + (idy + "
									"loop*%llu)*%llu + idx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) \n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (smaller_dim));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA, iOffset + (lidy + "
									"loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata);\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text, "tmpm = inputA[(idy + loop*%llu)*%llu + idx];\n", (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
								"%llu + lidy + loop*%llu)<%llu) \n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (smaller_dim));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpt = inputA[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					bufsetbuf(&dtInput, &dtPlanar);
					bufsetbuf(&dtOutput, &dtPlanar);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu&& idx<%llu) {\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (smaller_dim), (unsigned long long) (smaller_dim));
					} while (0);
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + "
									"(idy + loop*%llu)*%llu + idx, "
									"pre_userdata, localmem); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) {\n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (smaller_dim));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + (lidy "
									"+ loop*%llu)*%llu + lidx + starting_index_yx, "
									"pre_userdata, localmem); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpm = %s(inputA_R, inputA_I, iOffset + (idy "
									"+ loop*%llu)*%llu + idx, pre_userdata); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
									"%llu + lidy + loop*%llu)<%llu) {\n",
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
									(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (smaller_dim));
							} while (0);
							do
							{
								clKernWrite(&transKernel, 12);
								bufprintf(&transKernel.text,
									"tmpt = %s(inputA_R, inputA_I, iOffset + "
									"(lidy + loop*%llu)*%llu + lidx + "
									"starting_index_yx, pre_userdata); }\n",
									params.fft_preCallback.funcname, (unsigned long long) (16 / reShapeFactor),
									(unsigned long long) (params.fft_N[0]));
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpm.x = inputA_R[(idy + loop*%llu)*%llu + "
								"idx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpm.y = inputA_I[(idy + loop*%llu)*%llu + "
								"idx]; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p *%llu + lidx)<%llu && (t_gx_p * "
								"%llu + lidy + loop*%llu)<%llu) {\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (smaller_dim));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpt.x = inputA_R[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"tmpt.y = inputA_I[(lidy + loop*%llu)*%llu + "
								"lidx + starting_index_yx]; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			// If requested, generate the Twiddle math to multiply constant
			// values
			if (params.fft_3StepTwiddle)
				clfft_transpose_generator_genTwiddleMathLeadingDimensionBatched(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "xy_s[index] = tmpm;\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "yx_s[index] = tmpt;\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "barrier(CLK_LOCAL_MEM_FENCE);\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "\n");
			} while (0);

			// Step2: Write from shared to global

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "if (%llu - (t_gx_p + 1) *%llu>0){\n", (unsigned long long) (smaller_dim), (unsigned long long) (16 * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidx*%llu + lidy + %llu*loop ;\n", (unsigned long long) (16 * reShapeFactor),
					(unsigned long long) (16 / reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"%s(outputA, ((idy + loop*%llu)*%llu + idx), "
								"post_userdata, yx_s[index]",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"%s(outputA, ((lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx), post_userdata, xy_s[index]",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index]; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}

					break;
				case CLFFT_COMPLEX_PLANAR:
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"%s(outputA_R, outputA_I, ((idy + "
								"loop*%llu)*%llu + idx), post_userdata, "
								"yx_s[index].x, yx_s[index].y",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"%s(outputA_R, outputA_I, ((lidy + loop*%llu)*%llu "
								"+ lidx + starting_index_yx), post_userdata, "
								"xy_s[index].x, xy_s[index].y",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_R[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_I[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].y;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_R[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].x; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"outputA_I[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].y; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "}\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "else{\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for (size_t loop = 0; loop<%llu; ++loop){\n", (unsigned long long) (reShapeFactor * reShapeFactor));
			} while (0);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "index = lidx*%llu + lidy + %llu*loop;\n", (unsigned long long) (16 * reShapeFactor),
					(unsigned long long) (16 / reShapeFactor));
			} while (0);

			// Handle planar and interleaved right here
			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu && idx<%llu)\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (smaller_dim), (unsigned long long) (smaller_dim));
					} while (0);
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"%s(outputA, ((idy + loop*%llu)*%llu + idx), "
								"post_userdata, yx_s[index]",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu)\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (smaller_dim));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"%s(outputA, ((lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx), post_userdata, xy_s[index]",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index]; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu)\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (smaller_dim));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index];\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if ((idy + loop*%llu)<%llu && idx<%llu) {\n", (unsigned long long) (16 / reShapeFactor),
							(unsigned long long) (smaller_dim), (unsigned long long) (smaller_dim));
					} while (0);

					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"%s(outputA_R, outputA_I, ((idy + "
								"loop*%llu)*%llu + idx), post_userdata, "
								"yx_s[index].x, yx_s[index].y",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, "); }\n");
						} while (0);

						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu) {\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (smaller_dim));
						} while (0);

						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"%s(outputA_R, outputA_I, ((lidy + loop*%llu)*%llu "
								"+ lidx + starting_index_yx), post_userdata, "
								"xy_s[index].x, xy_s[index].y",
								params.fft_postCallback.funcname, (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (params.fft_N[0]));
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, "); }\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_R[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].x; \n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_I[(idy + loop*%llu)*%llu + idx] = "
								"yx_s[index].y; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"if ((t_gy_p * %llu + lidx)<%llu && (t_gx_p "
								"* %llu + lidy + loop*%llu)<%llu) {\n",
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (smaller_dim),
								(unsigned long long) (16 * reShapeFactor), (unsigned long long) (16 / reShapeFactor),
								(unsigned long long) (smaller_dim));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_R[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].x;\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 12);
							bufprintf(&transKernel.text,
								"outputA_I[(lidy + loop*%llu)*%llu + lidx + "
								"starting_index_yx] = xy_s[index].y; }\n",
								(unsigned long long) (16 / reShapeFactor), (unsigned long long) (params.fft_N[0]));
						} while (0);
					}

					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			do
			{
				clKernWrite(&transKernel, 6);
				bufstream_cat_cstr(&transKernel, "}");
				bufstream_endline(&transKernel); // end for
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufstream_cat_cstr(&transKernel, "}");
				bufstream_endline(&transKernel); // end else
			} while (0);
		}
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "}\n");
		} while (0);

		// Copy generated source into the caller-owned buffer explicitly.
		bufsetbuf(strKernel, &transKernel.text);

		if (!params.fft_3StepTwiddle)
			break;
	}

	return CLFFT_SUCCESS;
}

/* End copied source: src\library\generator.transpose.cpp */

#define clKernWrite clfft_amalgamated_gcn_clKernWrite
#define OffsetCalc clfft_amalgamated_gcn_OffsetCalc
#define genTwiddleMath clfft_amalgamated_gcn_genTwiddleMath
#define genTransposePrototype clfft_amalgamated_gcn_genTransposePrototype
#define genTransposeKernel clfft_amalgamated_gcn_genTransposeKernel
#define pmRealIn clfft_amalgamated_gcn_pmRealIn
#define pmImagIn clfft_amalgamated_gcn_pmImagIn
#define pmRealOut clfft_amalgamated_gcn_pmRealOut
#define pmImagOut clfft_amalgamated_gcn_pmImagOut
#define pmComplexIn clfft_amalgamated_gcn_pmComplexIn
#define pmComplexOut clfft_amalgamated_gcn_pmComplexOut
#define lwSize clfft_amalgamated_gcn_lwSize
#define reShapeFactor clfft_amalgamated_gcn_reShapeFactor
/* Begin copied source: src\library\generator.transpose.gcn.cpp */

// clfft.generator.Transpose.cpp : Dynamic run-time generator of openCL
// transpose kernels
//

// TODO: generalize the kernel to work with any size

static FFTGeneratedTransposeGCNAction *FFTGeneratedTransposeGCNActionCreate(clfftPlanHandle plHandle, FFTPlan *plan, cl_command_queue queue, clfftStatus *err)
{
	// Allocate action storage and clear it before explicit initialization.
	FFTGeneratedTransposeGCNAction *action = (FFTGeneratedTransposeGCNAction *) clfft_checked_malloc(sizeof(FFTGeneratedTransposeGCNAction));
	memset((void *) action, 0, sizeof(*action));

	// Initialize the common action base and signature payload.
	FFTActionInit(&action->base, plan, Transpose_GCN, action, err);
	FFTKernelSignatureTransposeInit(&action->signature);
	if (*err != CLFFT_SUCCESS)
	{
		// Report base initialization failure and leave cleanup to the caller.
		fprintf(stderr, "FFTGeneratedTransposeGCNActionCreate: FFTActionInit failed!\n");
		return action;
	}

	// Initialize the FFTAction kernel-generation parameter member.
	*err = FFTGeneratedTransposeGCNActionInitParams(action);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedTransposeGCNActionInitParams failed!\n");
		return action;
	}

	// Generate the kernel source through the shared repository.
	FFTRepo *fftRepo = FFTRepoGetInstance();
	*err = FFTActionGenerateKernel(&action->base, fftRepo, queue);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedTransposeGCNActionCreate: "
			"FFTActionGenerateKernel failed\n");
		return action;
	}

	// Compile generated kernels for the requested queue and plan.
	*err = FFTActionCompileKernels(&action->base, queue, plHandle, plan);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedTransposeGCNActionCreate: "
			"FFTActionCompileKernels failed\n");
		return action;
	}

	// Mark action creation as successful.
	*err = CLFFT_SUCCESS;
	return action;
}

static bool FFTGeneratedTransposeGCNActionBuildForwardKernel(FFTGeneratedTransposeGCNAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || r2c_transform;
}

static bool FFTGeneratedTransposeGCNActionBuildBackwardKernel(FFTGeneratedTransposeGCNAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || c2r_transform;
}

// A structure that represents a bounding box or tile, with convenient names for
// the row and column addresses local work sizes
typedef struct tile
{
	size_t x;
	size_t y;
} tile;

static inline void clKernWrite(buffer_stream_t *rhs, const size_t tabIndex)
{
	// Emit indentation spaces into the kernel source buffer.
	for (size_t i = 0; i < tabIndex; ++i)
		bufstream_cat_char(rhs, ' ');
}

static void OffsetCalc(buffer_stream_t *transKernel, const FFTKernelGenKeyParams *params, bool input)
{
	const size_t *stride = input ? params->fft_inStride : params->fft_outStride;
	buffer_t offset = buffer_from_cstr(input ? "iOffset" : "oOffset");

	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "size_t %s = 0;\n", bufcstr(&offset));
	} while (0);
	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "currDimIndex = groupIndex.y;\n");
	} while (0);

	for (size_t i = params->fft_DataDim - 2; i > 0; i--)
	{
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "%s += (currDimIndex/numGroupsY_%llu)*%llu;\n", bufcstr(&offset), (unsigned long long) (i),
				(unsigned long long) (stride[i + 1]));
		} while (0);
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "currDimIndex = currDimIndex %% numGroupsY_%llu;\n", (unsigned long long) (i));
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 3);
		bufprintf(&transKernel->text, "rowSizeinUnits = %llu;\n", (unsigned long long) (stride[1]));
	} while (0);

	if (params->transOutHorizontal)
	{
		if (input)
		{
			do
			{
				clKernWrite(transKernel, 3);
				bufprintf(&transKernel->text,
					"%s += rowSizeinUnits * wgTileExtent.y * wgUnroll * "
					"groupIndex.x;\n",
					bufcstr(&offset));
			} while (0);
			do
			{
				clKernWrite(transKernel, 3);
				bufprintf(&transKernel->text, "%s += currDimIndex * wgTileExtent.x;\n", bufcstr(&offset));
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 3);
				bufprintf(&transKernel->text, "%s += rowSizeinUnits * wgTileExtent.x * currDimIndex;\n", bufcstr(&offset));
			} while (0);
			do
			{
				clKernWrite(transKernel, 3);
				bufprintf(&transKernel->text, "%s += groupIndex.x * wgTileExtent.y * wgUnroll;\n", bufcstr(&offset));
			} while (0);
		}
	}
	else if (input)
	{
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text,
				"%s += rowSizeinUnits * wgTileExtent.y * wgUnroll * "
				"currDimIndex;\n",
				bufcstr(&offset));
		} while (0);
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "%s += groupIndex.x * wgTileExtent.x;\n", bufcstr(&offset));
		} while (0);
	}
	else
	{
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "%s += rowSizeinUnits * wgTileExtent.x * groupIndex.x;\n", bufcstr(&offset));
		} while (0);
		do
		{
			clKernWrite(transKernel, 3);
			bufprintf(&transKernel->text, "%s += currDimIndex * wgTileExtent.y * wgUnroll;\n", bufcstr(&offset));
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 3);
		bufstream_endline(transKernel);
	} while (0);
}

// Small snippet of code that multiplies the twiddle factors into the
// butterfiles.  It is only emitted if the plan tells the generator that it
// wants the twiddle factors generated inside of the transpose
static clfftStatus genTwiddleMath(const FFTKernelGenKeyParams *params, buffer_stream_t *transKernel, const buffer_t *dtComplex, bool fwd)
{
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text,
			"%s W = TW3step( (groupIndex.x * wgTileExtent.x + xInd) * "
			"(currDimIndex * wgTileExtent.y * wgUnroll + yInd) );\n",
			bufcstr(dtComplex));
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "%s T;\n", bufcstr(dtComplex));
	} while (0);

	if (fwd)
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "T.x = ( W.x * tmp.x ) - ( W.y * tmp.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "T.y = ( W.y * tmp.x ) + ( W.x * tmp.y );\n");
		} while (0);
	}
	else
	{
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "T.x =  ( W.x * tmp.x ) + ( W.y * tmp.y );\n");
		} while (0);
		do
		{
			clKernWrite(transKernel, 9);
			bufprintf(&transKernel->text, "T.y = -( W.y * tmp.x ) + ( W.x * tmp.y );\n");
		} while (0);
	}

	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmp.x = T.x;\n");
	} while (0);
	do
	{
		clKernWrite(transKernel, 9);
		bufprintf(&transKernel->text, "tmp.y = T.y;\n");
	} while (0);

	return CLFFT_SUCCESS;
}

// These strings represent the names that are used as strKernel parameters
static const char pmRealIn[] = "pmRealIn";
static const char pmImagIn[] = "pmImagIn";
static const char pmRealOut[] = "pmRealOut";
static const char pmImagOut[] = "pmImagOut";
static const char pmComplexIn[] = "pmComplexIn";
static const char pmComplexOut[] = "pmComplexOut";

static clfftStatus genTransposePrototype(const FFTKernelGenKeyParams *params, const tile *lwSize, const buffer_t *dtPlanar, const buffer_t *dtComplex, const buffer_t *funcName,
	buffer_stream_t *transKernel, buffer_t *dtInput, buffer_t *dtOutput)
{
	// Declare and define the function
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "__attribute__(( reqd_work_group_size( %llu, %llu, 1 ) ))\n", (unsigned long long) (lwSize->x), (unsigned long long) (lwSize->y));
	} while (0);
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "kernel void\n");
	} while (0);

	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, "%s( ", bufcstr(funcName));
	} while (0);

	switch (params->fft_inputLayout)
	{
		case CLFFT_COMPLEX_INTERLEAVED:
			bufsetbuf(dtInput, dtComplex);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict %s", bufcstr(dtInput), pmComplexIn);
			} while (0);

			switch (params->fft_placeness)
			{
				case CLFFT_INPLACE: bufsetbuf(dtOutput, dtComplex); break;
				case CLFFT_OUTOFPLACE:
					switch (params->fft_outputLayout)
					{
						case CLFFT_COMPLEX_INTERLEAVED:
							bufsetbuf(dtOutput, dtComplex);
							do
							{
								clKernWrite(transKernel, 0);
								bufprintf(&transKernel->text, ", global %s* restrict %s", bufcstr(dtOutput), pmComplexOut);
							} while (0);
							break;
						case CLFFT_COMPLEX_PLANAR:
							bufsetbuf(dtOutput, dtPlanar);
							do
							{
								clKernWrite(transKernel, 0);
								bufprintf(&transKernel->text, ", global %s* restrict %s, global %s* restrict %s", bufcstr(dtOutput), pmRealOut,
									bufcstr(dtOutput), pmImagOut);
							} while (0);
							break;
						case CLFFT_HERMITIAN_INTERLEAVED:
						case CLFFT_HERMITIAN_PLANAR:
						case CLFFT_REAL:
						default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
					}
					break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}
			break;
		case CLFFT_COMPLEX_PLANAR:
			bufsetbuf(dtInput, dtPlanar);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict %s, global %s* restrict %s", bufcstr(dtInput), pmRealIn, bufcstr(dtInput), pmImagIn);
			} while (0);

			switch (params->fft_placeness)
			{
				case CLFFT_INPLACE: bufsetbuf(dtOutput, dtPlanar); break;
				case CLFFT_OUTOFPLACE:
					switch (params->fft_outputLayout)
					{
						case CLFFT_COMPLEX_INTERLEAVED:
							bufsetbuf(dtOutput, dtComplex);
							do
							{
								clKernWrite(transKernel, 0);
								bufprintf(&transKernel->text, ", global %s* restrict %s", bufcstr(dtOutput), pmComplexOut);
							} while (0);
							break;
						case CLFFT_COMPLEX_PLANAR:
							bufsetbuf(dtOutput, dtPlanar);
							do
							{
								clKernWrite(transKernel, 0);
								bufprintf(&transKernel->text, ", global %s* restrict %s, global %s* restrict %s", bufcstr(dtOutput), pmRealOut,
									bufcstr(dtOutput), pmImagOut);
							} while (0);
							break;
						case CLFFT_HERMITIAN_INTERLEAVED:
						case CLFFT_HERMITIAN_PLANAR:
						case CLFFT_REAL:
						default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
					}
					break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}
			break;
		case CLFFT_HERMITIAN_INTERLEAVED:
		case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
		case CLFFT_REAL:
			bufsetbuf(dtInput, dtPlanar);
			do
			{
				clKernWrite(transKernel, 0);
				bufprintf(&transKernel->text, "global %s* restrict %s", bufcstr(dtInput), pmRealIn);
			} while (0);

			switch (params->fft_placeness)
			{
				case CLFFT_INPLACE: bufsetbuf(dtOutput, dtPlanar); break;
				case CLFFT_OUTOFPLACE:
					switch (params->fft_outputLayout)
					{
						case CLFFT_COMPLEX_INTERLEAVED:
						case CLFFT_COMPLEX_PLANAR:
						case CLFFT_HERMITIAN_INTERLEAVED:
						case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
						case CLFFT_REAL:
							bufsetbuf(dtOutput, dtPlanar);
							do
							{
								clKernWrite(transKernel, 0);
								bufprintf(&transKernel->text, ", global %s* restrict %s", bufcstr(dtOutput), pmRealOut);
							} while (0);
							break;
						default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
					}
					break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}
			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
	}

	if (params->fft_hasPreCallback)
	{
		assert(!params->fft_hasPostCallback);

		if (params->fft_preCallback.localMemSize > 0)
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* pre_userdata, __local void* localmem");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* pre_userdata");
			} while (0);
		}
	}

	if (params->fft_hasPostCallback)
	{
		assert(!params->fft_hasPreCallback);

		if (params->fft_postCallback.localMemSize > 0)
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* post_userdata, __local void* localmem");
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(transKernel, 0);
				bufstream_cat_cstr(transKernel, ", __global void* post_userdata");
			} while (0);
		}
	}

	// Close the method signature
	do
	{
		clKernWrite(transKernel, 0);
		bufprintf(&transKernel->text, " )\n{\n");
	} while (0);

	return CLFFT_SUCCESS;
}

static clfftStatus genTransposeKernel(const FFTKernelGenKeyParams params, buffer_t *strKernel, tile lwSize, const size_t reShapeFactor, const size_t loopCount, tile blockSize)
{
	bufreserve(strKernel, 4096);
	buffer_stream_t transKernel;
	// Initialize stream formatting state explicitly.
	bufstream_init(&transKernel);
	// These strings represent the various data types we read or write in the
	// kernel, depending on how the plan is configured
	buffer_t dtInput = buffer_empty();   // The type read as input into kernel
	buffer_t dtOutput = buffer_empty();  // The type written as output from kernel
	buffer_t dtPlanar = buffer_empty();  // Fundamental type for planar arrays
	buffer_t dtComplex = buffer_empty(); // Fundamental type for complex arrays

	// NOTE:  Enable only for debug

	switch (params.fft_precision)
	{
		case CLFFT_SINGLE:
		case CLFFT_SINGLE_FAST:
			bufsetcstr(&dtPlanar, "float");
			bufsetcstr(&dtComplex, "float2");
			break;
		case CLFFT_DOUBLE:
		case CLFFT_DOUBLE_FAST:
			bufsetcstr(&dtPlanar, "double");
			bufsetcstr(&dtComplex, "double2");

			// Emit code that enables double precision in the kernel
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#ifdef cl_khr_fp64\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#else\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "#pragma OPENCL EXTENSION cl_amd_fp64 : enable\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 0);
				bufprintf(&transKernel.text, "#endif\n\n");
			} while (0);
			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED; break;
	}

	//	If twiddle computation has been requested, generate the lookup function
	if (params.fft_3StepTwiddle)
	{
		buffer_t str = buffer_empty();
		TwiddleTableLarge twLarge;
		// Initialize large twiddle table storage explicitly.
		TwiddleTableLargeInit(&twLarge, params.fft_N[0] * params.fft_N[1]);
		if ((params.fft_precision == CLFFT_SINGLE) || (params.fft_precision == CLFFT_SINGLE_FAST))
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_SINGLE, &str);
		else
			TwiddleTableLargeGenerateTwiddleTable(&twLarge, P_DOUBLE, &str);
		// Release large twiddle table storage after emitting source.
		TwiddleTableLargeFree(&twLarge);
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", bufcstr(&str));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "// Local structure to embody/capture tile dimensions\n");
	} while (0);
	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "typedef struct tag_Tile\n");
	} while (0);
	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "{\n");
	} while (0);
	do
	{
		clKernWrite(&transKernel, 3);
		bufprintf(&transKernel.text, "size_t x;\n");
	} while (0);
	do
	{
		clKernWrite(&transKernel, 3);
		bufprintf(&transKernel.text, "size_t y;\n");
	} while (0);
	do
	{
		clKernWrite(&transKernel, 0);
		bufprintf(&transKernel.text, "} Tile;\n\n");
	} while (0);

	if (params.fft_placeness == CLFFT_INPLACE)
		return CLFFT_TRANSPOSED_NOTIMPLEMENTED;

	// If pre-callback is set for the plan
	if (params.fft_hasPreCallback)
	{
		// Insert callback function code at the beginning
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", params.fft_preCallback.funcstring);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	// If post-callback is set for the plan
	if (params.fft_hasPostCallback)
	{
		// Insert callback function code at the beginning
		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "%s\n", params.fft_postCallback.funcstring);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 0);
			bufstream_endline(&transKernel);
		} while (0);
	}

	for (size_t bothDir = 0; bothDir < 2; bothDir++)
	{
		//	Generate the kernel entry point and parameter list
		//
		bool fwd = bothDir ? false : true;

		buffer_t funcName = buffer_empty();
		if (params.fft_3StepTwiddle)
			bufsetcstr(&funcName, fwd ? "transpose_gcn_tw_fwd" : "transpose_gcn_tw_back");
		else
			bufsetcstr(&funcName, "transpose_gcn");

		genTransposePrototype(&params, &lwSize, &dtPlanar, &dtComplex, &funcName, &transKernel, &dtInput, &dtOutput);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"const Tile localIndex = { get_local_id( 0 ), "
				"get_local_id( 1 ) }; \n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"const Tile localExtent = { get_local_size( 0 ), "
				"get_local_size( 1 ) }; \n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"const Tile groupIndex = { get_group_id( 0 ), "
				"get_group_id( 1 ) };\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"// Calculate the unit address (in terms of datatype) of "
				"the beginning of the Tile for the WG block\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"// Transpose of input & output blocks happens with the "
				"Offset calculation\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t reShapeFactor = %llu;\n", (unsigned long long) (reShapeFactor));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t wgUnroll = %llu;\n", (unsigned long long) (loopCount));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"const Tile wgTileExtent = { localExtent.x * "
				"reShapeFactor, localExtent.y / reShapeFactor };\n");
		} while (0);

		// This is the size of a matrix in the y dimension in units of group
		// size; used to calculate stride[2] indexing
		// size_t numGroupsY = DivRoundingUp( params.fft_N[ 1 ], lwSize.y /
		// reShapeFactor * loopCount );

		// numGroupY_1 is the number of cumulative work groups up to 1st
		// dimension numGroupY_2 is the number of cumulative work groups up to
		// 2nd dimension and so forth

		size_t numGroupsTemp;
		if (params.transOutHorizontal)
			numGroupsTemp = DivRoundingUpSizeT(params.fft_N[0], blockSize.x);
		else
			numGroupsTemp = DivRoundingUpSizeT(params.fft_N[1], blockSize.y);

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t numGroupsY_1 = %llu;\n", (unsigned long long) (numGroupsTemp));
		} while (0);
		for (int i = 2; i < params.fft_DataDim - 1; i++)
		{
			numGroupsTemp *= params.fft_N[i];
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "const size_t numGroupsY_%llu = %llu;\n", (unsigned long long) (i), (unsigned long long) (numGroupsTemp));
			} while (0);
		}

		// Generate the amount of local data share we need
		// Assumption: Even for planar data, we will still store values in LDS
		// as interleaved
		tile ldsSize = { blockSize.x, blockSize.y };
		switch (params.fft_outputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
			case CLFFT_COMPLEX_PLANAR:
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text,
						"// LDS is always complex and allocated transposed: lds[ "
						"wgTileExtent.y * wgUnroll ][ wgTileExtent.x ];\n");
				} while (0);
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "local %s lds[ %llu ][ %llu ];\n\n", bufcstr(&dtComplex), (unsigned long long) (ldsSize.x),
						(unsigned long long) (ldsSize.y));
				} while (0);
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_REAL:
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "local %s lds[ %llu ][ %llu ];\n\n", bufcstr(&dtPlanar), (unsigned long long) (ldsSize.x),
						(unsigned long long) (ldsSize.y));
				} while (0);
				break;
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t currDimIndex;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "size_t rowSizeinUnits;\n\n");
		} while (0);

		OffsetCalc(&transKernel, &params, true);

		switch (params.fft_inputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
				// No need of tileIn declaration when precallback is set as the
				// global buffer is used directly
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* tileIn = %s + iOffset;\n", bufcstr(&dtInput), pmComplexIn);
					} while (0);
				}
				break;
			case CLFFT_COMPLEX_PLANAR:
				// No need of tileIn declaration when precallback is set as the
				// global buffer is used directly
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* realTileIn = %s + iOffset;\n", bufcstr(&dtInput), pmRealIn);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* imagTileIn = %s + iOffset;\n", bufcstr(&dtInput), pmImagIn);
					} while (0);
				}
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_REAL:
				// No need of tileIn declaration when precallback is set as the
				// global buffer is used directly
				if (!params.fft_hasPreCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* tileIn = %s + iOffset;\n", bufcstr(&dtInput), pmRealIn);
					} while (0);
				}
				break;
		}

		// This is the loop reading through the Tile
		if (params.fft_inputLayout == CLFFT_REAL)
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "%s tmp;\n", bufcstr(&dtPlanar));
			} while (0);
		}
		else
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "%s tmp;\n", bufcstr(&dtComplex));
			} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "rowSizeinUnits = %llu;\n", (unsigned long long) (params.fft_inStride[1]));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n\n");
		} while (0);

		//
		// Group index traversal is logical where X direction is horizontal in
		// input buffer and vertical in output buffer when transOutHorizontal is
		// enabled X direction is vertical in input buffer and horizontal in
		// output buffer Not to be confused within a tile, where X is horizontal
		// in input and vertical in output always

		bool branchingInGroupX = params.transOutHorizontal ? ((params.fft_N[1] % blockSize.y) != 0) : ((params.fft_N[0] % blockSize.x) != 0);
		bool branchingInGroupY = params.transOutHorizontal ? ((params.fft_N[0] % blockSize.x) != 0) : ((params.fft_N[1] % blockSize.y) != 0);
		bool branchingInBoth = branchingInGroupX && branchingInGroupY;
		bool branchingInAny = branchingInGroupX || branchingInGroupY;

		size_t branchBlocks = branchingInBoth ? 4 : (branchingInAny ? 2 : 1);

		size_t cornerGroupX = params.transOutHorizontal ? (params.fft_N[1] / blockSize.y) : (params.fft_N[0] / blockSize.x);
		size_t cornerGroupY = params.transOutHorizontal ? (params.fft_N[0] / blockSize.x) : (params.fft_N[1] / blockSize.y);

		buffer_t gIndexX = buffer_from_cstr("groupIndex.x"); // params.transOutHorizontal ? "currDimIndex" :
								     // "groupIndex.x";
		buffer_t gIndexY = buffer_from_cstr("currDimIndex"); // params.transOutHorizontal ? "groupIndex.x" :
								     // "currDimIndex";

		buffer_t wIndexX = buffer_from_cstr(params.transOutHorizontal ? "yInd" : "xInd");
		buffer_t wIndexY = buffer_from_cstr(params.transOutHorizontal ? "xInd" : "yInd");

		size_t wIndexXEnd = params.transOutHorizontal ? params.fft_N[1] % blockSize.y : params.fft_N[0] % blockSize.x;
		size_t wIndexYEnd = params.transOutHorizontal ? params.fft_N[0] % blockSize.x : params.fft_N[1] % blockSize.y;

		// If precallback is set
		if (params.fft_hasPreCallback && params.fft_inputLayout == CLFFT_COMPLEX_PLANAR)
		{
			do
			{
				clKernWrite(&transKernel, 3);
				bufprintf(&transKernel.text, "%s retCallback;\n", bufcstr(&dtComplex));
			} while (0);
		}

		for (size_t i = 0; i < branchBlocks; i++)
		{
			if (branchingInBoth)
				if (i == 0)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "if( (%s == %llu) && (%s == %llu) )\n", bufcstr(&gIndexX), (unsigned long long) (cornerGroupX),
							bufcstr(&gIndexY), (unsigned long long) (cornerGroupY));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 1)
				{
					if (!cornerGroupY)
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else if( %s == %llu )\n", bufcstr(&gIndexX), (unsigned long long) (cornerGroupX));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 2)
				{
					if (!cornerGroupX)
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else if( %s == %llu )\n", bufcstr(&gIndexY), (unsigned long long) (cornerGroupY));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else
				{
					if ((!cornerGroupX) || (!cornerGroupY))
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
			else if (branchingInAny)
				if (i == 0)
				{
					if (branchingInGroupX)
					{
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "if( %s == %llu )\n", bufcstr(&gIndexX), (unsigned long long) (cornerGroupX));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "if( %s == %llu )\n", bufcstr(&gIndexY), (unsigned long long) (cornerGroupY));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
				}
				else
				{
					if ((!cornerGroupX) || (!cornerGroupY))
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for( uint t=0; t < wgUnroll; t++ )\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "{\n");
			} while (0);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text,
					"size_t xInd = localIndex.x + localExtent.x * ( "
					"localIndex.y %% wgTileExtent.y ); \n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text,
					"size_t yInd = localIndex.y/wgTileExtent.y + t * "
					"wgTileExtent.y; \n");
			} while (0);

			// Calculating the index seperately enables easier debugging through
			// tools
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "size_t gInd = xInd + rowSizeinUnits * yInd;\n");
			} while (0);

			if (branchingInBoth)
			{
				if (i == 0)
				{
					do
					{
						clKernWrite(&transKernel, 9);
						bufstream_endline(&transKernel);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if( (%s< %llu) && (%s < %llu) )\n", bufcstr(&wIndexX), (unsigned long long) (wIndexXEnd),
							bufcstr(&wIndexY), (unsigned long long) (wIndexYEnd));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 1)
				{
					do
					{
						clKernWrite(&transKernel, 9);
						bufstream_endline(&transKernel);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexX), (unsigned long long) (wIndexXEnd));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 2)
				{
					do
					{
						clKernWrite(&transKernel, 9);
						bufstream_endline(&transKernel);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexY), (unsigned long long) (wIndexYEnd));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
			}
			else if (branchingInAny)
			{
				if (i == 0)
				{
					if (branchingInGroupX)
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufstream_endline(&transKernel);
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexX), (unsigned long long) (wIndexXEnd));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufstream_endline(&transKernel);
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexY), (unsigned long long) (wIndexYEnd));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
				}
				else
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
			}

			switch (params.fft_inputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmp = %s(%s, iOffset + gInd, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, pmComplexIn);
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text, "tmp = %s(%s, iOffset + gInd, pre_userdata);\n", params.fft_preCallback.funcname,
									pmComplexIn);
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmp = tileIn[ gInd ];\n");
						} while (0);
					}
				}
				break;
				case CLFFT_COMPLEX_PLANAR:
				{
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"retCallback = %s(%s, %s, iOffset + "
									"gInd, pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, pmRealIn, pmImagIn);
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"retCallback = %s(%s, %s, iOffset + "
									"gInd, pre_userdata);\n",
									params.fft_preCallback.funcname, pmRealIn, pmImagIn);
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmp.s0 = retCallback.x;\n");
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmp.s1 = retCallback.y;\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmp.s0 = realTileIn[ gInd ];\n");
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmp.s1 = imagTileIn[ gInd ];\n");
						} while (0);
					}
				}
				break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL:
					if (params.fft_hasPreCallback)
					{
						if (params.fft_preCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"tmp = %s(%s, iOffset + gInd, "
									"pre_userdata, localmem);\n",
									params.fft_preCallback.funcname, pmRealIn);
							} while (0);
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text, "tmp = %s(%s, iOffset + gInd, pre_userdata);\n", params.fft_preCallback.funcname,
									pmRealIn);
							} while (0);
						}
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tmp = tileIn[ gInd ];\n");
						} while (0);
					}
					break;
			}

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "// Transpose of Tile data happens here\n");
			} while (0);

			// If requested, generate the Twiddle math to multiply constant
			// values
			if (params.fft_3StepTwiddle)
				genTwiddleMath(&params, &transKernel, &dtComplex, fwd);

			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "lds[ xInd ][ yInd ] = tmp; \n");
			} while (0);

			if (branchingInAny)
			{
				do
				{
					clKernWrite(&transKernel, 9);
					bufprintf(&transKernel.text, "}\n");
				} while (0);
				do
				{
					clKernWrite(&transKernel, 9);
					bufstream_endline(&transKernel);
				} while (0);
			}

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			if (branchingInAny)
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "}\n");
				} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "barrier( CLK_LOCAL_MEM_FENCE );\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufstream_endline(&transKernel);
		} while (0);

		OffsetCalc(&transKernel, &params, false);

		switch (params.fft_outputLayout)
		{
			case CLFFT_COMPLEX_INTERLEAVED:
				// No need of tileOut declaration when postcallback is set as the
				// global buffer is used directly
				if (!params.fft_hasPostCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* tileOut = %s + oOffset;\n\n", bufcstr(&dtOutput), pmComplexOut);
					} while (0);
				}
				break;
			case CLFFT_COMPLEX_PLANAR:
				// No need of tileOut declaration when postcallback is set as the
				// global buffer is used directly
				if (!params.fft_hasPostCallback)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* realTileOut = %s + oOffset;\n", bufcstr(&dtOutput), pmRealOut);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "global %s* imagTileOut = %s + oOffset;\n", bufcstr(&dtOutput), pmImagOut);
					} while (0);
				}
				break;
			case CLFFT_HERMITIAN_INTERLEAVED:
			case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			case CLFFT_REAL:
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "global %s* tileOut = %s + oOffset;\n\n", bufcstr(&dtOutput), pmRealOut);
				} while (0);
				break;
		}

		// Write the transposed values from LDS into global memory
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "rowSizeinUnits = %llu;\n", (unsigned long long) (params.fft_outStride[1]));
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text,
				"const size_t transposeRatio = wgTileExtent.x / ( "
				"wgTileExtent.y * wgUnroll );\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "const size_t groupingPerY = wgUnroll / wgTileExtent.y;\n");
		} while (0);
		do
		{
			clKernWrite(&transKernel, 3);
			bufprintf(&transKernel.text, "\n\n");
		} while (0);

		for (size_t i = 0; i < branchBlocks; i++)
		{
			if (branchingInBoth)
				if (i == 0)
				{
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "if( (%s == %llu) && (%s == %llu) )\n", bufcstr(&gIndexX), (unsigned long long) (cornerGroupX),
							bufcstr(&gIndexY), (unsigned long long) (cornerGroupY));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 1)
				{
					if (!cornerGroupY)
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else if( %s == %llu )\n", bufcstr(&gIndexX), (unsigned long long) (cornerGroupX));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 2)
				{
					if (!cornerGroupX)
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else if( %s == %llu )\n", bufcstr(&gIndexY), (unsigned long long) (cornerGroupY));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else
				{
					if ((!cornerGroupX) || (!cornerGroupY))
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
			else if (branchingInAny)
				if (i == 0)
				{
					if (branchingInGroupX)
					{
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "if( %s == %llu )\n", bufcstr(&gIndexX), (unsigned long long) (cornerGroupX));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "if( %s == %llu )\n", bufcstr(&gIndexY), (unsigned long long) (cornerGroupY));
						} while (0);
						do
						{
							clKernWrite(&transKernel, 3);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
				}
				else
				{
					if ((!cornerGroupX) || (!cornerGroupY))
						continue;

					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "else\n");
					} while (0);
					do
					{
						clKernWrite(&transKernel, 3);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "for( uint t=0; t < wgUnroll; t++ )\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "{\n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text,
					"size_t xInd = localIndex.x + localExtent.x * ( "
					"localIndex.y %% groupingPerY ); \n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text,
					"size_t yInd = localIndex.y/groupingPerY + t * "
					"(wgTileExtent.y * transposeRatio); \n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "tmp = lds[ yInd ][ xInd ]; \n");
			} while (0);
			do
			{
				clKernWrite(&transKernel, 9);
				bufprintf(&transKernel.text, "size_t gInd = xInd + rowSizeinUnits * yInd;\n");
			} while (0);

			if (branchingInBoth)
			{
				if (i == 0)
				{
					do
					{
						clKernWrite(&transKernel, 9);
						bufstream_endline(&transKernel);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if( (%s < %llu) && (%s < %llu) )\n", bufcstr(&wIndexY), (unsigned long long) (wIndexXEnd),
							bufcstr(&wIndexX), (unsigned long long) (wIndexYEnd));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 1)
				{
					do
					{
						clKernWrite(&transKernel, 9);
						bufstream_endline(&transKernel);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexY), (unsigned long long) (wIndexXEnd));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else if (i == 2)
				{
					do
					{
						clKernWrite(&transKernel, 9);
						bufstream_endline(&transKernel);
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexX), (unsigned long long) (wIndexYEnd));
					} while (0);
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
				}
				else
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
			}
			else if (branchingInAny)
			{
				buffer_t limitToWGForRealSpecial = buffer_from_cstr(params.transOutHorizontal ? "groupIndex.x" : "currDimIndex");

				if (i == 0)
				{
					if (branchingInGroupX)
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufstream_endline(&transKernel);
						} while (0);
						if (params.fft_realSpecial)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if( ((%s == %llu) && (%s < 1) && "
									"(%s == 0)) ",
									bufcstr(&wIndexY), (unsigned long long) (wIndexXEnd - 1), bufcstr(&wIndexX),
									bufcstr(&limitToWGForRealSpecial));
							} while (0);
							if (wIndexXEnd > 1)
							{
								do
								{
									clKernWrite(&transKernel, 0);
									bufprintf(&transKernel.text, "|| (%s < %llu) )\n", bufcstr(&wIndexY),
										(unsigned long long) (wIndexXEnd - 1));
								} while (0);
							}
							else
							{
								do
								{
									clKernWrite(&transKernel, 0);
									bufprintf(&transKernel.text, ")\n");
								} while (0);
							}
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexY), (unsigned long long) (wIndexXEnd));
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufstream_endline(&transKernel);
						} while (0);
						if (params.fft_realSpecial)
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text,
									"if( ((%s == %llu) && (%s < 1) && "
									"(%s == 0)) ",
									bufcstr(&wIndexX), (unsigned long long) (wIndexYEnd - 1), bufcstr(&wIndexY),
									bufcstr(&limitToWGForRealSpecial));
							} while (0);
							if (wIndexYEnd > 1)
							{
								do
								{
									clKernWrite(&transKernel, 0);
									bufprintf(&transKernel.text, "|| (%s < %llu) )\n", bufcstr(&wIndexX),
										(unsigned long long) (wIndexYEnd - 1));
								} while (0);
							}
							else
							{
								do
								{
									clKernWrite(&transKernel, 0);
									bufprintf(&transKernel.text, ")\n");
								} while (0);
							}
						}
						else
						{
							do
							{
								clKernWrite(&transKernel, 9);
								bufprintf(&transKernel.text, "if( (%s < %llu) )\n", bufcstr(&wIndexX), (unsigned long long) (wIndexYEnd));
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "{\n");
						} while (0);
					}
				}
				else
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "{\n");
					} while (0);
			}

			switch (params.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "%s(%s, (oOffset + gInd), post_userdata, tmp", params.fft_postCallback.funcname, pmComplexOut);
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "tileOut[ gInd ] = tmp;\n");
						} while (0);
					}
					break;
				case CLFFT_COMPLEX_PLANAR:
					if (params.fft_hasPostCallback)
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text,
								"%s(%s, %s, (oOffset + gInd), post_userdata, "
								"tmp.s0, tmp.s1",
								params.fft_postCallback.funcname, pmRealOut, pmImagOut);
						} while (0);
						if (params.fft_postCallback.localMemSize > 0)
						{
							do
							{
								clKernWrite(&transKernel, 0);
								bufstream_cat_cstr(&transKernel, ", localmem");
							} while (0);
						}
						do
						{
							clKernWrite(&transKernel, 0);
							bufprintf(&transKernel.text, ");\n");
						} while (0);
					}
					else
					{
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "realTileOut[ gInd ] = tmp.s0;\n");
						} while (0);
						do
						{
							clKernWrite(&transKernel, 9);
							bufprintf(&transKernel.text, "imagTileOut[ gInd ] = tmp.s1;\n");
						} while (0);
					}
					break;
				case CLFFT_HERMITIAN_INTERLEAVED:
				case CLFFT_HERMITIAN_PLANAR: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
				case CLFFT_REAL:
					do
					{
						clKernWrite(&transKernel, 9);
						bufprintf(&transKernel.text, "tileOut[ gInd ] = tmp;\n");
					} while (0);
					break;
			}

			if (branchingInAny)
			{
				do
				{
					clKernWrite(&transKernel, 9);
					bufprintf(&transKernel.text, "}\n");
				} while (0);
			}

			do
			{
				clKernWrite(&transKernel, 6);
				bufprintf(&transKernel.text, "}\n");
			} while (0);

			if (branchingInAny)
				do
				{
					clKernWrite(&transKernel, 3);
					bufprintf(&transKernel.text, "}\n");
				} while (0);
		}

		do
		{
			clKernWrite(&transKernel, 0);
			bufprintf(&transKernel.text, "}\n\n");
		} while (0);

		// Copy generated source into the caller-owned buffer explicitly.
		bufsetbuf(strKernel, &transKernel.text);

		if (!params.fft_3StepTwiddle)
			break;
	}

	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedTransposeGCNActionInitParams(FFTGeneratedTransposeGCNAction *action)
{
	action->signature.data.fft_precision = action->base.plan->precision;
	action->signature.data.fft_placeness = action->base.plan->placeness;
	action->signature.data.fft_inputLayout = action->base.plan->inputLayout;
	action->signature.data.fft_outputLayout = action->base.plan->outputLayout;
	action->signature.data.fft_3StepTwiddle = false;

	action->signature.data.fft_realSpecial = action->base.plan->realSpecial;

	action->signature.data.transOutHorizontal = action->base.plan->transOutHorizontal; // using the twiddle front flag to specify
											   // horizontal write we do this so as to reuse
											   // flags in FFTKernelGenKeyParams and to avoid
											   // making a new one

	ARG_CHECK(array_size(&action->base.plan->inStride) == array_size(&action->base.plan->outStride));

	if (CLFFT_INPLACE == action->signature.data.fft_placeness)
	{
		//	If this is an in-place transform the
		//	input and output layout, dimensions and strides
		//	*MUST* be the same.
		//
		ARG_CHECK(action->signature.data.fft_inputLayout == action->signature.data.fft_outputLayout)

		for (size_t u = array_size(&action->base.plan->inStride); u-- > 0;)
		{
			ARG_CHECK(action->base.plan->inStride.buf[u] == action->base.plan->outStride.buf[u]);
		}
	}

	action->signature.data.fft_DataDim = array_size(&action->base.plan->length) + 1;
	int i = 0;
	for (i = 0; i < (action->signature.data.fft_DataDim - 1); i++)
	{
		action->signature.data.fft_N[i] = action->base.plan->length.buf[i];
		action->signature.data.fft_inStride[i] = action->base.plan->inStride.buf[i];
		action->signature.data.fft_outStride[i] = action->base.plan->outStride.buf[i];
	}
	action->signature.data.fft_inStride[i] = action->base.plan->iDist;
	action->signature.data.fft_outStride[i] = action->base.plan->oDist;

	if (action->base.plan->large1D != 0)
	{
		ARG_CHECK(action->signature.data.fft_N[0] != 0)
		ARG_CHECK((action->base.plan->large1D % action->signature.data.fft_N[0]) == 0)
		action->signature.data.fft_3StepTwiddle = true;
		ARG_CHECK(action->base.plan->large1D == (action->signature.data.fft_N[1] * action->signature.data.fft_N[0]));
	}

	//	Query the devices in this context for their local memory sizes
	//	How we generate a kernel depends on the *minimum* LDS size for all
	// devices.
	//
	const FFTEnvelope *pEnvelope = NULL;
	OPENCL_V(FFTPlanGetEnvelope(action->base.plan, &pEnvelope), _T( "GetEnvelope failed" ));
	BUG_CHECK(NULL != pEnvelope);

	// TODO:  Since I am going with a 2D workgroup size now, I need a better
	// check than this 1D use Check:
	// CL_DEVICE_MAX_WORK_GROUP_SIZE/CL_KERNEL_WORK_GROUP_SIZE
	// CL_DEVICE_MAX_WORK_ITEM_SIZES
	action->signature.data.fft_R = 1;				  // Dont think i'll use
	action->signature.data.fft_SIMD = pEnvelope->limit_WorkGroupSize; // Use devices maximum workgroup size

	// Set callback if specified
	if (action->base.plan->hasPreCallback)
	{
		action->signature.data.fft_hasPreCallback = true;
		action->signature.data.fft_preCallback = action->base.plan->preCallback;
	}
	if (action->base.plan->hasPostCallback)
	{
		action->signature.data.fft_hasPostCallback = true;
		action->signature.data.fft_postCallback = action->base.plan->postCallbackParam;
	}
	action->signature.data.limit_LocalMemSize = action->base.plan->envelope.limit_LocalMemSize;

	return CLFFT_SUCCESS;
}

// Constants that specify the bounding sizes of the block that each workgroup
// will transpose
static const tile lwSize = { 16, 16 };
static const size_t reShapeFactor = 4; // wgTileSize = { lwSize.x * reShapeFactor, lwSize.y / reShapeFactor }

static clfftStatus CalculateBlockSize(clfftPrecision precision, size_t *loopCount, tile *blockSize)
{
	switch (precision)
	{
		case CLFFT_SINGLE:
		case CLFFT_SINGLE_FAST: *loopCount = 16; break;
		case CLFFT_DOUBLE:
		case CLFFT_DOUBLE_FAST:
			// Double precisions need about half the amount of LDS space as singles
			// do
			*loopCount = 8;
			break;
		default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED; break;
	}

	blockSize->x = lwSize.x * reShapeFactor;
	blockSize->y = lwSize.y / reShapeFactor * (*loopCount);

	return CLFFT_SUCCESS;
}

//	OpenCL does not take unicode strings as input, so this routine returns only
// ASCII strings 	Feed this generator the FFTPlan, and it returns the
// generated program as a string
static clfftStatus FFTGeneratedTransposeGCNActionGenerateKernel(FFTGeneratedTransposeGCNAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT)
{
	size_t loopCount = 0;
	tile blockSize = { 0, 0 };
	OPENCL_V(CalculateBlockSize(action->signature.data.fft_precision, &loopCount, &blockSize), "CalculateBlockSize() failed!");

	// Requested local memory size by callback must not exceed the device LDS
	// limits after factoring the LDS size required by main FFT kernel
	if ((action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0) ||
		(action->signature.data.fft_hasPostCallback && action->signature.data.fft_postCallback.localMemSize > 0))
	{
		assert(!(action->signature.data.fft_hasPreCallback && action->signature.data.fft_hasPostCallback));

		bool validLDSSize = false;
		size_t length = blockSize.x * blockSize.y;

		size_t requestedCallbackLDS = 0;

		if (action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0)
			requestedCallbackLDS = action->signature.data.fft_preCallback.localMemSize;
		else if (action->signature.data.fft_hasPostCallback && action->signature.data.fft_postCallback.localMemSize > 0)
			requestedCallbackLDS = action->signature.data.fft_postCallback.localMemSize;

		validLDSSize = ((length * FFTPlanElementSize(action->base.plan)) + requestedCallbackLDS) < action->base.plan->envelope.limit_LocalMemSize;

		if (!validLDSSize)
		{
			fprintf(stderr, "Requested local memory size not available\n");
			return CLFFT_INVALID_ARG_VALUE;
		}
	}

	buffer_t programCode = buffer_empty();
	OPENCL_V(genTransposeKernel(action->signature.data, &programCode, lwSize, reShapeFactor, loopCount, blockSize), _T( "GenerateTransposeKernel() failed!" ));

	cl_int status = CL_SUCCESS;
	cl_device_id Device = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_DEVICE, sizeof(cl_device_id), &Device, NULL);
	OPENCL_V(status, _T( "clGetCommandQueueInfo failed" ));

	cl_context QueueContext = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_CONTEXT, sizeof(cl_context), &QueueContext, NULL);
	OPENCL_V(status, _T( "clGetCommandQueueInfo failed" ));

	OPENCL_V(FFTRepoSetProgramCode(fftRepo, Transpose_GCN, &action->signature.header, &programCode, Device, QueueContext), _T( "FFTRepoSetProgramCode failed!" ));

	// Note:  See genFunctionPrototype( )
	if (action->signature.data.fft_3StepTwiddle)
	{
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_GCN, &action->signature.header, "transpose_gcn_tw_fwd", "transpose_gcn_tw_back", Device, QueueContext),
			_T( "FFTRepoSetProgramEntryPoints failed!" ));
	}
	else
	{
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_GCN, &action->signature.header, "transpose_gcn", "transpose_gcn", Device, QueueContext),
			_T( "FFTRepoSetProgramEntryPoints failed!" ));
	}

	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedTransposeGCNActionGetWorkSizes(FFTGeneratedTransposeGCNAction *action, array_size_t *globalWS, array_size_t *localWS)
{
	size_t loopCount = 0;
	tile blockSize = { 0, 0 };
	OPENCL_V(CalculateBlockSize(action->signature.data.fft_precision, &loopCount, &blockSize), "CalculateBlockSize() failed!");

	// We need to make sure that the global work size is evenly divisible by the
	// local work size Our transpose works in tiles, so divide tiles in each
	// dimension to get count of blocks, rounding up for remainder items
	size_t numBlocksX = action->signature.data.transOutHorizontal ? DivRoundingUpSizeT(action->signature.data.fft_N[1], blockSize.y)
								      : DivRoundingUpSizeT(action->signature.data.fft_N[0], blockSize.x);
	size_t numBlocksY = action->signature.data.transOutHorizontal ? DivRoundingUpSizeT(action->signature.data.fft_N[0], blockSize.x)
								      : DivRoundingUpSizeT(action->signature.data.fft_N[1], blockSize.y);
	size_t numWIX = numBlocksX * lwSize.x;

	// Batches of matrices are lined up along the Y axis, 1 after the other
	size_t numWIY = numBlocksY * lwSize.y * action->base.plan->batchsize;
	// fft_DataDim has one more dimension than the actual fft data, which is
	// devoted to batch. dim from 2 to fft_DataDim - 2 are lined up along the Y
	// axis
	for (int i = 2; i < action->signature.data.fft_DataDim - 1; i++)
		numWIY *= action->signature.data.fft_N[i];

	array_clear(globalWS);
	array_push_back(globalWS, numWIX);
	array_push_back(globalWS, numWIY);

	array_clear(localWS);
	array_push_back(localWS, lwSize.x);
	array_push_back(localWS, lwSize.y);

	return CLFFT_SUCCESS;
}
/* End copied source: src\library\generator.transpose.gcn.cpp */

#undef clKernWrite
#undef OffsetCalc
#undef genTwiddleMath
#undef genTransposePrototype
#undef genTransposeKernel
#undef pmRealIn
#undef pmImagIn
#undef pmRealOut
#undef pmImagOut
#undef pmComplexIn
#undef pmComplexOut
#undef lwSize
#undef reShapeFactor
/* Begin copied source: src\library\action.transpose.cpp */

// action.transpose.nonsquare.cpp provides the entry points of "baking"
// nonsquare inplace transpose kernels called in plan.cpp.
// the actual kernel string generation is provided by generator.transpose.cpp

static FFTGeneratedTransposeNonSquareAction *FFTGeneratedTransposeNonSquareActionCreate(clfftPlanHandle plHandle, FFTPlan *plan, cl_command_queue queue, clfftStatus *err)
{
	// Allocate action storage and clear it before explicit initialization.
	FFTGeneratedTransposeNonSquareAction *action = (FFTGeneratedTransposeNonSquareAction *) clfft_checked_malloc(sizeof(FFTGeneratedTransposeNonSquareAction));
	memset((void *) action, 0, sizeof(*action));

	// Initialize the common action base and signature payload.
	FFTActionInit(&action->base, plan, Transpose_NONSQUARE, action, err);
	FFTKernelSignatureTransposeInit(&action->signature);
	if (*err != CLFFT_SUCCESS)
	{
		// Report base initialization failure and leave cleanup to the caller.
		fprintf(stderr,
			"FFTGeneratedTransposeNonSquareActionCreate: "
			"FFTActionInit failed!\n");
		return action;
	}

	// Initialize the FFTAction kernel-generation parameter member.
	*err = FFTGeneratedTransposeNonSquareActionInitParams(action);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedTransposeNonSquareActionInitParams failed!\n");
		return action;
	}

	// Generate the kernel source through the shared repository.
	FFTRepo *fftRepo = FFTRepoGetInstance();
	*err = FFTActionGenerateKernel(&action->base, fftRepo, queue);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedTransposeNonSquareActionCreate: "
			"FFTActionGenerateKernel failed\n");
		return action;
	}

	// Compile generated kernels for the requested queue and plan.
	*err = FFTActionCompileKernels(&action->base, queue, plHandle, plan);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedTransposeNonSquareActionCreate: "
			"FFTActionCompileKernels failed\n");
		return action;
	}

	// Mark action creation as successful.
	*err = CLFFT_SUCCESS;
	return action;
}

static bool FFTGeneratedTransposeNonSquareActionBuildForwardKernel(FFTGeneratedTransposeNonSquareAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || r2c_transform;
}

static bool FFTGeneratedTransposeNonSquareActionBuildBackwardKernel(FFTGeneratedTransposeNonSquareAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || c2r_transform;
}

// These strings represent the names that are used as strKernel parameters
static const char pmRealIn[] = "pmRealIn";
static const char pmImagIn[] = "pmImagIn";
static const char pmRealOut[] = "pmRealOut";
static const char pmImagOut[] = "pmImagOut";
static const char pmComplexIn[] = "pmComplexIn";
static const char pmComplexOut[] = "pmComplexOut";

static clfftStatus FFTGeneratedTransposeNonSquareActionInitParams(FFTGeneratedTransposeNonSquareAction *action)
{
	action->signature.data.fft_precision = action->base.plan->precision;
	action->signature.data.fft_placeness = action->base.plan->placeness;
	action->signature.data.fft_inputLayout = action->base.plan->inputLayout;
	action->signature.data.fft_outputLayout = action->base.plan->outputLayout;
	action->signature.data.fft_3StepTwiddle = false;
	action->signature.data.nonSquareKernelType = action->base.plan->nonSquareKernelType;

	action->signature.data.fft_realSpecial = action->base.plan->realSpecial;

	action->signature.data.transOutHorizontal = action->base.plan->transOutHorizontal; // using the twiddle front flag to specify
											   // horizontal write we do this so as to reuse
											   // flags in FFTKernelGenKeyParams and to avoid
											   // making a new one

	ARG_CHECK(array_size(&action->base.plan->inStride) == array_size(&action->base.plan->outStride));

	if (CLFFT_INPLACE == action->signature.data.fft_placeness)
	{
		//	If this is an in-place transform the
		//	input and output layout
		//	*MUST* be the same.
		//
		ARG_CHECK(action->signature.data.fft_inputLayout == action->signature.data.fft_outputLayout)

		/*        for (size_t u = array_size(&action->base.plan->inStride); u--
		   > 0; )
				{
					ARG_CHECK(action->base.plan->inStride.buf[u] ==
		   action->base.plan->outStride.buf[u]);
				}*/
	}

	action->signature.data.fft_DataDim = array_size(&action->base.plan->length) + 1;

	int i = 0;
	for (i = 0; i < (action->signature.data.fft_DataDim - 1); i++)
	{
		action->signature.data.fft_N[i] = action->base.plan->length.buf[i];
		action->signature.data.fft_inStride[i] = action->base.plan->inStride.buf[i];
		action->signature.data.fft_outStride[i] = action->base.plan->outStride.buf[i];
	}
	action->signature.data.fft_inStride[i] = action->base.plan->iDist;
	action->signature.data.fft_outStride[i] = action->base.plan->oDist;

	if (action->base.plan->large1D != 0)
	{
		ARG_CHECK(action->signature.data.fft_N[0] != 0)
		// ToDo:ENABLE ASSERT
		//     ARG_CHECK((action->base.plan->large1D %
		//     action->signature.data.fft_N[0]) == 0)
		action->signature.data.fft_3StepTwiddle = true;
		// ToDo:ENABLE ASSERT
		// ARG_CHECK(action->base.plan->large1D ==
		// (action->signature.data.fft_N[1] * action->signature.data.fft_N[0]));
	}

	//	Query the devices in this context for their local memory sizes
	//	How we generate a kernel depends on the *minimum* LDS size for all
	// devices.
	//
	const FFTEnvelope *pEnvelope = NULL;
	OPENCL_V(FFTPlanGetEnvelope(action->base.plan, &pEnvelope), "GetEnvelope failed");
	BUG_CHECK(NULL != pEnvelope);

	// TODO:  Since I am going with a 2D workgroup size now, I need a better
	// check than this 1D use Check:
	// CL_DEVICE_MAX_WORK_GROUP_SIZE/CL_KERNEL_WORK_GROUP_SIZE
	// CL_DEVICE_MAX_WORK_ITEM_SIZES
	action->signature.data.fft_R = 1;				  // Dont think i'll use
	action->signature.data.fft_SIMD = pEnvelope->limit_WorkGroupSize; // Use devices maximum workgroup size

	// Set callback if specified
	if (action->base.plan->hasPreCallback)
	{
		action->signature.data.fft_hasPreCallback = true;
		action->signature.data.fft_preCallback = action->base.plan->preCallback;
	}
	if (action->base.plan->hasPostCallback)
	{
		action->signature.data.fft_hasPostCallback = true;
		action->signature.data.fft_postCallback = action->base.plan->postCallbackParam;
	}
	action->signature.data.limit_LocalMemSize = action->base.plan->envelope.limit_LocalMemSize;

	action->signature.data.transposeMiniBatchSize = action->base.plan->transposeMiniBatchSize;
	action->signature.data.nonSquareKernelOrder = action->base.plan->nonSquareKernelOrder;
	action->signature.data.transposeBatchSize = action->base.plan->batchsize;

	return CLFFT_SUCCESS;
}

static const size_t lwSize = 256;
static const size_t reShapeFactor = 2;

//	OpenCL does not take unicode strings as input, so this routine returns only
// ASCII strings 	Feed this generator the FFTPlan, and it returns the
// generated program as a string
static clfftStatus FFTGeneratedTransposeNonSquareActionGenerateKernel(FFTGeneratedTransposeNonSquareAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT)
{
	buffer_t programCode = buffer_empty();
	buffer_t kernelFuncName = buffer_empty(); // applied to swap kernel for now
	if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED_LEADING)
	{
		// Requested local memory size by callback must not exceed the device
		// LDS limits after factoring the LDS size required by transpose kernel
		if (action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0)
		{
			assert(!action->signature.data.fft_hasPostCallback);

			bool validLDSSize = false;
			size_t requestedCallbackLDS = 0;

			requestedCallbackLDS = action->signature.data.fft_preCallback.localMemSize;

			validLDSSize = ((2 * FFTPlanElementSize(action->base.plan) * 16 * reShapeFactor * 16 * reShapeFactor) + requestedCallbackLDS) <
				action->base.plan->envelope.limit_LocalMemSize;

			if (!validLDSSize)
			{
				fprintf(stderr, "Requested local memory size not available\n");
				return CLFFT_INVALID_ARG_VALUE;
			}
		}
		OPENCL_V(clfft_transpose_generator_genTransposeKernelLeadingDimensionBatched(action->signature.data, &programCode, lwSize, reShapeFactor),
			"genTransposeKernel() failed!");
	}
	else if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED)
	{
		// pre call back check
		// Requested local memory size by callback must not exceed the device
		// LDS limits after factoring the LDS size required by transpose kernel
		if (action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0)
		{
			assert(!action->signature.data.fft_hasPostCallback);

			bool validLDSSize = false;
			size_t requestedCallbackLDS = 0;

			requestedCallbackLDS = action->signature.data.fft_preCallback.localMemSize;

			validLDSSize = ((2 * FFTPlanElementSize(action->base.plan) * 16 * reShapeFactor * 16 * reShapeFactor) + requestedCallbackLDS) <
				action->base.plan->envelope.limit_LocalMemSize;

			if (!validLDSSize)
			{
				fprintf(stderr, "Requested local memory size not available\n");
				return CLFFT_INVALID_ARG_VALUE;
			}
		}
		OPENCL_V(clfft_transpose_generator_genTransposeKernelBatched(action->signature.data, &programCode, lwSize, reShapeFactor), "genTransposeKernel() failed!");
	}
	else
	{
		// pre-callback is possible in swap kernel now
		if (action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0)
		{
			assert(!action->signature.data.fft_hasPostCallback);

			bool validLDSSize = false;
			size_t requestedCallbackLDS = 0;

			requestedCallbackLDS = action->signature.data.fft_preCallback.localMemSize;
			// LDS usage of swap lines is exactly 2 lines
			size_t lineSize = (action->signature.data.fft_N[0]) < (action->signature.data.fft_N[1]) ? action->signature.data.fft_N[0] : action->signature.data.fft_N[1];
			validLDSSize = ((2 * FFTPlanElementSize(action->base.plan) * lineSize) + requestedCallbackLDS) < action->base.plan->envelope.limit_LocalMemSize;

			if (!validLDSSize)
			{
				fprintf(stderr, "Requested local memory size not available\n");
				return CLFFT_INVALID_ARG_VALUE;
			}
		}
		// general swap kernel takes care of all ratio
		OPENCL_V(clfft_transpose_generator_genSwapKernelGeneral(action->signature.data, &programCode, &kernelFuncName, lwSize, reShapeFactor), "genSwapKernel() failed!");
	}
	cl_int status = CL_SUCCESS;
	cl_device_id Device = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_DEVICE, sizeof(cl_device_id), &Device, NULL);
	OPENCL_V(status, "clGetCommandQueueInfo failed");

	cl_context QueueContext = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_CONTEXT, sizeof(cl_context), &QueueContext, NULL);
	OPENCL_V(status, "clGetCommandQueueInfo failed");

	OPENCL_V(FFTRepoSetProgramCode(fftRepo, Transpose_NONSQUARE, &action->signature.header, &programCode, Device, QueueContext), "FFTRepoSetProgramCode failed!");
	if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED_LEADING)
	{
		// Note:  See genFunctionPrototype( )
		if (action->signature.data.fft_3StepTwiddle)
		{
			OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE, &action->signature.header, "transpose_nonsquare_tw_fwd", "transpose_nonsquare_tw_back",
					 Device, QueueContext),
				"FFTRepoSetProgramEntryPoints failed!");
		}
		else
		{
			OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE, &action->signature.header, "transpose_nonsquare", "transpose_nonsquare", Device,
					 QueueContext),
				"FFTRepoSetProgramEntryPoints failed!");
		}
	}
	else if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED)
	{
		// for non square we do twiddling in swap kernel
		/*
		if (action->signature.data.fft_3StepTwiddle &&
		(action->signature.data.transposeMiniBatchSize == 1))
		{
			OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE,
		&action->signature.header, "transpose_square_tw_fwd",
		"transpose_square_tw_back", Device, QueueContext),
		"FFTRepoSetProgramEntryPoints failed!");
		}
		else
		{
			OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE,
		&action->signature.header, "transpose_square", "transpose_square",
		Device, QueueContext), "FFTRepoSetProgramEntryPoints failed!");
		}
		*/
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE, &action->signature.header, "transpose_square", "transpose_square", Device, QueueContext),
			"FFTRepoSetProgramEntryPoints failed!");
	}
	else if (action->signature.data.fft_3StepTwiddle) // if miniBatchSize > 1 twiddling is done in
	// swap kernel
	{
		// Build the twiddle entry point names without temporary merge helpers.
		buffer_t kernelFwdFuncName = buffer_empty();
		buffer_t kernelBwdFuncName = buffer_empty();
		bufsetbuf(&kernelFwdFuncName, &kernelFuncName);
		bufcatcstr(&kernelFwdFuncName, "_tw_fwd");
		bufsetbuf(&kernelBwdFuncName, &kernelFuncName);
		bufcatcstr(&kernelBwdFuncName, "_tw_back");
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE, &action->signature.header, bufcstr(&kernelFwdFuncName), bufcstr(&kernelBwdFuncName), Device,
				 QueueContext),
			"FFTRepoSetProgramEntryPoints failed!");
	}
	else
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_NONSQUARE, &action->signature.header, bufcstr(&kernelFuncName), bufcstr(&kernelFuncName), Device,
				 QueueContext),
			"FFTRepoSetProgramEntryPoints failed!");
	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedTransposeNonSquareActionGetWorkSizes(FFTGeneratedTransposeNonSquareAction *action, array_size_t *globalWS, array_size_t *localWS)
{
	size_t wg_slice;
	size_t smaller_dim = (action->signature.data.fft_N[0] < action->signature.data.fft_N[1]) ? action->signature.data.fft_N[0] : action->signature.data.fft_N[1];
	size_t bigger_dim = (action->signature.data.fft_N[0] >= action->signature.data.fft_N[1]) ? action->signature.data.fft_N[0] : action->signature.data.fft_N[1];
	size_t dim_ratio = bigger_dim / smaller_dim;
	size_t global_item_size;

	if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED_LEADING)
	{
		if (smaller_dim % (16 * reShapeFactor) == 0)
			wg_slice = smaller_dim / 16 / reShapeFactor;
		else
			wg_slice = (smaller_dim / (16 * reShapeFactor)) + 1;

		global_item_size = wg_slice * (wg_slice + 1) / 2 * 16 * 16 * action->base.plan->batchsize;

		for (int i = 2; i < action->signature.data.fft_DataDim - 1; i++)
			global_item_size *= action->signature.data.fft_N[i];

		/*Push the data required for the transpose kernels*/
		array_clear(globalWS);
		if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED_LEADING)
			array_push_back(globalWS, global_item_size * dim_ratio);
		else if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED)
			array_push_back(globalWS, global_item_size);

		array_clear(localWS);
		array_push_back(localWS, lwSize);
	}
	else if (action->signature.data.nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED)
	{
		if (smaller_dim % (16 * reShapeFactor) == 0)
			wg_slice = smaller_dim / 16 / reShapeFactor;
		else
			wg_slice = (smaller_dim / (16 * reShapeFactor)) + 1;

		global_item_size = wg_slice * (wg_slice + 1) / 2 * 16 * 16 * action->base.plan->batchsize;

		for (int i = 2; i < array_size(&action->base.plan->length); i++)
			global_item_size *= action->base.plan->length.buf[i];

		/*Push the data required for the transpose kernels*/
		array_clear(globalWS);
		array_push_back(globalWS, global_item_size);

		array_clear(localWS);
		array_push_back(localWS, lwSize);
	}
	else
	{
		/*Now calculate the data for the swap kernels */
		// general swap kernel takes care of all ratio. need clean up here
		if (dim_ratio == 2 && 0)
		{
			// 1:2 ratio
			size_t input_elm_size_in_bytes;
			switch (action->signature.data.fft_precision)
			{
				case CLFFT_SINGLE:
				case CLFFT_SINGLE_FAST: input_elm_size_in_bytes = 4; break;
				case CLFFT_DOUBLE:
				case CLFFT_DOUBLE_FAST: input_elm_size_in_bytes = 8; break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}

			switch (action->signature.data.fft_outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				case CLFFT_COMPLEX_PLANAR: input_elm_size_in_bytes *= 2; break;
				case CLFFT_REAL: break;
				default: return CLFFT_TRANSPOSED_NOTIMPLEMENTED;
			}
			size_t max_elements_loaded = AVAIL_MEM_SIZE / input_elm_size_in_bytes;
			size_t num_elements_loaded;
			size_t local_work_size_swap, num_grps_pro_row;

			if ((max_elements_loaded >> 1) > smaller_dim)
			{
				local_work_size_swap = (smaller_dim < 256) ? smaller_dim : 256;
				num_elements_loaded = smaller_dim;
				num_grps_pro_row = 1;
			}
			else
			{
				num_grps_pro_row = (smaller_dim << 1) / max_elements_loaded;
				num_elements_loaded = max_elements_loaded >> 1;
				local_work_size_swap = (num_elements_loaded < 256) ? num_elements_loaded : 256;
			}
			size_t num_reduced_row;
			size_t num_reduced_col;

			if (action->signature.data.fft_N[1] == smaller_dim)
			{
				num_reduced_row = smaller_dim;
				num_reduced_col = 2;
			}
			else
			{
				num_reduced_row = 2;
				num_reduced_col = smaller_dim;
			}

			size_t *cycle_map = (size_t *) clfft_checked_malloc(num_reduced_row * num_reduced_col * 2 * sizeof(size_t));
			/* The memory required by cycle_map cannot exceed 2 times row*col by
			 * design*/
			clfft_transpose_generator_get_cycles(cycle_map, num_reduced_row, num_reduced_col);

			global_item_size = local_work_size_swap * num_grps_pro_row * cycle_map[0] * action->base.plan->batchsize;

			for (int i = 2; i < action->signature.data.fft_DataDim - 1; i++)
				global_item_size *= action->signature.data.fft_N[i];
			free(cycle_map);

			array_push_back(globalWS, global_item_size);
			array_push_back(localWS, local_work_size_swap);
		}
		else
		{
			// if (dim_ratio == 2 || dim_ratio == 3 || dim_ratio == 5 ||
			// dim_ratio == 10)
			if (dim_ratio % 2 == 0 || dim_ratio % 3 == 0 || dim_ratio % 5 == 0 || dim_ratio % 10 == 0)
			{
				size_t local_work_size_swap = 256;
				array_size_t_array permutationTable;
				// Initialize dynamic array storage explicitly.
				array_init(&permutationTable);
				clfft_transpose_generator_permutation_calculation(dim_ratio, smaller_dim, &permutationTable);
				size_t global_item_size;
				if (action->base.plan->large1D && (dim_ratio > 1))
					global_item_size = (array_size(&permutationTable) + 2) * local_work_size_swap * action->base.plan->batchsize;
				else
					global_item_size = (array_size(&permutationTable) + 2) * local_work_size_swap * action->base.plan->batchsize;
				// for (int i = 2; i < array_size(&action->base.plan->length); i++)
				//	global_item_size *= action->base.plan->length.buf[i];
				size_t LDS_per_WG = smaller_dim;
				while (LDS_per_WG > 1024) // avoiding using too much lds memory. the biggest
							  // LDS memory we will allocate would be
							  // 1024*sizeof(float2/double2)*2
				{
					if (LDS_per_WG % 2 == 0)
					{
						LDS_per_WG /= 2;
						continue;
					}
					if (LDS_per_WG % 3 == 0)
					{
						LDS_per_WG /= 3;
						continue;
					}
					if (LDS_per_WG % 5 == 0)
					{
						LDS_per_WG /= 5;
						continue;
					}
					// Release permutation table before leaving this branch
					// unsupported.
					array_free_size_t_array(&permutationTable);
					return CLFFT_NOTIMPLEMENTED;
				}

				size_t WG_per_line = smaller_dim / LDS_per_WG;
				global_item_size *= WG_per_line;
				array_push_back(globalWS, global_item_size);
				array_push_back(localWS, local_work_size_swap);
				// Release permutation table after deriving work sizes.
				array_free_size_t_array(&permutationTable);
			}
			else
				return CLFFT_NOTIMPLEMENTED;
		}
	}
	return CLFFT_SUCCESS;
}

static FFTGeneratedTransposeSquareAction *FFTGeneratedTransposeSquareActionCreate(clfftPlanHandle plHandle, FFTPlan *plan, cl_command_queue queue, clfftStatus *err)
{
	// Allocate action storage and clear it before explicit initialization.
	FFTGeneratedTransposeSquareAction *action = (FFTGeneratedTransposeSquareAction *) clfft_checked_malloc(sizeof(FFTGeneratedTransposeSquareAction));
	memset((void *) action, 0, sizeof(*action));

	// Initialize the common action base and signature payload.
	FFTActionInit(&action->base, plan, Transpose_SQUARE, action, err);
	FFTKernelSignatureTransposeInit(&action->signature);
	if (*err != CLFFT_SUCCESS)
	{
		// Report base initialization failure and leave cleanup to the caller.
		fprintf(stderr, "FFTGeneratedTransposeSquareActionCreate: FFTActionInit failed!\n");
		return action;
	}

	// Initialize the FFTAction kernel-generation parameter member.
	*err = FFTGeneratedTransposeSquareActionInitParams(action);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedTransposeSquareActionInitParams failed!\n");
		return action;
	}

	// Generate the kernel source through the shared repository.
	FFTRepo *fftRepo = FFTRepoGetInstance();
	*err = FFTActionGenerateKernel(&action->base, fftRepo, queue);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedTransposeSquareActionCreate: "
			"FFTActionGenerateKernel failed\n");
		return action;
	}

	// Compile generated kernels for the requested queue and plan.
	*err = FFTActionCompileKernels(&action->base, queue, plHandle, plan);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr,
			"FFTGeneratedTransposeSquareActionCreate: "
			"FFTActionCompileKernels failed\n");
		return action;
	}

	// Mark action creation as successful.
	*err = CLFFT_SUCCESS;
	return action;
}

static bool FFTGeneratedTransposeSquareActionBuildForwardKernel(FFTGeneratedTransposeSquareAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || r2c_transform;
}

static bool FFTGeneratedTransposeSquareActionBuildBackwardKernel(FFTGeneratedTransposeSquareAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool real_transform = (r2c_transform || c2r_transform);

	return (!real_transform) || c2r_transform;
}

/*sqaure action*/
static clfftStatus FFTGeneratedTransposeSquareActionInitParams(FFTGeneratedTransposeSquareAction *action)
{
	action->signature.data.fft_precision = action->base.plan->precision;
	action->signature.data.fft_placeness = action->base.plan->placeness;
	action->signature.data.fft_inputLayout = action->base.plan->inputLayout;
	action->signature.data.fft_outputLayout = action->base.plan->outputLayout;
	action->signature.data.fft_3StepTwiddle = false;

	action->signature.data.fft_realSpecial = action->base.plan->realSpecial;

	action->signature.data.transOutHorizontal = action->base.plan->transOutHorizontal; // using the twiddle front flag to specify
											   // horizontal write we do this so as to reuse
											   // flags in FFTKernelGenKeyParams and to avoid
											   // making a new one

	ARG_CHECK(array_size(&action->base.plan->inStride) == array_size(&action->base.plan->outStride));

	if (CLFFT_INPLACE == action->signature.data.fft_placeness)
	{
		//	If this is an in-place transform the
		//	input and output layout, dimensions and strides
		//	*MUST* be the same.
		//
		ARG_CHECK(action->signature.data.fft_inputLayout == action->signature.data.fft_outputLayout)

		for (size_t u = array_size(&action->base.plan->inStride); u-- > 0;)
		{
			ARG_CHECK(action->base.plan->inStride.buf[u] == action->base.plan->outStride.buf[u]);
		}
	}

	action->signature.data.fft_DataDim = array_size(&action->base.plan->length) + 1;
	int i = 0;
	for (i = 0; i < (action->signature.data.fft_DataDim - 1); i++)
	{
		action->signature.data.fft_N[i] = action->base.plan->length.buf[i];
		action->signature.data.fft_inStride[i] = action->base.plan->inStride.buf[i];
		action->signature.data.fft_outStride[i] = action->base.plan->outStride.buf[i];
	}
	action->signature.data.fft_inStride[i] = action->base.plan->iDist;
	action->signature.data.fft_outStride[i] = action->base.plan->oDist;

	if (action->base.plan->large1D != 0)
	{
		ARG_CHECK(action->signature.data.fft_N[0] != 0)
		ARG_CHECK((action->base.plan->large1D % action->signature.data.fft_N[0]) == 0)
		action->signature.data.fft_3StepTwiddle = true;
		ARG_CHECK(action->base.plan->large1D == (action->signature.data.fft_N[1] * action->signature.data.fft_N[0]));
	}

	//	Query the devices in this context for their local memory sizes
	//	How we generate a kernel depends on the *minimum* LDS size for all
	// devices.
	//
	const FFTEnvelope *pEnvelope = NULL;
	OPENCL_V(FFTPlanGetEnvelope(action->base.plan, &pEnvelope), "GetEnvelope failed");
	BUG_CHECK(NULL != pEnvelope);

	// TODO:  Since I am going with a 2D workgroup size now, I need a better
	// check than this 1D use Check:
	// CL_DEVICE_MAX_WORK_GROUP_SIZE/CL_KERNEL_WORK_GROUP_SIZE
	// CL_DEVICE_MAX_WORK_ITEM_SIZES
	action->signature.data.fft_R = 1;				  // Dont think i'll use
	action->signature.data.fft_SIMD = pEnvelope->limit_WorkGroupSize; // Use devices maximum workgroup size

	// Set callback if specified
	if (action->base.plan->hasPreCallback)
	{
		action->signature.data.fft_hasPreCallback = true;
		action->signature.data.fft_preCallback = action->base.plan->preCallback;
	}
	if (action->base.plan->hasPostCallback)
	{
		action->signature.data.fft_hasPostCallback = true;
		action->signature.data.fft_postCallback = action->base.plan->postCallbackParam;
	}
	action->signature.data.limit_LocalMemSize = action->base.plan->envelope.limit_LocalMemSize;

	action->signature.data.transposeMiniBatchSize = action->base.plan->transposeMiniBatchSize;
	action->signature.data.transposeBatchSize = action->base.plan->batchsize;

	return CLFFT_SUCCESS;
}

//	OpenCL does not take unicode strings as input, so this routine returns only
// ASCII strings 	Feed this generator the FFTPlan, and it returns the
// generated program as a string
static clfftStatus FFTGeneratedTransposeSquareActionGenerateKernel(FFTGeneratedTransposeSquareAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT)
{
	// Requested local memory size by callback must not exceed the device LDS
	// limits after factoring the LDS size required by main FFT kernel
	if ((action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0) ||
		(action->signature.data.fft_hasPostCallback && action->signature.data.fft_postCallback.localMemSize > 0))
	{
		assert(!(action->signature.data.fft_hasPreCallback && action->signature.data.fft_hasPostCallback));

		bool validLDSSize = false;
		size_t requestedCallbackLDS = 0;

		if (action->signature.data.fft_hasPreCallback && action->signature.data.fft_preCallback.localMemSize > 0)
			requestedCallbackLDS = action->signature.data.fft_preCallback.localMemSize;
		else if (action->signature.data.fft_hasPostCallback && action->signature.data.fft_postCallback.localMemSize > 0)
			requestedCallbackLDS = action->signature.data.fft_postCallback.localMemSize;

		validLDSSize = ((2 * FFTPlanElementSize(action->base.plan) * 16 * reShapeFactor * 16 * reShapeFactor) + requestedCallbackLDS) <
			action->base.plan->envelope.limit_LocalMemSize;

		if (!validLDSSize)
		{
			fprintf(stderr, "Requested local memory size not available\n");
			return CLFFT_INVALID_ARG_VALUE;
		}
	}

	buffer_t programCode = buffer_empty();
	OPENCL_V(clfft_transpose_generator_genTransposeKernelBatched(action->signature.data, &programCode, lwSize, reShapeFactor), "GenerateTransposeKernel() failed!");

	cl_int status = CL_SUCCESS;
	cl_device_id Device = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_DEVICE, sizeof(cl_device_id), &Device, NULL);
	OPENCL_V(status, "clGetCommandQueueInfo failed");

	cl_context QueueContext = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_CONTEXT, sizeof(cl_context), &QueueContext, NULL);
	OPENCL_V(status, "clGetCommandQueueInfo failed");

	OPENCL_V(FFTRepoSetProgramCode(fftRepo, Transpose_SQUARE, &action->signature.header, &programCode, Device, QueueContext), "FFTRepoSetProgramCode failed!");

	// Note:  See genFunctionPrototype( )
	if (action->signature.data.fft_3StepTwiddle)
	{
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_SQUARE, &action->signature.header, "transpose_square_tw_fwd", "transpose_square_tw_back", Device,
				 QueueContext),
			"FFTRepoSetProgramEntryPoints failed!");
	}
	else
	{
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, Transpose_SQUARE, &action->signature.header, "transpose_square", "transpose_square", Device, QueueContext),
			"FFTRepoSetProgramEntryPoints failed!");
	}

	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedTransposeSquareActionGetWorkSizes(FFTGeneratedTransposeSquareAction *action, array_size_t *globalWS, array_size_t *localWS)
{
	size_t wg_slice;
	if (action->signature.data.fft_N[0] % (16 * reShapeFactor) == 0)
		wg_slice = action->signature.data.fft_N[0] / 16 / reShapeFactor;
	else
		wg_slice = (action->signature.data.fft_N[0] / (16 * reShapeFactor)) + 1;

	size_t global_item_size = wg_slice * (wg_slice + 1) / 2 * 16 * 16 * action->base.plan->batchsize;

	for (int i = 2; i < action->signature.data.fft_DataDim - 1; i++)
		global_item_size *= action->signature.data.fft_N[i];

	array_clear(globalWS);
	array_push_back(globalWS, global_item_size);

	array_clear(localWS);
	array_push_back(localWS, lwSize);

	return CLFFT_SUCCESS;
}
/* End copied source: src\library\action.transpose.cpp */

/* Begin copied source: src\library\generator.copy.cpp */

static FFTGeneratedCopyAction *FFTGeneratedCopyActionCreate(clfftPlanHandle plHandle, FFTPlan *plan, cl_command_queue queue, clfftStatus *err)
{
	// Allocate action storage and clear it before explicit initialization.
	FFTGeneratedCopyAction *action = (FFTGeneratedCopyAction *) clfft_checked_malloc(sizeof(FFTGeneratedCopyAction));
	memset((void *) action, 0, sizeof(*action));

	// Initialize the common action base and signature payload.
	FFTActionInit(&action->base, plan, Copy, action, err);
	FFTKernelSignatureCopyInit(&action->signature);
	if (*err != CLFFT_SUCCESS)
	{
		// Report base initialization failure and leave cleanup to the caller.
		fprintf(stderr, "FFTGeneratedCopyActionCreate: FFTActionInit failed!\n");
		return action;
	}

	// Initialize the FFTAction kernel-generation parameter member.
	*err = FFTGeneratedCopyActionInitParams(action);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedCopyActionInitParams failed!\n");
		return action;
	}

	// Generate the kernel source through the shared repository.
	FFTRepo *fftRepo = FFTRepoGetInstance();
	*err = FFTActionGenerateKernel(&action->base, fftRepo, queue);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedCopyActionCreate: FFTActionGenerateKernel failed\n");
		return action;
	}

	// Compile generated kernels for the requested queue and plan.
	*err = FFTActionCompileKernels(&action->base, queue, plHandle, plan);
	if (*err != CLFFT_SUCCESS)
	{
		fprintf(stderr, "FFTGeneratedCopyActionCreate: FFTActionCompileKernels failed\n");
		return action;
	}

	// Mark action creation as successful.
	*err = CLFFT_SUCCESS;
	return action;
}

static bool FFTGeneratedCopyActionBuildForwardKernel(FFTGeneratedCopyAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool r2c_transform = (inputLayout == CLFFT_REAL);
	bool h2c = (inputLayout == CLFFT_HERMITIAN_PLANAR) || (inputLayout == CLFFT_HERMITIAN_INTERLEAVED);
	bool c2h = (outputLayout == CLFFT_HERMITIAN_PLANAR) || (outputLayout == CLFFT_HERMITIAN_INTERLEAVED);

	return (r2c_transform || c2h) || (!(h2c || c2h));
}

static bool FFTGeneratedCopyActionBuildBackwardKernel(FFTGeneratedCopyAction *action)
{
	clfftLayout inputLayout = action->signature.data.fft_inputLayout;
	clfftLayout outputLayout = action->signature.data.fft_outputLayout;

	bool c2r_transform = (outputLayout == CLFFT_REAL);
	bool h2c = (inputLayout == CLFFT_HERMITIAN_PLANAR) || (inputLayout == CLFFT_HERMITIAN_INTERLEAVED);
	bool c2h = (outputLayout == CLFFT_HERMITIAN_PLANAR) || (outputLayout == CLFFT_HERMITIAN_INTERLEAVED);

	return (c2r_transform || h2c) || (!(h2c || c2h));
}

// Copy kernel
typedef struct CopyKernel
{
	Precision pr;
	size_t N;
	size_t Nt;
	FFTKernelGenKeyParams params;
	bool h2c, c2h;
	bool general;
} CopyKernel;

static inline buffer_t CopyKernelOffsetCalc(const CopyKernel *kernel, const char *off, bool input)
{
	// Generate index-offset source for the selected buffer side.
	buffer_t str = buffer_empty();

	const size_t *pStride = input ? kernel->params.fft_inStride : kernel->params.fft_outStride;

	bufcatcstr(&str, "\t");
	bufcatcstr(&str, off);
	bufcatcstr(&str, " = ");
	buffer_t nextBatch = buffer_from_cstr("batch");
	for (size_t i = (kernel->params.fft_DataDim - 1); i > 1; i--)
	{
		size_t currentLength = 1;
		for (int j = 1; j < i; j++)
			currentLength *= kernel->params.fft_N[j];

		bufcatcstr(&str, "(");
		bufcatbuf(&str, &nextBatch);
		bufcatcstr(&str, "/");
		BUFCAT_BUFFER_VALUE(&str, SztToStr(currentLength));
		bufcatcstr(&str, ")*");
		BUFCAT_BUFFER_VALUE(&str, SztToStr(pStride[i]));
		bufcatcstr(&str, " + ");

		// Rebuild the next batch expression without temporary merge helpers.
		buffer_t nextBatchOld = buffer_empty();
		buffer_t currentLengthStr = SztToStr(currentLength);
		bufsetbuf(&nextBatchOld, &nextBatch);
		bufsetcstr(&nextBatch, "(");
		bufcatbuf(&nextBatch, &nextBatchOld);
		bufcatcstr(&nextBatch, "%");
		bufcatbuf(&nextBatch, &currentLengthStr);
		bufcatcstr(&nextBatch, ")");
	}

	bufcatbuf(&str, &nextBatch);
	bufcatcstr(&str, "*");
	BUFCAT_BUFFER_VALUE(&str, SztToStr(pStride[1]));
	bufcatcstr(&str, ";\n");

	return str;
}

static inline void CopyKernelInit(CopyKernel *kernel, Precision prVal, const FFTKernelGenKeyParams paramsVal)
{
	// Initialize copy-kernel generation state from the FFT signature.
	kernel->pr = prVal;
	kernel->params = paramsVal;
	kernel->N = kernel->params.fft_N[0];
	kernel->Nt = 1 + kernel->N / 2;
	kernel->h2c = ((kernel->params.fft_inputLayout == CLFFT_HERMITIAN_PLANAR) || (kernel->params.fft_inputLayout == CLFFT_HERMITIAN_INTERLEAVED)) ? true : false;
	kernel->c2h = ((kernel->params.fft_outputLayout == CLFFT_HERMITIAN_PLANAR) || (kernel->params.fft_outputLayout == CLFFT_HERMITIAN_INTERLEAVED)) ? true : false;
	kernel->general = !(kernel->h2c || kernel->c2h);

	// Assert that only the supported out-of-place copy form is generated.
	assert(kernel->params.fft_placeness == CLFFT_OUTOFPLACE);
}

static void CopyKernelGenerateKernel(const CopyKernel *kernel, buffer_t *str)
{
	// Generate OpenCL source for a copy kernel from explicit state.
	buffer_t rType = RegBaseType(kernel->pr, 1);
	buffer_t r2Type = RegBaseType(kernel->pr, 2);

	bool inIlvd;  // Input is interleaved format
	bool outIlvd; // Output is interleaved format
	inIlvd = ((kernel->params.fft_inputLayout == CLFFT_COMPLEX_INTERLEAVED) || (kernel->params.fft_inputLayout == CLFFT_HERMITIAN_INTERLEAVED)) ? true : false;
	outIlvd = ((kernel->params.fft_outputLayout == CLFFT_COMPLEX_INTERLEAVED) || (kernel->params.fft_outputLayout == CLFFT_HERMITIAN_INTERLEAVED)) ? true : false;

	// Pragma
	BUFCAT_BUFFER_VALUE(str, ClPragma(kernel->pr));

	buffer_t sfx = FloatSuffix(kernel->pr);

	// If pre-callback is set for the plan
	if (kernel->params.fft_hasPreCallback && kernel->h2c)
	{
		// Insert callback function code at the beginning
		bufcatcstr(str, kernel->params.fft_preCallback.funcstring);
		bufcatcstr(str, "\n\n");
	}

	// if postcallback is set
	if (kernel->params.fft_hasPostCallback)
	{
		// Insert callback function code at the beginning
		bufcatcstr(str, kernel->params.fft_postCallback.funcstring);
		bufcatcstr(str, "\n\n");
	}

	// Copy kernel begin
	bufcatcstr(str, "__kernel void ");

	// Function name
	if (kernel->general)
		bufcatcstr(str, "copy_general");
	else if (kernel->h2c)
		bufcatcstr(str, "copy_h2c");
	else
		bufcatcstr(str, "copy_c2h");

	bufcatcstr(str, "(");

	if (inIlvd)
	{
		bufcatcstr(str, "__global const ");
		bufcatbuf(str, &r2Type);
		bufcatcstr(str, " * restrict gbIn, ");
	}
	else
	{
		bufcatcstr(str, "__global const ");
		bufcatbuf(str, &rType);
		bufcatcstr(str, " * restrict gbInRe, ");
		bufcatcstr(str, "__global const ");
		bufcatbuf(str, &rType);
		bufcatcstr(str, " * restrict gbInIm, ");
	}

	if (outIlvd)
	{
		bufcatcstr(str, "__global ");
		bufcatbuf(str, &r2Type);
		bufcatcstr(str, " * restrict gbOut");
	}
	else
	{
		bufcatcstr(str, "__global ");
		bufcatbuf(str, &rType);
		bufcatcstr(str, " * restrict gbOutRe, ");
		bufcatcstr(str, "__global ");
		bufcatbuf(str, &rType);
		bufcatcstr(str, " * restrict gbOutIm");
	}

	if (kernel->params.fft_hasPreCallback && kernel->h2c)
	{
		assert(!kernel->params.fft_hasPostCallback);

		bufcatcstr(str, ", __global void* pre_userdata");
		if (kernel->params.fft_preCallback.localMemSize > 0)
			bufcatcstr(str, ", __local void* localmem");
	}

	if (kernel->params.fft_hasPostCallback)
	{
		assert(!kernel->params.fft_hasPreCallback);

		bufcatcstr(str, ", __global void* post_userdata");
		if (kernel->params.fft_postCallback.localMemSize > 0)
			bufcatcstr(str, ", __local void* localmem");
	}

	bufcatcstr(str, ")\n");

	bufcatcstr(str, "{\n");

	// Initialize
	if (kernel->general)
	{
		bufcatcstr(str, "\tuint me = get_local_id(0);\n\t");
		bufcatcstr(str, "uint batch = get_group_id(0);\n\t");
	}
	else
	{
		bufcatcstr(str, "\tuint me = get_global_id(0);\n\t");
	}

	// Declare memory pointers
	bufcatcstr(str, "\n\t");
	bufcatcstr(str, "uint iOffset;\n\t");
	bufcatcstr(str, "uint oOffset;\n\t");

	if (!(kernel->params.fft_hasPreCallback && kernel->h2c))
	{
		// input
		if (inIlvd)
		{
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &r2Type);
			bufcatcstr(str, " *lwbIn;\n\t");
		}
		else
		{
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &rType);
			bufcatcstr(str, " *lwbInRe;\n\t");
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &rType);
			bufcatcstr(str, " *lwbInIm;\n\t");
		}
	}

	// output
	if (outIlvd)
	{
		if (!kernel->params.fft_hasPostCallback)
		{
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &r2Type);
			bufcatcstr(str, " *lwbOut;\n");
		}
		if (kernel->h2c)
		{
			bufcatcstr(str, "\t");
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &r2Type);
			bufcatcstr(str, " *lwbOut2;\n\n");
		}
	}
	else
	{
		if (!kernel->params.fft_hasPostCallback)
		{
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &rType);
			bufcatcstr(str, " *lwbOutRe;\n\t");
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &rType);
			bufcatcstr(str, " *lwbOutIm;\n");
		}
		if (kernel->h2c)
		{
			bufcatcstr(str, "\t");
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &rType);
			bufcatcstr(str, " *lwbOutRe2;\n\t");
			bufcatcstr(str, "__global ");
			bufcatbuf(str, &rType);
			bufcatcstr(str, " *lwbOutIm2;\n\n");
		}
	}

	// Setup registers
	bufcatcstr(str, "\t");
	BUFCAT_BUFFER_VALUE(str, RegBaseType(kernel->pr, 2));
	bufcatcstr(str, " R;\n\n");

	size_t NtRounded64 = DivRoundingUpSizeT(kernel->Nt, 64) * 64;

	if (!kernel->general)
	{
		// Setup variables
		bufcatcstr(str, "\tuint batch, meg, mel, mel2;\n\t");
		bufcatcstr(str, "batch = me/");
		BUFCAT_BUFFER_VALUE(str, SztToStr(NtRounded64));
		bufcatcstr(str, ";\n\t");
		bufcatcstr(str, "meg = me%");
		BUFCAT_BUFFER_VALUE(str, SztToStr(NtRounded64));
		bufcatcstr(str, ";\n\t");
		bufcatcstr(str, "mel = me%");
		BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->Nt));
		bufcatcstr(str, ";\n\t");
		bufcatcstr(str, "mel2 = (");
		BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->N));
		bufcatcstr(str, " - mel)%");
		BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->N));
		bufcatcstr(str, ";\n\n");
	}

	// Setup memory pointers
	BUFCAT_BUFFER_VALUE(str, CopyKernelOffsetCalc(kernel, "iOffset", true));
	BUFCAT_BUFFER_VALUE(str, CopyKernelOffsetCalc(kernel, "oOffset", false));

	// offset strings
	buffer_t inF = buffer_empty();
	buffer_t inF2 = buffer_empty();
	buffer_t outF = buffer_empty();
	buffer_t outF2 = buffer_empty();
	if (kernel->general)
	{
		bufsetcstr(&inF, "");
		bufsetcstr(&inF2, "");
		bufsetcstr(&outF, "");
		bufsetcstr(&outF2, "");
	}
	else
	{
		bufsetcstr(&inF, " + (mel*");
		BUFCAT_BUFFER_VALUE(&inF, SztToStr(kernel->params.fft_inStride[0]));
		bufcatcstr(&inF, ")");
		bufsetcstr(&inF2, " + (mel2*");
		BUFCAT_BUFFER_VALUE(&inF2, SztToStr(kernel->params.fft_inStride[0]));
		bufcatcstr(&inF2, ")");
		bufsetcstr(&outF, " + (mel*");
		BUFCAT_BUFFER_VALUE(&outF, SztToStr(kernel->params.fft_outStride[0]));
		bufcatcstr(&outF, ")");
		bufsetcstr(&outF2, " + (mel2*");
		BUFCAT_BUFFER_VALUE(&outF2, SztToStr(kernel->params.fft_outStride[0]));
		bufcatcstr(&outF2, ")");
	}

	bufcatcstr(str, "\n\t");

	if (!(kernel->params.fft_hasPreCallback && kernel->h2c))
	{
		// inputs
		if (inIlvd)
		{
			bufcatcstr(str, "lwbIn = gbIn + iOffset");
			bufcatbuf(str, &inF);
			bufcatcstr(str, ";\n\t");
		}
		else
		{
			bufcatcstr(str, "lwbInRe = gbInRe + iOffset");
			bufcatbuf(str, &inF);
			bufcatcstr(str, ";\n\t");
			bufcatcstr(str, "lwbInIm = gbInIm + iOffset");
			bufcatbuf(str, &inF);
			bufcatcstr(str, ";\n\t");
		}
	}

	// outputs
	if (outIlvd)
	{
		if (!kernel->params.fft_hasPostCallback)
		{
			bufcatcstr(str, "lwbOut = gbOut + oOffset");
			bufcatbuf(str, &outF);
			bufcatcstr(str, ";\n");
		}
		if (kernel->h2c)
		{
			bufcatcstr(str, "\t");
			bufcatcstr(str, "lwbOut2 = gbOut + oOffset");
			bufcatbuf(str, &outF2);
			bufcatcstr(str, ";\n");
		}
	}
	else
	{
		if (!kernel->params.fft_hasPostCallback)
		{
			bufcatcstr(str, "lwbOutRe = gbOutRe + oOffset");
			bufcatbuf(str, &outF);
			bufcatcstr(str, ";\n\t");
			bufcatcstr(str, "lwbOutIm = gbOutIm + oOffset");
			bufcatbuf(str, &outF);
			bufcatcstr(str, ";\n");
		}
		if (kernel->h2c)
		{
			bufcatcstr(str, "\t");
			bufcatcstr(str, "lwbOutRe2 = gbOutRe + oOffset");
			bufcatbuf(str, &outF2);
			bufcatcstr(str, ";\n\t");
			bufcatcstr(str, "lwbOutIm2 = gbOutIm + oOffset");
			bufcatbuf(str, &outF2);
			bufcatcstr(str, ";\n");
		}
	}

	bufcatcstr(str, "\n\t");

	// Do the copy
	if (kernel->general)
	{
		bufcatcstr(str, "for(uint t=0; t<");
		BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->N / 64));
		bufcatcstr(str, "; t++)\n\t{\n\t\t");

		if (inIlvd)
		{
			bufcatcstr(str, "R = lwbIn[me + t*64];\n\t\t");
		}
		else
		{
			bufcatcstr(str, "R.x = lwbInRe[me + t*64];\n\t\t");
			bufcatcstr(str, "R.y = lwbInIm[me + t*64];\n\t\t");
		}

		if (outIlvd)
		{
			bufcatcstr(str, "lwbOut[me + t*64] = R;\n");
		}
		else
		{
			bufcatcstr(str, "lwbOutRe[me + t*64] = R.x;\n\t\t");
			bufcatcstr(str, "lwbOutIm[me + t*64] = R.y;\n");
		}

		bufcatcstr(str, "\t}\n\n");
	}
	else
	{
		bufcatcstr(str, "if(meg < ");
		BUFCAT_BUFFER_VALUE(str, SztToStr(kernel->Nt));
		bufcatcstr(str, ")\n\t{\n\t");
		if (kernel->c2h)
		{
			if (inIlvd)
			{
				bufcatcstr(str, "R = lwbIn[0];\n\t");
			}
			else
			{
				bufcatcstr(str, "R.x = lwbInRe[0];\n\t");
				bufcatcstr(str, "R.y = lwbInIm[0];\n\t");
			}

			if (outIlvd)
			{
				if (kernel->params.fft_hasPostCallback)
				{
					bufcatcstr(str, kernel->params.fft_postCallback.funcname);
					bufcatcstr(str, "(gbOut, oOffset");
					bufcatbuf(str, &outF);
					bufcatcstr(str, ", post_userdata, R");
					if (kernel->params.fft_postCallback.localMemSize > 0)
						bufcatcstr(str, ", localmem");
					bufcatcstr(str, ");\n\n");
				}
				else
				{
					bufcatcstr(str, "lwbOut[0] = R;\n\n");
				}
			}
			else if (kernel->params.fft_hasPostCallback)
			{
				bufcatcstr(str, kernel->params.fft_postCallback.funcname);
				bufcatcstr(str, "(gbOutRe, gbOutIm, oOffset");
				bufcatbuf(str, &outF);
				bufcatcstr(str, ", post_userdata, R.x, R.y");

				if (kernel->params.fft_postCallback.localMemSize > 0)
					bufcatcstr(str, ", localmem");
				bufcatcstr(str, ");\n\t");
			}
			else
			{
				bufcatcstr(str, "lwbOutRe[0] = R.x;\n\t");
				bufcatcstr(str, "lwbOutIm[0] = R.y;\n\t");
			}
		}
		else
		{
			if (kernel->params.fft_hasPreCallback)
			{
				if (inIlvd)
				{
					bufcatcstr(str, "R = ");
					bufcatcstr(str, kernel->params.fft_preCallback.funcname);
					bufcatcstr(str, "( gbIn, (iOffset");
					bufcatbuf(str, &inF);
					bufcatcstr(str, "), pre_userdata");
				}
				else
				{
					bufcatcstr(str, "R = ");
					bufcatcstr(str, kernel->params.fft_preCallback.funcname);
					bufcatcstr(str, "( gbInRe, gbInIm, (iOffset");
					bufcatbuf(str, &inF);
					bufcatcstr(str, "), pre_userdata");
				}
				if (kernel->params.fft_preCallback.localMemSize > 0)
					bufcatcstr(str, ", localmem");
				bufcatcstr(str, ");\n\t\t");
			}
			else if (inIlvd)
			{
				bufcatcstr(str, "R = lwbIn[0];\n\t");
			}
			else
			{
				bufcatcstr(str, "R.x = lwbInRe[0];\n\t");
				bufcatcstr(str, "R.y = lwbInIm[0];\n\t");
			}

			if (outIlvd)
			{
				bufcatcstr(str, "lwbOut[0] = R;\n\t");
				bufcatcstr(str, "R.y = -R.y;\n\t");
				bufcatcstr(str, "lwbOut2[0] = R;\n\t");
			}
			else
			{
				bufcatcstr(str, "lwbOutRe[0] = R.x;\n\t");
				bufcatcstr(str, "lwbOutIm[0] = R.y;\n\t");
				bufcatcstr(str, "R.y = -R.y;\n\t");
				bufcatcstr(str, "lwbOutRe2[0] = R.x;\n\t");
				bufcatcstr(str, "lwbOutIm2[0] = R.y;\n\t");
			}
		}
		bufcatcstr(str, "}\n\n");
	}

	bufcatcstr(str, "}\n");
}

static clfftStatus FFTGeneratedCopyActionInitParams(FFTGeneratedCopyAction *action)
{
	//    Query the devices in this context for their local memory sizes
	//    How we generate a kernel depends on the *minimum* LDS size for all
	//    devices.
	//
	const FFTEnvelope *pEnvelope = NULL;
	OPENCL_V(FFTPlanGetEnvelope(action->base.plan, &pEnvelope), "GetEnvelope failed");
	BUG_CHECK(NULL != pEnvelope);

	action->signature.data.fft_precision = action->base.plan->precision;
	action->signature.data.fft_placeness = action->base.plan->placeness;
	action->signature.data.fft_inputLayout = action->base.plan->inputLayout;
	action->signature.data.fft_MaxWorkGroupSize = action->base.plan->envelope.limit_WorkGroupSize;

	ARG_CHECK(array_size(&action->base.plan->inStride) == array_size(&action->base.plan->outStride))

	action->signature.data.fft_outputLayout = action->base.plan->outputLayout;

	action->signature.data.fft_DataDim = array_size(&action->base.plan->length) + 1;
	int i = 0;
	for (i = 0; i < (action->signature.data.fft_DataDim - 1); i++)
	{
		action->signature.data.fft_N[i] = action->base.plan->length.buf[i];
		action->signature.data.fft_inStride[i] = action->base.plan->inStride.buf[i];
		action->signature.data.fft_outStride[i] = action->base.plan->outStride.buf[i];
	}
	action->signature.data.fft_inStride[i] = action->base.plan->iDist;
	action->signature.data.fft_outStride[i] = action->base.plan->oDist;

	action->signature.data.fft_fwdScale = action->base.plan->forwardScale;
	action->signature.data.fft_backScale = action->base.plan->backwardScale;

	// Set callback if specified
	if (action->base.plan->hasPreCallback)
	{
		assert(!action->base.plan->hasPostCallback);

		action->signature.data.fft_hasPreCallback = true;
		action->signature.data.fft_preCallback = action->base.plan->preCallback;

		// Requested local memory size by callback must not exceed the device
		// LDS limits after factoring the LDS size required by main FFT kernel
		if (action->base.plan->preCallback.localMemSize > action->base.plan->envelope.limit_LocalMemSize)
		{
			fprintf(stderr, "Requested local memory size not available\n");
			return CLFFT_INVALID_ARG_VALUE;
		}
	}

	if (action->base.plan->hasPostCallback)
	{
		assert(!action->base.plan->hasPreCallback);

		action->signature.data.fft_hasPostCallback = true;
		action->signature.data.fft_postCallback = action->base.plan->postCallbackParam;

		// Requested local memory size by callback must not exceed the device
		// LDS limits after factoring the LDS size required by main FFT kernel
		// Assumes copy kernel does not have both pre and post callback
		if (action->base.plan->postCallbackParam.localMemSize > action->base.plan->envelope.limit_LocalMemSize)
		{
			fprintf(stderr, "Requested local memory size not available\n");
			return CLFFT_INVALID_ARG_VALUE;
		}
	}
	action->signature.data.limit_LocalMemSize = action->base.plan->envelope.limit_LocalMemSize;

	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedCopyActionGetWorkSizes(FFTGeneratedCopyAction *action, array_size_t *globalWS, array_size_t *localWS)
{
	bool h2c, c2h;
	h2c = ((action->signature.data.fft_inputLayout == CLFFT_HERMITIAN_PLANAR) || (action->signature.data.fft_inputLayout == CLFFT_HERMITIAN_INTERLEAVED));
	c2h = ((action->signature.data.fft_outputLayout == CLFFT_HERMITIAN_PLANAR) || (action->signature.data.fft_outputLayout == CLFFT_HERMITIAN_INTERLEAVED));

	bool general = !(h2c || c2h);

	size_t count = action->base.plan->batchsize;

	switch (action->signature.data.fft_DataDim)
	{
		case 5: assert(false);
		case 4: count *= action->signature.data.fft_N[2];
		case 3: count *= action->signature.data.fft_N[1];
		case 2:
		{
			if (general)
			{
				count *= 64;
			}
			else
			{
				count *= (DivRoundingUpSizeT((1 + action->signature.data.fft_N[0] / 2), 64) * 64);
			}
		}
		break;
		case 1: assert(false);
	}

	array_push_back(globalWS, count);
	array_push_back(localWS, 64);

	return CLFFT_SUCCESS;
}

static clfftStatus FFTGeneratedCopyActionGenerateKernel(FFTGeneratedCopyAction *action, FFTRepo *fftRepo, const cl_command_queue commQueueFFT)
{
	bool h2c, c2h;
	h2c = ((action->signature.data.fft_inputLayout == CLFFT_HERMITIAN_PLANAR) || (action->signature.data.fft_inputLayout == CLFFT_HERMITIAN_INTERLEAVED));
	c2h = ((action->signature.data.fft_outputLayout == CLFFT_HERMITIAN_PLANAR) || (action->signature.data.fft_outputLayout == CLFFT_HERMITIAN_INTERLEAVED));

	bool general = !(h2c || c2h);

	buffer_t programCode = buffer_empty();
	Precision pr = (action->signature.data.fft_precision == CLFFT_SINGLE) ? P_SINGLE : P_DOUBLE;
	switch (pr)
	{
		case P_SINGLE:
		{
			CopyKernel kernel;
			CopyKernelInit(&kernel, P_SINGLE, action->signature.data);
			CopyKernelGenerateKernel(&kernel, &programCode);
		}
		break;
		case P_DOUBLE:
		{
			CopyKernel kernel;
			CopyKernelInit(&kernel, P_DOUBLE, action->signature.data);
			CopyKernelGenerateKernel(&kernel, &programCode);
		}
		break;
	}

	cl_int status = CL_SUCCESS;
	cl_device_id Device = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_DEVICE, sizeof(cl_device_id), &Device, NULL);
	OPENCL_V(status, _T( "clGetCommandQueueInfo failed" ));

	cl_context QueueContext = NULL;
	status = clGetCommandQueueInfo(commQueueFFT, CL_QUEUE_CONTEXT, sizeof(cl_context), &QueueContext, NULL);
	OPENCL_V(status, _T( "clGetCommandQueueInfo failed" ));

	OPENCL_V(FFTRepoSetProgramCode(fftRepo, FFTActionGetGenerator(&action->base), &action->signature.header, &programCode, Device, QueueContext),
		_T( "FFTRepoSetProgramCode failed!" ));

	if (general)
	{
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, FFTActionGetGenerator(&action->base), &action->signature.header, "copy_general", "copy_general", Device,
				 QueueContext),
			_T( "FFTRepoSetProgramEntryPoints failed!" ));
	}
	else
	{
		OPENCL_V(FFTRepoSetProgramEntryPoints(fftRepo, FFTActionGetGenerator(&action->base), &action->signature.header, "copy_c2h", "copy_h2c", Device, QueueContext),
			_T( "FFTRepoSetProgramEntryPoints failed!" ));
	}

	return CLFFT_SUCCESS;
}
/* End copied source: src\library\generator.copy.cpp */

/* Begin copied source: src\library\enqueue.cpp */

static clfftStatus FFTActionSelectBufferArguments(FFTPlan *fftPlan, cl_mem *clInputBuffers, cl_mem *clOutputBuffers, array_cl_mem *inputBuff, array_cl_mem *outputBuff)
{
	// Reserve space for the maximum input/output buffer count used here.
	array_reserve(inputBuff, 2);
	array_reserve(outputBuff, 2);

	//	Decode the relevant properties from the plan paramter to figure out how
	// many input/output buffers we have
	switch (fftPlan->inputLayout)
	{
		case CLFFT_COMPLEX_INTERLEAVED:
		{
			switch (fftPlan->outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						array_push_back(inputBuff, clInputBuffers[0]);
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_COMPLEX_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						//	Invalid to be an inplace transform, and go from 1 to 2
						// buffers
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_HERMITIAN_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_HERMITIAN_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_REAL:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						array_push_back(inputBuff, clInputBuffers[0]);
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				default:
				{
					//	Don't recognize output layout
					return CLFFT_INVALID_ARG_VALUE;
				}
			}

			break;
		}
		case CLFFT_COMPLEX_PLANAR:
		{
			switch (fftPlan->outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_COMPLEX_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_HERMITIAN_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_HERMITIAN_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_REAL:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				default:
				{
					//	Don't recognize output layout
					return CLFFT_INVALID_ARG_VALUE;
				}
			}

			break;
		}
		case CLFFT_HERMITIAN_INTERLEAVED:
		{
			switch (fftPlan->outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_COMPLEX_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_HERMITIAN_INTERLEAVED:
				{
					return CLFFT_INVALID_ARG_VALUE;
				}
				case CLFFT_HERMITIAN_PLANAR:
				{
					return CLFFT_INVALID_ARG_VALUE;
				}
				case CLFFT_REAL:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						array_push_back(inputBuff, clInputBuffers[0]);
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				default:
				{
					//	Don't recognize output layout
					return CLFFT_INVALID_ARG_VALUE;
				}
			}

			break;
		}
		case CLFFT_HERMITIAN_PLANAR:
		{
			switch (fftPlan->outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_COMPLEX_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_HERMITIAN_INTERLEAVED:
				{
					return CLFFT_INVALID_ARG_VALUE;
				}
				case CLFFT_HERMITIAN_PLANAR:
				{
					return CLFFT_INVALID_ARG_VALUE;
				}
				case CLFFT_REAL:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(inputBuff, clInputBuffers[1]);

						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				default:
				{
					//	Don't recognize output layout
					return CLFFT_INVALID_ARG_VALUE;
				}
			}

			break;
		}
		case CLFFT_REAL:
		{
			switch (fftPlan->outputLayout)
			{
				case CLFFT_COMPLEX_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						array_push_back(inputBuff, clInputBuffers[0]);
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_COMPLEX_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				case CLFFT_HERMITIAN_INTERLEAVED:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						array_push_back(inputBuff, clInputBuffers[0]);
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[0]);
					}

					break;
				}
				case CLFFT_HERMITIAN_PLANAR:
				{
					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						return CLFFT_INVALID_ARG_VALUE;
					}
					else
					{
						array_push_back(inputBuff, clInputBuffers[0]);

						array_push_back(outputBuff, clOutputBuffers[0]);
						array_push_back(outputBuff, clOutputBuffers[1]);
					}

					break;
				}
				default:
				{
					if (fftPlan->transflag)
					{
						if (fftPlan->placeness == CLFFT_INPLACE)
						{
							return CLFFT_INVALID_ARG_VALUE;
						}
						else
						{
							array_push_back(inputBuff, clInputBuffers[0]);
							array_push_back(outputBuff, clOutputBuffers[0]);
						}
					}
					else
					{
						//	Don't recognize output layout
						return CLFFT_INVALID_ARG_VALUE;
					}
				}
			}

			break;
		}
		default:
		{
			//	Don't recognize output layout
			return CLFFT_INVALID_ARG_VALUE;
		}
	}

	return CLFFT_SUCCESS;
}

static clfftStatus FFTActionEnqueue(FFTAction *action, clfftPlanHandle plHandle, clfftDirection dir, cl_uint numQueuesAndEvents, cl_command_queue *commQueues,
	cl_uint numWaitEvents, const cl_event *waitEvents, cl_event *outEvents, cl_mem *clInputBuffers, cl_mem *clOutputBuffers)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();

	array_cl_mem inputBuff;
	// Initialize dynamic array storage explicitly.
	array_init(&inputBuff);
	array_cl_mem outputBuff;
	// Initialize dynamic array storage explicitly.
	array_init(&outputBuff);

	clfftStatus status = FFTActionSelectBufferArguments(action->plan, clInputBuffers, clOutputBuffers, &inputBuff, &outputBuff);

	if (status != CLFFT_SUCCESS)
		return status;

	//	TODO:  In the case of length == 1, FFT is a trivial NOP, but we still
	// need to apply the forward and backwards tranforms
	//	TODO:  Are map lookups expensive to call here?  We can cache a pointer
	// to the cl_program/cl_kernel in the plan

	//	Translate the user plan into the structure that we use to map plans to
	// clPrograms

	cl_program prog;
	cl_kernel kern;
	lockRAII *kernelLock;
	OPENCL_V(FFTRepoGetclProgram(fftRepo, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), &prog, action->plan->bakeDevice, action->plan->context),
		_T( "FFTRepoGetclProgram failed" ));
	OPENCL_V(FFTRepoGetclKernel(fftRepo, prog, dir, &kern, &kernelLock), _T( "FFTRepoGetclKernel failed" ));

	// Enter the kernel lock until the status helper releases it.
	lockRAII *sLock = kernelLock;
	lockRAIIEnter(sLock);

	cl_uint uarg = 0;
	if (!action->plan->transflag && !(action->plan->gen == Copy))
	{
		//	clSetKernelArg() is not thread safe, according to the openCL spec
		// for the same cl_kernel object
		//	TODO:  Need to verify that two different plans (which would get
		// through our lock above) with exactly the same 	parameters would NOT
		// share the same cl_kernel objects

		/* constant buffer */
		OPENCL_V_LOCKED(sLock, clSetKernelArg(kern, uarg++, sizeof(cl_mem), (void *) &action->plan->const_buffer), _T( "clSetKernelArg failed" ));
	}

	//	Input buffer(s)
	//	Input may be 1 buffer  (CLFFT_COMPLEX_INTERLEAVED)
	//	          or 2 buffers (CLFFT_COMPLEX_PLANAR)

	for (size_t i = 0; i < array_size(&inputBuff); ++i)
	{
		OPENCL_V_LOCKED(sLock, clSetKernelArg(kern, uarg++, sizeof(cl_mem), (void *) &inputBuff.buf[i]), _T( "clSetKernelArg failed" ));
	}
	//	Output buffer(s)
	//	Output may be 0 buffers (CLFFT_INPLACE)
	//	           or 1 buffer  (CLFFT_COMPLEX_INTERLEAVED)
	//	           or 2 buffers (CLFFT_COMPLEX_PLANAR)
	for (size_t o = 0; o < array_size(&outputBuff); ++o)
	{
		OPENCL_V_LOCKED(sLock, clSetKernelArg(kern, uarg++, sizeof(cl_mem), (void *) &outputBuff.buf[o]), _T( "clSetKernelArg failed" ));
	}

	// If callback function is set for the plan, pass the appropriate aruments
	if (action->plan->hasPreCallback || action->plan->hasPostCallback)
	{
		if (action->plan->hasPreCallback)
		{
			OPENCL_V_LOCKED(sLock, clSetKernelArg(kern, uarg++, sizeof(cl_mem), (void *) &action->plan->precallUserData), _T( "clSetKernelArg failed" ));
		}

		// If post-callback function is set for the plan, pass the appropriate
		// aruments
		if (action->plan->hasPostCallback)
		{
			OPENCL_V_LOCKED(sLock, clSetKernelArg(kern, uarg++, sizeof(cl_mem), (void *) &action->plan->postcallUserData), _T( "clSetKernelArg failed" ));
		}

		// Pass LDS size arument if set
		if ((action->plan->hasPreCallback && action->plan->preCallback.localMemSize > 0) ||
			(action->plan->hasPostCallback && action->plan->postCallbackParam.localMemSize > 0))
		{
			int localmemSize = 0;
			if (action->plan->hasPreCallback && action->plan->preCallback.localMemSize > 0)
				localmemSize = action->plan->preCallback.localMemSize;
			if (action->plan->hasPostCallback && action->plan->postCallbackParam.localMemSize > 0)
				localmemSize += action->plan->postCallbackParam.localMemSize;

			OPENCL_V_LOCKED(sLock, clSetKernelArg(kern, uarg++, localmemSize, NULL), _T( "clSetKernelArg failed" ));
		}
	}

	array_size_t gWorkSize;
	// Initialize dynamic array storage explicitly.
	array_init(&gWorkSize);
	array_size_t lWorkSize;
	// Initialize dynamic array storage explicitly.
	array_init(&lWorkSize);
	clfftStatus result = FFTActionGetWorkSizes(action, &gWorkSize, &lWorkSize);

	// TODO:  if getWorkSizes returns CLFFT_INVALID_GLOBAL_WORK_SIZE, that means
	// that this multidimensional input data array is too large to be
	// transformed with a single call to clEnqueueNDRangeKernel.  For now, we
	// will just return the error code back up the call stack. The *correct*
	// course of action would be to split the work into mutliple calls to
	// clEnqueueNDRangeKernel.
	if (CLFFT_INVALID_GLOBAL_WORK_SIZE == result)
	{
		OPENCL_V_LOCKED(sLock, result, "Work size too large for clEnqueNDRangeKernel()");
	}
	else
	{
		OPENCL_V_LOCKED(sLock, result, "FFTActionGetWorkSizes failed");
	}
	BUG_CHECK(array_size(&gWorkSize) == array_size(&lWorkSize));

	cl_int call_status =
		clEnqueueNDRangeKernel(*commQueues, kern, ((cl_uint) (array_size(&gWorkSize))), NULL, &gWorkSize.buf[0], &lWorkSize.buf[0], numWaitEvents, waitEvents, outEvents);
	OPENCL_V_LOCKED(sLock, call_status, _T( "clEnqueueNDRangeKernel failed" ));
	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

//	Read the kernels that this plan uses from file, and store into the plan
static clfftStatus FFTActionWriteKernel(clfftPlanHandle plHandle, clfftGenerators gen, const FFTKernelSignatureHeader *data, cl_context context, cl_device_id device)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();

	buffer_t kernelPath = getKernelName(gen, plHandle, true);

	// Open the generated kernel file with C stdio.
	FILE *kernelFile = fopen(bufcstr(&kernelPath), "wb");
	if (!kernelFile)
	{
		// Report kernel dump creation failures with C stdio.
		fprintf(stderr, "Failed to open kernel file for writing: %s\n", bufcstr(&kernelPath));
		return CLFFT_FILE_CREATE_FAILURE;
	}

	buffer_t kernel = buffer_empty();
	OPENCL_V(FFTRepoGetProgramCode(fftRepo, gen, data, &kernel, device, context), _T( "FFTRepoGetProgramCode failed." ));

	// Write the generated kernel source with a trailing newline.
	if (fwrite(bufcstr(&kernel), 1, kernel.len, kernelFile) != kernel.len || fwrite("\n", 1, 1, kernelFile) != 1)
	{
		fclose(kernelFile);
		return CLFFT_FILE_CREATE_FAILURE;
	}

	if (fclose(kernelFile) != 0)
		return CLFFT_FILE_CREATE_FAILURE;

	return CLFFT_SUCCESS;
}

// **************** TODO TODO TODO ***********************
// Making compileKernels function take in command queue parameter so we can
// build for 1 particular device only; this may not be desirable for persistent
// plans, where we may have to compile for all devices in the context; make
// changes appropriately before enabling persistent plans and then remove this
// comment

//	Compile the kernels that this plan uses, and store into the plan
static clfftStatus FFTActionCompileKernels(FFTAction *action, const cl_command_queue commQueueFFT, const clfftPlanHandle plHandle, FFTPlan *fftPlan)
{
	cl_int status = 0;
	size_t deviceListSize = 0;

	FFTRepo *fftRepo = FFTRepoGetInstance();

	// create a cl program executable for the device associated with command
	// queue Get the device
	cl_device_id q_device = fftPlan->bakeDevice;

	cl_program program;
	if (FFTRepoGetclProgram(fftRepo, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), &program, q_device, fftPlan->context) == CLFFT_INVALID_PROGRAM)
	{
		// Build generated kernels directly for the target device.

		//	If the user wishes us to write the kernels out to disk, we do so
		if (fftRepo->setupData.debugFlags & CLFFT_DUMP_PROGRAMS)
		{
			OPENCL_V(FFTActionWriteKernel(plHandle, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), fftPlan->context, fftPlan->bakeDevice),
				_T( "writeKernel failed." ));
		}

		buffer_t programCode = buffer_empty();
		OPENCL_V(FFTRepoGetProgramCode(fftRepo, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), &programCode, q_device, fftPlan->context),
			_T( "FFTRepoGetProgramCode failed." ));

		const char *source = bufcstr(&programCode);
		program = clCreateProgramWithSource(fftPlan->context, 1, &source, NULL, &status);
		OPENCL_V(status, _T( "clCreateProgramWithSource failed." ));

		// create a cl program executable for the device associated with command
		// queue

#if defined(DEBUGGING)
		status = clBuildProgram(program, 1, &q_device, "-g -cl-opt-disable", NULL, NULL); // good for debugging kernels

  // if you have trouble creating smbols that GDB can pick up to set a breakpoint
  // after kernels are loaded into memory this can be used to stop execution to
  // allow you to set a breakpoint in a kernel after kernel symbols are in memory.
  #ifdef DEBUG_BREAK_GDB
		__debugbreak();
  #endif
#else
		status = clBuildProgram(program, 1, &q_device, "", NULL, NULL);
#endif
		if (status != CL_SUCCESS)
		{
			if (status == CL_BUILD_PROGRAM_FAILURE)
			{
				size_t buildLogSize = 0;
				OPENCL_V(clGetProgramBuildInfo(program, q_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &buildLogSize), _T( "clGetProgramBuildInfo failed" ));

				array_char buildLog;
				// Initialize dynamic array storage explicitly.
				array_init(&buildLog);
				// Allocate build log scratch storage explicitly.
				array_resize(&buildLog, buildLogSize);
				memset(&buildLog.buf[0], 0x0, buildLogSize);

				OPENCL_V(clGetProgramBuildInfo(program, q_device, CL_PROGRAM_BUILD_LOG, buildLogSize, &buildLog.buf[0], NULL),
					_T( "clGetProgramBuildInfo failed" ));

				// Print the OpenCL build log with C stdio.
				fprintf(stderr, "\n\t\t\tBUILD LOG\n");
				fprintf(stderr, "************************************************\n");
				fprintf(stderr, "%s\n", &buildLog.buf[0]);
				fprintf(stderr, "************************************************\n");
			}

			OPENCL_V(status, _T( "clBuildProgram failed" ));
		}

		FFTRepoSetclProgram(fftRepo, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), program, q_device, fftPlan->context);

		// For real transforms we compile either forward or backward kernel
		bool buildFwdKernel = FFTActionBuildForwardKernel(action);
		bool buildBwdKernel = FFTActionBuildBackwardKernel(action);

		// get a kernel object handle for a kernel with the given name
		cl_kernel kernel;
		if (buildFwdKernel)
		{
			lockRAII *kernelLock;
			if (FFTRepoGetclKernel(fftRepo, program, CLFFT_FORWARD, &kernel, &kernelLock) == CLFFT_INVALID_KERNEL)
			{
				buffer_t entryPoint = buffer_empty();
				OPENCL_V(FFTRepoGetProgramEntryPoint(fftRepo, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), CLFFT_FORWARD, &entryPoint,
						 q_device, fftPlan->context),
					_T( "FFTRepoGetProgramEntryPoint failed." ));

				kernel = clCreateKernel(program, bufcstr(&entryPoint), &status);
				OPENCL_V(status, _T( "clCreateKernel failed" ));

				FFTRepoSetclKernel(fftRepo, program, CLFFT_FORWARD, kernel);
			}
		}

		if (buildBwdKernel)
		{
			lockRAII *kernelLock;
			if (FFTRepoGetclKernel(fftRepo, program, CLFFT_BACKWARD, &kernel, &kernelLock) == CLFFT_INVALID_KERNEL)
			{
				buffer_t entryPoint = buffer_empty();
				OPENCL_V(FFTRepoGetProgramEntryPoint(fftRepo, FFTActionGetGenerator(action), FFTActionGetSignatureData(action), CLFFT_BACKWARD, &entryPoint,
						 q_device, fftPlan->context),
					_T( "FFTRepoGetProgramEntryPoint failed." ));

				kernel = clCreateKernel(program, bufcstr(&entryPoint), &status);
				OPENCL_V(status, _T( "clCreateKernel failed" ));

				FFTRepoSetclKernel(fftRepo, program, CLFFT_BACKWARD, kernel);
			}
		}
	}

	return CLFFT_SUCCESS;
}

/* End copied source: src\library\enqueue.cpp */

/* Begin copied source: src\library\plan.cpp */

////////////////////////////////////////////

// clfft.plan.cpp : Defines the entry point for the console application.
//

static const char beginning_of_binary[] = "<[_beginning_of_binary_]>";
static const char end_of_binary[] = "<[_end_of_binary_]>";
static const char end_of_file[] = "<[_end_of_file_]>";

static bool pow235(size_t num, size_t *pow2, size_t *pow3, size_t *pow5)
{
	// a helper function to decide if a number is only radix 2, 3 and 5
	if (num % 2 != 0 && num % 3 != 0 && num % 5 != 0)
		return false;

	while (num > 1)
	{
		if (num % 5 == 0)
		{
			num /= 5;
			(*pow5)++;
			continue;
		}
		if (num % 3 == 0)
		{
			num /= 3;
			(*pow3)++;
			continue;
		}
		if (num % 2 == 0)
		{
			num /= 2;
			(*pow2)++;
			continue;
		}
		return false;
	}
	return true;
}

static bool split1D_for_inplace(size_t num, array_size_t_array *splitNums, clfftPrecision precision, size_t threshold)
{
	/* a helper function to split big 1D to friendly 2D sizes for inplace
	   transpose kernels currently only radix 2, 3 and 5 are supported the
	   algorithm looks for ways to split up the 1D into 2D such that one of the
	   dimensions is multiples of the other dimension. And this mupliple is
	   radix2, 3 or 5. each splited dimentsion should be further splited until
	   that it is smaller than 4096
	*/
	if (num <= threshold)
		return true;
	if (num % 2 != 0 && num % 3 != 0 && num % 5 != 0)
		return false;

	// let's figure out pow2, pow3 and pow5 such that num = 2^pow2 * 3^pow3 *
	// 5^pow5
	size_t pow2, pow3, pow5;
	pow2 = pow3 = pow5 = 0;
	bool status = pow235(num, &pow2, &pow3, &pow5);
	if (!status)
		return status;

	size_t divide_factor;
	if (pow2 % 2 != 0)
	{
		// pow2 is odd
		if (pow3 % 2 != 0)
		{
			// pow2 and pow3 are odd
			if (pow5 % 2 != 0)
			{
				// pow2, pow3 and pow5 are odd
				// one dimension is 2*3*5 = 30 times bigger than the other
				// dimension
				divide_factor = 2 * 3 * 5;
			}
			else
			{
				// pow2 and pow3 are odd, pow 5 is even
				// one dimension is 2*3 = 6 times bigger than the other
				// dimension
				divide_factor = 2 * 3;
			}
		}
		else
		{
			// pow2 is odd, pow3 is even
			if (pow5 % 2 != 0)
			{
				// pow2, pow5 are odd pow3 is eve
				divide_factor = 2 * 5;
			}
			else
			{
				// pow2 is odd, pow3 and pow5 are even
				divide_factor = 2;
			}
		}
	}
	else
	{
		// pow2 is even
		if (pow3 % 2 != 0)
		{
			// pow3 is odd pow2 is even
			if (pow5 % 2 != 0)
			{
				// pow2 is even, pow3 and pow5 are odd
				divide_factor = 3 * 5;
			}
			else
			{
				// pow2 and pow5 are even, pow3 is odd
				divide_factor = 3;
			}
		}
		else
		{
			// pow2 and are even
			if (pow5 % 2 != 0)
			{
				// pow5 is odd pow2 pow3 is eve
				divide_factor = 5;
			}
			else
			{
				// all even
				divide_factor = 1;
			}
		}
	}
	// add some special cases
	if (num == 2687385600)
		divide_factor = 2 * 2 * 3 * 3;
	if (num == 2916000000)
		divide_factor = 2 * 2 * 3 * 3 * 5 * 5;
	if (num == 3057647616)
		divide_factor = 2 * 2 * 3 * 3;

	num = num / divide_factor;
	// now the remaining num should have even number of pow2, pow3 and pow5 and
	// we can do sqrt
	size_t temp = (size_t) sqrt((double) num);
	array_size_t splitVec;
	// Initialize dynamic array storage explicitly.
	array_init(&splitVec);
	array_push_back(&splitVec, temp * divide_factor);
	array_push_back(&splitVec, temp);
	array_push_back_size_t_array(splitNums, &splitVec);
	// Release the temporary split vector after copying it into the output list.
	array_free(&splitVec);

	status = status && split1D_for_inplace(temp * divide_factor, splitNums, precision, threshold);
	status = status && split1D_for_inplace(temp, splitNums, precision, threshold);
	return status;
}

// Returns CLFFT_SUCCESS if the fp64 is present, CLFFT_DEVICE_NO_DOUBLE if it is
// not found.
static clfftStatus checkDevExt(const char *ext, cl_device_id device)
{
	size_t deviceExtSize = 0;
	OPENCL_V(clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, NULL, &deviceExtSize),
		"Getting CL_DEVICE_EXTENSIONS Platform Info string size ( "
		"clGetDeviceInfo() )");

	array_char szDeviceExt;
	// Initialize dynamic array storage explicitly.
	array_init(&szDeviceExt);
	// Allocate device extension scratch storage explicitly.
	array_resize(&szDeviceExt, deviceExtSize);
	OPENCL_V(clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, deviceExtSize, &szDeviceExt.buf[0], NULL),
		"Getting CL_DEVICE_EXTENSIONS Platform Info string ( "
		"clGetDeviceInfo() )");

	buffer_t strDeviceExt = buffer_from_cstr(&szDeviceExt.buf[0]);

	if (buffindc(&strDeviceExt, ext, 0) == BUFFER_NPOS)
		return CLFFT_DEVICE_NO_DOUBLE;

	return CLFFT_SUCCESS;
}

static clfftStatus clfftCreateDefaultPlanInternal(clfftPlanHandle *plHandle, cl_context context, const clfftDim dim, const size_t *clLengths)
{
	if (clLengths == NULL)
		return CLFFT_INVALID_HOST_PTR;

	size_t lenX = 1, lenY = 1, lenZ = 1;

	switch (dim)
	{
		case CLFFT_1D:
		{
			//	Minimum length size is 1
			if (clLengths[DimX] == 0)
				return CLFFT_INVALID_ARG_VALUE;

			if (!IsASupportedLength(clLengths[DimX]))
				return CLFFT_NOTIMPLEMENTED;

			lenX = clLengths[DimX];
		}
		break;
		case CLFFT_2D:
		{
			//	Minimum length size is 1
			if (clLengths[DimX] == 0 || clLengths[DimY] == 0)
				return CLFFT_INVALID_ARG_VALUE;

			if (!IsASupportedLength(clLengths[DimX]) || !IsASupportedLength(clLengths[DimY]))
			{
				return CLFFT_NOTIMPLEMENTED;
			}

			lenX = clLengths[DimX];
			lenY = clLengths[DimY];
		}
		break;
		case CLFFT_3D:
		{
			//	Minimum length size is 1
			if (clLengths[DimX] == 0 || clLengths[DimY] == 0 || clLengths[DimZ] == 0)
				return CLFFT_INVALID_ARG_VALUE;

			if (!IsASupportedLength(clLengths[DimX]) || !IsASupportedLength(clLengths[DimY]) || !IsASupportedLength(clLengths[DimZ]))
			{
				return CLFFT_NOTIMPLEMENTED;
			}

			lenX = clLengths[DimX];
			lenY = clLengths[DimY];
			lenZ = clLengths[DimZ];
		}
		break;
		default: return CLFFT_NOTIMPLEMENTED; break;
	}

	FFTPlan *fftPlan = NULL;
	FFTRepo *fftRepo = FFTRepoGetInstance();
	OPENCL_V(FFTRepoCreatePlan(fftRepo, plHandle, &fftPlan), _T( "FFTRepoCreatePlan failed" ));

	fftPlan->baked = false;
	fftPlan->dim = dim;
	fftPlan->placeness = CLFFT_INPLACE;
	fftPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
	fftPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
	fftPlan->precision = CLFFT_SINGLE;
	fftPlan->context = context;
	fftPlan->forwardScale = 1.0;
	fftPlan->backwardScale = 1.0 / ((double) (lenX * lenY * lenZ));
	fftPlan->batchsize = 1;
	fftPlan->gen = Stockham; // default setting

	OPENCL_V(FFTPlanSetEnvelope(fftPlan), "SetEnvelope failed");

	clRetainContext(fftPlan->context);

#if 0
	/////////////////////////////////////////////////////////////////
	// Detect OpenCL devices
	/////////////////////////////////////////////////////////////////
	// First, get the size of device list data
	size_t deviceListSize;
	OPENCL_V( clGetContextInfo( context, CL_CONTEXT_DEVICES, 0, NULL, &deviceListSize ),
		"Getting device array size ( clGetContextInfo() )" );

	//	Allocate memory for the devices
	array_resize(&fftPlan->devices,  deviceListSize / sizeof( cl_device_id ) );

	/* Now, get the device list data */
	OPENCL_V( clGetContextInfo( context, CL_CONTEXT_DEVICES, deviceListSize, &fftPlan->devices.buf[ 0 ], NULL ),
		"Getting device array ( clGetContextInfo() )" );
#endif

	//	Need to devise a way to generate better names
	buffer_stream_t tstream;
	// Initialize stream formatting state explicitly.
	bufstream_init(&tstream);
	bufprintf(&tstream.text, "plan_%llu", (unsigned long long) (*plHandle));

	lockRAII *planLock = NULL;
	OPENCL_V(FFTRepoGetPlan(fftRepo, *plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	lockRAIISetName(planLock, &tstream.text);

	//	Set the lengths and default strides/pitches depending on the dim that
	// the user passes to us
	switch (dim)
	{
		case CLFFT_1D:
		{
			array_push_back(&fftPlan->length, lenX);
			array_push_back(&fftPlan->inStride, 1);
			array_push_back(&fftPlan->outStride, 1);
			fftPlan->iDist = lenX;
			fftPlan->oDist = lenX;
		}
		break;
		case CLFFT_2D:
		{
			array_push_back(&fftPlan->length, lenX);
			array_push_back(&fftPlan->length, lenY);
			array_push_back(&fftPlan->inStride, 1);
			array_push_back(&fftPlan->inStride, lenX);
			array_push_back(&fftPlan->outStride, 1);
			array_push_back(&fftPlan->outStride, lenX);
			fftPlan->iDist = lenX * lenY;
			fftPlan->oDist = lenX * lenY;
		}
		break;
		case CLFFT_3D:
		{
			array_push_back(&fftPlan->length, lenX);
			array_push_back(&fftPlan->length, lenY);
			array_push_back(&fftPlan->length, lenZ);
			array_push_back(&fftPlan->inStride, 1);
			array_push_back(&fftPlan->inStride, lenX);
			array_push_back(&fftPlan->inStride, lenX * lenY);
			array_push_back(&fftPlan->outStride, 1);
			array_push_back(&fftPlan->outStride, lenX);
			array_push_back(&fftPlan->outStride, lenX * lenY);
			fftPlan->iDist = lenX * lenY * lenZ;
			fftPlan->oDist = lenX * lenY * lenZ;
		}
		break;
	}

	fftPlan->plHandle = *plHandle;

	return CLFFT_SUCCESS;
}

// This external entry-point should not be called from within the library. Use
// clfftCreateDefaultPlanInternal instead.
clfftStatus clfftCreateDefaultPlan(clfftPlanHandle *plHandle, cl_context context, const clfftDim dim, const size_t *clLengths)
{
	clfftStatus ret = clfftCreateDefaultPlanInternal(plHandle, context, dim, clLengths);

	if (ret == CLFFT_SUCCESS)
	{
		FFTRepo *fftRepo = FFTRepoGetInstance();
		FFTPlan *fftPlan = NULL;
		lockRAII *planLock = NULL;
		OPENCL_V(FFTRepoGetPlan(fftRepo, *plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));

		fftPlan->userPlan = true;
	}

	return ret;
}

static buffer_t getKernelName(const clfftGenerators gen, const clfftPlanHandle plHandle, bool withPlHandle)
{
	//	Logic to define a sensible filename
	const buffer_t kernelPrefix = buffer_from_cstr("clfft.kernel.");
	buffer_t generatorName = buffer_empty();
	buffer_stream_t kernelPath;
	// Initialize stream formatting state explicitly.
	bufstream_init(&kernelPath);
	switch (gen)
	{
		case Stockham: bufsetcstr(&generatorName, "Stockham"); break;
		case Transpose_GCN: bufsetcstr(&generatorName, "Transpose"); break;
		case Transpose_SQUARE: bufsetcstr(&generatorName, "Transpose"); break;
		case Transpose_NONSQUARE: bufsetcstr(&generatorName, "TransposeNonSquare"); break;
		case Copy: bufsetcstr(&generatorName, "Copy"); break;
	}

	bufprintf(&kernelPath.text, "%s%s", bufcstr(&kernelPrefix), bufcstr(&generatorName));

	if (withPlHandle)
		bufstream_cat_size(&kernelPath, plHandle);

	bufstream_cat_cstr(&kernelPath, ".cl");

	return kernelPath.text;
}

static clfftStatus selectAction(FFTPlan *fftPlan, FFTAction **action, cl_command_queue *commQueueFFT)
{
	// set the action we are baking a leaf
	clfftStatus err;

	switch (fftPlan->gen)
	{
		case Stockham:
		{
			// Instantiate the default stockham generator
			// Keep the generated object owned by its embedded action base.
			FFTGeneratedStockhamAction *generatedAction = FFTGeneratedStockhamActionCreate(fftPlan->plHandle, fftPlan, *commQueueFFT, &err);
			*action = &generatedAction->base;
			OPENCL_V(err, "FFTGeneratedStockhamActionCreate failed");
		}
		break;

		case Transpose_GCN:
		{
			// Keep the generated object owned by its embedded action base.
			FFTGeneratedTransposeGCNAction *generatedAction = FFTGeneratedTransposeGCNActionCreate(fftPlan->plHandle, fftPlan, *commQueueFFT, &err);
			*action = &generatedAction->base;
			OPENCL_V(err, "FFTGeneratedTransposeGCNActionCreate failed");
		}
		break;

		case Copy:
		{
			// Keep the generated object owned by its embedded action base.
			FFTGeneratedCopyAction *generatedAction = FFTGeneratedCopyActionCreate(fftPlan->plHandle, fftPlan, *commQueueFFT, &err);
			*action = &generatedAction->base;
			OPENCL_V(err, "FFTGeneratedCopyActionCreate failed");
		}
		break;

		default:
		{
			assert(false);
			OPENCL_V(CLFFT_NOTIMPLEMENTED, "selectAction failed");
		}
	}

	return CLFFT_SUCCESS;
}

static inline size_t PrecisionWidth(clfftPrecision pr)
{
	switch (pr)
	{
		case CLFFT_SINGLE: return 1;
		case CLFFT_DOUBLE: return 2;
		default: assert(false); return 1;
	}
}

clfftStatus clfftBakePlan(clfftPlanHandle plHandle, cl_uint numQueues, cl_command_queue *commQueueFFT, void(CL_CALLBACK *pfn_notify)(clfftPlanHandle plHandle, void *user_data),
	void *user_data)
{
	//	We do not currently support multi-GPU transforms
	if (numQueues > 1)
		return CLFFT_NOTIMPLEMENTED;

	//	Notification mechanism is not set up yet; BakePlan can be called
	// recursively to decompose higher dimension FFT's into 	arrays of 1d
	// transforms, and this must be implemented to make only a single callback
	// to the user.
	if (pfn_notify != NULL)
		return CLFFT_NOTIMPLEMENTED;

	if (user_data != NULL)
		return CLFFT_NOTIMPLEMENTED;

	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	// if we have already baked the plan and nothing has changed since, we're
	// done here
	if (fftPlan->baked == true)
		return clfftReturnLocked(sLock, CLFFT_SUCCESS);

	// Store the device for which we are baking
	clGetCommandQueueInfo(*commQueueFFT, CL_QUEUE_DEVICE, sizeof(cl_device_id), &fftPlan->bakeDevice, NULL);

	// find product of lengths
	size_t maxLengthInAnyDim = 1;
	switch (fftPlan->dim)
	{
		case CLFFT_3D: maxLengthInAnyDim = maxLengthInAnyDim > fftPlan->length.buf[DimZ] ? maxLengthInAnyDim : fftPlan->length.buf[DimZ];
		case CLFFT_2D: maxLengthInAnyDim = maxLengthInAnyDim > fftPlan->length.buf[DimY] ? maxLengthInAnyDim : fftPlan->length.buf[DimY];
		case CLFFT_1D: maxLengthInAnyDim = maxLengthInAnyDim > fftPlan->length.buf[DimX] ? maxLengthInAnyDim : fftPlan->length.buf[DimX];
	}

	const bool rc = (fftPlan->inputLayout == CLFFT_REAL) || (fftPlan->outputLayout == CLFFT_REAL);

	// upper bounds on transfrom lengths - address this in the next release
	size_t SP_MAX_LEN = 1 << 24;
	size_t DP_MAX_LEN = 1 << 22;
	if ((fftPlan->precision == CLFFT_SINGLE) && (maxLengthInAnyDim > SP_MAX_LEN) && rc)
		return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);
	if ((fftPlan->precision == CLFFT_DOUBLE) && (maxLengthInAnyDim > DP_MAX_LEN) && rc)
		return clfftReturnLocked(sLock, CLFFT_NOTIMPLEMENTED);

	// release buffers, as these will be created only in EnqueueTransform
	if (NULL != fftPlan->intBuffer)
	{
		OPENCL_V_LOCKED(sLock, clReleaseMemObject(fftPlan->intBuffer), _T( "Failed to release internal temporary buffer" ));
		fftPlan->intBuffer = NULL;
	}
	if (NULL != fftPlan->intBufferRC)
	{
		OPENCL_V_LOCKED(sLock, clReleaseMemObject(fftPlan->intBufferRC), _T( "Failed to release internal temporary buffer" ));
		fftPlan->intBufferRC = NULL;
	}
	if (NULL != fftPlan->intBufferC2R)
	{
		OPENCL_V_LOCKED(sLock, clReleaseMemObject(fftPlan->intBufferC2R), _T( "Failed to release internal temporary buffer" ));
		fftPlan->intBufferC2R = NULL;
	}

	if (fftPlan->userPlan) // confirm it is top-level plan (user plan)
	{
		if (fftPlan->placeness == CLFFT_INPLACE)
		{
			if ((fftPlan->inputLayout == CLFFT_HERMITIAN_PLANAR) || (fftPlan->outputLayout == CLFFT_HERMITIAN_PLANAR))
				return clfftReturnLocked(sLock, CLFFT_INVALID_PLAN);
		}

		// Make sure strides & distance are same for C-C transforms
		if (fftPlan->placeness == CLFFT_INPLACE)
		{
			if ((fftPlan->inputLayout != CLFFT_REAL) && (fftPlan->outputLayout != CLFFT_REAL))
			{
				// check strides
				for (size_t i = 0; i < fftPlan->dim; i++)
					if (fftPlan->inStride.buf[i] != fftPlan->outStride.buf[i])
						return clfftReturnLocked(sLock, CLFFT_INVALID_PLAN);

				// check distance
				if (fftPlan->iDist != fftPlan->oDist)
					return clfftReturnLocked(sLock, CLFFT_INVALID_PLAN);
			}
		}
	}

	if (fftPlan->gen == Copy)
	{
		clfftStatus err;
		// Keep the generated object owned by its embedded action base.
		FFTGeneratedCopyAction *generatedAction = FFTGeneratedCopyActionCreate(plHandle, fftPlan, *commQueueFFT, &err);
		fftPlan->action = &generatedAction->base;
		OPENCL_V_LOCKED(sLock, err, _T( "FFTGeneratedCopyActionCreate failed" ));
		fftPlan->baked = true;
		return clfftReturnLocked(sLock, CLFFT_SUCCESS);
	}

	if (fftPlan->userPlan)
	{
		//	If the user specifies double precision, check that the device
		// supports double precision first
		if (fftPlan->precision == CLFFT_DOUBLE || fftPlan->precision == CLFFT_DOUBLE_FAST)
		{
			clfftStatus retAmdFp64 = checkDevExt("cl_amd_fp64", fftPlan->bakeDevice);
			if (retAmdFp64 != CLFFT_SUCCESS)
			{
				//	If AMD's extention is not supported, check for Khronos
				// extention
				clfftStatus retKhrFp64 = checkDevExt("cl_khr_fp64", fftPlan->bakeDevice);
				if (retKhrFp64 != CLFFT_SUCCESS)
					return clfftReturnLocked(sLock, retKhrFp64);
			}
		}
	}

	// Compress the plan by discarding length '1' dimensions
	// decision to pick generator
	if (fftPlan->userPlan && !rc) // confirm it is top-level plan (user plan)
	{
		size_t dmnsn = fftPlan->dim;
		bool pow2flag = true;

		// switch case flows with no 'break' statements
		switch (fftPlan->dim)
		{
			case CLFFT_3D:

				if (fftPlan->length.buf[DimZ] == 1)
				{
					dmnsn -= 1;
					array_erase(&fftPlan->inStride, fftPlan->inStride.buf + 2);
					array_erase(&fftPlan->outStride, fftPlan->outStride.buf + 2);
					array_erase(&fftPlan->length, fftPlan->length.buf + 2);
				}
				else
				{
					if (!IsPo2(fftPlan->length.buf[DimZ]))
						pow2flag = false;
				}
			case CLFFT_2D:

				if (fftPlan->length.buf[DimY] == 1)
				{
					dmnsn -= 1;
					array_erase(&fftPlan->inStride, fftPlan->inStride.buf + 1);
					array_erase(&fftPlan->outStride, fftPlan->outStride.buf + 1);
					array_erase(&fftPlan->length, fftPlan->length.buf + 1);
				}
				else
				{
					if (!IsPo2(fftPlan->length.buf[DimY]))
						pow2flag = false;
				}

			case CLFFT_1D:

				if ((fftPlan->length.buf[DimX] == 1) && (dmnsn > 1))
				{
					dmnsn -= 1;
					array_erase(&fftPlan->inStride, fftPlan->inStride.buf);
					array_erase(&fftPlan->outStride, fftPlan->outStride.buf);
					array_erase(&fftPlan->length, fftPlan->length.buf);
				}
				else
				{
					if (!IsPo2(fftPlan->length.buf[DimX]))
						pow2flag = false;
				}
		}

		fftPlan->dim = (clfftDim) dmnsn;
	}

	// first time check transposed
	if (fftPlan->transposed != CLFFT_NOTRANSPOSE && fftPlan->dim != CLFFT_2D && fftPlan->dim == array_size(&fftPlan->length))
		return clfftReturnLocked(sLock, CLFFT_TRANSPOSED_NOTIMPLEMENTED);

	//	The largest vector we can transform in a single pass
	//	depends on the GPU caps -- especially the amount of LDS
	//	available
	//
	size_t Large1DThreshold = 0;

	OPENCL_V_LOCKED(sLock, FFTPlanGetMax1DLength(fftPlan, &Large1DThreshold), "GetMax1DLength failed");
	BUG_CHECK(Large1DThreshold > 1);

	//	Verify that the data passed to us is packed
	switch (fftPlan->dim)
	{
		case CLFFT_1D:
		{
			if (!Is1DPossible(fftPlan->length.buf[0], Large1DThreshold))
			{
				size_t clLengths[] = { 1, 1, 0 };
				size_t in_1d, in_x, count;

				BUG_CHECK(IsPo2(Large1DThreshold))

				if (IsPo2(fftPlan->length.buf[0]))
				{
					// Enable block compute under these conditions
					if ((fftPlan->inStride.buf[0] == 1) && (fftPlan->outStride.buf[0] == 1) && !rc &&
						(fftPlan->length.buf[0] <= 262144 / PrecisionWidth(fftPlan->precision)) && (array_size(&fftPlan->length) <= 1) &&
						(!clfftGetRequestLibNoMemAlloc() || (fftPlan->placeness == CLFFT_OUTOFPLACE)))
					{
						fftPlan->blockCompute = true;

						if (1 == PrecisionWidth(fftPlan->precision))
						{
							switch (fftPlan->length.buf[0])
							{
								case 8192: clLengths[1] = 64; break;
								case 16384: clLengths[1] = 64; break;
								case 32768: clLengths[1] = 128; break;
								case 65536: clLengths[1] = 256; break;
								case 131072: clLengths[1] = 64; break;
								case 262144: clLengths[1] = 64; break;
								case 524288: clLengths[1] = 256; break;
								case 1048576: clLengths[1] = 256; break;
								default: assert(false);
							}
						}
						else
						{
							switch (fftPlan->length.buf[0])
							{
								case 4096: clLengths[1] = 64; break;
								case 8192: clLengths[1] = 64; break;
								case 16384: clLengths[1] = 64; break;
								case 32768: clLengths[1] = 128; break;
								case 65536: clLengths[1] = 64; break;
								case 131072: clLengths[1] = 64; break;
								case 262144: clLengths[1] = 128; break;
								case 524288: clLengths[1] = 256; break;
								default: assert(false);
							}
						}
					}
					else if (clfftGetRequestLibNoMemAlloc() && !rc && (fftPlan->placeness == CLFFT_INPLACE))
					{
						in_x = BitScanF(fftPlan->length.buf[0]);
						in_x /= 2;
						clLengths[1] = (size_t) 1 << in_x;
					}
					else if (fftPlan->length.buf[0] > (Large1DThreshold * Large1DThreshold))
					{
						clLengths[1] = fftPlan->length.buf[0] / Large1DThreshold;
					}
					else
					{
						in_1d = BitScanF(Large1DThreshold);	 // this is log2(LARGE1D_THRESHOLD)
						in_x = BitScanF(fftPlan->length.buf[0]); // this is log2(length)
						BUG_CHECK(in_1d > 0)
						count = in_x / in_1d;
						if (count * in_1d < in_x)
						{
							count++;
							in_1d = in_x / count;
							if (in_1d * count < in_x)
								in_1d++;
						}
						clLengths[1] = (size_t) 1 << in_1d;
					}
				}
				else
				{
					// This array must be kept sorted in the ascending order

					size_t supported[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 21, 22, 24, 25, 26, 27, 28, 30, 32, 33, 35, 36, 39,
						40, 42, 44, 45, 48, 49, 50, 52, 54, 55, 56, 60, 63, 64, 65, 66, 70, 72, 75, 77, 78, 80, 81, 84, 88, 90, 91, 96, 98, 99, 100, 104,
						105, 108, 110, 112, 117, 120, 121, 125, 126, 128, 130, 132, 135, 140, 143, 144, 147, 150, 154, 156, 160, 162, 165, 168, 169, 175,
						176, 180, 182, 189, 192, 195, 196, 198, 200, 208, 210, 216, 220, 224, 225, 231, 234, 240, 242, 243, 245, 250, 252, 256, 260, 264,
						270, 273, 275, 280, 286, 288, 294, 297, 300, 308, 312, 315, 320, 324, 325, 330, 336, 338, 343, 350, 351, 352, 360, 363, 364, 375,
						378, 384, 385, 390, 392, 396, 400, 405, 416, 420, 429, 432, 440, 441, 448, 450, 455, 462, 468, 480, 484, 486, 490, 495, 500, 504,
						507, 512, 520, 525, 528, 539, 540, 546, 550, 560, 567, 572, 576, 585, 588, 594, 600, 605, 616, 624, 625, 630, 637, 640, 648, 650,
						660, 672, 675, 676, 686, 693, 700, 702, 704, 715, 720, 726, 728, 729, 735, 750, 756, 768, 770, 780, 784, 792, 800, 810, 819, 825,
						832, 840, 845, 847, 858, 864, 875, 880, 882, 891, 896, 900, 910, 924, 936, 945, 960, 968, 972, 975, 980, 990, 1000, 1001, 1008,
						1014, 1024, 1029, 1040, 1050, 1053, 1056, 1078, 1080, 1089, 1092, 1100, 1120, 1125, 1134, 1144, 1152, 1155, 1170, 1176, 1183, 1188,
						1200, 1210, 1215, 1225, 1232, 1248, 1250, 1260, 1274, 1280, 1287, 1296, 1300, 1320, 1323, 1331, 1344, 1350, 1352, 1365, 1372, 1375,
						1386, 1400, 1404, 1408, 1430, 1440, 1452, 1456, 1458, 1470, 1485, 1500, 1512, 1521, 1536, 1540, 1560, 1568, 1573, 1575, 1584, 1600,
						1617, 1620, 1625, 1638, 1650, 1664, 1680, 1690, 1694, 1701, 1715, 1716, 1728, 1750, 1755, 1760, 1764, 1782, 1792, 1800, 1815, 1820,
						1848, 1859, 1872, 1875, 1890, 1911, 1920, 1925, 1936, 1944, 1950, 1960, 1980, 2000, 2002, 2016, 2025, 2028, 2048, 2058, 2079, 2080,
						2100, 2106, 2112, 2145, 2156, 2160, 2178, 2184, 2187, 2197, 2200, 2205, 2240, 2250, 2268, 2275, 2288, 2304, 2310, 2340, 2352, 2366,
						2376, 2400, 2401, 2420, 2430, 2450, 2457, 2464, 2475, 2496, 2500, 2520, 2535, 2541, 2548, 2560, 2574, 2592, 2600, 2625, 2640, 2646,
						2662, 2673, 2688, 2695, 2700, 2704, 2730, 2744, 2750, 2772, 2800, 2808, 2816, 2835, 2860, 2880, 2904, 2912, 2916, 2925, 2940, 2970,
						3000, 3003, 3024, 3025, 3042, 3072, 3080, 3087, 3120, 3125, 3136, 3146, 3150, 3159, 3168, 3185, 3200, 3234, 3240, 3250, 3267, 3276,
						3300, 3328, 3360, 3375, 3380, 3388, 3402, 3430, 3432, 3456, 3465, 3500, 3510, 3520, 3528, 3549, 3564, 3575, 3584, 3600, 3630, 3640,
						3645, 3675, 3696, 3718, 3744, 3750, 3773, 3780, 3822, 3840, 3850, 3861, 3872, 3888, 3900, 3920, 3960, 3969, 3993, 4000, 4004, 4032,
						4050, 4056, 4095, 4096 };

					size_t lenSupported = sizeof(supported) / sizeof(supported[0]);
					size_t maxFactoredLength = (supported[lenSupported - 1] < Large1DThreshold) ? supported[lenSupported - 1] : Large1DThreshold;

					size_t halfPowerLength = (size_t) 1 << ((CeilPo2(fftPlan->length.buf[0]) + 1) / 2);
					size_t factoredLengthStart = (halfPowerLength < maxFactoredLength) ? halfPowerLength : maxFactoredLength;

					size_t indexStart = 0;
					while (supported[indexStart] < factoredLengthStart)
						indexStart++;

					for (size_t i = indexStart; i >= 1; i--)
					{
						if (fftPlan->length.buf[0] % supported[i] == 0)
						{
							if (Is1DPossible(supported[i], Large1DThreshold))
							{
								clLengths[1] = supported[i];
								break;
							}
						}
					}
				}
				// add some special cases
				/*
			if (fftPlan->length.buf[0] == 10000)
				clLengths[1] = 100;//100 x 100
			if (fftPlan->length.buf[0] == 100000)
				clLengths[1] = 100;//100 x 1,000
			if (fftPlan->length.buf[0] == 10000000)
				clLengths[1] = 1000;//1,000 x 10,000
			if (fftPlan->length.buf[0] == 100000000)
				clLengths[1] = 10000;//10,000 x 10,000
			if (fftPlan->length.buf[0] == 1000000000)
				clLengths[1] = 10000;//10,000 x 100,000

			if (fftPlan->length.buf[0] == 3099363912)
				clLengths[1] = 78732;//39366 x 78732
			if (fftPlan->length.buf[0] == 39366)
				clLengths[1] = 81;//81*486
			if (fftPlan->length.buf[0] == 78732)
				clLengths[1] = 162;//162*486
			if (fftPlan->length.buf[0] == 354294)
				clLengths[1] = 243;
			*/
				size_t threshold = 4096;
				if (fftPlan->precision == CLFFT_DOUBLE)
					threshold = 2048;
				if (clfftGetRequestLibNoMemAlloc() && fftPlan->placeness == CLFFT_INPLACE && (fftPlan->inputLayout == fftPlan->outputLayout) &&
					fftPlan->length.buf[0] > threshold)
				{
					// for inplace fft with inplace transpose, the split logic is
					// different
					array_size_t_array splitNums;
					// Initialize dynamic array storage explicitly.
					array_init(&splitNums);
					bool implemented = split1D_for_inplace(fftPlan->length.buf[0], &splitNums, fftPlan->precision, threshold);
					if (implemented)
						clLengths[1] = splitNums.buf[0].buf[0];
					// Release the split table after selecting the requested 2D
					// shape.
					array_free_size_t_array(&splitNums);
				}

				clLengths[0] = fftPlan->length.buf[0] / clLengths[1];

				// Start of block where transposes are generated; 1D FFT
				while (1 && (fftPlan->inputLayout != CLFFT_REAL) && (fftPlan->outputLayout != CLFFT_REAL))
				{
					if (fftPlan->length.buf[0] <= Large1DThreshold)
						break;

					if (fftPlan->inStride.buf[0] != 1 || fftPlan->outStride.buf[0] != 1)
						break;

					if (IsPo2(fftPlan->length.buf[0]) && (fftPlan->length.buf[0] <= 262144 / PrecisionWidth(fftPlan->precision)) &&
						(array_size(&fftPlan->length) <= 1) && (!clfftGetRequestLibNoMemAlloc() || (fftPlan->placeness == CLFFT_OUTOFPLACE)))
						break;

					if (clLengths[0] <= 32 && clLengths[1] <= 32)
						break;

					size_t biggerDim = clLengths[0] > clLengths[1] ? clLengths[0] : clLengths[1];
					size_t smallerDim = biggerDim == clLengths[0] ? clLengths[1] : clLengths[0];
					size_t padding = 0;
					if ((smallerDim % 64 == 0) || (biggerDim % 64 == 0))
						padding = 64;

					clfftGenerators transGen = Transpose_GCN;

					size_t dim_ratio = biggerDim / smallerDim;
					size_t dim_residue = biggerDim % smallerDim;
					//    If this is an in-place transform the
					//    input and output layout, dimensions and strides
					//    *MUST* be the same.
					//
					bool inStrideEqualsOutStride = true;
					for (size_t u = array_size(&fftPlan->inStride); u-- > 0;)
					{
						if (fftPlan->inStride.buf[u] != fftPlan->outStride.buf[u])
						{
							inStrideEqualsOutStride = false;
							break;
						}
					}
					// packed data is required for inplace transpose
					bool isDataPacked = true;
					for (size_t u = 0; u < array_size(&fftPlan->inStride); u++)
					{
						if (u == 0)
						{
							if (fftPlan->inStride.buf[0] == 1)
								continue;
							else
							{
								isDataPacked = false;
								break;
							}
						}
						else
						{
							size_t packDataSize = 1;
							for (size_t i = 0; i < u; i++)
								packDataSize *= fftPlan->length.buf[i];
							if (fftPlan->inStride.buf[u] == packDataSize)
								continue;
							else
							{
								isDataPacked = false;
								break;
							}
						}
					}
					if (clfftGetRequestLibNoMemAlloc() && dim_residue == 0 &&
						((dim_ratio % 2 == 0) || (dim_ratio % 3 == 0) || (dim_ratio % 5 == 0) || (dim_ratio % 10 == 0)) &&
						fftPlan->placeness == CLFFT_INPLACE && (fftPlan->inputLayout == fftPlan->outputLayout) && (inStrideEqualsOutStride) &&
						(isDataPacked))
					{
						padding = 0;
						fftPlan->allOpsInplace = true;
						transGen = Transpose_NONSQUARE;
					}

					if (clfftGetRequestLibNoMemAlloc() && (clLengths[0] == clLengths[1]) && fftPlan->placeness == CLFFT_INPLACE)
					{
						padding = 0;
						fftPlan->allOpsInplace = true;
						transGen = Transpose_SQUARE;
					}

					if (fftPlan->tmpBufSize != 0)
						padding = 0;

					if ((fftPlan->tmpBufSize == 0) && !fftPlan->allOpsInplace)
					{
						fftPlan->tmpBufSize = (smallerDim + padding) * biggerDim * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSize *= fftPlan->length.buf[index];
					}

					// Transpose
					// Input --> tmp buffer
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, clLengths),
						_T( "CreateDefaultPlan Large1d transpose 1 failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->placeness = fftPlan->allOpsInplace ? CLFFT_INPLACE : CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->inputLayout = fftPlan->inputLayout;
					trans1Plan->outputLayout = fftPlan->allOpsInplace ? fftPlan->inputLayout : CLFFT_COMPLEX_INTERLEAVED;
					trans1Plan->inStride.buf[0] = fftPlan->inStride.buf[0];
					trans1Plan->inStride.buf[1] = clLengths[0];
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = clLengths[1] + padding;
					trans1Plan->iDist = fftPlan->iDist;
					trans1Plan->oDist = clLengths[0] * trans1Plan->outStride.buf[1];
					trans1Plan->gen = transGen;
					trans1Plan->transflag = true;

					if (trans1Plan->gen == Transpose_NONSQUARE || trans1Plan->gen == Transpose_SQUARE) // inplace transpose
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							// array_push_back(&trans1Plan->length,
							// fftPlan->length.buf[index]);
							/*
						replacing the line above with the two lines below since:
						fftPlan is still 1D, thus the broken down transpose
						should be 2D not 3D the batchSize for the transpose
						should increase accordingly. the iDist should decrease
						accordingly. Push back to length will cause a 3D
						transpose
						*/
							trans1Plan->batchsize = trans1Plan->batchsize * fftPlan->length.buf[index];
							trans1Plan->iDist = trans1Plan->iDist / fftPlan->length.buf[index];

							array_push_back(&trans1Plan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
							trans1Plan->oDist *= fftPlan->length.buf[index];
						}
					}
					else
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);

							array_push_back(&trans1Plan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
							trans1Plan->oDist *= fftPlan->length.buf[index];
						}
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						trans1Plan->hasPreCallback = true;
						trans1Plan->preCallback = fftPlan->preCallback;
						trans1Plan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans1 plan failed" ));

					// Row transform
					// tmp->output
					// size clLengths[1], batch clLengths[0], with length[0] twiddle
					// factor multiplication
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &clLengths[1]),
						_T( "CreateDefaultPlan Large1d column failed" ));

					FFTPlan *row1Plan = NULL;
					lockRAII *row1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &row1Plan, &row1Lock), _T( "FFTRepoGetPlan failed" ));

					row1Plan->placeness = fftPlan->allOpsInplace ? CLFFT_INPLACE : CLFFT_OUTOFPLACE;
					row1Plan->precision = fftPlan->precision;
					row1Plan->forwardScale = 1.0f;
					row1Plan->backwardScale = 1.0f;
					row1Plan->tmpBufSize = 0;
					row1Plan->batchsize = fftPlan->batchsize;

					row1Plan->gen = fftPlan->gen;
					row1Plan->envelope = fftPlan->envelope;

					// twiddling is done in row2
					row1Plan->large1D = 0;

					array_push_back(&row1Plan->length, clLengths[0]);
					row1Plan->inputLayout = fftPlan->allOpsInplace ? fftPlan->inputLayout : CLFFT_COMPLEX_INTERLEAVED;
					row1Plan->outputLayout = fftPlan->outputLayout;
					row1Plan->inStride.buf[0] = 1;
					row1Plan->outStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&row1Plan->inStride, clLengths[1] + padding);
					array_push_back(&row1Plan->outStride, clLengths[1]);
					row1Plan->iDist = clLengths[0] * row1Plan->inStride.buf[1];
					row1Plan->oDist = fftPlan->oDist;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&row1Plan->length, fftPlan->length.buf[index]);
						array_push_back(&row1Plan->inStride, row1Plan->iDist);
						row1Plan->iDist *= fftPlan->length.buf[index];
						array_push_back(&row1Plan->outStride, fftPlan->outStride.buf[index]);
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d first row plan failed" ));

					// Transpose 2
					// Output --> tmp buffer
					clLengths[2] = clLengths[0];
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, &clLengths[1]),
						_T( "CreateDefaultPlan Large1d transpose 2 failed" ));

					FFTPlan *trans2Plan = NULL;
					lockRAII *trans2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &trans2Plan, &trans2Lock), _T( "FFTRepoGetPlan failed" ));

					trans2Plan->placeness = fftPlan->allOpsInplace ? CLFFT_INPLACE : CLFFT_OUTOFPLACE;
					trans2Plan->precision = fftPlan->precision;
					trans2Plan->tmpBufSize = 0;
					trans2Plan->batchsize = fftPlan->batchsize;
					trans2Plan->envelope = fftPlan->envelope;
					trans2Plan->inputLayout = fftPlan->outputLayout;
					trans2Plan->outputLayout = fftPlan->allOpsInplace ? fftPlan->inputLayout : CLFFT_COMPLEX_INTERLEAVED;
					trans2Plan->inStride.buf[0] = fftPlan->outStride.buf[0];
					trans2Plan->inStride.buf[1] = clLengths[1];
					trans2Plan->outStride.buf[0] = 1;
					trans2Plan->outStride.buf[1] = clLengths[0] + padding;
					trans2Plan->iDist = fftPlan->oDist;
					trans2Plan->oDist = clLengths[1] * trans2Plan->outStride.buf[1];
					trans2Plan->gen = transGen;

					// if (transGen != Transpose_NONSQUARE)//twiddle
					trans2Plan->large1D = fftPlan->length.buf[0];

					trans2Plan->transflag = true;

					if (trans2Plan->gen == Transpose_NONSQUARE || trans2Plan->gen == Transpose_SQUARE) // inplace transpose
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							// array_push_back(&trans2Plan->length,
							// fftPlan->length.buf[index]);
							/*
						replacing the line above with the two lines below since:
						fftPlan is still 1D, thus the broken down transpose
						should be 2D not 3D the batchSize for the transpose
						should increase accordingly. the iDist should decrease
						accordingly. Push back to length will cause a 3D
						transpose
						*/
							trans2Plan->batchsize = trans2Plan->batchsize * fftPlan->length.buf[index];
							trans2Plan->iDist = trans2Plan->iDist / fftPlan->length.buf[index];
							array_push_back(&trans2Plan->inStride, fftPlan->outStride.buf[index]);
							array_push_back(&trans2Plan->outStride, trans2Plan->oDist);
							trans2Plan->oDist *= fftPlan->length.buf[index];
						}
					}
					else
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&trans2Plan->length, fftPlan->length.buf[index]);

							array_push_back(&trans2Plan->inStride, fftPlan->outStride.buf[index]);
							array_push_back(&trans2Plan->outStride, trans2Plan->oDist);
							trans2Plan->oDist *= fftPlan->length.buf[index];
						}
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans2 plan failed" ));

					// Row transform 2
					// tmp->tmp
					// size clLengths[0], batch clLengths[1]
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &clLengths[0]),
						_T( "CreateDefaultPlan Large1d second row plan failed" ));

					FFTPlan *row2Plan = NULL;
					lockRAII *row2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &row2Plan, &row2Lock), _T( "FFTRepoGetPlan failed" ));

					row2Plan->placeness = CLFFT_INPLACE;
					row2Plan->precision = fftPlan->precision;
					row2Plan->forwardScale = fftPlan->forwardScale;
					row2Plan->backwardScale = fftPlan->backwardScale;
					row2Plan->tmpBufSize = 0;
					row2Plan->batchsize = fftPlan->batchsize;

					row2Plan->gen = fftPlan->gen;
					row2Plan->envelope = fftPlan->envelope;

					array_push_back(&row2Plan->length, clLengths[1]);
					row2Plan->inputLayout = fftPlan->allOpsInplace ? fftPlan->inputLayout : CLFFT_COMPLEX_INTERLEAVED;
					row2Plan->outputLayout = fftPlan->allOpsInplace ? fftPlan->inputLayout : CLFFT_COMPLEX_INTERLEAVED;
					row2Plan->inStride.buf[0] = 1;
					row2Plan->outStride.buf[0] = 1;
					array_push_back(&row2Plan->inStride, clLengths[0] + padding);
					array_push_back(&row2Plan->outStride, clLengths[0] + padding);
					row2Plan->iDist = clLengths[1] * row2Plan->inStride.buf[1];
					row2Plan->oDist = clLengths[1] * row2Plan->outStride.buf[1];

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&row2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&row2Plan->inStride, row2Plan->iDist);
						array_push_back(&row2Plan->outStride, row2Plan->oDist);
						row2Plan->iDist *= fftPlan->length.buf[index];
						row2Plan->oDist *= fftPlan->length.buf[index];
					}

					// if (transGen != Transpose_NONSQUARE)//twiddle in transform
					//{
					//	row2Plan->large1D = fftPlan->length.buf[0];
					//	row2Plan->twiddleFront = true;
					// }

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d second row plan failed" ));

					// Transpose 3
					// tmp --> output
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTZ, fftPlan->context, CLFFT_2D, clLengths),
						_T( "CreateDefaultPlan Large1d transpose 3 failed" ));

					FFTPlan *trans3Plan = NULL;
					lockRAII *trans3Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTZ, &trans3Plan, &trans3Lock), _T( "FFTRepoGetPlan failed" ));

					trans3Plan->placeness = fftPlan->allOpsInplace ? CLFFT_INPLACE : CLFFT_OUTOFPLACE;
					trans3Plan->precision = fftPlan->precision;
					trans3Plan->tmpBufSize = 0;
					trans3Plan->batchsize = fftPlan->batchsize;
					trans3Plan->envelope = fftPlan->envelope;
					trans3Plan->inputLayout = fftPlan->allOpsInplace ? fftPlan->inputLayout : CLFFT_COMPLEX_INTERLEAVED;
					trans3Plan->outputLayout = fftPlan->outputLayout;
					trans3Plan->inStride.buf[0] = 1;
					trans3Plan->inStride.buf[1] = clLengths[0] + padding;
					trans3Plan->outStride.buf[0] = fftPlan->outStride.buf[0];
					trans3Plan->outStride.buf[1] = clLengths[1];
					trans3Plan->iDist = clLengths[1] * trans3Plan->inStride.buf[1];
					trans3Plan->oDist = fftPlan->oDist;
					trans3Plan->gen = transGen;
					trans3Plan->transflag = true;
					trans3Plan->transOutHorizontal = true;

					if (trans3Plan->gen == Transpose_NONSQUARE) // inplace transpose
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							// array_push_back(&trans3Plan->length,
							// fftPlan->length.buf[index]);
							/*
						replacing the line above with the two lines below since:
						fftPlan is still 1D, thus the broken down transpose
						should be 2D not 3D the batchSize for the transpose
						should increase accordingly. the iDist should decrease
						accordingly. Push back to length will cause a 3D
						transpose
						*/
							trans3Plan->batchsize = trans3Plan->batchsize * fftPlan->length.buf[index];
							// trans3Plan->iDist = trans3Plan->iDist /
							// fftPlan->length.buf[index];
							// array_push_back(&trans3Plan->inStride,
							// trans3Plan->iDist);
							array_push_back(&trans3Plan->inStride, fftPlan->inStride.buf[index]);
							// trans3Plan->iDist *= fftPlan->length.buf[index];
							array_push_back(&trans3Plan->outStride, fftPlan->outStride.buf[index]);
						}
					}
					else if (trans3Plan->gen == Transpose_SQUARE)
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							trans3Plan->batchsize = trans3Plan->batchsize * fftPlan->length.buf[index];
							// trans3Plan->iDist = trans3Plan->iDist /
							// fftPlan->length.buf[index];
							// array_push_back(&trans3Plan->inStride,
							// trans3Plan->iDist);
							array_push_back(&trans3Plan->inStride, fftPlan->inStride.buf[index]);
							// trans3Plan->iDist *= fftPlan->length.buf[index];
							array_push_back(&trans3Plan->outStride, fftPlan->outStride.buf[index]);
						}
					}
					else
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&trans3Plan->length, fftPlan->length.buf[index]);

							array_push_back(&trans3Plan->inStride, trans3Plan->iDist);
							trans3Plan->iDist *= fftPlan->length.buf[index];
							array_push_back(&trans3Plan->outStride, fftPlan->outStride.buf[index]);
						}
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						trans3Plan->hasPostCallback = true;
						trans3Plan->postCallbackParam = fftPlan->postCallbackParam;
						trans3Plan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans3 plan failed" ));

					fftPlan->transflag = true;
					fftPlan->baked = true;
					return clfftReturnLocked(sLock, CLFFT_SUCCESS);
				}

				size_t length0 = clLengths[0];
				size_t length1 = clLengths[1];

				// For real transforms
				// Special case optimization with 5-step algorithm
				if ((fftPlan->inputLayout == CLFFT_REAL) && IsPo2(fftPlan->length.buf[0]) && (array_size(&fftPlan->length) == 1) &&
					(fftPlan->inStride.buf[0] == 1) && (fftPlan->outStride.buf[0] == 1) && (fftPlan->length.buf[0] > 4096) &&
					(array_size(&fftPlan->length) == 1))
				{
					ARG_CHECK(clLengths[0] <= Large1DThreshold);

					size_t biggerDim = clLengths[0] > clLengths[1] ? clLengths[0] : clLengths[1];
					size_t smallerDim = biggerDim == clLengths[0] ? clLengths[1] : clLengths[0];
					size_t padding = 0;
					if ((smallerDim % 64 == 0) || (biggerDim % 64 == 0))
						padding = 64;

					if (fftPlan->tmpBufSize == 0)
					{
						size_t Nf = (1 + smallerDim / 2) * biggerDim;
						fftPlan->tmpBufSize = (smallerDim + padding) * biggerDim / 2;

						if (fftPlan->tmpBufSize < Nf)
							fftPlan->tmpBufSize = Nf;

						fftPlan->tmpBufSize *= (fftPlan->batchsize * FFTPlanElementSize(fftPlan));

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							fftPlan->tmpBufSize *= fftPlan->length.buf[index];
						}
					}

					if (fftPlan->tmpBufSizeRC == 0)
						fftPlan->tmpBufSizeRC = fftPlan->tmpBufSize;

					// Transpose
					// Input --> tmp buffer
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, clLengths),
						_T( "CreateDefaultPlan Large1d transpose 1 failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->placeness = CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->inputLayout = fftPlan->inputLayout;
					trans1Plan->outputLayout = CLFFT_REAL;
					trans1Plan->inStride.buf[0] = fftPlan->inStride.buf[0];
					trans1Plan->inStride.buf[1] = clLengths[0];
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = clLengths[1] + padding;
					trans1Plan->iDist = fftPlan->iDist;
					trans1Plan->oDist = clLengths[0] * trans1Plan->outStride.buf[1];
					trans1Plan->gen = Transpose_GCN;
					trans1Plan->transflag = true;

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						trans1Plan->hasPreCallback = true;
						trans1Plan->preCallback = fftPlan->preCallback;
						trans1Plan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans1 plan failed" ));

					// Row transform
					// tmp->output
					// size clLengths[1], batch clLengths[0], with length[0] twiddle
					// factor multiplication
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &clLengths[1]),
						_T( "CreateDefaultPlan Large1d column failed" ));

					FFTPlan *row1Plan = NULL;
					lockRAII *row1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &row1Plan, &row1Lock), _T( "FFTRepoGetPlan failed" ));

					row1Plan->placeness = CLFFT_OUTOFPLACE;
					row1Plan->precision = fftPlan->precision;
					row1Plan->forwardScale = 1.0f;
					row1Plan->backwardScale = 1.0f;
					row1Plan->tmpBufSize = 0;
					row1Plan->batchsize = fftPlan->batchsize;

					row1Plan->gen = fftPlan->gen;
					row1Plan->envelope = fftPlan->envelope;

					// twiddling is done in row2
					row1Plan->large1D = 0;

					array_push_back(&row1Plan->length, clLengths[0]);
					row1Plan->inputLayout = CLFFT_REAL;
					row1Plan->outputLayout = CLFFT_HERMITIAN_INTERLEAVED;
					row1Plan->inStride.buf[0] = 1;
					row1Plan->outStride.buf[0] = 1;
					array_push_back(&row1Plan->inStride, clLengths[1] + padding);
					array_push_back(&row1Plan->outStride, 1 + clLengths[1] / 2);
					row1Plan->iDist = clLengths[0] * row1Plan->inStride.buf[1];
					row1Plan->oDist = clLengths[0] * row1Plan->outStride.buf[1];

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d first row plan failed" ));

					// Transpose 2
					// Output --> tmp buffer
					clLengths[2] = clLengths[0];
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, &clLengths[1]),
						_T( "CreateDefaultPlan Large1d transpose 2 failed" ));

					FFTPlan *trans2Plan = NULL;
					lockRAII *trans2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &trans2Plan, &trans2Lock), _T( "FFTRepoGetPlan failed" ));

					trans2Plan->transflag = true;

					size_t transLengths[2];
					transLengths[0] = 1 + clLengths[1] / 2;
					transLengths[1] = clLengths[0];
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTY, CLFFT_2D, transLengths), _T( "clfftSetPlanLength for planTY transpose failed" ));

					trans2Plan->placeness = CLFFT_OUTOFPLACE;
					trans2Plan->precision = fftPlan->precision;
					trans2Plan->tmpBufSize = 0;
					trans2Plan->batchsize = fftPlan->batchsize;
					trans2Plan->envelope = fftPlan->envelope;
					trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					trans2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					trans2Plan->inStride.buf[0] = 1;
					trans2Plan->inStride.buf[1] = 1 + clLengths[1] / 2;
					trans2Plan->outStride.buf[0] = 1;
					trans2Plan->outStride.buf[1] = clLengths[0];
					trans2Plan->iDist = clLengths[0] * trans2Plan->inStride.buf[1];
					trans2Plan->oDist = (1 + clLengths[1] / 2) * trans2Plan->outStride.buf[1];
					trans2Plan->gen = Transpose_GCN;
					trans2Plan->transflag = true;
					trans2Plan->transOutHorizontal = true;

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans2 plan failed" ));

					// Row transform 2
					// tmp->tmp
					// size clLengths[0], batch clLengths[1]
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &clLengths[0]),
						_T( "CreateDefaultPlan Large1d second row plan failed" ));

					FFTPlan *row2Plan = NULL;
					lockRAII *row2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &row2Plan, &row2Lock), _T( "FFTRepoGetPlan failed" ));

					row2Plan->placeness = CLFFT_OUTOFPLACE;
					row2Plan->precision = fftPlan->precision;
					row2Plan->forwardScale = fftPlan->forwardScale;
					row2Plan->backwardScale = fftPlan->backwardScale;
					row2Plan->tmpBufSize = 0;
					row2Plan->batchsize = fftPlan->batchsize;

					row2Plan->gen = fftPlan->gen;
					row2Plan->envelope = fftPlan->envelope;

					array_push_back(&row2Plan->length, 1 + clLengths[1] / 2);
					row2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					row2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					row2Plan->inStride.buf[0] = 1;
					row2Plan->outStride.buf[0] = 1;
					array_push_back(&row2Plan->inStride, clLengths[0]);
					array_push_back(&row2Plan->outStride, 1 + clLengths[0] / 2);
					row2Plan->iDist = (1 + clLengths[1] / 2) * row2Plan->inStride.buf[1];
					row2Plan->oDist = clLengths[1] * row2Plan->outStride.buf[1];

					row2Plan->large1D = fftPlan->length.buf[0];
					row2Plan->twiddleFront = true;

					row2Plan->realSpecial = true;
					row2Plan->realSpecial_Nr = clLengths[1];

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d second row plan failed" ));

					// Transpose 3
					// tmp --> output
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTZ, fftPlan->context, CLFFT_2D, clLengths),
						_T( "CreateDefaultPlan Large1d transpose 3 failed" ));

					FFTPlan *trans3Plan = NULL;
					lockRAII *trans3Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTZ, &trans3Plan, &trans3Lock), _T( "FFTRepoGetPlan failed" ));

					trans3Plan->transflag = true;

					transLengths[0] = 1 + clLengths[0] / 2;
					transLengths[1] = clLengths[1];
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTZ, CLFFT_2D, transLengths), _T( "clfftSetPlanLength for planTZ transpose failed" ));

					trans3Plan->placeness = CLFFT_OUTOFPLACE;
					trans3Plan->precision = fftPlan->precision;
					trans3Plan->tmpBufSize = 0;
					trans3Plan->batchsize = fftPlan->batchsize;
					trans3Plan->envelope = fftPlan->envelope;
					trans3Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					if (fftPlan->outputLayout == CLFFT_HERMITIAN_PLANAR)
						trans3Plan->outputLayout = CLFFT_COMPLEX_PLANAR;
					else
						trans3Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					trans3Plan->inStride.buf[0] = 1;
					trans3Plan->inStride.buf[1] = 1 + clLengths[0] / 2;
					trans3Plan->outStride.buf[0] = 1;
					trans3Plan->outStride.buf[1] = clLengths[1];
					trans3Plan->iDist = clLengths[1] * trans3Plan->inStride.buf[1];
					trans3Plan->oDist = fftPlan->oDist;
					trans3Plan->gen = Transpose_GCN;
					trans3Plan->transflag = true;
					trans3Plan->realSpecial = true;
					trans3Plan->transOutHorizontal = true;

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						trans3Plan->hasPostCallback = true;
						trans3Plan->postCallbackParam = fftPlan->postCallbackParam;
						trans3Plan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans3 plan failed" ));

					fftPlan->transflag = true;
					fftPlan->baked = true;
					return clfftReturnLocked(sLock, CLFFT_SUCCESS);
				}
				else if (fftPlan->inputLayout == CLFFT_REAL)
				{
					if (fftPlan->tmpBufSizeRC == 0)
					{
						fftPlan->tmpBufSizeRC = length0 * length1 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSizeRC *= fftPlan->length.buf[index];
					}

					// column FFT, size clLengths[1], batch clLengths[0], with
					// length[0] twiddle factor multiplication transposed output
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &clLengths[1]),
						"CreateDefaultPlan Large1d column failed");

					FFTPlan *colTPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &colTPlan, &colLock), "FFTRepoGetPlan failed");

					// current plan is to create intermediate buffer, packed and
					// interleave This is a column FFT, the first elements distance
					// between each FFT is the distance of the first two elements in
					// the original buffer. Like a transpose of the matrix we need
					// to pass clLengths[0] and instride size to kernel, so kernel
					// can tell the difference

					// this part are common for both passes
					colTPlan->placeness = CLFFT_OUTOFPLACE;
					colTPlan->precision = fftPlan->precision;
					colTPlan->forwardScale = 1.0f;
					colTPlan->backwardScale = 1.0f;
					colTPlan->tmpBufSize = 0;
					colTPlan->batchsize = fftPlan->batchsize;

					colTPlan->gen = fftPlan->gen;
					colTPlan->envelope = fftPlan->envelope;

					// Pass large1D flag to confirm we need multiply twiddle factor
					colTPlan->large1D = fftPlan->length.buf[0];
					colTPlan->RCsimple = true;

					array_push_back(&colTPlan->length, clLengths[0]);

					// first Pass
					colTPlan->inputLayout = fftPlan->inputLayout;
					colTPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colTPlan->inStride.buf[0] = fftPlan->inStride.buf[0] * clLengths[0];
					colTPlan->outStride.buf[0] = 1;
					colTPlan->iDist = fftPlan->iDist;
					colTPlan->oDist = length0 * length1; // fftPlan->length.buf[0];
					array_push_back(&colTPlan->inStride, fftPlan->inStride.buf[0]);
					array_push_back(&colTPlan->outStride,
						length1); // clLengths[1]);

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colTPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colTPlan->inStride, fftPlan->inStride.buf[index]);
						// tmp buffer is tightly packed
						array_push_back(&colTPlan->outStride, colTPlan->oDist);
						colTPlan->oDist *= fftPlan->length.buf[index];
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						colTPlan->hasPreCallback = true;
						colTPlan->preCallback = fftPlan->preCallback;
						colTPlan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), "BakePlan large1d first column plan failed");

					// another column FFT, size clLengths[0], batch clLengths[1],
					// output without transpose
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &clLengths[0]),
						"CreateDefaultPlan large1D row failed");

					FFTPlan *col2Plan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &col2Plan, &rowLock), "FFTRepoGetPlan failed");

					// This is second column fft, intermediate buffer is packed and
					// interleaved we need to pass clLengths[1] and instride size to
					// kernel, so kernel can tell the difference

					col2Plan->precision = fftPlan->precision;
					col2Plan->forwardScale = fftPlan->forwardScale;
					col2Plan->backwardScale = fftPlan->backwardScale;
					col2Plan->tmpBufSize = 0;
					col2Plan->batchsize = fftPlan->batchsize;

					col2Plan->gen = fftPlan->gen;
					col2Plan->envelope = fftPlan->envelope;

					array_push_back(&col2Plan->length, length1);

					col2Plan->inStride.buf[0] = length1;
					array_push_back(&col2Plan->inStride, 1);
					col2Plan->iDist = length0 * length1;

					// make sure colTPlan (first column plan) does not recurse,
					// otherwise large twiddle mul cannot be done with this
					// algorithm sequence
					assert(colTPlan->planX == 0);

					col2Plan->placeness = CLFFT_INPLACE;
					col2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					col2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;

					col2Plan->outStride.buf[0] = length1;
					array_push_back(&col2Plan->outStride, 1);
					col2Plan->oDist = length0 * length1;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&col2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&col2Plan->inStride, col2Plan->iDist);
						array_push_back(&col2Plan->outStride, col2Plan->oDist);
						col2Plan->iDist *= fftPlan->length.buf[index];
						col2Plan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL),
						_T( "BakePlan large1d second column plan failed" ));

					if ((fftPlan->outputLayout == CLFFT_HERMITIAN_INTERLEAVED) || (fftPlan->outputLayout == CLFFT_HERMITIAN_PLANAR))
					{
						// copy plan to get back to hermitian
						OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planRCcopy, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[0]),
							"CreateDefaultPlan RC copy failed");

						FFTPlan *copyPlan = NULL;
						lockRAII *copyLock = NULL;
						OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planRCcopy, &copyPlan, &copyLock), "FFTRepoGetPlan failed");

						// This is second column fft, intermediate buffer is packed
						// and interleaved we need to pass clLengths[1] and instride
						// size to kernel, so kernel can tell the difference

						// common part for both passes
						copyPlan->placeness = CLFFT_OUTOFPLACE;
						copyPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						copyPlan->outputLayout = fftPlan->outputLayout;

						copyPlan->precision = fftPlan->precision;
						copyPlan->forwardScale = 1.0f;
						copyPlan->backwardScale = 1.0f;
						copyPlan->tmpBufSize = 0;
						copyPlan->batchsize = fftPlan->batchsize;

						copyPlan->gen = Copy;
						copyPlan->envelope = fftPlan->envelope;

						copyPlan->inStride.buf[0] = 1;
						copyPlan->iDist = fftPlan->length.buf[0];

						copyPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
						copyPlan->oDist = fftPlan->oDist;

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&copyPlan->length, fftPlan->length.buf[index]);
							array_push_back(&copyPlan->inStride, copyPlan->inStride.buf[index - 1] * fftPlan->length.buf[index - 1]);
							copyPlan->iDist *= fftPlan->length.buf[index];
							array_push_back(&copyPlan->outStride, fftPlan->outStride.buf[index]);
						}

						// Set callback data if set on top level plan
						if (fftPlan->hasPostCallback)
						{
							copyPlan->hasPostCallback = true;
							copyPlan->postCallbackParam = fftPlan->postCallbackParam;
							copyPlan->postcallUserData = fftPlan->postcallUserData;
						}

						OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planRCcopy, numQueues, commQueueFFT, NULL, NULL),
							"BakePlan large1d RC copy plan failed");
					}
				}
				else if (fftPlan->outputLayout == CLFFT_REAL)
				{
					if (fftPlan->tmpBufSizeRC == 0)
					{
						fftPlan->tmpBufSizeRC = length0 * length1 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSizeRC *= fftPlan->length.buf[index];
					}

					if ((fftPlan->inputLayout == CLFFT_HERMITIAN_INTERLEAVED) || (fftPlan->inputLayout == CLFFT_HERMITIAN_PLANAR))
					{
						// copy plan to from hermitian to full complex
						OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planRCcopy, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[0]),
							"CreateDefaultPlan RC copy failed");

						FFTPlan *copyPlan = NULL;
						lockRAII *copyLock = NULL;
						OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planRCcopy, &copyPlan, &copyLock), "FFTRepoGetPlan failed");

						// This is second column fft, intermediate buffer is packed
						// and interleaved we need to pass clLengths[1] and instride
						// size to kernel, so kernel can tell the difference

						// common part for both passes
						copyPlan->placeness = CLFFT_OUTOFPLACE;
						copyPlan->inputLayout = fftPlan->inputLayout;
						copyPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;

						copyPlan->precision = fftPlan->precision;
						copyPlan->forwardScale = 1.0f;
						copyPlan->backwardScale = 1.0f;
						copyPlan->tmpBufSize = 0;
						copyPlan->batchsize = fftPlan->batchsize;

						copyPlan->gen = Copy;
						copyPlan->envelope = fftPlan->envelope;

						copyPlan->inStride.buf[0] = fftPlan->inStride.buf[0];
						copyPlan->iDist = fftPlan->iDist;

						copyPlan->outStride.buf[0] = 1;
						copyPlan->oDist = fftPlan->length.buf[0];

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&copyPlan->length, fftPlan->length.buf[index]);
							array_push_back(&copyPlan->outStride, copyPlan->outStride.buf[index - 1] * fftPlan->length.buf[index - 1]);
							copyPlan->oDist *= fftPlan->length.buf[index];
							array_push_back(&copyPlan->inStride, fftPlan->inStride.buf[index]);
						}

						// Set callback data if set on top level plan
						if (fftPlan->hasPreCallback)
						{
							copyPlan->hasPreCallback = true;
							copyPlan->preCallback = fftPlan->preCallback;
							copyPlan->precallUserData = fftPlan->precallUserData;
						}

						OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planRCcopy, numQueues, commQueueFFT, NULL, NULL),
							"BakePlan large1d RC copy plan failed");
					}

					// column FFT, size clLengths[1], batch clLengths[0], with
					// length[0] twiddle factor multiplication transposed output
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &clLengths[1]),
						_T( "CreateDefaultPlan Large1d column failed" ));

					FFTPlan *colTPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &colTPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					// current plan is to create intermediate buffer, packed and
					// interleave This is a column FFT, the first elements distance
					// between each FFT is the distance of the first two elements in
					// the original buffer. Like a transpose of the matrix we need
					// to pass clLengths[0] and instride size to kernel, so kernel
					// can tell the difference

					// this part are common for both passes
					colTPlan->precision = fftPlan->precision;
					colTPlan->forwardScale = 1.0f;
					colTPlan->backwardScale = 1.0f;
					colTPlan->tmpBufSize = 0;
					colTPlan->batchsize = fftPlan->batchsize;

					colTPlan->gen = fftPlan->gen;
					colTPlan->envelope = fftPlan->envelope;

					// Pass large1D flag to confirm we need multiply twiddle factor
					colTPlan->large1D = fftPlan->length.buf[0];

					array_push_back(&colTPlan->length, clLengths[0]);

					colTPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colTPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;

					colTPlan->inStride.buf[0] = length0;
					array_push_back(&colTPlan->inStride, 1);
					colTPlan->iDist = length0 * length1;

					colTPlan->outStride.buf[0] = length0;
					array_push_back(&colTPlan->outStride, 1);
					colTPlan->oDist = length0 * length1;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colTPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colTPlan->inStride, colTPlan->iDist);
						array_push_back(&colTPlan->outStride, colTPlan->oDist);
						colTPlan->iDist *= fftPlan->length.buf[index];
						colTPlan->oDist *= fftPlan->length.buf[index];
					}

					if ((fftPlan->inputLayout == CLFFT_HERMITIAN_INTERLEAVED) || (fftPlan->inputLayout == CLFFT_HERMITIAN_PLANAR))
					{
						colTPlan->placeness = CLFFT_INPLACE;
					}
					else
					{
						colTPlan->placeness = CLFFT_OUTOFPLACE;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL),
						_T( "BakePlan large1d first column plan failed" ));

					// another column FFT, size clLengths[0], batch clLengths[1],
					// output without transpose
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &clLengths[0]),
						_T( "CreateDefaultPlan large1D row failed" ));

					FFTPlan *col2Plan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &col2Plan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					// This is second column fft, intermediate buffer is packed and
					// interleaved we need to pass clLengths[1] and instride size to
					// kernel, so kernel can tell the difference

					// common part for both passes
					col2Plan->placeness = CLFFT_OUTOFPLACE;
					col2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					col2Plan->outputLayout = fftPlan->outputLayout;

					col2Plan->precision = fftPlan->precision;
					col2Plan->forwardScale = fftPlan->forwardScale;
					col2Plan->backwardScale = fftPlan->backwardScale;
					col2Plan->tmpBufSize = 0;
					col2Plan->batchsize = fftPlan->batchsize;

					col2Plan->gen = fftPlan->gen;
					col2Plan->envelope = fftPlan->envelope;

					col2Plan->RCsimple = true;
					array_push_back(&col2Plan->length, length1);

					col2Plan->inStride.buf[0] = 1;
					array_push_back(&col2Plan->inStride, length0);
					col2Plan->iDist = length0 * length1;

					col2Plan->outStride.buf[0] = length1 * fftPlan->outStride.buf[0];
					array_push_back(&col2Plan->outStride, fftPlan->outStride.buf[0]);
					col2Plan->oDist = fftPlan->oDist;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&col2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&col2Plan->inStride, col2Plan->iDist);
						col2Plan->iDist *= fftPlan->length.buf[index];
						array_push_back(&col2Plan->outStride, fftPlan->outStride.buf[index]);
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						col2Plan->hasPostCallback = true;
						col2Plan->postCallbackParam = fftPlan->postCallbackParam;
						col2Plan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL),
						_T( "BakePlan large1d second column plan failed" ));
				}
				else

					if ((fftPlan->length.buf[0] > 262144 / PrecisionWidth(fftPlan->precision)) && fftPlan->blockCompute)
				{
					assert(fftPlan->length.buf[0] <= 1048576);

					size_t padding = 64;
					if (fftPlan->tmpBufSize == 0)
					{
						fftPlan->tmpBufSize = (length1 + padding) * length0 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSize *= fftPlan->length.buf[index];
					}

					// Algorithm in this case is
					// T(with pad, out_of_place), R (in_place), C(in_place),
					// Unpad(out_of_place)

					size_t len[3] = { clLengths[1], clLengths[0], 1 };

					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, len),
						_T( "CreateDefaultPlan Large1d trans1 failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->placeness = CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->inputLayout = fftPlan->inputLayout;
					trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					trans1Plan->inStride.buf[0] = fftPlan->inStride.buf[0];
					trans1Plan->inStride.buf[1] = length1;
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = length0 + padding;
					trans1Plan->iDist = fftPlan->iDist;
					trans1Plan->oDist = length1 * trans1Plan->outStride.buf[1];
					trans1Plan->gen = Transpose_GCN;
					trans1Plan->transflag = true;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans1Plan->inStride, fftPlan->inStride.buf[index]);
						array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
						trans1Plan->oDist *= fftPlan->length.buf[index];
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						trans1Plan->hasPreCallback = true;
						trans1Plan->preCallback = fftPlan->preCallback;
						trans1Plan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d trans1 plan failed" ));

					// row FFT
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &clLengths[0]),
						_T( "CreateDefaultPlan Large1d column failed" ));

					FFTPlan *rowPlan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					assert(fftPlan->large1D == 0);

					rowPlan->placeness = CLFFT_INPLACE;
					rowPlan->precision = fftPlan->precision;
					rowPlan->forwardScale = 1.0f;
					rowPlan->backwardScale = 1.0f;
					rowPlan->tmpBufSize = 0;
					rowPlan->batchsize = fftPlan->batchsize;

					rowPlan->gen = fftPlan->gen;
					rowPlan->envelope = fftPlan->envelope;

					array_push_back(&rowPlan->length, length1);

					rowPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					rowPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					rowPlan->inStride.buf[0] = 1;
					rowPlan->outStride.buf[0] = 1;
					array_push_back(&rowPlan->inStride, length0 + padding);
					array_push_back(&rowPlan->outStride, length0 + padding);
					rowPlan->iDist = (length0 + padding) * length1;
					rowPlan->oDist = (length0 + padding) * length1;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&rowPlan->length, fftPlan->length.buf[index]);
						array_push_back(&rowPlan->inStride, rowPlan->iDist);
						rowPlan->iDist *= fftPlan->length.buf[index];
						array_push_back(&rowPlan->outStride, rowPlan->oDist);
						rowPlan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d first row plan failed" ));

					// column FFT
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &clLengths[1]),
						_T( "CreateDefaultPlan large1D column failed" ));

					FFTPlan *col2Plan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &col2Plan, &colLock), _T( "FFTRepoGetPlan failed" ));

					col2Plan->placeness = CLFFT_INPLACE;
					col2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					col2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					col2Plan->precision = fftPlan->precision;
					col2Plan->forwardScale = fftPlan->forwardScale;
					col2Plan->backwardScale = fftPlan->backwardScale;
					col2Plan->tmpBufSize = 0;
					col2Plan->batchsize = fftPlan->batchsize;

					col2Plan->gen = fftPlan->gen;
					col2Plan->envelope = fftPlan->envelope;

					col2Plan->large1D = fftPlan->length.buf[0];
					col2Plan->twiddleFront = true;

					array_push_back(&col2Plan->length, clLengths[0]);

					col2Plan->blockCompute = true;
					col2Plan->blockComputeType = BCT_C2C;

					col2Plan->inStride.buf[0] = length0 + padding;
					col2Plan->outStride.buf[0] = length0 + padding;
					col2Plan->iDist = (length0 + padding) * length1;
					col2Plan->oDist = (length0 + padding) * length1;
					array_push_back(&col2Plan->inStride, 1);
					array_push_back(&col2Plan->outStride, 1);

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&col2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&col2Plan->inStride, col2Plan->iDist);
						array_push_back(&col2Plan->outStride, col2Plan->oDist);
						col2Plan->iDist *= fftPlan->length.buf[index];
						col2Plan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL),
						_T( "BakePlan large1d second column plan failed" ));

					// copy plan to get results back to packed output
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planCopy, fftPlan->context, CLFFT_1D, &clLengths[0]),
						_T( "CreateDefaultPlan Copy failed" ));

					FFTPlan *copyPlan = NULL;
					lockRAII *copyLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planCopy, &copyPlan, &copyLock), _T( "FFTRepoGetPlan failed" ));

					copyPlan->placeness = CLFFT_OUTOFPLACE;
					copyPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					copyPlan->outputLayout = fftPlan->outputLayout;

					copyPlan->precision = fftPlan->precision;
					copyPlan->forwardScale = 1.0f;
					copyPlan->backwardScale = 1.0f;
					copyPlan->tmpBufSize = 0;
					copyPlan->batchsize = fftPlan->batchsize;

					copyPlan->gen = Copy;
					copyPlan->envelope = fftPlan->envelope;

					array_push_back(&copyPlan->length, length1);

					copyPlan->inStride.buf[0] = 1;
					array_push_back(&copyPlan->inStride, length0 + padding);
					copyPlan->iDist = length1 * (length0 + padding);

					copyPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&copyPlan->outStride, length0);
					copyPlan->oDist = fftPlan->oDist;

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&copyPlan->length, fftPlan->length.buf[index]);
						array_push_back(&copyPlan->inStride, copyPlan->inStride.buf[index] * copyPlan->length.buf[index]);
						copyPlan->iDist *= fftPlan->length.buf[index];
						array_push_back(&copyPlan->outStride, fftPlan->outStride.buf[index]);
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planCopy, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan large1d copy plan failed" ));
				}
				else
				{
					if (fftPlan->tmpBufSize == 0)
					{
						fftPlan->tmpBufSize = length0 * length1 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSize *= fftPlan->length.buf[index];
					}

					// column FFT, size clLengths[1], batch clLengths[0], with
					// length[0] twiddle factor multiplication
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &clLengths[1]),
						_T( "CreateDefaultPlan Large1d column failed" ));

					FFTPlan *colTPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &colTPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					assert(fftPlan->large1D == 0);

					// current plan is to create intermediate buffer, packed and
					// interleave This is a column FFT, the first elements distance
					// between each FFT is the distance of the first two elements in
					// the original buffer. Like a transpose of the matrix we need
					// to pass clLengths[0] and instride size to kernel, so kernel
					// can tell the difference

					// this part are common for both passes
					colTPlan->placeness = CLFFT_OUTOFPLACE;
					colTPlan->precision = fftPlan->precision;
					colTPlan->forwardScale = 1.0f;
					colTPlan->backwardScale = 1.0f;
					colTPlan->tmpBufSize = 0;
					colTPlan->batchsize = fftPlan->batchsize;

					colTPlan->gen = fftPlan->gen;
					colTPlan->envelope = fftPlan->envelope;

					// Pass large1D flag to confirm we need multiply twiddle factor
					colTPlan->large1D = fftPlan->length.buf[0];

					array_push_back(&colTPlan->length, length0);

					colTPlan->inputLayout = fftPlan->inputLayout;
					colTPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colTPlan->inStride.buf[0] = fftPlan->inStride.buf[0] * length0;
					colTPlan->outStride.buf[0] = length0;
					colTPlan->iDist = fftPlan->iDist;
					colTPlan->oDist = length0 * length1;
					array_push_back(&colTPlan->inStride, fftPlan->inStride.buf[0]);
					array_push_back(&colTPlan->outStride, 1);

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						colTPlan->hasPreCallback = true;
						colTPlan->preCallback = fftPlan->preCallback;
						colTPlan->precallUserData = fftPlan->precallUserData;
					}

					// Enabling block column compute
					if ((colTPlan->inStride.buf[0] == length0) && IsPo2(fftPlan->length.buf[0]) && (fftPlan->length.buf[0] < 524288))
					{
						colTPlan->blockCompute = true;
						colTPlan->blockComputeType = BCT_C2C;
					}

					for (size_t index = 1; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colTPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colTPlan->inStride, fftPlan->inStride.buf[index]);
						// tmp buffer is tightly packed
						array_push_back(&colTPlan->outStride, colTPlan->oDist);
						colTPlan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL),
						_T( "BakePlan large1d first column plan failed" ));

					// another column FFT, size clLengths[0], batch clLengths[1],
					// output with transpose
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &clLengths[0]),
						_T( "CreateDefaultPlan large1D row failed" ));

					FFTPlan *col2Plan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &col2Plan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					// This is second column fft, intermediate buffer is packed and
					// interleaved we need to pass clLengths[1] and instride size to
					// kernel, so kernel can tell the difference

					// common part for both passes
					col2Plan->outputLayout = fftPlan->outputLayout;
					col2Plan->precision = fftPlan->precision;
					col2Plan->forwardScale = fftPlan->forwardScale;
					col2Plan->backwardScale = fftPlan->backwardScale;
					col2Plan->tmpBufSize = 0;
					col2Plan->batchsize = fftPlan->batchsize;
					col2Plan->oDist = fftPlan->oDist;

					col2Plan->gen = fftPlan->gen;
					col2Plan->envelope = fftPlan->envelope;

					array_push_back(&col2Plan->length, clLengths[1]);

					bool integratedTranposes = true;

					if (colTPlan->blockCompute && (fftPlan->outStride.buf[0] == 1) && clLengths[0] <= 256)
					{
						col2Plan->blockCompute = true;
						col2Plan->blockComputeType = BCT_R2C;

						col2Plan->placeness = CLFFT_OUTOFPLACE;
						col2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						col2Plan->inStride.buf[0] = 1;
						col2Plan->outStride.buf[0] = length1;
						col2Plan->iDist = length0 * length1;
						array_push_back(&col2Plan->inStride, length0);
						array_push_back(&col2Plan->outStride, 1);
					}
					else if (colTPlan->blockCompute && (fftPlan->outStride.buf[0] == 1))
					{
						integratedTranposes = false;

						col2Plan->placeness = CLFFT_INPLACE;
						col2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						col2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
						col2Plan->inStride.buf[0] = 1;
						col2Plan->outStride.buf[0] = 1;
						col2Plan->iDist = length0 * length1;
						col2Plan->oDist = length0 * length1;
						array_push_back(&col2Plan->inStride, length0);
						array_push_back(&col2Plan->outStride, length0);
					}
					else
					{
						// first layer, large 1D from tmp buffer to output buffer
						col2Plan->placeness = CLFFT_OUTOFPLACE;
						col2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						col2Plan->inStride.buf[0] = 1;
						col2Plan->outStride.buf[0] = fftPlan->outStride.buf[0] * clLengths[1];
						col2Plan->iDist = length0 * length1; // fftPlan->length.buf[0];
						array_push_back(&col2Plan->inStride, length0);
						array_push_back(&col2Plan->outStride, fftPlan->outStride.buf[0]);
					}

					if (!integratedTranposes)
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&col2Plan->length, fftPlan->length.buf[index]);
							array_push_back(&col2Plan->inStride, col2Plan->iDist);
							array_push_back(&col2Plan->outStride, col2Plan->oDist);
							col2Plan->iDist *= fftPlan->length.buf[index];
							col2Plan->oDist *= fftPlan->length.buf[index];
						}
					}
					else
					{
						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&col2Plan->length, fftPlan->length.buf[index]);
							array_push_back(&col2Plan->inStride, col2Plan->iDist);
							array_push_back(&col2Plan->outStride, fftPlan->outStride.buf[index]);
							col2Plan->iDist *= fftPlan->length.buf[index];
						}
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback && integratedTranposes)
					{
						col2Plan->hasPostCallback = true;
						col2Plan->postCallbackParam = fftPlan->postCallbackParam;
						col2Plan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL),
						_T( "BakePlan large1d second column plan failed" ));

					if (!integratedTranposes)
					{
						// Transpose
						// tmp --> output
						OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTZ, fftPlan->context, CLFFT_2D, clLengths),
							_T( "CreateDefaultPlan Large1d transpose failed" ));

						FFTPlan *trans3Plan = NULL;
						lockRAII *trans3Lock = NULL;
						OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTZ, &trans3Plan, &trans3Lock), _T( "FFTRepoGetPlan failed" ));

						trans3Plan->placeness = CLFFT_OUTOFPLACE;
						trans3Plan->precision = fftPlan->precision;
						trans3Plan->tmpBufSize = 0;
						trans3Plan->batchsize = fftPlan->batchsize;
						trans3Plan->envelope = fftPlan->envelope;
						trans3Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						trans3Plan->outputLayout = fftPlan->outputLayout;
						trans3Plan->inStride.buf[0] = 1;
						trans3Plan->inStride.buf[1] = clLengths[0];
						trans3Plan->outStride.buf[0] = fftPlan->outStride.buf[0];
						trans3Plan->outStride.buf[1] = clLengths[1] * fftPlan->outStride.buf[0];
						trans3Plan->iDist = fftPlan->length.buf[0];
						trans3Plan->oDist = fftPlan->oDist;
						trans3Plan->gen = Transpose_GCN;
						trans3Plan->transflag = true;

						for (size_t index = 1; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&trans3Plan->length, fftPlan->length.buf[index]);
							array_push_back(&trans3Plan->inStride, trans3Plan->iDist);
							trans3Plan->iDist *= fftPlan->length.buf[index];
							array_push_back(&trans3Plan->outStride, fftPlan->outStride.buf[index]);
						}

						// Set callback data if set on top level plan
						if (fftPlan->hasPostCallback)
						{
							trans3Plan->hasPostCallback = true;
							trans3Plan->postCallbackParam = fftPlan->postCallbackParam;
							trans3Plan->postcallUserData = fftPlan->postcallUserData;
						}

						OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTZ, numQueues, commQueueFFT, NULL, NULL),
							_T( "BakePlan large1d trans plan failed" ));
					}
				}

				fftPlan->baked = true;
				return clfftReturnLocked(sLock, CLFFT_SUCCESS);
			}
		}
		break;
		case CLFFT_2D:
		{
			if (fftPlan->transflag) // Transpose for 2D
			{
				clfftStatus err = CLFFT_SUCCESS;
				if (fftPlan->gen == Transpose_GCN)
				{
					// Keep the generated object owned by its embedded action base.
					FFTGeneratedTransposeGCNAction *generatedAction = FFTGeneratedTransposeGCNActionCreate(plHandle, fftPlan, *commQueueFFT, &err);
					fftPlan->action = &generatedAction->base;
				}
				else if (fftPlan->gen == Transpose_SQUARE)
				{
					// Keep the generated object owned by its embedded action base.
					FFTGeneratedTransposeSquareAction *generatedAction = FFTGeneratedTransposeSquareActionCreate(plHandle, fftPlan, *commQueueFFT, &err);
					fftPlan->action = &generatedAction->base;
				}
				else if (fftPlan->gen == Transpose_NONSQUARE)
				{
					if (fftPlan->nonSquareKernelType != NON_SQUARE_TRANS_PARENT)
					{
						// Keep the generated object owned by its embedded action
						// base.
						FFTGeneratedTransposeNonSquareAction *generatedAction =
							FFTGeneratedTransposeNonSquareActionCreate(plHandle, fftPlan, *commQueueFFT, &err);
						fftPlan->action = &generatedAction->base;
					}
					else
					{
						size_t clLengths[] = { 1, 1, 0 };
						clLengths[0] = fftPlan->length.buf[0];
						clLengths[1] = fftPlan->length.buf[1];

						// NON_SQUARE_KERNEL_ORDER currKernelOrder;
						//  controling the transpose and swap kernel order
						//  if leading dim is larger than the other dim it makes
						//  sense to swap and transpose
						if (clLengths[0] > clLengths[1])
						{
							// Twiddling will be done in swap kernel, in regardless
							// of the order
							fftPlan->nonSquareKernelOrder = SWAP_AND_TRANSPOSE;
						}
						else if (fftPlan->large1D != 0 && 0)
						{
							// this is not going to happen anymore
							fftPlan->nonSquareKernelOrder = TRANSPOSE_LEADING_AND_SWAP;
						}
						else
						{
							// twiddling can be done in swap
							fftPlan->nonSquareKernelOrder = TRANSPOSE_AND_SWAP;
						}

						// ends tranpose kernel order

						// Transpose stage 1
						OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, clLengths),
							"CreateDefaultPlan "
							"transpose_nsq_stage1 plan failed");

						FFTPlan *trans1Plan = NULL;
						lockRAII *trans1Lock = NULL;
						OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans1Plan, &trans1Lock), "FFTRepoGetPlan failed");

						trans1Plan->placeness = CLFFT_INPLACE;
						trans1Plan->precision = fftPlan->precision;
						trans1Plan->tmpBufSize = 0;
						trans1Plan->batchsize = fftPlan->batchsize;
						trans1Plan->envelope = fftPlan->envelope;
						trans1Plan->inputLayout = fftPlan->inputLayout;
						trans1Plan->outputLayout = fftPlan->outputLayout;
						trans1Plan->inStride.buf[0] = fftPlan->inStride.buf[0];
						trans1Plan->outStride.buf[0] = fftPlan->outStride.buf[0];
						trans1Plan->inStride.buf[1] = fftPlan->inStride.buf[1];
						trans1Plan->outStride.buf[1] = fftPlan->outStride.buf[1];
						trans1Plan->iDist = fftPlan->iDist;
						trans1Plan->oDist = fftPlan->oDist;
						trans1Plan->gen = Transpose_NONSQUARE;
						trans1Plan->nonSquareKernelOrder = fftPlan->nonSquareKernelOrder;
						if (fftPlan->nonSquareKernelOrder == SWAP_AND_TRANSPOSE)
							trans1Plan->nonSquareKernelType = NON_SQUARE_TRANS_SWAP;
						else if (fftPlan->nonSquareKernelOrder == TRANSPOSE_AND_SWAP)
							trans1Plan->nonSquareKernelType = NON_SQUARE_TRANS_TRANSPOSE_BATCHED;
						else if (fftPlan->nonSquareKernelOrder == TRANSPOSE_LEADING_AND_SWAP)
							trans1Plan->nonSquareKernelType = NON_SQUARE_TRANS_TRANSPOSE_BATCHED_LEADING;
						trans1Plan->transflag = true;
						trans1Plan->large1D = fftPlan->large1D; // twiddling may happen in this kernel

						if (trans1Plan->nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED)
						{
							// this should be in a function to avoide duplicate code
							// TODO need to treat a non square matrix as a sqaure
							// matrix with bigger batch size
							size_t lengthX = trans1Plan->length.buf[0];
							size_t lengthY = trans1Plan->length.buf[1];

							size_t BatchFactor = (lengthX > lengthY) ? (lengthX / lengthY) : (lengthY / lengthX);
							trans1Plan->transposeMiniBatchSize = BatchFactor;
							trans1Plan->batchsize *= BatchFactor;
							trans1Plan->iDist = trans1Plan->iDist / BatchFactor;
							if (lengthX > lengthY)
							{
								trans1Plan->length.buf[0] = lengthX / BatchFactor;
								trans1Plan->inStride.buf[1] = lengthX / BatchFactor;
							}
							else if (lengthX < lengthY)
							{
								trans1Plan->length.buf[1] = lengthY / BatchFactor;
								trans1Plan->inStride.buf[1] = lengthX;
							}
						}

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);
							array_push_back(&trans1Plan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&trans1Plan->outStride, fftPlan->outStride.buf[index]);
						}

						if (fftPlan->hasPreCallback)
						{
							trans1Plan->hasPreCallback = true;
							trans1Plan->preCallback = fftPlan->preCallback;
							trans1Plan->precallUserData = fftPlan->precallUserData;
						}

						OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL),
							"BakePlan transpose_nsq_stage1 plan failed");

						// Transpose stage 2
						OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, clLengths),
							"CreateDefaultPlan "
							"transpose_nsq_stage2 plan failed");

						FFTPlan *trans2Plan = NULL;
						lockRAII *trans2Lock = NULL;
						OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &trans2Plan, &trans2Lock), "FFTRepoGetPlan failed");

						trans2Plan->placeness = CLFFT_INPLACE;
						trans2Plan->precision = fftPlan->precision;
						trans2Plan->tmpBufSize = 0;
						trans2Plan->batchsize = fftPlan->batchsize;
						trans2Plan->envelope = fftPlan->envelope;
						trans2Plan->inputLayout = fftPlan->inputLayout;
						trans2Plan->outputLayout = fftPlan->outputLayout;
						trans2Plan->inStride.buf[0] = fftPlan->inStride.buf[0];
						trans2Plan->outStride.buf[0] = fftPlan->outStride.buf[0];
						trans2Plan->inStride.buf[1] = fftPlan->inStride.buf[1];
						trans2Plan->outStride.buf[1] = fftPlan->outStride.buf[1];
						trans2Plan->iDist = fftPlan->iDist;
						trans2Plan->oDist = fftPlan->oDist;
						trans2Plan->gen = Transpose_NONSQUARE;
						trans2Plan->nonSquareKernelOrder = fftPlan->nonSquareKernelOrder;
						if (fftPlan->nonSquareKernelOrder == SWAP_AND_TRANSPOSE)
							trans2Plan->nonSquareKernelType = NON_SQUARE_TRANS_TRANSPOSE_BATCHED;
						else if (fftPlan->nonSquareKernelOrder == TRANSPOSE_AND_SWAP)
							trans2Plan->nonSquareKernelType = NON_SQUARE_TRANS_SWAP;
						else if (fftPlan->nonSquareKernelOrder == TRANSPOSE_LEADING_AND_SWAP)
							trans2Plan->nonSquareKernelType = NON_SQUARE_TRANS_SWAP;
						trans2Plan->transflag = true;
						trans2Plan->large1D = fftPlan->large1D; // twiddling may happen in this kernel

						if (trans2Plan->nonSquareKernelType == NON_SQUARE_TRANS_TRANSPOSE_BATCHED)
						{
							// need to treat a non square matrix as a sqaure matrix
							// with bigger batch size
							size_t lengthX = trans2Plan->length.buf[0];
							size_t lengthY = trans2Plan->length.buf[1];

							size_t BatchFactor = (lengthX > lengthY) ? (lengthX / lengthY) : (lengthY / lengthX);
							trans2Plan->transposeMiniBatchSize = BatchFactor;
							trans2Plan->batchsize *= BatchFactor;
							trans2Plan->iDist = trans2Plan->iDist / BatchFactor;
							if (lengthX > lengthY)
							{
								trans2Plan->length.buf[0] = lengthX / BatchFactor;
								trans2Plan->inStride.buf[1] = lengthX / BatchFactor;
							}
							else if (lengthX < lengthY)
							{
								trans2Plan->length.buf[1] = lengthY / BatchFactor;
								trans2Plan->inStride.buf[1] = lengthX;
							}
						}

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&trans2Plan->length, fftPlan->length.buf[index]);
							array_push_back(&trans2Plan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&trans2Plan->outStride, fftPlan->outStride.buf[index]);
						}

						if (fftPlan->hasPostCallback)
						{
							trans2Plan->hasPostCallback = true;
							trans2Plan->postCallbackParam = fftPlan->postCallbackParam;
							trans2Plan->postcallUserData = fftPlan->postcallUserData;
						}

						OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL),
							"BakePlan transpose_nsq_stage2 plan failed");
					}
				}
				else
				{
					// Keep the generated object owned by its embedded action base.
					FFTGeneratedTransposeGCNAction *generatedAction = FFTGeneratedTransposeGCNActionCreate(plHandle, fftPlan, *commQueueFFT, &err);
					fftPlan->action = &generatedAction->base;
				}

				OPENCL_V_LOCKED(sLock, err, "clfftBakePlan transpose action setup failed");

				fftPlan->baked = true;
				return clfftReturnLocked(sLock, CLFFT_SUCCESS);
			}

			size_t length0 = fftPlan->length.buf[0];
			size_t length1 = fftPlan->length.buf[1];

			if (fftPlan->length.buf[0] > Large1DThreshold || fftPlan->length.buf[1] > Large1DThreshold)
				fftPlan->large2D = true;

			while (1 && (fftPlan->inputLayout != CLFFT_REAL) && (fftPlan->outputLayout != CLFFT_REAL))
			{
				// break;

				// TODO : Check for a better way to do this.
				bool isnvidia = false;
				for (size_t Idx = 0; !isnvidia && Idx < numQueues; Idx++)
				{
					cl_command_queue QIdx = commQueueFFT[Idx];
					cl_device_id Device;
					clGetCommandQueueInfo(QIdx, CL_QUEUE_DEVICE, sizeof(Device), &Device, NULL);
					char Vendor[256];
					clGetDeviceInfo(Device, CL_DEVICE_VENDOR, sizeof(Vendor), &Vendor, NULL);
					isnvidia |= (strncmp(Vendor, "NVIDIA", 6) == 0);
				}
				// nvidia gpus are failing when doing transpose for 2D FFTs
				if (isnvidia)
					break;

				if (array_size(&fftPlan->length) != 2)
					break;
				if (!(IsPo2(fftPlan->length.buf[0])) || !(IsPo2(fftPlan->length.buf[1])))
					break;
				if (fftPlan->length.buf[1] < 32)
					break;
				// TBD: restrict the use large2D in x!=y case becase we will need
				// two temp buffers
				//      (1) for 2D usage (2) for 1D large usage
				// if (fftPlan->large2D) break;
				// Performance show 512 is the good case with transpose
				// if user want the result to be transposed, then we will.

				if (fftPlan->length.buf[0] < 64)
					break;
				// x!=y case, we need tmp buffer, currently temp buffer only support
				// interleaved format if (fftPlan->length.buf[0] !=
				// fftPlan->length.buf[1] && fftPlan->outputLayout ==
				// CLFFT_COMPLEX_PLANAR) break;
				if (fftPlan->inStride.buf[0] != 1 || fftPlan->outStride.buf[0] != 1 || fftPlan->inStride.buf[1] != fftPlan->length.buf[0] ||
					fftPlan->outStride.buf[1] != fftPlan->length.buf[0])
					break;
				// if (fftPlan->placeness != CLFFT_INPLACE || fftPlan->inputLayout
				// != CLFFT_COMPLEX_PLANAR) 	break; if (fftPlan->batchsize != 1)
				// break; if (fftPlan->precision != CLFFT_SINGLE) break;

				fftPlan->transflag = true;

				// create row plan,
				//  x=y & x!=y, In->In for inplace, In->out for outofplace
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimX]),
					_T( "CreateDefaultPlan for planX failed" ));

				FFTPlan *rowPlan = NULL;
				lockRAII *rowLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

				rowPlan->inputLayout = fftPlan->inputLayout;
				rowPlan->outputLayout = fftPlan->outputLayout;
				rowPlan->placeness = fftPlan->placeness;
				rowPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
				array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[1]);
				rowPlan->oDist = fftPlan->oDist;
				rowPlan->precision = fftPlan->precision;
				rowPlan->forwardScale = 1.0f;
				rowPlan->backwardScale = 1.0f;
				rowPlan->tmpBufSize = 0;

				rowPlan->gen = fftPlan->gen;
				rowPlan->envelope = fftPlan->envelope;
				rowPlan->batchsize = fftPlan->batchsize;
				rowPlan->inStride.buf[0] = fftPlan->inStride.buf[0];
				array_push_back(&rowPlan->length, fftPlan->length.buf[1]);
				array_push_back(&rowPlan->inStride, fftPlan->inStride.buf[1]);
				rowPlan->iDist = fftPlan->iDist;

				// Set callback data if set on top level plan
				if (fftPlan->hasPreCallback)
				{
					rowPlan->hasPreCallback = true;
					rowPlan->preCallback = fftPlan->preCallback;
					rowPlan->precallUserData = fftPlan->precallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planX failed" ));

				// Create transpose plan for first transpose
				// x=y: inplace. x!=y inplace: in->tmp, outofplace out->tmp
				size_t clLengths[] = { 1, 1, 0 };
				clLengths[0] = fftPlan->length.buf[0];
				clLengths[1] = fftPlan->length.buf[1];

				size_t biggerDim = clLengths[0] > clLengths[1] ? clLengths[0] : clLengths[1];
				size_t smallerDim = biggerDim == clLengths[0] ? clLengths[1] : clLengths[0];
				size_t padding = 0;

				fftPlan->transpose_in_2d_inplace = (clLengths[0] == clLengths[1]) ? true : false;
				if ((!fftPlan->transpose_in_2d_inplace) && fftPlan->tmpBufSize == 0 && array_size(&fftPlan->length) <= 2)
				{
					if ((smallerDim % 64 == 0) || (biggerDim % 64 == 0))
						if (biggerDim > 512)
							padding = 64;

					// we need tmp buffer for x!=y case
					// we assume the tmp buffer is packed interleaved
					fftPlan->tmpBufSize = (smallerDim + padding) * biggerDim * fftPlan->batchsize * FFTPlanElementSize(fftPlan);
				}

				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, clLengths),
					_T( "CreateDefaultPlan for planT failed" ));

				FFTPlan *transPlanX = NULL;
				lockRAII *transLockX = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &transPlanX, &transLockX), _T( "FFTRepoGetPlan failed" ));

				transPlanX->inputLayout = fftPlan->outputLayout;
				transPlanX->precision = fftPlan->precision;
				transPlanX->tmpBufSize = 0;

				transPlanX->envelope = fftPlan->envelope;
				transPlanX->batchsize = fftPlan->batchsize;
				transPlanX->inStride.buf[0] = fftPlan->outStride.buf[0];
				transPlanX->inStride.buf[1] = fftPlan->outStride.buf[1];
				transPlanX->iDist = fftPlan->oDist;
				transPlanX->transflag = true;

				if (!fftPlan->transpose_in_2d_inplace)
				{
					transPlanX->gen = Transpose_GCN;
					transPlanX->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					transPlanX->placeness = CLFFT_OUTOFPLACE;
					transPlanX->outStride.buf[0] = 1;
					transPlanX->outStride.buf[1] = clLengths[1] + padding;
					transPlanX->oDist = clLengths[0] * transPlanX->outStride.buf[1];
				}
				else
				{
					transPlanX->gen = Transpose_SQUARE;
					transPlanX->outputLayout = fftPlan->outputLayout;
					transPlanX->placeness = CLFFT_INPLACE;
					transPlanX->outStride.buf[0] = fftPlan->outStride.buf[0];
					transPlanX->outStride.buf[1] = fftPlan->outStride.buf[1];
					transPlanX->oDist = fftPlan->oDist;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTX failed" ));

				// create second row plan
				// x!=y: tmp->tmp, x=y case: In->In or Out->Out
				// if Transposed result is a choice x!=y: tmp->In or out
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimY]),
					_T( "CreateDefaultPlan for planY failed" ));

				FFTPlan *colPlan = NULL;
				lockRAII *colLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

				if (!fftPlan->transpose_in_2d_inplace)
				{
					colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colPlan->inStride.buf[0] = 1;
					array_push_back(&colPlan->inStride, clLengths[1] + padding);
					colPlan->iDist = clLengths[0] * colPlan->inStride.buf[1];

					if (fftPlan->transposed == CLFFT_NOTRANSPOSE)
					{
						colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
						colPlan->outStride.buf[0] = 1;
						array_push_back(&colPlan->outStride, clLengths[1] + padding);
						colPlan->oDist = clLengths[0] * colPlan->outStride.buf[1];
						colPlan->placeness = CLFFT_INPLACE;
					}
					else
					{
						colPlan->outputLayout = fftPlan->outputLayout;
						colPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
						array_push_back(&colPlan->outStride, clLengths[1] * fftPlan->outStride.buf[0]);
						colPlan->oDist = fftPlan->oDist;
						colPlan->placeness = CLFFT_OUTOFPLACE;
					}
				}
				else
				{
					colPlan->inputLayout = fftPlan->outputLayout;
					colPlan->outputLayout = fftPlan->outputLayout;
					colPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&colPlan->outStride, fftPlan->outStride.buf[1]);
					colPlan->oDist = fftPlan->oDist;
					colPlan->inStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&colPlan->inStride, fftPlan->outStride.buf[1]);
					colPlan->iDist = fftPlan->oDist;
					colPlan->placeness = CLFFT_INPLACE;
				}

				colPlan->precision = fftPlan->precision;
				colPlan->forwardScale = fftPlan->forwardScale;
				colPlan->backwardScale = fftPlan->backwardScale;
				colPlan->tmpBufSize = 0;

				colPlan->gen = fftPlan->gen;
				colPlan->envelope = fftPlan->envelope;
				colPlan->batchsize = fftPlan->batchsize;
				array_push_back(&colPlan->length, fftPlan->length.buf[0]);

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planY failed" ));

				if (fftPlan->transposed == CLFFT_TRANSPOSED)
				{
					fftPlan->baked = true;
					return clfftReturnLocked(sLock, CLFFT_SUCCESS);
				}

				// Create transpose plan for second transpose
				// x!=y case tmp->In or Out, x=y case In->In or Out->out
				size_t clLengthsY[2] = { clLengths[1], clLengths[0] };
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, clLengthsY),
					_T( "CreateDefaultPlan for planTY failed" ));

				FFTPlan *transPlanY = NULL;
				lockRAII *transLockY = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &transPlanY, &transLockY), _T( "FFTRepoGetPlan failed" ));

				if (!fftPlan->transpose_in_2d_inplace)
				{
					transPlanY->gen = Transpose_GCN;
					transPlanY->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					transPlanY->placeness = CLFFT_OUTOFPLACE;
					transPlanY->inStride.buf[0] = 1;
					transPlanY->inStride.buf[1] = clLengths[1] + padding;
					transPlanY->iDist = clLengths[0] * transPlanY->inStride.buf[1];
					transPlanY->transOutHorizontal = true;
				}
				else
				{
					transPlanY->gen = Transpose_SQUARE;
					transPlanY->inputLayout = fftPlan->outputLayout;
					transPlanY->placeness = CLFFT_INPLACE;
					transPlanY->inStride.buf[0] = fftPlan->outStride.buf[0];
					transPlanY->inStride.buf[1] = fftPlan->outStride.buf[1];
					transPlanY->iDist = fftPlan->oDist;
				}
				transPlanY->outputLayout = fftPlan->outputLayout;
				transPlanY->outStride.buf[0] = fftPlan->outStride.buf[0];
				transPlanY->outStride.buf[1] = fftPlan->outStride.buf[1];
				transPlanY->oDist = fftPlan->oDist;
				transPlanY->precision = fftPlan->precision;
				transPlanY->tmpBufSize = 0;

				transPlanY->envelope = fftPlan->envelope;
				transPlanY->batchsize = fftPlan->batchsize;
				transPlanY->transflag = true;

				// Set callback data if set on top level plan
				if (fftPlan->hasPostCallback)
				{
					transPlanY->hasPostCallback = true;
					transPlanY->postCallbackParam = fftPlan->postCallbackParam;
					transPlanY->postcallUserData = fftPlan->postcallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTY failed" ));

				fftPlan->baked = true;
				return clfftReturnLocked(sLock, CLFFT_SUCCESS);
			}

			// check transposed
			if (fftPlan->transposed != CLFFT_NOTRANSPOSE)
				return clfftReturnLocked(sLock, CLFFT_TRANSPOSED_NOTIMPLEMENTED);

			if (fftPlan->inputLayout == CLFFT_REAL)
			{
				length0 = fftPlan->length.buf[0];
				length1 = fftPlan->length.buf[1];

				size_t Nt = (1 + length0 / 2);

				// create row plan
				// real to hermitian

				// create row plan
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimX]),
					_T( "CreateDefaultPlan for planX failed" ));

				FFTPlan *rowPlan = NULL;
				lockRAII *rowLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

				rowPlan->outputLayout = fftPlan->outputLayout;
				rowPlan->inputLayout = fftPlan->inputLayout;
				rowPlan->placeness = fftPlan->placeness;
				array_push_back(&rowPlan->length, length1);

				rowPlan->inStride.buf[0] = fftPlan->inStride.buf[0];
				array_push_back(&rowPlan->inStride, fftPlan->inStride.buf[1]);
				rowPlan->iDist = fftPlan->iDist;

				rowPlan->precision = fftPlan->precision;
				rowPlan->forwardScale = 1.0f;
				rowPlan->backwardScale = 1.0f;
				rowPlan->tmpBufSize = 0;

				rowPlan->gen = fftPlan->gen;
				rowPlan->envelope = fftPlan->envelope;

				rowPlan->batchsize = fftPlan->batchsize;

				rowPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
				array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[1]);
				rowPlan->oDist = fftPlan->oDist;

				// this 2d is decomposed from 3d
				for (size_t index = 2; index < array_size(&fftPlan->length); index++)
				{
					array_push_back(&rowPlan->length, fftPlan->length.buf[index]);
					array_push_back(&rowPlan->inStride, fftPlan->inStride.buf[index]);
					array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[index]);
				}

				// Set callback data if set on top level plan
				if (fftPlan->hasPreCallback)
				{
					rowPlan->hasPreCallback = true;
					rowPlan->preCallback = fftPlan->preCallback;
					rowPlan->precallUserData = fftPlan->precallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planX failed" ));

				if ((rowPlan->inStride.buf[0] == 1) && (rowPlan->outStride.buf[0] == 1) &&
					(((rowPlan->inStride.buf[1] == Nt * 2) && (rowPlan->placeness == CLFFT_INPLACE)) ||
						((rowPlan->inStride.buf[1] == length0) && (rowPlan->placeness == CLFFT_OUTOFPLACE))) &&
					(rowPlan->outStride.buf[1] == Nt))
				{
					// calc temp buf size
					if (fftPlan->tmpBufSize == 0)
					{
						fftPlan->tmpBufSize = Nt * length1 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSize *= fftPlan->length.buf[index];
					}

					// create first transpose plan

					// Transpose
					//  output --> tmp
					size_t transLengths[2] = { length0, length1 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, transLengths),
						_T( "CreateDefaultPlan for planTX transpose failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->transflag = true;

					transLengths[0] = Nt;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTX, CLFFT_2D, transLengths), _T( "clfftSetPlanLength for planTX transpose failed" ));

					switch (fftPlan->outputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					trans1Plan->placeness = CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->forwardScale = 1.0f;
					trans1Plan->backwardScale = 1.0f;

					trans1Plan->inStride.buf[0] = 1;
					trans1Plan->inStride.buf[1] = Nt;
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = length1;
					trans1Plan->iDist = rowPlan->oDist;
					trans1Plan->oDist = Nt * length1;
					trans1Plan->transOutHorizontal = true;

					trans1Plan->gen = Transpose_GCN;

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans1Plan->inStride, rowPlan->outStride.buf[index]);
						array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
						trans1Plan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTX failed" ));

					// Create column plan as a row plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimY]),
						_T( "CreateDefaultPlan for planY failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					colPlan->outputLayout = trans1Plan->outputLayout;
					colPlan->inputLayout = trans1Plan->outputLayout;
					colPlan->placeness = CLFFT_INPLACE;
					array_push_back(&colPlan->length, Nt);

					colPlan->inStride.buf[0] = 1;
					array_push_back(&colPlan->inStride, length1);
					colPlan->iDist = Nt * length1;

					colPlan->outStride.buf[0] = 1;
					array_push_back(&colPlan->outStride, length1);
					colPlan->oDist = Nt * length1;

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = fftPlan->forwardScale;
					colPlan->backwardScale = fftPlan->backwardScale;
					colPlan->tmpBufSize = 0;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					// this 2d is decomposed from 3d
					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colPlan->inStride, colPlan->iDist);
						array_push_back(&colPlan->outStride, colPlan->oDist);
						colPlan->iDist *= fftPlan->length.buf[index];
						colPlan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planY failed" ));

					if (fftPlan->transposed == CLFFT_TRANSPOSED)
					{
						fftPlan->baked = true;
						return clfftReturnLocked(sLock, CLFFT_SUCCESS);
					}

					// create second transpose plan

					// Transpose
					// output --> tmp
					size_t trans2Lengths[2] = { length1, length0 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, trans2Lengths),
						_T( "CreateDefaultPlan for planTY transpose failed" ));

					FFTPlan *trans2Plan = NULL;
					lockRAII *trans2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &trans2Plan, &trans2Lock), _T( "FFTRepoGetPlan failed" ));

					trans2Plan->transflag = true;

					trans2Lengths[1] = Nt;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTY, CLFFT_2D, trans2Lengths),
						_T( "clfftSetPlanLength for planTY transpose failed" ));

					switch (fftPlan->outputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							trans2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							trans2Plan->outputLayout = CLFFT_COMPLEX_PLANAR;
							trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						default: assert(false);
					}

					trans2Plan->placeness = CLFFT_OUTOFPLACE;
					trans2Plan->precision = fftPlan->precision;
					trans2Plan->tmpBufSize = 0;
					trans2Plan->batchsize = fftPlan->batchsize;
					trans2Plan->envelope = fftPlan->envelope;
					trans2Plan->forwardScale = 1.0f;
					trans2Plan->backwardScale = 1.0f;

					trans2Plan->inStride.buf[0] = 1;
					trans2Plan->inStride.buf[1] = length1;
					trans2Plan->outStride.buf[0] = 1;
					trans2Plan->outStride.buf[1] = Nt;
					trans2Plan->iDist = Nt * length1;
					trans2Plan->oDist = fftPlan->oDist;

					trans2Plan->gen = Transpose_GCN;
					trans2Plan->transflag = true;

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans2Plan->inStride, trans2Plan->iDist);
						trans2Plan->iDist *= fftPlan->length.buf[index];
						array_push_back(&trans2Plan->outStride, fftPlan->outStride.buf[index]);
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						trans2Plan->hasPostCallback = true;
						trans2Plan->postCallbackParam = fftPlan->postCallbackParam;
						trans2Plan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTY failed" ));
				}
				else
				{
					// create col plan
					// complex to complex

					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimY]),
						_T( "CreateDefaultPlan for planY failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					switch (fftPlan->outputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_PLANAR;
							colPlan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					colPlan->placeness = CLFFT_INPLACE;
					array_push_back(&colPlan->length, Nt);

					colPlan->outStride.buf[0] = fftPlan->outStride.buf[1];
					array_push_back(&colPlan->outStride, fftPlan->outStride.buf[0]);
					colPlan->oDist = fftPlan->oDist;

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = fftPlan->forwardScale;
					colPlan->backwardScale = fftPlan->backwardScale;
					colPlan->tmpBufSize = fftPlan->tmpBufSize;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					colPlan->inStride.buf[0] = rowPlan->outStride.buf[1];
					array_push_back(&colPlan->inStride, rowPlan->outStride.buf[0]);
					colPlan->iDist = rowPlan->oDist;

					// this 2d is decomposed from 3d
					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colPlan->outStride, fftPlan->outStride.buf[index]);
						array_push_back(&colPlan->inStride, rowPlan->outStride.buf[index]);
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						colPlan->hasPostCallback = true;
						colPlan->postCallbackParam = fftPlan->postCallbackParam;
						colPlan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planY failed" ));
				}
			}
			else if (fftPlan->outputLayout == CLFFT_REAL)
			{
				length0 = fftPlan->length.buf[0];
				length1 = fftPlan->length.buf[1];

				size_t Nt = (1 + length0 / 2);
				if (fftPlan->tmpBufSize == 0)
				{
					fftPlan->tmpBufSize = Nt * length1 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						fftPlan->tmpBufSize *= fftPlan->length.buf[index];
				}

				if ((fftPlan->tmpBufSizeC2R == 0) && (fftPlan->placeness == CLFFT_OUTOFPLACE) && (array_size(&fftPlan->length) == 2))
				{
					fftPlan->tmpBufSizeC2R = fftPlan->tmpBufSize;
				}

				if ((fftPlan->inStride.buf[0] == 1) && (fftPlan->outStride.buf[0] == 1) &&
					(((fftPlan->outStride.buf[1] == Nt * 2) && (fftPlan->oDist == Nt * 2 * length1) && (fftPlan->placeness == CLFFT_INPLACE)) ||
						((fftPlan->outStride.buf[1] == length0) && (fftPlan->oDist == length0 * length1) && (fftPlan->placeness == CLFFT_OUTOFPLACE))) &&
					(fftPlan->inStride.buf[1] == Nt) && (fftPlan->iDist == Nt * length1))
				{
					// create first transpose plan

					// Transpose
					//  input --> tmp
					size_t transLengths[2] = { length0, length1 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, transLengths),
						_T( "CreateDefaultPlan for planTY transpose failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->transflag = true;

					transLengths[0] = Nt;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTY, CLFFT_2D, transLengths), _T( "clfftSetPlanLength for planTY transpose failed" ));

					switch (fftPlan->inputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					trans1Plan->placeness = CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->forwardScale = 1.0f;
					trans1Plan->backwardScale = 1.0f;

					trans1Plan->inStride.buf[0] = 1;
					trans1Plan->inStride.buf[1] = Nt;
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = length1;
					trans1Plan->iDist = fftPlan->iDist;
					trans1Plan->oDist = Nt * length1;
					trans1Plan->transOutHorizontal = true;

					trans1Plan->gen = Transpose_GCN;

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans1Plan->inStride, fftPlan->inStride.buf[index]);
						array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
						trans1Plan->oDist *= fftPlan->length.buf[index];
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						trans1Plan->hasPreCallback = true;
						trans1Plan->preCallback = fftPlan->preCallback;
						trans1Plan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTY failed" ));

					// create col plan
					// complex to complex

					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimY]),
						_T( "CreateDefaultPlan for planY failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					array_push_back(&colPlan->length, Nt);

					colPlan->inStride.buf[0] = 1;
					array_push_back(&colPlan->inStride, length1);
					colPlan->iDist = trans1Plan->oDist;

					colPlan->placeness = CLFFT_INPLACE;
					colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;

					colPlan->outStride.buf[0] = colPlan->inStride.buf[0];
					array_push_back(&colPlan->outStride, colPlan->inStride.buf[1]);
					colPlan->oDist = colPlan->iDist;

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colPlan->inStride, trans1Plan->outStride.buf[index]);
						array_push_back(&colPlan->outStride, trans1Plan->outStride.buf[index]);
					}

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = 1.0f;
					colPlan->backwardScale = 1.0f;
					colPlan->tmpBufSize = 0;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planY failed" ));

					// create second transpose plan

					// Transpose
					// tmp --> output
					size_t trans2Lengths[2] = { length1, length0 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, trans2Lengths),
						_T( "CreateDefaultPlan for planTX transpose failed" ));

					FFTPlan *trans2Plan = NULL;
					lockRAII *trans2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans2Plan, &trans2Lock), _T( "FFTRepoGetPlan failed" ));

					trans2Plan->transflag = true;

					trans2Lengths[1] = Nt;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTX, CLFFT_2D, trans2Lengths),
						_T( "clfftSetPlanLength for planTX transpose failed" ));

					trans2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;

					trans2Plan->placeness = CLFFT_OUTOFPLACE;
					trans2Plan->precision = fftPlan->precision;
					trans2Plan->tmpBufSize = 0;
					trans2Plan->batchsize = fftPlan->batchsize;
					trans2Plan->envelope = fftPlan->envelope;
					trans2Plan->forwardScale = 1.0f;
					trans2Plan->backwardScale = 1.0f;

					trans2Plan->inStride.buf[0] = 1;
					trans2Plan->inStride.buf[1] = length1;
					trans2Plan->outStride.buf[0] = 1;
					trans2Plan->outStride.buf[1] = Nt;
					trans2Plan->iDist = colPlan->oDist;
					trans2Plan->oDist = Nt * length1;

					trans2Plan->gen = Transpose_GCN;
					trans2Plan->transflag = true;

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans2Plan->inStride, colPlan->outStride.buf[index]);
						array_push_back(&trans2Plan->outStride, trans2Plan->oDist);
						trans2Plan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTX failed" ));

					// create row plan
					// hermitian to real

					// create row plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimX]),
						_T( "CreateDefaultPlan for planX failed" ));

					FFTPlan *rowPlan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					rowPlan->outputLayout = fftPlan->outputLayout;
					rowPlan->inputLayout = CLFFT_HERMITIAN_INTERLEAVED;

					array_push_back(&rowPlan->length, length1);

					rowPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[1]);
					rowPlan->oDist = fftPlan->oDist;

					rowPlan->inStride.buf[0] = trans2Plan->outStride.buf[0];
					array_push_back(&rowPlan->inStride, trans2Plan->outStride.buf[1]);
					rowPlan->iDist = trans2Plan->oDist;

					for (size_t index = 2; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&rowPlan->length, fftPlan->length.buf[index]);
						array_push_back(&rowPlan->inStride, trans2Plan->outStride.buf[index]);
						array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[index]);
					}

					if (fftPlan->placeness == CLFFT_INPLACE)
						rowPlan->placeness = CLFFT_INPLACE;
					else
						rowPlan->placeness = CLFFT_OUTOFPLACE;

					rowPlan->precision = fftPlan->precision;
					rowPlan->forwardScale = fftPlan->forwardScale;
					rowPlan->backwardScale = fftPlan->backwardScale;
					rowPlan->tmpBufSize = 0;

					rowPlan->gen = fftPlan->gen;
					rowPlan->envelope = fftPlan->envelope;

					rowPlan->batchsize = fftPlan->batchsize;

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						rowPlan->hasPostCallback = true;
						rowPlan->postCallbackParam = fftPlan->postCallbackParam;
						rowPlan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planX failed" ));
				}
				else
				{
					// create col plan
					// complex to complex

					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimY]),
						_T( "CreateDefaultPlan for planY failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					switch (fftPlan->inputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							colPlan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					array_push_back(&colPlan->length, Nt);

					colPlan->inStride.buf[0] = fftPlan->inStride.buf[1];
					array_push_back(&colPlan->inStride, fftPlan->inStride.buf[0]);
					colPlan->iDist = fftPlan->iDist;

					if (fftPlan->placeness == CLFFT_INPLACE)
						colPlan->placeness = CLFFT_INPLACE;
					else if (array_size(&fftPlan->length) > 2)
						colPlan->placeness = CLFFT_INPLACE;
					else
						colPlan->placeness = CLFFT_OUTOFPLACE;

					if (colPlan->placeness == CLFFT_INPLACE)
					{
						colPlan->outStride.buf[0] = colPlan->inStride.buf[0];
						array_push_back(&colPlan->outStride, colPlan->inStride.buf[1]);
						colPlan->oDist = colPlan->iDist;

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&colPlan->length, fftPlan->length.buf[index]);
							array_push_back(&colPlan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&colPlan->outStride, fftPlan->inStride.buf[index]);
						}
					}
					else
					{
						colPlan->outStride.buf[0] = Nt;
						array_push_back(&colPlan->outStride, 1);
						colPlan->oDist = Nt * length1;

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&colPlan->length, fftPlan->length.buf[index]);
							array_push_back(&colPlan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&colPlan->outStride, colPlan->oDist);
							colPlan->oDist *= fftPlan->length.buf[index];
						}
					}

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = 1.0f;
					colPlan->backwardScale = 1.0f;
					colPlan->tmpBufSize = 0;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						colPlan->hasPreCallback = true;
						colPlan->preCallback = fftPlan->preCallback;
						colPlan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planY failed" ));

					// create row plan
					// hermitian to real

					// create row plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimX]),
						_T( "CreateDefaultPlan for planX failed" ));

					FFTPlan *rowPlan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					rowPlan->outputLayout = fftPlan->outputLayout;
					rowPlan->inputLayout = CLFFT_HERMITIAN_INTERLEAVED;

					array_push_back(&rowPlan->length, length1);

					rowPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[1]);
					rowPlan->oDist = fftPlan->oDist;

					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						rowPlan->placeness = CLFFT_INPLACE;

						rowPlan->inStride.buf[0] = colPlan->outStride.buf[1];
						array_push_back(&rowPlan->inStride, colPlan->outStride.buf[0]);
						rowPlan->iDist = colPlan->oDist;

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&rowPlan->length, fftPlan->length.buf[index]);
							array_push_back(&rowPlan->inStride, colPlan->outStride.buf[index]);
							array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[index]);
						}
					}
					else
					{
						rowPlan->placeness = CLFFT_OUTOFPLACE;

						rowPlan->inStride.buf[0] = 1;
						array_push_back(&rowPlan->inStride, Nt);
						rowPlan->iDist = Nt * length1;

						for (size_t index = 2; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&rowPlan->length, fftPlan->length.buf[index]);
							array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[index]);
							array_push_back(&rowPlan->inStride, rowPlan->iDist);
							rowPlan->iDist *= fftPlan->length.buf[index];
						}
					}

					rowPlan->precision = fftPlan->precision;
					rowPlan->forwardScale = fftPlan->forwardScale;
					rowPlan->backwardScale = fftPlan->backwardScale;
					rowPlan->tmpBufSize = 0;

					rowPlan->gen = fftPlan->gen;
					rowPlan->envelope = fftPlan->envelope;

					rowPlan->batchsize = fftPlan->batchsize;

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						rowPlan->hasPostCallback = true;
						rowPlan->postCallbackParam = fftPlan->postCallbackParam;
						rowPlan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planX failed" ));
				}
			}
			else
			{
				if (fftPlan->tmpBufSize == 0 && array_size(&fftPlan->length) <= 2)
				{
					fftPlan->tmpBufSize = length0 * length1 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);
				}

				// create row plan
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimX]),
					_T( "CreateDefaultPlan for planX failed" ));

				FFTPlan *rowPlan = NULL;
				lockRAII *rowLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

				rowPlan->inputLayout = fftPlan->inputLayout;
				if (fftPlan->large2D || array_size(&fftPlan->length) > 2)
				{
					rowPlan->outputLayout = fftPlan->outputLayout;
					rowPlan->placeness = fftPlan->placeness;
					rowPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[1]);
					rowPlan->oDist = fftPlan->oDist;
				}
				else
				{
					rowPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					rowPlan->placeness = CLFFT_OUTOFPLACE;
					rowPlan->outStride.buf[0] = length1;	 // 1;
					array_push_back(&rowPlan->outStride, 1); // length0);
					rowPlan->oDist = length0 * length1;
				}
				rowPlan->precision = fftPlan->precision;
				rowPlan->forwardScale = 1.0f;
				rowPlan->backwardScale = 1.0f;
				rowPlan->tmpBufSize = fftPlan->tmpBufSize;

				rowPlan->gen = fftPlan->gen;
				rowPlan->envelope = fftPlan->envelope;

				// This is the row fft, the first elements distance between the
				// first two FFTs is the distance of the first elements of the first
				// two rows in the original buffer.
				rowPlan->batchsize = fftPlan->batchsize;
				rowPlan->inStride.buf[0] = fftPlan->inStride.buf[0];

				// pass length and other info to kernel, so the kernel knows this is
				// decomposed from higher dimension
				array_push_back(&rowPlan->length, fftPlan->length.buf[1]);
				array_push_back(&rowPlan->inStride, fftPlan->inStride.buf[1]);

				// this 2d is decomposed from 3d
				if (array_size(&fftPlan->length) > 2)
				{
					array_push_back(&rowPlan->length, fftPlan->length.buf[2]);
					array_push_back(&rowPlan->inStride, fftPlan->inStride.buf[2]);
					array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[2]);
				}

				rowPlan->iDist = fftPlan->iDist;

				// Set callback data if set on top level plan
				if (fftPlan->hasPreCallback)
				{
					rowPlan->hasPreCallback = true;
					rowPlan->preCallback = fftPlan->preCallback;
					rowPlan->precallUserData = fftPlan->precallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planX failed" ));

				// create col plan
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planY, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimY]),
					_T( "CreateDefaultPlan for planY failed" ));

				FFTPlan *colPlan = NULL;
				lockRAII *colLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planY, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

				if (fftPlan->large2D || array_size(&fftPlan->length) > 2)
				{
					colPlan->inputLayout = fftPlan->outputLayout;
					colPlan->placeness = CLFFT_INPLACE;
					colPlan->inStride.buf[0] = fftPlan->outStride.buf[1];
					array_push_back(&colPlan->inStride, fftPlan->outStride.buf[0]);
					colPlan->iDist = fftPlan->oDist;
				}
				else
				{
					colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colPlan->placeness = CLFFT_OUTOFPLACE;
					colPlan->inStride.buf[0] = 1;		      // length0;
					array_push_back(&colPlan->inStride, length1); // 1);
					colPlan->iDist = length0 * length1;
				}

				colPlan->outputLayout = fftPlan->outputLayout;
				colPlan->precision = fftPlan->precision;
				colPlan->forwardScale = fftPlan->forwardScale;
				colPlan->backwardScale = fftPlan->backwardScale;
				colPlan->tmpBufSize = fftPlan->tmpBufSize;

				colPlan->gen = fftPlan->gen;
				colPlan->envelope = fftPlan->envelope;

				// This is a column FFT, the first elements distance between each
				// FFT is the distance of the first two elements in the original
				// buffer. Like a transpose of the matrix
				colPlan->batchsize = fftPlan->batchsize;
				colPlan->outStride.buf[0] = fftPlan->outStride.buf[1];

				// pass length and other info to kernel, so the kernel knows this is
				// decomposed from higher dimension
				array_push_back(&colPlan->length, fftPlan->length.buf[0]);
				array_push_back(&colPlan->outStride, fftPlan->outStride.buf[0]);
				colPlan->oDist = fftPlan->oDist;

				// this 2d is decomposed from 3d
				if (array_size(&fftPlan->length) > 2)
				{
					// assert(fftPlan->large2D);
					array_push_back(&colPlan->length, fftPlan->length.buf[2]);
					array_push_back(&colPlan->inStride, fftPlan->outStride.buf[2]);
					array_push_back(&colPlan->outStride, fftPlan->outStride.buf[2]);
				}

				// Set callback data if set on top level plan
				if (fftPlan->hasPostCallback)
				{
					colPlan->hasPostCallback = true;
					colPlan->postCallbackParam = fftPlan->postCallbackParam;
					colPlan->postcallUserData = fftPlan->postcallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planY failed" ));
			}

			fftPlan->baked = true;
			return clfftReturnLocked(sLock, CLFFT_SUCCESS);
		}
		case CLFFT_3D:
		{
			if (fftPlan->inputLayout == CLFFT_REAL)
			{
				size_t length0 = fftPlan->length.buf[DimX];
				size_t length1 = fftPlan->length.buf[DimY];
				size_t length2 = fftPlan->length.buf[DimZ];

				size_t Nt = (1 + length0 / 2);

				// create 2D xy plan
				size_t clLengths[] = { length0, length1, 0 };
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_2D, clLengths),
					_T( "CreateDefaultPlan 2D planX failed" ));

				FFTPlan *xyPlan = NULL;
				lockRAII *rowLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &xyPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

				xyPlan->inputLayout = fftPlan->inputLayout;
				xyPlan->outputLayout = fftPlan->outputLayout;
				xyPlan->placeness = fftPlan->placeness;
				xyPlan->precision = fftPlan->precision;
				xyPlan->forwardScale = 1.0f;
				xyPlan->backwardScale = 1.0f;
				xyPlan->tmpBufSize = fftPlan->tmpBufSize;

				xyPlan->gen = fftPlan->gen;
				xyPlan->envelope = fftPlan->envelope;

				// This is the xy fft, the first elements distance between the first
				// two FFTs is the distance of the first elements of the first two
				// rows in the original buffer.
				xyPlan->batchsize = fftPlan->batchsize;
				xyPlan->inStride.buf[0] = fftPlan->inStride.buf[0];
				xyPlan->inStride.buf[1] = fftPlan->inStride.buf[1];
				xyPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
				xyPlan->outStride.buf[1] = fftPlan->outStride.buf[1];

				// pass length and other info to kernel, so the kernel knows this is
				// decomposed from higher dimension
				array_push_back(&xyPlan->length, fftPlan->length.buf[2]);
				array_push_back(&xyPlan->inStride, fftPlan->inStride.buf[2]);
				array_push_back(&xyPlan->outStride, fftPlan->outStride.buf[2]);
				xyPlan->iDist = fftPlan->iDist;
				xyPlan->oDist = fftPlan->oDist;

				// this 3d is decomposed from 4d
				for (size_t index = 3; index < array_size(&fftPlan->length); index++)
				{
					array_push_back(&xyPlan->length, fftPlan->length.buf[index]);
					array_push_back(&xyPlan->inStride, fftPlan->inStride.buf[index]);
					array_push_back(&xyPlan->outStride, fftPlan->outStride.buf[index]);
				}

				// Set callback data if set on top level plan
				if (fftPlan->hasPreCallback)
				{
					xyPlan->hasPreCallback = true;
					xyPlan->preCallback = fftPlan->preCallback;
					xyPlan->precallUserData = fftPlan->precallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan 3D->2D planX failed" ));

				if ((xyPlan->inStride.buf[0] == 1) && (xyPlan->outStride.buf[0] == 1) && (xyPlan->outStride.buf[2] == Nt * length1) &&
					(((xyPlan->inStride.buf[2] == Nt * 2 * length1) && (xyPlan->placeness == CLFFT_INPLACE)) ||
						((xyPlan->inStride.buf[2] == length0 * length1) && (xyPlan->placeness == CLFFT_OUTOFPLACE))))
				{
					if (fftPlan->tmpBufSize == 0)
					{
						fftPlan->tmpBufSize = Nt * length1 * length2 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

						for (size_t index = 3; index < array_size(&fftPlan->length); index++)
							fftPlan->tmpBufSize *= fftPlan->length.buf[index];
					}

					// create first transpose plan

					// Transpose
					//  output --> tmp
					size_t transLengths[2] = { length0 * length1, length2 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, transLengths),
						_T( "CreateDefaultPlan for planTX transpose failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->transflag = true;

					transLengths[0] = Nt * length1;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTX, CLFFT_2D, transLengths), _T( "clfftSetPlanLength for planTX transpose failed" ));

					switch (fftPlan->outputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					trans1Plan->placeness = CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->forwardScale = 1.0f;
					trans1Plan->backwardScale = 1.0f;

					trans1Plan->inStride.buf[0] = 1;
					trans1Plan->inStride.buf[1] = Nt * length1;
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = length2;
					trans1Plan->iDist = xyPlan->oDist;
					trans1Plan->oDist = Nt * length1 * length2;
					trans1Plan->transOutHorizontal = true;

					trans1Plan->gen = Transpose_GCN;

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans1Plan->inStride, xyPlan->outStride.buf[index]);
						array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
						trans1Plan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTX failed" ));

					// Create column plan as a row plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planZ, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimZ]),
						_T( "CreateDefaultPlan for planZ failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planZ, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					colPlan->outputLayout = trans1Plan->outputLayout;
					colPlan->inputLayout = trans1Plan->outputLayout;
					colPlan->placeness = CLFFT_INPLACE;
					array_push_back(&colPlan->length, Nt * length1);

					colPlan->inStride.buf[0] = 1;
					array_push_back(&colPlan->inStride, length2);
					colPlan->iDist = Nt * length1 * length2;

					colPlan->outStride.buf[0] = 1;
					array_push_back(&colPlan->outStride, length2);
					colPlan->oDist = Nt * length1 * length2;

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = fftPlan->forwardScale;
					colPlan->backwardScale = fftPlan->backwardScale;
					colPlan->tmpBufSize = 0;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					// this 2d is decomposed from 3d
					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colPlan->inStride, colPlan->iDist);
						array_push_back(&colPlan->outStride, colPlan->oDist);
						colPlan->iDist *= fftPlan->length.buf[index];
						colPlan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planZ failed" ));

					if (fftPlan->transposed == CLFFT_TRANSPOSED)
					{
						fftPlan->baked = true;
						return clfftReturnLocked(sLock, CLFFT_SUCCESS);
					}

					// create second transpose plan

					// Transpose
					// output --> tmp
					size_t trans2Lengths[2] = { length2, length0 * length1 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTY, fftPlan->context, CLFFT_2D, trans2Lengths),
						_T( "CreateDefaultPlan for planTY transpose failed" ));

					FFTPlan *trans2Plan = NULL;
					lockRAII *trans2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTY, &trans2Plan, &trans2Lock), _T( "FFTRepoGetPlan failed" ));

					trans2Plan->transflag = true;

					trans2Lengths[1] = Nt * length1;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTY, CLFFT_2D, trans2Lengths),
						_T( "clfftSetPlanLength for planTY transpose failed" ));

					switch (fftPlan->outputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							trans2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							trans2Plan->outputLayout = CLFFT_COMPLEX_PLANAR;
							trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						default: assert(false);
					}

					trans2Plan->placeness = CLFFT_OUTOFPLACE;
					trans2Plan->precision = fftPlan->precision;
					trans2Plan->tmpBufSize = 0;
					trans2Plan->batchsize = fftPlan->batchsize;
					trans2Plan->envelope = fftPlan->envelope;
					trans2Plan->forwardScale = 1.0f;
					trans2Plan->backwardScale = 1.0f;

					trans2Plan->inStride.buf[0] = 1;
					trans2Plan->inStride.buf[1] = length2;
					trans2Plan->outStride.buf[0] = 1;
					trans2Plan->outStride.buf[1] = Nt * length1;
					trans2Plan->iDist = Nt * length1 * length2;
					trans2Plan->oDist = fftPlan->oDist;

					trans2Plan->gen = Transpose_GCN;
					trans2Plan->transflag = true;

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans2Plan->inStride, trans2Plan->iDist);
						trans2Plan->iDist *= fftPlan->length.buf[index];
						array_push_back(&trans2Plan->outStride, fftPlan->outStride.buf[index]);
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						trans2Plan->hasPostCallback = true;
						trans2Plan->postCallbackParam = fftPlan->postCallbackParam;
						trans2Plan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTY, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTY failed" ));
				}
				else
				{
					clLengths[0] = fftPlan->length.buf[DimZ];
					clLengths[1] = clLengths[2] = 0;
					// create 1D col plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planZ, fftPlan->context, CLFFT_1D, clLengths),
						_T( "CreateDefaultPlan for planZ failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planZ, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					switch (fftPlan->outputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_PLANAR;
							colPlan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					colPlan->placeness = CLFFT_INPLACE;
					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = fftPlan->forwardScale;
					colPlan->backwardScale = fftPlan->backwardScale;
					colPlan->tmpBufSize = fftPlan->tmpBufSize;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					// This is a column FFT, the first elements distance between
					// each FFT is the distance of the first two elements in the
					// original buffer. Like a transpose of the matrix
					colPlan->batchsize = fftPlan->batchsize;
					colPlan->inStride.buf[0] = fftPlan->outStride.buf[2];
					colPlan->outStride.buf[0] = fftPlan->outStride.buf[2];

					// pass length and other info to kernel, so the kernel knows
					// this is decomposed from higher dimension
					array_push_back(&colPlan->length, 1 + fftPlan->length.buf[0] / 2);
					array_push_back(&colPlan->length, fftPlan->length.buf[1]);
					array_push_back(&colPlan->inStride, fftPlan->outStride.buf[0]);
					array_push_back(&colPlan->inStride, fftPlan->outStride.buf[1]);
					array_push_back(&colPlan->outStride, fftPlan->outStride.buf[0]);
					array_push_back(&colPlan->outStride, fftPlan->outStride.buf[1]);
					colPlan->iDist = fftPlan->oDist;
					colPlan->oDist = fftPlan->oDist;

					// this 3d is decomposed from 4d
					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colPlan->inStride, xyPlan->outStride.buf[index]);
						array_push_back(&colPlan->outStride, fftPlan->outStride.buf[index]);
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						colPlan->hasPostCallback = true;
						colPlan->postCallbackParam = fftPlan->postCallbackParam;
						colPlan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan 3D->1D planZ failed" ));
				}
			}
			else if (fftPlan->outputLayout == CLFFT_REAL)
			{
				size_t length0 = fftPlan->length.buf[DimX];
				size_t length1 = fftPlan->length.buf[DimY];
				size_t length2 = fftPlan->length.buf[DimZ];

				size_t Nt = (1 + length0 / 2);

				if (fftPlan->tmpBufSize == 0)
				{
					fftPlan->tmpBufSize = Nt * length1 * length2 * fftPlan->batchsize * FFTPlanElementSize(fftPlan);

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
						fftPlan->tmpBufSize *= fftPlan->length.buf[index];
				}

				if ((fftPlan->tmpBufSizeC2R == 0) && (fftPlan->placeness == CLFFT_OUTOFPLACE))
				{
					fftPlan->tmpBufSizeC2R = fftPlan->tmpBufSize;
				}

				if ((fftPlan->inStride.buf[0] == 1) && (fftPlan->outStride.buf[0] == 1) &&
					(((fftPlan->outStride.buf[2] == Nt * 2 * length1) && (fftPlan->oDist == Nt * 2 * length1 * length2) &&
						 (fftPlan->placeness == CLFFT_INPLACE)) ||
						((fftPlan->outStride.buf[2] == length0 * length1) && (fftPlan->oDist == length0 * length1 * length2) &&
							(fftPlan->placeness == CLFFT_OUTOFPLACE))) &&
					(fftPlan->inStride.buf[2] == Nt * length1) && (fftPlan->iDist == Nt * length1 * length2))
				{
					// create first transpose plan

					// Transpose
					//  input --> tmp
					size_t transLengths[2] = { length0 * length1, length2 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTZ, fftPlan->context, CLFFT_2D, transLengths),
						_T( "CreateDefaultPlan for planTZ transpose failed" ));

					FFTPlan *trans1Plan = NULL;
					lockRAII *trans1Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTZ, &trans1Plan, &trans1Lock), _T( "FFTRepoGetPlan failed" ));

					trans1Plan->transflag = true;

					transLengths[0] = Nt * length1;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTZ, CLFFT_2D, transLengths), _T( "clfftSetPlanLength for planTZ transpose failed" ));

					switch (fftPlan->inputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							trans1Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							trans1Plan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					trans1Plan->placeness = CLFFT_OUTOFPLACE;
					trans1Plan->precision = fftPlan->precision;
					trans1Plan->tmpBufSize = 0;
					trans1Plan->batchsize = fftPlan->batchsize;
					trans1Plan->envelope = fftPlan->envelope;
					trans1Plan->forwardScale = 1.0f;
					trans1Plan->backwardScale = 1.0f;

					trans1Plan->inStride.buf[0] = 1;
					trans1Plan->inStride.buf[1] = Nt * length1;
					trans1Plan->outStride.buf[0] = 1;
					trans1Plan->outStride.buf[1] = length2;
					trans1Plan->iDist = fftPlan->iDist;
					trans1Plan->oDist = Nt * length1 * length2;
					trans1Plan->transOutHorizontal = true;

					trans1Plan->gen = Transpose_GCN;

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans1Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans1Plan->inStride, fftPlan->inStride.buf[index]);
						array_push_back(&trans1Plan->outStride, trans1Plan->oDist);
						trans1Plan->oDist *= fftPlan->length.buf[index];
					}

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						trans1Plan->hasPreCallback = true;
						trans1Plan->preCallback = fftPlan->preCallback;
						trans1Plan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTZ failed" ));

					// create col plan
					// complex to complex

					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planZ, fftPlan->context, CLFFT_1D, &fftPlan->length.buf[DimZ]),
						_T( "CreateDefaultPlan for planZ failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planZ, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					array_push_back(&colPlan->length, Nt * length1);

					colPlan->inStride.buf[0] = 1;
					array_push_back(&colPlan->inStride, length2);
					colPlan->iDist = trans1Plan->oDist;

					colPlan->placeness = CLFFT_INPLACE;
					colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
					colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;

					colPlan->outStride.buf[0] = colPlan->inStride.buf[0];
					array_push_back(&colPlan->outStride, colPlan->inStride.buf[1]);
					colPlan->oDist = colPlan->iDist;

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&colPlan->length, fftPlan->length.buf[index]);
						array_push_back(&colPlan->inStride, trans1Plan->outStride.buf[index - 1]);
						array_push_back(&colPlan->outStride, trans1Plan->outStride.buf[index - 1]);
					}

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = 1.0f;
					colPlan->backwardScale = 1.0f;
					colPlan->tmpBufSize = 0;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planZ failed" ));

					// create second transpose plan

					// Transpose
					// tmp --> output
					size_t trans2Lengths[2] = { length2, length0 * length1 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planTX, fftPlan->context, CLFFT_2D, trans2Lengths),
						_T( "CreateDefaultPlan for planTX transpose failed" ));

					FFTPlan *trans2Plan = NULL;
					lockRAII *trans2Lock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planTX, &trans2Plan, &trans2Lock), _T( "FFTRepoGetPlan failed" ));

					trans2Plan->transflag = true;

					trans2Lengths[1] = Nt * length1;
					OPENCL_V_LOCKED(sLock, clfftSetPlanLength(fftPlan->planTX, CLFFT_2D, trans2Lengths),
						_T( "clfftSetPlanLength for planTX transpose failed" ));

					trans2Plan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
					trans2Plan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;

					trans2Plan->placeness = CLFFT_OUTOFPLACE;
					trans2Plan->precision = fftPlan->precision;
					trans2Plan->tmpBufSize = 0;
					trans2Plan->batchsize = fftPlan->batchsize;
					trans2Plan->envelope = fftPlan->envelope;
					trans2Plan->forwardScale = 1.0f;
					trans2Plan->backwardScale = 1.0f;

					trans2Plan->inStride.buf[0] = 1;
					trans2Plan->inStride.buf[1] = length2;
					trans2Plan->outStride.buf[0] = 1;
					trans2Plan->outStride.buf[1] = Nt * length1;
					trans2Plan->iDist = colPlan->oDist;
					trans2Plan->oDist = Nt * length1 * length2;

					trans2Plan->gen = Transpose_GCN;
					trans2Plan->transflag = true;

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&trans2Plan->length, fftPlan->length.buf[index]);
						array_push_back(&trans2Plan->inStride, colPlan->outStride.buf[index - 1]);
						array_push_back(&trans2Plan->outStride, trans2Plan->oDist);
						trans2Plan->oDist *= fftPlan->length.buf[index];
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planTX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planTX failed" ));

					// create row plan
					// hermitian to real

					// create 2D xy plan
					size_t clLengths[] = { length0, length1, 0 };
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_2D, clLengths),
						_T( "CreateDefaultPlan for 2D planX failed" ));

					FFTPlan *rowPlan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &rowPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					rowPlan->outputLayout = fftPlan->outputLayout;
					rowPlan->inputLayout = CLFFT_HERMITIAN_INTERLEAVED;

					array_push_back(&rowPlan->length, length2);

					rowPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					rowPlan->outStride.buf[1] = fftPlan->outStride.buf[1];
					array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[2]);
					rowPlan->oDist = fftPlan->oDist;

					rowPlan->inStride.buf[0] = trans2Plan->outStride.buf[0];
					rowPlan->inStride.buf[1] = Nt;
					array_push_back(&rowPlan->inStride, Nt * length1);
					rowPlan->iDist = trans2Plan->oDist;

					for (size_t index = 3; index < array_size(&fftPlan->length); index++)
					{
						array_push_back(&rowPlan->length, fftPlan->length.buf[index]);
						array_push_back(&rowPlan->inStride, trans2Plan->outStride.buf[index - 1]);
						array_push_back(&rowPlan->outStride, fftPlan->outStride.buf[index]);
					}

					if (fftPlan->placeness == CLFFT_INPLACE)
						rowPlan->placeness = CLFFT_INPLACE;
					else
						rowPlan->placeness = CLFFT_OUTOFPLACE;

					rowPlan->precision = fftPlan->precision;
					rowPlan->forwardScale = fftPlan->forwardScale;
					rowPlan->backwardScale = fftPlan->backwardScale;
					rowPlan->tmpBufSize = 0;

					rowPlan->gen = fftPlan->gen;
					rowPlan->envelope = fftPlan->envelope;

					rowPlan->batchsize = fftPlan->batchsize;

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						rowPlan->hasPostCallback = true;
						rowPlan->postCallbackParam = fftPlan->postCallbackParam;
						rowPlan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan for planX failed" ));
				}
				else
				{
					size_t clLengths[] = { 1, 0, 0 };

					clLengths[0] = fftPlan->length.buf[DimZ];

					// create 1D col plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planZ, fftPlan->context, CLFFT_1D, clLengths),
						_T( "CreateDefaultPlan for planZ failed" ));

					FFTPlan *colPlan = NULL;
					lockRAII *colLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planZ, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

					switch (fftPlan->inputLayout)
					{
						case CLFFT_HERMITIAN_INTERLEAVED:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							colPlan->inputLayout = CLFFT_COMPLEX_INTERLEAVED;
						}
						break;
						case CLFFT_HERMITIAN_PLANAR:
						{
							colPlan->outputLayout = CLFFT_COMPLEX_INTERLEAVED;
							colPlan->inputLayout = CLFFT_COMPLEX_PLANAR;
						}
						break;
						default: assert(false);
					}

					array_push_back(&colPlan->length, Nt);
					array_push_back(&colPlan->length, length1);

					colPlan->inStride.buf[0] = fftPlan->inStride.buf[2];
					array_push_back(&colPlan->inStride, fftPlan->inStride.buf[0]);
					array_push_back(&colPlan->inStride, fftPlan->inStride.buf[1]);
					colPlan->iDist = fftPlan->iDist;

					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						colPlan->placeness = CLFFT_INPLACE;

						colPlan->outStride.buf[0] = colPlan->inStride.buf[0];
						array_push_back(&colPlan->outStride, colPlan->inStride.buf[1]);
						array_push_back(&colPlan->outStride, colPlan->inStride.buf[2]);
						colPlan->oDist = colPlan->iDist;

						for (size_t index = 3; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&colPlan->length, fftPlan->length.buf[index]);
							array_push_back(&colPlan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&colPlan->outStride, fftPlan->inStride.buf[index]);
						}
					}
					else
					{
						colPlan->placeness = CLFFT_OUTOFPLACE;

						colPlan->outStride.buf[0] = Nt * length1;
						array_push_back(&colPlan->outStride, 1);
						array_push_back(&colPlan->outStride, Nt);
						colPlan->oDist = Nt * length1 * length2;

						for (size_t index = 3; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&colPlan->length, fftPlan->length.buf[index]);
							array_push_back(&colPlan->inStride, fftPlan->inStride.buf[index]);
							array_push_back(&colPlan->outStride, colPlan->oDist);
							colPlan->oDist *= fftPlan->length.buf[index];
						}
					}

					colPlan->precision = fftPlan->precision;
					colPlan->forwardScale = 1.0f;
					colPlan->backwardScale = 1.0f;
					colPlan->tmpBufSize = 0;

					colPlan->gen = fftPlan->gen;
					colPlan->envelope = fftPlan->envelope;

					colPlan->batchsize = fftPlan->batchsize;

					// Set callback data if set on top level plan
					if (fftPlan->hasPreCallback)
					{
						colPlan->hasPreCallback = true;
						colPlan->preCallback = fftPlan->preCallback;
						colPlan->precallUserData = fftPlan->precallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan 3D->1D planZ failed" ));

					clLengths[0] = fftPlan->length.buf[DimX];
					clLengths[1] = fftPlan->length.buf[DimY];

					// create 2D xy plan
					OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_2D, clLengths),
						_T( "CreateDefaultPlan 2D planX failed" ));

					FFTPlan *xyPlan = NULL;
					lockRAII *rowLock = NULL;
					OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &xyPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

					xyPlan->inputLayout = CLFFT_HERMITIAN_INTERLEAVED;
					xyPlan->outputLayout = fftPlan->outputLayout;

					array_push_back(&xyPlan->length, length2);

					xyPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
					xyPlan->outStride.buf[1] = fftPlan->outStride.buf[1];
					array_push_back(&xyPlan->outStride, fftPlan->outStride.buf[2]);
					xyPlan->oDist = fftPlan->oDist;

					if (fftPlan->placeness == CLFFT_INPLACE)
					{
						xyPlan->placeness = CLFFT_INPLACE;

						xyPlan->inStride.buf[0] = colPlan->outStride.buf[1];
						xyPlan->inStride.buf[1] = colPlan->outStride.buf[2];
						array_push_back(&xyPlan->inStride, colPlan->outStride.buf[0]);
						xyPlan->iDist = colPlan->oDist;

						for (size_t index = 3; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&xyPlan->length, fftPlan->length.buf[index]);
							array_push_back(&xyPlan->inStride, colPlan->outStride.buf[index]);
							array_push_back(&xyPlan->outStride, fftPlan->outStride.buf[index]);
						}
					}
					else
					{
						xyPlan->placeness = CLFFT_OUTOFPLACE;

						xyPlan->inStride.buf[0] = 1;
						xyPlan->inStride.buf[1] = Nt;
						array_push_back(&xyPlan->inStride, Nt * length1);
						xyPlan->iDist = Nt * length1 * length2;

						for (size_t index = 3; index < array_size(&fftPlan->length); index++)
						{
							array_push_back(&xyPlan->length, fftPlan->length.buf[index]);
							array_push_back(&xyPlan->outStride, fftPlan->outStride.buf[index]);
							array_push_back(&xyPlan->inStride, xyPlan->iDist);
							xyPlan->iDist *= fftPlan->length.buf[index];
						}
					}

					xyPlan->precision = fftPlan->precision;
					xyPlan->forwardScale = fftPlan->forwardScale;
					xyPlan->backwardScale = fftPlan->backwardScale;
					xyPlan->tmpBufSize = fftPlan->tmpBufSize;

					xyPlan->gen = fftPlan->gen;
					xyPlan->envelope = fftPlan->envelope;

					xyPlan->batchsize = fftPlan->batchsize;

					// Set callback data if set on top level plan
					if (fftPlan->hasPostCallback)
					{
						xyPlan->hasPostCallback = true;
						xyPlan->postCallbackParam = fftPlan->postCallbackParam;
						xyPlan->postcallUserData = fftPlan->postcallUserData;
					}

					OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan 3D->2D planX failed" ));
				}
			}
			else
			{
				if (fftPlan->tmpBufSize == 0 &&
					(fftPlan->length.buf[0] > Large1DThreshold || fftPlan->length.buf[1] > Large1DThreshold || fftPlan->length.buf[2] > Large1DThreshold))
				{
					fftPlan->tmpBufSize =
						fftPlan->length.buf[0] * fftPlan->length.buf[1] * fftPlan->length.buf[2] * fftPlan->batchsize * FFTPlanElementSize(fftPlan);
				}

				size_t clLengths[] = { 1, 1, 0 };
				clLengths[0] = fftPlan->length.buf[DimX];
				clLengths[1] = fftPlan->length.buf[DimY];

				// create 2D xy plan
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planX, fftPlan->context, CLFFT_2D, clLengths),
					_T( "CreateDefaultPlan 2D planX failed" ));

				FFTPlan *xyPlan = NULL;
				lockRAII *rowLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planX, &xyPlan, &rowLock), _T( "FFTRepoGetPlan failed" ));

				xyPlan->inputLayout = fftPlan->inputLayout;
				xyPlan->outputLayout = fftPlan->outputLayout;
				xyPlan->placeness = fftPlan->placeness;
				xyPlan->precision = fftPlan->precision;
				xyPlan->forwardScale = 1.0f;
				xyPlan->backwardScale = 1.0f;
				xyPlan->tmpBufSize = fftPlan->tmpBufSize;

				xyPlan->gen = fftPlan->gen;
				xyPlan->envelope = fftPlan->envelope;

				// This is the xy fft, the first elements distance between the first
				// two FFTs is the distance of the first elements of the first two
				// rows in the original buffer.
				xyPlan->batchsize = fftPlan->batchsize;
				xyPlan->inStride.buf[0] = fftPlan->inStride.buf[0];
				xyPlan->inStride.buf[1] = fftPlan->inStride.buf[1];
				xyPlan->outStride.buf[0] = fftPlan->outStride.buf[0];
				xyPlan->outStride.buf[1] = fftPlan->outStride.buf[1];

				// pass length and other info to kernel, so the kernel knows this is
				// decomposed from higher dimension
				array_push_back(&xyPlan->length, fftPlan->length.buf[2]);
				array_push_back(&xyPlan->inStride, fftPlan->inStride.buf[2]);
				array_push_back(&xyPlan->outStride, fftPlan->outStride.buf[2]);
				xyPlan->iDist = fftPlan->iDist;
				xyPlan->oDist = fftPlan->oDist;

				// Set callback data if set on top level plan
				if (fftPlan->hasPreCallback)
				{
					xyPlan->hasPreCallback = true;
					xyPlan->preCallback = fftPlan->preCallback;
					xyPlan->precallUserData = fftPlan->precallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planX, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan 3D->2D planX failed" ));

				clLengths[0] = fftPlan->length.buf[DimZ];
				clLengths[1] = clLengths[2] = 0;
				// create 1D col plan
				OPENCL_V_LOCKED(sLock, clfftCreateDefaultPlanInternal(&fftPlan->planZ, fftPlan->context, CLFFT_1D, clLengths),
					_T( "CreateDefaultPlan for planZ failed" ));

				FFTPlan *colPlan = NULL;
				lockRAII *colLock = NULL;
				OPENCL_V_LOCKED(sLock, FFTRepoGetPlan(fftRepo, fftPlan->planZ, &colPlan, &colLock), _T( "FFTRepoGetPlan failed" ));

				colPlan->inputLayout = fftPlan->outputLayout;
				colPlan->outputLayout = fftPlan->outputLayout;
				colPlan->placeness = CLFFT_INPLACE;
				colPlan->precision = fftPlan->precision;
				colPlan->forwardScale = fftPlan->forwardScale;
				colPlan->backwardScale = fftPlan->backwardScale;
				colPlan->tmpBufSize = fftPlan->tmpBufSize;

				colPlan->gen = fftPlan->gen;
				colPlan->envelope = fftPlan->envelope;

				// This is a column FFT, the first elements distance between each
				// FFT is the distance of the first two elements in the original
				// buffer. Like a transpose of the matrix
				colPlan->batchsize = fftPlan->batchsize;
				colPlan->inStride.buf[0] = fftPlan->outStride.buf[2];
				colPlan->outStride.buf[0] = fftPlan->outStride.buf[2];

				// pass length and other info to kernel, so the kernel knows this is
				// decomposed from higher dimension
				array_push_back(&colPlan->length, fftPlan->length.buf[0]);
				array_push_back(&colPlan->length, fftPlan->length.buf[1]);
				array_push_back(&colPlan->inStride, fftPlan->outStride.buf[0]);
				array_push_back(&colPlan->inStride, fftPlan->outStride.buf[1]);
				array_push_back(&colPlan->outStride, fftPlan->outStride.buf[0]);
				array_push_back(&colPlan->outStride, fftPlan->outStride.buf[1]);
				colPlan->iDist = fftPlan->oDist;
				colPlan->oDist = fftPlan->oDist;

				// Set callback data if set on top level plan
				if (fftPlan->hasPostCallback)
				{
					colPlan->hasPostCallback = true;
					colPlan->postCallbackParam = fftPlan->postCallbackParam;
					colPlan->postcallUserData = fftPlan->postcallUserData;
				}

				OPENCL_V_LOCKED(sLock, clfftBakePlan(fftPlan->planZ, numQueues, commQueueFFT, NULL, NULL), _T( "BakePlan 3D->1D planZ failed" ));
			}

			fftPlan->baked = true;
			return clfftReturnLocked(sLock, CLFFT_SUCCESS);
		}
	}

	clfftStatus err = selectAction(fftPlan, &fftPlan->action, commQueueFFT);

	//	Allocate resources
	OPENCL_V_LOCKED(sLock, FFTPlanAllocateBuffers(fftPlan), "AllocateBuffers() failed");

	FFTPlanConstructAndEnqueueConstantBuffers(fftPlan, commQueueFFT);

	//	Record that we baked the plan
	fftPlan->baked = true;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

static clfftStatus FFTPlanConstructAndEnqueueConstantBuffers(FFTPlan *fftPlan, cl_command_queue *commQueueFFT)
{
	//	Construct the constant buffer and call clEnqueueWriteBuffer
	//
	cb_t ConstantBufferParams[CLFFT_CB_SIZE];
	memset(&ConstantBufferParams, 0, sizeof(ConstantBufferParams));

	ConstantBufferParams[0].u = clfft_max_cl_uint((cl_uint) 1, (cl_uint) fftPlan->batchsize);

	OPENCL_V(clEnqueueWriteBuffer(*commQueueFFT,
			 /*fftPlan->*/ fftPlan->const_buffer,
			 1, // TODO? non-blocking write?
			 0, sizeof(ConstantBufferParams), &ConstantBufferParams, 0, NULL, NULL),
		"clEnqueueWriteBuffer failed");

	return CLFFT_SUCCESS;
}

clfftStatus clfftDestroyPlan(clfftPlanHandle *plHandle)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	OPENCL_V(FFTRepoGetPlan(fftRepo, *plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));

	//	Recursively destroy subplans, that are used for higher dimensional FFT's
	if (fftPlan->planX)
		clfftDestroyPlan(&fftPlan->planX);
	if (fftPlan->planY)
		clfftDestroyPlan(&fftPlan->planY);
	if (fftPlan->planZ)
		clfftDestroyPlan(&fftPlan->planZ);
	if (fftPlan->planTX)
		clfftDestroyPlan(&fftPlan->planTX);
	if (fftPlan->planTY)
		clfftDestroyPlan(&fftPlan->planTY);
	if (fftPlan->planTZ)
		clfftDestroyPlan(&fftPlan->planTZ);
	if (fftPlan->planRCcopy)
		clfftDestroyPlan(&fftPlan->planRCcopy);
	if (fftPlan->planCopy)
		clfftDestroyPlan(&fftPlan->planCopy);

	FFTRepoDeletePlan(fftRepo, plHandle);

	return CLFFT_SUCCESS;
}

//	This routine will query the OpenCL context for it's devices
//	and their hardware limitations, which we synthesize into a
//	hardware "envelope".
//	We only query the devices the first time we're called after
//	the object's context is set.  On 2nd and subsequent calls,
//	we just return the pointer.
//
static clfftStatus FFTPlanSetEnvelope(FFTPlan *fftPlan)
{
	// TODO  The caller has already acquired the lock on *this
	//	However, we shouldn't depend on it.

	if (0 == fftPlan->envelope.limit_LocalMemSize)
		do
		{
			//	First time, query OpenCL for the device info
			//
			memset(&fftPlan->envelope, 0, sizeof(fftPlan->envelope));

			//	Get the size needed for the device list
			//
			size_t deviceListSize = 0;
			OPENCL_V(clGetContextInfo(fftPlan->context, CL_CONTEXT_DEVICES, 0, NULL, &deviceListSize), _T("Getting device array size ( clGetContextInfo() )" ));
			cl_uint n = (cl_uint) (deviceListSize / sizeof(cl_device_id));
			if (n == 0)
				break;

			array_cl_device_id devices;
			// Initialize dynamic array storage explicitly.
			array_init(&devices);
			// Allocate device list scratch storage explicitly.
			array_resize(&devices, n + 1);
			//	Get the device list
			//
			OPENCL_V(clGetContextInfo(fftPlan->context, CL_CONTEXT_DEVICES, deviceListSize, &devices.buf[0], NULL), "Getting device array ( clGetContextInfo() )");

			//	Get the # of devices
			//
			cl_uint cContextDevices = 0;

			size_t deviceVersionSize = 0;
			OPENCL_V(clGetDeviceInfo(devices.buf[0], CL_DEVICE_VERSION, 0, NULL, &deviceVersionSize),
				_T("Getting CL_DEVICE_VERSION Info string size ( clGetDeviceInfo() )" ));

			array_char szDeviceVersion;
			// Initialize dynamic array storage explicitly.
			array_init(&szDeviceVersion);
			// Allocate device version scratch storage explicitly.
			array_resize(&szDeviceVersion, deviceVersionSize);
			OPENCL_V(clGetDeviceInfo(devices.buf[0], CL_DEVICE_VERSION, deviceVersionSize, &szDeviceVersion.buf[0], NULL),
				_T("Getting CL_DEVICE_VERSION Platform Info string ( clGetDeviceInfo() )" ));

			char openclstr[11] = "OpenCL 1.0";

			if (!strncmp((const char *) &szDeviceVersion.buf[0], openclstr, 10))
			{
				cContextDevices = 1;
			}
			else
			{
				OPENCL_V(clGetContextInfo(fftPlan->context, CL_CONTEXT_NUM_DEVICES, sizeof(cContextDevices), &cContextDevices, NULL),
					_T("Getting number of context devices ( clGetContextInfo() )" ));
			}

			cContextDevices = clfft_min_cl_uint(cContextDevices, n);
			if (0 == cContextDevices)
				break;

			fftPlan->envelope.limit_LocalMemSize = 32768;
			fftPlan->envelope.limit_WorkGroupSize = 256;
			fftPlan->envelope.limit_Dimensions = countOf(fftPlan->envelope.limit_Size);
			for (size_t u = 0; u < countOf(fftPlan->envelope.limit_Size); ++u)
				fftPlan->envelope.limit_Size[u] = 256;

			for (cl_uint i = 0; i < cContextDevices; ++i)
			{
				cl_device_id devId = devices.buf[i];

				cl_ulong memsize = 0;
				unsigned int maxdim = 0;
				size_t temp[countOf(fftPlan->envelope.limit_Size)];
				memset(&temp, 0, sizeof(temp));

				OPENCL_V(clGetDeviceInfo(devId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &memsize, NULL),
					"Getting CL_DEVICE_LOCAL_MEM_SIZE device info ( "
					"clGetDeviceInfo() )");
				fftPlan->envelope.limit_LocalMemSize = clfft_min_size_t(fftPlan->envelope.limit_LocalMemSize, (size_t) memsize);

				OPENCL_V(clGetDeviceInfo(devId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(unsigned int), &maxdim, NULL),
					"Getting CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS "
					"device info ( clGetDeviceInfo() )");
				BUG_CHECK(countOf(fftPlan->envelope.limit_Size) >= maxdim);
				fftPlan->envelope.limit_Dimensions = clfft_min_size_t(fftPlan->envelope.limit_Dimensions, (size_t) maxdim);

				OPENCL_V(clGetDeviceInfo(devId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &temp[0], NULL),
					"Getting CL_DEVICE_MAX_WORK_GROUP_SIZE device "
					"info ( clGetDeviceInfo() )");
				fftPlan->envelope.limit_WorkGroupSize = clfft_min_size_t(fftPlan->envelope.limit_WorkGroupSize, temp[0]);

				OPENCL_V(clGetDeviceInfo(devId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(temp), &temp[0], NULL),
					"Getting CL_DEVICE_MAX_WORK_ITEM_SIZES device "
					"info ( clGetDeviceInfo() )");
				for (size_t u = 0; u < fftPlan->envelope.limit_Dimensions; ++u)
				{
					BUG_CHECK(temp[u] > 0)
					fftPlan->envelope.limit_Size[u] = clfft_min_size_t(fftPlan->envelope.limit_Size[u], temp[u]);
				}
			}

			BUG_CHECK(fftPlan->envelope.limit_LocalMemSize >= 1024)
		} while (0);

	return CLFFT_SUCCESS;
}

static clfftStatus FFTPlanAllocateBuffers(FFTPlan *fftPlan)
{
	cl_int status = CL_SUCCESS;

	assert(NULL == fftPlan->const_buffer);
	FFTPlanReleaseBuffers(fftPlan);

	assert(4 == sizeof(int));

	do
	{
		fftPlan->const_buffer = clCreateBuffer(fftPlan->context, CL_MEM_READ_ONLY, CLFFT_CB_SIZE * sizeof(int), 0, &status);
		if (CL_SUCCESS != status)
			break;
	} while (0);

	return (clfftStatus) status;
}

static clfftStatus FFTPlanReleaseBuffers(FFTPlan *fftPlan)
{
	clfftStatus result = CLFFT_SUCCESS;
	clfftStatus tmp;

	if (NULL != fftPlan->const_buffer)
	{
		tmp = ((clfftStatus) (clReleaseMemObject(fftPlan->const_buffer)));
		fftPlan->const_buffer = NULL;
		if (CLFFT_SUCCESS == result)
			result = tmp;
	}

	if ((NULL != fftPlan->intBuffer) && fftPlan->libCreatedIntBuffer)
	{
		tmp = ((clfftStatus) (clReleaseMemObject(fftPlan->intBuffer)));
		fftPlan->intBuffer = NULL;
		if (CLFFT_SUCCESS == result)
			result = tmp;
	}

	if (NULL != fftPlan->intBufferRC)
	{
		tmp = ((clfftStatus) (clReleaseMemObject(fftPlan->intBufferRC)));
		fftPlan->intBufferRC = NULL;
		if (CLFFT_SUCCESS == result)
			result = tmp;
	}

	if (NULL != fftPlan->intBufferC2R)
	{
		tmp = ((clfftStatus) (clReleaseMemObject(fftPlan->intBufferC2R)));
		fftPlan->intBufferC2R = NULL;
		if (CLFFT_SUCCESS == result)
			result = tmp;
	}

	return result;
}

static clfftStatus FFTPlanGetMax1DLength(const FFTPlan *fftPlan, size_t *longest)
{
	switch (fftPlan->gen)
	{
		case Stockham: return FFTPlanGetMax1DLengthStockham(fftPlan, longest);
		case Transpose_GCN: *longest = 4096; return CLFFT_SUCCESS;
		case Transpose_SQUARE: *longest = 4096; return CLFFT_SUCCESS;
		case Transpose_NONSQUARE: *longest = 4096; return CLFFT_SUCCESS;
		case Copy: *longest = 4096; return CLFFT_SUCCESS;
		default: assert(false); return CLFFT_NOTIMPLEMENTED;
	}
}

static clfftStatus FFTPlanGetEnvelope(const FFTPlan *fftPlan, const FFTEnvelope **ppEnvelope)
{
	if (&fftPlan->envelope == NULL)
	{
		assert(false);
		return CLFFT_NOTIMPLEMENTED;
	}

	*ppEnvelope = &fftPlan->envelope;
	return CLFFT_SUCCESS;
}

static size_t FFTPlanElementSize(const FFTPlan *fftPlan)
{
	return (((fftPlan->precision == CLFFT_DOUBLE) || (fftPlan->precision == CLFFT_DOUBLE_FAST)) ? 2 * sizeof(double) : 2 * sizeof(float));
}

/* End copied source: src\library\plan.cpp */

/* Begin copied source: src\library\transform.cpp */

// clfft.transform.cpp : Defines the entry point for the console application.
//

// #define DEBUGGING

clfftStatus clfftEnqueueTransform(clfftPlanHandle plHandle, clfftDirection dir, cl_uint numQueuesAndEvents, cl_command_queue *commQueues, cl_uint numWaitEvents,
	const cl_event *waitEvents, cl_event *outEvents, cl_mem *clInputBuffers, cl_mem *clOutputBuffers, cl_mem clTmpBuffers)
{
	cl_int status = CLFFT_SUCCESS;

	//	We do not currently support multiple command queues, which is necessary
	// to support multi-gpu operations
	if (numQueuesAndEvents > 1)
		return CLFFT_NOTIMPLEMENTED;

	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTPlan *fftPlan = NULL;
	lockRAII *planLock = NULL;

	//	At this point, the user wants to enqueue a plan to execute.  We lock the
	// plan down now, such that 	after we finish baking the plan (if the user
	// did not do that explicitely before), the plan cannot 	change again
	// through the action of other thread before we enqueue this plan for
	// execution.
	OPENCL_V(FFTRepoGetPlan(fftRepo, plHandle, &fftPlan, &planLock), _T( "FFTRepoGetPlan failed" ));
	// Enter the plan lock until the status helper releases it.
	lockRAII *sLock = planLock;
	lockRAIIEnter(sLock);

	if (fftPlan->baked == false)
	{
		OPENCL_V_LOCKED(sLock, clfftBakePlan(plHandle, numQueuesAndEvents, commQueues, NULL, NULL), _T( "Failed to bake plan" ));
	}

	// get the device information
	cl_device_id q_device;
	clGetCommandQueueInfo(*commQueues, CL_QUEUE_DEVICE, sizeof(cl_device_id), &q_device, NULL);

	// verify if the current device is the same as the one used for baking the
	// plan
	if (q_device != fftPlan->bakeDevice)
		return clfftReturnLocked(sLock, CLFFT_DEVICE_MISMATCH);

	if (fftPlan->inputLayout == CLFFT_REAL)
		dir = CLFFT_FORWARD;
	else if (fftPlan->outputLayout == CLFFT_REAL)
		dir = CLFFT_BACKWARD;

	// we do not check the user provided buffer at this release
	cl_mem localIntBuffer = clTmpBuffers;

	if (clTmpBuffers == NULL && fftPlan->tmpBufSize > 0 && fftPlan->intBuffer == NULL)
	{
		// create the intermediate buffers
		// The intermediate buffer is always interleave and packed
		// For outofplace operation, we have the choice not to create
		// intermediate buffer input ->(col+Transpose) output ->(col) output
		fftPlan->intBuffer = clCreateBuffer(fftPlan->context, CL_MEM_READ_WRITE, fftPlan->tmpBufSize, 0, &status);
		OPENCL_V_LOCKED(sLock, status, "Creating the intermediate buffer for large1D Failed");
		fftPlan->libCreatedIntBuffer = true;

#if defined(DEBUGGING)
		// Print the intermediate buffer allocation notice with C stdio.
		fprintf(stdout, "One intermediate buffer is created\n");
#endif
	}

	if (localIntBuffer == NULL && fftPlan->intBuffer != NULL)
		localIntBuffer = fftPlan->intBuffer;

	if (fftPlan->intBufferRC == NULL && fftPlan->tmpBufSizeRC > 0)
	{
		fftPlan->intBufferRC = clCreateBuffer(fftPlan->context, CL_MEM_READ_WRITE, fftPlan->tmpBufSizeRC, 0, &status);
		OPENCL_V_LOCKED(sLock, status, "Creating the intermediate buffer for large1D RC Failed");
	}

	if (fftPlan->intBufferC2R == NULL && fftPlan->tmpBufSizeC2R > 0)
	{
		fftPlan->intBufferC2R = clCreateBuffer(fftPlan->context, CL_MEM_READ_WRITE, fftPlan->tmpBufSizeC2R, 0, &status);
		OPENCL_V_LOCKED(sLock, status, "Creating the intermediate buffer for large1D YZ C2R Failed");
	}

	//	The largest vector we can transform in a single pass
	//	depends on the GPU caps -- especially the amount of LDS
	//	available
	//
	size_t Large1DThreshold = 0;
	OPENCL_V_LOCKED(sLock, FFTPlanGetMax1DLength(fftPlan, &Large1DThreshold), "GetMax1DLength failed");
	BUG_CHECK(Large1DThreshold > 1);

	// Large1DThreshold = 128;

	if (fftPlan->gen != Copy)
		switch (fftPlan->dim)
		{
			case CLFFT_1D:
			{
				if (Is1DPossible(fftPlan->length.buf[0], Large1DThreshold))
					break;

				if ((fftPlan->inputLayout == CLFFT_REAL) && (fftPlan->planTZ != 0))
				{
					// First transpose
					//  Input->tmp
					cl_event transTXOutEvents = NULL;
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &transTXOutEvents,
							clInputBuffers, &localIntBuffer, NULL),
						"clfftEnqueueTransform for large1D transTX failed");

					cl_mem *mybuffers;
					if (fftPlan->placeness == CLFFT_INPLACE)
						mybuffers = clInputBuffers;
					else
						mybuffers = clOutputBuffers;

#if defined(DEBUGGING)
					//  For debugging interleave data only,
					//  read the input buffer back into memory.
					clFinish(*commQueues);
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					// First Row
					// tmp->output
					cl_event rowXOutEvents = NULL;
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transTXOutEvents, &rowXOutEvents, &localIntBuffer,
							&(fftPlan->intBufferRC), NULL),
						"clfftEnqueueTransform for large1D rowX failed");
					clReleaseEvent(transTXOutEvents);

#if defined(DEBUGGING)
					//  For debugging interleave data only,
					//  read the input buffer back into memory.
					clFinish(*commQueues);
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, *mybuffers, CL_TRUE, 0, 536870912, &temp.buf[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					// Second Transpose
					//  output->tmp
					cl_event transTYOutEvents = NULL;
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &rowXOutEvents, &transTYOutEvents,
							&(fftPlan->intBufferRC), &localIntBuffer, NULL),
						"clfftEnqueueTransform for large1D transTY failed");
					clReleaseEvent(rowXOutEvents);

#if defined(DEBUGGING)
					//  For debugging interleave data only,
					//  read the input buffer back into memory.
					clFinish(*commQueues);
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					// Second Row
					// tmp->tmp, inplace
					cl_event rowYOutEvents = NULL;
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transTYOutEvents, &rowYOutEvents, &localIntBuffer,
							&(fftPlan->intBufferRC), NULL),
						"clfftEnqueueTransform for large1D rowY failed");
					clReleaseEvent(transTYOutEvents);

#if defined(DEBUGGING)
					//  For debugging interleave data only,
					//  read the input buffer back into memory.
					clFinish(*commQueues);
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					// Third Transpose
					//  tmp->output
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planTZ, dir, numQueuesAndEvents, commQueues, 1, &rowYOutEvents, outEvents, &(fftPlan->intBufferRC),
							mybuffers, NULL),
						"clfftEnqueueTransform for large1D transTZ failed");
					clReleaseEvent(rowYOutEvents);
				}
				else if (fftPlan->inputLayout == CLFFT_REAL)
				{
					cl_event colOutEvents = NULL;
					cl_event copyInEvents = NULL;

					// First pass
					// column with twiddle first, OUTOFPLACE, + transpose
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planX, CLFFT_FORWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &colOutEvents,
							clInputBuffers, &(fftPlan->intBufferRC), localIntBuffer),
						"clfftEnqueueTransform large1D col pass failed");

					cl_mem *out_local;
					out_local = (fftPlan->placeness == CLFFT_INPLACE) ? clInputBuffers : clOutputBuffers;

					// another column FFT output, INPLACE
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planY, CLFFT_FORWARD, numQueuesAndEvents, commQueues, 1, &colOutEvents, &copyInEvents,
							&(fftPlan->intBufferRC), &(fftPlan->intBufferRC), localIntBuffer),
						"clfftEnqueueTransform large1D second column failed");
					clReleaseEvent(colOutEvents);

					// copy from full complex to hermitian
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planRCcopy, CLFFT_FORWARD, numQueuesAndEvents, commQueues, 1, &copyInEvents, outEvents,
							&(fftPlan->intBufferRC), out_local, localIntBuffer),
						"clfftEnqueueTransform large1D RC copy failed");
					clReleaseEvent(copyInEvents);
				}
				else if (fftPlan->outputLayout == CLFFT_REAL)
				{
					cl_event colOutEvents = NULL;
					cl_event copyOutEvents = NULL;

					if (fftPlan->planRCcopy)
					{
						// copy from hermitian to full complex
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planRCcopy, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
								&copyOutEvents, clInputBuffers, &(fftPlan->intBufferRC), localIntBuffer),
							"clfftEnqueueTransform large1D RC copy failed");

						// First pass
						// column with twiddle first, INPLACE,
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, 1, &copyOutEvents, &colOutEvents,
								&(fftPlan->intBufferRC), &(fftPlan->intBufferRC), localIntBuffer),
							"clfftEnqueueTransform large1D col pass failed");
						clReleaseEvent(copyOutEvents);
					}
					else
					{
						// First pass
						// column with twiddle first, INPLACE,
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
								&colOutEvents, clInputBuffers, &(fftPlan->intBufferRC), localIntBuffer),
							"clfftEnqueueTransform large1D col pass failed");
						clReleaseEvent(copyOutEvents);
					}

					cl_mem *out_local;
					out_local = (fftPlan->placeness == CLFFT_INPLACE) ? clInputBuffers : clOutputBuffers;

					// another column FFT output, OUTOFPLACE + transpose
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planY, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, 1, &colOutEvents, outEvents,
							&(fftPlan->intBufferRC), out_local, localIntBuffer),
						"clfftEnqueueTransform large1D second column failed");
					clReleaseEvent(colOutEvents);
				}
				else
				{
#if defined(DEBUGGING)
					// For debugging interleave data only, initialize the
					// intermediate buffer to a data pattern.  This will show which
					// data in the buffer are being written by the kernel
					//
					size_t buffSizeBytes_complex = fftPlan->tmpBufSize;
					size_t buffersize = buffSizeBytes_complex / (2 * sizeof(float));
					array_float temp;
					// Initialize dynamic array storage explicitly.
					array_init(&temp);
					// Allocate temporary complex buffer storage explicitly.
					array_resize(&temp, buffersize * 2);

					for (size_t u = 0; u < buffersize; ++u)
					{
						temp.buf[2 * u] = (float) (u + 1);
						temp.buf[2 * u + 1] = (float) (buffersize - u);
					}

					if (fftPlan->large1D == 0)
					{
						// First time usage, we can initialize tmp buffer
						OPENCL_V_LOCKED(sLock,
							clEnqueueWriteBuffer(*commQueues, localIntBuffer,
								CL_TRUE, // blocking write
								0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
							"clEnqueueWriteBuffer failed");
					}
#endif

					if (fftPlan->transflag)
					{
						// First transpose
						//  Input->tmp
						cl_event transTXOutEvents = NULL;
						if (fftPlan->allOpsInplace)
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&transTXOutEvents, clInputBuffers, NULL, NULL),
								"clfftEnqueueTransform for large1D transTX "
								"failed");
						}
						else
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&transTXOutEvents, clInputBuffers, &localIntBuffer, NULL),
								"clfftEnqueueTransform for large1D "
								"transTX failed");
						}

						cl_mem *mybuffers;
						if (fftPlan->placeness == CLFFT_INPLACE)
							mybuffers = clInputBuffers;
						else
							mybuffers = clOutputBuffers;

#if defined(DEBUGGING)
						//  For debugging interleave data only,
						//  read the input buffer back into memory.
						clFinish(*commQueues);
						OPENCL_V_LOCKED(sLock,
							clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// First Row
						// tmp->output
						cl_event rowXOutEvents = NULL;
						if (fftPlan->allOpsInplace)
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transTXOutEvents, &rowXOutEvents,
									clInputBuffers, NULL, NULL),
								"clfftEnqueueTransform for large1D "
								"rowX failed");
						}
						else
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transTXOutEvents, &rowXOutEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform for large1D "
								"rowX failed");
						}
						clReleaseEvent(transTXOutEvents);

#if defined(DEBUGGING)
						//  For debugging interleave data only,
						//  read the input buffer back into memory.
						clFinish(*commQueues);
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, *mybuffers, CL_TRUE, 0, 536870912, &temp.buf[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Second Transpose
						//  output->tmp
						cl_event transTYOutEvents = NULL;
						if (fftPlan->allOpsInplace)
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &rowXOutEvents, &transTYOutEvents,
									clInputBuffers, NULL, NULL),
								"clfftEnqueueTransform for large1D "
								"transTY failed");
						}
						else
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &rowXOutEvents, &transTYOutEvents,
									mybuffers, &localIntBuffer, NULL),
								"clfftEnqueueTransform for large1D "
								"transTY failed");
						}
						clReleaseEvent(rowXOutEvents);

#if defined(DEBUGGING)
						//  For debugging interleave data only,
						//  read the input buffer back into memory.
						clFinish(*commQueues);
						OPENCL_V_LOCKED(sLock,
							clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Second Row
						// tmp->tmp, inplace
						cl_event rowYOutEvents = NULL;
						if (fftPlan->allOpsInplace)
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transTYOutEvents, &rowYOutEvents,
									clInputBuffers, NULL, NULL),
								"clfftEnqueueTransform for large1D "
								"rowY failed");
						}
						else
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transTYOutEvents, &rowYOutEvents,
									&localIntBuffer, NULL, NULL),
								"clfftEnqueueTransform for large1D "
								"rowY failed");
						}
						clReleaseEvent(transTYOutEvents);

#if defined(DEBUGGING)
						//  For debugging interleave data only,
						//  read the input buffer back into memory.
						clFinish(*commQueues);
						OPENCL_V_LOCKED(sLock,
							clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Third Transpose
						//  tmp->output
						if (fftPlan->allOpsInplace)
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTZ, dir, numQueuesAndEvents, commQueues, 1, &rowYOutEvents, outEvents,
									clInputBuffers, NULL, NULL),
								"clfftEnqueueTransform for large1D "
								"transTZ failed");
						}
						else
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTZ, dir, numQueuesAndEvents, commQueues, 1, &rowYOutEvents, outEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform for large1D "
								"transTZ failed");
						}
						clReleaseEvent(rowYOutEvents);
					}
					else if (fftPlan->large1D == 0)
					{
						if (fftPlan->planCopy)
						{
							// Transpose OUTOFPLACE
							cl_event transTXOutEvents = NULL;
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&transTXOutEvents, clInputBuffers, &localIntBuffer, NULL),
								"clfftEnqueueTransform for large1D "
								"transTX failed");

#if defined(DEBUGGING)
							//  For debugging interleave data only,
							//  read the input buffer back into memory.
							clFinish(*commQueues);
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// FFT INPLACE
							cl_event rowXOutEvents = NULL;
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transTXOutEvents, &rowXOutEvents,
									&localIntBuffer, NULL, NULL),
								"clfftEnqueueTransform large1D "
								"first row pass failed");
							clReleaseEvent(transTXOutEvents);

#if defined(DEBUGGING)
							//  For debugging interleave data only,
							//  read the input buffer back into memory.
							clFinish(*commQueues);
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// FFT INPLACE
							cl_event colYOutEvents = NULL;
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &rowXOutEvents, &colYOutEvents,
									&localIntBuffer, NULL, NULL),
								"clfftEnqueueTransform large1D "
								"second column failed");
							clReleaseEvent(rowXOutEvents);

#if defined(DEBUGGING)
							//  For debugging interleave data only,
							//  read the input buffer back into memory.
							clFinish(*commQueues);
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							cl_mem *mybuffers;
							if (fftPlan->placeness == CLFFT_INPLACE)
								mybuffers = clInputBuffers;
							else
								mybuffers = clOutputBuffers;

							// Copy kernel
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planCopy, dir, numQueuesAndEvents, commQueues, 1, &colYOutEvents, outEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform large1D copy failed");
							clReleaseEvent(colYOutEvents);
						}
						else
						{
							cl_event colOutEvents = NULL;
							// First pass
							// column with twiddle first, OUTOFPLACE
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &colOutEvents,
									clInputBuffers, &localIntBuffer, NULL),
								"clfftEnqueueTransform large1D col "
								"pass failed");

#if defined(DEBUGGING)
							// debug purpose, interleave input <-> interleave output
							// read the intermediate buffer and print part of it.
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 1, &colOutEvents,
									NULL),
								"Reading the result buffer failed");
#endif
							if (fftPlan->planTZ)
							{
								cl_event rowYOutEvents = NULL;
								OPENCL_V_LOCKED(sLock,
									clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents, &rowYOutEvents,
										&localIntBuffer, NULL, NULL),
									"clfftEnqueueTransform large1D "
									"second row failed");

								if (fftPlan->placeness == CLFFT_INPLACE)
								{
									OPENCL_V_LOCKED(sLock,
										clfftEnqueueTransform(fftPlan->planTZ, dir, numQueuesAndEvents, commQueues, 1, &rowYOutEvents,
											outEvents, &localIntBuffer, clInputBuffers, NULL),
										"clfftEnqueueTransform large1D trans3 "
										"failed");
								}
								else
								{
									OPENCL_V_LOCKED(sLock,
										clfftEnqueueTransform(fftPlan->planTZ, dir, numQueuesAndEvents, commQueues, 1, &rowYOutEvents,
											outEvents, &localIntBuffer, clOutputBuffers, NULL),
										"clfftEnqueueTransform large1D trans3 "
										"failed");
								}

								clReleaseEvent(rowYOutEvents);
							}
							else
							{
								// another column FFT output, OUTOFPLACE + transpose
								if (fftPlan->placeness == CLFFT_INPLACE)
								{
									OPENCL_V_LOCKED(sLock,
										clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents,
											outEvents, &localIntBuffer, clInputBuffers, NULL),
										"clfftEnqueueTransform large1D second "
										"column failed");

#if defined(DEBUGGING)
									//  For debugging interleave data only,
									//  read the input buffer back into memory.
									OPENCL_V_LOCKED(sLock,
										clEnqueueReadBuffer(*commQueues, clInputBuffers[0], CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0],
											1, outEvents, NULL),
										"Reading the result buffer failed");
#endif
								}
								else
								{
#if defined(DEBUGGING)
									// debug purpose, interleave input <->
									// interleave output
									OPENCL_V_LOCKED(sLock,
										clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 1,
											&colOutEvents, NULL),
										"Reading the result buffer failed");
#endif
									OPENCL_V_LOCKED(sLock,
										clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents,
											outEvents, &localIntBuffer, clOutputBuffers, NULL),
										"clfftEnqueueTransform large1D second "
										"column failed");

#if defined(DEBUGGING)
									//  For debugging interleave data only, read
									//  back the output buffer
									//
									OPENCL_V_LOCKED(sLock,
										clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes_complex,
											&temp.buf[0], 1, outEvents, NULL),
										"Reading the result buffer failed");
#endif
								}
							}

							clReleaseEvent(colOutEvents);
						}
					}
					else
					{
						cl_event colOutEvents = NULL;

						// second pass for huge 1D
						// column with twiddle first, OUTOFPLACE, + transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &colOutEvents,
								&localIntBuffer, clOutputBuffers, localIntBuffer),
							"clfftEnqueueTransform Huge1D col pass failed");
#if defined(DEBUGGING)
						// debug purpose, interleave input <-> interleave output
						OPENCL_V_LOCKED(sLock,
							clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes_complex, &temp.buf[0], 1, &colOutEvents,
								NULL),
							"Reading the result buffer failed");
#endif

						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents, outEvents, clOutputBuffers,
								clOutputBuffers, localIntBuffer),
							"clfftEnqueueTransform large1D second "
							"column failed");

						clReleaseEvent(colOutEvents);
					}
				}
				return clfftReturnLocked(sLock, CLFFT_SUCCESS);
			}
			case CLFFT_2D:
			{
				// if transpose kernel, we will fall below
				if (fftPlan->transflag && !(fftPlan->planTX))
					break;

				if ((fftPlan->gen == Transpose_NONSQUARE) && (fftPlan->nonSquareKernelType == NON_SQUARE_TRANS_PARENT))
				{
					cl_event stage1OutEvents = NULL;

					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &stage1OutEvents,
							clInputBuffers, NULL, NULL),
						"clfftEnqueueTransform stage1 failed");

					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &stage1OutEvents, outEvents, clInputBuffers, NULL,
							NULL),
						"clfftEnqueueTransform stage2 failed");
					clReleaseEvent(stage1OutEvents);
					return clfftReturnLocked(sLock, CLFFT_SUCCESS);
				}

				cl_event rowOutEvents = NULL;

#if defined(DEBUGGING)
				size_t buffersize = fftPlan->length.buf[0] * fftPlan->length.buf[1] * fftPlan->batchsize;
				if (array_size(&fftPlan->length) > 2)
					buffersize *= fftPlan->length.buf[2];
				// size_t buffSizeBytes=(2 * sizeof(float))*buffersize;
				// float *output2 = NULL;
				size_t buffSizeBytes = sizeof(float) * buffersize;
				float *output2 = (float *) clfft_checked_malloc(buffersize * 2 * sizeof(float));
#endif
#if defined(DEBUGGING)
				OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, clInputBuffers[0], CL_TRUE, 0, buffSizeBytes, &output2[0], 0, NULL, NULL),
					"Reading the result buffer failed");

				if (fftPlan->placeness == CLFFT_OUTOFPLACE)
				{
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes, &output2[0], 0, NULL, NULL),
						"Reading the result buffer failed");
				}
#endif
				if (fftPlan->transflag)
				{ // first time set up transpose kernel for 2D
					// First row
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents, clInputBuffers,
							clOutputBuffers, NULL),
						"clfftEnqueueTransform for row failed");

					cl_mem *mybuffers;

					if (fftPlan->placeness == CLFFT_INPLACE)
						mybuffers = clInputBuffers;
					else
						mybuffers = clOutputBuffers;

#if defined(DEBUGGING)
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					cl_event transXOutEvents = NULL;
					cl_event colOutEvents = NULL;

					if (!fftPlan->transpose_in_2d_inplace)
					{
						// First transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, &transXOutEvents, mybuffers,
								&localIntBuffer, NULL),
							"clfftEnqueueTransform for first transpose failed");
						clReleaseEvent(rowOutEvents);

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						if (fftPlan->transposed == CLFFT_NOTRANSPOSE)
						{
							// Second Row transform
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, &colOutEvents,
									&localIntBuffer, NULL, NULL),
								"clfftEnqueueTransform for second row failed");
							clReleaseEvent(transXOutEvents);

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// Second transpose
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents, outEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform for second "
								"transpose failed");
							clReleaseEvent(colOutEvents);

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif
						}
						else
						{
							// Second Row transform
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, outEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform for second row failed");
							clReleaseEvent(transXOutEvents);
						}
					}
					else
					{
						// First Transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, &transXOutEvents, mybuffers,
								NULL, NULL),
							"clfftEnqueueTransform for first transpose failed");
						clReleaseEvent(rowOutEvents);

						if (fftPlan->transposed == CLFFT_NOTRANSPOSE)
						{
							// Second Row transform
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, &colOutEvents,
									mybuffers, NULL, NULL),
								"clfftEnqueueTransform for Second Row failed");
							clReleaseEvent(transXOutEvents);

							// Second transpose
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents, outEvents, mybuffers,
									NULL, NULL),
								"clfftEnqueueTransform for second "
								"transpose failed");
							clReleaseEvent(colOutEvents);
						}
						else
						{
							// Second Row transform
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, outEvents,
									mybuffers, NULL, NULL),
								"clfftEnqueueTransform for second row failed");
							clReleaseEvent(transXOutEvents);
						}
					}
				}
				else
				{
					if ((fftPlan->large2D || array_size(&fftPlan->length) > 2) && (fftPlan->inputLayout != CLFFT_REAL) && (fftPlan->outputLayout != CLFFT_REAL))
					{
						if (fftPlan->placeness == CLFFT_INPLACE)
						{
							// deal with row first
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents,
									clInputBuffers, NULL, localIntBuffer),
								"clfftEnqueueTransform for row failed");

							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									clInputBuffers, NULL, localIntBuffer),
								"clfftEnqueueTransform for column failed");
						}
						else
						{
							// deal with row first
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents,
									clInputBuffers, clOutputBuffers, localIntBuffer),
								"clfftEnqueueTransform for row failed");

							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									clOutputBuffers, NULL, localIntBuffer),
								"clfftEnqueueTransform for column failed");
						}
					}
					else if (fftPlan->inputLayout == CLFFT_REAL)
					{
						if (fftPlan->planTX)
						{
							// First row
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents,
									clInputBuffers, clOutputBuffers, NULL),
								"clfftEnqueueTransform for row failed");

							cl_mem *mybuffers;

							if (fftPlan->placeness == CLFFT_INPLACE)
								mybuffers = clInputBuffers;
							else
								mybuffers = clOutputBuffers;

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							cl_event transXOutEvents = NULL;
							cl_event colOutEvents = NULL;

							// First transpose
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, &transXOutEvents,
									mybuffers, &localIntBuffer, NULL),
								"clfftEnqueueTransform for first "
								"transpose failed");
						// clReleaseEvent(rowOutEvents);

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// Second Row transform
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, &colOutEvents,
									&localIntBuffer, NULL, NULL),
								"clfftEnqueueTransform for second row failed");
							clReleaseEvent(transXOutEvents);

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// Second transpose
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents, outEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform for second "
								"transpose failed");
							clReleaseEvent(colOutEvents);

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif
						}
						else if (fftPlan->placeness == CLFFT_INPLACE)
						{
							// deal with row
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, CLFFT_FORWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&rowOutEvents, clInputBuffers, NULL, localIntBuffer),
								"clfftEnqueueTransform for row failed");

							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, CLFFT_FORWARD, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									clInputBuffers, NULL, localIntBuffer),
								"clfftEnqueueTransform for column failed");
						}
						else
						{
							// deal with row
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, CLFFT_FORWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&rowOutEvents, clInputBuffers, clOutputBuffers, localIntBuffer),
								"clfftEnqueueTransform for row failed");

							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, CLFFT_FORWARD, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									clOutputBuffers, NULL, localIntBuffer),
								"clfftEnqueueTransform for column failed");
						}
					}
					else if (fftPlan->outputLayout == CLFFT_REAL)
					{
						if (fftPlan->planTY)
						{
							cl_mem *mybuffers;

							if ((fftPlan->placeness == CLFFT_INPLACE) ||
								((fftPlan->placeness == CLFFT_OUTOFPLACE) && (array_size(&fftPlan->length) > 2)))
								mybuffers = clInputBuffers;
							else
								mybuffers = &(fftPlan->intBufferC2R);

							cl_event transYOutEvents = NULL;
							cl_event transXOutEvents = NULL;

							// First transpose
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&transYOutEvents, clInputBuffers, &localIntBuffer, NULL),
								"clfftEnqueueTransform for first "
								"transpose failed");

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// First row
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &transYOutEvents, &rowOutEvents,
									&localIntBuffer, NULL, NULL),
								"clfftEnqueueTransform for col failed");
							clReleaseEvent(transYOutEvents);

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// Second transpose
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, &transXOutEvents,
									&localIntBuffer, mybuffers, NULL),
								"clfftEnqueueTransform for second "
								"transpose failed");

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif

							// Second Row transform
							if (fftPlan->placeness == CLFFT_INPLACE)
							{
								OPENCL_V_LOCKED(sLock,
									clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, outEvents,
										clInputBuffers, NULL, NULL),
									"clfftEnqueueTransform for "
									"second row failed");
							}
							else
							{
								OPENCL_V_LOCKED(sLock,
									clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, outEvents,
										mybuffers, clOutputBuffers, NULL),
									"clfftEnqueueTransform for second row "
									"failed");
							}
							clReleaseEvent(transXOutEvents);
#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
								"Reading the result buffer failed");
#endif
						}
						else
						{
							cl_mem *out_local, *int_local, *out_y;

							if (fftPlan->placeness == CLFFT_INPLACE)
							{
								out_local = NULL;
								int_local = NULL;
								out_y = clInputBuffers;
							}
							else if (array_size(&fftPlan->length) > 2)
							{
								out_local = clOutputBuffers;
								int_local = NULL;
								out_y = clInputBuffers;
							}
							else
							{
								out_local = clOutputBuffers;
								int_local = &(fftPlan->intBufferC2R);
								out_y = int_local;
							}

							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
									&rowOutEvents, clInputBuffers, int_local, localIntBuffer),
								"clfftEnqueueTransform for row failed");

							// deal with row
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									out_y, out_local, localIntBuffer),
								"clfftEnqueueTransform for column failed");
						}
					}
					else
					{
						// deal with row first
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents,
								clInputBuffers, &localIntBuffer, NULL),
							"clfftEnqueueTransform for row failed");

						if (fftPlan->placeness == CLFFT_INPLACE)
						{
							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									&localIntBuffer, clInputBuffers, NULL),
								"clfftEnqueueTransform for column failed");
						}
						else
						{
							// deal with column
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planY, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents,
									&localIntBuffer, clOutputBuffers, NULL),
								"clfftEnqueueTransform for column failed");

#if defined(DEBUGGING)
							OPENCL_V_LOCKED(sLock,
								clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes, &output2[0], 1, outEvents, NULL),
								"Reading the result buffer failed");
#endif
						}
					}

					clReleaseEvent(rowOutEvents);
				}
				return clfftReturnLocked(sLock, CLFFT_SUCCESS);
			}
			case CLFFT_3D:
			{
				cl_event rowOutEvents = NULL;

#if defined(DEBUGGING)
				size_t buffersize = fftPlan->length.buf[0] * fftPlan->length.buf[1] * fftPlan->length.buf[2] * fftPlan->batchsize;
				size_t buffSizeBytes = (2 * sizeof(float)) * buffersize;
				array_float output3;
				// Initialize dynamic array storage explicitly.
				array_init(&output3);
				// Allocate debug output scratch storage explicitly.
				array_resize(&output3, buffersize * 2);
#endif
				if (fftPlan->inputLayout == CLFFT_REAL)
				{
					if (fftPlan->planTX)
					{
						// First row
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents,
								clInputBuffers, clOutputBuffers, localIntBuffer),
							"clfftEnqueueTransform for row failed");

						cl_mem *mybuffers;

						if (fftPlan->placeness == CLFFT_INPLACE)
							mybuffers = clInputBuffers;
						else
							mybuffers = clOutputBuffers;

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						cl_event transXOutEvents = NULL;
						cl_event colOutEvents = NULL;

						// First transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, &transXOutEvents, mybuffers,
								&localIntBuffer, NULL),
							"clfftEnqueueTransform for first transpose failed");
					// clReleaseEvent(rowOutEvents);

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Second Row transform
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planZ, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, &colOutEvents,
								&localIntBuffer, NULL, NULL),
							"clfftEnqueueTransform for second row failed");
						clReleaseEvent(transXOutEvents);

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Second transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planTY, dir, numQueuesAndEvents, commQueues, 1, &colOutEvents, outEvents, &localIntBuffer,
								mybuffers, NULL),
							"clfftEnqueueTransform for second transpose "
							"failed");
						clReleaseEvent(colOutEvents);

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif
					}
					else
					{
						cl_mem *tmp_local, *out_local;

						tmp_local = (fftPlan->placeness == CLFFT_INPLACE) ? NULL : clOutputBuffers;
						out_local = (fftPlan->placeness == CLFFT_INPLACE) ? clInputBuffers : clOutputBuffers;

						// deal with 2D row first
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, CLFFT_FORWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
								&rowOutEvents, clInputBuffers, tmp_local, localIntBuffer),
							"clfftEnqueueTransform for 3D-XY row failed");

						// deal with 1D Z column
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planZ, CLFFT_FORWARD, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents, out_local,
								NULL, localIntBuffer),
							"clfftEnqueueTransform for 3D-Z column failed");
					}
				}
				else if (fftPlan->outputLayout == CLFFT_REAL)
				{
					if (fftPlan->planTZ)
					{
						cl_mem *mybuffers;

						if (fftPlan->placeness == CLFFT_INPLACE)
							mybuffers = clInputBuffers;
						else
							mybuffers = &(fftPlan->intBufferC2R);

						cl_event transZOutEvents = NULL;
						cl_event transXOutEvents = NULL;

						// First transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planTZ, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &transZOutEvents,
								clInputBuffers, &localIntBuffer, NULL),
							"clfftEnqueueTransform for first transpose failed");

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// First row
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planZ, dir, numQueuesAndEvents, commQueues, 1, &transZOutEvents, &rowOutEvents,
								&localIntBuffer, NULL, NULL),
							"clfftEnqueueTransform for col failed");
						clReleaseEvent(transZOutEvents);

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, mybuffers[0], CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Second transpose
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planTX, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, &transXOutEvents,
								&localIntBuffer, mybuffers, NULL),
							"clfftEnqueueTransform for second transpose "
							"failed");

#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif

						// Second Row transform
						if (fftPlan->placeness == CLFFT_INPLACE)
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, outEvents,
									clInputBuffers, NULL, NULL),
								"clfftEnqueueTransform for second row failed");
						}
						else
						{
							OPENCL_V_LOCKED(sLock,
								clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, 1, &transXOutEvents, outEvents,
									mybuffers, clOutputBuffers, NULL),
								"clfftEnqueueTransform for second row failed");
						}
						clReleaseEvent(transXOutEvents);
#if defined(DEBUGGING)
						OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, localIntBuffer, CL_TRUE, 0, buffSizeBytes * 2, &output2[0], 0, NULL, NULL),
							"Reading the result buffer failed");
#endif
					}
					else
					{
						cl_mem *out_local, *int_local, *out_z;

						if (fftPlan->placeness == CLFFT_INPLACE)
						{
							out_local = NULL;
							int_local = NULL;
							out_z = clInputBuffers;
						}
						else
						{
							out_local = clOutputBuffers;
							int_local = &(fftPlan->intBufferC2R);
							out_z = int_local;
						}

						// deal with 1D Z column first
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planZ, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents,
								&rowOutEvents, clInputBuffers, int_local, localIntBuffer),
							"clfftEnqueueTransform for 3D-Z column failed");

						// deal with 2D row
						OPENCL_V_LOCKED(sLock,
							clfftEnqueueTransform(fftPlan->planX, CLFFT_BACKWARD, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents, out_z,
								out_local, localIntBuffer),
							"clfftEnqueueTransform for 3D-XY row failed");
					}
				}
				else if (fftPlan->placeness == CLFFT_INPLACE)
				{
					// deal with 2D row first
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents, clInputBuffers,
							NULL, localIntBuffer),
						"clfftEnqueueTransform for 3D-XY row failed");

					// deal with 1D Z column
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planZ, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents, clInputBuffers, NULL,
							localIntBuffer),
						"clfftEnqueueTransform for 3D-Z column failed");
				}
				else
				{
#if defined(DEBUGGING)
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes, &output3.buf[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					// deal with 2D row first
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planX, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, &rowOutEvents, clInputBuffers,
							clOutputBuffers, localIntBuffer),
						"clfftEnqueueTransform for 3D-XY row failed");

#if defined(DEBUGGING)
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes, &output3.buf[0], 0, NULL, NULL),
						"Reading the result buffer failed");
#endif

					// deal with 1D Z column
					OPENCL_V_LOCKED(sLock,
						clfftEnqueueTransform(fftPlan->planZ, dir, numQueuesAndEvents, commQueues, 1, &rowOutEvents, outEvents, clOutputBuffers, NULL,
							localIntBuffer),
						"clfftEnqueueTransform for 3D-Z column failed");
#if defined(DEBUGGING)
					OPENCL_V_LOCKED(sLock, clEnqueueReadBuffer(*commQueues, clOutputBuffers[0], CL_TRUE, 0, buffSizeBytes, &output3.buf[0], 1, outEvents, NULL),
						"Reading the result buffer failed");
#endif
				}

				clReleaseEvent(rowOutEvents);
				return clfftReturnLocked(sLock, CLFFT_SUCCESS);
			}
		}

	return clfftReturnLocked(sLock,
		FFTActionEnqueue(fftPlan->action, plHandle, dir, numQueuesAndEvents, commQueues, numWaitEvents, waitEvents, outEvents, clInputBuffers, clOutputBuffers));
}
/* End copied source: src\library\transform.cpp */

/* Begin copied source: src\library\lifetime.cpp */

// clfft.lifetime.cpp : Functions that control the lifetime of the FFT library
// and their supporting functions
//

clfftStatus clfftInitSetupData(clfftSetupData *setupData)
{
	setupData->major = clfftVersionMajor;
	setupData->minor = clfftVersionMinor;
	setupData->patch = clfftVersionPatch;
	setupData->debugFlags = 0;

	return CLFFT_SUCCESS;
}

//	Allow AMD's implementation of FFT's to allocate internal resources
clfftStatus clfftSetup(const clfftSetupData *sData)
{
	//	Static data is not thread safe (to create), so we implement a lock to
	// protect instantiation for the first call 	Implemented outside of
	// FFTRepoGetInstance to minimize lock overhead; this is only necessary on
	// first creation
	// Enter the repository lock until the status helper releases it.
	lockRAII *sLock = FFTRepoLockRepo();
	lockRAIIEnter(sLock);

	//	First invocation of this function will allocate the FFTRepo singleton;
	// thereafter the object always exists
	FFTRepo *fftRepo = FFTRepoGetInstance();

	clfftInitRequestLibNoMemAlloc();

	// If the client has no setupData, we are done
	if (sData == NULL)
		return clfftReturnLocked(sLock, CLFFT_SUCCESS);

	//	Versioning checks commented out until necessary
	////	If the major version number between the client and library do not
	/// match, return mismatch
	// if( sData->major > clfftVersionMajor )
	//	return CLFFT_VERSION_MISMATCH;

	////	If the minor version number between the client and library do not
	/// match, return mismatch
	// if( sData->minor > clfftVersionMinor )
	//	return CLFFT_VERSION_MISMATCH;

	////	We ignore patch version number for version validation

	fftRepo->setupData = *sData;

	return clfftReturnLocked(sLock, CLFFT_SUCCESS);
}

//	Allow AMD's implementation of FFT's to destroy internal resources
clfftStatus clfftTeardown(void)
{
	FFTRepo *fftRepo = FFTRepoGetInstance();
	FFTRepoReleaseResources(fftRepo);

	return CLFFT_SUCCESS;
}

clfftStatus clfftGetVersion(cl_uint *major, cl_uint *minor, cl_uint *patch)
{
	*major = clfftVersionMajor;
	*minor = clfftVersionMinor;
	*patch = clfftVersionPatch;

	return CLFFT_SUCCESS;
}
/* End copied source: src\library\lifetime.cpp */

#if defined(__GNUC__) && !defined(_WIN32)
  #pragma GCC visibility pop
#endif
