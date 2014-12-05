#include "Capture.hpp"
#include "App.hpp"

#define IMG_DIR "output/image_"
#define FILTERS 7			//Number of detecting objects
#define AREA 10000			//Tolerance for vertices detection
#define R_MUL 1.1			//Region of detection radious multiplier
#define FOCAL_LENGTH 1000	//Focal length
#define CAM_ID 0			//Camera number

//Parameters default values
              // CUB  yel  ora  red  blu  ???
int iLowH[]  = {   0,   0,   0,   0,   0,   0 };	//0
int iHighH[] = { 179, 179, 179, 179, 179, 179 };	//179
int iLowS[]  = {   0,   0,   0,   0,   0,   0 };	//0
int iHighS[] = { 255, 255, 255, 255, 255, 255 };	//255
int iLowV[]  = {   0,   0,   0,   0,   0,   0 };	//0
int iHighV[] = { 255, 255, 255, 255, 255, 255 };	//255
string names[] = { "CUB", "YEL", "ORA", "RED", "BLU", "???" };

int oldVal = 0;		//Variable to store ID of previously edited filter
Mat mask;			//Mask to set the region of detection
Mat edStrEl = getStructuringElement(MORPH_ELLIPSE, Size(4, 4));  //Structuring element for erode and dilate

VideoCapture *cap;	//Camera descriptor

//Errors printing function
int errorFunc(string text, int value){
	destroyAllWindows();
	cout << "------------------------------------------------------------" << endl;
	cout << endl << "ERROR: " << text << endl;
	cout << "Press ENTER to exit." << endl;
	getchar();
	cout << "------------------------------------------------------------" << endl;
	return value;
}

//Loading filters parameters
void loadConfig(){
	string tmp;
	fstream config;
	config.open("data/config.cfg", fstream::in);
	for (int i = 0; i < FILTERS; i++){
		getline(config, tmp);
		iLowH[i] = atoi(tmp.c_str());
		getline(config, tmp);
		iHighH[i] = atoi(tmp.c_str());
		getline(config, tmp);
		iLowS[i] = atoi(tmp.c_str());
		getline(config, tmp);
		iHighS[i] = atoi(tmp.c_str());
		getline(config, tmp);
		iLowV[i] = atoi(tmp.c_str());
		getline(config, tmp);
		iHighV[i] = atoi(tmp.c_str());
	}
	config.close();
}

//Saving filters parameters
void saveConfig(){
	fstream config;
	config.open("data/config.cfg", fstream::out | fstream::trunc);
	for (int i = 0; i < FILTERS; i++){	//Saving parameters
		config << iLowH[i] << endl;
		config << iHighH[i] << endl;
		config << iLowS[i] << endl;
		config << iHighS[i] << endl;
		config << iLowV[i] << endl;
		config << iHighV[i] << endl;
	}
	config.close();
}

//Initialization of capture
int initCap(){
	double cWidth;
	double cHeight;

	//Threads to parallel detection of vertices
	omp_set_num_threads(FILTERS-1);

	//Load config
	loadConfig();

	//Initialize Cam 0
	cap = new VideoCapture(CAM_ID);
	if (!cap->isOpened()){
		return errorFunc("Cannot open Cam 0!", -1);
	}
	//Get camera frame size
	cWidth = cap->get(CV_CAP_PROP_FRAME_WIDTH);
	cHeight = cap->get(CV_CAP_PROP_FRAME_HEIGHT);

	//Create initial mask to select the region of detection
	mask = Mat::zeros(Size((int)cWidth, (int)cHeight), CV_8UC1);
	return 0;
}

//Finalization of capture
int destCap(){
	destroyAllWindows();
	cap->~VideoCapture();
	return 0;
}

//What function call from this modules
int capture(int arg, CvMatr32f rotation, CvMatr32f translation){
	switch (arg){
	case 1:
		return detecting();					//<requires initCap()>
	case 2:
		return detectOnce(rotation, translation);	//<requires initCap()>
	case 3:
		return initCap();					//<VIDEO INIT>
	case 4:
		return initialCalibration();		//<requires initCap()>
	case 5:
		return destCap();					//<VIDEO FINA>
	case 6:
		return imgProcess();
	case 7:
		return filtersOnImages();
	case 8:
		return takePictures();				//<requires initCap()>
	default:
		return errorFunc("Unrecognized argument!", -1);
	}
}

