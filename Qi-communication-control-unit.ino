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
	const int powerValueDelay = 1; // delay for taking a power sample

}

namespace Pins
{
    const int modComPin = 2; 
    const int rectifiedVoltagePin = A1; 
    const int onOffSwitchPin = 3; 
    const int powerLevelSwitch = 4;
    const int shuntPin1 = A2;
    const int shuntPin2 = A3;
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


bool checkPing() // vet inte riktigt hur den ska implementeras eller vart den ska komma ifrån. kanske en global variabel?
{ 
  
	float lowerLimitVoltage = 0.1;//defines the lower limit of our recitified current of the power signal and which level is considered on/off
	//int sensorValue = 0;

	int sensorValue = analogRead(Pins::rectifiedVoltagePin); 
	float voltage = sensorValue * (5.0 / 1024.0);
	
	pingVoltage = voltage; 
	return (voltage >= lowerLimitVoltage);
   
}

bool checkOnSwitch()
{
	bool isOn = digitalRead(Pins::onOffSwitchPin);
	//return isOn;
	return true; //ÄNDRA sätt dedär kommentaren ovan
}

//conver int to binary
char binaryCharArray[8] = {'0', '0','0', '0','0', '0','0', '0'}; 
void intToBinary(int n)
{
	int index = 7; 
	while(n!=0) {binaryCharArray[index] = (n%2==0 ?'0':'1'); index--; n/=2;}
}

bool oneOrHalfWatt()
{
	return (digitalRead(Pins::powerLevelSwitch)); //Vet inte om det här stämmer, är HIGH=TRUE här eller?
}

double calculateVoltage()
{
	double inputValue = analogRead(Pins::shuntPin1) - analogRead(Pins::shuntPin2);

	double voltage = inputValue * 5.0 / 1024.0;

	return voltage; 
}

double calculatePower()
{
	const double valueShunt = 1;
	double voltage = calculateVoltage();
	double current = voltage / valueShunt;
	double power = pow(voltage, 2) / valueShunt;
	return power;
}

void setReceivedPowerMessage()
{
	
	double powerValues [8]; 
	for(int i = 0; i < 8; i++)
	{
		Serial.println(i);
		powerValues[i] = calculatePower();
		delayMicroseconds(900); 
	}
	
	

	double powerReceived = 0;  
	for(int i = 0; i < 8; i++)
	{
		powerReceived += powerValues[i]; 
	}
	powerReceived = powerReceived/8; 
	int receivedPowerValue = (receivedPowerValue*128);
	intToBinary(receivedPowerValue);
	Signals::receivedPowerPacket.setMessageIndex(0, ByteGenerator(binaryCharArray)); 
	
}

void pingPhase()
{
	delay(QiDelays::t_wake); 
	if(digitalRead(Pins::onOffSwitchPin)) //ÄNDRA sätt ett !
	{
		
		Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
		sendSignal(Signals::endPowerTransfer);
		return; 
	}

	else{
		
		double maxValue = 5.0; 

		// int sensorValue = analogRead(Pins::rectifiedVoltagePin);
		// double voltage = analogRead(Pins::rectifiedVoltagePin)*5/1024; 


		int signalStrengthValue = (pingVoltage/maxValue)*256;

		if(signalStrengthValue >256)
		{
			signalStrengthValue = 255;
		}
		intToBinary(255);
		Signals::signalStrengthPacket.setMessageIndex(0, ByteGenerator(binaryCharArray));
		sendSignal(Signals::signalStrengthPacket); 
		delay(7);
	}
}

void idConfigPhase()
{
	delay(2);

	//Not completed
	Signals::identificationPacket.setMessageIndex(0, ByteGenerator('0','0','0','1','0','0','1','0'));//Major/Minor version
	Signals::identificationPacket.setMessageIndex(1, ByteGenerator('0','0','0','0','0','0','0','0'));//Påhittad manufacturer code del1
	Signals::identificationPacket.setMessageIndex(2, ByteGenerator('0','0','0','0','0','0','0','1'));
	//dessa nedan kan behöva ändras, beror på om vi behöver randomiza grejer
	Signals::identificationPacket.setMessageIndex(3, ByteGenerator('1','0','0','0','0','0','0','0'));
	Signals::identificationPacket.setMessageIndex(4, ByteGenerator('0','0','0','0','0','0','0','0'));
	Signals::identificationPacket.setMessageIndex(5, ByteGenerator('0','0','0','0','0','0','0','0'));
	Signals::identificationPacket.setMessageIndex(6, ByteGenerator('0','0','0','0','1','1','1','1'));


	sendSignal(Signals::identificationPacket);
	delay(QiDelays::t_silent);

		
	Signals::configurationPacket.setMessageIndex(0, ByteGenerator('0','0','0','0','0','1','0','0'));
	Signals::configurationPacket.setMessageIndex(1, ByteGenerator('0','0','0','0','0','0','0','0'));//Reserved
	Signals::configurationPacket.setMessageIndex(2, ByteGenerator('0','0','0','0','0','0','0','0'));
	Signals::configurationPacket.setMessageIndex(3, ByteGenerator('0','0','0','1','0','0','0','1'));//window size 8 samt window offset 4
	Signals::configurationPacket.setMessageIndex(4, ByteGenerator('0','0','0','0','0','0','0','0'));

	sendSignal(Signals::configurationPacket);

}

void powerTransfer()
{
	
	delay(90); 
	//bool current_power = oneOrHalfWatt(); //if true 1 watt, else 0.5 watt
	
	
	Signals::controlErrorPacket.setMessageIndex(0,ByteGenerator('0', '0', '0', '0', '1', '0', '0','0')); 
	sendSignal(Signals::controlErrorPacket);  
	delay(40); 
	
	for(int i = 0; i < 28 ; i++)
	{
		sendSignal(Signals::controlErrorPacket);
		delay(40);
	}
	
	setReceivedPowerMessage();
	 
	delay(QiDelays::t_offset); 
	sendSignal(Signals::receivedPowerPacket); 
	delay(40); 
	

	// This if else claus is for changing the power level (0.5W to 1W)
	
	while(checkPing()&&checkOnSwitch())
	{
		bool current_power = oneOrHalfWatt(); 
		int index = 0; 
		float currentOneWatt = 1/5; 
		float currentHalfWatt = 1/10; 
		int controlErrorValue = 0; 
		float desiredCurrent = ((current_power)?currentOneWatt:currentHalfWatt);
		
		if(current_power)
		{
		//if want to recieve one watt
		//Change the value of the message
		//1/5 amp för 1 for one watt
		//1/10 amp för 0.5 watt 
		//t-d = t-a(1+c/128)
		//t-d = t-a + t-a(c/128)
		//t-d - t-a = t-a(c/128)
		//128(t-d/t-a - 1) = c
		
		intToBinary(controlErrorValue); 
			while((0.9 > calculatePower()) && (index != 28))
			{	
				float presentCurrent = calculateVoltage(); // divided by 1 because it is the value of shunt (not shown here)
				int controlErrorValue = 128*((desiredCurrent/presentCurrent) - 1);
				intToBinary(controlErrorValue); 
				Signals::controlErrorPacket.setMessageIndex(0,ByteGenerator(binaryCharArray)); 
				sendSignal(Signals::controlErrorPacket);  
				index++;
				delay(40); 
			} 

		}
		else if (!current_power)
		{
		//if want to recieve half watt
		//Change the value of the message
		// Signals::signalStrengthPacket.setMessageIndex(0,ByteGenerator('1', '1', '0', '0', '0', '0', '0','0')); 
			while((calculatePower() < 0.5)&&index != 28)
			{
				float presentCurrent = calculateVoltage(); // divided by 1 because it is the value of shunt (not shown here)
				int controlErrorValue = 128*((desiredCurrent/presentCurrent) - 1);
				Signals::controlErrorPacket.setMessageIndex(0,ByteGenerator(binaryCharArray)); 
				sendSignal(Signals::controlErrorPacket);  
				index++; 
				delay(40);
			}   
		}
		//The values below are preliminary and have to be changed in order 
		setReceivedPowerMessage(); 
		delay(QiDelays::t_offset); 
		sendSignal(Signals::receivedPowerPacket); 
		delay(40);
		current_power = oneOrHalfWatt();
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
	pinMode(Pins::shuntPin1, INPUT);
	pinMode(Pins::shuntPin2, INPUT); 
	Serial.begin(9600); 
}


void loop() 
{   
// put your main code here, to run repeatedly:
//BEGIN selection phase
	
  
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


			if(checkPing() && checkOnSwitch())
			{
				//END power transfer phase
				
				
				powerTransfer();
				
				if(!checkOnSwitch())
				{
					Serial.println("End on switch");
					while(checkPing())
					{
						Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
						sendSignal(Signals::endPowerTransfer);
						return; 
					}
				}
				else
				{
					Serial.println("End on unknown");
					while(checkPing())
					Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','0'));
					sendSignal(Signals::endPowerTransfer);
					return; 
				}
				//BEGIN power transfer phase
			}
	
		}
		
	}
    
//END selection phase
}

