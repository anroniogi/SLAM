#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//using namespace qrk;
//using namespace std;
//using namespace cv;

#pragma warning (disable : 4996)


int main(int argc, char *argv[]) {
	
	laserScannerInitialize(argc, argv);

	/* ������ debug��
	while (1) {
		
		getLaserData();
		findRoute();

	}
	*/
	
	initialize(argc, argv);


	while (1) {

		localize();
		
		//������ debug��
		findRoute();

		mapping();
		
		move();
	
		Sleep(1000);

	}

	return 0;
}