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
	case 0: return "���� ����";
	case 1: return "������ �� ������ ����";
	case 2: return "���� �� ������ ����";
	case 3: return "���� �� ������ ����";
	case 4: return "������ ����(�ΰ����� 15V �̻�)";
	case 5: return "������ ����(�ΰ����� 10V ����)";
	case 6: return "��ǥ��ġ ���� ����";
	case 7: return "�� ȸ�� ���� ����";
	default: return "�˷����� ���� ����";
	}
}

const char *CStellaB1::GetErrorString (char errorCode)
{
	switch (errorCode) {
	case  8: return "�Է��� ��ġ ������ ���� �Ѱ� �ӵ����� �Ѿ�� ����Դϴ�.";
	case  9: return "��� ���ڿ��� ���ڰ� �� ���ų� �� �����ϴ�.";
	case 10: return "��� ���ڿ��� �ùٸ��� �ʽ��ϴ�.";
	case 11: return "��� ���ڿ� �� ���� �����Ͱ� �ùٸ��� �ʽ��ϴ�.";
	case 12: return "�Է� �����Ͱ� ������ ������ϴ�.";
	default: return "�˷����� ���� ���� �ڵ��Դϴ�.";
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
	// response �ȿ� command�� ����.
	// ���� �����͸� ��� �����Ѵ�.
	return 0;
}
