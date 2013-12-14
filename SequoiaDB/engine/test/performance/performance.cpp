#include <stdio.h>
#include "../client/client.hpp"
#include <iostream>

using namespace std ;

string toJson( const bson &b )
{
   string json ;
   INT32 len = bson_sprint_length( &b ) + 1;
   CHAR *buf = (CHAR *)malloc(len) ;
   if ( NULL != buf )
   {
      if ( bsonToJson( buf, len, &b, FALSE ) )
      {
         json = string( buf ) ;
      }
      free( buf ) ;
   }
   return json ;
}

INT32 main()
{
   bson obj ;
   bson_init( &obj ) ;
   UINT64 _id = 0 ;
   CHAR text[1000] = {'1'} ;
   memset( text, '1', 1000 ) ;
   text[999] = '\0' ;
   bson_append_long( &obj, "_id", _id ) ;
   bson_append_long( &obj, "index1", _id ) ;
   bson_append_long( &obj, "index2", _id ) ;
   bson_append_string( &obj, "text", text ) ;
   bson_finish( &obj ) ;
   cout << bson_size( &obj ) << endl ;
   cout << toJson( obj ) << endl ;
   _id++ ;
   *((UINT64 *)(obj.data+8)) = _id ;
   cout << toJson( obj ) << endl ;
   bson_destroy( &obj ) ;

   return 0 ;
}
