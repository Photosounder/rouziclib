//	gcc tablegen.c -o tablegen.exe -std=c99 -lm -lgmp -lmpfr -lwinmm -lcomdlg32 -lole32 -Wno-incompatible-pointer-types -O0 && ./tablegen.exe

#include "../rouziclib/rouziclib.h"
#include "../rouziclib/rouziclib.c"

void write_coefs_float(FILE *file, double *c, const int order, int is_end)
{
	int i;

	for (i=0; i<=order; i++)
		fprintf(file, "%.9g%sf%s", c[i], (c[i]==floor(c[i])) ? "." : "", (i==order && is_end) ? "};" : ", ");
}

void write_isqrt_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 10, prec = 24, abdp = lutsp;

	/*
		 2 -  7 = 75		
		 3 - 13 = 180		
		 4 - 14 = 600		
		 5 - 14 = 2k		
		 6 - 17 = 8k		
		 7 - 17 = 30k		 7 - 20 -  7 = 34k
		 8 - 18 = 100k		 8 - 23 -  8 = 130k
		 9 - 20 = 300k		 9 - 23 -  9 = 490k
		10 - 20 = 600k		10 - 24 - 10 = 1.9M
		11 - 21 = 1M		11 - 27 - 11 = 7.2M
		12 - 21 = 1.3M		12 - 27 - 12 = 24M
		13 - 22 = 2M		13 - 28 - 13 = 76M
		14 - 23 = 2.7M		
		15 - 23 = 4.2M		
		16 - 23 = 5.5M		
		17 - 24 = 8.4M		
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);

	file = fopen("../rouziclib/fixedpoint/fracsqrt_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const uint32_t fracsqrt[] = {");
	for (i=0; i<lutsize; i++)
		fprintf(file, "%d, ", (uint32_t) roundaway(sqrt((double) i / ls) * ratio));

	fprintf(file, "%d};", (uint32_t) ratio);

	fclose (file);
}

void write_fplog2_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 10, prec = 23, abdp = lutsp-2;

	/*
		 - with abdp
		 8 - 20 -  6 = 11.28e-6
		 9 - 21 -  7 = 2.846e-6  // relinearised error is 1 in 507k
		10 - 23 -  8 = 0.7695e-6 // relinearised error is 1 in 1.87M
		11 - 24 -  9 = 0.2204e-6
		12 - 25 - 10 = 0.07819e-6

		 - without abdp
		 7 - 17 = 43.94 e-6
		 8 - 18 = 12.60 e-6
		 9 - 18 = 4.584 e-6	// relinearised error is 1 in 315k
		10 - 19 = 2.211 e-6	// relinearised error is 1 in 653k
		11 - 20 = 1.194 e-6
		12 - 20 = 0.8222 e-6

		old
		 7 - 15 = 54.78 e-6
		 8 - 15 = 26.71 e-6
		 9 - 16 = 12.39 e-6	// relinearised error is 1 in 116k
		10 - 17 = 8.899 e-6	// relinearised error is 1 in 162k
		11 - 17 = 6.387 e-6
		12 - 18 = 4.548 e-6

		log2f   = 0.08599 e-6
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	int32_t buf[1UL<<lutsp], abd=0;

	for (i=0; i<lutsize+1; i++)
		buf[i] = (int32_t) roundaway(log((double) i / ls)/log(2.) * ratio);
	buf[0] = (int32_t) roundaway(log((double) 1. / ls)/log(2.) * ratio);	// exception for 0

	//abdp = log2_ffo(abd);

	file = fopen("../rouziclib/fixedpoint/fraclog2_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const int32_t fraclog2[] = {");
	for (i=0; i<lutsize; i++)
		fprintf(file, "%d, ", buf[i]);

	fprintf(file, "%d};", buf[lutsize]);

	fclose (file);
}

void write_fpexp2_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 10, prec = 27, abdp = 11;

	/*
		Test mesures error ratio between 2^30 and 2^32-1
		 9 - 25 - 10 = 4.8M
		 9 - 26 - 10 = 4.9M
		 9 - 27 - 10 = 4.97M
		 9 - 28 - 10 = 3.1M


		10 - 21 -  7 = 4.0M
		10 - 22 -  8 = 6.3M
		10 - 23 -  9 = 9.7M
		10 - 24 - 10 = 12.2M
		10 - 25 - 11 = 14.2M
		10 - 26 - 11 = 18.8M
		10 - 27 - 11 = 28.34M
		10 - 28 - 11 = 13.4M
		10 - 29 - 11 = 6.4M
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);

	file = fopen("../rouziclib/fixedpoint/fracexp2_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const int32_t fracexp2[] = {");
	for (i=0; i<lutsize; i++)
		fprintf(file, "%d, ", (int32_t) roundaway(exp2((double) i / ls) * ratio));

	fprintf(file, "%d};", (uint32_t) (ratio*2.));

	fclose (file);
}

void write_fpcos_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 12, prec = 25, abdp = 10;//lutsp-2;

	/*
		 -
		 5 - 16 = 209
		 6 - 17 = 831		 6 - 17 -  4 = 832
		 7 - 17 = 3.3k		 7 - 20 -  5 = 3.3k
		 8 - 18 = 13k		 8 - 22 -  6 = 13k
		 9 - 19 = 49k		 9 - 22 -  7 = 53k
		10 - 19 = 157k		10 - 23 -  8 = 211k
		11 - 19 = 367k		11 - 25 -  9 = 834k
		12 - 19 = 596k		12 - 25 - 10 = 3.2M
		13 - 20 = 824k		13 - 26 - 11 = 12M
		14 - 20 = 1.2M		14 - 27 - 12 = 44M

		Average errors:
		 9 - 19 - 0 = 119k
		 9 - 22 - 7 = 126k
		10 - 19 - 0 = 427k
		10 - 23 - 8 = 505k
	*/

	FILE *file;
	int32_t i, ip, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	char errlvl[256];

	file = fopen("../rouziclib/fixedpoint/fpcos_d1i.h", "w");
	
	fprintf(file, "#ifndef FPCOS_PREC\n#define FPCOS_PREC 10\n#endif");

	for (ip=0; ip < 3; ip++)
	{
		switch (ip)
		{
			case 0:	lutsp =  9; prec = 22; abdp =  7;	sprintf(errlvl, "max error: 1 in 53k. average error: 1 in 126k");	break;
			case 1:	lutsp = 10; prec = 23; abdp =  8;	sprintf(errlvl, "max error: 1 in 211k. average error: 1 in 505k");	break;
			case 2:	lutsp = 12; prec = 25; abdp = 10;	sprintf(errlvl, "max error: 1 in 3.2M. average error: 1 in 7.2M");	break;
		}

		lutsize=1UL<<lutsp;
		ls=lutsize;
		ratio = (double) (1UL<<prec);

		fprintf(file, "\n\n#if FPCOS_PREC==%d\n", lutsp);
		fprintf(file, "\tconst uint32_t lutsp = %d, prec = %d, abdp = %d;\t// %s\n", lutsp, prec, abdp, errlvl);
		fprintf(file, "\tstatic const int32_t fpcos_lut[] = {");
		for (i=0; i<lutsize; i++)
			fprintf(file, "%d, ", (int32_t) roundaway(cos((double) i * 2.*pi / ls) * ratio));
	
		fprintf(file, "%d};\n#endif", (int32_t) ratio);
	}

	fclose (file);
}

