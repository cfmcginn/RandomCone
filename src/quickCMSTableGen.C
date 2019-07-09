#include <iostream>

int quickCMSTableGen()
{
  for(int i = 200; i >= 0; --i){
    if(i%2 == 0) std::cout << i << "," << std::endl;    
  }

  return 0;
}

int main()
{
  int retVal = 0;
  retVal += quickCMSTableGen(); 
  return 0;
}
