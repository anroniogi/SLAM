# CEKF_SLAM 클래스
---
## 위치
EKF_SLAM.h에 존재

## 인자

double x
double y
double theta

## public

### 메소드

predict(double sl double sr)
update(double r, double a)

### 변수 선언
dMatrixX, P;
-> typedef techsoft::matrix<double>	dMatrix;
int selX;



## private

### 메소드
double correspondence_test(double r, double a);
void Feature_initialization(double r, double a);


### 변수 선언

dMatrix _ S, _ v, _ H;


---

# CDialog
- MS에서 지원하는 C++ library에 있음
- 클래스 형식
- afxwin.h 에 있음

- CWnd::DoDataExchange()
	- VC에 존재
	- 인자로 cDataExchange* pDX
		- CDataExchange클래스
			- / Attributes
				public:
				BOOL m_bSaveAndValidate;   // TRUE => save and validate data
				CWnd* m_pDlgWnd;           // container usually a dialog

				// Operations (for implementors of DDX and DDV procs)
				HWND PrepareCtrl(int nIDC);     
				HWND PrepareEditCtrl(int nIDC);
				void Fail();                    // will throw exception
				CDataExchange(CWnd* pDlgWnd, BOOL bSaveAndValidate);

---

# 지도생성부분
rpc_client_init()
- TCP/IP 프로토콜 사용 선언 부분
	- TCP/IP network 사용부
	- port 선언

load_feature2D()

# x	y	r
# x - feature's x position
# y - feature's y position
# r - feature's radius

  1.	3.	0.2
  14.	6.	0.2
  5.	5.	0.2
  11.	9.	0.2
  6.	8.	0.2
  10.	1.	0.2
  12.	5.	0.2
  3.	1.	0.2
  7.	2.	0.2
  10.	3.	0.2
  2.	8.	0.2
  9.	6.	0.2
  13.	9.	0.2
  3.	4.	0.2
  15.	3.	0.2

---
# OnBNClickedBtReset
void SetRobotPosition(6.1.0);
- x,y,theta를 인자로 받음
- NdrClientCall2(){
	PMIDL_STUB_DESC         pStubDescriptor,
		- typedef const MIDL_STUB_DESC  * PMIDL_STUB_DESC;


  PFORMAT_STRING          pFormat,
}

void GetRobotPosition(){
	double * x
	double * y
	double * theta
}

	double robot_x, robot_y, robot_theta;
	GetRobotPosition(&robot_x, &robot_y, &robot_theta);

	_ slam = new CEKF_SLAM(robot_x, robot_y, robot_theta);

	GetMotorPosition(&_ plm, &_ prm);

lm,rm




# Fast SLAM
## GetMap
## GetPosition
## RestartSLAM  
## GetCell)ccProb
## onStart
## LoadProperty
## DoSLAMOneCycle
- Localize
- Mapping

Prediction Parameter
OPROS::MobilePositionData(class)
- previousPosition;
bool doPrediction //


OPRoS::MobileVelocityData
current MobileVelocityData
- 현재 속력
previous MobileVelocityData
- 이전 속력
stepVelocity

autoVelocity



변수
wheel_radius: 바퀴 반지름
lm rm : 좌우 모터의 속력을 받아와서 저장할 변수
plm prm : 이전 속력 값 (현재 속력을 저장하여 다음 동작후에 dlm,drm값을 구하는데 사용)
dlm drm : (lm ? plm) * wheel_radius으로 현재 위치를 추정
## 동작
1. GetMotorPosition()함수를 사용하여 lm과dm에 모터의 속력을 저장
2. 가져온 현재 속력에 이전 속력을 빼고 바퀴사이 거리를 곱함
   = (llm ? plm) * wheel_radius
3. predict()함수에 dlm,drm을 매개변수로 전달
- 변수
    axle_length : 이동길이 ?
    n 		: (X의 행개수 ?3 )/2
    ds 		: dlm과 drm을 더하고 2로 나눔 = (dlm+drm)/2
    dtheta 	: (현재 오른쪽 모터 속력 ? 현재 왼쪽 모터 속력)/이동길이
            	   = (dlm-drm)/_ axel_length
    dMatrix Fv: 3,3+2*n을 저장함  
    th 		: X행렬의 2,0에 dtheta의 절반을 더함
    da 		: ds를 이동길이의 두배로 나눈다
    cos_th 	: theta의 cos값
    sin_th 	: theta의 sin값
