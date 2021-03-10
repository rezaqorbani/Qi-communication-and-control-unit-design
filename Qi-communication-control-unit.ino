/*
Created by: Reza Qorbani & Tova Stroeven
*/
#include "ByteGenerator.h"
#include "SignalGenerator.h"


//The delays (in millisecond) for the signal in Qi-spec. 
namespace QiDelays
{
	const int t_wake = 40; 
	const int t_silent = 7; 
	const int t_window = 8; //unit of 4ms, total 8 ms
	const int t_offset = 4; //unit of 4ms, total 4ms

}

namespace Pins
{
    const int modComPin = 2; 
    const int rectifiedVoltagePin = A4; 
    const int onOffSwitchPin = 3; 
    const int powerLevelSwitch = 4;
	const int outputConnect = 5; //When HIGH, Output connect
	const int actualLoadConnect = 6; //When HIGH actual load and not dummy load is connected
	const int loadLed1 = 7; 
	const int loadLed2 = 8;
};

namespace Signals
{
	SignalGenerator signalStrengthPacket(0x01);
	SignalGenerator identificationPacket(0x71);
	SignalGenerator configurationPacket(0x51);
	SignalGenerator powerControlHoldoffPacket(0x06);
	SignalGenerator controlErrorPacket(0x03); 
	SignalGenerator receivedPowerPacket(0x04);
	SignalGenerator endPowerTransfer(0x02); 
}


float pingVoltage;
char binaryCharArray[8] = {'0', '0','0', '0','0', '0','0', '0'}; 


void sendSignal(SignalGenerator signal) 
{
	bool signalState = 0;
	char* signalArray = signal.getSignal();
	int signalSize = signal.getSignalSize();

	for (int i = 0; i < signalSize; i++)
	{

		
		signalState = !signalState;
		digitalWrite(Pins::modComPin, signalState); 
		delayMicroseconds(250);

		if(signalArray[i] == '1')
		{
			signalState = !signalState;
		}
		digitalWrite(Pins::modComPin, signalState);
		delayMicroseconds(250);

	}
	
	digitalWrite(Pins::modComPin, LOW); 
	delete[] signalArray;
	signalArray = nullptr;
	return; 
}


bool checkPing() 
{ 
	float lowerLimitVoltage = 1.4;//defines the lower limit of our recitified current of the power signal and which level is considered high enough ping	
	double voltage = checkRectifiedAverage();
	//pingVoltage = voltage; 
	return (voltage >= lowerLimitVoltage);
}
bool checkRectified(){
	float lowerLimitVoltage = 0.5;//defines the lower limit of our recitified current of the power signal and which level is considered on/off
	double voltage = checkRectifiedAverage();
	return (voltage >= lowerLimitVoltage);
}

bool checkOnSwitch()
{ 
	return (digitalRead(Pins::onOffSwitchPin));
}

//conver int to binary
void intToBinary(int n)
{
	int index = 7; 
	while(n!=0) {binaryCharArray[index] = (n%2==0 ?'0':'1'); index--; n/=2;}
	
}


bool oneOrHalfWatt()
{
	return (digitalRead(Pins::powerLevelSwitch)); 
}

double calculatePower()//Approximate value of the power based upon known resistance - this is ONLY used for Recivied Power Packet
{
	const double valueShunt = 1.2;
	double loadVoltage = 3*checkRectifiedAverage();
	double power = loadVoltage*loadVoltage/84;//84 ohms is an approximation of the resistance of the load
	return power;
}

double checkRectifiedAverage()
{
	double totalVoltage = 0.0;
	for(int i = 0; i < 10; i++){
		totalVoltage = totalVoltage + analogRead(Pins::rectifiedVoltagePin); 
	}
	double voltage = totalVoltage/10.0 * (5.0 / 1024.0);
	return voltage;
}
void setReceivedPowerMessage()
{
	double powerValues [8]; 
	for(int i = 0; i < 8; i++)
	{
		powerValues[i] = calculatePower();
		delayMicroseconds(900); 
	}
	
	double powerReceived = 0;  
	for(int i = 0; i < 8; i++)
	{
		powerReceived += powerValues[i]; 
	}
	powerReceived = powerReceived/8; 
	int receivedPowerValue = (powerReceived*128);
	intToBinary(receivedPowerValue);
	Signals::receivedPowerPacket.setMessageIndex(0, ByteGenerator(binaryCharArray)); 
	
}


void pingPhase()
{
	delay(QiDelays::t_wake); 

	if(digitalRead(Pins::onOffSwitchPin)==0) 
	{	
		Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
		sendSignal(Signals::endPowerTransfer);
		return; 
	}

	else{
		/*
		//Alternative code below, this is more dynamic but does not really fulfill a purpose with this particular 
		// transmitter. With a more diverse system it could be used in place of what is currently used
		double maxValue = 2.0; //adapt depending on max ping value
		int signalStrengthValue = (pingVoltage/maxValue)*256;
		if(signalStrengthValue >256)
		{
			signalStrengthValue = 255;
		}
		intToBinary(signalStrengthValue);
		*/
		intToBinary(255);
		Signals::signalStrengthPacket.setMessageIndex(0, ByteGenerator(binaryCharArray));
		sendSignal(Signals::signalStrengthPacket); 
		delay(7);
	}
}

