#pragma once
#define _CRT_SECURE_NO_WARNINGS


//Stella B1
#include "stdafx.h"
#include "SerialPort.h"
#include "StellaB1.h"
#include <windows.h>
#include <math.h>

//Laser Scanner
#include "Laser.hpp"
#include <thread>
#include "Urg_driver.h"
#include "Connection_information.h"
#include "math_utilities.h"

//GUI
#include <opencv2/opencv.hpp>
#include <thread>


void initialize(int argc, char *argv[]);
void window();
void localize();
void getPosition();
void getLaserData();
void move();
void mapping();
void find_corner();

