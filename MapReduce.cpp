#include "MapReduce.h"
#include <fstream>
#include <sstream>
#include<string>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#define PATH "/tmp/" //PART OF CONFIG

static int  fileAppend()
{
  static int count = 0;
  return ++count;
}


void getNextWord(std::ifstream& inFile, std::string& token, std::string tokenizer, int64_t endOffSet)
{
   token.clear();
   char byte[1];

  while(inFile.good())
  {
     if(inFile.tellg() >= endOffSet)
       return;

     byte[0] =inFile.peek();
     if(byte[0]=='\n' || byte[0] == tokenizer.c_str()[0]){
       inFile.get();
     }else{
       break;
     }
  }


  while(inFile.good()){
   byte[0] = inFile.get();   
   if(byte[0]=='\n' || byte[0] == tokenizer.c_str()[0]){
     return;
   }
    token.append(byte,1);
   }
}

std::string wordcountmap(std::string inputFile, int64_t startOffset, int64_t endOffset)
{
  std::ifstream inFile;
  inFile.open(inputFile.c_str(), std::fstream::in);
  inFile.seekg (startOffset, inFile.beg); //read all till < endOffset
  std::string token;
  std::stringstream ss;
  uint64_t count =0;
 
  while(inFile.tellg() < endOffset){
    getNextWord(inFile,token," ",endOffset); //tokenizer

    if(token.size() == 0) continue;

    count++;
  }

  inFile.close();

  ss.clear();
  ss<<PATH<<getpid()<<fileAppend();
  std::string pidstr(ss.str());

  std::ofstream outFile;
  outFile.open(pidstr.c_str(), std::fstream::out);
  outFile<<"count "<<count<<std::endl;
  outFile.close();
  return ss.str();

}

std::string wordcountreduce(std::string inputFile, std::string inputFile2)
{
  std::ifstream inFile,inFile2;
  inFile.open(inputFile.c_str(), std::fstream::in);
  inFile2.open(inputFile2.c_str(), std::fstream::in);
  
  uint64_t i,count;
  std::string str;

  inFile>> str>> i;
  count =i;
  inFile2>>str >>i;
  count = count +i;
  std::stringstream ss;

  ss.clear();
  ss<<PATH<<getpid()<<"reduce"<<fileAppend();
  std::string pidstr(ss.str());

  std::ofstream outFile;
  outFile.open(pidstr.c_str(), std::fstream::out);
  outFile<<"count "<<count<<std::endl;
  outFile.close();
  return ss.str();

}

std::string wordcountMapmap(std::string inputFile, int64_t startOffset, int64_t endOffset)
{
  std::ifstream inFile;
  inFile.open(inputFile.c_str(), std::fstream::in);
  inFile.seekg (startOffset, inFile.beg); //read all till < endOffset
  std::string token;
  std::stringstream ss;
  std::map<std::string, int> wcMap;
 
  while(inFile.tellg() < endOffset){
    getNextWord(inFile,token," ",endOffset); //tokenizer

    if(!token.compare(" ")) continue;
    if(token.size()==0) continue;
    
    if(wcMap.count(token)){
      wcMap[token]++;
    }else{
      wcMap[token]=1;
    }
  }

  inFile.close();

  ss.clear();
  ss<<PATH<<getpid()<<fileAppend();
  std::string pidstr(ss.str());

  std::ofstream outFile;
  outFile.open(pidstr.c_str(), std::fstream::out);

  std::map<std::string, int>::iterator it;
  for(it = wcMap.begin();it!= wcMap.end(); it++){
  outFile<<it->first<<"  "<<it->second<<std::endl;
  }
  outFile.close();
  return ss.str();

}

std::string wordcountMapreduce(std::string inputFile, std::string inputFile2)
{
  std::ifstream inFile,inFile2;
  inFile.open(inputFile.c_str(), std::fstream::in);
  inFile2.open(inputFile2.c_str(), std::fstream::in);
  
  uint64_t i;
  std::string str;
  std::map<std::string, int> wcMap;
 

  while(inFile>> str>> i){
      wcMap[str] = i; 
  }

  while(inFile2>> str>> i){
   if(wcMap.count(str)){
      wcMap[str] = wcMap[str] + i;  
   }else{
      wcMap[str] = i;  
   }
  }

  std::stringstream ss;

  ss.clear();
  ss<<PATH<<getpid()<<"reduce"<<fileAppend();
  std::string pidstr(ss.str());

  std::ofstream outFile;
  outFile.open(pidstr.c_str(), std::fstream::out);

  std::map<std::string, int>::iterator it;
  for(it = wcMap.begin();it!= wcMap.end(); it++){
  outFile<<it->first<<"  "<<it->second<<std::endl;
  }

  outFile.close();
  return ss.str();

}



std::map<std::string, mapfunc> createMap_map()
{
      std::map<std::string, mapfunc> m;
      m["wordcount"] = wordcountmap;   
      m["wordcountmap"] = wordcountMapmap;    
      return m; 
}

std::map<std::string, reducefunc> createReduce_map()
{
      std::map<std::string, reducefunc> m;
      m["wordcount"] = wordcountreduce;   
      m["wordcountmap"] = wordcountMapreduce;    
      return m; 
}


std::map<std::string,mapfunc> mapReduce:: mapFunctions =  createMap_map();
std::map<std::string,reducefunc> mapReduce:: reduceFunctions =  createReduce_map();

