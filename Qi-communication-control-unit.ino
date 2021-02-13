/*
Created by: 
*/
#include "ByteGenerator.h"
#include "SignalGenerator.h"

//The delays (in millisecond) for the signal in Qi-spec. 
namespace qiDelays
{
  int t_wake = 60; 
  int t_silent = 7; 
};

namespace pins
{
  const int ModComPin = 1;
  const int powerSignalPin  =  A0;//Ändra siffran beroende på vilken pin som faktiskt används (detta är en avläsningspin, analog)
  const int onOffSwitchPin  =  2;//Ändra siffran beroende på vilken pin som faktiskt används (detta är en avläsningspin, digital)
};


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
  if (digitalRead(pins::ModComPin)==HIGH){
    current_state = true;
  }
  else{
    current_state = false;
  }
  
  for (int i = 0; i < signal.get_signal_size(); i++)
  {
    if( signal.get_signal()[i] == '1')
    {
      if(current_state){
        //observera att syntax för arduinoswitches är fel
        digitalWrite(ModComPin, LOW);
        delay(0.25);
        digitalWrite(ModComPin, HIGH);
        delay(0.25);
        //Switch twice, from HIGH->LOW->HIGH
        current_state=true;
      }
      else{
        digitalWrite(ModComPin, HIGH);
        delay(0.25);
        digitalWrite(ModComPin, LOW);
        delay(0.25);
        //Switch twice, from LOW->HIGH->LOW
        current_state=false;
        }
      }
    else if(signal.get_signal()[i] == '0')
    {
      if(current_state){
        //observera att syntax för arduinoswitches är fel
        digitalWrite(ModComPin, LOW);
        delay(0.5);
        //Switch once, from HIGH->LOW
        current_state=false;
      }
      else{
        digitalWrite(ModComPin, HIGH);
        delay(0.5);
        //Switch once, from LOW->HIGH
        current_state=true;
        }
        
    }
  }

}


void check_powsignal_switches(IDK currentPowerLevelState) // vet inte riktigt hur den ska implementeras eller vart den ska komma ifrån. kanske en global variabel?
{ 
  // checks if the Primary Cell current amplitude crosses 50% of the stable level
  //A method that should be called repeteadly, to ensure that we do not miss a signalchange. FIX IDK idk vad det ska ha för type
  //the value below can be changed, idk what is right
  //If the ifstatment below is fullfilled, it means we have a power signal, and the system should boot up. 
  int X;//defines the lower limit of our recitified current of the power signal and which level is considered on/off
  //dessa kan ev. ersättas av ett fett if(X or Y or Z), vet inte vilket som blir bäst 
  if(analogRead(powerSignalPin)<X){
    //No more power signal = return to selection phase/turn off
    return false;
    }
  else if(digitalRead(onOffSwitchPin)==LOW){//Assuming LOW=on off position
    //System has been turned off, so disconnect the output to the load
    return false;
    }
  else if(digitalRead(powerLevelSwitch)!=currentPowerLevelState){//måste veta current state, vi får hitta på en variabel som jag nu döpt till powerLevelSwitchState, och definiera den. om det har förändrats ska:
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
  char* ret [8];
  int index = 0; 
  while(n!=0) {ret[index]=(n%2==0 ?"0":"1"); index++; n/=2;}
  return r;
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
  int signalStrengthValue;
  int maxValue; //Set correct value idk what, same type as what analogRead returns 
  signalStrengthValue = (analogRead(rectifiedVoltagePin)/maxValue*256)

  char signalStrengthValueBinary[8] = T)B/vet inte hur det binära talet ska hamna när men ja
  SignalGenerator signalStrengthPacket(0x01);
  signalStrengthPacket.set_message_index(0, ByteGenerator(signalStrengthValueBinary[0],signalStrengthValueBinary[1],signalStrengthValueBinary[2],signalStrengthValueBinary[3],signalStrengthValueBinary[4],signalStrengthValueBinary[5],signalStrengthValueBinary[6],signalStrengthValueBinary[7]));
  send_signal(signalStrengthPacket);
}

