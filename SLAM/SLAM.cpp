#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"
std::mutex mtx;

//stella ����
CSerialPort *_rc;
CStellaB1 *_sg;
char state;
int turn_��eft= 0 ;



// Mobile Robot Position, ������ cm
struct Position {
	long x = 0, y = 0, theta = 0;
} Position;

namespace MAP {
	int map[300][300] = { 0, };
}
cv::Mat image;
struct Position landmark;
struct Position prevPosition;

// map�� edge ���� �����
cv::vector<cv::Point2f> corners;

// ������ ����
qrk::Urg_driver urg;
long time_stamp = 0;
long time_before = 0;
long min_distance; 	//�ּҰŸ�
long max_distance;	//�ִ�Ÿ�
long tempx, tempy;
std::vector<long> data;

struct Laser {
	size_t num;
	long x, y;
} laser[143];

// BMP���� ������ ���� �����...
std::FILE *pfile;

unsigned int width = 200, height = 200;
unsigned int bpp = 24;
unsigned int rowsize = ((bpp*width + 31) / 32) * 4;
unsigned int area = width*height;
unsigned int sizeOfArr = rowsize *height;
unsigned int fileSize = 54 + sizeOfArr;

unsigned int resolution = 2835;//see wiki. 72dpi* 39.3701 in/meter

unsigned char bmpfileheader[14] = { 'B','M',   0,0,0,0 ,   0,0,0,0 ,   54,0,0,0 };
unsigned char bmpinfoheader[40] = { 40,0,0,0,  width,0,0,0,   height,0,0,0 ,   1,0,24,0 };

union RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};
RGB white[3] = { 255, 255, 255 };
RGB black[3] = { 0, 0, 0 };




//opencv gui â����
void window() {
	image = cv::Mat::zeros(300, 300, CV_8UC3);	// ����, ���� ���� 400, 400 �������� ���� �̹����� �����.
	//image = cv::Scalar(255, 255, 255);			// �̹��� ������� �ʱ�ȭ
	while (1) {
		int cnt = 0;
		for (int i = 0; i < 300; ++i) {
			for (int j = 0; j < 300; ++j) {
				if (MAP::map[i][j] != 0) {
					//ellipse(image, cv::Point(i, j), cv::Size(2, 2), 0, 0, 360, cv::Scalar(0, 0, 0), 1, 8);
					cv::line(image, cv::Point(floor(Position.x/50), floor((Position.y+6000)/50)), cv::Point(i, j), cv::Scalar(255, 255, 255), 1, 8, 0);
					++cnt;
				}
			}
		}
		imshow("result 1", image);
		
		//printf("%d�� ���\n", cnt);
		cv::waitKey(100);
	}
}

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

void laserScannerInitialize(int argc, char *argv[]) {

	// LaserScanner ����
	qrk::Connection_information information(argc, argv);
	if (!urg.open(information.device_or_ip_name(),
		information.baudrate_or_port_number(),
		information.connection_type())) {
		std::cout << "Urg_driver::open(): " << information.device_or_ip_name() << ": " << urg.what() << std::endl;
		Sleep(10000);
	}

	// Laser ���� ���� ���� 150��, step 143 // position max 4000
	urg.set_scanning_parameter(urg.deg2step(-75), urg.deg2step(+75), 3);

	enum { Capture_times = 10 };
	urg.start_measurement(qrk::Urg_driver::Distance, qrk::Urg_driver::Infinity_times, 0);	// ���� ����

	min_distance = urg.min_distance();
	max_distance = urg.max_distance();

}

void laserThread(int argc, char *argv[]) {
	while (1) {
		getLaserData(argc, argv);
	}
	//Sleep(100);
}

void initialize(int argc, char *argv[]) {
	//landmark ����
	landmark.x = 100;
	landmark.y = 100;
	landmark.theta = 0;

	Position.x = 0;
	Position.y = 0;
	Position.theta = 0;

	stellaInitialize();
	Sleep(500);
	laserScannerInitialize(argc, argv);
	Sleep(500);
	//opencv gui thread ����
	std::thread t1(window);
	t1.detach();
	Sleep(500);
	//laserData �޾ƿ��� thread ����
	std::thread t2(laserThread, argc, argv);
	t2.detach();
	Sleep(500);
}

void predict() {

	
}

void update() {


}


void localize(int argc, char *argv[]) {
	//_sg->Velocity(0, 0);
	//prevPosition = Position;
	getPosition();
	//getLaserData(argc, argv);
	printf("position : %ld, %ld, %ld\n", Position.x, Position.y, Position.theta);
	//�����ؾߵ�
	predict();
	update();
	
}


void move() {
	mtx.lock();
	//printf("data[70] = %ld, data[74] = %ld, data[78] = %ld\n", data[70], data[74], data[78]);
	if ((400 < data[70] && data[70] < 1000) || 
		(400 < data[74] && data[74] < 1000) ||
		(400 < data[78] && data[78] < 1000)) {
		if (time_before != time_stamp && time_before == 0) {
			//_sg->Velocity(0, 0);
			printf("���濡 ��ֹ�!\n");
			_sg->TurnLeft();
			//�ٽ� �����ؾߵǳ�?
			//_sg->Velocity(20, 20);
			Sleep(4000);
			Position.theta = -1.57;
			data.clear();
			_sg->Velocity(0, 0);
			printf("\n");
			_sg->Velocity(20, 20);
			time_before = time_stamp;
		}
	} else {
		_sg->Velocity(20, 20);
	}
	mtx.unlock();
	//Sleep(1000);		//lasesr �����ð��� ����ϳ�??
}

