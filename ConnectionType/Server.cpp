#include "Server.h"
#include "MicroEpsilon.h"
#include <cmath>


//test
#include <fstream>

std::string mirrorIPOC(std::string receive, std::string send)
{
	// separate IPO counter as string
	int startdummy = receive.find("<IPOC>") + 6;
	int stopdummy = receive.find("</IPOC>");
	std::string Ipocount = receive.substr(startdummy, stopdummy - startdummy);

	/*std::cout << startdummy << "   " << stopdummy << std::endl;*/

	//std::cout << "IPOCOUNT: " << Ipocount << std::endl;

	// Находим позицию вставки      
	startdummy = send.find("<IPOC>") + 6;
	stopdummy = send.find("</IPOC>");


	//Проверяем не добавлен ли IPOC одним тегом /IPOC
	int start_if_last_scob = send.find("<IPOC/>");
	if (start_if_last_scob != std::string::npos) {
		std::string str = "<IPOC>" + Ipocount + "</IPOC>";
		send.replace(start_if_last_scob, 7, str);
	}
	else {
		// remove the old value and insert the actual value
		send.replace(startdummy, stopdummy - startdummy, Ipocount);
	}

	// send back the string
	return send;
}

//Поменять инициализацию порта
Server::Server(unsigned int p, scanCONTROL &sC) : port(), sCONTROL(sC) {}


void Server::startListening(const std::string &ct)
{
	if (ct == "UDP") {
		port = 6008;
		connectionType = ct;
		startUdpListening();
	}
	else if (ct == "TCP") {
		port = 59152;
		connectionType = ct;
		startTcpListening();
	}
	else {
		std::cout << "Bad connection type, u need TCP or UDP...\nYour current connection type: " << ct << std::endl;
		port = 6008;
		connectionType = ct;
		weldingEasyTest(scanCONTROL());
	}
}

void Server::startUdpListening()
{
	std::cout << "Starting UDP connect...\n\n";

	//
	boost::asio::io_service io_service;
	/*udp::endpoint sender_endpoint_;*/


	udp::socket socket_(io_service, udp::endpoint(udp::v4(), port));

	tinyxml2::XMLDocument SendXML;
	SendXML.LoadFile("ExternalData.xml");


	//Test
	std::ifstream input("ExternalData.xml", std::ios_base::binary);
	char buffFromIfstream[1024];

	input.read(buffFromIfstream, 1024);
	int bytesFromFile = input.gcount();

	buffFromIfstream[bytesFromFile] = '\0';

	input.close();

	//Для преобразования в string
	//Вставить режим без пробелов
	//tinyxml2::XMLPrinter printer;
	//SendXML.Accept(&printer);
	//std::cout << printer.CStr();

	std::cout << "Current XML:\n\n" << buffFromIfstream;

	//Найти строку где можем поменять XYZ
	std::string xmlForChange = buffFromIfstream;

	int xStartPosInXML = xmlForChange.find("X=\"") + 3;
	int yStartPosInXML = xmlForChange.find("Y=\"") + 3;
	int zStartPosInXML = xmlForChange.find("Z=\"") + 3;

	/*int xEndPosInXML = xStartPosInXML + 6;
	int yEndPosInXML = yStartPosInXML + 6;
	int zEndtPosInXML = zStartPosInXML + 6;*/

	std::cout << "\nX = " << xmlForChange.substr(xStartPosInXML, 6) << std::endl << "Y = " << xmlForChange.substr(yStartPosInXML, 6) << std::endl
		<< "Z = " << xmlForChange.substr(zStartPosInXML, 6) << std::endl;

	std::cout << "\nChange XYZ???? Enter YES or NO: ";

	std::cout << std::endl;

	std::cout << "\nStart Listening on port " << port << std::endl;

	enum { max_length = 1024 };
	char data_[max_length];

	std::ofstream someOutputInfo("SomeOutputInfo.txt");

	int x = 0;

	while (true)
	{
		udp::endpoint sender_endpoint_;
		size_t bytes_received = socket_.receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_);
		std::cout << bytes_received << " bytes received" << std::endl;

		//Елси что то принято
		if (true) {
			//Для обработки помещаяем в str
			std::string strDataKrc(data_, bytes_received);

			//Test
			//std::cout << "Received: \n" << strDataKrc << std::endl;

			//ПРоверяем присутствует ли закрывающий тег(правильный ли XML файл)
			if (strDataKrc.find("</Rob>") == std::string::npos) {
				continue;
			}
			else {
				//Если файл в порядке, меняем IPOC
				//std::string strExternalData     = printer.CStr();
				//strExternalData				= mirrorIPOC(strDataKrc, strExternalData);
				std::string strExternalData;
				if (x < 50) {
					strExternalData = mirrorIPOC(strDataKrc, changeExternalData("y", 0.5));
					cout << x << endl;
					x++;
				}
				else strExternalData = mirrorIPOC(strDataKrc, buffFromIfstream);

				//std::cout << strDataKrc << std::endl;
				//Size??!!!
				//int sendSize = strExternalData.size();
				//sock->send(boost::asio::buffer(strStr, sendSize));

				//int sendSize = strExternalData.size();
				//const char *sendStr = strExternalData.c_str();


				//std::cout << strExternalData << std::endl;
				//std::string test(sendStr, sendSize - 1);

				//std::cout << result;

				/*socket_.send_to(boost::asio::buffer(sendStr), sender_endpoint_);*/
				int s = socket_.send_to(boost::asio::buffer(strExternalData, strExternalData.size()), sender_endpoint_);
				//std::cout << "Was sending " << s << "bytes" << std::endl << sendStr << std::endl;


				//Если включить то будет неуспевать
				//tinyxml2::XMLDocument test;
				//test.Parse(strDataKrc.c_str());
				//test.SaveFile("Test.xml");

				std::cout << "Send " << s << " bytes" << std::endl;
				someOutputInfo << strDataKrc << std::endl;
			}

		}
		else {
			continue;
		}

		memset(data_, 0, max_length);
	}
}

