#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include <fstream>
#include <omp.h>
#include "opencv2\opencv.hpp"
#include "opencv2\legacy\compat.hpp"

using namespace cv;
using namespace std;

int initCap();
int destCap();
int capture(int, CvMatr32f rotation = NULL, CvMatr32f translation = NULL);	//What function call from this modules

int detecting();						//VERTICES DETECTION
int detectOnce(CvMatr32f, CvMatr32f);	//VERTICES SINGLE DETECTION - created to be used by BARTOSZ KOSTANIAK
int initialCalibration();				//Filters video calibration
int imgProcess();						//Process images using filters
int filtersOnImages();					//Calibration on images
int takePictures();						//Taking pictures

#endif
