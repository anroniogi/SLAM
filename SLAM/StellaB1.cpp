#include "stdafx.h"
#include "StellaB1.h"
#include <math.h>
#include <string.h>
#include <windows.h>
#include <assert.h>

static const unsigned char STX = 0x02;
static const unsigned char ETX = 0x03;

inline int bound (int v, int lo, int hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

CStellaB1::CStellaB1 (CSerialPort *serial)
: _serial(serial)
{
	_serial->Flush ();
}

CStellaB1::~CStellaB1 ()
{
}

bool CStellaB1::Velocity (int leftMotor, int rightMotor)
{
	float a, b;
	a = leftMotor;
	b = rightMotor;
	leftMotor = bound (leftMotor, -999, 999);
	rightMotor = bound (rightMotor, -999, 999);

	unsigned char leftSign = (leftMotor >= 0) ? 'F' : 'B';
	unsigned char rightSign = (leftMotor >= 0) ? 'F' : 'B';

	leftMotor = abs(leftMotor);
	rightMotor = abs(rightMotor);

	unsigned char cmd_left[] = { STX, 
		'C', 'V', 'L', 
		leftSign, 
		(leftMotor/100)%10 + '0', 
		(leftMotor/10)%10 + '0', 
		(leftMotor/1)%10 + '0', 
		ETX };
	WritePacket (cmd_left, sizeof(cmd_left));

	unsigned char cmd_right[] = { STX, 
		'C', 'V', 'R', 
		rightSign, 
		(rightMotor/100)%10 + '0', 
		(rightMotor/10)%10 + '0', 
		(rightMotor/1)%10 + '0', 
		ETX };
	WritePacket (cmd_right, sizeof(cmd_right));

	return true;
}

bool CStellaB1::Stop (int mode)
{
	assert (mode == 1 || mode == 2 || mode == 3);

	unsigned char cmd[] = { STX, 'C', 'S', 'T', 'O', 'P', mode + '0', ETX };
	WritePacket (cmd, sizeof(cmd));
	return true;
}

bool CStellaB1::Reset ()
{
	unsigned char cmd[] = { STX, 'C', 'R', 'E', 'S', 'E', 'T', 'A', ETX };
	WritePacket (cmd, sizeof(cmd));
	return true;
}

bool CStellaB1::Init ()
{
	unsigned char cmd[] = { STX, 'C', 'I', 'N', 'I', 'T', ETX };
	WritePacket (cmd, sizeof(cmd));
	return true;
}

bool CStellaB1::GetState (char *state)
{
	unsigned char cmd[] = { STX, 'G', 'S', 'T', 'A', 'T', 'E', ETX };
	WritePacket (cmd, sizeof(cmd));

	unsigned char res[3] = { 0, };
	int n_res = ReadPacket (res, sizeof(res));
	if (n_res == sizeof(res) && res[0] == STX && res[2] == ETX) {
		*state = res[1] - '0';
		return true;
	}
	else {
		return false;
	}
}

bool CStellaB1::GetPosition (float *leftMotor, float *rightMotor)
{
	unsigned char cmd[] = { STX, 'G', 'P', 'O', 'S', 'I', 'T', 'I', 'O', 'N', 'A', ETX };
	WritePacket (cmd, sizeof(cmd));

	unsigned char res[16] = { 0, };
	int n_res = ReadPacket (res, sizeof(res));
	if (n_res == sizeof(res) && res[0] == STX && res[15] == ETX) {
		*leftMotor = (float)atof ((char *)&res[2]);
		*rightMotor = (float)atof ((char *)&res[9]);
		if (res[1] == 'B') *leftMotor = -*leftMotor;
		if (res[8] == 'B') *rightMotor = -*rightMotor;
		return true;
	}
	else {
		return false;
	}
}

const char *CStellaB1::GetStateString (char state)
{
	switch (state) {
	case 0: return "정상 상태";
	case 1: return "오른쪽 휠 과부하 상태";
	case 2: return "왼쪽 휠 과부하 상태";
	case 3: return "양쪽 휠 과부하 상태";
	case 4: return "과전압 상태(인가전압 15V 이상)";
	case 5: return "저전압 상태(인가전압 10V 이하)";
	case 6: return "목표위치 도달 상태";
	case 7: return "휠 회전 중인 상태";
	default: return "알려지지 않은 상태";
	}
}

const char *CStellaB1::GetErrorString (char errorCode)
{
	switch (errorCode) {
	case  8: return "입력한 위치 지령이 모터 한계 속도값을 넘어서는 명령입니다.";
	case  9: return "명령 문자열에 문자가 더 들어갔거나 덜 들어갔습니다.";
	case 10: return "명령 문자열이 올바르지 않습니다.";
	case 11: return "명령 문자열 중 방향 데이터가 올바르지 않습니다.";
	case 12: return "입력 데이터가 범위를 벗어났습니다.";
	default: return "알려지지 않은 에러 코드입니다.";
	}
}


int CStellaB1::WritePacket (unsigned char *command, int length)
{
	for (int i=0; i<length; ++i) {
		_serial->Write ((char *)&command[i], 1);
		Sleep (10);
	}
	return length;
}

int CStellaB1::ReadPacket (unsigned char *response, int length, unsigned long timeout)
{
	int n_recv = 0;

	timeout += GetTickCount ();

	while (0 < length) {
		int n = _serial->Read ((char *)response, length);
		if (0 < n) {
			if (n_recv == 0 && response[0] != STX) {
				n = FindAndMove (response, n, STX);			
			}
			response += n;
			length -= n;
			n_recv += n;

			if (0 < n_recv && response[-1] == ETX) {
				break;
			}
		}
		else if (n == 0) {
			if (timeout < GetTickCount()) {
				break;
			}
		}
		else {
			return -1;
		}
	};
	return n_recv;
}

int CStellaB1::FindAndMove (unsigned char *response, int n, unsigned char command)
{
	for (int i=0; i<n; ++i) {
		if (response[i] == command) {
			memmove (&response[0], &response[i], n-i);
			return n-i;
		}
	}
	// response 안에 command가 없다.
	// 받은 데이터를 모두 무시한다.
	return 0;
}
