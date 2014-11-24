#include "capture.h"

#define IMG_DIR "output\\image_"
#define FILTERS 8
#define AREA 100000

//Parameters default values
              // red  gre  dbl  lbl  bla   bro  yel  pur CUB
int iLowH[]  = { 159,  43, 105,  92,  98, 109,  21, 159,   0 };	//0
int iHighH[] = { 179,  77, 115, 108, 179, 179,  29, 173, 179 };	//179
int iLowS[]  = { 124,  70, 108, 128,  11,  39, 127,   0,   0 };	//0
int iHighS[] = { 255, 255, 204, 255,  57,  83, 255, 213, 255 };	//255
int iLowV[]  = {  85,   0,  72,  77,  25,  44, 123,  72,   0 };	//0
int iHighV[] = { 255, 255, 255, 187,  71, 114, 255, 255, 255 };	//255
string names[] = { "RED", "GRE", "DBL", "LBL", "BLA", "BRO", "YEL", "PUR" };

int oldVal = 0;		//Variable to store ID of previously edited filter
Mat mask;			//Mask to set the region of detection

VideoCapture *cap;	//Camera descriptor
double cWidth;		//Camera frame width
double cHeight;		//Camera frame height

//Errors printing function
int errorFunc(String text, int value){
	destroyAllWindows();
	cout << "------------------------------------------------------------" << endl;
	cout << endl << "ERROR: " << text << endl;
	cout << "Press ENTER to exit." << endl;
	getchar();
	return value;
}

//Loading filters parameters
void loadConfig(){
	string tmp;
	fstream config;
	config.open("config.cfg", fstream::in);
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
	config.open("config.cfg", fstream::out | fstream::trunc);
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
	//Threads to parallel detection of vertices
	omp_set_num_threads(8);

	//Load config
	loadConfig();

	//Initialize Cam 0
	cap = new VideoCapture(0);
	if (!cap->isOpened()){
		return errorFunc("Cannot open Cam 0!", -1);
	}
	//Get camera frame size
	cWidth = cap->get(CV_CAP_PROP_FRAME_WIDTH);
	cHeight = cap->get(CV_CAP_PROP_FRAME_HEIGHT);

	//Create mask to select the region of detection
	mask = Mat::zeros(Size((int)cWidth, (int)cHeight), CV_8UC1);
	rectangle(mask, Point((int)(cWidth*0.0), (int)(cHeight*0.0)), Point((int)(cWidth*1.0), (int)(cHeight*1.0)), Scalar(255, 255, 255), -1);
	return 0;
}

//Finalization of capture
int destCap(){
	destroyAllWindows();
	cap->~VideoCapture();
	return 0;
}

