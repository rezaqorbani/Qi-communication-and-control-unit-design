#include "SignalGenerator.h"


SignalGenerator::SignalGenerator(int header)
{
    m_header_hex = header;
    this->m_create_header(header);   

}


void SignalGenerator::setMessageIndex(int index, ByteGenerator message)
{
    for(int i = 0; i < 11; i++)
    {
    this->m_messages[index][i] = message.get_byte()[i];
    }
    this->m_generate_checksum();
}


char* SignalGenerator::getMessageByte(int index)
{
    return m_messages[index];
}


char* SignalGenerator::getSignal()
{

    char* m_signal = new char [this->getSignalSize()];


    int byte_index = 0;

    for(int i = 0; i < 15; i++,byte_index++ )//preamble
    {

                m_signal[byte_index] = this->m_preamble[i];

    }


    for(int j =  0 ; j < 11; j++, byte_index++)//header
    {
        m_signal[byte_index] = m_header_byte.get_byte()[j];
    }



    for(int i = 0; i < this->m_calculate_message_size() ; i++ )//message
    {
        for(int j = 0; j < 11; j++, byte_index++)
        {
                m_signal[byte_index] = this->m_messages[i][j];

        }
    }


    for(int j =  0 ; j < 11; j++, byte_index++)
    {
         m_signal[byte_index] = m_checksum.get_byte()[j];
    }

    return m_signal;
}




int SignalGenerator::getSignalSize()
{
    int signal_size = 0;

    signal_size += 15; //preamble size

    signal_size += 11; // add 11 for the size of header


    signal_size += (this->m_calculate_message_size()*11); // add eleven bits for every message byte

    signal_size += 11; // add 11 bits for checksum

    return signal_size;


}

void SignalGenerator::m_create_header(int header) //creates a char array with '1' and '0' from an in int
{
    int byte_size = 8;
    unsigned int mask = 1U << (byte_size-1);
    int index;
    for (index = 0; index < byte_size; index++) {
         char bit = (header & mask) ? '1' : '0';
         this->m_header_byte.set_one_bit(index, bit);
        header <<= 1;
    }



}


int SignalGenerator::m_calculate_message_size()
{

  if(0 < m_header_hex && m_header_hex <= 0x1F )
  {

    return ((int) (1+((m_header_hex-0)/32)));
  }

  if(0x20 <= m_header_hex && m_header_hex <= 0x7F )
  {

    return ((int) (2+((m_header_hex-32)/16)));
  }

  if(0x80 <= m_header_hex && m_header_hex <= 0xDF )
  {
    return ((int) (8+((m_header_hex-128)/8)));
  }

  if(0xE0 <= m_header_hex && m_header_hex <= 0xFF )
  {
    return ((int) (20+((m_header_hex-224)/4)));
  }
 return -1;
}



void SignalGenerator::m_generate_checksum()
{

    for(int bit = 1; bit < 9; bit++)
    {
        this->m_checksum.get_byte()[bit] = ((this->m_header_byte.get_byte()[bit] - '0') ^ (this->m_messages[0][bit] - '0')) ?'1':'0'; //XOR each pair of bit of adjacent fields
    }

    for (int index = 1; index < this->m_calculate_message_size(); index++)
    {

        for(int bit = 1; bit < 9 ; bit++)
        {
            this->m_checksum.get_byte()[bit] = ((this->m_messages[index][bit] - '0') ^ (this->m_checksum.get_byte()[bit] - '0'))?'1':'0';
        }
    }

}


SignalGenerator::~SignalGenerator()
{
    
}
