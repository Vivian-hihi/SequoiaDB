#ifndef MYCLASS_HPP_
#define MYCLASS_HPP_

#include "sptApi.hpp"

using namespace engine;

class myclass : public SDBObject
{
JS_DECLARE_CLASS(myclass)

public:
   myclass() ;
   virtual ~myclass() ;

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