void write_fpwsinc_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 12, prec = 29, abdp = 11;

	/*	With a .16 fp input format
		 7 - 28 - 6 = 
		 8 - 27 - 6 = 24762 - 123670
		 8 - 29 - 7 = 24762 - 98476
		 9 - 29 - 8 = 

		10 - 27 - 4 = 394521 - 1978762
		10 - 28 - 5 = 394767 - 1978711 <-----
		10 - 29 - 6 = 394613 - 1978764
		10 - 30 - 7 = 394660 - 1978802

		11 - 27 - 3 = 1572588 - 7916614
		11 - 28 - 4 = 1577294 - 7915416
		11 - 29 - 5 = 1577412 - 7914879
		11 - 30 - 6 = 1578026 - 7915414 <-----

		12 - 29 - 4 = 6285284 - 31650569 <----- 1 in 6.3M
		12 - 30 - 5 = 6283828 - 31657632
	*/
	/*
	   	With a .24 fp input format

		11 - 28 - 10 = 1575121 - 6883246

	   	12 - 29 - 11 = 6253013 - 25213116
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double x, y, ls=lutsize, ratio = (double) (1UL<<prec);

	file = fopen("../rouziclib/fixedpoint/fpwsinc_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const int32_t fpwsinc_lut[] = {%d, ", (int32_t) ratio);
	for (i=1; i<lutsize; i++)
	{
		x = 2. * (double) i / ls;	// 2. is for the range, [-2.0 , 2.0]
		y = sin(pi*x) / (pi*x);
		y *= 0.42 - 0.5*cos(2.*pi*(x*0.25+0.5)) + 0.08*cos(4.*pi*(x*0.25+0.5));
		fprintf(file, "%d, ", (int32_t) roundaway(y * ratio));
	}

	fprintf(file, "%d};", 0);

	fclose (file);
}

void write_fpatan_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 10, prec = 24, abdp = lutsp;

	/*
		 -
		 1 -  2 = 14670 (4 degrees)
		 2 - 11 = 3877 (1.08 degrees)
		 3 - 11 = 997 (16.6 arcmin)
		 4 - 15 = 261 (4.35 arcmin)
		 5 - 15 = 64.91
		 6 - 16 = 17.26
		 7 - 18 = 5.144				 7 - 20 -  7 = 4.153
		 8 - 18 = 1.849				 8 - 23 -  8 = 1.063
		 9 - 19 = 0.8290			 9 - 23 -  9 = 0.2779
		10 - 19 = 0.5025 arcsec			10 - 24 - 10 = 0.07667 arcsec
		11 - 20 = 0.3482			11 - 25 - 11 = 0.02231
		12 - 20 = 0.2503			12 - 27 - 12 = 0.007482
		13 - 21 = 0.1744
		14 - 21 = 0.1256
		15 - 22 = 0.08717
		16 - 22 = 0.06277
		17 - 23 = 0.04366
		18 - 23 = 0.03133
		19 - 24 = 0.02192
		20 - 24 = 0.01578
		21 - 25 = 0.01096
		atan2f() = 0.01226 arcsec
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	double x, y, a, t;

	file = fopen("../rouziclib/fixedpoint/fpatan_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const int32_t fpatan_lut[] = {");
	for (i=0; i<lutsize; i++)
	{
		t = (double) i / ls;
		t = 0.5 - atan(1.-2.*t) * 2./pi;	// this function maps the index from the (y-x)/(y+x) form to the argument for cos and sin
		y = cos(t*0.5*pi);
		x = sin(t*0.5*pi);
		// range for atan2(y, x) is [0.5*pi , 0]
		// except we prefer to use turns rather than radians which would give us a range of [0.25 , 0]
		// but we'll make it [1 , 0] to not waste bits
		fprintf(file, "%d, ", (int32_t) roundaway(atan2(y, x) / (0.5*pi) * ratio));
		//fprintf(file, "%f, ", atan2(y, x) / (0.5*pi));
	}

	fprintf(file, "%d, 0};", (uint32_t) 0);		// the extra entry is accessed although its value doesn't matter

	fclose (file);
}

void write_fpgauss_d1i_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 8, prec = 29, abdp = 7;

	/*
		 7 - 28 - 6 = 4100
		 8 - 29 - 7 = 16k
		 9 - 29 - 8 = 66k

		10 - 29 - 7 = 262120
		10 - 30 - 8 = 262120
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	double x;

	file = fopen("../rouziclib/fixedpoint/fpgauss_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const int32_t fpgauss_lut[] = {");
	for (i=0; i<lutsize; i++)
	{
		x = 4. * (double) i / ls;
		fprintf(file, "%d, ", (int32_t) roundaway(exp(-x*x) * ratio));
	}

	x = 4. * (double) i / ls;
	fprintf(file, "%d};", (int32_t) roundaway(exp(-x*x) * ratio));

	fclose (file);
}

void write_fpgauss_d0_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 10, prec = 15;

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	double x;

	file = fopen("../rouziclib/fixedpoint/fpgauss_d0.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d;\n", lutsp, prec);
	fprintf(file, "static const uint16_t fpgauss_lut[] = {");
	for (i=0; i<lutsize; i++)
	{
		x = 4. * (double) i / ls;
		fprintf(file, "%d, ", (int32_t) roundaway(exp(-x*x) * ratio));
	}

	x = 4. * (double) i / ls;
	fprintf(file, "%d};", (int32_t) roundaway(exp(-x*x) * ratio));

	fclose (file);
}

void write_fperfr_d1i_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 8, prec = 24, abdp = 5;

	/*
		 -
		 8 - 22 - 3 = 16908
		 8 - 23 - 4 = 16925
		 8 - 24 - 5 = 16934	<---
		 8 - 25 - 6 = 16930
		 8 - 26 - 6 = 15527
		 8 - 27 - 6 = 13208
	*/

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	double x;

	file = fopen("../rouziclib/fixedpoint/fperfr_d1i.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d, abdp = %d;\n", lutsp, prec, abdp);
	fprintf(file, "static const int32_t fperfr_lut[] = {");
	for (i=0; i<lutsize; i++)
	{
		x = 8. * (double) i / ls - 4.;
		fprintf(file, "%d, ", (int32_t) roundaway((0.5*erf(x)+0.5) * ratio));
	}

	x = 8. * (double) i / ls - 4.;
	fprintf(file, "%d};", (int32_t) roundaway((0.5*erf(x)+0.5) * ratio));

	fclose (file);
}

