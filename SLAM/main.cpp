#define _CRT_SECURE_NO_WARNINGS


/*!
\example get_distance.cpp Obtains distance data
\author Satofumi KAMIMURA

$Id$
*/
//Stella B1
#include "stdafx.h"
#include "SerialPort.h"
#include "StellaB1.h"
#include <windows.h>
#include <math.h>

//Laser Scanner
#include "Laser.hpp"

#include <thread>
#include <opencv2/opencv.hpp>


using namespace qrk;
using namespace std;
using namespace cv;
#pragma warning (disable : 4996)

//opencv gui â����
void window() {
	Mat image = Mat::zeros(300, 300, CV_8UC3);	// ����, ���� ���� 400, 400 �������� ���� �̹����� �����.
	image = Scalar(255, 255, 255);				// �̹��� ������� �ʱ�ȭ
	while (1) {
		int cnt = 0;
		for (int i = 0; i < 300; ++i) {
			for (int j = 0; j < 300; ++j) {
				if (MAP::map[i][j] != 0) {
					ellipse(image, Point(i, j), Size(2, 2), 0, 0, 360, Scalar(0, 0, 0), 1, 8);
					++cnt;
				}
			}
		}
		imshow("result 1", image);
		printf("%d�� ���\n", cnt);
		waitKey(500);
	}
}


int main(int argc, char *argv[])
{	//opencv gui thread ����
	thread t1(window);
	t1.detach();


	// ������ ��ĳ�� ���� ����
	Connection_information information(argc, argv);

	// Stella B1 ��������
	CSerialPort *_rc = new CSerialPort();
	_rc->Open("COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	_rc->SetTimeout(100, 100, 10);

	CStellaB1 *_sg = new CStellaB1(_rc);







	// ������ ����
	char name[80] = "";
	char image[80] = "image";
	char* bmp = ".bmp";
	char buffe[10];
	char* buffer = buffe;

	// ���ڶ� ����
	char state;
	float left, right;
	double wheel_distance = 75.2;
	double robot_x, robot_y, robot_theta;
	double ds;


	// ���ڶ� �̴ϼȶ�����
	_sg->Init();
	_sg->Reset();
	_sg->GetState(&state);
	_sg->GetPosition(&left, &right);


	// Connects to the sensor
	Urg_driver urg;
	if (!urg.open(information.device_or_ip_name(),
		information.baudrate_or_port_number(),
		information.connection_type())) {
		std::cout << "Urg_driver::open(): " << information.device_or_ip_name() << ": " << urg.what() << endl;
		Sleep(10000);
		return 1;
	}

	// Gets measurement data
#if 1
	// Case where the measurement range (start/end steps) is defined
	// Laser ���� ���� ���� 150��, step 143 // position max 4000
	urg.set_scanning_parameter(urg.deg2step(-75), urg.deg2step(+75), 3);
#endif
	enum { Capture_times = 10 };
	urg.start_measurement(Urg_driver::Distance, Urg_driver::Infinity_times, 0);	// ���� ����

	std::ofstream LaserFile("laser.txt");	// ������ ������¿�





											//for (int i = 0; i < Capture_times; ++i) {
											//while(1) {
	for (int i = 0; i < 30; ++i) {
		vector<long> data;
		long time_stamp = 0;

		// ���ڶ� �ӵ� ����
		_sg->Velocity(20, 20);
		Sleep(1000);
		_sg->Velocity(0, 0);



		if (!urg.get_distance(data, &time_stamp)) {		//laser ����
			cout << "Urg_driver::get_distance(): " << urg.what() << endl;
			Sleep(10000);
			return 1;
		}


		//print_data(urg, data, time_stamp);

		_itoa(i, buffe, 10);
		strcat(name, image);
		strcat(name, buffer);
		strcat(name, bmp);

		// Get Mobile Robot Position
		// data ��� �� ����
		_sg->GetState(&state);
		_sg->GetPosition(&left, &right);


		//// meter������ cm������ ��ȯ
		//left *= 1000;
		//right *= 1000;

		// ���� �̵��Ÿ� left�� right�� x, y, theta ���
		ds = (right + left) / 2;
		robot_theta = (right - left) / (wheel_distance);
		robot_x = ds*cos(robot_theta / 2);
		robot_y = ds*sin(robot_theta / 2);

		Position.x = ds*cos(robot_theta / 2);
		Position.y = ds*sin(robot_theta / 2);
		Position.theta = (right - left) / (wheel_distance);


		printf("\nrobot position, theta\n");
		printf("robot_x = %f, robot_y = %f, robot_theta = %f\n", robot_x, robot_y, robot_theta);
		printf("\n%1d  %10.1f  %10.1f \n", (int)state, (double)left, (double)right);



		//data ���(bmp���Ϸ� ���)
		print_data(urg, data, time_stamp, name);	//laser�� �޾ƿ�
		std::cout << "lasersensor ���� : " << i << std::endl;
		std::cout << "Position x : " << Position.x << "\nPosition y : " <<
			Position.y << "\nPosition theta : " << Position.theta << std::endl;
		Sleep(1000);
		name[0] = '\0';
		cout << "\n" << endl;


		//Position ��ȭ BMP���� ����Test
		//Position.x += 10;

	}

	for (int y = 0; y < 10000; ++y) {
		LaserFile << laser[y].num << " " << laser[y].x << " " << laser[y].y << std::endl;
	}
	LaserFile.close();


	printf("---------------------------------------------------------------------\n");
	printf("finished, insert any key to end\n");
	printf("---------------------------------------------------------------------\n");
	_sg->Velocity(00, 00);
	urg.close();
	getchar();

#if defined(URG_MSC)
	getchar();
#endif
	return 0;
}