//Switching parameters (on filter change)
void edit(int value, void* userdata){
	iLowH[oldVal] = getTrackbarPos("LowH", "Configure");
	iHighH[oldVal] = getTrackbarPos("HighH", "Configure");
	iLowS[oldVal] = getTrackbarPos("LowS", "Configure");
	iHighS[oldVal] = getTrackbarPos("HighS", "Configure");
	iLowV[oldVal] = getTrackbarPos("LowV", "Configure");
	iHighV[oldVal] = getTrackbarPos("HighV", "Configure");

	oldVal = value;

	setTrackbarPos("LowH", "Configure", iLowH[value]);
	setTrackbarPos("HighH", "Configure", iHighH[value]);
	setTrackbarPos("LowS", "Configure", iLowS[value]);
	setTrackbarPos("HighS", "Configure", iHighS[value]);
	setTrackbarPos("LowV", "Configure", iLowV[value]);
	setTrackbarPos("HighV", "Configure", iHighV[value]);
}

//Creater config window
void configWindow(int *filter, int *iLowHt, int *iHighHt, int *iLowSt, int *iHighSt, int *iLowVt, int *iHighVt){
	namedWindow("Configure", CV_WINDOW_NORMAL);
	CvTrackbarCallback onChange = (CvTrackbarCallback)edit;
	cvCreateTrackbar("FILTER", "Configure", filter, FILTERS-1, onChange);
	cvCreateTrackbar("LowH", "Configure", iLowHt, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Configure", iHighHt, 179);
	cvCreateTrackbar("LowS", "Configure", iLowSt, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Configure", iHighSt, 255);
	cvCreateTrackbar("LowV", "Configure", iLowVt, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Configure", iHighVt, 255);
}



//VERTICES DETECTION
int detecting(){
	unsigned i;
	float radius, mRadius;
	double dM01, dM10, dArea, posX[FILTERS], posY[FILTERS];
	Point2f center, mCenter;
	Moments oMoments;
	Mat imgOriginal, imgHSV, imgThresholded, imgToShow, imgThresh[FILTERS];
	vector<vector<Point>> contours;

	cout << "CONTINUOUS DETECTION." << endl;
	cout << "Press ENTER to exit. <focused on video window>" << endl;
	while (true){
		if (!cap->read(imgOriginal)){
			return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
		}
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
		//Create mask
		inRange(imgHSV, Scalar(iLowH[0], iLowS[0], iLowV[0]), Scalar(iHighH[0], iHighS[0], iHighV[0]), imgThresh[0]);
		erode(imgThresh[0], imgThresh[0], edStrEl);
		dilate(imgThresh[0], imgThresh[0], edStrEl);
		dilate(imgThresh[0], imgThresh[0], edStrEl);
		erode(imgThresh[0], imgThresh[0], edStrEl);
		findContours(imgThresh[0], contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		if (contours.size() > 0){
			mRadius = 0;
			for (i = 0; i < contours.size(); i++){
				minEnclosingCircle(Mat(contours[i]), center, radius);
				if (radius > mRadius){
					mRadius = radius;
					mCenter = center;
				}
			}
			mask = Mat::zeros(mask.size(), CV_8UC1);
			circle(mask, mCenter, (int)(mRadius*R_MUL), Scalar(255, 255, 255), -1);
		}

		imgThresholded = Mat::zeros(mask.size(), CV_8UC1);
		imgToShow = imgOriginal.clone();
		#pragma omp parallel for schedule(static, 1)
		for (i = 1; i < FILTERS; i++){
			inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]);
			bitwise_and(imgThresh[i], mask, imgThresh[i]);

			//morphological opening (remove small objects from the foreground)
			erode(imgThresh[i], imgThresh[i], edStrEl);
			dilate(imgThresh[i], imgThresh[i], edStrEl);
			//morphological closing (fill small holes in the foreground)
			dilate(imgThresh[i], imgThresh[i], edStrEl);
			erode(imgThresh[i], imgThresh[i], edStrEl);

			oMoments = moments(imgThresh[i]);
			dM01 = oMoments.m01;
			dM10 = oMoments.m10;
			dArea = oMoments.m00;
			if (dArea > AREA){
				posX[i] = dM10 / dArea - App::getWindowWidth()/2;
				posY[i] = App::getWindowHeight()/2 - dM01 / dArea;
				#pragma omp critical
				{
					circle(imgToShow, Point((int)posX[i], (int)posY[i]), 10, Scalar(0, 255, 0), -1);
					putText(imgToShow, names[i], Point((int)posX[i] - 8, (int)posY[i] + 2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
				}
			}
		}
		for (i = 1; i < FILTERS; i++){
			bitwise_or(imgThresholded, imgThresh[i], imgThresholded);
		}

		imshow("Region of detection", mask);
		imshow("Thresholded", imgThresholded);
		imshow("VIDEO STREAM", imgToShow);

		if (waitKey(20) == 1048586){		//ENTER - close
			destroyAllWindows();
			cout << "ENTER: closing." << endl;
			cout << "------------------------------------------------------------" << endl;
			break;
		}
	}
	return 0;
}

//VERTICES SINGLE DETECTION
int detectOnce(CvMatr32f rotation, CvMatr32f translation){
	static unsigned i;
	static bool vert[FILTERS];
	static float radius, mRadius;
	static double dM01, dM10, dArea, posX[FILTERS], posY[FILTERS];
	static Point2f center, mCenter;
	static Moments oMoments;
	static Mat imgOriginal, imgHSV, imgThresh[FILTERS];
	static vector<vector<Point>> contours;

	static vector<CvPoint2D32f> srcImagePoints;
	static vector<CvPoint3D32f> modelPoints;
	static CvPOSITObject* positObject;
	static double model[6][3] = {
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
	};
	static CvTermCriteria criteria = cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 100, 1.0e-4f);

	if (!cap->read(imgOriginal)){
		return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
	}
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
	//Create mask
	inRange(imgHSV, Scalar(iLowH[0], iLowS[0], iLowV[0]), Scalar(iHighH[0], iHighS[0], iHighV[0]), imgThresh[0]);
	erode(imgThresh[0], imgThresh[0], edStrEl);
	dilate(imgThresh[0], imgThresh[0], edStrEl);
	dilate(imgThresh[0], imgThresh[0], edStrEl);
	erode(imgThresh[0], imgThresh[0], edStrEl);
	findContours(imgThresh[0], contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	if (contours.size() > 0){
		mRadius = 0;
		for (i = 0; i < contours.size(); i++){
			minEnclosingCircle(Mat(contours[i]), center, radius);
			if (radius > mRadius){
				mRadius = radius;
				mCenter = center;
			}
		}
		mask = Mat::zeros(mask.size(), CV_8UC1);
		circle(mask, mCenter, (int)(mRadius*R_MUL), Scalar(255, 255, 255), -1);
	}
	#pragma omp parallel for schedule(static, 1)
	for (i = 1; i < FILTERS; i++){
		inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]);
		bitwise_and(imgThresh[i], mask, imgThresh[i]);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresh[i], imgThresh[i], edStrEl);
		dilate(imgThresh[i], imgThresh[i], edStrEl);
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresh[i], imgThresh[i], edStrEl);
		erode(imgThresh[i], imgThresh[i], edStrEl);

		oMoments = moments(imgThresh[i]);
		dM01 = oMoments.m01;
		dM10 = oMoments.m10;
		dArea = oMoments.m00;
		vert[i] = false;
		if (dArea > AREA){
			posX[i] = dM10/dArea - App::getWindowWidth()/2;
			posY[i] = App::getWindowHeight()/2 - dM01/dArea;
			vert[i] = true;
			//#pragma omp critical			//DELETE IN FINAL VERSION
			//{								//DELETE IN FINAL VERSION
			//	circle(imgToShow, Point((int)posX[i], (int)posY[i]), 10, Scalar(0, 255, 0), -1);
			//	putText(imgToShow, names[i], Point((int)posX[i] - 8, (int)posY[i] + 2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
			//}								//DELETE IN FINAL VERSION
		}
	}

	modelPoints.clear();
	srcImagePoints.clear();
	for (i = 1; i < FILTERS; i++){
		if (vert[i]){
			//Create the model points
			modelPoints.push_back(cvPoint3D32f(model[i-1][0], model[i-1][1], model[i-1][2]));
			//Create the image points
			srcImagePoints.push_back(cvPoint2D32f(posX[i], posY[i]));
		}
	}
	if (modelPoints.size() > 3){
		//Create the POSIT object with the model points
		positObject = cvCreatePOSITObject(&modelPoints[0], (int)modelPoints.size());
		//Estimate the pose
		cvPOSIT(positObject, &srcImagePoints[0], FOCAL_LENGTH, criteria, rotation, translation);
	}

	//imshow("VIDEO STREAM", imgOriginal);		//DELETE IN FINAL VERSION
	return 0;
}