void Server::startTcpListening()
{
	std::cout << "Starting TCP connect...\n";

	boost::asio::io_service io_service;
	tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));

	std::cout << "Server listening on port " << port << ".\n";

	//ForTest
	//sCONTROL.GetProfileWithName("FirstCalibData.txt");
	//sCONTROL.VideoMode();

	/*std::vector<Point> points = getPointsFromFile("temp.txt");
	findEasyAngle(points);*/

	//double minimumZInProfile = findTopZ("FirstCalibData.txt");
	//cout << "Minimum Z = " << minimumZInProfile << endl;

	boost::system::error_code acceptError;
	boost::shared_ptr<boost::asio::ip::tcp::socket> sock(new tcp::socket(io_service));

	acceptor.accept(*sock, acceptError);


	if (acceptError)
	{
		std::cerr << "Boost error during accept. " << acceptError << std::endl;
		//sleep(1000);
	}
	else
	{
		std::cout << "A client connected.\n";
		std::cout << "Calibration started:\n";

		enum { max_length = 1024 };
		char data_[max_length];

		/*tinyxml2::XMLDocument XMLTransmit;
		int xmlLoadErr = XMLTransmit.LoadFile("XmlTransmit.xml");

		if (xmlLoadErr != tinyxml2::XML_SUCCESS) {
			std::cout << "XML load error...\n";
			return;
		}

		tinyxml2::XMLPrinter printer;
		XMLTransmit.Accept(&printer);*/

		int countOfReceivedData = 0;

		while (true) {
			size_t bytes_received = sock->receive(boost::asio::buffer(data_, max_length));

			if (bytes_received != 0) {
				std::string strReceived(data_, bytes_received);

				//Test
				std::cout << "Received by PC: \n" << strReceived << std::endl;

				if (countOfReceivedData == 0) {
					std::cout << "Saving to file...\n";

					saveDataFromString("FirstRobotData.xml", strReceived);
					sCONTROL.GetProfileWithName("FirstCalibData.txt");

					int zPrev = findTopZ("FirstCalibData.txt");

					//Ждём перепад
					while (true) {
						sCONTROL.GetProfileWithName("SecondCalibData.txt");
						int zCurrent = findTopZ("SecondCalibData.txt");
						
						if (abs(zCurrent) - abs(zPrev) > 3) {
							std::cout << "\nFirst position cathed!!!!!\n";
							break;
						}
						else zPrev = zCurrent;

						Sleep(10);
					}

				}
				else if (countOfReceivedData == 1) {
					//Ищем Х и в файлах и находим разницу
					//Открываем First Robot = X start
					//Открываем Second Robot = X last
					//X last - X start				
					//Выводим отклонение по Х

					sCONTROL.GetProfileWithName("scanControlXY.txt");
					std::vector<Point> points = getPointsFromFile("scanControlXY.txt");
					//findEasyAngle(points);

					std::cout << "Calibration is Done...\n";
					break;
				}

				std::string strTempSend = getStringFromFile("XmlTransmit.xml");

				//const char *strSend = strTempSend.data();

				//Size??!!!
				int sendSize = strTempSend.size();

				sock->send(boost::asio::buffer(strTempSend, strTempSend.size()));

				countOfReceivedData++;
			}
			else {
				continue;
			}

			memset(data_, 0, max_length);
		}
		/*
		If previously connected, threads and socket shall be dead,
		queues and data reset.
		*/
		//messageQueue.reset();
		//responseQueue.reset();
		//resetData();            // counters etc

		//connected = true;       // atomic

		//						// spawn
		//boost::thread read_thread(&Server::readThread, this, sock);
		//boost::thread write_thread(&Server::writeThread, this, sock);
		//boost::thread response_thread(&Server::responseThread, this);

		//// block for a second to let system get things rolling
		//sleep(1000);

		//accepting = true;   // finally allow clients to interact, atomic

		//read_thread.join();     // calls closeConnection() before exit
		//write_thread.join();
		//response_thread.join();
	}

	// if we get here, connection is terminated, try to shutdown sockets