void write_fperfr_d0_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 10, prec = 15;

	FILE *file;
	int32_t i, lutsize=1UL<<lutsp;
	double ls=lutsize, ratio = (double) (1UL<<prec);
	double x;

	file = fopen("../rouziclib/fixedpoint/fperfr_d0.h", "w");

	fprintf(file, "const uint32_t lutsp = %d, prec = %d;\n", lutsp, prec);
	fprintf(file, "static const uint16_t fperfr_lut[] = {");
	for (i=0; i<lutsize; i++)
	{
		x = 8. * (double) i / ls - 4.;
		fprintf(file, "%d, ", (int32_t) roundaway((0.5*erf(x)+0.5) * ratio));
	}

	x = 8. * (double) i / ls - 4.;
	fprintf(file, "%d};", (int32_t) roundaway((0.5*erf(x)+0.5) * ratio));

	fclose (file);
}

#define t(x) ((end-start)*(x)+start)
#define dft(x) df(t(x))
#define df(x) diff_f(f, x, c)
double diff_f(double (*f)(double), double x, double *c)
{
	return f(x) - (((((c[5]*x + c[4])*x + c[3])*x + c[2])*x + c[1])*x + c[0]);
}

double cb(double x) { return x*x*x; }

void find_polynomial_fit(double (*f)(double), double start, double end, double *c, int order)
{
	double	cep1=0.5-0.5*sqrt(0.5),
		cep2=0.5+0.5*sqrt(0.5);		// cubic error peaks
	double	qep1 = (3.-sqrt(5.))/8.,	// quartic error peaks
		qep2 = (5.-sqrt(5.))/8.,
		qep3 = (3.+sqrt(5.))/8.,
		qep4 = (5.+sqrt(5.))/8.;
	double	sep1 = (2.-sqrt(3.))/4.,	// sextic error peaks
		sep2 = (2.+sqrt(3.))/4.;
	
	memset(c, 0, 6*sizeof(double));

	if (order >= 0)
	{
		c[0] += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 1)
	{
		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[0] += 0.5 * (dft(1.0) + dft(0.0));
		c[0] += 0.5 * dft(0.5);
	}

	if (order >= 2)
	{
		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
		    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));

		c[0] += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 3)
	{
		c[3] += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
		     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[0] += 0.5 * (dft(1.0) + dft(0.0));
		c[0] += 0.5 * (dft(cep1) + dft(cep2));
	}

	if (order >= 4)
	{
		c[4] += -0.5 * (dft(cep1) - dft(0.5)) / sq(sq(t(cep1) - t(0.5)))
		     - 0.5 * (dft(cep2) - dft(0.5)) / sq(sq(t(cep2) - t(0.5)));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
		    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));

		c[3] += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
		     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += 0.5 * (dft(qep3) - dft(qep1)) / (t(qep3) - t(qep1))
		    + 0.5 * (dft(qep4) - dft(qep2)) / (t(qep4) - t(qep2));

		c[0] += 0.5 * (dft(1.0) + dft(0.0));
	}

	if (order >= 5)
	{
		c[5] += 0.72131471451 * (dft(qep3) - dft(qep2)) / pow(t(qep3) - t(qep2), 5.);	// why 0.72131471451?

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
		    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));

		c[3] += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
		     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		// error curve is now mostly quartic
		c[4] += -0.5 * (dft(cep1) - dft(0.5)) / sq(sq(t(cep1) - t(0.5)))
		     - 0.5 * (dft(cep2) - dft(0.5)) / sq(sq(t(cep2) - t(0.5)));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[2] += (dft(1.0) - dft(0.5)) / sq(t(1.0) - t(0.5));

		c[1] += 0.5 * (dft(0.75) - dft(0.0)) / (t(0.75) - t(0.0))
		    + 0.5 * (dft(1.0) - dft(0.25)) / (t(1.0) - t(0.25));

		c[3] += -0.25 * (dft(0.75) - dft(0.5)) / cb(t(0.75) - t(0.5))
		     - 0.25 * (dft(0.25) - dft(0.5)) / cb(t(0.25) - t(0.5));

		c[1] += 0.5 * (dft(qep3) - dft(qep1)) / (t(qep3) - t(qep1))
		    + 0.5 * (dft(qep4) - dft(qep2)) / (t(qep4) - t(qep2));

		c[2] += 0.5 * (dft(sep1) - dft(0.5)) / sq(t(sep1) - t(0.5))
		    + 0.5 * (dft(sep2) - dft(0.5)) / sq(t(sep2) - t(0.5));

		c[1] += (dft(1.0) - dft(0.0)) / (t(1.0) - t(0.0));

		c[0] += 0.5 * (dft(1.0) + dft(0.0));
		c[0] += 0.5 * dft(0.5);
	}
}