void drawMap() {
	if (Position.x > -3.0 && Position.y > -3.0) {
		if (sqrt((Position.x - prevPosition.x) ^ 2 + (Position.y - prevPosition.y) ^ 2) > 400) {
			printf("������ ������\n");
			getPosition();
		}
		// bmp ���� ����
		bmpinfoheader[25] = (unsigned char)(resolution);      //horizontal resolutions
		bmpinfoheader[29] = (unsigned char)(resolution);      //vertical resolutions
		pfile = fopen("map.bmp", "wb");
		fwrite(bmpfileheader, 1, 14, pfile);
		fwrite(bmpinfoheader, 1, 40, pfile);

		int cnt = 0;
		int tempxx, tempyy;

		size_t data_n = data.size();
		if (data_n < 142) {
			for (size_t i = 0; i < data_n; ++i) {
				long l = data[i];
				if ((l <= min_distance) || (l >= max_distance)) {
					continue;
				}


				//laser scanner ������ġ x,y ���
				double radian = urg.index2rad(i);
				long x = static_cast<long>(l * cos(radian));
				long y = static_cast<long>(l * sin(radian));
				//std::cout << i << " : (" << x << ", " << y << ")" << std::endl;

				laser[cnt].num = i;
				laser[cnt].x = x+6000;
				laser[cnt].y = y;
				++cnt;

				//mobile robot�� Position x, y��ŭ ��ֹ���ġ �̵�
				tempx = floor(x + 6000 + Position.x);
				tempy = floor(y + Position.y);

				//mobile robot�� theta��ŭ ��ֹ���ġ ȸ��
				/*
				tempxx = (int)((tempx - Position.x)*cos(Position.theta) - (int)(tempy - Position.y)*sin(Position.theta));
				tempyy = (int)((tempx - Position.x)*sin(Position.theta) - (int)(tempy - Position.y)*cos(Position.theta));
				*/
				tempxx = tempx * (cos(Position.theta)) - tempy * (sin(Position.theta));
				tempyy = tempx * (sin(Position.theta)) + tempy * (cos(Position.theta));
				
				tempxx = tempxx / 50;
				tempyy = tempyy / 50;

				// �迭�� ����
				MAP::map[tempxx][tempyy] += 1;
				//MAP::map[tempx][tempy] += 1;

			}
		}
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (MAP::map[i][j] == 0) {
					fwrite(white, sizeof(unsigned char), 3, pfile);
				}
				else fwrite(black, sizeof(unsigned char), 3, pfile);
			}
		}

		fclose(pfile);
	}

}

void correction() {
	
	int max = 0;

	for (int i = 0; i < corners.size(); ++i) {
		if (corners[i].x == landmark.x && corners[i].y == landmark.y)
		{
			printf("landmark detection\n");
		}
	}


}

void mapping() {

	//findCorner();

	//correction();

	drawMap();
		
}

void getLaserData(int argc, char *argv[]) {
	while (1) {
		mtx.lock();
		data.clear();
		time_stamp = 0;
		if (!urg.get_distance(data, &time_stamp)) {		//laser ����
			std::cout << "Urg_driver::get_distance(): " << urg.what() << std::endl;
			//_sg->Velocity(0, 0);
			urg.close();
			laserScannerInitialize(argc, argv);
			urg.get_distance(data, &time_stamp);
		}
		mtx.unlock();
		Sleep(50);
	}
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
	//Position.theta = (right - left) / (wheel_distance);

#ifdef DEBUG
	printf("\nrobot position, theta\n");
	printf("robot_x = %f, robot_y = %f, robot_theta = %f\n", robot_x, robot_y, robot_theta);
	printf("\n%1d  %10.1f  %10.1f \n", (int)state, (double)left, (double)right);
#endif

}

void findCorner(){

	cv::Mat gray;

	cvtColor(image, gray, CV_RGB2GRAY);	// bmp����

	cv::Mat cvCorner = image.clone();

	int corner = 10;		// ã�� �ڳ� ����
	goodFeaturesToTrack(gray, corners, corner, 0.01, 10);

#ifdef DEBUG
	for (int i = 0; i < corners.size(); ++i) {
		std::cout << i << " , " << corners[i] << std::endl;
	}
#endif


	for (int i = 0; i<corners.size(); i++) {
		circle(cvCorner, corners[i], 5, cv::Scalar(0, 255, 0));
	}

	//
	cv::Mat image2;

	cv::flip(image.t(), image2, 0);
	cvCorner = image2.clone();
	//
	imshow("cvCorner", cvCorner);
	cv::waitKey(1);
}

void findRoute() {

	long data1, data2, data3;
	data1 = data[71];
	data2 = data[74];
	data3 = data[77];

	printf("��������ĳ�� left = %ld, center = %ld, right = %ld, time = %ld, data size = %d\n", data1, data2, data3, time_stamp, data.size());
	data.clear();

}

