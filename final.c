#include "FreeImage.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//helper functions

void brightness(FIBITMAP * img, int level); //level is -100 to 100
void contrast(FIBITMAP * img, int level); //level is -100 to 100
void saturation(FIBITMAP * img, int level, unsigned height, unsigned width, unsigned pitch); //level is 
void addNoise(FIBITMAP * img, unsigned height, unsigned width, unsigned pitch);
void RGBtoHSV(float r, float g, float b, float *h, float *s, float *v);
void HSVtoRGB(float h, float s, float v, float *r, float *g, float *b);

float MIN(float a, float b, float c);
float MAX(float a, float b, float c);
float floatmod(float a, float b);

#define NOISE_AMOUNT 1

int main (int argc, char * argv[]) {
	char * fileName = argv[1];
	FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(fileName, 0);
	FIBITMAP * img = FreeImage_Load(formato, fileName, 0);
	if(!img) {
		puts("bruh what");
		exit(1);
	}
	
	unsigned width = FreeImage_GetWidth(img);
	unsigned height = FreeImage_GetHeight(img);
	unsigned pitch = FreeImage_GetPitch(img);

	for(int i = 0; i < NOISE_AMOUNT; ++i) {
		addNoise(img, height, width, pitch);
	}
	contrast(img, 95);	
	brightness(img, 80);
	saturation(img, 80, height, width, pitch);	

	puts("Image nuked! File found at ./output.jpg");
	FreeImage_Save(FIF_JPEG, img, "output.jpg", 0);

	return 0;
}

float floatmod(float a, float b) {
	return (a - b * floor(a/b));
}

float MAX(float a, float b, float c) {
	float out = a;
	if(b > out)
		out = b;
	if(c > out)
		out = c;
	return out;
}

float MIN(float a, float b, float c) {
	float out = a;
	if(b < out)
		out = b;
	if(c < out)
		out = c;

	return out;
}

void HSVtoRGB(float h, float s, float v, float *r, float *g, float *b) {
	if( s > 1.0)
		s = 1.0;

	if( s < 0.002) {
		*r = v*255;
		*g = v*255;
		*b = v*255;
	} else {
		float C = v * s;
		float X = C * (1-fabs(floatmod(h/60.0, 2) - 1));
		float m = v - C;

		float var_r, var_g, var_b;


		if      ( h >=0 && h < 60 )    { var_r = C     ; var_g = X     ; var_b = 0     ;}
   		else if ( h >= 60 && h < 120)  { var_r = X     ; var_g = C     ; var_b = 0     ;}
   		else if ( h >= 120 && h < 180) { var_r = 0     ; var_g = C     ; var_b = X     ;}
   		else if ( h >= 180 && h < 240) { var_r = 0     ; var_g = X     ; var_b = C     ;}
 		else if ( h >= 240 && h < 300) { var_r = X     ; var_g = 0     ; var_b = C     ;}
		else                           { var_r = C     ; var_g = 0     ; var_b = X     ;}	
		
		*r = (var_r+m) * 255;
		*g = (var_g+m) * 255;
		*b = (var_b+m) * 255;
	}
}	

void RGBtoHSV(float r, float g, float b, float *h, float *s, float *v) {
	float min, max, delta;
	r=r/255.0;
	g=g/255.0;
	b=b/255.0;


	min = MIN(r, g, b);
	max = MAX(r, g, b);
	*v = max;
	delta = max-min;

	


	if(max != 0)
		*s = delta/max;
	else {
		*s = 0;
		*h = 0;
		return;
	}


	if (r == max) {
		*h = (g-b) / delta;
	} else if(g == max) {
		*h = 2 + (b-r) / delta;
	} else {
		*h = 4 + (r-g) / delta;
	}
	*h *= 60; //degrees
	if(*h < 0)
		*h += 360;

	if (delta == 0) {
		*h = 0;
	}
}

void brightness(FIBITMAP * img, int level) {
	FreeImage_AdjustBrightness(img, (double) level);
}

void contrast(FIBITMAP * img, int level) {
	FreeImage_AdjustContrast(img, (double) level);
}

void saturation(FIBITMAP * img, int level, unsigned height, unsigned width, unsigned pitch) {
	float h, s, v, r, g, b;
	BYTE *bits = (BYTE*)FreeImage_GetBits(img);
	for (unsigned y = 0; y < height; ++y) {
		BYTE *pixel = (BYTE*)bits;
		for (unsigned x = 0; x < width; ++x) {
			r = pixel[FI_RGBA_RED];
			g = pixel[FI_RGBA_GREEN];
			b = pixel[FI_RGBA_BLUE];
			RGBtoHSV(r, g, b, &h, &s, &v);
			s *= (double)level/100.0 + 1;
			HSVtoRGB(h, s, v, &r, &g, &b);
			pixel[FI_RGBA_RED] = (int)r;
			pixel[FI_RGBA_GREEN] = (int)g;
			pixel[FI_RGBA_BLUE] = (int)b;
			pixel += 3;
		}
		bits += pitch;
	}
}

void addNoise(FIBITMAP * img, unsigned height, unsigned width, unsigned pitch) {
	BYTE * bits = (BYTE*)FreeImage_GetBits(img);
	float noise;
	int r,g,b;
	for (unsigned y = 0; y < height; ++y) {
		BYTE *pixel = (BYTE*)bits;
		for (unsigned x = 0; x < width; ++x) {
			noise = (float)rand()/(float)RAND_MAX/7.0;
			r = (int)(((float)pixel[FI_RGBA_RED] / 255.0 + noise)*255);
			g = (int)(((float)pixel[FI_RGBA_GREEN] / 255.0 + noise)*255);
			b = (int)(((float)pixel[FI_RGBA_BLUE] / 255.0 + noise)*255);
			pixel[FI_RGBA_RED] = r>255? 255: r;
			pixel[FI_RGBA_GREEN] = g>255? 255: g;
			pixel[FI_RGBA_BLUE] = b>255? 255: b;
			pixel += 3;
		}
		bits += pitch;
	}	
}	
