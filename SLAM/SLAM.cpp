#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//stella ����
CSerialPort *_rc;
CStellaB1 *_sg;
char state;

// Mobile Robot Position
struct Position {
	long x = 0, y = 0, theta = 0;
} Position;

namespace MAP {
	int map[300][300] = { 0, };
}
cv::Mat image;
struct Position landmark;

// map�� edge ���� �����
cv::vector<cv::Point2f> corners;

// ������ ����
qrk::Urg_driver urg;
long time_stamp = 0;
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
	//cv::Mat image = cv::Mat::zeros(300, 300, CV_8UC3);	// ����, ���� ���� 400, 400 �������� ���� �̹����� �����.
	image = cv::Mat::zeros(300, 300, CV_8UC3);	// ����, ���� ���� 400, 400 �������� ���� �̹����� �����.
	image = cv::Scalar(255, 255, 255);				// �̹��� ������� �ʱ�ȭ
	while (1) {
		int cnt = 0;
		for (int i = 0; i < 300; ++i) {
			for (int j = 0; j < 300; ++j) {
				if (MAP::map[i][j] != 0) {
					ellipse(image, cv::Point(i, j), cv::Size(2, 2), 0, 0, 360, cv::Scalar(0, 0, 0), 1, 8);
					++cnt;
				}
			}
		}
		imshow("result 1", image);
		
		//printf("%d�� ���\n", cnt);
		cv::waitKey(500);
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


void initialize(int argc, char *argv[]) {
	landmark.x = 100;
	landmark.y = 100;
	landmark.theta = 0;

	//opencv gui thread ����
	std::thread t1(window);
	t1.detach();

	stellaInitialize();

	laserScannerInitialize(argc, argv);


}

void predict() {

	
}

void update() {


}


void localize() {
	
	getPosition();
	getLaserData();

	//�����ؾߵ�
	predict();
	update();
	
}

void move() {
	//_sg->Run();
	_sg->Velocity(20, 20);
}

void drawMap() {

	// bmp ���� ����
	bmpinfoheader[25] = (unsigned char)(resolution);      //horizontal resolutions
	bmpinfoheader[29] = (unsigned char)(resolution);      //vertical resolutions
	pfile = fopen("map.bmp", "wb");
	fwrite(bmpfileheader, 1, 14, pfile);
	fwrite(bmpinfoheader, 1, 40, pfile);

	int cnt = 0;

	size_t data_n = data.size();
	for (size_t i = 0; i < data_n; ++i) {
		long l = data[i];
		if ((l <= min_distance) || (l >= max_distance)) {
			continue;
		}

		double radian = urg.index2rad(i);
		long x = static_cast<long>(l * cos(radian));
		long y = static_cast<long>(l * sin(radian));
		//std::cout << i << " : (" << x << ", " << y << ")" << std::endl;

		laser[cnt].num = i;
		laser[cnt].x = x;
		laser[cnt].y = y + 4000;
		++cnt;

		//mobile robot�� Position x, y��ŭ ��ֹ���ġ �̵�
		tempx = floor(x / 50) + Position.x;
		tempy = floor((y + 4000) / 50) + Position.y;

		//mobile robot�� theta��ŭ ��ֹ���ġ ȸ��
		tempx = tempx * (cos(Position.theta)) - tempy * (sin(Position.theta));
		tempy = tempx * (sin(Position.theta)) + tempy * (cos(Position.theta));


		// �迭�� ����
		MAP::map[tempx][tempy] += 1;
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

	findCorner();

	correction();

	drawMap();
		
}

void getLaserData() {

	if (!urg.get_distance(data, &time_stamp)) {		//laser ����
		std::cout << "Urg_driver::get_distance(): " << urg.what() << std::endl;
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

	Position.x = ds*cos(robot_theta / 2);
	Position.y = ds*sin(robot_theta / 2);
	Position.theta = (right - left) / (wheel_distance);

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
	data1 = data[0];
	data2 = data[71];
	data3 = data[140];

	printf("��������ĳ�� left = %ld, center = %ld, right = %ld, time = %ld\n", data1, data2, data3, time_stamp);

}

