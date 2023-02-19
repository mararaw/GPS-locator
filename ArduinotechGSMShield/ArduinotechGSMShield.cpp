//Utilities for ArduinotechGSMShield

#include "Arduino.h"
#include "ArduinotechGSMShield.h"
#include "SoftwareSerial.h"


GSM_modul::GSM_modul()
{
}

//SIM800 initialization procedure for simple SMS and call features
void GSM_modul::SIM800Init()
{
	while (sendATcommand("AT", "OK", 2000) == 0);
	delay(2000);
	//otestuj registraci do site
	while ((sendATcommand("AT+CREG?", "+CREG: 0,1", 1000) ||
		sendATcommand("AT+CREG?", "+CREG: 0,5", 1000)) == 0);
	//parameters to obtain time stamp from GSM network
	while (sendATcommand("AT+CLTS=1", "OK", 500) == 0);
	while (sendATcommand("AT+CENG=3", "OK", 500) == 0);
	sendATcommand("AT+CMGF=1", "OK", 500);
	//zakaz indikace SMS
	sendATcommand("AT+CNMI=0,0", "OK", 500);
	//CLIP enabled
	sendATcommand("AT+CLIP=1", "OK", 1000);
	//smaz vsechny SMSky
	sendATcommand("AT+CMGD=1,4", "OK", 2000);
	sendATcommand("AT+CMGD=1", "OK", 2000);
}

bool GSM_modul::GPRSInit(String APN)
{
	APN = "AT+SAPBR=3,1,\"APN\",\"" + APN + "\"" ;
	char APNchar[sizeof(APN)+1];
	for (uint8_t i; i < (sizeof(APN) + 1); i++) APNchar[i] = APN[i];
	
	while( (sendATcommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 500)) == 0 );
	//APN = "AT+SAPBR=3,1,\"APN\",\" + APN + "\"";
	while( (sendATcommand(APNchar, "OK", 1000)) == 0 );
	SIM800.println("AT+SAPBR=1,1");
	delay(1000);
	SIM800.println("AT+SAPBR=2,1");
	delay(1000);
	clrSIMbuffer();
	while( (sendATcommand("AT+HTTPINIT", "OK", 3000))|| (sendATcommand("AT+HTTPINIT", "ERROR", 3000))== 0 );
	//while( (sendATcommand("AT+HTTPINIT", "OK", 3000)));
		return 1;
}

String GSM_modul::sendDataGPRS(String dataToSend)
{
	int dataLength;
	dataToSend = "\""+dataToSend+"\"";
	delay(10);
	clrSIMbuffer();
	SIM800.println("AT+HTTPPARA=\"URL\"," + dataToSend);
	while (SIM800.find("OK"));
	delay(10);
	clrSIMbuffer();
	while( (sendATcommand("AT+HTTPACTION=0", "+HTTPACTION:", 10000)) == 0 );
	delay(10);
	clrSIMbuffer();
	//read response
	SIM800.println("AT+HTTPREAD");
	delay(500);
	char gsmc;
	String content="";	
	if(SIM800.find("+HTTPREAD:"))
	{
		while(SIM800.available()>0)
		{
			gsmc = SIM800.read();
			content += gsmc; 
			if (gsmc == '\n') 
			{
				content = content.substring(0,content.length()-2);
				dataLength = content.toInt();
				content = "";
				break;
			}
		}
		//read content
		delay(100);
		while(SIM800.available()>0)
		{
			gsmc = SIM800.read();
			content += gsmc;
			dataLength --;
			if (dataLength < -1)
			{
				content = content.substring(0,content.length()-2);
				content = "";
				break;
			}
		}
	
	}
	else
	{
		return "COMMUNICATION FAILURE";
	}
	delay(10);
    clrSIMbuffer();
	return content;
}


