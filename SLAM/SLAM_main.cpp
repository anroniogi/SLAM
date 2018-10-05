#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//using namespace qrk;
//using namespace std;
//using namespace cv;

#pragma warning (disable : 4996)


int main(int argc, char *argv[]) {
	
	
	initialize(argc, argv);


	while (1) {

		localize();
		
		mapping();
		
		find_corner();
		
		move();
	
		Sleep(500);

	}



	return 0;
}