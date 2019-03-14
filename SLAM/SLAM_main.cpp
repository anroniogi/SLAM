#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

#pragma warning (disable : 4996)


int main(int argc, char *argv[]) {
	
	stellaInitialize();
	
	while (1) {
		
		//move(왼쪽 바퀴 속도, 오른쪽 바퀴 속도)
		move(20, 20);
		
		Sleep(1000);

	}

	return 0;
}