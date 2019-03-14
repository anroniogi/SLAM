#pragma once
#define _CRT_SECURE_NO_WARNINGS


//Stella B1
#include "stdafx.h"
#include "SerialPort.h"
#include "StellaB1.h"
#include <windows.h>
#include <math.h>
#include <iostream>

void stellaInitialize();
void getPosition();
void move(int left, int right);
