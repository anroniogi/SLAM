#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//stella 변수
CSerialPort *_rc;
CStellaB1 *_sg;
char state;
int turn_ㅣeft= 0 ;

// Mobile Robot Position, 단위는 cm
struct Position {
	long x = 0, y = 0, theta = 0;
} Position;


void stellaInitialize() {
	//stella 연결
	_rc = new CSerialPort();
	_rc->Open("COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	_rc->SetTimeout(100, 100, 10);
	_sg = new CStellaB1(_rc);

	// 스텔라 이니셜라이즈
	_sg->Init();
	_sg->Reset();
}


void move(int left, int right) {
	
	_sg->Velocity(left, right);
	printf("\n");
	
}


void getPosition() {
	float left, right;
	double wheel_distance = 75.2;
	double robot_x, robot_y, robot_theta;
	double ds;

	_sg->GetPosition(&left, &right);
	
	// stella 포지션 계산
	// meter단위를 cm단위로 변환
	left *= 1000;
	right *= 1000;

	// 휠의 이동거리 left와 right로 x, y, theta 계산
	ds = (right + left) / 2;
	robot_theta = (right - left) / (wheel_distance);
	robot_x = ds*cos(robot_theta / 2);
	robot_y = ds*sin(robot_theta / 2);

	Position.x = (ds*cos(robot_theta / 2))*10;
	Position.y = (ds*sin(robot_theta / 2))*10;
	Position.theta = (right - left) / (wheel_distance);

#ifdef DEBUG
	printf("\nrobot position, theta\n");
	printf("robot_x = %f, robot_y = %f, robot_theta = %f\n", robot_x, robot_y, robot_theta);
	printf("\n%1d  %10.1f  %10.1f \n", (int)state, (double)left, (double)right);
#endif

}