//	boost::system::error_code shutdownError;
//	sock->shutdown(boost::asio::ip::tcp::socket::shutdown_both, shutdownError);    // recommended by boost
//	if (shutdownError)
//	{
//		std::cerr << "Socket shutdown complains:" << shutdownError << std::endl;
//	}
//
//	boost::system::error_code sockCloseError;
//	sock->close(sockCloseError);
//	if (sockCloseError)
//	{
//		std::cout << "Socket close complains:" << sockCloseError << std::endl;
//	}
//
//	// let socket pointer go out of scope, this should release it
//
//}
//while (keepAlive);
//cout << "Session thread exiting." << endl;
//
//	}
//	catch (std::exception &e)
//	{
//		cout << "Session exception: " << e.what() << endl;
//	}
}

void Server::weldingEasyTest(scanCONTROL &s)
{
	std::cout << "Starting UDP connect...\n\n";

	//
	boost::asio::io_service io_service;

	udp::socket socket_(io_service, udp::endpoint(udp::v4(), port));

	std::ifstream input("ExternalData.xml", std::ios_base::binary);
	char buffFromIfstream[1024];

	input.read(buffFromIfstream, 1024);
	int bytesFromFile = input.gcount();

	buffFromIfstream[bytesFromFile] = '\0';

	input.close();

	std::cout << "Current XML:\n\n" << buffFromIfstream;

	//std::string xmlForChange = buffFromIfstream;

	std::ifstream input2("Krc.xml", std::ios_base::binary);
	char buffFromIfstream2[1024];
	input2.read(buffFromIfstream2, 1024);
	int bytesFromFile2 = input.gcount();
	buffFromIfstream[bytesFromFile2] = '\0';
	input2.close();

	string krc = buffFromIfstream2;

	getRobotPos(krc);

	std::cout << "\nStart Listening on port " << port << std::endl;

	enum { max_length = 1024 };
	char data_[max_length];

	std::ofstream someOutputInfo("SomeOutputInfo.txt");

	//Для того чтобы принять сразу в string без использования char[]
	std::string strReceived;

	//Находим место где начинается шов и запоминаем позицию  prevRobotPosition
	RobotPosition prevRobotPosition;

	//Ищем минимальный Z в начальном профиле
	vector<Point> points(640);
	//sCONTROL.getCurrentProfileVec(points);

	//Ждём перепад
	sCONTROL.startProfileStreaming();

	//while (true) {
	//	sCONTROL.getCurrentProfileVec(points);
	//	sred(points, 1);
	//
	//	if (isLine(points)) {
	//		continue;
	//	}
	//	else {
	//		cout << "Some object!!!!!!...\n";
	//		break;
	//	}
	//	Sleep(5);
	//}

	////0 - L     1 - R
	//cout << "U need " << findEasyAngle(points, 1, 0) << " correction\n";
	//
	bool startTracking = false;
	int whenINeedChangeXML = 1;

	while (true)
	{
		udp::endpoint sender_endpoint_;
		size_t bytes_received = socket_.receive_from(boost::asio::buffer(data_, max_length), sender_endpoint_);
		//size_t bytes_r = socket_.receive_from(boost::asio::buffer(strReceived), sender_endpoint_);
		//std::cout << bytes_received << " bytes received" << std::endl;

		//Елси что то принято
		double coorVal;
		if (true) {		
			std::string strDataKrc(data_, bytes_received);

			//Если объект ещё не найден
			if (!startTracking) {
				sCONTROL.getCurrentProfileVec(points);
				sred(points, 1);

				if (!isLine(points)) {
					cout << "Some object!!!!!!...\n";
					coorVal = findEasyAngle(points, 1, 0);
					startTracking = true;
				}
			}
			else {   //Если найден каждый мм проверяем пакет и если есть сдвиг то менять итоговый xml
				if (whenINeedChangeXML == 100) {
					sCONTROL.getCurrentProfileVec(points);
					sred(points, 1);

					coorVal = findEasyAngle(points, 1, 1);

					//cout << "U need " << coorVal << endl;

					//if (strDataKrc.find("</Rob>") == std::string::npos) {
					//	continue;
					//}
					//else {
					//	std::string strExternalData = mirrorIPOC(strDataKrc, changeExternalData("y", coorVal / 10));

					//	int s = socket_.send_to(boost::asio::buffer(strExternalData, strExternalData.size()), sender_endpoint_);

					//	//std::cout << "Send " << s << " bytes" << std::endl;
					//	someOutputInfo << strDataKrc << std::endl;

					//	memset(data_, 0, max_length);
					//	continue;
					//}

					whenINeedChangeXML = 1;
				}
	
				whenINeedChangeXML++;
			}

			if (strDataKrc.find("</Rob>") == std::string::npos) {
				continue;
			}
			else {
				std::string strExternalData = mirrorIPOC(strDataKrc, changeExternalData("y", coorVal / 100));

				int s = socket_.send_to(boost::asio::buffer(strExternalData, strExternalData.size()), sender_endpoint_);

				//std::cout << "Send " << s << " bytes" << std::endl;
				//someOutputInfo << strDataKrc << std::endl;
			}

		}
		
		memset(data_, 0, max_length);
	}

	sCONTROL.stopProfileStreaming();
}