//What function call from this modules
int capture(int arg, void* data){
	switch (arg){
	case 1:
		return detecting();			//<requires initCap()>
	case 2:
		return detectOnce((Vec3d*)data);		//<requires initCap()>
	case 3:
		return initCap();
	case 4:
		return initialCalibration();//<requires initCap()>
	case 5:
		return destCap();			//<requires initCap()>
	case 6:
		return imgProcess();
	case 7:
		return filterImages();
	case 8:
		return takePictures();		//<requires initCap()>
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
	cvCreateTrackbar("FILTER", "Configure", filter, FILTERS - 1, onChange);
	cvCreateTrackbar("LowH", "Configure", iLowHt, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Configure", iHighHt, 179);
	cvCreateTrackbar("LowS", "Configure", iLowSt, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Configure", iHighSt, 255);
	cvCreateTrackbar("LowV", "Configure", iLowVt, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Configure", iHighVt, 255);
}



//VERTICES DETECTION
int detecting(){
	omp_set_num_threads(8);

	int i;
	Mat imgOriginal, imgHSV, imgThresholded, imgToShow;
	Mat imgThresh[FILTERS];
	double posX[FILTERS];
	double posY[FILTERS];
	bool bSuccess;

	cout << "Press ENTER to exit. <focused on video window>" << endl;
	cout << "------------------------------------------------------------" << endl;
	while (true){
		bSuccess = cap->read(imgOriginal);
		if (!bSuccess){
			return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
		}
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
		imgThresholded = Mat::zeros(Size((int)cWidth, (int)cHeight), CV_8UC1);
		imgToShow = imgOriginal.clone();
		#pragma omp parallel for schedule(static, 1)
		for (i = 0; i < FILTERS; i++){
			inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]);
			bitwise_and(imgThresh[i], mask, imgThresh[i]);

			//morphological opening (remove small objects from the foreground)
			erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
			dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
			//morphological closing (fill small holes in the foreground)
			dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
			erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));

			Moments oMoments = moments(imgThresh[i]);
			double dM01 = oMoments.m01;
			double dM10 = oMoments.m10;
			double dArea = oMoments.m00;
			if (dArea > AREA){
				posX[i] = dM10 / dArea;
				posY[i] = dM01 / dArea;
				#pragma omp critical
				{
					circle(imgToShow, Point((int)posX[i], (int)posY[i]), 10, Scalar(0, 255, 0), -1);
					putText(imgToShow, names[i], Point((int)posX[i] - 8, (int)posY[i] + 2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
				}
			}
		}
		for (i = 0; i < FILTERS; i++){
			bitwise_or(imgThresholded, imgThresh[i], imgThresholded);
		}

		imshow("Region of detection", mask);
		imshow("Thresholded", imgThresholded);
		imshow("VIDEO STREAM", imgToShow);

		if (waitKey(20) == 13){		//ENTER - exit
			cout << "Detecting END.\n" << endl;
			break;
		}
	}
	return 0;
}

//VERTICES SINGLE DETECTION
int detectOnce(Vec3d *vec){
	static int i;
	static Mat imgOriginal, imgHSV, imgThresholded, imgToShow;
	static Mat imgThresh[FILTERS];
	static double posX[FILTERS];
	static double posY[FILTERS];
	static bool bSuccess;
	bSuccess = cap->read(imgOriginal);
	if (!bSuccess){
		return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
	}
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
	imgThresholded = Mat::zeros(Size((int)cWidth, (int)cHeight), CV_8UC1);
	imgToShow = imgOriginal.clone();
	#pragma omp parallel for schedule(static, 1)
	for (i = 0; i < FILTERS; i++){
		inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]);
		bitwise_and(imgThresh[i], mask, imgThresh[i]);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));

		Moments oMoments = moments(imgThresh[i]);
		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;
		if (dArea > AREA){
			posX[i] = dM10 / dArea;
			posY[i] = dM01 / dArea;
			#pragma omp critical
			{
				circle(imgToShow, Point((int)posX[i], (int)posY[i]), 10, Scalar(0, 255, 0), -1);
				putText(imgToShow, names[i], Point((int)posX[i] - 8, (int)posY[i] + 2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
			}
		}
	}
	for (i = 0; i < FILTERS; i++){
		bitwise_or(imgThresholded, imgThresh[i], imgThresholded);
	}
	imshow("Thresholded", imgThresholded);
	imshow("VIDEO STREAM", imgToShow);

	//Angles get (POSIT)
	*vec = Vec3d((int)posX[0], (int)posY[0], 0);
	return 0;	//return wektor k¹tów obrotów wokó³ osi (x, y, z)
}

