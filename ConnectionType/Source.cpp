#include <iostream>
#include <string>
#include <conio.h>

//connect TCP/UDP
#include <boost\asio.hpp>

//threads
#include <boost\thread.hpp>

//My
#include "Server.h"
#include "MicroEpsilon.h"

#include <opencv2\opencv.hpp>
#include <opencv2\highgui.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

typedef boost::shared_ptr<tcp::socket> socket_ptr;
typedef boost::shared_ptr<boost::thread> thread_ptr;

const int MINIMUM_SLEEP_TIME_FOR_PROFILE_OR_PICTURE = 5;

void getOneProfile(scanCONTROL &sC, const std::string &fileName)
{
	sC.GetProfileWithName(fileName);
}

int main()
{
	//setlocale(0, "");
	scanCONTROL myLaser;

	Server myServer(6008, myLaser);

	//�� ������ �������� LLT.dll � ����� � �������� ��������� ����������� �� ���������
	//scanCONTROL myLaser;

	//���� ����� ���� ������� �� ������ ������ ��� m_pLLT->SetDeviceInterface


	//�������� � ����� ������� � �� ��� �������� getLLTInfos � �����
	myLaser.start();
	myLaser.getLLTInfos();

	//getOneProfile(myLaser, "Test.txt");

	//myLaser.getSomeInfo();

	//myLaser.GetProfiles_Poll();
	//myLaser.VideoMode();
	/*myLaser.GetProfiles_Poll();
	myLaser.VideoMode();

	Sleep(MINIMUM_SLEEP_TIME_FOR_PROFILE_OR_PICTURE);
	myLaser.GetProfiles_Poll();
	myLaser.VideoMode();

	Sleep(MINIMUM_SLEEP_TIME_FOR_PROFILE_OR_PICTURE);
	myLaser.GetProfiles_Poll();
	myLaser.VideoMode();*/


	/*myLaser.VideoMode();
	myLaser.VideoMode();
	myLaser.VideoMode();*/
	
	
	//Go ��������� ����������� � ������ � ��������� ������
	//myLaser.goStreaming();
	//start ��������� � ���� �� ������
	//myLaser.startStreaming();

	std::string str;

	/*do {
		std::cout << "������� ��� ����������...\nTCP (KRL ����������)\t\tUDP (RSI ������):\n";
		std::cin >> str;

	} while (str != "TCP" && str != "UDP");*/

	//myServer.startListening("UDP");
	myServer.startListening("helloworld");


	/*thread_ptr session = thread_ptr(, 6008);*/


	_getch();
	return 0;
}