void find_quadratic_fit(double (*f)(double), double start, double end, double *c)
{
	double middle, p1, p2, height2, width2;

	memset(c, 0, 6*sizeof(double));

	c[1] = (f(end) - f(start)) / (end - start);

	middle = (end+start) * 0.5;
	height2 = (f(end)-c[1]*end) - (f(middle)-c[1]*middle);
	width2 = end - middle;
	c[2] = height2 / (width2*width2);

	p1 = 0.75*(end-start) + start;
	p2 = 0.25*(end-start) + start;
	c[1] = ((f(p1)-c[2]*p1*p1) - (f(start)-c[2]*start*start)) / (p1-start);
	c[1] += ((f(end)-c[2]*end*end) - (f(p2)-c[2]*p2*p2)) / (end-p2);
	c[1] *= 0.5;

	c[0] = ((f(end)-((c[2]*end + c[1])*end)) + (f(start)-((c[2]*start + c[1])*start))) * 0.5;
}

void find_linear_fit(double (*f)(double), double start, double end, double *c, int errmode)
{
	double middle;

	memset(c, 0, 6*sizeof(double));

	c[1] = (f(end) - f(start)) / (end - start);

	middle = (end+start) * 0.5;
	c[0] = ((f(start)-c[1]*start) + (f(middle)-c[1]*middle)) * 0.5;
}

double f_reciprocal(double x)
{
	return 1./x;
}

void write_quad_fit_fpdiv()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 7, prec = 27;

	/*
	   -
	   1 = 0.2525%	(1 in 396)
	   4 = 7.189 per million (1 in 139k)
	   5 = 937.4 per billion (1 in 1.07M)
	   6 = 133.2 per billion (1 in 7.5M)
	   7 = 32.42 per billion (1 in 30.8M)
	   8 = 23.75 per billion (1 in 42.1M)
	   9 = 21.99 per billion (1 in 45.5M)
	  10 = 23.14 per billion
	   */

	int32_t i, is, segcount=1UL<<lutsp;	// the precision is proportional to the cube of segcount, with a max error of 2.222e-7 / ~2.0 for 64
	double c[6], err, cstart=0.5, cend=1.0;
	double segstart, segratio=1./(double) segcount, segend, range, res;
	double aff0, aff1, aff0o;
	double range_mul;

	FILE *file;
	double ratio = (double) (1ULL<<prec);

	file = fopen("../rouziclib/fixedpoint/fpdiv_d2.h", "w");
	fprintf(file, "const uint32_t lutsp = %d, prec = %d;\n", lutsp, prec);
	fprintf(file, "static const int32_t fpdiv_lut[] = {");

	for (is=0; is<segcount; is++)
	{
		segstart = (cend-cstart) * (double) is / (double) segcount + cstart;
		segend = segstart+(cend-cstart)*segratio;

		find_quadratic_fit(f_reciprocal, segstart, segend, c);
		err = get_polynomial_error(f_reciprocal, segstart, segend, c, 2, DIVMODE);

		//printf("%.16f*x^2 + %.16f*x + %.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.7f*x^2 + %.7f*x + %.7f  e %g [%f , %f]\n", c[2], c[1], c[0], err, segstart, segend);

		fprintf(file, "%d, %d, %d%s", (int32_t) roundaway(c[0] * ratio), (int32_t) roundaway(c[1] * ratio), (int32_t) roundaway(c[2] * ratio), (is==segcount-1) ? "};" : ", ");
	}

	fclose (file);
}

