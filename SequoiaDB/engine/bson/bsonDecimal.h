/**
 * @file bsonDecimal.h
 * @brief CPP BSONObjBuilder and BSONArrayBuilder Declarations
 */

/*    Copyright 2009 10gen Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once
#if defined (SDB_ENGINE) || defined (SDB_CLIENT)
#include "core.hpp"
#include "oss.hpp"
#endif
#include <string>
#include <cstring>
#include "../client/bson/common_decimal.h"

using namespace std;
/** \namespace bson
    \brief Include files for C++ BSON module
*/
namespace bson {

   #define BSONDECIMAL_TOSTRING_FULL   ( 0 )
   #define BSONDECIMAL_TOSTRING_NICE   ( 1 )
   #define BSONDECIMAL_TOSTRING_SIMPLE ( 2 )

   class bsonDecimal
   {
   public:
      bsonDecimal() ;
      bsonDecimal( const bsonDecimal &right ) ;
      ~bsonDecimal() ;

      bsonDecimal& operator= ( const bsonDecimal &right ) ;

   public:
      INT32          init() ;
      INT32          init( INT32 precision, INT32 scale ) ;
      INT32          fromInt( INT32 value ) ;
      INT32          toInt( INT32 *value ) ;

      INT32          fromLong( INT64 value ) ;
      INT32          toLong( INT64 *value ) ;

      INT32          fromDouble( FLOAT64 value ) ;
      INT32          toDouble( FLOAT64 *value ) ;

      INT32          fromString( const CHAR *value ) ;
      string         toString() ;

      string         toJsonString() ;

      INT32          fromBsonValue( const CHAR *bsonValue ) ;

      INT32          compare( const bsonDecimal &right ) ;

   public:
      INT32          add( const bsonDecimal &right, bsonDecimal &result ) ;
      INT32          sub( const bsonDecimal &right, bsonDecimal &result ) ;
      INT32          mul( const bsonDecimal &right, bsonDecimal &result ) ;
      INT32          div( const bsonDecimal &right, bsonDecimal &result ) ;
      INT32          abs() ;
      INT32          ceil( bsonDecimal &result ) ;
      INT32          floor( bsonDecimal &result ) ;
      INT32          mod( bsonDecimal &right, bsonDecimal &result ) ;
      

   public:
      /* getter */
      INT16          getWeight() const ;
      INT32          getTypemod() const ;
      INT32          getPrecision( INT32 *precision, INT32 *scale ) const ;

      // decimal->dscale | decimal->sign ;
      INT16          getStorageScale() const ;
      INT16          getScale() const ;
      INT16          getSign() const ;

      INT32          getNdigit() const ;
      const INT16*   getDigits() const ;
      INT32          getSize() const ;

   private:
      bson_decimal   _decimal ;
   } ;

}


