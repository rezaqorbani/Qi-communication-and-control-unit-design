/*
Made by Reza Qorbani
*/
#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H


#include "ByteGenerator.h"



class SignalGenerator
{
  public:
    SignalGenerator(int header); //Enter hex value of the header to create a signal, the size of the message is then automatically calculated
    void setMessageIndex(int index, ByteGenerator message); //set the message BYTE at the specified index (of all messages), set the message with a ByteGenerator object as argument (eg. ByteGenerator('1','0','1','1','1','0','1','0') note 8 bits) (write)
    char* getMessageByte(int index); //get the message byte at the specified index (of all messages) (read)
    char* getSignal(); // return the entire signal, every bit of it, in one array. You can iterate through this array by using a for loop for example and getSignalSize() for the size of the array.
    int getSignalSize(); //return the size of the array that represents the signal.
    ~SignalGenerator();

  private:
    char m_preamble [15] = {'1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'};
    ByteGenerator m_header_byte;
    int m_header_hex;               //the the numerical value of header (not the byte representing the header)
    int m_messages_size;
    char m_messages [7][11];
    ByteGenerator m_checksum = ByteGenerator('1','1','1','1','1','1','1','1');
    int m_calculate_message_size(); //calculates the size of the message
    void m_generate_checksum();  //Creates the checksum for the signal
    void m_create_header(int); // Get a hex value and turn it into a array of chars for representing the byte.

};

#endif