double f_atan(double x)
{
	const double m2pi = -2./pi;
	double y;

	y = m2pi * 0.25 * atan((x+1.)/(x-1.));
	if (y < -0.125)		// phase unwrapping
		y += 0.5;

	return y;
}

void write_quad_fit_fpatan2()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 7, prec = 32;

	/*
	   - with fpdiv at 7		with fpdiv at 9
	   7 = 0.00894 arc seconds	0.00897
	   8 = 0.00373 arc seconds
	   9 = 0.00348 arc seconds	0.00258
	   */

	int32_t i, is, segcount=1UL<<lutsp;	// the precision is proportional to the cube of segcount
	double c[6], err, cstart=-1., cend=1.0;
	double segstart, segratio=1./(double) segcount, segend, range, res;
	double aff0, aff1, aff0o;
	double range_mul;
	double maxc0=0., maxc1=0., maxc2=0., maxerr=0.;

	FILE *file;
	double ratio = (double) (1ULL<<prec);

	file = fopen("../rouziclib/fixedpoint/fpatan2_d2.h", "w");
	fprintf(file, "const uint32_t lutsp = %d, prec = %d;\n", lutsp, prec);
	fprintf(file, "static const int32_t fpatan2_lut_off[] = {");

	for (is=-1; is<segcount; is++)
	{
		segstart = (cend-cstart) * (double) is / (double) segcount + cstart;
		segend = segstart+(cend-cstart)*segratio;

		find_quadratic_fit(f_atan, segstart, segend, c);
		err = get_polynomial_error(f_atan, segstart, segend, c, 2, NEGMODE);

		//printf("%.16f*x^2 + %.16f*x + %.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.14f*x^2 %+.14f*x %+.14f, x=%f to %f  e %.4g\n", c[2], c[1], c[0], segstart, segend, err);

		fprintf(file, "%d, %d, %d, ", (int32_t) roundaway(c[0] * ratio), (int32_t) roundaway(c[1] * ratio), (int32_t) roundaway(c[2] * ratio));

		if (fabs(c[0]) > fabs(maxc0)) maxc0 = c[0];
		if (fabs(c[1]) > fabs(maxc1)) maxc1 = c[1];
		if (fabs(c[2]) > fabs(maxc2)) maxc2 = c[2];
		if (err > maxerr) maxerr = err;
	}

	fprintf(file, "%d, %d, %d};", (int32_t) roundaway(c[0] * ratio), (int32_t) roundaway(c[1] * ratio), (int32_t) roundaway(c[2] * ratio));
	fprintf(file, "\nstatic const int32_t *fpatan2_lut = &fpatan2_lut_off[%d];", (segcount/2 + 1)*3);

	//printf("\nMax coefs are c[2]: %g, c[1]: %g, c[0]: %g. Max error: %g\n", maxc2, maxc1, maxc0, maxerr);

	fclose (file);
}

double f_cos(double x)
{
	return cos(2.*pi*x);
}

void write_poly_fit_fpcos()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 8, prec = 25;

	/*
	   -
	   7 = 691  per billion (1 in 1,447,170)
	   8 = 149  per billion (1 in 6,696,546)
	   9 = 96.9 per billion (1 in 10,318,347)
	  10 = 89.5 per billion (1 in 11,170,962)
	  11 = 90.1 per billion (1 in 11,096,075)
	  12 = 97.8 per billion (1 in 10,220,459)

	  http://www.wolframalpha.com/input/?i=plot+cos%282pix%29+-+%2815.2463631081088806*x%5E2+%2B+-14.6002950290567526*x+%2B+2.4624920137007509%29%2C+x%3D0.59375..0.625
	  http://www.wolframalpha.com/input/?i=plot+cos%282pix%29+-+%28-26.211103167836*x%5E3%2B63.163536086807*x%5E2-43.795022531079*x%2B8.390721499994%29%2C+x%3D0.59375..0.625
	  http://www.wolframalpha.com/input/?i=plot+cos%282pix%29+-+%28-50.174643762875*x%5E4%2B96.089591003503*x%5E3-48.614692509672*x%5E2%2B1.604929787255*x%2B1.476596611406%29%2C+x%3D0.59375..0.625
	  http://www.wolframalpha.com/input/?i=plot+x%3D0.59375..0.625%2C+cos%282pix%29+-+%28%2B51.7471898319371562*x%5E5-207.8418628056533635*x%5E4%2B288.2307222592789913*x%5E3-165.6814481548432809*x%5E2%2B37.2649118503205727*x-2.8680348385694590%29
	   */

	int32_t i, is, segcount=1UL<<lutsp;	// the precision is proportional to the cube of segcount
	double c[6], err, cstart=0., cend=1.0;
	double segstart, segratio=1./(double) segcount, segend, range, res;
	double maxc0=0., maxc1=0., maxc2=0., maxerr=0.;

	FILE *file;
	double ratio = (double) (1ULL<<prec);

