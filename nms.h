#include <iostream>

using namespace std;
using namespace cv;

vector<Point2i> getNeighborsToCheck(double angle, int i, int j) {
	vector<Point2i> v;

	if ((angle > 0 && angle <= 22.5) || angle > 157.5 && angle <= 180) {
		// round to 0
		v.push_back(Point2i(i, j - 1));
		v.push_back(Point2i(i, j + 1));
		return v;
	} else if (angle > 22.5 && angle <= 67.5) {
		//round to 45
		v.push_back(Point2i(i - 1, j - 1));
		v.push_back(Point2i(i + 1, j + 1));
		return v;
	} else if (angle > 67.5 && angle <= 112.5) {
		//round to 90
		v.push_back(Point2i(i - 1, j));
		v.push_back(Point2i(i + 1, j));
		return v;
	} else if (angle > 112.5 && angle <= 157.5) {
		//round to 135
		v.push_back(Point2i(i - 1, j + 1));
		v.push_back(Point2i(i + 1, j - 1));
		return v;
	}
	return v;
}

void nonMaximaSuppression(const Mat1d &srcAngles, const Mat1d &srcImage, Mat1d &dst) {
	vector<Point2i> neighborsToCheck;
	for (int j = 0; j < srcAngles.rows; j++) {
		for (int i = 0; i < srcAngles.cols; i++) {
			neighborsToCheck = getNeighborsToCheck(srcAngles(j, i), i, j);
			for (int k = 0; k < neighborsToCheck.size(); k++) {
				if (neighborsToCheck[k].x >= 0 && neighborsToCheck[k].x < srcAngles.cols && neighborsToCheck[k].y >= 0 && neighborsToCheck[k].y < srcAngles.rows) {
					if (srcImage(j, i) < srcImage(neighborsToCheck[k].y, neighborsToCheck[k].x)) {
						dst(j, i) = 0;
					} else if (srcImage(j, i)){
						dst(j, i) = 1;
					}
				}
			}
		}
	}
}

void hysteresis(const Mat1d &supressed_image, const Mat1d &gradient, int lower, int upper, Mat1d &dst) {
	int gradient_color;
	for (int j = 0; j < gradient.rows; j++) {
		for (int i = 0; i < gradient.cols; i++) {
			gradient_color = 255 * gradient(j, i);
			if (gradient_color != 0) {
				if (gradient_color > upper) {
					dst(j, i) = 1;
				} else {
					if (gradient_color < lower) {
						dst(j, i) = 0;
					} else {
						for (int y = -1; y <= 1; y++) {
							for (int x = -1; x <= 1; x++) {
								if ((x + i) >= 0 && (x + i) < gradient.cols && (y + j) >= 0 && (y + j) < gradient.rows) {
									if (gradient(j + y, i + x) * 255 > upper) {
										dst(j, i) = 1;
									} else {
										dst(j, i) = 0;
									}
								}
							}
						}
					}
				}
			} else {
				dst(j, i) = 0;
			}
		}
	}
}