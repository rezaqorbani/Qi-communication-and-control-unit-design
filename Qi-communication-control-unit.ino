/*
Created by: 
*/
#include "ByteGenerator.h"
#include "SignalGenerator.h"

//The delays (in millisecond) for the signal in Qi-spec. 
namespace qiDelays
{
  const int t_wake = 60; 
  const int t_silent = 7; 
};

namespace pins
{
  const int modComPin = 1;
  const int powerSignalPin  =  A0;//Ändra siffran beroende på vilken pin som faktiskt används (detta är en avläsningspin, analog)
  const int onOffSwitchPin  =  2;//Ändra siffran beroende på vilken pin som faktiskt används (detta är en avläsningspin, digital)
  const int powerSignalPin; 
  const int powerLevelSwitch; 
  const int rectifiedVoltagePin; 
};

const bool currentPowerLevelState; 


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
  if (digitalRead(pins::modComPin)==HIGH){
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
        digitalWrite(pins::modComPin, LOW);
        delay(0.25);
        digitalWrite(pins::modComPin, HIGH);
        delay(0.25);
        //Switch twice, from HIGH->LOW->HIGH
        current_state=true;
      }
      else{
        digitalWrite(pins::modComPin, HIGH);
        delay(0.25);
        digitalWrite(pins::modComPin, LOW);
        delay(0.25);
        //Switch twice, from LOW->HIGH->LOW
        current_state=false;
        }
      }
    else if(signal.getSignal()[i] == '0')
    {
      if(current_state){
        //observera att syntax för arduinoswitches är fel
        digitalWrite(pins::modComPin, LOW);
        delay(0.5);
        //Switch once, from HIGH->LOW
        current_state=false;
      }
      else{
        digitalWrite(pins::modComPin, HIGH);
        delay(0.5);
        //Switch once, from LOW->HIGH
        current_state=true;
        }
        
    }
  }

}


