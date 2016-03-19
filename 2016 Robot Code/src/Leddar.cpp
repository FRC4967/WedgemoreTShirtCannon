/*
 * Leddar.cpp
 *
 *  Created on: Feb 3, 2016
 *      Author: Win7
 */
#define _USE_MATH_DEFINES
#include <Leddar.h>
#include <cmath>
Leddar::Leddar() :
	m_RS_232(115200)
{

}


Leddar::~Leddar() {

}


vector<Point> Leddar::GetDetections() {
	vector<Point> detections;
	unsigned bytesRecived, numDetections;

	m_RS_232.Reset();
	//sending the bytes to the leddar sensor
	char query[] = {0x01, 0x41,0xC0,0x10};
	m_RS_232.Write(query, 4);


	Wait(0.250);
	//This is causing too much of a delay on the drive

	bytesRecived = m_RS_232.GetBytesReceived();
	SmartDashboard::PutNumber("bytes recived", bytesRecived);

	//saving the data recived back from the leddar
	char response[bytesRecived];
	m_RS_232.Read(response, bytesRecived);

	if(response[0] != 0x01 || response[1] != 0x41) {
		return detections;
	}

	//putting it on the SmartDashboard so that we could see a few of the bytes we were getting back
	SmartDashboard::PutNumber("byte 0 address", response[0]);
	SmartDashboard::PutNumber("byte 1 function code", response[1]);
	SmartDashboard::PutNumber("byte 2 # of detections", response[2]);

	//store detections in arrays

	numDetections = response[2];
	unsigned short distances[numDetections];
	unsigned short amplitudes[numDetections];
	unsigned char  flags[numDetections];
	char *rawDetections = response + 3;
	Point detectionPosition = {0,0};

	for(unsigned i = 0; i < numDetections; i++) {
		distances[i] = rawDetections[i*5] | (rawDetections[i*5+1] << 8);
		amplitudes[i] = (rawDetections[i*5+2] | (rawDetections[i*5+1+2] << 8))/64;
		flags[i] = rawDetections[i*5+4];

		if(flags[i] != 0) {
			continue;
		}

		float angle = 1.4 + (flags[i]&0xF0)*2.8;
		detectionPosition.x = cos(angle*M_PI/180)*distances[i];
		detectionPosition.y = sin(angle*M_PI/180)*distances[i];

		detections.push_back(detectionPosition);

		SmartDashboard::PutNumber("distance of detection " + to_string(i+1), distances[i]);
		SmartDashboard::PutNumber("amplitude of detection " + to_string(i+1), amplitudes[i]);
		SmartDashboard::PutNumber("flags of detection " + to_string(i+1), flags[i] & 0x0F);
	}
	return detections;
}
