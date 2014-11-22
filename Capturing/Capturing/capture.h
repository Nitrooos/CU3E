#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int calibrate();
int imgProcess();
int capturing(VideoCapture*);

#endif