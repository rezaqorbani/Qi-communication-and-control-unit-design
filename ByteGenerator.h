#ifndef BYTEGENERATOR_H
#define BYTEGENERATOR_H


class ByteGenerator
{
  public:
    //ByteGenerator(){};
    //Generate a byte with the given bits, MSB to LSB
    ByteGenerator(char bit_7=0 , char bit_6 =0, char bit_5=0 , char bit_4=0 , char bit_3=0 , char bit_2 =0, char bit_1=0 , char bit_0=0 );

    //Initialize a ByteGenerator with an array
    ByteGenerator(char* init_array );

    //Get a character array of the bits of byte
    char* get_byte();

    //change all the bits (b7-b0)  MSB to LSB
    void set_all_bits(char bit_7 = 0, char bit_6 = 0, char bit_5 = 0, char bit_4 = 0, char bit_3 = 0, char bit_2 = 0, char bit_1 = 0, char bit_0 = 0);

    //set all bits with a 8 element array
    void set_all_bits_array(char* bitmap);

    //change on bit (b7-b0) at a certain index
    void set_one_bit(int index, char value);

  private:
    char m_bit_array[11];
    char get_parity();
};

#endif
