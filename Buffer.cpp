#include "Buffer.h"

void Buffer::reset(int size )
{
  buf = bufstart;
  cur=0;
  this->size = size;
}

int Buffer::getSize()
{
  return cur;
}

int Buffer::getCurr()
{
  return cur;
}

uint8_t* Buffer::getBuffer()
{
  return bufstart;
}

uint16_t Buffer::getShort()
{
  consume(2);
  return *(uint16_t*)(buf-2);
}

uint32_t Buffer::getLong()
{
  consume(4);
  return *(uint32_t*)(buf-4);
}

uint64_t Buffer::getLongLong()
{
  consume(8);
  return *(uint64_t*)(buf-8);
}


void Buffer::consume(int n){
  buf = buf+n;
  cur = cur+n;
}

uint8_t Buffer::getByte(){
  consume(1);
  return *(buf-1);
}

char* Buffer::getRaw(){
  return (char*)buf;
}

void Buffer::append(uint16_t input){
  *(uint16_t*)buf = input;
  consume(2);
}

void Buffer::append(uint64_t input){
  *(uint64_t*)buf = input;
  consume(8);
}



void Buffer::append(uint32_t input){
  *(uint32_t*)buf = input;
  consume(4);
}


void Buffer::append(uint8_t input){
  *buf = input;
  consume(1);
}
void Buffer::appendLengthValue(std::string& input){
  *buf = (uint8_t)input.size();
  consume(1);
  memcpy(buf,input.c_str(),input.size());
  consume(input.size());
}

void Buffer::getLengthValue(std::string& input){
  uint8_t len = getByte();
  input.assign(getRaw(),len);
  consume(len);
}

