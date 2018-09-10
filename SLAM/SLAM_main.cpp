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


using namespace qrk;
using namespace std;
#pragma warning (disable : 4996)

// Mobile Robot Position
struct Position {
	long x = 0, y = 0, theta = 0;
};


int main(int argc, char *argv[])
{
	// ������ ��ĳ�� ���� ����
	Connection_information information(argc, argv);

	// Stella B1 ��������
	CSerialPort *_rc = new CSerialPort();
	_rc->Open("COM3", CBR_115200, 8, ONESTOPBIT, NOPARITY);
	_rc->SetTimeout(100, 100, 10);

	CStellaB1 *_sg = new CStellaB1(_rc);

	//SLAM����

	Position Position;

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

	// Connects to the Lasersensor
	Urg_driver urg;
	if (!urg.open(information.device_or_ip_name(),
		information.baudrate_or_port_number(),
		information.connection_type())) {
		std::cout << "Urg_driver::open(): " << information.device_or_ip_name() << ": " << urg.what() << endl;
		Sleep(10000);
		return 1;
	}
	// Stella �̴ϼȶ�����
	_sg->Init();
	_sg->Reset();

	// Gets measurement data
#if 1
	// Case where the measurement range (start/end steps) is defined
	// Laser ���� ���� ���� 150��, step 143 // position max 4000
	urg.set_scanning_parameter(urg.deg2step(-75), urg.deg2step(+75), 3);
#endif
	enum { Capture_times = 10 };
	urg.start_measurement(Urg_driver::Distance, Urg_driver::Infinity_times, 0);	// ���� ����
	std::ofstream LaserFile("laser.txt");	// ������ ������¿�
	std::ofstream PositionFile("position.txt");	// ������ ������¿�



	//for (int i = 0; i < Capture_times; ++i) {
		//while (1) {
			for (int i = 0; i < 300; ++i) {


				//Stella �̵�
				_sg->Run();

				
				//_sg->TurnLeft();

				Sleep(1000);
				//_sg->Reset();

				vector<long> data;
				long time_stamp = 0;
				printf("check\n");
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

				//Stella ���¿� ��ġ�� �޾ƿ�
				_sg->GetState(&state);
				_sg->GetPosition(&left, &right);
				cout << "Urg_driver::get_distance(): " << _sg->GetState(&state) << endl;

				// Get Mobile Robot Position
				// data ��� �� ����
				_sg->GetState(&state);
				_sg->GetPosition(&left, &right);


				//data ���(bmp���Ϸ� ���)
				//print_data(urg, data, time_stamp, name);

				std::cout << "lasersensor ���� : " << i << std::endl;
				std::cout << "Position x : " << Position.x << "\nPosition y : " <<
					Position.y << "\nPosition theta : " << Position.theta << std::endl;
				Sleep(1000);
				name[0] = '\0';
				cout << "\n" << endl;
				PositionFile << Position.x << " " << Position.y << " " << Position.theta << std::endl;


			}

			for (int y = 0; y < 10000; ++y) {
				LaserFile << laser[y].num << " " << laser[y].x << " " << laser[y].y << std::endl;
			}
			LaserFile.close();
			PositionFile.close();

			printf("---------------------------------------------------------------------\n");
			printf("finished, insert any key to end\n");
			printf("---------------------------------------------------------------------\n");

			//������� ����, ������
			_sg->Velocity(00, 00);
			urg.close();
			getchar();

#if defined(URG_MSC)
			getchar();
#endif
			return 0;
		}
