#ifndef MICROEPSILON_H_
#define MICROEPSILON_H_

#include "MicroEpsilonAdd\InterfaceLLT_2.h"
#include <iostream>
#include <conio.h>

#include "stdafx.h"

#include <boost/thread.hpp>
#include "Points.h"

using namespace std;


//for videomode
static unsigned int m_uiPacketSize = 1024;

#define MAX_INTERFACE_COUNT    5
#define MAX_RESOULUTIONS       6

typedef boost::shared_ptr<boost::thread> thread_ptr;

class scanCONTROL {
public:
	scanCONTROL() : vuiInterfaces(MAX_INTERFACE_COUNT) {}

	bool start();


	void getLLTInfos();
	void GetProfiles_Poll();
	void GetProfileWithName(const std::string &);


	//Get one picture from video
	void VideoMode();

	//TODO
	void disconnect();

	//Открывает изображение с камеры в отдельном потоке
	void goStreaming();
	void getSomeInfo();

	//Включение коннект и трансфе
	void startProfileStreaming();
	void getCurrentProfileVec(vector<Point> &);
	void stopProfileStreaming();

private:
	//Открывает изображение с камеры в одном потоке
	void startStreaming();
	bool isProfileStreaming = false;

	//Провиль при открытом потоке
	

	//!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!
	bool status = false;


	std::vector<unsigned int> vuiInterfaces;
	unsigned int uiInterfaceCount = 0;
	unsigned int uiShutterTime = 100;
	unsigned int uiIdleTime = 3900;


	//For GetLLTInfos
	void GetLLTInfos(unsigned int uiDeviceID, unsigned int uiLLTNumber);
	bool IsMeasurementRange10(TScannerType);
	bool IsMeasurementRange25(TScannerType);
	bool IsMeasurementRange50(TScannerType);
	bool IsMeasurementRange100(TScannerType);

	//Error GetLLTInfos
	void OnError(const char* szErrorTxt, int iErrorValue);

	//For GetProfile
	void DisplayProfile(double *pdValueX, double *pdValueZ, unsigned int uiResolution);
	void DisplayProfile(double *pdValueX, double *pdValueZ, unsigned int uiResolution, const std::string &fileName);
	void DisplayTimestamp(unsigned char *pucTimestamp);
	std::string Double2Str(double dValue);


	//streaming
	static CInterfaceLLT* m_pLLT;
	static unsigned int m_uiResolution;
	static TScannerType m_tscanCONTROLType;

	thread_ptr session;	
};


#endif // !MICROEPSILON_H_
