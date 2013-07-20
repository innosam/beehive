#include <string>
#include "string.h"
#include <stdint.h>

#ifndef BUFFER
#define BUFFER
struct Buffer{
  uint8_t* buf;
  uint8_t* bufstart;
  int size;
  int bufferSize;
  int cur;

  Buffer(char* buffer,int bufSize):size(0),bufferSize(bufSize),cur(0)
  {
    buf = bufstart = (uint8_t*)(buffer);
  }

  Buffer(char* buffer,int bufSize, int _size):size(_size),bufferSize(bufSize),cur(0)
  {
    buf = bufstart =(uint8_t*)(buffer);
  }
 
  void reset(int size = 0);
  int getSize();
  int getCurr();
  uint8_t* getBuffer();
  uint16_t getShort();
  uint32_t getLong();
  uint64_t getLongLong();
  void consume(int n);
  uint8_t getByte();
  char* getRaw();
  void append(uint16_t input);
  void append(uint64_t input);
  void append(uint32_t input);
  void append(uint8_t input);
  void appendLengthValue(std::string& input);
  void getLengthValue(std::string& input);

};
#endif
