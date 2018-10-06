#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"

//stella 변수
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

// map의 edge 검출 저장용
cv::vector<cv::Point2f> corners;

// 레이저 변수
qrk::Urg_driver urg;
long time_stamp = 0;
long min_distance; 	//최소거리
long max_distance;	//최대거리
long tempx, tempy;
std::vector<long> data;

struct Laser {
	size_t num;
	long x, y;
} laser[143];

// BMP파일 생성을 위한 선언들...
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




//opencv gui 창선언
void window() {
	//cv::Mat image = cv::Mat::zeros(300, 300, CV_8UC3);	// 세로, 가로 각각 400, 400 사이즈의 검정 이미지를 만든다.
	image = cv::Mat::zeros(300, 300, CV_8UC3);	// 세로, 가로 각각 400, 400 사이즈의 검정 이미지를 만든다.
	image = cv::Scalar(255, 255, 255);				// 이미지 흰색으로 초기화
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
		
		//printf("%d개 출력\n", cnt);
		cv::waitKey(500);
	}
}

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

void laserScannerInitialize(int argc, char *argv[]) {

	// LaserScanner 연결
	qrk::Connection_information information(argc, argv);
	if (!urg.open(information.device_or_ip_name(),
		information.baudrate_or_port_number(),
		information.connection_type())) {
		std::cout << "Urg_driver::open(): " << information.device_or_ip_name() << ": " << urg.what() << std::endl;
		Sleep(10000);
	}

	// Laser 측정 각도 설정 150도, step 143 // position max 4000
	urg.set_scanning_parameter(urg.deg2step(-75), urg.deg2step(+75), 3);
	enum { Capture_times = 10 };
	urg.start_measurement(qrk::Urg_driver::Distance, qrk::Urg_driver::Infinity_times, 0);	// 측정 시작

	min_distance = urg.min_distance();
	max_distance = urg.max_distance();

}


void initialize(int argc, char *argv[]) {
	landmark.x = 100;
	landmark.y = 100;
	landmark.theta = 0;

	//opencv gui thread 선언
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

	//구현해야됨
	predict();
	update();
	
}

void move() {
	//_sg->Run();
	_sg->Velocity(20, 20);
}

void drawMap() {

	// bmp 파일 생성
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

		//mobile robot의 Position x, y만큼 장애물위치 이동
		tempx = floor(x / 50) + Position.x;
		tempy = floor((y + 4000) / 50) + Position.y;

		//mobile robot의 theta만큼 장애물위치 회전
		tempx = tempx * (cos(Position.theta)) - tempy * (sin(Position.theta));
		tempy = tempx * (sin(Position.theta)) + tempy * (cos(Position.theta));


		// 배열에 대입
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

	if (!urg.get_distance(data, &time_stamp)) {		//laser 측정
		std::cout << "Urg_driver::get_distance(): " << urg.what() << std::endl;
	}

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

	cvtColor(image, gray, CV_RGB2GRAY);	// bmp파일

	cv::Mat cvCorner = image.clone();

	int corner = 10;		// 찾을 코너 갯수
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

	printf("레이저스캐너 left = %ld, center = %ld, right = %ld, time = %ld\n", data1, data2, data3, time_stamp);

}

