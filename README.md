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
- Various utility functions and macros

All graphical functions operate on pixels in a linear colour space. Please do not use these functions directly in a gamma-compressed colour space, instead use an intermediary linear framebuffer which you can then convert to an sRGB framebuffer using the function `convert_lrgb_to_srgb`.

In the near future more code will be ported from my projects to rouziclib, in particular functions for displaying bitmap variable-width fonts, GUI elements, FFT and code that relies on specific APIs.

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

#define LBD 13	// this is an optional macro that rouziclib will then use for your project instead of the default of LBD==15

#include <rouziclib/rouziclib.h>

#ifdef __cplusplus
}
#endif
#endif
```

rl.c
```C
#include "rl.h"

#include <rouziclib/rouziclib.c>
```

I realise that this is kind of weird, but it's pretty simple and handy.