//Filters video calibration
int initialCalibration(){
	//Trackbar initial values
	int filter = 0;
	int iLowHt = iLowH[0];
	int iHighHt = iHighH[0];
	int iLowSt = iLowS[0];
	int iHighSt = iHighS[0];
	int iLowVt = iLowV[0];
	int iHighVt = iHighV[0];

	//Creating Calibration window
	configWindow(&filter, &iLowHt, &iHighHt, &iLowSt, &iHighSt, &iLowVt, &iHighVt);

	//Initalizing variables for while" loop
	int key;
	unsigned i;
	float radius, mRadius;
	double posX, posY, dM01, dM10, dArea;
	Point2f center, mCenter;
	Mat imgOriginal, imgHSV, imgThresholded;
	Moments oMoments;
	vector<vector<Point>> contours;

	cout << "CONFIGURE PARAMETERS." << endl;
	cout << "Press ENTER to confirm and save. <focused on video window>" << endl;
	cout << "Press ESC to discard changes. <focused on video window>" << endl;
	while (true){
		if (!cap->read(imgOriginal)){
			return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
		}
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		//Create mask
		inRange(imgHSV, Scalar(iLowH[0], iLowS[0], iLowV[0]), Scalar(iHighH[0], iHighS[0], iHighV[0]), imgThresholded);
		erode(imgThresholded, imgThresholded, edStrEl);
		dilate(imgThresholded, imgThresholded, edStrEl);
		dilate(imgThresholded, imgThresholded, edStrEl);
		erode(imgThresholded, imgThresholded, edStrEl);
		findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		if (contours.size() > 0){
			mRadius = 0;
			for (i = 0; i < contours.size(); i++){
				minEnclosingCircle(Mat(contours[i]), center, radius);
				if (radius > mRadius){
					mRadius = radius;
					mCenter = center;
				}
			}
			mask = Mat::zeros(mask.size(), CV_8UC1);
			circle(mask, mCenter, (int)(mRadius*R_MUL), Scalar(255, 255, 255), -1);
		}

		inRange(imgHSV, Scalar(iLowHt, iLowSt, iLowVt), Scalar(iHighHt, iHighSt, iHighVt), imgThresholded);
		if (filter) bitwise_and(imgThresholded, mask, imgThresholded);
		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, edStrEl);
		dilate(imgThresholded, imgThresholded, edStrEl);
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, edStrEl);
		erode(imgThresholded, imgThresholded, edStrEl);

		oMoments = moments(imgThresholded);
		dM01 = oMoments.m01;
		dM10 = oMoments.m10;
		dArea = oMoments.m00;
		//cout << names[filter] << " : " << dArea << "                      \r";
		if (dArea > AREA){
			posX = dM10 / dArea;
			posY = dM01 / dArea;
			circle(imgOriginal, Point((int)posX, (int)posY), 10, Scalar(0, 255, 0), -1);
			putText(imgOriginal, names[filter], Point((int)posX - 8, (int)posY + 2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
		}

		imshow("Thresholded video", imgThresholded);
		imshow("VIDEO STREAM", imgOriginal);
		imshow("Mask", mask);

		key = waitKey(20);
		if (key == 1048586){			//ENTER - confirm and save
			cout << "\rCalibration DONE.               " << endl;
			cout << "ENTER: Parameters saved." << endl;
			cout << "------------------------------------------------------------" << endl;
			iLowH[filter] = iLowHt;
			iHighH[filter] = iHighHt;
			iLowS[filter] = iLowSt;
			iHighS[filter] = iHighSt;
			iLowV[filter] = iLowVt;
			iHighV[filter] = iHighVt;
			saveConfig();
			break;
		}
		else if (key == 1048603){	//ESC - discard changes
			cout << "\rCalibration done.        " << endl;
			cout << "ESC: Using previous parameters." << endl;
			cout << "------------------------------------------------------------" << endl;
			loadConfig();	//Load config again
			break;
		}
	}
	destroyAllWindows();
	return 0;
}

