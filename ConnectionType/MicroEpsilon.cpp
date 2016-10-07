#include "MicroEpsilon.h"

#include <opencv2\highgui.hpp>

CInterfaceLLT*scanCONTROL::m_pLLT = NULL;
unsigned int scanCONTROL::m_uiResolution = 0;
TScannerType scanCONTROL::m_tscanCONTROLType = scanCONTROL2xxx;

bool scanCONTROL::start()
{
	bool bLoadError = false;
	int iRetValue;

	//Creating a LLT-object
	//The LLT-Object will load the LLT.dll automaticly and give us a error if ther no LLT.dll
	m_pLLT = new CInterfaceLLT("LLT.dll", &bLoadError);

	if (bLoadError)
	{
		cout << "Error loading LLT.dll \n";

		//Wait for a keyboard hit
		//while (!_kbhit()) {}

		//Deletes the LLT-object
		delete m_pLLT;
		return -1;
	}

	//Test if the LLT.dll supports GetLLTType (Version 3.0.0.0 or higher)
	if (m_pLLT->m_pFunctions->CreateLLTDevice == NULL)
	{
		cout << "Please use a LLT.dll version 3.0.0.0 or higher! \n";
	}
	else
	{
		//Create a Firewire Device
		if (m_pLLT->CreateLLTDevice(INTF_TYPE_ETHERNET))
			cout << "CreateLLTDevice OK \n";
		else
			cout << "Error during CreateLLTDevice\n";



		//Gets the available interfaces from the scanCONTROL-device
		iRetValue = m_pLLT->GetDeviceInterfacesFast(&vuiInterfaces[0], (unsigned int)vuiInterfaces.size());

		//Если не срабатывает подключение а лазер в сети и работает то необходимо поменять IP на лазере!!!!!!
		//Менял в стандартном прилоэении scanCONTROL
		//Есть Sensor finder который позволяет найти сенсор и конфигурировать его IP("D:\Development\Micro-Epsilon\scanCONTROL Software CD 8.1\Support\Ethernet")


		if (iRetValue == ERROR_GETDEVINTERFACES_REQUEST_COUNT)
		{
			cout << "There are more or equal than " << vuiInterfaces.size() << " scanCONTROL connected \n";
			uiInterfaceCount = vuiInterfaces.size();
		}
		else if (iRetValue < 0)
		{
			cout << "A error occured during searching for connected scanCONTROL \n";
			uiInterfaceCount = 0;
		}
		else
		{
			uiInterfaceCount = iRetValue;
		}

		if (uiInterfaceCount == 0)
			cout << "There is no scanCONTROL connected \n";
		else if (uiInterfaceCount == 1)
			cout << "There is 1 scanCONTROL connected \n";
		else
			cout << "There are " << uiInterfaceCount << " scanCONTROL's connected \n";

	}

	//cout << "Connecting to scanCONTROL " << 1 << "\n";
	//if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
	//{
	//	OnError("Error during Connect", iRetValue);
	//	return false;
	//}


	//TODO
	return true;
}


void scanCONTROL::getLLTInfos()
{
	for (unsigned int i = 0; i < uiInterfaceCount; i++)
	{
		GetLLTInfos(vuiInterfaces[i], i + 1);
	}
}

