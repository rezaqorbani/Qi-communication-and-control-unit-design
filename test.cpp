

#include "SignalGenerator.h"
#include "ByteGenerator.h"
#include <iostream>
//conver int to binary
char* intToBinary(int n)
{
  char ret [8];
  int index = 0; 
  while(n!=0) {ret[index] = (n%2==0 ?'0':'1'); index++; n/=2;}
  return ret;
}


int main()
{
    SignalGenerator sig(0x01); 
    sig.setMessageIndex(0, ByteGenerator('0','0','0','0','0','0','0','0')); 
    sig.setMessageIndex(0, ByteGenerator('1','1','1','1','1','1','1','1')); 
    for(int i = 0; i < sig.getSignalSize(); i++)
    {
        std::cout << sig.getSignal()[i] << "|"; 
    }
}
