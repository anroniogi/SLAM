#define _CRT_SECURE_NO_WARNINGS

#include "SLAM.hpp"
std::mutex mtx;

//stella 변수
CSerialPort *_rc;
CStellaB1 *_sg;
char state;
int turn_ㅣeft= 0 ;



// Mobile Robot Position, 단위는 cm
struct Position {
	long x = 0, y = 0, theta = 0;
} Position;

namespace MAP {
	int map[300][300] = { 0, };
}
cv::Mat image;
struct Position landmark;
struct Position prevPosition;

// map의 edge 검출 저장용
cv::vector<cv::Point2f> corners;

// 레이저 변수
qrk::Urg_driver urg;
long time_stamp = 0;
long time_before = 0;
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
	image = cv::Mat::zeros(300, 300, CV_8UC3);	// 세로, 가로 각각 400, 400 사이즈의 검정 이미지를 만든다.
	//image = cv::Scalar(255, 255, 255);			// 이미지 흰색으로 초기화
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
		
		//printf("%d개 출력\n", cnt);
		cv::waitKey(100);
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

void laserThread(int argc, char *argv[]) {
	while (1) {
		getLaserData(argc, argv);
	}
	//Sleep(100);
}

void initialize(int argc, char *argv[]) {
	//landmark 선언
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
	//opencv gui thread 선언
	std::thread t1(window);
	t1.detach();
	Sleep(500);
	//laserData 받아오는 thread 생성
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
	//구현해야됨
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
			printf("전방에 장애물!\n");
			_sg->TurnLeft();
			//다시 직진해야되나?
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
	//Sleep(1000);		//lasesr 측정시간을 줘야하나??
}

void drawMap() {
	if (Position.x > -3.0 && Position.y > -3.0) {
		if (sqrt((Position.x - prevPosition.x) ^ 2 + (Position.y - prevPosition.y) ^ 2) > 400) {
			printf("포지션 재측정\n");
			getPosition();
		}
		// bmp 파일 생성
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


				//laser scanner 측정위치 x,y 계산
				double radian = urg.index2rad(i);
				long x = static_cast<long>(l * cos(radian));
				long y = static_cast<long>(l * sin(radian));
				//std::cout << i << " : (" << x << ", " << y << ")" << std::endl;

				laser[cnt].num = i;
				laser[cnt].x = x+6000;
				laser[cnt].y = y;
				++cnt;

				//mobile robot의 Position x, y만큼 장애물위치 이동
				tempx = floor(x + 6000 + Position.x);
				tempy = floor(y + Position.y);

				//mobile robot의 theta만큼 장애물위치 회전
				/*
				tempxx = (int)((tempx - Position.x)*cos(Position.theta) - (int)(tempy - Position.y)*sin(Position.theta));
				tempyy = (int)((tempx - Position.x)*sin(Position.theta) - (int)(tempy - Position.y)*cos(Position.theta));
				*/
				tempxx = tempx * (cos(Position.theta)) - tempy * (sin(Position.theta));
				tempyy = tempx * (sin(Position.theta)) + tempy * (cos(Position.theta));
				
				tempxx = tempxx / 50;
				tempyy = tempyy / 50;

				// 배열에 대입
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
		if (!urg.get_distance(data, &time_stamp)) {		//laser 측정
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
	//Position.theta = (right - left) / (wheel_distance);

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
	data1 = data[71];
	data2 = data[74];
	data3 = data[77];

	printf("레이저스캐너 left = %ld, center = %ld, right = %ld, time = %ld, data size = %d\n", data1, data2, data3, time_stamp, data.size());
	data.clear();

}