//double n, errmin=1.;
//cend = pi/2.;

	file = fopen("../rouziclib/fixedpoint/fpcos_d2.h", "w");
	fprintf(file, "const uint32_t lutsp = %d, prec = %d;\n", lutsp, prec);
	fprintf(file, "static const int32_t fpcos_lut[] = {");

	for (is=0; is<segcount; is++)
	{
		segstart = (cend-cstart) * (double) is / (double) segcount + cstart;
		segend = segstart+(cend-cstart)*segratio;

		//find_quadratic_fit(f_cos, segstart, segend, &c[0], &c[1], &c[2]);
		/*if (segstart==0.59375)
		for (n=0.721314; n<=0.73; n+=0.00000000001)
		{*/
		find_polynomial_fit(f_cos, segstart, segend, c, 2);
		err = get_polynomial_error(f_cos, segstart, segend, c, 2, NEGMODE);

		/*if (err < errmin)
		{
			errmin = err;
			printf("n=%.16f, err %g\n", n, err);
		}
		}*/

		//if (segstart==0.59375)
		//printf("%+.12f*x^5%+.12f*x^4%+.12f*x^3%+.12f*x^2%+.12f*x%+.12f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[5], c[4], c[3], c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%+.16f*x^5%+.16f*x^4%+.16f*x^3%+.16f*x^2%+.16f*x%+.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[5], c[4], c[3], c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.16f*x^2 + %.16f*x + %.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.14f*x^2 %+.14f*x %+.14f, x=%f to %f  e %.4g\n", c[2], c[1], c[0], segstart, segend, err);

		fprintf(file, "%d, %d, %d%s", (int32_t) roundaway(c[0] * ratio), (int32_t) roundaway(c[1] * ratio), (int32_t) roundaway(c[2] * ratio), (is==segcount-1) ? "};" : ", ");

		if (fabs(c[0]) > fabs(maxc0)) maxc0 = c[0];
		if (fabs(c[1]) > fabs(maxc1)) maxc1 = c[1];
		if (fabs(c[2]) > fabs(maxc2)) maxc2 = c[2];
		if (err > maxerr) maxerr = err;
	}

	//printf("\nMax coefs are c[2]: %g, c[1]: %g, c[0]: %g. Max error: %g\n", maxc2, maxc1, maxc0, maxerr);

	fclose (file);
}

double f_log2(double x)
{
	return log(x)/log(2.);
}

void write_fastlog2_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 7;

	/*
	   - for doubles:
	   6 = 5.625e-08 (1.5 kB)
	   7 = 7.098e-09 (3 kB)
	   8 = 8.915e-10 (6 kB)
	  10 = 1.398e-11 (24 kB)
	  12 = 2.274e-13	// diminishing returns past this point
	  16 = 2.842e-14
	   */

	int32_t i, is, segcount=1UL<<lutsp;
	double c[6], err, cstart=1.0, cend=2.0;
	double segstart, segend, segratio=1./(double) segcount;

	FILE *file;

	file = fopen("../rouziclib/fastfloat/fastlog2.h", "w");
	//fprintf(file, "static const double fastlog2_lut[] = {");
	fprintf(file, "{");

	for (is=0; is<segcount; is++)
	{
		segstart = (cend-cstart) * (double) is / (double) segcount + cstart;
		segend = segstart+(cend-cstart)*segratio;

		find_quadratic_fit(f_log2, segstart, segend, c);
		err = get_polynomial_error(f_log2, segstart, segend, c, 2, DIVMODE);

		//printf("%.16f*x^2 + %.16f*x + %.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.7f*x^2 + %.7f*x + %.7f  e %g [%f , %f]\n", c[2], c[1], c[0], err, segstart, segend);

		fprintf(file, "%.16g, %.16g, %.16g%s", c[0], c[1], c[2], (is==segcount-1) ? "};\n" : ", ");
	}

	fprintf(file, "const uint32_t lutsp = %d;", lutsp);

	fclose (file);
}

/*	For fastpow()
	error is in e-8
lutsps		6		7		8	<--fastlog2
     5		20.85	2.25 kB	7.291	3.75 kB	5.592	6.75 kB
     6		16.24	3 kB	2.627	4.5 kB	0.9086	7.5 kB
     7		15.66	4.5 kB	2.044	6 kB	0.3288	9 kB
     8		15.59	7.5 kB	1.978	9 kB	0.2574	12 kB
^ fastexp2

Best combinations:
size		error	fastlog2	fastexp2	size*error	size^3*error
2.25 kB		20.85	6		5		46.9125		237.5 <---
3 kB		16.24	6		6		48.72		438.5
3.75 kB		7.291	7		5		27.34125	384.5
4.5 kB		2.627	7		6		11.8215		239.4 <---
6 kB		2.044	7		7		12.264		441.5
7.5 kB		0.9086	8		6		6.8145		383.3
9 kB		0.3288	8		7		2.9592		239.7 <---
12 kB		0.2474	8		8		2.9688		427.5
*/