bool sendOneWatt()
{
  //checks the button on arduino that chooses between 1 and 0.5 watt power transfer. 
  //if true send 1 watt
  //else send 0.5 watt
}
// starts the components in the 
void startSystem()
{
}


void setup()
{
  // put your setup code here, to run once:
  // Här definierar vi alla pins som ska användas
  const int ModComPin = pinMode(1, OUTPUT);//Number here to correspond to the ModComPin
  
  //This is how you implement a signal
  //-----------------------------------------------------------
  SignalGenerator sig(0x1f);    //Give the signal header in hex
  sig.set_preamble();           //set the preamble (maybe uneccessary)
  sig.set_message_index(1, ByteGenerator('1','1','1',1,1,0,0)); // set messeage(s) according to the Qi-spec. 
  //-----------------------------------------------------------

  //To get the array of the bits of all bytes of a given signal you just defined, use: 
  char* sig_array = sig.get_signal();
}


void loop() 
{   
// put your main code here, to run repeatedly:
//BEGIN selection phase

  if (check_powsignal_switches())
  {
    bool system_started = start_system(); 
    delay(qiDelay::t_wake); 



    //BEGIN ping phase
    if(system_started)//lägg till condition ifall vi INTE vill börja överföring, isf ska vi skicka EndPowerTransfer
    {
      int signalStrengthValue;
      int maxValue; //Set correct value idk what, same type as what analogRead returns 
      signalStrengthValue = (analogRead(rectifiedVoltagePin)/maxValue*256)
      char signalStrengthValueBinary[8] = T)B/vet inte hur det binära talet ska hamna när men ja
      SignalGenerator signalStrengthPacket(0x01);
      signalStrengthPacket.set_message_index(0, ByteGenerator(signalStrengthValueBinary[0],signalStrengthValueBinary[1],signalStrengthValueBinary[2],signalStrengthValueBinary[3],signalStrengthValueBinary[4],signalStrengthValueBinary[5],signalStrengthValueBinary[6],signalStrengthValueBinary[7]));
      send_signal(signalStrengthPacket); 


      //BEGIN ID & Config phase 
      if(check_powsignal_switches())
      {
        SignalGenerator IdentificationPacket(0x71);//Not completed
        IdentificationPacket.set_message_index(0,ByteGenerator('0','0','0','0','0','0','0','1'))
        IdentificationPacket.set_message_index(1)
        IdentificationPacket.set_message_index(2)
        IdentificationPacket.set_message_index(3)
        IdentificationPacket.set_message_index(4)
        IdentificationPacket.set_message_index(5)
        IdentificationPacket.set_message_index(6)
        

        send_signal(IdentificationPacket)
        delay(qiDelays::t_silent);
        //Kanske följande
        SignalGenerator PowerControlHoldoffPacket(0x06);
        send_signal(PowerControlHoldoffPacket); // Om vi vill ha en delay på reaktion från sändaren när vi ber om en ändrad spänning
        delay(qiDelays::t_silent);
        //Garanterat följande
        SignalGenerator ConfigurationPacket(0x51);
        send_signal(ConfigurationPacket);
        delay(qiDelays::t_silent);

      }
      //END ID & Config phase




      //BEGIN power transfer phase
      
      while(check_powsignal_switches())
      {

          delay(qiDelays::t_silent);
          bool current_power = send_one_watt(); //if true 1 watt, else 0.5 watt
          bool cont = true;


          // This if else claus is for changing the power level (0.5W to 1W)
          if(current_power)
          {
            //if want to recieve one watt
            //Change the value of the message
            send_signal(control_error_packet);  
          }
          else if (!current_power)
          {
            //if want to recieve half watt
            //Change the value of the message
            send_signal(control_error_packet);  

          }
          bool after_power =  send_one_watt(); 
        
      }
      //END power transfer phase

    }

    else //If we cant start the system or something else happens
    {
      
      while(check_powsignal_switches()) //makes sure the transmitter has received the EndPowerTransfer
      {
        send_signal(EndPowerTransfer); 
      }
    
    }
    //END ping phase  
    

  
}

//END selection phase
}
