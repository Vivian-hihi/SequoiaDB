#ifndef MYCLASS2_HPP_
#define MYCLASS2_HPP_

#include "sptApi.hpp"

using namespace engine;

class myclass2 : public SDBObject
{
JS_DECLARE_CLASS(myclass2)

public:
   myclass2() ;
   virtual ~myclass2() ;

public:
   INT32 construct( const _sptParamContainer &arg,
                    _sptReturnVal &rval,
                    bson::BSONObj &detail) ;

   INT32 func( const _sptParamContainer &arg,
               _sptReturnVal &rval,
               bson::BSONObj &detail ) ;

   INT32 destruct() ; 
} ;

#endif