double f_exp2(double x)
{
	return pow(2., x);
}

void write_fastexp2_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 6;

	/*
	   - for doubles:
	   5 = 5.35096e-08 (0.75 kB)
	   6 = 6.65255e-09 (1.5 kB)
	   7 = 8.293e-10 (3 kB)
	   8 = 1.03525e-10 (6 kB)
	   */

	int32_t i, is, segcount=1UL<<lutsp;
	double c[6], err, cstart=0.0, cend=1.0;
	double segstart, segend, segratio=1./(double) segcount;

	FILE *file;

	file = fopen("../rouziclib/fastfloat/fastexp2.h", "w");
	//fprintf(file, "static const double fastexp2_lut[] = {");
	fprintf(file, "{");

	for (is=0; is<segcount; is++)
	{
		segstart = (cend-cstart) * (double) is / (double) segcount + cstart;
		segend = segstart+(cend-cstart)*segratio;

		find_quadratic_fit(f_exp2, segstart, segend, c);
		err = get_polynomial_error(f_exp2, segstart, segend, c, 2, DIVMODE);

		//printf("%.16f*x^2 + %.16f*x + %.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.7f*x^2 + %.7f*x + %.7f  e %g [%f , %f]\n", c[2], c[1], c[0], err, segstart, segend);

		fprintf(file, "%.16g, %.16g, %.16g%s", c[0], c[1], c[2], (is==segcount-1) ? "};\n" : ", ");
	}

	fprintf(file, "const uint32_t lutsp = %d;", lutsp);

	fclose (file);
}

double f_sqrt(double x)
{
	return sqrt(x);
}

void write_fastsqrt_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t lutsp = 7;

	/*
	   - for doubles:
	   7 = 9.231e-10
	   */

	int32_t i, is, segcount=1UL<<lutsp;
	double c[6], err, cstart=1.0, cend=2.0;
	double segstart, segend, segratio=1./(double) segcount;

	FILE *file;

	file = fopen("../rouziclib/fastfloat/fastsqrt.h", "w");
	//fprintf(file, "static const double fastsqrt_lut[] = {");
	fprintf(file, "{");

	for (is=0; is<segcount; is++)
	{
		segstart = (cend-cstart) * (double) is / (double) segcount + cstart;
		segend = segstart+(cend-cstart)*segratio;

		find_quadratic_fit(f_sqrt, segstart, segend, c);
		err = get_polynomial_error(f_sqrt, segstart, segend, c, 2, DIVMODE);

		//printf("%.16f*x^2 + %.16f*x + %.16f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[2], c[1], c[0], err, 1./segratio, segstart, segend);
		//printf("%.7f*x^2 + %.7f*x + %.7f  e %g [%f , %f]\n", c[2], c[1], c[0], err, segstart, segend);

		fprintf(file, "%.16g, %.16g, %.16g%s", c[0], c[1], c[2], (is==segcount-1) ? "};\n" : ", ");
	}

	fprintf(file, "const uint32_t lutsp = %d;", lutsp);

	fclose (file);
}

double f_gauss(double x)
{
	return exp(-x*x);
}

double f_erfr(double x)
{
	return 0.5*erf(x) + 0.5;
}

void write_poly_fit_generic_offset_noshift(const char *name, double (*f)(double), int32_t lutsp, double xoffset, double cstart, double cend, int order)
{
	double offset = (double) (1UL << 23-lutsp);		// 23 (mantissa) - 8 (fractional bits after masking) = 15 (offset)
	int32_t i, is, segcount = (cend-cstart) * (double) (1UL<<lutsp) + 0.5;
	double c[6], err;
	double segstart, segratio=1./(double) segcount, segend, rangeoffset, res;
	double maxpos, maxerr=0.;
	char path[1024];
	FILE *file;

	if ((order+1 & order)==0)		// if the number of coefficients is 2^n-1
	{
		offset /= (double) (order+1);	// multiply the offset to avoid having to shift the index
	}

	sprintf(path, "../rouziclib/fastfloat/%s_d%d.h", name, order);
	file = fopen(path, "w");
	fprintf(file, "const float offset = %.0f.f, limit = %g%sf;\n", offset+xoffset, cend, (cend==floor(cend)) ? "." : "");
	fprintf(file, "static const float %s_lut[] = {", name);

	rangeoffset = 0.5 / (double) (order+1);		// range offset needed to make up for floating point rounding after adding the offset, only verified for orders 0 and 1

	for (is=0; is<segcount+1; is++)
	{
		//segstart = ((double) is - rangeoffset) / (double) (1UL<<lutsp);
		segstart = (((double) is - rangeoffset) / (double) segcount) * (cend-cstart) + cstart;
		segend = segstart+(cend-cstart)*segratio;

		if (segstart < cstart)
			segstart = cstart;
		if (segend > cend)
			segend = cend;

		find_polynomial_fit(f, segstart, segend, c, order);
		err = get_polynomial_error(f, segstart, segend, c, 2, NEGMODE);

		//printf("%+.8f\n%g max error (segratio 1/%.0f) [%.7f , %.7f]\n", c[0], err, 1./segratio, segstart, segend);

		write_coefs_float(file, c, order, (is==segcount));

		if (err > maxerr) { maxerr = err;	maxpos = segstart; }
	}

	printf("\n%s:\n\tLUT size: %.2g kB\n\tMax error: %g at %g\n", path, (double) ((segcount+1)*(order+1)) * sizeof(float) / 1024., maxerr, maxpos);

	fclose (file);
}

