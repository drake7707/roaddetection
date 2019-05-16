#pragma once

#include "opencv2/opencv.hpp"

using namespace cv;


void xyz2rgb(double xp, double yp, double zp, double& r, double& g, double& b) {
	double x = xp / 100.0;
	double y = yp / 100.0;
	double z = zp / 100.0;

	r = (x * 3.2406) + (y * -1.5372) + (z * -0.4986);
	g = (x * -0.9689) + (y * 1.8758) + (z * 0.0415);
	b = (x * 0.0557) + (y * -0.2040) + (z * 1.0570);

	// assume sRGB
	r = r > 0.0031308
		? ((1.055 * pow(r, 1.0 / 2.4)) - 0.055)
		: r *= 12.92;

	g = g > 0.0031308
		? ((1.055 * pow(g, 1.0 / 2.4)) - 0.055)
		: g *= 12.92;

	b = b > 0.0031308
		? ((1.055 * pow(b, 1.0 / 2.4)) - 0.055)
		: b *= 12.92;

	/*if (r < 0) r = 0;
	if (r > 1) r = 1;
	if (g < 0) g = 0;
	if (g > 1) g = 1;
	if (b < 0) b = 0;
	if (b > 1) b = 1;
*/
	r *= 255;
	g *= 255;
	b *= 255;
}

void lab2xyz(double l, double a, double b, double& x, double& y, double& z) {
	double y2;

	if (l <= 8) {
		y = (l * 100) / 903.3;
		y2 = (7.787 * (y / 100)) + (16 / 116.0);
	}
	else {
		y = 100 * pow((l + 16) / 116, 3);
		y2 = pow(y / 100, 1 / 3);
	}

	x = x / 95.047 <= 0.008856
		? x = (95.047 * ((a / 500) + y2 - (16 / 116.0))) / 7.787
		: 95.047 *  pow((a / 500) + y2, 3);
	z = z / 108.883 <= 0.008859
		? z = (108.883 * (y2 - (b / 200) - (16 / 116.0))) / 7.787
		: 108.883 * pow(y2 - (b / 200), 3);
}


void rgb2xyz(double rp, double gp, double bp, double& x, double& y, double& z) {

	double r = rp / 255;
	double g = gp / 255;
	double b = bp / 255;

	// assume sRGB
	r = r > 0.04045 ? pow(((r + 0.055) / 1.055), 2.4) : (r / 12.92);
	g = g > 0.04045 ? pow(((g + 0.055) / 1.055), 2.4) : (g / 12.92);
	b = b > 0.04045 ? pow(((b + 0.055) / 1.055), 2.4) : (b / 12.92);

	x = (r * 0.4124) + (g * 0.3576) + (b * 0.1805);
	y = (r * 0.2126) + (g * 0.7152) + (b * 0.0722);
	z = (r * 0.0193) + (g * 0.1192) + (b * 0.9505);

	x *= 100;
	y *= 100;
	z *= 100;

}

void lab2rgb(double lp, double ap, double bp, double& r, double& g, double& b) {
	double x;
	double y;
	double z;
	lab2xyz(lp, ap, bp, x, y, z);
	xyz2rgb(x, y, z, r, g, b);
}

void rgb2lab(double rp, double gp, double bp, double& l, double& a, double& b) {
	double x;
	double y;
	double z;
	rgb2xyz(rp, gp, bp,x,y,z);


	x /= 95.047;
	y /= 100;
	z /= 108.883;

	x = x > 0.008856 ? pow(x, 1 / 3.0) : (7.787 * x) + (16 / 116.0);
	y = y > 0.008856 ? pow(y, 1 / 3.0) : (7.787 * y) + (16 / 116.0);
	z = z > 0.008856 ? pow(z, 1 / 3.0) : (7.787 * z) + (16 / 116.0);

	l = (116 * y) - 16;
	a = 500 * (x - y);
	b = 200 * (y - z);
}


namespace Algorithms {

	void removeShadows(Mat& input, Mat& output) {
		// implementation according to https://www.osapublishing.org/oe/fulltext.cfm?uri=oe-23-3-2220&id=310863

		double beta1 = 2.557;
		double beta2 = 1.889;
		double beta3 = 1.682;
		double eps = 0.15;
		double kappa = 0.02;

		Vec3f u0 = Vec3f(beta1*beta2 - 1, 1 + beta1, 1 + beta2);
		u0 /= norm(u0);

		Vec3f t(0, 0, 0);

		int nrPixelsInS = 0;
		for (int j = 0; j < input.rows; j++)
		{
			for (int i = 0; i < input.cols; i++)
			{
				Vec3b val = input.at<Vec3b>(Point(i, j));

				Vec3f u(log(val[2] + 14), log(val[1] + 14), log(val[0] + 14));
				double norm_u = norm(u);

				double normDiff = norm(u / norm_u - u0);
				if (normDiff < eps) {
					t[0] += u0[0] - u[0] / norm_u;
					t[1] += u0[1] - u[1] / norm_u;
					t[2] += u0[2] - u[2] / norm_u;
					nrPixelsInS++;
				}
			}
		}
		t /= nrPixelsInS;

		for (int j = 0; j < input.rows; j++)
		{
			for (int i = 0; i < input.cols; i++)
			{
				Vec3b val = input.at<Vec3b>(Point(i, j));

				Vec3f u(log(val[0] + 14), log(val[1] + 14), log(val[2] + 14));
				double norm_u = norm(u);

				double normDiff = norm(u / norm_u - u0);
				if (normDiff < eps) {

					double alpha = -u.ddot(u0);
					Vec3f up = u + alpha * u0;

					Vec3f smoothing = up / norm(up) + 1 / (kappa * normDiff * normDiff * normDiff + 1) * t;
					Vec3f uc = norm(up) * smoothing;

					Vec3f up_rgb(exp(up[0]), exp(up[1]), exp(up[2]));
					Vec3f uc_rgb(exp(uc[0]), exp(uc[1]), exp(uc[2]) );

					Vec3d up_lab;
					rgb2lab(up_rgb[0], up_rgb[1], up_rgb[2], up_lab[0], up_lab[1], up_lab[2]);

					Vec3d uc_lab;
					rgb2lab(uc_rgb[0], uc_rgb[1], uc_rgb[2], uc_lab[0], uc_lab[1], uc_lab[2]);

					Vec3f mixed(up_lab[0], up_lab[1], up_lab[2]);
					
					Vec3d mixed_rgb;
					lab2rgb(mixed[0], mixed[1], mixed[2], mixed_rgb[0], mixed_rgb[1], mixed_rgb[2]);

					Vec3b outputVal(mixed_rgb[2], mixed_rgb[1], mixed_rgb[0]);
					output.at<Vec3b>(Point(i, j)) = outputVal;
				}
			}
		}
	}
}