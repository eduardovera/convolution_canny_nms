// Teste_OPENCV.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <math.h>
#include "nms.h"

using namespace cv;
using namespace std;

Mat1d image;
Mat1d output;
const int max_lowThreshold = 100;
const int ratio = 2;
const double RadtoDeg = 180/3.14;

int lowThreshold;

const Mat1d gaussian = (Mat1d(5, 5) <<
	2.0,  4.0,  5.0,  4.0, 2.0,
	4.0,  9.0, 12.0,  9.0, 4.0,
	5.0, 12.0, 15.0, 12.0, 5.0,
	4.0,  9.0, 12.0,  9.0, 4.0,
	2.0,  4.0,  5.0,  4.0, 2.0
) / 159;

const Mat1d kernelH = (Mat1d(3, 3) << 
	-1.0, 0.0, 1.0,
	-2.0, 0.0, 2.0,
	-1.0, 0.0, 1.0
);

const Mat1d kernelV = (Mat1d(3, 3) <<
	-1.0, -2.0, -1.0,
	 0.0,  0.0,  0.0,
	 1.0,  2.0,  1.0
	);

Mat1d convolutionSingleDirectional(const Mat1d &kernel, const Mat1d &image) {
	
	assert(kernel.rows == kernel.cols);
	int halfKernelSize = kernel.cols / 2;
	double sum = 0.0;

	Mat1d img(image.rows, image.cols, 0.0);

	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			for (int j = -halfKernelSize; j <= halfKernelSize; j++) {
				for (int i = -halfKernelSize; i <= halfKernelSize; i++) {
					if ((x + i) >= 0 && (x + i) < image.cols && (y + j) >= 0 && (y + j) < image.rows) {
						sum += kernel(i + halfKernelSize, j + halfKernelSize) * image(y + j, x + i);
					}
				}
			}
			img(y, x) = sum / 255;
			sum = 0;
		}
	}
	return img;
}

Mat1d convolutionMultiDirectional(const vector<Mat1d> &kernels, const Mat1d &image, Mat1d &angles) {
	
	Mat1d img(image.rows, image.cols, 0.0);
	
	vector<Mat1d> outputsFromConvolution;
	vector<double> angs;

	for (int k = 0; k < kernels.size(); k++) {
		outputsFromConvolution.push_back(convolutionSingleDirectional(kernels[k], image));
	}


	for (int j = 0; j < image.rows; j++) {
		for (int i = 0; i < image.cols; i++) {
			for (int k = 0; k < outputsFromConvolution.size(); k++) {
				img(j, i) += abs(outputsFromConvolution[k](j, i));
				angs.push_back(outputsFromConvolution[k](j, i));
			}
			img(j, i) = img(j, i) * 255;
			angles(j, i) = atan2(angs[1], angs[0]) * RadtoDeg;
			angs.clear();
		}
	}
	return img;
}

void setThreshold(int, void *) {
	Mat1d gaussian_output = convolutionSingleDirectional(gaussian, image);
	vector<Mat1d> kernels;
	kernels.push_back(kernelH);
	kernels.push_back(kernelV);

	Mat1d angles(image.rows, image.cols);
	Mat1d gradient = convolutionMultiDirectional(kernels, gaussian_output, angles);
	
	double max_gradient = DBL_MIN;
	double min_gradient = DBL_MAX;

	for (int j = 0; j < gradient.rows; j++) {
		for (int i = 0; i < gradient.cols; i++) {
			if (gradient(j, i) > max_gradient) {
				max_gradient = gradient(j, i);
			}
			if (gradient(j, i) < min_gradient) {
				min_gradient = gradient(j, i);
			}
		}
	}

	for (int j = 0; j < gradient.rows; j++) {
		for (int i = 0; i < gradient.cols; i++) {
			gradient(j, i) = (gradient(j, i) - min_gradient) / (max_gradient - min_gradient);
		}
	}

	Mat1d supressed_output(image.rows, image.cols, 0.0);
	output = Mat1d(image.rows, image.cols);

	nonMaximaSuppression(angles, gradient, supressed_output);
	hysteresis(supressed_output, gradient, lowThreshold, ratio * lowThreshold, output);
	imshow("Canny Edge Detection", output);

}

int _tmain(int argc, _TCHAR* argv[]) {
	image = imread("valve.png", CV_LOAD_IMAGE_GRAYSCALE);
	
	namedWindow("Canny Edge Detection", CV_WINDOW_AUTOSIZE);

	createTrackbar("Min Threshold:", "Canny Edge Detection", &lowThreshold, max_lowThreshold, setThreshold);
	setThreshold(0, 0);

	waitKey(0);
	return 0;
}