void write_poly_fit_fastgauss(int32_t lutsp, double cend, int order)
{
	/* Order - lutsp:
	 * 0 - 8 = 0.001675
	 * 1 - 6 = 3.052e-5
	 */

	write_poly_fit_generic_offset_noshift("fastgauss", f_gauss, lutsp, 0., 0., cend, order);
}

void write_poly_fit_fasterfr(int32_t lutsp, double cend, int order)
{
	/* Order - lutsp:
	 * 0 - 7 = 0.002204
	 * 1 - 5 = 2.959e-5
	 */

	write_poly_fit_generic_offset_noshift("fasterfr", f_erfr, lutsp, cend, -cend, cend, order);
}

double f_lsrgb(double x)
{
	//return 1.055 * pow(x, 1.0/2.4) - 0.055;
	return lsrgb(x);
}

void write_fastlsrgb_lut()
{
	// The two important values to define, the number of bits in each entry and the size of the LUT in powers of two
	int32_t ish = 21;

	int32_t i, is;
	double c[6], err;
	double segstart, segend=0.;
	float offset = 0.0031308;

	FILE *file;

	file = fopen("../rouziclib/fastfloat/fastlsrgb.h", "w");
	fprintf(file, "{");

	for (is=0; segend < 1.; is++)
	{
		segstart = u32_as_float( float_as_u32(offset) + (is << ish) ) - offset;
		segend = u32_as_float( float_as_u32(offset) + ((is+1) << ish) - 1 ) - offset;

		//segstart = MAXN(segstart, 0.0031308);	// no fit needed below 0.0031308
		segend = MINN(segend, 1.);

		find_quadratic_fit(f_lsrgb, segstart, segend, c);
		if (segend <= 0.0031308)
		{
			c[0] = c[2] = 0.;
			c[1] = 12.92;
		}
		err = get_polynomial_error(f_lsrgb, segstart, segend, c, 2, NEGMODE);

		if (segstart > segend)
		{
			memset(c, 0, sizeof(c));
			err = 0.;
		}

		printf("[%02d] e %5.1fe-6 [%f , %f] span 1/%g\n", is, err*1e6, segstart, segend, MAXN(0., 1./(segend-segstart)));

		fprintf(file, "%.16g, %.16g, %.16g%s", c[0], c[1], c[2], (segend >= 1.) ? "};\n" : ", ");
	}

	fprintf(file, "const int order = 2, ish = %d;\n", ish);

	fclose (file);
}

double f_exp(double x)
{
	return exp(-x);
}

void write_fastexp_limited_lut()
{
	int ish = 49, order = 2;
	double offset=2., end=12.;

	int i;
	int64_t is;
	double c[6], err;
	double segstart, segend=0.;

	FILE *file;

	file = fopen("../rouziclib/fastfloat/fastexp_limited.h", "w");
	fprintf(file, "{");

	for (is=0; segend < end; is++)
	{
		segstart = u64_as_double( double_as_u64(offset) + (is << ish) ) - offset;
		segend = u64_as_double( double_as_u64(offset) + ((is+1) << ish) - 1 ) - offset;

		polynomial_fit_on_function_by_dct(f_exp, segstart, segend, c, order);
		err = get_polynomial_error(f_exp, segstart, segend, c, order, NEGMODE);
		err = reduce_digits(f_exp, segstart, segend, c, order, NEGMODE, 1.0001, 20.);

		printf("[%02d] e 1/%5.0f [%f , %f] span 1/%g\n", is, 1./err, segstart, segend, MAXN(0., 1./(segend-segstart)));

		fprintf(file, "%.9g, %.9g, %.9g%s", c[0], c[1], c[2], (segend >= end) ? "};\n" : ",\n");
	}

	fprintf(file, "const int order = %d, ish = %d;\nconst double offset = %g, end = %g;\n", order, ish, offset, segend);

	fclose (file);
}

int main(int argc, char *argv[])
{
	/*write_isqrt_lut();
	write_fplog2_lut();
	write_fpexp2_lut();
	write_fpcos_lut();
	write_fpwsinc_lut();
	//write_fpatan_lut();
	write_fpgauss_d1i_lut();
	write_fpgauss_d0_lut();
	write_fperfr_d1i_lut();
	write_fperfr_d0_lut();

	write_quad_fit_fpdiv();
	write_quad_fit_fpatan2();
	write_poly_fit_fpcos();

	write_fastlog2_lut();
	write_fastexp2_lut();
	write_fastsqrt_lut();

	write_poly_fit_fastgauss(8, 3., 0);
	write_poly_fit_fastgauss(6, 4., 1);

	write_poly_fit_fasterfr(7, 3., 0);
	write_poly_fit_fasterfr(5, 4., 1);

	write_fastlsrgb_lut();*/
	write_fastexp_limited_lut();

	//comp_cos(16.*pi);

	return 0;
}
