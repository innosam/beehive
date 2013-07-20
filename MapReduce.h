#include <map>
#include <string>
#include <stdint.h>
#ifndef __MAP_REDUCE
#define __MAP_REDUCE

typedef std::string (*mapfunc)(std::string inputFile, int64_t startOffset, int64_t endOffset);
typedef std::string (*reducefunc)(std::string inputFile, std::string inputFile2);

struct mapReduce{
 static std::map<std::string, mapfunc> mapFunctions;
 static std::map<std::string, reducefunc> reduceFunctions;

};

#endif
