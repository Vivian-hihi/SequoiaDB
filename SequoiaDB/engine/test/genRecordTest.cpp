
#include "utilBsongen.hpp"
#include <iostream>
using namespace std;

int main()
{
BSONObj obj;
genRandomRecord(100,10,10,3,obj);

std::cout<<obj.toString()<<std::endl;

return 0;
}
