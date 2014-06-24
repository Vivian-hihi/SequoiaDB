#include "myclass.hpp"
#include <iostream>

using namespace std ;

JS_MEMBER_FUNC_DEFINE( myclass, func)
JS_CONSTRUCT_FUNC_DEFINE( myclass, construct)
JS_DESTRUCT_FUNC_DEFINE(myclass, destruct)

JS_BEGIN_MAPPING(myclass, "myjsclass")
  JS_ADD_MEMBER_FUNC("func", func)
  JS_ADD_CONSTRUCT_FUNC(construct)
  JS_ADD_DESTRUCT_FUNC(destruct)
JS_MAPPING_END()

myclass::myclass()
{
   cout << "myclass::myclass" << endl ;
}

myclass::~myclass()
{
   cout << "myclass""~myclass" << endl ;
}

INT32 myclass::func( const _sptParamContainer &arg,
                    _sptReturnVal &rval,
                    bson::BSONObj &detail )
{
   cout << "myclass::fun" << endl ;
   return SDB_OK ;
}

INT32 myclass::construct( const _sptParamContainer &arg,
                          _sptReturnVal &rval,
                           bson::BSONObj &detail)
{
   cout << "myclass::construct" << endl;
   return SDB_OK ;
}

INT32 myclass::destruct()
{
   cout << "myclass::destruct" << endl ;
   return SDB_OK ;
}