std::string Server::getStringFromFile(const std::string &fileName)
{
	std::ifstream input(fileName, std::ios_base::binary);
	char buffFromIfstream[1024];

	input.read(buffFromIfstream, 1024);
	int bytesFromFile = input.gcount();

	buffFromIfstream[bytesFromFile] = '\0';

	std::string result(buffFromIfstream, bytesFromFile);

	input.close();

	return result;
}

void Server::saveDataFromString(const std::string &fileName, const std::string &strForSaving)
{
	std::ofstream output(fileName, std::ios::binary);
	output << fileName;
	output.close();
}

double Server::findTopZ(const string &fileName)
{
	//TODO insert resolution
	std::ifstream input(fileName);
	char tempch;
	double tempX;
	double tempZ;

	double minZ = 0;
	int start = 0;

	while (minZ == 0) {
		input >> tempch >> tempch >> tempX;
		input >> tempch >> tempch >> minZ;

		start++;
		if (start > 640) return -1;
	}



	for (start; start < 640; start++) {
		input >> tempch >> tempch >> tempX;
		input >> tempch >> tempch >> tempZ;

		if (tempZ < minZ) minZ = tempZ;
	}

	input.close();
	return tempZ;
}

double Server::findTopZ(const vector<Point> &points)
{
	//TODO insert resolution

	char tempch;
	double tempX;
	double tempZ;

	double minZ = 0;
	int start = 0;

	while (minZ == 0) {
		minZ = points[start].z;

		start++;
		if (start > 640) return -1;
	}



	for (start; start < 640; start++) {
		if (points[start].z < minZ) minZ = points[start].z;
	}

	return minZ;
}

