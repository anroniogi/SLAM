#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//stella ����
CSerialPort *_rc;
CStellaB1 *_sg;
char state;
int turn_��eft= 0 ;

// Mobile Robot Position, ������ cm
struct Position {
	long x = 0, y = 0, theta = 0;
} Position;


void stellaInitialize() {
	//stella ����
	_rc = new CSerialPort();
	_rc->Open("COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	_rc->SetTimeout(100, 100, 10);
	_sg = new CStellaB1(_rc);

	// ���ڶ� �̴ϼȶ�����
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
	
	// stella ������ ���
	// meter������ cm������ ��ȯ
	left *= 1000;
	right *= 1000;

	// ���� �̵��Ÿ� left�� right�� x, y, theta ���
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