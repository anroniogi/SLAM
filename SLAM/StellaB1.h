#pragma once

#include "SerialPort.h"

class CStellaB1 {
public:
	CStellaB1 (CSerialPort *serial);
	~CStellaB1 ();

	bool Velocity (int leftMotor, int rightMotor);
	bool Stop (int mode = 1); // mode: 1 - Motor Free, 2 - 긴급 정지, 3 - 감속 정지
	bool Reset ();
	bool Init ();
	bool GetState (char *state);
	bool GetPosition (float *leftMotor, float *rightMotor);

	const char *GetStateString (char state);
	const char *GetErrorString (char errorCode);

private:
	int WritePacket (unsigned char *command, int length);
	int ReadPacket (unsigned char *response, int length, unsigned long timeout = 100);
	int FindAndMove (unsigned char *response, int n, unsigned char command);

private:
	CSerialPort *_serial;
};