double Server::findEasyAngle(std::vector<Point> &p, int direction, int topBot)
{
	int indexL = 0;
	int indexR = 0;
	double z1 = 0;
	double z2 = 0;
	double maxRazn = 0;

	int tempIndex = 0;
	double temp = 0;

	int startIndex = 0;
	//int finishIndex = 0;
	int finishIndex = p.size();

	/*if (direction == 0) {
		startIndex = 0;
		finishIndex = p.size() / 2;
	}
	else {
		startIndex = p.size() / 2;
		finishIndex = p.size();
	}*/

	for (int i = startIndex; i < finishIndex - 1; i++) {
		tempIndex = i + 1;
		while (p[tempIndex].z == 0 && tempIndex < finishIndex - 2) {
			tempIndex++;
		}

		temp = abs(p[i].z - p[tempIndex].z);

		if (temp > maxRazn) {
			maxRazn = temp;
			indexL = i;
			indexR = tempIndex;
			z1 = p[i].z;
			z2 = p[tempIndex].z;

		}
		i = tempIndex - 1;
	}
	//Console.WriteLine($"Raz = {maxRazn} Left = {indexL} Z1 = {z1} Right = {indexR} Z2 = {z2}");

	int resultIndex = 0;
	if (p[indexL].z > p[indexR].z) {
		if (topBot == 0)  resultIndex = indexR;
		else resultIndex = indexL;
	}
	else {
		if (topBot == 0)  resultIndex = indexL;
		else resultIndex = indexR;
	}


	double result = p[p.size() / 2].x - p[resultIndex].x;
	//std::cout << "Distance of Centre  Left = " << abs(p[640].x - p[indexL].x);
	//std::cout << "\nDistance of Centre  RIght = " << abs(p[640].x - p[indexR].x) << std::endl;

	//return abs(result);
	return result;
}

std::vector<Point> Server::getPointsFromFile(const std::string &fileName)
{
	std::ifstream input(fileName);
	char tempch;
	Point tempPoint;

	std::vector<Point> points;

	for (int i = 0; i < 1280; i++) {
		input >> tempch >> tempch >> tempPoint.x;
		input >> tempch >> tempch >> tempPoint.z;

		points.push_back(tempPoint);
	}

	input.close();
	return points;
}

