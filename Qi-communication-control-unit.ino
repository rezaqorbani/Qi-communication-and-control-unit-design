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
    const int rectifiedVoltagePin = A4; 
    const int onOffSwitchPin = 3; 
    const int powerLevelSwitch = 4;
	const int outputConnect = 5; //When HIGH, Output connect
	const int loadLed = 6; // If HIGH = 1W if LOW=0.5W
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
  
	float lowerLimitVoltage = 0.5;//defines the lower limit of our recitified current of the power signal and which level is considered on/off
	//int sensorValue = 0;

	int sensorValue = analogRead(Pins::rectifiedVoltagePin); 
	float voltage = sensorValue * (5.0 / 1024.0);
	
	/*
	if(voltage>=lowerLimitVoltage){
		Serial.println(voltage);
	}
	*/
	
	
	
	pingVoltage = voltage; 
	return (voltage >= lowerLimitVoltage);
   //return false;
}

bool checkOnSwitch()
{
	
	bool isOn = digitalRead(Pins::onOffSwitchPin);
	return isOn; //ÄNDRA sätt dedär kommentaren ovan
	
}

//conver int to binary
char binaryCharArray[8] = {'0', '0','0', '0','0', '0','0', '0'}; 
void intToBinary(int n)
{
	int index = 7; 
	while(n!=0) {binaryCharArray[index] = (n%2==0 ?'0':'1'); index--; n/=2;}
	
}

void intToTwosComplement(int number)
{
	int copy = number; 
	int index = 7; 
	while(number!=0) {binaryCharArray[index] = (number%2==0 ?'0':'1'); index--; number/=2;}
	if(copy < 0)
	{
		int i; 
		int n = 8; 
		for (i = n-1 ; i >= 0 ; i--) 
			if (binaryCharArray[i] == '1') 
				break; 
	
		// If there exists no '1' concatenate 1 at the 
		// starting of string 
		if (i == -1) 
		{
		binaryCharArray[7] = '1'; 
			return; 
		}
		
	
		// Continue traversal after the position of 
		// first '1' 
		for (int k = i-1 ; k >= 0; k--) 
		{ 
			//Just flip the values 
			if (binaryCharArray[k] == '1') 
				binaryCharArray[k] = '0'; 
			else
				binaryCharArray[k] = '1'; 
		} 
	}	
	
    // return the modified string 
    return; 
} 


bool oneOrHalfWatt()
{
	return (digitalRead(Pins::powerLevelSwitch)); 
}

double calculateVoltage()
{
	double inputValue = analogRead(Pins::shuntPin1) - analogRead(Pins::shuntPin2);
	
	double voltage = inputValue * 5.0 / 1024.0;

	return voltage; 
}

double calculatePower()
{
	const double valueShunt = 1.0;
	double voltage = calculateVoltage();
	double power = (voltage*voltage) / valueShunt;
	return power;
}

void checkRectified()
{
	int sensorValue = analogRead(Pins::rectifiedVoltagePin); 
	double voltage = sensorValue * (5.0 / 1024.0);
	Serial.println(voltage);
}
void setReceivedPowerMessage()
{
	//if want to recieve one watt
	//Change the value of the message
	//1/5 amp för 1 for one watt
	//1/10 amp för 0.5 watt 
	//t-d = t-a(1+c/128)
	//t-d = t-a + t-a(c/128)
	//t-d - t-a = t-a(c/128)
	//128(t-d/t-a - 1) = c

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

	if(digitalRead(Pins::onOffSwitchPin)==0) //ÄNDRA sätt ett !
	{	
		Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
		sendSignal(Signals::endPowerTransfer);
		return; 
	}

	else{
		
		double maxValue = 2.0; 
		

		// int sensorValue = analogRead(Pins::rectifiedVoltagePin);
		// double voltage = analogRead(Pins::rectifiedVoltagePin)*5/1024; 


		int signalStrengthValue = (pingVoltage/maxValue)*256;
		

		if(signalStrengthValue >256)
		{
			signalStrengthValue = 255;
		}
		//intToBinary(255);
		intToBinary(signalStrengthValue);
		
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
	digitalWrite(Pins::outputConnect,HIGH);
	
	// This if else claus is for changing the power level (0.5W to 1W)
	
	while(checkPing()&&checkOnSwitch())
	{
		
		int index = 0; 
		double powerLevel; 

		
		if(oneOrHalfWatt())
		{
			powerLevel = 1.0;
		}
		else
		{
			powerLevel = 0.5; 
		}
	
		
		 
		while(checkPing() && index <= 28)
		{	

			int controlErrorValue = (100 * (powerLevel - calculatePower() )); 
			controlErrorValue = controlErrorValue % 128; 
			intToTwosComplement(controlErrorValue);	
			Signals::controlErrorPacket.setMessageIndex(0,ByteGenerator(binaryCharArray)); 
			//Signals::controlErrorPacket.setMessageIndex(0, ByteGenerator('0','1','1','1','1','1','1','1'));
			sendSignal(Signals::controlErrorPacket);  
			index++;
			checkRectified();
			delay(35); 
		} 
		
		
		//The values below are preliminary and have to be changed in order 
		if(checkPing()){
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
	pinMode(Pins::loadLed, OUTPUT);
	pinMode(Pins::shuntPin1, INPUT);
	pinMode(Pins::shuntPin2, INPUT); 
	Serial.begin(9600); 
	digitalWrite(Pins::outputConnect,LOW);
	digitalWrite(Pins::loadLed,digitalRead(Pins::powerLevelSwitch));//Ändra

}


void loop() 
{   
// put your main code here, to run repeatedly:
//BEGIN selection phase
	
	digitalWrite(Pins::outputConnect, LOW); //Disconnect the output
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
				//BEGIN power transfer phase
				
				
				powerTransfer();
				
				if(!checkOnSwitch())
				{
					while(checkPing())
					{
						Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
						sendSignal(Signals::endPowerTransfer);
						return; 
					}
				}
				else
				{
					while(checkPing()){
					Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','0'));
					sendSignal(Signals::endPowerTransfer);
					//Serial.println("signal sent");
					return; 
					}
				}
				//BEGIN power transfer phase
			}
	
		}
		
	}
    
//END selection phase
}

