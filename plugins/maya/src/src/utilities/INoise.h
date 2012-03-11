#pragma once

#include "../definitions.h"

#include <math.h>

typedef struct {
	double x;
	double y;
	double z;
} vector;

class INoise{
	public:
		static  vector	noise4d_v( double x, double y, double z, double t);
//		static  double	noise4d_f( double x, double y, double z, double t);
	private:
		static  double	atPointAndTime( double x, double y, double z, double t, int *p);
		static	double	fade(double t);
		static	double	lerp(double t, double a, double b);
		static	double	grad(int hash, double x, double y, double z, double t);
		static	int		p_x[512];
		static	int		p_y[512];
		static	int		p_z[512];
};
