#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

#pragma warning (disable : 4996)


int main(int argc, char *argv[]) {
	
	stellaInitialize();
	
	while (1) {
		
		//move(���� ���� �ӵ�, ������ ���� �ӵ�)
		move(20, 20);
		
		Sleep(1000);

	}

	return 0;
}