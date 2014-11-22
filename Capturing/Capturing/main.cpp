#include "capture.h"

int main(int argc, char** argv){
	//return calibrate();
	//return imgProcess();

	VideoCapture cap(0);	//capture the video from Cam 0
	if (!cap.isOpened()){
		cout << "ERROR: Cannot open Cam 0!" << endl;
		return -1;
	}
	return capturing(&cap);
}

//Video Capturing
/*
#include <iostream>
#include <fstream>
#include <string>
#include <omp.h>
#include "opencv2/highgui/highgui.hpp"

#define IMG_DIR "output/"

using namespace cv;
using namespace std;

int main(int argc, char* argv[]){
	unsigned i=5;
	bool stop = false;
	double dWidth;
	double dHeight;
	bool bSuccess;
	string name;
	Mat frame;
	int key;

	VideoCapture cam(0);		// open the video
	if (!cam.isOpened()){		// if not success, exit program
		cout << "ERROR: Cannot open Cam 0!" << endl;
		return -1;
	}
	dWidth = cam.get(CV_CAP_PROP_FRAME_WIDTH);		//get the width of frames
	dHeight = cam.get(CV_CAP_PROP_FRAME_HEIGHT);	//get the height of frames
	cout << "Frame size of Cam 0: "  << dWidth << " x " << dHeight << endl;
	namedWindow("Cam 0", CV_WINDOW_AUTOSIZE);		//create a window

	i = 0;
	//omp_set_num_threads(2);
	while (!stop){
		//#pragma omp parallel for schedule(static, CAM_NUM)
		bSuccess = cam.read(frame);	//read a new frame
		if (bSuccess){
			imshow("Cam 0", frame);	//show the frame

			//name = "image_" + to_string(i) +".jpg";
			//imwrite(IMG_DIR + name, frame);
			//cout << name << endl;

			key = waitKey(20);
			if (key == 27){			//wait for ESC key press for 20ms
				cout << endl << "ESC key is pressed by user." << endl;
				stop = true;
			}
			
			if (key == 99){
				i++;
				name = "image_" + to_string(i) +".jpg";
				imwrite(IMG_DIR + name, frame);
				cout << name << endl;
			}
		}
		else{
			cout << "ERROR: Cannot read a frame from Cam 0." << endl;
		}
	}
	cam.~VideoCapture();
	destroyWindow("Cam 0");

	cout << endl << "Cam 0 stopped." << endl << endl << "Press ENTER to end...";
	getchar();
	return 0;
}
*/

//Corner detection
/*
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

/// Global variables
Mat src, src_gray;
int thresh = 200;
int max_thresh = 255;

char* source_window = "Source image";
char* corners_window = "Corners detected";

/// Function header
void cornerHarris_demo(int, void*);

/// @function main
int main(int argc, char** argv){
	bool stop = false;
	/// Load source image and convert it to gray
	VideoCapture cam(0);
	if (!cam.isOpened()){
		cout << "ERROR: Cannot open Cam 0!" << endl;
		return -1;
	}
	int i = 0;
	while (!stop){
		i++;
		if (!cam.read(src)){
			cout << "ERROR: Cannot get frame from Cam 0!" << endl;
			return -1;
		}
		cout << i << "\ttake_pic";
		cvtColor(src, src_gray, CV_BGR2GRAY);

		/// Create a window and a trackbar
		namedWindow(source_window, CV_WINDOW_AUTOSIZE);
		createTrackbar("Threshold: ", source_window, &thresh, max_thresh, cornerHarris_demo);
		imshow(source_window, src);
		cout << " Harris_begin";
		cornerHarris_demo(0, 0);
		cout << " Harris_end" << endl;

		if (waitKey(30) == 27){			//wait for ESC key press for 20ms
			cout << endl << "ESC key is pressed by user." << endl;
			stop = true;
		}
	}
	return(0);
}

/// @function cornerHarris_demo
void cornerHarris_demo(int, void*){
	Mat dst, dst_norm, dst_norm_scaled;
	dst = Mat::zeros(src.size(), CV_32FC1);

	/// Detector parameters
	int blockSize = 2;
	int apertureSize = 3;
	double k = 0.03;

	/// Detecting corners
	cornerHarris(src_gray, dst, blockSize, apertureSize, k, BORDER_DEFAULT);

	/// Normalizing
	normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst_norm, dst_norm_scaled);

	/// Drawing a circle around corners
	for (int j = 0; j < dst_norm.rows; j++){
		for (int i = 0; i < dst_norm.cols; i++){
			if ((int)dst_norm.at<float>(j, i) > thresh){
				circle(dst_norm_scaled, Point(i, j), 5, Scalar(0), 2, 8, 0);
			}
		}
	}

	/// Showing the result
	namedWindow(corners_window, CV_WINDOW_AUTOSIZE);
	imshow(corners_window, dst_norm_scaled);
}
*/

//Color writing
/*
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv){
	VideoCapture cap(0); //capture the video from webcam

	if (!cap.isOpened()){  // if not success, exit program
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

	int iLowH = 170;
	int iHighH = 179;

	int iLowS = 150;
	int iHighS = 255;

	int iLowV = 60;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	int iLastX = -1;
	int iLastY = -1;

	//Capture a temporary image from the camera
	Mat imgTmp;
	cap.read(imgTmp);

	//Create a black image with the size as the camera output
	Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);

	while (true){
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal); // read a new frame from video

		if (!bSuccess){ //if not success, break loop
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		//morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//morphological closing (removes small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

		//Calculate the moments of the thresholded image
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;

		// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero 
		if (dArea > 10000){
			//calculate the position of the ball
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;

			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0){
				//Draw a red line from the previous point to the current point
				line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
			}

			iLastX = posX;
			iLastY = posY;
		}

		imshow("Thresholded Image", imgThresholded); //show the thresholded image

		imgOriginal = imgOriginal + imgLines;
		imshow("Original", imgOriginal); //show the original image

		if (waitKey(30) == 27){ //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return 0;
}
*/