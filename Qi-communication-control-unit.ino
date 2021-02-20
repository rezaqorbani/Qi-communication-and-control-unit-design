/*
Created by: 
*/
#include "ByteGenerator.h"
#include "SignalGenerator.h"

//The delays (in millisecond) for the signal in Qi-spec. 
namespace QiDelays
{
  const int t_wake = 60; 
  const int t_silent = 7; 
  const int t_received; 
  const int t_window;
  const int t_control;
  const int t_offset;
  const int powerValueDelay = 1; // delay for taking a power sample

};

namespace Pins
{
  const int modComPin;  //Digital
  const int onOffSwitchPin; //Digital
  const int powerLevelSwitch; //Digital
  const int rectifiedVoltagePin; //Analog
  const int shuntPin1; //Analog
  const int shuntPin2; //Analog
};

namespace Signals
{
  //--------------------Signals--------------------
  SignalGenerator signalStrengthPacket(0x01);
  SignalGenerator identificationPacket(0x71);
  SignalGenerator configurationPacket(0x51);
  SignalGenerator powerControlHoldoffPacket(0x06);
  SignalGenerator controlErrorPacket(0x03); 
  SignalGenerator receivedPowerPacket(0x04);
  SignalGenerator endPowerTransfer(0x02); 
};


bool currentPowerLevelState; //Den ska väl inte vara konstant, vi ändrar den ju


// Sends a given signal
// behöver lägga in en variabel med sista statet från förra loopen av koden prev_last_state
// Om vi skickar en signal när KomMod varit avstängd skall prev_last_state=FALSE
// Pin LOW = switch öppen 
// Pin HIGH = switch stängd 
// Med utgång i att en HIGH pin ger en LOW på primärsida och LOW pin her HIGH på primärsidan 
// När komMod är avstängd är pin LOW (dvs switch öppen)
// För bools är TRUE = HIGH och FALSE=LOW
bool sendSignal(SignalGenerator signal) 
{
  bool current_state;
  if (digitalRead(Pins::modComPin)==HIGH){
    current_state = true;
  }
  else{
    current_state = false;
  }
  
  for (int i = 0; i < signal.getSignalSize(); i++)
  {
    if( signal.getSignal()[i] == '1')
    {
      if(current_state){
        //observera att syntax för arduinoswitches är fel
        digitalWrite(Pins::modComPin, LOW);
        delay(0.25);
        digitalWrite(Pins::modComPin, HIGH);
        delay(0.25);
        //Switch twice, from HIGH->LOW->HIGH
        current_state=true;
      }
      else{
        digitalWrite(Pins::modComPin, HIGH);
        delay(0.25);
        digitalWrite(Pins::modComPin, LOW);
        delay(0.25);
        //Switch twice, from LOW->HIGH->LOW
        current_state=false;
        }
      }
    else if(signal.getSignal()[i] == '0')
    {
      if(current_state){
        //observera att syntax för arduinoswitches är fel
        digitalWrite(Pins::modComPin, LOW);
        delay(0.5);
        //Switch once, from HIGH->LOW
        current_state=false;
      }
      else{
        digitalWrite(Pins::modComPin, HIGH);
        delay(0.5);
        //Switch once, from LOW->HIGH
        current_state=true;
        }
        
    }
  }

}



bool checkPowerAndSwitches() // vet inte riktigt hur den ska implementeras eller vart den ska komma ifrån. kanske en global variabel?
{ 
  // checks if the Primary Cell current amplitude crosses 50% of the stable level
  //A method that should be called repeteadly, to ensure that we do not miss a signalchange. FIX IDK idk vad det ska ha för type
  //the value below can be changed, idk what is right
  //If the ifstatment below is fullfilled, it means we have a power signal, and the system should boot up. 
  int X;//defines the lower limit of our recitified current of the power signal and which level is considered on/off
  //dessa kan ev. ersättas av ett fett if(X or Y or Z), vet inte vilket som blir bäst 
  if(analogRead(Pins::rectifiedVoltagePin)<X){
    //No more power signal = return to selection phase/turn off
    return false;
    }
  else if(digitalRead(Pins::onOffSwitchPin)==LOW){//Assuming LOW=on off position
    //System has been turned off, so disconnect the output to the load
    return false;
    }
  //Nedan behöver fixas - digitalRead returnerar ju ett HIGH eller LOW vi måste veta vilken som är vilken
  else if(digitalRead(Pins::powerLevelSwitch)!=currentPowerLevelState){//måste veta current state, vi får hitta på en variabel som jag nu döpt till powerLevelSwitchState, och definiera den. om det har förändrats ska:
    //Reboot with new configuration, send EndPowerTransfer
    return false;
    }
  else{
    return true;
  }
}


//conver int to binary
char* intToBinary(int n)
{
  char ret [8];
  int index = 0; 
  while(n!=0) {ret[index] = (n%2==0 ?'0':'1'); index++; n/=2;}
  return ret;
}




