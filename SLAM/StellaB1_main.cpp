// StellaB1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SerialPort.h"
#include "StellaB1.h"
#include <windows.h>
#include <math.h>

/*
int _tmain(int argc, _TCHAR* argv[])
{
	CSerialPort *_rc = new CSerialPort ();
	_rc->Open ("COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	_rc->SetTimeout (100, 100, 10);

	CStellaB1 *_sg = new CStellaB1 (_rc);

	char state;
	float left, right;
	double wheel_distance = 75.2;
	double robot_x, robot_y, robot_theta;
	double ds;


	_sg->Init ();
	_sg->Reset ();

	_sg->GetState(&state);
	_sg->GetPosition(&left, &right);



	// 휠의 이동거리 left와 right로 x, y, theta 계산
	ds = (right + left) / 2;
	robot_theta = (right - left) / (wheel_distance);
	robot_x = ds*cos(robot_theta/2);
	robot_y = ds*sin(robot_theta/2);

	printf("\nrobot position, theta\n");
	printf("robot_x = %f, robot_y = %f, robot_theta = %f\n", robot_x, robot_y, robot_theta);



	printf("\n%1d  %10.1f  %10.1f \n", (int)state, (double)left, (double)right);



	_sg->Velocity (80, 0);
	Sleep(10000);

	_sg->Stop (3);

	_sg->GetState (&state);
	_sg->GetPosition (&left, &right);

	printf ("\n%1d  %10.1f  %10.1f \n", (int)state, (double)left, (double)right);

	// 휠의 이동거리 left와 right로 x, y, theta 계산
	ds = (right + left) / 2;
	robot_theta = (right - left) / (wheel_distance);
	robot_x = ds*cos(robot_theta / 2);
	robot_y = ds*sin(robot_theta / 2);

	printf("\nrobot position, theta\n");
	printf("robot_x = %f, robot_y = %f, robot_theta = %f\n", robot_x, robot_y, robot_theta);


	

	delete _sg;
	delete _rc;

	return 0;
}

*/