//Filters video calibration
int initialCalibration(){
	if (!cap->isOpened()){
		return errorFunc("Cam 0 was not initialized!", -1);
	}

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
	Mat imgOriginal, imgHSV, imgThresholded;
	Moments oMoments;
	int key;
	double posX, posY, dM01, dM10, dArea;
	bool bSuccess;
	
	//Configuring filters parameters
	cout << "CONFIGURE PARAMETERS." << endl;
	cout << "To confirm and save, press ENTER. <focused on video window>" << endl;
	cout << "To discard changes, press ESC. <focused on video window>" << endl;
	while (true){
		bSuccess = cap->read(imgOriginal);
		if (!bSuccess){
			return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
		}
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
		inRange(imgHSV, Scalar(iLowHt, iLowSt, iLowVt), Scalar(iHighHt, iHighSt, iHighVt), imgThresholded);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));

		oMoments = moments(imgThresholded);
		dM01 = oMoments.m01;
		dM10 = oMoments.m10;
		dArea = oMoments.m00;
		cout << "dArea: " << dArea << "                   \r";
		if (dArea > AREA){
			posX = dM10 / dArea;
			posY = dM01 / dArea;
			circle(imgOriginal, Point((int)posX, (int)posY), 10, Scalar(0, 255, 0), -1);
			putText(imgOriginal, names[filter], Point((int)posX - 8, (int)posY + 2), CV_FONT_HERSHEY_SIMPLEX, 0.25, Scalar(0, 0, 0), 1);
		}

		imshow("Thresholded video", imgThresholded);
		imshow("Video stream", imgOriginal);

		key = waitKey(20);
		if (key == 13){			//ENTER - confirm
			cout << "\rCalibration DONE.        \n" << endl;
			iLowH[filter] = iLowHt;
			iHighH[filter] = iHighHt;
			iLowS[filter] = iLowSt;
			iHighS[filter] = iHighSt;
			iLowV[filter] = iLowVt;
			iHighV[filter] = iHighVt;
			saveConfig();
			break;
		}
		else if (key == 27){	//ESC - discard changes
			cout << "\rCalibration skipped.        \n" << endl;
			//Load config again
			loadConfig();
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

	Mat imgThresh[FILTERS];
	Mat imgOriginal;
	Mat imgHSV;
	Moments oMoments;
	double posX, posY, dM01, dM10, dArea;
	int i, img = 0;

	cout << "This function process image_n.bmp where n = 0, 1, 2, ..." << endl;
	cout << "------------------------------------------------------------" << endl;
	while (1){
		imgOriginal = imread(IMG_DIR + to_string(img) + ".bmp");
		if (!imgOriginal.data){
			cout << "------------------------------------------------------------" << endl;
			cout << "Processed " << img << " images." << endl;
			cout << "Press ENTER to close." << endl;

			getchar();
			break;
		}
		cout << "Procesing image_" + to_string(img) + ".bmp" << endl;
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		for (i = 0; i < FILTERS; i++){
			inRange(imgHSV, Scalar(iLowH[i], iLowS[i], iLowV[i]), Scalar(iHighH[i], iHighS[i], iHighV[i]), imgThresh[i]);
				
			//morphological opening (remove small objects from the foreground)
			erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
			dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
			//morphological closing (fill small holes in the foreground)
			dilate(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
			erode(imgThresh[i], imgThresh[i], getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
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
int filterImages(){
	//Load configuration
	loadConfig();

	int filter = 0;
	int iLowHt = iLowH[0];
	int iHighHt = iHighH[0];
	int iLowSt = iLowS[0];
	int iHighSt = iHighS[0];
	int iLowVt = iLowV[0];
	int iHighVt = iHighV[0];

	int img = 0;
	Mat imgOriginal, imgHSV;
	imgOriginal = imread(IMG_DIR + to_string(img) + ".bmp");
	if (!imgOriginal.data){
		return errorFunc("Properly named images not found! <image_n.bmp where n = 0, 1, 2, ...>", -1);
	}
	cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

	//Creating Calibration window
	configWindow(&filter, &iLowHt, &iHighHt, &iLowSt, &iHighSt, &iLowVt, &iHighVt);
	
	Mat imgThresholded, imgToShow;
	Moments oMoments;
	double posX, posY, dM01, dM10, dArea;
	int key;
	cout << "Press N to switch to next image. <focused on image window>" << endl;
	cout << "Press S to save configuration. <focused on image window>" << endl;
	cout << "Press ENTER to close. <focused on image window>" << endl;
	cout << "------------------------------------------------------------" << endl;
	while (true){
		inRange(imgHSV, Scalar(iLowHt, iLowSt, iLowVt), Scalar(iHighHt, iHighSt, iHighVt), imgThresholded);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(4, 4)));

		oMoments = moments(imgThresholded);
		dM01 = oMoments.m01;
		dM10 = oMoments.m10;
		dArea = oMoments.m00;
		imgToShow = imgOriginal.clone();
		cout << "dArea: " << dArea << "                   \r";
		if (dArea > AREA){
			posX = dM10 / dArea;
			posY = dM01 / dArea;
			circle(imgToShow, Point((int)posX, (int)posY), 10, Scalar(0, 255, 0), -1);
		}

		imshow("Thresholded image", imgThresholded);
		imshow("Original image", imgToShow);

		key = waitKey(20);
		if (key == 13){			//ENTER - close
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
			cout << "Configuration saved." << endl;
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
	if (!cap->isOpened()){
		return errorFunc("Cam 0 was not initialized!", -1);
	}
	Mat imgOriginal;
	bool bSuccess;
	int key, img = 0;

	cout << "Press C to take a picture. <focused on video window>" << endl;
	cout << "Press ENTER to close. <focused on video window>" << endl;
	cout << "------------------------------------------------------------" << endl;
	while (true){
		bSuccess = cap->read(imgOriginal);
		if (!bSuccess){
			return errorFunc("Cannot read a frame from video stream of Cam 0!", -1);
		}
		imshow("Video stream", imgOriginal);

		key = waitKey(20);
		if (key == 13){			//ENTER - close
			break;
		}
		else if (key == 99){	//C - take picture
			imwrite(IMG_DIR + to_string(img) + ".bmp", imgOriginal);
			cout << "Picture saved to image_" + to_string(img) + ".bmp" << endl;
			img++;
		}
	}
	return 0;
}