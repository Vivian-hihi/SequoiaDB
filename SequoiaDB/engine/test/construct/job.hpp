#ifndef JOB_HPP_
#define JOB_HPP_

#include <string>

using namespace std;

enum JOB_TYPE
{
   JOB_TYPE_QUIT = 0,
   JOB_TYPE_INSERT,
   JOB_TYPE_DELETE,
   JOB_TYPE_UPDATE,
   JOB_TYPE_QUERY
};

class job
{
   public:
      job(){}
      ~job(){}
   public:
      JOB_TYPE _type;
      string _cs;
      string _collection;
};

#endif

