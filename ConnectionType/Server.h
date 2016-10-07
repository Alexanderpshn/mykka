#pragma once

#include <string>
#include <iostream>
#include <vector>

#include <boost\asio.hpp>
#include <boost\shared_ptr.hpp>

#include <tinyxml2.h>

#include "MicroEpsilon.h"
#include "Points.h"


using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Server {
public:
	Server(unsigned int,scanCONTROL &);

	void startListening(const std::string &);
	
	//getdata
	//changedata
	//parse???
	
private:
	std::string ExternalData;

	unsigned short port;
	scanCONTROL &sCONTROL;

	std::string connectionType;

	void startUdpListening();
	void startTcpListening();

	void weldingEasyTest(scanCONTROL &sC);
	//enum {TCP, UDP};

	std::string getStringFromFile(const std::string &);

	void saveDataFromString(const std::string &, const std::string &);

	//Bad day bad function
	//Return topY index from profile fromscanControl
	double findTopZ(const string &);
	double findTopZ(const vector<Point> &);
	
	double findEasyAngle(std::vector<Point> &p, int direction, int topBot);

	std::vector<Point> getPointsFromFile(const std::string &);

	string changeExternalData(const std::string &, double);

	RobotPosition getRobotPos(const std::string &);

	bool isLine(vector<Point> &);

	void sred(vector<Point> &, int raz);
};