#include "capture.h"
	          //                          ???   ???
	          // red  red  gre  dbl  lbl  bla   bro  yel  pur
int iLowH[]  = { 159,   0,  43, 105,  92,  98, 109,  21, 159 };	//0
int iHighH[] = { 179, 179,  77, 115, 108, 179, 179,  29, 173 };	//179
int iLowS[]  = { 124, 300,  70, 108, 128,  11,  39, 127,   0 };	//0
int iHighS[] = { 255, 255, 255, 204, 255,  57,  83, 255, 213 };	//255
int iLowV[]  = {  85,   0,   0,  72,  77,  25,  44, 123,  72 };	//0
int iHighV[] = { 255, 255, 255, 255, 187,  71, 114, 255, 255 };	//255


int calibrate(){
	int edo = 3;
	int edc = 3;

	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	//Create trackbars in "Control" window
	cvCreateTrackbar("EDOpen", "Control", &edo, 19);
	cvCreateTrackbar("EDClose", "Control", &edc, 19);

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	
	int img = 0;
	Mat imgOriginal;
	Mat imgHSV;
	Mat imgThresholded;
	imgOriginal = imread("output\\" + to_string(img) + ".jpg");
	int key;

	while (true)
	{
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
		
		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edo+1, edo+1)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edo+1, edo+1)));

		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edc+1, edc+1)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edc+1, edc+1)));

		imshow("Thresholded Image", imgThresholded);
		imshow("Original", imgOriginal);

		key = waitKey(10);
		if (key == 27)
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
		else if(key == 99){
			img = (img + 1) % 9;
			imgOriginal = imread("output\\" + to_string(img) + ".jpg");
		}
	}
	return 0;
}

int imgProcess(){
	int edo = 3;
	int edc = 3;

	bool forAll = false;

	Mat imgThresh[9];
	Mat imgHSV[9];
	Mat tmp;

	for (int i = 0; i < 9; i++){
		imgThresh[i] = imread("output\\" + to_string(i) + ".jpg");
		cvtColor(imgThresh[i], imgHSV[i], COLOR_BGR2HSV);

		if (forAll){
			tmp = Mat::zeros(imgThresh[i].size(), CV_8UC1);
			for (int j = 0; j < 9; j++){
				inRange(imgHSV[i], Scalar(iLowH[j], iLowS[j], iLowV[j]), Scalar(iHighH[j], iHighS[j], iHighV[j]), imgThresh[j]); //Threshold the image
				tmp += imgThresh[j];
			}
			imgThresh[i] = tmp;
		}
		else{
			inRange(imgHSV[i], Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]); //Threshold the image
		}
		
		//morphological opening (remove small objects from the foreground)
		erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(edo + 1, edo + 1)));
		dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(edo+1, edo+1)));
		
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(edc+1, edc+1)));
		erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(edc+1, edc+1)));
		
		imwrite("output\\" + to_string(i) + "p.jpg", imgThresh[i]);
	}
	return 0;
}

int capturing(VideoCapture *cap){
	int edo = 3;
	int edc = 3;

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	//Create trackbars in "Control" window
	cvCreateTrackbar("EDOpen", "Control", &edo, 19);
	cvCreateTrackbar("EDClose", "Control", &edc, 19);

	Mat imgOriginal;
	Mat imgHSV;
	Mat imgThresh[9];
	Mat imgThresholded;
	int i;
	bool bSuccess;

	while (true){
		bSuccess = cap->read(imgOriginal); // read a new frame from Cam 0
		if (!bSuccess){
			cout << "ERROR: Cannot read a frame from video stream of Cam 0!" << endl;
			return -1;
		}

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
		imgThresholded = Mat::zeros(imgOriginal.size(), CV_8UC1);
		for (i = 0; i < 9; i++){
			inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]); //Threshold the image
			imgThresholded += imgThresh[i];
		}
		imshow("Source threshold", imgThresholded);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edo+1, edo+1)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edo+1, edo+1)));
		imshow("After open", imgThresholded);

		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edc+1, edc+1)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(edc+1, edc+1)));
		imshow("After close", imgThresholded); //show the thresholded image

		imshow("Original", imgOriginal); //show the original image

		if (waitKey(20) == 27){ //wait for 'esc' key press for 20ms. If 'esc' key is pressed, break loop
			cout << "ESC key is pressed by user." << endl;
			break;
		}
	}
	return 0;
}