//Send AT command to SIM800
int8_t GSM_modul::sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout)
{
	SIM800.listen();
	//WDT_Restart(WDT);
	uint8_t x = 0, answer = 0;
	char response[100];
	unsigned long previous;

	memset(response, '\0', 100);    // Initialize the string

	delay(100);

	//clrSIMbuffer();
	SIM800.println(ATcommand);    // Send the AT command
								  //WDT_Restart(WDT);

	x = 0;
	previous = millis();

	// this loop waits for the answer
	do
	{
		if (SIM800.available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = SIM800.read();
			x++;
			// check if the desired answer  is in the response of the module
			if (strstr(response, expected_answer) != NULL)
			{
				answer = 1;
			}
		}
		// Waits for the asnwer with time out
	} while ((answer == 0) && ((millis() - previous) < timeout));

	//clrSIMbuffer();
	return answer;
}


//Send AT command to SIM800 with response
String GSM_modul::sendATcommandResponse(char* ATcommand, char* expected_answer, unsigned int timeout, unsigned int buf)
{
	SIM800.listen();
	//WDT_Restart(WDT);
	uint8_t x = 0, answer = 0;
	char response[150];
	unsigned long previous;
	String rest;

	memset(response, '\0', 100);    // Initialize the string

	delay(100);

	//clrSIMbuffer();
	SIM800.println(ATcommand);    // Send the AT command


	x = 0;
	previous = millis();
	//WDT_Restart(WDT);
	// this loop waits for the answer
	do
	{
		if (SIM800.available() != 0) {
			// if there are data in the UART input buffer, reads it and checks for the asnwer
			response[x] = SIM800.read();
			x++;
			// check if the desired answer  is in the response of the module
			if (strstr(response, expected_answer) != NULL)
			{
				answer = 1;
			}
		}
		// Waits for the asnwer with time out
	} while ((answer == 0) && ((millis() - previous) < timeout));

	//p?e?ti zbytek - max 20 byte
	memset(response, '\0', buf);    // Initialize the string
	delay(100);
	for (x = 0; x < buf; x++) response[x] = SIM800.read();
	//clrSIMbuffer();
	for (x = 0; x < buf; x++) rest += char(response[x]);
	delay(100);
	return rest;

}
//Check SIM800 if call or SMS is present
uint8_t GSM_modul::checkCallAndSMS()
{
	SIM800.listen();
	char g;
	String gcmd;
	uint8_t gindex;
	
	
	//check call presence
	
	while (SIM800.available()>0)
	{
		g = SIM800.read();
		gcmd += g;
		if (g == '\n')
		{
			gcmd = gcmd.substring(0, gcmd.length() - 2);
			gindex = gcmd.indexOf('+');
			if (gcmd.substring(gindex+1,gindex+6) == "CLIP:") 
			{
				
				//+CLIP: "420739822476"
				//cut +CLIP:
				//check if + is present and parse according this character
				gcmd = gcmd.substring(gindex+6);
				if (gcmd.indexOf('+') != -1)
				{
					gindex = gcmd.indexOf('+');
					gcmd = gcmd.substring((gindex + 1), (gindex + 13));
				}
				//parse according first " character
				else
				{
					gindex = gcmd.indexOf('"');
					gcmd = gcmd.substring((gindex + 1), (gindex + 13));
				}
				
				callInProgress = false;
				number = gcmd;
				return 1;
			}
			else 
			{
			//get timeStamp
			//actTime = timeStamp();
			return 0;
			}	
			
		}
	}
	
	gcmd = "";
	
	
	//test SMS presence
	clrSIMbuffer();
	SIM800.println("AT+CMGR=1");
	delay(100);
	//echo surpress
	while (SIM800.available()>0)
	{
		g = SIM800.read();
		gcmd += g;
		if (g == '\n')
		{			
			gcmd = "";
			break;
		}
	}

	//read first line with command response
	while (SIM800.available()>0)
	{
		g = SIM800.read();
		gcmd += g;
		if (g == '\n')
		{

			if (gcmd.substring(0, 2) == "OK") return 0;
			
			if (gcmd.substring(0, 5) == "+CMGR")
			{
								
				//first + "+CMGR"
				gindex = gcmd.indexOf('+');
				if (gindex < 0) return 0;
				//cut off first + character
				gcmd = gcmd.substring(gindex + 1);
				//sender number
				gindex = gcmd.indexOf('+');
				number = gcmd.substring(gindex + 1, gindex + 13);
				//read SMS content
				gcmd = "";
				delay(50);
				while (SIM800.available()>0)
				{
					g = SIM800.read();
					gcmd += g;
					if ((g == '\n') && (gcmd.length()>2))
					{
						gcmd = gcmd.substring(0, gcmd.length() - 2);
						sendATcommand("AT+CMGD=1", "OK", 2000);
						sendATcommand("AT+CMGD=1,4", "OK", 2000);
						clrSIMbuffer();
						SMScontent = gcmd;
						return 2;
					}
				}
				gcmd = "";
				sendATcommand("AT+CMGD=1", "OK", 2000);
				sendATcommand("AT+CMGD=1,4", "OK", 2000);
				clrSIMbuffer();
				//get timeStamp
				//actTime = timeStamp();
				return 0;
			}


		}
	}
	//get timeStamp
	//actTime = timeStamp();
	return 0;
}