bool check_powsignal_switches() // vet inte riktigt hur den ska implementeras eller vart den ska komma ifrån. kanske en global variabel?
{ 
  // checks if the Primary Cell current amplitude crosses 50% of the stable level
  //A method that should be called repeteadly, to ensure that we do not miss a signalchange. FIX IDK idk vad det ska ha för type
  //the value below can be changed, idk what is right
  //If the ifstatment below is fullfilled, it means we have a power signal, and the system should boot up. 
  int X;//defines the lower limit of our recitified current of the power signal and which level is considered on/off
  //dessa kan ev. ersättas av ett fett if(X or Y or Z), vet inte vilket som blir bäst 
  if(analogRead(pins::powerSignalPin)<X){
    //No more power signal = return to selection phase/turn off
    return false;
    }
  else if(digitalRead(pins::onOffSwitchPin)==LOW){//Assuming LOW=on off position
    //System has been turned off, so disconnect the output to the load
    return false;
    }
  else if(digitalRead(pins::powerLevelSwitch)!=currentPowerLevelState){//måste veta current state, vi får hitta på en variabel som jag nu döpt till powerLevelSwitchState, och definiera den. om det har förändrats ska:
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


void pingPhase()
{
  // Här ska det egentligen in en ifsats
  /*
  if(everything is okay)
    send signalStrength
    check_powsignal_switches()
    procceed to Identification and Configuration 
  else(if we don't want power (fully charged/unknown))
    send EndPowerTransfer WITH reason
  */
  
  int maxValue; //Set correct value idk what, same type as what analogRead returns 
  int signalStrengthValue = (analogRead(pins::rectifiedVoltagePin)/maxValue*256);


  char* signalStrengthValueBinary = intToBinary(signalStrengthValue);  //vet inte hur det binära talet ska hamna när men ja
  SignalGenerator signalStrengthPacket(0x01);
  signalStrengthPacket.setMessageIndex(0, ByteGenerator(signalStrengthValueBinary[0],signalStrengthValueBinary[1],signalStrengthValueBinary[2],signalStrengthValueBinary[3],signalStrengthValueBinary[4],signalStrengthValueBinary[5],signalStrengthValueBinary[6],signalStrengthValueBinary[7]));
  sendSignal(signalStrengthPacket);
}

bool sendOneWatt()
{
  //checks the button on arduino that chooses between 1 and 0.5 watt power transfer. 
  //if true send 1 watt
  //else send 0.5 watt
}
// starts the components in the 



void setup()
{
  // put your setup code here, to run once:
  // Här definierar vi alla pins som ska användas

  //const int modComPin = pinMode(1, OUTPUT);//Number here to correspond to the modComPin
  //const int powerSignalPin  =  A0;//Ändra siffran beroende på vilken pin som faktiskt används (detta är en avläsningspin, analog)
  //const int onOffSwitchPin  =  2;//Ändra siffran beroende på vilken pin som faktiskt används (detta är en avläsningspin, digital)

  //--------------------Signals--------------------
  SignalGenerator signalStrengthPacket(0x01);
  SignalGenerator IdentificationPacket(0x71);
  SignalGenerator ConfigurationPacket(0x51);
  SignalGenerator PowerControlHoldoffPacket(0x06);
  SignalGenerator controlErrorPacket(0x03); 
  SignalGenerator receivedPowerPacket(0x04);
  SignalGenerator endPowerTransfer(0x02); 


}


void loop() 
{   
// put your main code here, to run repeatedly:
//BEGIN selection phase


  if (check_powsignal_switches())
  {
    //BEGIN ping phase
    delay(qiDelays::t_wake);
    if(digitalRead(pins::onOffSwitchPin)==HIGH){
      int maxValue; //Set correct value idk what, same type as what analogRead returns 
      int signalStrengthValue = (analogRead(pins::rectifiedVoltagePin)/maxValue*256);
      char* signalStrengthValueBinary = intToBinary(signalStrengthValue);
      signalStrengthPacket.setMessageIndex(0, ByteGenerator(signalStrengthValueBinary));
      sendSignal(signalStrengthPacket); 
    }
    else{
      sendSignal(endPowerTransfer);
    }


    //BEGIN ID & Config phase 
    if(check_powsignal_switches())
    {
      //Not completed
      IdentificationPacket.set_message_index(0, ByteGenerator('0','0','0','1','0','0','1','0'))//Major/Minor version
      IdentificationPacket.set_message_index(1, ByteGenerator('0','0','0','0','0','0','0','0'))//Påhittad manufacturer code del1
      IdentificationPacket.set_message_index(2, ByteGenerator('0','0','0','0','0','0','0','1'))
      //dessa nedan kan behöva ändras, beror på om vi behöver randomiza grejer
      IdentificationPacket.set_message_index(3, ByteGenerator('1','0','0','0','0','0','0','0'))
      IdentificationPacket.set_message_index(4, ByteGenerator('0','0','0','0','0','0','0','0'))
      IdentificationPacket.set_message_index(5, ByteGenerator('0','0','0','0','0','0','0','0'))
      IdentificationPacket.set_message_index(6, ByteGenerator('0','0','0','0','1','1','1','1'))
      sendSignal(IdentificationPacket)
      delay(qiDelays::t_silent);
      
      //Kanske följande
      //SignalGenerator PowerControlHoldoffPacket(0x06);
      //sendSignal(PowerControlHoldoffPacket); // Om vi vill ha en delay på reaktion från sändaren när vi ber om en ändrad spänning
      //delay(qiDelays::t_silent);
      //Garanterat följande
      char* config_array;//osäker hur man initializar arrays 
      config_array[7] = 0;
      config_array[6] = 0;
      // 5-0 ska vara maximum power value
      ConfigurationPacket.set_message_index(0, ByteGenerator(config_array))
      ConfigurationPacket.set_message_index(1, ByteGenerator('0','0','0','0','0','0','0','0'))//Reserved
      ConfigurationPacket.set_message_index(2, ByteGenerator('0','0','0','0','0','0','0','0'))
      ConfigurationPacket.set_message_index(3, ByteGenerator('0','0','0','0','0','0','0','0'))//Skall ändras
      ConfigurationPacket.set_message_index(4, ByteGenerator('0','0','0','0','0','0','0','0'))
      sendSignal(ConfigurationPacket);
      delay(qiDelays::t_silent);

    }
    //END ID & Config phase




    //BEGIN power transfer phase
    
    while(check_powsignal_switches())
    {

        delay(qiDelays::t_silent);
        bool current_power = sendOneWAtt(); //if true 1 watt, else 0.5 watt
        bool cont = true;


        // This if else claus is for changing the power level (0.5W to 1W)
        if(current_power)
        {
          //if want to recieve one watt
          //Change the value of the message
          sendSignal(controlErrorPacket);  
        }
        else if (!current_power)
        {
          //if want to recieve half watt
          //Change the value of the message
          sendSignal(controlErrorPacket);  

        }
        bool after_power =  sendOneWatt(); 
      
    }
    //END power transfer phase

    

    else //If we cant start the system or something else happens
    {
      
      while(check_powsignal_switches()) //makes sure the transmitter has received the EndPowerTransfer
      {
        sendSignal(EndPowerTransfer); 
      }
    
    }
    //END ping phase  
    

  
}

//END selection phase
}