bool oneOrHalfWatt()
{
  //checks the button on arduino that chooses between 1 and 0.5 watt power transfer. 
  //if true send 1 watt
  //else send 0.5 watt
  int stateButton = digitalRead(Pins::powerLevelSwitch); //Vet inte om det här stämmer, är HIGH=TRUE här eller?
  return bool(stateButton); 

}
// starts the components in the 

double calculatePower()
{
  const double valueShunt = 0.5;

  double inputValue = analogRead(Pins::shuntPin1) - analogRead(Pins::shuntPin2);

  double voltage = inputValue * 5.0 / 1024.0;
  double current = voltage / valueShunt;
  double power = pow(voltage, 2) / valueShunt;
  return power;
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

}

void loop() 
{   
// put your main code here, to run repeatedly:
//BEGIN selection phase

  try{
      
      if(checkPowerAndSwitches())
      {
        //BEGIN ping phase
        delay(QiDelays::t_wake); 
        if(digitalRead(Pins::onOffSwitchPin)){
          Signals::endPowerTransfer.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','1'));
          sendSignal(Signals::endPowerTransfer);
        }
        else{
        int maxValue = 5; //
        int signalStrengthValue = (analogRead(Pins::rectifiedVoltagePin)*5/1024/maxValue*256);
        if(signalStrengthValue >256){
          signalStrengthValue = 255;
        }
        char* signalStrengthValueBinary = intToBinary(signalStrengthValue);
        
        Signals::signalStrengthPacket.setMessageIndex(0, ByteGenerator(signalStrengthValueBinary));
        sendSignal(Signals::signalStrengthPacket); 
        }


        //BEGIN ID & Config phase 
        if(checkPowerAndSwitches())
        {
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
          
          //Kanske följande
          //SignalGenerator PowerControlHoldoffPacket(0x06);
          //sendSignal(PowerControlHoldoffPacket); // Om vi vill ha en delay på reaktion från sändaren när vi ber om en ändrad spänning
          //delay(qiDelays::t_silent);
          //Garanterat följande
          //char* config_array;//osäker hur man initializar arrays 
          //config_array[7] = 0;
          //config_array[6] = 0;
          // 5-0 ska vara maximum power value
          if(oneOrHalfWatt()){
            currentPowerLevelState = true;
            Signals::configurationPacket.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','1','0'));


          }
          else{
            currentPowerLevelState = false;
            Signals::configurationPacket.setMessageIndex(0, ByteGenerator('0','0','0','0','0','1','0','0'));

            
          }
          Signals::configurationPacket.setMessageIndex(1, ByteGenerator('0','0','0','0','0','0','0','0'));//Reserved
          Signals::configurationPacket.setMessageIndex(2, ByteGenerator('0','0','0','0','0','0','0','0'));
          Signals::configurationPacket.setMessageIndex(3, ByteGenerator('0','1','0','1','0','0','0','0'));//Skall ändras detta är window size samt window offset
          Signals::configurationPacket.setMessageIndex(4, ByteGenerator('0','0','0','0','0','0','0','0'));
          sendSignal(Signals::configurationPacket);
          delay(QiDelays::t_silent);

        }
        //END ID & Config phase




          //BEGIN power transfer phase
          
          while(checkPowerAndSwitches())
          {

            delay(QiDelays::t_silent);
            bool current_power = oneOrHalfWatt(); //if true 1 watt, else 0.5 watt
            bool cont = true;
            

            // This if else claus is for changing the power level (0.5W to 1W)
            if(current_power)
            {
              //if want to recieve one watt
              //Change the value of the message
              //Signals::signalStrengthPacket.setMessageIndex(0,ByteGenerator('0', '0', '0', '0', '1', '0', '0','0')); 
              while(calculatePower() < 1)
              {
                sendSignal(Signals::controlErrorPacket);  
              } 
              
            }
            else if (!current_power)
            {
              //if want to recieve half watt
              //Change the value of the message
             // Signals::signalStrengthPacket.setMessageIndex(0,ByteGenerator('1', '1', '0', '0', '0', '0', '0','0')); 
              while(calculatePower() < 1)
              {
                sendSignal(Signals::controlErrorPacket);  
              }   

            }

            //The values below are preliminary and have to be changed in order 
            double powerValues [10]; 
            for(int i = 0; i < 10; i)
            {
              powerValues[i] = calculatePower();
              delay(1); 
            }

            double averagePower = 0;  
            for(int i = 0; i < 10; i)
            {
              averagePower += powerValues[i]; 
            }


            Signals::receivedPowerPacket.setMessageIndex(0,ByteGenerator(intToBinary(averagePower)));

            bool after_power =  oneOrHalfWatt(); 
            delay(QiDelays::t_silent);
          }
          //END power transfer phase
        }
      }
    

  catch(...) //If we cant start the system or something else happens
    {
      
      while(checkPowerAndSwitches()) //makes sure the transmitter has received the EndPowerTransfer
      {
        sendSignal(Signals::endPowerTransfer); 
      }
    
    }
    //END ping phase  
    
//END selection phase
  
}


