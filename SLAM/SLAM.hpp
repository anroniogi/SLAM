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
void stellaInitialize();
void laserScannerInitialize(int argc, char *argv[]);
void window();
void localize(int argc, char *argv[]);
void getPosition();
void getLaserData(int argc, char *argv[]);
void move();
void mapping();
void findCorner();
void correction();
void predict();
void update();
void drawMap();
void findRoute();
void turnLeft();
