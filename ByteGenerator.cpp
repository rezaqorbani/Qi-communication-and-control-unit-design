#include "ByteGenerator.h"

ByteGenerator::ByteGenerator(char bit_7, char bit_6, char bit_5, char bit_4, char bit_3, char bit_2, char bit_1, char bit_0)
{
  m_bit_array[0] = '0';
  m_bit_array[1] = bit_7;
  m_bit_array[2] = bit_6;
  m_bit_array[3] = bit_5;
  m_bit_array[4] = bit_4;
  m_bit_array[5] = bit_3;
  m_bit_array[6] = bit_2;
  m_bit_array[7] = bit_1;
  m_bit_array[8] = bit_0;
  m_bit_array[9] = this->get_parity();
  m_bit_array[10] = '1';
}


ByteGenerator::ByteGenerator(char* init_array )
{
  m_bit_array[0] = '0';
  m_bit_array[1] = init_array[0];
  m_bit_array[2] = init_array[1];
  m_bit_array[3] = init_array[2];
  m_bit_array[4] = init_array[3];
  m_bit_array[5] = init_array[4];
  m_bit_array[6] = init_array[5];
  m_bit_array[7] = init_array[6];
  m_bit_array[8] = init_array[7];
  m_bit_array[9] = this->get_parity();
  m_bit_array[10] = '1';
}



char* ByteGenerator::get_byte()
{
  return m_bit_array;
}


void ByteGenerator::set_all_bits(char bit_7, char bit_6, char bit_5, char bit_4, char bit_3, char bit_2, char bit_1, char bit_0)
{
  m_bit_array[1] = bit_7;
  m_bit_array[2] = bit_6;
  m_bit_array[3] = bit_5;
  m_bit_array[4] = bit_4;
  m_bit_array[5] = bit_3;
  m_bit_array[6] = bit_2;
  m_bit_array[7] = bit_1;
  m_bit_array[8] = bit_0;
}


void ByteGenerator::set_one_bit( int index, char value)
{
  if( index >= 0 && index <= 7 && (value == '1' || value == '0'))
  {
      m_bit_array[8-index] = value;
  }
}


void ByteGenerator::set_all_bits_array(char * init_array)
{
  m_bit_array[0] = '0';
  m_bit_array[1] = init_array[0];
  m_bit_array[2] = init_array[1];
  m_bit_array[3] = init_array[2];
  m_bit_array[4] = init_array[3];
  m_bit_array[5] = init_array[4];
  m_bit_array[6] = init_array[5];
  m_bit_array[7] = init_array[6];
  m_bit_array[8] = init_array[7];
  m_bit_array[9] = this->get_parity();
  m_bit_array[10] = '1';
}


char ByteGenerator::get_parity()
{
    int number_of_ones = 0;
    for(int i = 1; i < 9; i++)
    {
      if(m_bit_array[i] == '1')
        {
                number_of_ones += 1;
        }
    }
    return ((number_of_ones % 2 == 0)? '1' : '0' ); // number_of_bits bigger than must be bigger than zero?
}

