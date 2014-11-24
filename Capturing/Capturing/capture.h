#ifndef CAPTURE_H
#define CAPTURE_H

#include <iostream>
#include <fstream>
#include <omp.h>
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

using namespace cv;
using namespace std;

int initCap();
int destCap();
int capture(int arg = 1, void *data = NULL);	//What function call from this modules

int detecting();			//VERTICES DETECTION
int detectOnce(Vec3d*);		//VERTICES SINGLE DETECTION - created to be used by BARTOSZ KOSTANIAK
int initialCalibration();	//Filters video calibration
int imgProcess();			//Process images using filters
int filterImages();			//Calibration on images
int takePictures();			//Taking pictures

#endif