string Server::changeExternalData(const std::string &nameOfCoordinate, double coorVal)
{
	std::ifstream input("ExternalData.xml", std::ios_base::binary);
	char buffFromIfstream[1024];

	input.read(buffFromIfstream, 1024);
	int bytesFromFile = input.gcount();

	buffFromIfstream[bytesFromFile] = '\0';

	input.close();

	std::string xmlForChange = buffFromIfstream;
	
	//Если ставим setlocale RU то будет запятая а не точка(Может не подойти по формату)
	std::stringstream ss;
	ss << fixed;
	ss.precision(4);
	ss << coorVal;
	std::cout << ss.str() << std::endl;

	if (nameOfCoordinate == "x" || nameOfCoordinate == "X") {
		int xStartPosInXML = xmlForChange.find("X=\"") + 3;		
		xmlForChange.replace(xStartPosInXML, 6, ss.str());
	}
	else if (nameOfCoordinate == "y" || nameOfCoordinate == "Y") {
		int yStartPosInXML = xmlForChange.find("Y=\"") + 3;
		xmlForChange.replace(yStartPosInXML, 6, ss.str());
	}
	else if (nameOfCoordinate == "z" || nameOfCoordinate == "Z") {
		int zStartPosInXML = xmlForChange.find("Z=\"") + 3;
		xmlForChange.replace(zStartPosInXML, 6, ss.str());
	}
	else {
		std::cout << "Bad name of Coordinate, try to X or Y or Z..." << std::endl;
		return string();
	}


	return xmlForChange;
}

RobotPosition Server::getRobotPos(const std::string &krcData)
{
	RobotPosition result;
	std::string value;
	int startPos;

	std::cout << krcData << endl;

	if ((startPos = krcData.find("X=\"")) != string::npos) {
		value = krcData.substr(startPos + 3, 6);
		result.x = stod(value);
		cout << "RIst X = " << result.x << endl;
	}
	else cout << "Bad X pos\n";

	if ((startPos = krcData.find("Y=\"")) != string::npos) {
		value = krcData.substr(startPos + 3, 6);
		result.y = stod(value);
		cout << "RIst Y = " << result.y << endl;
	}
	else cout << "Bad RIst Y pos\n";

	if ((startPos = krcData.find("Z=\"")) != string::npos) {
		value = krcData.substr(startPos + 3, 6);
		result.z = stod(value);
		cout << "RIst Z = " <<result.z << std::endl;
	}
	else cout << "Bad RIst Z pos\n";


	//Find seond XYZ
	int startRSol = krcData.find("<RSol");

	if ((startPos = krcData.find("X=\"", startRSol)) != string::npos) {
		value = krcData.substr(startPos + 3, 6);
		result.x = stod(value);
		cout << "Rsol X = " << result.x << endl;
	}
	else cout << "Bad RIst X pos\n";

	if ((startPos = krcData.find("Y=\"", startRSol)) != string::npos) {
		value = krcData.substr(startPos + 3, 6);
		result.y = stod(value);
		cout << "Rsol Y = " << result.y << endl;
	}
	else cout << "Bad RIst Y pos\n";

	if ((startPos = krcData.find("Z=\"", startRSol)) != string::npos) {
		value = krcData.substr(startPos + 3, 6);
		result.z = stod(value);
		cout << "Rsol Z = " << result.z << std::endl;
	}
	else cout << "Bad RIst Z pos\n";

	return result;
}

bool Server::isLine(vector<Point> &p)
{
	double prevZ = 0;
	int start = 0;

	//Ищем первую точку неравную 0
	while (prevZ == 0) {
		prevZ = p[start].z;
		start++;
		if (start == p.size()) return false;
	}

	//МЕНЯТЬ РАЗРЕШЕНИЕ
	for (start; start < p.size(); start++) {
		if (p[start].z != 0) {
			if (abs(p[start].z - prevZ) > 3) {
				cout << abs(p[start].z - prevZ) << endl;
				return false;
			}
			else {
				prevZ = p[start].z;
			}
		}
	}

	return true;
}

void Server::sred(vector<Point> &p, int raz)
{
	int index = 0;

	for (int i = 1; i < p.size() - 1; i++) {
		index = i + 1;
		while (p[index].z == 0 && index < p.size())
		{
			index++;
		}
		if (abs(p[i - 1].z - p[index].z) <= raz) {
			p[i].z = (p[i - 1].z + p[index].z) / 2;
		}
		i = index - 1;
	}
}


