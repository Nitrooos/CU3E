#include "capture.h"
/*
*	capture(arg)
*	arg:
*		1 - DETECTING VERTICES						<requires 3>
*		2 - DETECT VERTICES ONCE					<requires 3>
*		3 - camera stuff initialization				<VIDEO INIT>
*		4 - calibration								<requires 3>
*		5 - camera stuff finalization				<VIDEO FINA>
*		6 - processing images using filters			<>
*		7 - calibration on images					<>
*		8 - take pictures to use in 3 and 4			<requires 3>
*/


int main(int argc, char** argv){
	int result = 0;
	for (int i = 1; i < argc; i++){
		if (atoi(argv[i]) == 2){
			CvMatr32f rotation = new float[9];
			CvVect32f translation = new float[3];
			cout << "You are using Single Vertices Detection in infinite loop. Press ESC to stop." << endl;
			while (1){
				capture(2, rotation, translation);
				if (waitKey(20) == 27){		//ESC - exit
					cout << endl << "ESC: Exit" << endl;
					cout  << "------------------------------------------------------------" << endl;
					break;
				}
			}
		}
		else{
			result += capture(atoi(argv[i]));	//"capture()" decides what function to execute
		}
	}
	return result;
}