//Call End
void GSM_modul::callEnd()
{
	sendATcommand("ATH","OK",2000);
	return;
}
//Make call
void GSM_modul::makeCall(String callNumber)
{
	SIM800.print("ATD ");
	SIM800.print(callNumber);
	SIM800.println(";");
	clrSIMbuffer();
}

//Get last Sender or A-party number
String GSM_modul::getNumber()
{
	return number;
}
//get SMS content
String GSM_modul::getSMSContent()
{
	return SMScontent;
}

//Get Provider Name
String GSM_modul::getProviderName()
{
	providerName = sendATcommandResponse("AT+COPS?", "+COPS:", 1000, 20);
	tempIndex = providerName.indexOf('"');
	providerName = providerName.substring(tempIndex + 1);
	tempIndex = providerName.indexOf('"');
	providerName = providerName.substring(0, tempIndex);
	return providerName;

}
//Get signal quality
String GSM_modul::getQuality()
{
	GSMsignal = sendATcommandResponse("AT+CSQ", "+CSQ:", 1000, 3);
	return GSMsignal;
}

//Send SMS
void GSM_modul::sendSMS(String number, String sms)
{	
	clrSIMbuffer();
	//number = "+420"+number;
	SIM800.println("AT+CMGS=\"" + number + "\"");
	delay(200);
	//toSerial();
	SIM800.println(sms);        // message to send
	delay(100);
	SIM800.write((char)26);	//CTRL+Z
	delay(100);
	SIM800.println();
	delay(100);
	sendATcommand("AT+CMGD=1", "OK", 2000);
	sendATcommand("AT+CMGD=1,4", "OK", 2000);
	sendATcommand("AT+CMGD=1", "OK", 2000);
	delay(500);
	clrSIMbuffer();	
}

//Time stamp from SIM800 - GSM network time
String GSM_modul::timeStamp()
{
	String ts = "";
	ts = sendATcommandResponse("AT+CCLK?", "+CCLK: \"", 1000, 20);
	//13/11/04,15:23:19+04
	//return ("20"+ts.substring(0,2)+ts.substring(3,5)+ts.substring(6,8)+ts.substring(9,11)+ts.substring(12,14)+ts.substring(15,17));
	return ts;
}
String GSM_modul::actualTime()
{
	return actTime;
}

//HW restart of SIM800
void GSM_modul::restartSIMHW()
{
	//HW restart
	digitalWrite(GSMReset, LOW);
	delay(400);
	digitalWrite(GSMReset, HIGH);
	delay(400);
}

//Clear SIM800 buffer
void GSM_modul::clrSIMbuffer()
{
	while (SIM800.available()>0)
	{
		delay(1);
		SIM800.read();
	}
}
//Check if SIM800 is attached to GSM
bool GSM_modul::isConnected()
{
	if ((sendATcommand("AT+CREG?", "+CREG: 0,1", 1000) ||
		sendATcommand("AT+CREG?", "+CREG: 0,5", 1000)) == 1) return true;
	else return false;
}
//Check if SIM800 is OK
bool GSM_modul::checkModul()
{
  if (sendATcommand("AT", "OK", 2000) == 1) 
  return true;
  else 
  return false;
}
