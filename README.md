rouziclib
=========

This is my library of code that is common to my different projects (mostly [Spiral](http://photosounder.com/spiral/) and [SplineEQ](http://photosounder.com/splineeq/))

It includes some of the following:
- Fast lookup table-based fixed-point arithmetic approximations for sqrt, hypot, log2, exp2, pow, cos, atan2 (both noting angles in [turns](http://en.wikipedia.org/wiki/Turn_(geometry)), not radians), the Gaussian function ([e^-x²](http://www.wolframalpha.com/input/?i=e%5E-x%C2%B2)), the raised error function ([0.5+0.5erf(x)](http://www.wolframalpha.com/input/?i=0.5%2B0.5erf(x))) and a [windowed sinc](http://www.wolframalpha.com/input/?i=plot+sin%28pi*x%29%2F%28pi*x%29+*+%280.42+-+0.5*cos%282.*pi*%28x*0.25%2B0.5%29%29+%2B+0.08*cos%284.*pi*%28x*0.25%2B0.5%29%29%29%2C+x%3D-2+to+2) function. Some are implemented using linear interpolation, segmented quadratic polynomial approximation or simple lookup, which offers different levels of speed/precision/memory usage tradeoffs.
- Fast quadratic lookup table-based floating-point approximations for log2, exp2, pow and sqrt (fastsqrt() being slower than sqrt() depending on the machine)
- Functions to draw lines, points, circles, roundrects and gradients, all antialiased with Gaussian filtering
- Functions to convert from sRGB to linear RGB for loading images and to convert from linear RGB to sRGB with optional Gaussian dithering for displaying
- Geometric functions used for computing intersections between lines, shortest the distance of a point to a line or to limit a line to a bounding box
- Blending modes like additive, subtractive, multiplicative blending and alpha blending
- Blitting of a buffer onto another, like for displaying a sprite
- An original Hue-Saturation-Luminance colour space with a Luminance that is about perceptually correct (unlike the 1931 CIE XYZ colour space which gets the weights of each colour blatantly wrong) which is used for HSL to RGB conversion and for bringing out of gamut colours (such as colours that have components brighter than 1.0) to the most appropriate representable colour
- Vector font generation, vector text rendering and a built-in minimalistic vector font that is always available. A more complete typeface is available in the [vector_type](https://github.com/Photosounder/rouziclib/tree/master/vector_type) directory
- Vector procedural zoomable interface elements that for instance allow you to have a fully functional button just by calling a function with all the necessary information provided as parameters (without anything stored in memory) and simply getting the return value, with no need for storage for each instance of a control, no unique IDs or anything
- Built-in dependency-free loading of various image formats ([JPG, PNG, BMP, PSD](https://github.com/nothings/stb/blob/master/stb_image.h), TIFF), saving of images (32-bit TIFF, [PNG, JPG, BMP](https://github.com/nothings/stb/blob/master/stb_image_write.h)), loading of sounds (AIFF, WAVE, [FLAC](https://github.com/mackron/dr_libs/blob/master/dr_flac.h), [OGG](https://github.com/nothings/stb/blob/master/stb_vorbis.c)) and saving of sounds (AIFF, WAVE).
- Fast tiled mipmap generation, image rescaling based on [flat-top bilinear filtering](https://photosounder.com/michel_rouzic/#flattop), Gaussian blurring, YUV coding/decoding in C and OpenCL.
- Various utility functions and macros
- Code for working with [SDL](https://www.libsdl.org/), OpenGL, [OpenCL](https://www.khronos.org/opencl/), [clFFT](https://github.com/clMathLibraries/clFFT), [DevIL](http://openil.sourceforge.net/), [OpenCV](http://opencv.org/), [FFMPEG](https://www.ffmpeg.org/), [LibRAW](https://www.libraw.org/) and [MPFR](http://www.mpfr.org/).

All graphical functions operate on pixels in a linear colour space. Please do not use these functions directly in a gamma-compressed colour space, instead use an intermediary linear framebuffer which you can then convert to an sRGB framebuffer using the function `convert_lrgb_to_srgb`.

How to use it
-------------

Unusually for a library, rouziclib's code relies on macros that are defined inside your project's code. This means that rouziclib isn't entirely independently compiled. So the way to make this work is to create two files in your project, a header file which directly includes the main header, but not before you add the macros you can optionally define, and a code file which includes the aforementioned header file you just created and then includes the library's rouziclib.c. Here's how it looks:

rl.h
```C
#ifndef H_PRL
#define H_PRL
#ifdef __cplusplus
extern "C" {
#endif

// these are examples of optional macros that rouziclib will then use for your project
#define COL_FRGB	// this macro makes the internal format for colour be floating-point instead of fixed-point
#define RL_SDL		// this includes SDL-using code as well as the necessary SDL files
#define RL_OPENCL	// same for OpenCL

// this defines a wrapper for fprintf_rl, so you project can use a custom fprintf-type function that can for instance output to a file
#define fprintf_rl fprintf_wrapper
#include <stdio.h>
#include <stdarg.h>
extern void fprintf_wrapper (FILE *stream, const char* format, ...);

#include <rouziclib/rouziclib.h>

#ifdef __cplusplus
}
#endif
#endif
```

rl.c
```C
#include "rl.h"

// this creates that custom printing function that all calls to fprintf_rl in rouziclib will use

void fprintf_wrapper (FILE *stream, const char* format, ...)
{
	va_list args;

	va_start (args, format);

	vfprintf (stream, format, args);	// printf to original stream
	fflush (stream);

	va_end (args);
}

#include <rouziclib/rouziclib.c>
```

I realise that this is a bit unusual, but it's pretty simple and very handy. You can for instance include rouziclib in a simple command-line C program without having to worry about dependencies as none will be included, and in another project add dependencies as you need by adding the necessary macros, so without having the recompile anything separately (as you would have to were you to use two versions of a same library compiled with different dependencies) you can have in separate projects a rouziclib with no dependencies or a rouziclib that uses SDL, DevIL, OpenCV, OpenCL, clFFT, FFMPEG and LibRAW.

## Example project
Have a look at a minimal [picture viewer](https://github.com/Photosounder/rouziclib-picture-viewer) built around rouziclib, with explanations of its features, how it works and how to expand on it or create a similar program.