void scanCONTROL::VideoMode()
{
	int iRetValue;
	unsigned int uiWidth, uiHeight;
	std::vector<unsigned char> vucVideoBuffer;
	bool bOK = true;
	bool bConnected = false;

	cout << "Connecting to scanCONTROL\n";
	if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during Connect", iRetValue);
		bOK = false;
	}
	else
		bConnected = true;
	if (bOK)
	{
		cout << "Profile config set to VIDEO_IMAGE\n";
		if ((iRetValue = m_pLLT->SetProfileConfig(VIDEO_IMAGE)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetProfileConfig", iRetValue);
			bOK = false;
		}
	}

	if (bOK)
	{
		cout << "Set trigger to internal\n";
		if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_TRIGGER, 0x00000000)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetFeature(FEATURE_FUNCTION_TRIGGER)", iRetValue);
			bOK = false;
		}
	}

	if (bOK)
	{
		cout << "Set shutter time to " << uiShutterTime << "\n";
		if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_SHUTTERTIME, uiShutterTime)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetFeature(FEATURE_FUNCTION_SHUTTERTIME)", iRetValue);
			bOK = false;
		}
	}

	if (bOK)
	{
		cout << "Set idle time to " << uiIdleTime << "\n";
		if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_IDLETIME, uiIdleTime)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetFeature(FEATURE_FUNCTION_IDLETIME)", iRetValue);
			bOK = false;
		}
	}

	cout << "\nDemonstrate the Video mode\n";

	cout << "Sets the PacketSize to " << m_uiPacketSize << "\n";
	if ((iRetValue = m_pLLT->SetPacketSize(m_uiPacketSize)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during SetPacketSize", iRetValue);
		return;
	}

	cout << "Enable the video stream\n";
	if ((iRetValue = m_pLLT->TransferVideoStream(VIDEO_MODE_1, true, &uiWidth, &uiHeight)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferVideoStream", iRetValue);
		return;
	}

	//Sleep for a while to warm up the transfer
	Sleep(1000);


	static int videoFileNumber = 1;

	string fileName = "./VideoPicture" + to_string(videoFileNumber) + ".bmp";
	m_pLLT->SaveProfiles(fileName.c_str(), BMP);

	//Sleep for a while to warm up the transfer
	Sleep(1000);

	vucVideoBuffer.resize(uiWidth*uiHeight);


	//While button clicked...
	//Gets 1 profile in "polling-mode" and PROFILE configuration
	cout << "Get one video picture\n\n";
	if ((iRetValue = m_pLLT->GetActualProfile(&vucVideoBuffer[0], (unsigned int)vucVideoBuffer.size(), PROFILE, NULL))
		!= vucVideoBuffer.size())
	{
		OnError("Error during GetActualProfile", iRetValue);
		return;
	}
	//Else goto imageview

	cout << "Disable the video stream\n";
	if ((iRetValue = m_pLLT->TransferVideoStream(VIDEO_MODE_1, false, NULL, NULL)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferVideoStream", iRetValue);
		return;
	}

	videoFileNumber++;

	if (bConnected)
	{
		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
	}
}

void scanCONTROL::GetLLTInfos(unsigned int uiDeviceID, unsigned int uiLLTNumber)
{
	std::vector<DWORD> vdwResolutions(MAX_RESOULUTIONS);
	std::vector<char> vcDeviceName(100);
	TScannerType m_tscanCONTROLType;
	DWORD dwSerial = 0;
	int iRetValue;

	cout << "\nDemonstrate the read out of informations from the scanCONTROL\n";

	cout << "Select the device interface " << uiDeviceID << "\n";
	if ((iRetValue = m_pLLT->SetDeviceInterface(uiDeviceID, 0)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during SetDeviceInterface", iRetValue);
		return;
	}

	cout << "Connecting to scanCONTROL " << uiLLTNumber << "\n";
	if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during Connect", iRetValue);
		return;
	}

	cout << "Gets the device name\n";
	if ((iRetValue = m_pLLT->GetDeviceName(&vcDeviceName[0], (unsigned int)vcDeviceName.size(), NULL, NULL)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during GetDeviceName", iRetValue);

		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
		return;
	}

	cout << "Device name is " << &vcDeviceName[0] << "\n";


	cout << "Gets the type of the scanCONTROL\n";
	if ((iRetValue = m_pLLT->GetLLTType(&m_tscanCONTROLType)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during GetLLTType", iRetValue);

		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
		return;
	}

	if (IsMeasurementRange100(m_tscanCONTROLType))
	{
		cout << "The measurement range is 100 mm\n";
	}
	else if (IsMeasurementRange50(m_tscanCONTROLType))
	{
		cout << "The measurement range is 50 mm\n";
	}
	else if (IsMeasurementRange25(m_tscanCONTROLType))
	{
		cout << "The measurement range is 25 mm\n";
	}
	else if (IsMeasurementRange10(m_tscanCONTROLType))
	{
		cout << "The measurement range is 10 mm\n";
	}

	cout << "Gets the serial number of the scanCONTROL\n";
	if ((iRetValue = m_pLLT->GetFeature(FEATURE_FUNCTION_SERIAL, &dwSerial)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during GetSerial", iRetValue);

		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
		return;
	}

	cout << "Serial is " << dwSerial << "\n";

	//Gets the available resolutions from the scanCONTROL
	int uiResolutionCount = m_pLLT->GetResolutions(&vdwResolutions[0], (unsigned int)vdwResolutions.size());

	if (uiResolutionCount > 0)
	{
		cout << "Available resolutions: \n";
		for (int i = 0; i < uiResolutionCount; i++)
		{
			cout << "  " << vdwResolutions[i] << "\n";
		}
		cout << "\n";
	}
	else
	{
		OnError("Error during GetResolutions", iRetValue);

		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
		return;
	}

	cout << "Disconnect the scanCONTROL\n";
	if((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
	{
	  OnError("Error during Disconnect", iRetValue);
	}
}

void scanCONTROL::disconnect()
{
	//Deletes the LLT-object
	delete m_pLLT;
}

void scanCONTROL::goStreaming()
{
	try
	{
		session = thread_ptr(new boost::thread(&scanCONTROL::startStreaming, this));
	}
	catch (std::exception &e)
	{
		cerr << "Exception when creating session: " << e.what() << endl;
	}
}

void scanCONTROL::getSomeInfo()
{
	DWORD nValue;
	if (m_pLLT->GetFeature(INQUIRY_FUNCTION_LASERPOWER, &nValue) == GENERAL_FUNCTION_OK) {
		int val = nValue & 0xf0f00824;

		unsigned int nMaxvalue = nValue & 0x00000fff;
		unsigned int nMinvalue = (nValue & 0x00fff000) >> 12;
		bool bAutoMode = (bool)((nValue & 0x02000000) >> 25);
		bool bAvailable = (bool)((nValue & 0x80000000) >> 31);

		std::cout << "Laser power max value = " << nMaxvalue << std::endl << "Laser power min value = " << nMinvalue
			<< std::endl << "Laser auto mode = " << bAutoMode << std::endl << "Laser avaliable = " << bAvailable << std::endl;
	}

	//0x00000000 OFF laser
	//0x00000002 reduced power
	//0x00000001 full power
	//Default settings 0x82000002
	DWORD setValue = 0x82000002;

	if (m_pLLT->SetFeature(FEATURE_FUNCTION_LASERPOWER, setValue) == GENERAL_FUNCTION_OK) {
		std::cout << "Here u need change Laser power\n";
	}


	////setValue = (0x00000801);
	//if (m_pLLT->SetFeature(FEATURE_FUNCTION_LASERPOWER, setValue) == GENERAL_FUNCTION_OK) {
	//	std::cout << "Here u need change Laser power\n";
	//}

}

//Displaying the error text
void scanCONTROL::OnError(const char* szErrorTxt, int iErrorValue)
{
	char acErrorString[200];

	cout << szErrorTxt << "\n";
	if (m_pLLT->TranslateErrorValue(iErrorValue, acErrorString, sizeof(acErrorString)) >= GENERAL_FUNCTION_OK)
		cout << acErrorString << "\n\n";
}

//Ethernet
void scanCONTROL::GetProfiles_Poll()
{
	std::vector<DWORD> vdwResolutions(MAX_RESOULUTIONS);
	unsigned int uiShutterTime = 100;
	unsigned int uiIdleTime = 900;
	bool bOK = true;
	bool bConnected = false;
	int iRetValue;

	if (uiInterfaceCount >= 1)
	{
		cout << "\nSelect the device interface " << vuiInterfaces[0] << "\n";
		if ((iRetValue = m_pLLT->SetDeviceInterface(vuiInterfaces[0], 0)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetDeviceInterface", iRetValue);
			bOK = false;
		}

		if (bOK)
		{
			cout << "Connecting to scanCONTROL\n";
			if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during Connect", iRetValue);
				bOK = false;
			}
			else
				bConnected = true;
		}


		if (bOK)
		{
			cout << "Get scanCONTROL type\n";
			if ((iRetValue = m_pLLT->GetLLTType(&m_tscanCONTROLType)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during GetLLTType", iRetValue);
				bOK = false;
			}

			if (iRetValue == GENERAL_FUNCTION_DEVICE_NAME_NOT_SUPPORTED)
			{
				cout << "Can't decode scanCONTROL type. Please contact Micro-Epsilon for a newer version of the LLT.dll.\n";
			}

			if (m_tscanCONTROLType >= scanCONTROL28xx_25 && m_tscanCONTROLType <= scanCONTROL28xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL28xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL27xx_25 && m_tscanCONTROLType <= scanCONTROL27xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL27xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL26xx_25 && m_tscanCONTROLType <= scanCONTROL26xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL26xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL29xx_25 && m_tscanCONTROLType <= scanCONTROL29xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL29xx\n\n";
			}
			else
			{
				cout << "The scanCONTROL is a undefined type\nPlease contact Micro-Epsilon for a newer SDK\n\n";
			}


			cout << "Get all possible resolutions\n";
			if ((iRetValue = m_pLLT->GetResolutions(&vdwResolutions[0], vdwResolutions.size())) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during GetResolutions", iRetValue);
				bOK = false;
			}

			m_uiResolution = vdwResolutions[0];
		}

		if (bOK)
		{
			cout << "Set resolution to " << m_uiResolution << "\n";
			if ((iRetValue = m_pLLT->SetResolution(m_uiResolution)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetResolution", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Set trigger to internal\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_TRIGGER, 0x00000000)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_TRIGGER)", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Profile config set to PROFILE\n";
			if ((iRetValue = m_pLLT->SetProfileConfig(PROFILE)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetProfileConfig", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Set shutter time to " << uiShutterTime << "\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_SHUTTERTIME, uiShutterTime)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_SHUTTERTIME)", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Set idle time to " << uiIdleTime << "\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_IDLETIME, uiIdleTime)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_IDLETIME)", iRetValue);
				bOK = false;
			}
		}
	}

//	int iRetValue;
	std::vector<double> vdValueX(m_uiResolution);
	std::vector<double> vdValueZ(m_uiResolution);
	//Resize the profile buffer to the maximal profile size
	std::vector<unsigned char> vucProfileBuffer(m_uiResolution * 4 + 16);

	cout << "\nDemonstrate the profile transfer via poll function\n";

	cout << "Gets the type of the scanCONTROL (measurement range)\n";

	cout << "Enable the measurement\n";
	if ((iRetValue = m_pLLT->TransferProfiles(NORMAL_TRANSFER, true)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferProfiles", iRetValue);
		return;
	}

	//Sleep for a while to warm up the transfer
	Sleep(100);

	//Gets 1 profile in "polling-mode" and PURE_PROFILE configuration
	if ((iRetValue = m_pLLT->GetActualProfile(&vucProfileBuffer[0], (unsigned int)vucProfileBuffer.size(), PURE_PROFILE, NULL))
		!= vucProfileBuffer.size())
	{
		OnError("Error during GetActualProfile", iRetValue);
		return;
	}
	cout << "Get profile in polling-mode and PURE_PROFILE configuration OK \n";



	cout << "Converting of profile data from the first reflection\n";
	iRetValue = m_pLLT->ConvertProfile2Values(&vucProfileBuffer[0], m_uiResolution, PURE_PROFILE, m_tscanCONTROLType,
		0, true, NULL, NULL, NULL, &vdValueX[0], &vdValueZ[0], NULL, NULL);
	if (((iRetValue & CONVERT_X) == 0) || ((iRetValue & CONVERT_Z) == 0))
	{
		OnError("Error during Converting of profile data", iRetValue);
		return;
	}

	DisplayProfile(&vdValueX[0], &vdValueZ[0], m_uiResolution);

	cout << "\n\nDisplay the timestamp from the profile:";
	DisplayTimestamp(&vucProfileBuffer[m_uiResolution * 4]);

	cout << "Disable the measurement\n";
	if ((iRetValue = m_pLLT->TransferProfiles(NORMAL_TRANSFER, false)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferProfiles", iRetValue);
		return;
	}


	if (bConnected)
	{
	cout << "Disconnect the scanCONTROL\n";
	if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
	{
	OnError("Error during Disconnect", iRetValue);
	}
	}
}

void scanCONTROL::GetProfileWithName(const std::string &fileName)
{
	std::vector<DWORD> vdwResolutions(MAX_RESOULUTIONS);
	unsigned int uiShutterTime = 100;
	unsigned int uiIdleTime = 900;
	bool bOK = true;
	bool bConnected = false;
	int iRetValue;

	if (uiInterfaceCount >= 1)
	{
		//cout << "\nSelect the device interface " << vuiInterfaces[0] << "\n";
		if ((iRetValue = m_pLLT->SetDeviceInterface(vuiInterfaces[0], 0)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetDeviceInterface", iRetValue);
			bOK = false;
		}

		if (bOK)
		{
			//cout << "Connecting to scanCONTROL\n";
			if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during Connect", iRetValue);
				bOK = false;
			}
			else
				bConnected = true;
		}


		if (bOK)
		{
			//cout << "Get scanCONTROL type\n";
			if ((iRetValue = m_pLLT->GetLLTType(&m_tscanCONTROLType)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during GetLLTType", iRetValue);
				bOK = false;
			}

			if (iRetValue == GENERAL_FUNCTION_DEVICE_NAME_NOT_SUPPORTED)
			{
				cout << "Can't decode scanCONTROL type. Please contact Micro-Epsilon for a newer version of the LLT.dll.\n";
			}

			/*if (m_tscanCONTROLType >= scanCONTROL28xx_25 && m_tscanCONTROLType <= scanCONTROL28xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL28xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL27xx_25 && m_tscanCONTROLType <= scanCONTROL27xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL27xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL26xx_25 && m_tscanCONTROLType <= scanCONTROL26xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL26xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL29xx_25 && m_tscanCONTROLType <= scanCONTROL29xx_xxx)
			{
				cout << "The scanCONTROL is a scanCONTROL29xx\n\n";
			}
			else
			{
				cout << "The scanCONTROL is a undefined type\nPlease contact Micro-Epsilon for a newer SDK\n\n";
			}*/


			//cout << "Get all possible resolutions\n";
			if ((iRetValue = m_pLLT->GetResolutions(&vdwResolutions[0], vdwResolutions.size())) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during GetResolutions", iRetValue);
				bOK = false;
			}

			m_uiResolution = vdwResolutions[0];
		}

		if (bOK)
		{
			//cout << "Set resolution to " << m_uiResolution << "\n";
			if ((iRetValue = m_pLLT->SetResolution(m_uiResolution)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetResolution", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			//cout << "Set trigger to internal\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_TRIGGER, 0x00000000)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_TRIGGER)", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			//cout << "Profile config set to PROFILE\n";
			if ((iRetValue = m_pLLT->SetProfileConfig(PROFILE)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetProfileConfig", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			//cout << "Set shutter time to " << uiShutterTime << "\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_SHUTTERTIME, uiShutterTime)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_SHUTTERTIME)", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			//cout << "Set idle time to " << uiIdleTime << "\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_IDLETIME, uiIdleTime)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_IDLETIME)", iRetValue);
				bOK = false;
			}
		}
	}

	//	int iRetValue;
	std::vector<double> vdValueX(m_uiResolution);
	std::vector<double> vdValueZ(m_uiResolution);
	//Resize the profile buffer to the maximal profile size
	std::vector<unsigned char> vucProfileBuffer(m_uiResolution * 4 + 16);

	//cout << "\nDemonstrate the profile transfer via poll function\n";

	//cout << "Gets the type of the scanCONTROL (measurement range)\n";

	//cout << "Enable the measurement\n";
	if ((iRetValue = m_pLLT->TransferProfiles(NORMAL_TRANSFER, true)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferProfiles", iRetValue);
		return;
	}

	//Sleep for a while to warm up the transfer
	Sleep(100);

	//Gets 1 profile in "polling-mode" and PURE_PROFILE configuration
	if ((iRetValue = m_pLLT->GetActualProfile(&vucProfileBuffer[0], (unsigned int)vucProfileBuffer.size(), PURE_PROFILE, NULL))
		!= vucProfileBuffer.size())
	{
		OnError("Error during GetActualProfile", iRetValue);
		return;
	}
	cout << "Get profile in polling-mode and PURE_PROFILE configuration OK \n";



	cout << "Converting of profile data from the first reflection\n";
	iRetValue = m_pLLT->ConvertProfile2Values(&vucProfileBuffer[0], m_uiResolution, PURE_PROFILE, m_tscanCONTROLType,
		0, true, NULL, NULL, NULL, &vdValueX[0], &vdValueZ[0], NULL, NULL);
	if (((iRetValue & CONVERT_X) == 0) || ((iRetValue & CONVERT_Z) == 0))
	{
		OnError("Error during Converting of profile data", iRetValue);
		return;
	}

	DisplayProfile(&vdValueX[0], &vdValueZ[0], m_uiResolution, fileName);

	cout << "\n\nDisplay the timestamp from the profile:";
	DisplayTimestamp(&vucProfileBuffer[m_uiResolution * 4]);

	cout << "Disable the measurement\n";
	if ((iRetValue = m_pLLT->TransferProfiles(NORMAL_TRANSFER, false)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferProfiles", iRetValue);
		return;
	}


	if (bConnected)
	{
		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
	}
}

//Displays one profile
void scanCONTROL::DisplayProfile(double *pdValueX, double *pdValueZ, unsigned int uiResolution)
{
	static int filePartNumber = 1;
	string fileName = "Profile data #" + to_string(filePartNumber) + ".txt";

	cout << fileName << endl;

	size_t tNumberSize;
	ofstream out(fileName);

	for (unsigned int i = 0; i < uiResolution; i++)
	{
		//Prints the X- and Z-values
		tNumberSize = Double2Str(*pdValueX).size();

		out << "\r" << "X = " << *pdValueX++;

		for (; tNumberSize < 8; tNumberSize++)
		{
			out << " ";
		}

		tNumberSize = Double2Str(*pdValueZ).size();
		out << " Z = " << *pdValueZ++;
		for (; tNumberSize < 8; tNumberSize++)
		{
			out << " ";
		}

		//Somtimes wait a short time (only for display)
		/*if (i % 8 == 0)
		{
			Sleep(10);
		}*/
		out << endl;
	}

	filePartNumber++;
	out.close();
}

//SaveToNamedFile
void scanCONTROL::DisplayProfile(double *pdValueX, double *pdValueZ, unsigned int uiResolution, const std::string &fileName)
{
	cout << "Saving to " << fileName << endl;

	size_t tNumberSize;
	ofstream out(fileName);

	for (unsigned int i = 0; i < uiResolution; i++)
	{
		//Prints the X- and Z-values
		tNumberSize = Double2Str(*pdValueX).size();

		out << "\r" << "X = " << *pdValueX++;

		for (; tNumberSize < 8; tNumberSize++)
		{
			out << " ";
		}

		tNumberSize = Double2Str(*pdValueZ).size();
		out << " Z = " << *pdValueZ++;
		for (; tNumberSize < 8; tNumberSize++)
		{
			out << " ";
		}

		out << endl;
	}

	out.close();
}

//Displays the timestamp
void scanCONTROL::DisplayTimestamp(unsigned char *pucTimestamp)
{
	double dShutterOpen, dShutterClose;
	unsigned int uiProfileCount;

	//Decode the timestamp
	m_pLLT->Timestamp2TimeAndCount(pucTimestamp, &dShutterOpen, &dShutterClose, &uiProfileCount);
	cout << "\nShutterOpen: " << dShutterOpen << " ShutterClose: " << dShutterClose << "\n";
	cout << "ProfileCount: " << uiProfileCount << "\n";
	cout << "\n";
}

//Convert a double value to a string
std::string scanCONTROL::Double2Str(double dValue)
{
	std::ostringstream NewStreamApp;
	NewStreamApp << dValue;

	return NewStreamApp.str();
}

void scanCONTROL::startStreaming()
{
	int iRetValue;
	unsigned int uiWidth, uiHeight;
	std::vector<unsigned char> vucVideoBuffer;
	bool bOK = true;

	bool bConnected = false;

	cout << "Connecting to scanCONTROL\n";
	if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during Connect", iRetValue);
		bOK = false;
	}
	else
		bConnected = true;

	if (bOK)
	{
		cout << "Profile config set to VIDEO_IMAGE\n";
		if ((iRetValue = m_pLLT->SetProfileConfig(VIDEO_IMAGE)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetProfileConfig", iRetValue);
			bOK = false;
		}
	}

	if (bOK)
	{
		cout << "Set trigger to internal\n";
		if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_TRIGGER, 0x00000000)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetFeature(FEATURE_FUNCTION_TRIGGER)", iRetValue);
			bOK = false;
		}
	}

	if (bOK)
	{
		cout << "Set shutter time to " << uiShutterTime << "\n";
		if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_SHUTTERTIME, uiShutterTime)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetFeature(FEATURE_FUNCTION_SHUTTERTIME)", iRetValue);
			bOK = false;
		}
	}

	if (bOK)
	{
		cout << "Set idle time to " << uiIdleTime << "\n";
		if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_IDLETIME, uiIdleTime)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetFeature(FEATURE_FUNCTION_IDLETIME)", iRetValue);
			bOK = false;
		}
	}

	//cvNamedWindow("original", CV_WINDOW_AUTOSIZE);


	cout << "\nDemonstrate the Video mode\n";

	cout << "Sets the PacketSize to " << m_uiPacketSize << "\n";
	if ((iRetValue = m_pLLT->SetPacketSize(m_uiPacketSize)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during SetPacketSize", iRetValue);
		return;
	}

	cout << "Enable the video stream\n";
	
	if ((iRetValue = m_pLLT->TransferVideoStream(VIDEO_MODE_1, true, &uiWidth, &uiHeight)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferVideoStream", iRetValue);
		return;
	}

	//Sleep for a while to warm up the transfer
	Sleep(1000);


	static int videoFileNumber = 1;

	string fileName = "./VideoPicture" + to_string(videoFileNumber) + ".bmp";
	m_pLLT->SaveProfiles(fileName.c_str(), BMP);

	//Sleep for a while to warm up the transfer
	Sleep(1000);

	vucVideoBuffer.resize(uiWidth*uiHeight);

	cv::Mat currentProfile;
	cv::namedWindow("ProfileViewer", CV_WINDOW_FREERATIO);

	while (true) {
		//fileName = "d://ForTest//Test #"  + to_string(someCounter) + ".bmp";

		//cout << "Get one video picture\n\n";
		if ((iRetValue = m_pLLT->GetActualProfile(&vucVideoBuffer[0], (unsigned int)vucVideoBuffer.size(), PROFILE, NULL))
			!= vucVideoBuffer.size())
		{
			//OnError("Error during GetActualProfile", iRetValue);
			//return;
		}
		else {
			currentProfile = cv::imread(fileName.c_str());

			cv::imshow("ProfileViewer", currentProfile);

			cv::waitKey(1);

		}

		m_pLLT->SaveProfiles(fileName.c_str(), BMP);
		Sleep(100);
		//std::cout << someCounter++ << std::endl;
	}
	//Else goto imageview

	cout << "Disable the video stream\n";
	if ((iRetValue = m_pLLT->TransferVideoStream(VIDEO_MODE_1, false, NULL, NULL)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferVideoStream", iRetValue);
		return;
	}

	videoFileNumber++;

	if (bConnected)
	{
		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
	}
}

void scanCONTROL::startProfileStreaming()
{
	std::vector<DWORD> vdwResolutions(MAX_RESOULUTIONS);
	unsigned int uiShutterTime = 100;
	unsigned int uiIdleTime = 900;
	bool bOK = true;
	bool bConnected = false;
	int iRetValue;

	if (uiInterfaceCount >= 1)
	{
		cout << "\nSelect the device interface " << vuiInterfaces[0] << "\n";
		if ((iRetValue = m_pLLT->SetDeviceInterface(vuiInterfaces[0], 0)) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during SetDeviceInterface", iRetValue);
			bOK = false;
		}

		if (bOK)
		{
			cout << "Connecting to scanCONTROL\n";
			if ((iRetValue = m_pLLT->Connect()) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during Connect", iRetValue);
				bOK = false;
			}
			else
				bConnected = true;
		}


		if (bOK)
		{
			cout << "Get scanCONTROL type\n";
			if ((iRetValue = m_pLLT->GetLLTType(&m_tscanCONTROLType)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during GetLLTType", iRetValue);
				bOK = false;
			}

			if (iRetValue == GENERAL_FUNCTION_DEVICE_NAME_NOT_SUPPORTED)
			{
				cout << "Can't decode scanCONTROL type. Please contact Micro-Epsilon for a newer version of the LLT.dll.\n";
			}

			if (m_tscanCONTROLType >= scanCONTROL28xx_25 && m_tscanCONTROLType <= scanCONTROL28xx_xxx)
			{
			cout << "The scanCONTROL is a scanCONTROL28xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL27xx_25 && m_tscanCONTROLType <= scanCONTROL27xx_xxx)
			{
			cout << "The scanCONTROL is a scanCONTROL27xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL26xx_25 && m_tscanCONTROLType <= scanCONTROL26xx_xxx)
			{
			cout << "The scanCONTROL is a scanCONTROL26xx\n\n";
			}
			else if (m_tscanCONTROLType >= scanCONTROL29xx_25 && m_tscanCONTROLType <= scanCONTROL29xx_xxx)
			{
			cout << "The scanCONTROL is a scanCONTROL29xx\n\n";
			}
			else
			{
			cout << "The scanCONTROL is a undefined type\nPlease contact Micro-Epsilon for a newer SDK\n\n";
			}


			cout << "Get all possible resolutions\n";
			if ((iRetValue = m_pLLT->GetResolutions(&vdwResolutions[0], vdwResolutions.size())) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during GetResolutions", iRetValue);
				bOK = false;
			}

			//[0] 1280 [1] 640 [2] 320 [3] 160
			m_uiResolution = vdwResolutions[1];
		}

		if (bOK)
		{
			cout << "Set resolution to " << m_uiResolution << "\n";
			if ((iRetValue = m_pLLT->SetResolution(m_uiResolution)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetResolution", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Set trigger to internal\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_TRIGGER, 0x00000000)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_TRIGGER)", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Profile config set to PROFILE\n";
			if ((iRetValue = m_pLLT->SetProfileConfig(PROFILE)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetProfileConfig", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Set shutter time to " << uiShutterTime << "\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_SHUTTERTIME, uiShutterTime)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_SHUTTERTIME)", iRetValue);
				bOK = false;
			}
		}

		if (bOK)
		{
			cout << "Set idle time to " << uiIdleTime << "\n";
			if ((iRetValue = m_pLLT->SetFeature(FEATURE_FUNCTION_IDLETIME, uiIdleTime)) < GENERAL_FUNCTION_OK)
			{
				OnError("Error during SetFeature(FEATURE_FUNCTION_IDLETIME)", iRetValue);
				bOK = false;
			}
		}
	}

	//cout << "\nDemonstrate the profile transfer via poll function\n";

	cout << "Gets the type of the scanCONTROL (measurement range)\n";

	cout << "Enable the measurement\n";
	if ((iRetValue = m_pLLT->TransferProfiles(NORMAL_TRANSFER, true)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferProfiles", iRetValue);
		return;
	}

	//Sleep for a while to warm up the transfer
	Sleep(100);

	isProfileStreaming = true;

}

void scanCONTROL::getCurrentProfileVec(std::vector<Point> &result)
{
	if (result.size() < m_uiResolution) {
		cout << "Bad vector size\n";
		return;
	}

	if (isProfileStreaming == true) {
		std::vector<double> vdValueX(m_uiResolution);
		std::vector<double> vdValueZ(m_uiResolution);
		//Resize the profile buffer to the maximal profile size
		std::vector<unsigned char> vucProfileBuffer(m_uiResolution * 4 + 16);
		int iRetValue;

		//Gets 1 profile in "polling-mode" and PURE_PROFILE configuration
		if ((iRetValue = m_pLLT->GetActualProfile(&vucProfileBuffer[0], (unsigned int)vucProfileBuffer.size(), PURE_PROFILE, NULL))
			!= vucProfileBuffer.size())
		{
			//OnError("Error during GetActualProfile", iRetValue);
			return;
		}
		//cout << "Get profile in polling-mode and PURE_PROFILE configuration OK \n";

		//cout << "Converting of profile data from the first reflection\n";
		iRetValue = m_pLLT->ConvertProfile2Values(&vucProfileBuffer[0], m_uiResolution, PURE_PROFILE, m_tscanCONTROLType,
			0, true, NULL, NULL, NULL, &vdValueX[0], &vdValueZ[0], NULL, NULL);
		if (((iRetValue & CONVERT_X) == 0) || ((iRetValue & CONVERT_Z) == 0))
		{
			OnError("Error during Converting of profile data", iRetValue);
			return;
		}

		double *pdValueX = &vdValueX[0];
		double *pdValueZ = &vdValueZ[0];

		size_t tNumberSize;

		//std::vector<Point> result(m_uiResolution);

		for (unsigned int i = 0; i <  m_uiResolution; i++)
		{
			result[i].x = *pdValueX++;
			result[i].z = *pdValueZ++;
		}
	}
	else {
		cout << "Laser isnt connected...\n";
		return;
	}
}

void scanCONTROL::stopProfileStreaming()
{
	int iRetValue;

	cout << "Disable the measurement\n";
	if ((iRetValue = m_pLLT->TransferProfiles(NORMAL_TRANSFER, false)) < GENERAL_FUNCTION_OK)
	{
		OnError("Error during TransferProfiles", iRetValue);
		return;
	}


	if (isProfileStreaming)
	{
		cout << "Disconnect the scanCONTROL\n";
		if ((iRetValue = m_pLLT->Disconnect()) < GENERAL_FUNCTION_OK)
		{
			OnError("Error during Disconnect", iRetValue);
		}
	}
}

/**********************************************************************************************//**
 * \fn	bool IsMeasurementRange10(TScannerType scanCONTROLType)
 *
 * \brief	Query if 'scanCONTROLType' is measurement range 10.
 *
 * \date	04.11.2013
 *
 * \param	scanCONTROLType	Type of the scan control.
 *
 * \return	true if measurement range 10, false if not.
 **************************************************************************************************/
bool scanCONTROL::IsMeasurementRange10(TScannerType scanCONTROLType)
{
	return scanCONTROLType == scanCONTROL28xx_10;
}

/**********************************************************************************************//**
 * \fn	bool IsMeasurementRange25(TScannerType scanCONTROLType)
 *
 * \brief	Query if 'scanCONTROLType' is measurement range 25.
 *
 * \date	04.11.2013
 *
 * \param	scanCONTROLType	Type of the scan control.
 *
 * \return	true if measurement range 25, false if not.
 **************************************************************************************************/
bool scanCONTROL::IsMeasurementRange25(TScannerType scanCONTROLType)
{
	return scanCONTROLType == scanCONTROL26xx_25 ||
		scanCONTROLType == scanCONTROL27xx_25 ||
		scanCONTROLType == scanCONTROL28xx_25 ||
		scanCONTROLType == scanCONTROL29xx_25;
}
/**********************************************************************************************//**
 * \fn	bool IsMeasurementRange50(TScannerType scanCONTROLType)
 *
 * \brief	Query if 'scanCONTROLType' is measurement range 50.
 *
 * \date	04.11.2013
 *
 * \param	scanCONTROLType	Type of the scan control.
 *
 * \return	true if measurement range 50, false if not.
 **************************************************************************************************/
bool scanCONTROL::IsMeasurementRange50(TScannerType scanCONTROLType)
{
	return scanCONTROLType == scanCONTROL26xx_50 ||
		scanCONTROLType == scanCONTROL27xx_50 ||
		scanCONTROLType == scanCONTROL29xx_50;
}

/**********************************************************************************************//**
 * \fn	bool IsMeasurementRange100(TScannerType scanCONTROLType)
 *
 * \brief	Query if 'scanCONTROLType' is measurement range 100.
 *
 * \date	04.11.2013
 *
 * \param	scanCONTROLType	Type of the scan control.
 *
 * \return	true if measurement range 100, false if not.
 **************************************************************************************************/

bool scanCONTROL::IsMeasurementRange100(TScannerType scanCONTROLType)
{
	return scanCONTROLType == scanCONTROL26xx_100 ||
		scanCONTROLType == scanCONTROL27xx_100 ||
		scanCONTROLType == scanCONTROL28xx_100 ||
		scanCONTROLType == scanCONTROL29xx_100;
}