//Process images using filters
int imgProcess(){
	//Load config
	loadConfig();

	unsigned i, img = 0;
	float radius, mRadius;
	double posX, posY, dM01, dM10, dArea;
	Point2f center, mCenter;
	Moments oMoments;
	Mat imgHSV, imgOriginal;
	Mat imgThresh[FILTERS];
	vector<vector<Point>> contours;

	cout << "IMAGE PROCESSING" << endl;
	cout << "This function process image_n.bmp where n = 0, 1, 2, ..." << endl;
	cout << "--------------------" << endl;
	while (1){
		imgOriginal = imread(IMG_DIR + to_string(img) + ".bmp");
		if (!imgOriginal.data){
			cout << "--------------------" << endl;
			cout << "Processed " << img << " images." << endl;
			cout << "Press ENTER to close.";
			getchar();
			cout << "------------------------------------------------------------" << endl;
			break;
		}
		cout << "Procesing image_" + to_string(img) + ".bmp" << endl;
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
		//Create mask
		inRange(imgHSV, Scalar(iLowH[0], iLowS[0], iLowV[0]), Scalar(iHighH[0], iHighS[0], iHighV[0]), imgThresh[0]);
		erode(imgThresh[0], imgThresh[0], edStrEl);
		dilate(imgThresh[0], imgThresh[0], edStrEl);
		dilate(imgThresh[0], imgThresh[0], edStrEl);
		erode(imgThresh[0], imgThresh[0], edStrEl);
		if (contours.size() > 0){
			mRadius = 0;
			for (i = 0; i < contours.size(); i++){
				minEnclosingCircle(Mat(contours[i]), center, radius);
				if (radius > mRadius){
					mRadius = radius;
					mCenter = center;
				}
			}
			mask = Mat::zeros(mask.size(), CV_8UC1);
			circle(mask, mCenter, (int)(mRadius*R_MUL), Scalar(255, 255, 255), -1);
		}

		for (i = 1; i < FILTERS; i++){
			inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]);
			bitwise_and(imgThresh[i], mask, imgThresh[i]);

			//morphological opening (remove small objects from the foreground)
			erode(imgThresh[i], imgThresh[i], edStrEl);
			dilate(imgThresh[i], imgThresh[i], edStrEl);
			//morphological closing (fill small holes in the foreground)
			dilate(imgThresh[i], imgThresh[i], edStrEl);
			erode(imgThresh[i], imgThresh[i], edStrEl);
			//imwrite(IMG_DIR + to_string(img) + "_f" + to_string(i) + "p.bmp", imgThresh[i]);

			oMoments = moments(imgThresh[i]);
			dM01 = oMoments.m01;
			dM10 = oMoments.m10;
			dArea = oMoments.m00;
			if (dArea > AREA){
				posX = dM10 / dArea;
				posY = dM01 / dArea;
				circle(imgOriginal, Point((int)posX, (int)posY), 10, Scalar(0, 255, 0), -1);
				putText(imgOriginal, names[i], Point((int)posX-8, (int)posY+2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
			}
		}
		imwrite(IMG_DIR + to_string(img) + "p.bmp", imgOriginal);
		img++;
	}
	return 0;
}

