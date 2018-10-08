#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//using namespace qrk;
//using namespace std;
//using namespace cv;

#pragma warning (disable : 4996)


int main(int argc, char *argv[]) {
	

	// 인준이 debug용
	
	laserScannerInitialize(argc, argv);
	Sleep(1000);
	while (1) {
		
		getLaserData(argc, argv);
		findRoute();
		Sleep(500);
	}
	


	initialize(argc, argv);


	while (1) {

		localize(argc, argv);
		
		mapping();
		
		move();
	
		Sleep(1000);

	}

	return 0;
}