void idConfigPhase()
{
	delay(2);

	Signals::identificationPacket.setMessageIndex(0, ByteGenerator('0','0','0','1','0','0','1','0'));//Major/Minor version
	Signals::identificationPacket.setMessageIndex(1, ByteGenerator('0','0','0','0','0','0','0','0'));//manufacturer code del1
	Signals::identificationPacket.setMessageIndex(2, ByteGenerator('0','0','0','0','0','0','0','1'));
	//device identifier
	Signals::identificationPacket.setMessageIndex(3, ByteGenerator('1','0','0','0','0','0','0','0'));
	Signals::identificationPacket.setMessageIndex(4, ByteGenerator('0','0','0','0','0','0','0','0'));
	Signals::identificationPacket.setMessageIndex(5, ByteGenerator('0','0','0','0','0','0','0','0'));
	Signals::identificationPacket.setMessageIndex(6, ByteGenerator('0','0','0','0','1','1','1','1'));


	sendSignal(Signals::identificationPacket);
	delay(QiDelays::t_silent);

		
	Signals::configurationPacket.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
	Signals::configurationPacket.setMessageIndex(1, ByteGenerator('0','0','0','0','0','0','0','0'));//Reserved
	Signals::configurationPacket.setMessageIndex(2, ByteGenerator('0','0','0','0','0','0','0','0'));
	Signals::configurationPacket.setMessageIndex(3, ByteGenerator('0','0','0','1','0','0','0','1'));//window size 8 samt window offset 4
	Signals::configurationPacket.setMessageIndex(4, ByteGenerator('0','0','0','0','0','0','0','0'));

	sendSignal(Signals::configurationPacket);

}

void powerTransfer()
{
	
	bool loadDisconnected = true; 
	digitalWrite(Pins::outputConnect,HIGH);
	delay(90);
		
	while(checkRectified()&&checkOnSwitch())
	
	{
		int index = 0; 
		if(oneOrHalfWatt()){//power level 1W
			digitalWrite(Pins::loadLed1,HIGH);
			digitalWrite(Pins::loadLed2,HIGH);
		}
		else{//Power level 0.5W
			digitalWrite(Pins::loadLed1,LOW); 
			digitalWrite(Pins::loadLed2,HIGH);
		}
		if(3*checkRectifiedAverage()>5.5 && loadDisconnected)//Sufficiently high voltage to connect our load
		{
			digitalWrite(Pins::actualLoadConnect,HIGH);
			loadDisconnected = false;
		}
		while(checkRectified() && index <= 28)
		{	
			if(checkRectifiedAverage()*3>25){//Too much voltage, request a decrease (should in practice never occurr)
				Signals::controlErrorPacket.setMessageIndex(0, ByteGenerator('1','0','0','0','0','0','0','0'));
				sendSignal(Signals::controlErrorPacket);  
				index++;
				delay(40); 

			}
			else{//Usually we need to request continusly more power since our system uses enough power to function, we just need to provide enough
				Signals::controlErrorPacket.setMessageIndex(0, ByteGenerator('0','1','1','1','1','1','1','1'));
				sendSignal(Signals::controlErrorPacket);  
				index++;
				delay(40); 
			}
		} 		
		if(checkRectified()){
		setReceivedPowerMessage(); 
		delay(QiDelays::t_offset);
		sendSignal(Signals::receivedPowerPacket); 
		delay(40);
		}
	}
          
}

void setup()
{
	// put your setup code here, to run once:
	// Här definierar vi alla Pins som ska användas
	pinMode(Pins::modComPin, OUTPUT);
	pinMode(Pins::onOffSwitchPin,INPUT);
	pinMode(Pins::powerLevelSwitch, INPUT);
	pinMode(Pins::rectifiedVoltagePin, INPUT);
	pinMode(Pins::outputConnect, OUTPUT);
	pinMode(Pins::actualLoadConnect, OUTPUT);
	pinMode(Pins::loadLed1, OUTPUT);
	pinMode(Pins::loadLed2, OUTPUT);
	Serial.begin(9600); 
	digitalWrite(Pins::outputConnect,LOW);
	//digitalWrite(Pins::outputConnect,HIGH);
	if(digitalRead(Pins::powerLevelSwitch)){
		digitalWrite(Pins::loadLed1,HIGH);//Ändra
		digitalWrite(Pins::loadLed2,HIGH);
	}
	else{
		digitalWrite(Pins::loadLed1,LOW);
		digitalWrite(Pins::loadLed2,HIGH);
	}

}


void loop() 
{   
// put your main code here, to run repeatedly:
//BEGIN selection phase
	
	digitalWrite(Pins::actualLoadConnect, LOW);
	digitalWrite(Pins::outputConnect, LOW); //Disconnect the output
	
	//digitalWrite(Pins::outputConnect,HIGH);

	if(checkPing() && checkOnSwitch())
	{
	//Begin ping phase 	
		pingPhase();
	//END ping phase  

	
	
		if(checkOnSwitch())
		{
			//BEGIN ID & Config phase 
		
			idConfigPhase();
			//END ID & Config phase


			if(checkRectified() && checkOnSwitch())
			{
				//BEGIN power transfer phase
				
				
				powerTransfer();
				
				if(!checkOnSwitch())
				{
					while(checkRectified())
					{
						Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
						sendSignal(Signals::endPowerTransfer);
						return; 
					}
				}
				else
				{
					while(checkRectified()){
					Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','0'));
					sendSignal(Signals::endPowerTransfer);
					return; 
					}
				}
				//BEGIN power transfer phase
			}
	
		}
		
	}
    
//END selection phase
}