//Calibration on images
int filtersOnImages(){
	//Load configuration
	loadConfig();

	int filter = 0;
	int iLowHt = iLowH[0];
	int iHighHt = iHighH[0];
	int iLowSt = iLowS[0];
	int iHighSt = iHighS[0];
	int iLowVt = iLowV[0];
	int iHighVt = iHighV[0];

	int key, img = 0;
	unsigned i;
	float radius, mRadius;
	double posX, posY, dM01, dM10, dArea;
	Mat imgOriginal, imgHSV, imgThresholded, imgToShow;
	Moments oMoments;
	Point2f center, mCenter;
	vector<vector<Point>> contours;

	imgOriginal = imread(IMG_DIR + to_string(img) + ".bmp");
	if (!imgOriginal.data){
		return errorFunc("Properly named images not found! <image_n.bmp where n = 0, 1, 2, ...>", -1);
	}
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

	//Creating Calibration window
	configWindow(&filter, &iLowHt, &iHighHt, &iLowSt, &iHighSt, &iLowVt, &iHighVt);

	cout << "Press N to switch to next image. <focused on image window>" << endl;
	cout << "Press S to save configuration. <focused on image window>" << endl;
	cout << "Press ENTER to close. <focused on image window>" << endl;
	while (true){
		//Create mask
		inRange(imgHSV, Scalar(iLowH[0], iLowS[0], iLowV[0]), Scalar(iHighH[0], iHighS[0], iHighV[0]), imgThresholded);
		erode(imgThresholded, imgThresholded, edStrEl);
		dilate(imgThresholded, imgThresholded, edStrEl);
		dilate(imgThresholded, imgThresholded, edStrEl);
		erode(imgThresholded, imgThresholded, edStrEl);
		findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		if (contours.size() > 0){
			mRadius = 0;
			for (i = 0; i < contours.size(); i++){
				minEnclosingCircle(Mat(contours[i]), center, radius);
				if (radius > mRadius){
					mRadius = radius;
					mCenter = center;
				}
			}
			mask = Mat::zeros(mask.size(), CV_8UC1);
			circle(mask, mCenter, (int)(mRadius*R_MUL), Scalar(255, 255, 255), -1);
		}
		inRange(imgHSV, Scalar(iLowHt, iLowSt, iLowVt), Scalar(iHighHt, iHighSt, iHighVt), imgThresholded);
		if (filter) bitwise_and(imgThresholded, mask, imgThresholded);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, edStrEl);
		dilate(imgThresholded, imgThresholded, edStrEl);
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, edStrEl);
		erode(imgThresholded, imgThresholded, edStrEl);

		oMoments = moments(imgThresholded);
		dM01 = oMoments.m01;
		dM10 = oMoments.m10;
		dArea = oMoments.m00;
		imgToShow = imgOriginal.clone();
		cout << names[filter] << " : " << dArea << "                      \r";
		if (dArea > AREA){
			posX = dM10 / dArea;
			posY = dM01 / dArea;
			circle(imgToShow, Point((int)posX, (int)posY), 10, Scalar(0, 255, 0), -1);
		}

		imshow("Thresholded image", imgThresholded);
		imshow("Original image", imgToShow);

		key = waitKey(20);
		if (key == 13){			//ENTER - close
			destroyAllWindows();
			cout << "ENTER: closing.           " << endl;
			cout << "------------------------------------------------------------" << endl;
			break;
		}
		else if (key == 115){	//S - save config
			iLowH[filter] = iLowHt;
			iHighH[filter] = iHighHt;
			iLowS[filter] = iLowSt;
			iHighS[filter] = iHighSt;
			iLowV[filter] = iLowVt;
			iHighV[filter] = iHighVt;
			saveConfig();
			cout << "S: Configuration saved.   " << endl;
		}
		else if(key == 110){	//N - next image
			img++;
			imgOriginal = imread(IMG_DIR + to_string(img) + ".bmp");
			if (!imgOriginal.data){
				img = 0;
				imgOriginal = imread(IMG_DIR + to_string(img) + ".bmp");
				if (!imgOriginal.data){
					return errorFunc("Properly named images not found! <image_n.bmp where n = 0, 1, 2, ...>", -1);
				}
			}
			cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
		}
	}
	return 0;
}

//Taking pictures
int takePictures(){
	Mat imgOriginal;
	int key, img = 0;

	cout << "TAKING PICTURES." << endl;
	cout << "Press C to take a picture. <focused on video window>" << endl;
	cout << "Press ENTER to close. <focused on video window>" << endl;
	cout << "--------------------" << endl;
	while (true){
		if (!cap->read(imgOriginal)){
			return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
		}
		imshow("VIDEO STREAM", imgOriginal);

		key = waitKey(20);
		if (key == 13){			//ENTER - close
			destroyAllWindows();
			cout << "--------------------" << endl;
			cout << "ENTER: closing." << endl;
			cout << "------------------------------------------------------------" << endl;
			break;
		}
		else if (key == 99){	//C - take picture
			imwrite(IMG_DIR + to_string(img) + ".bmp", imgOriginal);
			cout << "C: Picture saved to image_" + to_string(img) + ".bmp" << endl;
			img++;
		}
	}
	return 0;
}
