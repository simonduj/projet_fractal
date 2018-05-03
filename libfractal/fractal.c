#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
	struct fractal *frac;
	frac = (struct fractal *)malloc(sizeof(struct fractal));

	if(frac == NULL)
	{
		return NULL;
	}
	frac->name = name;
	frac->width = width;
	frac->height = height;
	frac->a = a;
	frac->b = b; 

	frac->values = (int **)malloc(width * sizeof(int*));
	int * tmp;
	tmp = (int *)malloc(width*height*sizeof(int));
	
	if(frac->values == NULL)
	{
		return NULL;
	}
	
	for(int i = 0; i < width ; i++)
	{
		frac->values[i] = tmp + (i * height);
	}
    	return frac;
}

void fractal_free(struct fractal *f)
{
    free(f);
}

const char *fractal_get_name(const struct fractal *f)
{
	char const *name = f->name;
    	return name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    	int result = fractal_compute_value(f, x, y);
	return result;
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    	f->values[x][y] = val;
}

int fractal_get_width(const struct fractal *f)
{
   	int width = f->width;
	return width;
}

int fractal_get_height(const struct fractal *f)
{
	int height = f->height;
	return height;
}

double fractal_get_a(const struct fractal *f)
{
    	double a = f->a;
	return a;
}

double fractal_get_b(const struct fractal *f)
{
    	double b = f->b;
	return b;
}