# 하드웨어 초기화
- BPS가 가장 중요
usbrs232
- 드라이버 존재
- API사용
## 다른 방법
OPRoS화
stella map opros
###deviceAPI
- stellab가 deviceapi다
	- headear+ lib,dll 만제공
- 보안,기밀 FROM 사용자
	- 연구개발자는 거기에 내용 추가해야하니까
- 하나의 장치가 여러 응용에서 사용할 수 있게 함
- version상관없이 사용할수 있도로 추상화

COMMUNICATION VS WINDOW
		- 운영체제에 종속적 인지 종속적이지 않는지

### STellaB1
initialize
SetStellaB1Profile
initializeSerial
MobileController class
GetOdometry
SetVelocity
SetPosition
GetPosition
MobileComp에서 veclcity 넣는 함수호출Q
- opros_any를 찾을 수 없음
StellaB1에서 velocity 넣는 부분 찾기
MobileVelocityData tVel
opros_any * pdata
velocity.pop();
Mobile Component code에서
getPropertyMap()
- 컴포넌트class cdl에서 불러옴
Component profile의 값으로 Map 채우기

# SerialWindwos.cpp
## 생성자
SerialWindows(string portName, unsigned long timeOut, unsigned long baudRate, char dataBits, char stopBits, char parity, char flowControl);
			virtual ~SerialWindows(void);
CloseSerial
OpenSerial
- 시리얼 개방
ReadSerial
- 파싱할때 쓰는것?
SetParameter
- 값넣어주는것
WriteSerial
- 시리얼 저장할때 쓰는 것 ?

# StellaB1.cpp
ControlInit()
Controle
ControlVelocity()
ControlVelocityleft()
ControlVelocityright()
SetVelocityAcceleration()
SetVelocityDeacceleration()
RegisterUart
UnregisterUart
WriteCommand
ReadResponse
Request
RequestResponse
struct DCB{
		dword
		word
		byte
		char
		}
# slam 진행 사항

OPRoS Device API 사용하기
		.dll에서 읽기
		stellaB1을 초기화해서 사용 하기
Serial Communication
		- 오프로스에서 시리얼 통신하기 위해 사용하는 소스
SerialWindows
		- winodws에서 시리얼 통신하기 위한소스
Stella
- RS232통신가능
- usb사용
- cp2102칩 사용
- gnd,rx,tx 존재함

3개 전부다 있어야 stellaB1동작

알고리즘
Open serial
생성
velocity
연산
packet write
OpenSerial
- setParameter
데이터 전송


- 새로 만들 생각이면 이해하고 있어야함
- 사용 : 포팅,dll
- 구현 : 완성된 거 하려면 원래 코드로 하기

하드웨어만 기다리면 평생 안됨


## uart
- 상태정보

# 장비 확인

- USB 드라이브 설치 확인

ServicePort 로 값주기
DataPort 로 값주기
# 관련기술
USB 232 serial COMMUNICATION
- 통신은 bps 가 중요
- bps, startbit stop bit parity bit
# 환경설정
Low레벨이 아님
하드웨어 -> library
소프트웨어도 알아야 함
나머진 돈으로 하는 것 (외주)


stx
C(Command) 구동 명령어
S(Setting) 데이터 설정하는 코드
G(Get) 데이터 읽어 오는 코드

문자 대문자로
명령간 delay 시간 5~ 10ms
모터속도 rad/sec rad/sec^2



## 속도
- 모터기준 : 270 rad/sec
- 휠 기준  : 5.2km/h
위치명령
## 회전각도
이동거리
이동 시간
## 정지 명령



## Mapping
Grid기반의 Mapping
가정,예측값 vs 실제값을 보정하여 지도생성
특징점 추출해서 랜드마크를 결정
임의의 랜드마크 를 규정할수 없음
- 이름 내용 형식
- 사용 방식
- map자체는 string 으로 받아서 캐스팅하여 사용


# 용어
Deacceleration 감속
## 시리얼통신 알고리즘
		- 포트연결
		- 명령어 보내기
		- 연결해제


## ????
stella에서 준것과 OPRoS로 바꾼것 기준
StesllaB1 생성자 사용법
SerialWindow 생성자 사용법
Scale Factor
