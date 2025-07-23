/*******************************************************************************

   Copyright (C) 2011-Present SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = charsetICUConvertor.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains functions for agent processing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/11/2023  ZYS  Initial Draft

   Last Changed =

*******************************************************************************/
#include "charsetICUConvertor.hpp"
#include "charsetConvertorInterface.hpp"
#include "ossErr.h"
#include "ossTypes.h"
#include "pd.hpp"
#include "unicode/putil.h"
#include "unicode/ucnv.h"
#include "unicode/utypes.h"
#include <cstring>
#include <exception>
#include <memory>
#include "charsetUtils.hpp"
#include "../bson/bson.h"
#include "utilMemListPool.hpp"

namespace engine
{
   charsetICUConvertor::charsetICUConvertor( Charset inCharset,
                                             Charset outCharset,
                                             UConverter* inConv,
                                             UConverter* outConv )
      : charsetConvertorInterface( inCharset, outCharset )
   {
      SDB_ASSERT( inConv, "Invalid source UConvertor" ) ;
      SDB_ASSERT( outConv, "Invalid target UConvertor" ) ;
      _inConverter = inConv ;
      _outConverter = outConv ;
      _estimateMaxSize = ucnv_getMaxCharSize( _outConverter ) ;
      SDB_ASSERT( _estimateMaxSize > 0,
                  "Failed to get target Convertor max char size" ) ;
   }

   charsetConvertorInterface*
      charsetICUConvertor::make( Charset inCharset, Charset outCharset )
   {
      UErrorCode err = U_ZERO_ERROR ;
      StringData inCharsetName = charsetSerializer( inCharset ) ;
      StringData outCharsetName = charsetSerializer( outCharset ) ;
      UConverter *inConv = NULL, *outConv = NULL ;

      inConv = ucnv_open( inCharsetName.data(), &err ) ;
      // Report warning if inConv is not available
      if ( U_FAILURE(err) ) 
      {
         PD_LOG( PDERROR, "Failed to init UConvertor for %s, error: %s",
                 inCharsetName.data(), u_errorName(err) ) ;
         return NULL ;
      }

      outConv = ucnv_open( outCharsetName.data(), &err ) ;
      // Report warning if outConv is not available
      if ( U_FAILURE(err) )
      {
         PD_LOG( PDERROR, "Failed to init UConvertor for %s, error: %s",
                 outCharsetName.data(), u_errorName(err) ) ;
         ucnv_close( inConv ) ;
         inConv = NULL ;
         return NULL ;
      }

      return (new charsetICUConvertor( inCharset, outCharset,
                                       inConv, outConv) ) ;
   }

   charsetConvertorInterface* charsetICUConvertor::make_clone()
   {
      Charset inCharset = getInCharset() ;
      Charset outCharset = getOutCharset() ;
      UErrorCode err = U_ZERO_ERROR ;
      UConverter *inConv = NULL, *outConv = NULL;

      inConv = ucnv_safeClone( _inConverter, NULL, NULL, &err ) ;
      if ( NULL == inConv )
      {
         StringData in = charsetSerializer( getInCharset() ) ;
         PD_LOG( PDERROR, "Failed to clone in convertor for '%s'"
                 ", error: %s", in.data(), u_errorName(err) ) ;
         return NULL ;
      }

      outConv = ucnv_safeClone( _outConverter, NULL, NULL, &err ) ;
      if ( NULL == outConv )
      {
         StringData out = charsetSerializer( getOutCharset() ) ;
         PD_LOG( PDERROR, "Failed to clone out convertor for '%s'"
                 ", error: %s", out.data(), u_errorName( err) ) ;
         ucnv_close( inConv ) ;
         return NULL ;
      }
      return (new charsetICUConvertor( inCharset, outCharset,
                                       inConv, outConv) ) ;
   }

   charsetICUConvertor::~charsetICUConvertor()
   {
      if ( _inConverter )
      {
         ucnv_close(_inConverter ) ;
         _inConverter = NULL ;
      }

      if ( _outConverter )
      {
         ucnv_close(_outConverter ) ;
         _outConverter = NULL ;
      }
   }

   INT32 charsetICUConvertor::convertToBuffer( const char* inBuffer,
                                               int inSize,
                                               char* outBuffer,
                                               int outSize,
                                               int& convertedInSize,
                                               int& convertedOutSize) const
   {
      INT32  rc = SDB_OK ;
      if ( (getInCharset() == getOutCharset()) ||
           (!_inConverter) || (!_outConverter) )
      {
         int copySize = std::min( inSize, outSize ) ;
         memcpy( outBuffer, inBuffer, copySize ) ;
         convertedInSize = copySize ;
         convertedOutSize = copySize ;
         return SDB_OK ;
      }
      else if ( (0 == inSize) || (0 == outSize) )
      {
         convertedInSize = 0 ;
         convertedOutSize = 0 ;
         return SDB_OK;
      }

      UErrorCode err = U_ZERO_ERROR ;
      const char* originInBuffer = inBuffer ;
      char* originOutBuffer = outBuffer ;

      ucnv_convertEx( _outConverter,
                      _inConverter,
                      &outBuffer,
                      outBuffer + outSize,
                      &inBuffer,
                      inBuffer + inSize,
                      NULL,
                      NULL,
                      NULL,
                      NULL,  // Offsets not used
                      true,
                      true,  // Reset and flush
                      &err ) ;

      if ( U_FAILURE(err) )
      {
         StringData in = charsetSerializer( getInCharset() ) ;
         StringData out = charsetSerializer( getOutCharset() ) ;
         PD_LOG( PDERROR, "Failed to convert string from '%s' "
                 "to '%s', error: %s",
                 in.data(), out.data(), u_errorName(err) ) ;
         rc = SDB_SYS ;
      }
      else 
      {
         convertedInSize = inBuffer - originInBuffer ;
         convertedOutSize = outBuffer - originOutBuffer ;
      }
      return rc ;
   }

   INT32 charsetICUConvertor::convert( const std::string& inString,
                                       std::string& outString ) const
   {
      return convert( StringData(inString), outString ) ;
   }

   INT32 charsetICUConvertor::convert( const StringData &inString,
                                       std::string& outString ) const
   {
      int inStrLen = inString.size() ;
      if ( getInCharset() == getOutCharset() )
      {
         outString.assign(inString.data(), inStrLen ) ;
         return SDB_OK ;
      }
      else if ( !_inConverter || !_outConverter )
      {
         outString.assign( inString.data(), inStrLen ) ;
         return SDB_OK ;
      }
      else if ( 0 == inStrLen )
      {
         outString.clear() ;
         return SDB_OK ;
      }
      int convertedInSize = 0, convertedOutSize = 0;
      char *buff =  (CHAR*) SDB_THREAD_ALLOC ( inStrLen * _estimateMaxSize ) ;
      if ( NULL == buff )
      {
         return SDB_OOM ;
      }

      int rc = convertToBuffer( inString.data(),
                                inStrLen,
                                buff,
                                inStrLen * _estimateMaxSize,
                                convertedInSize,
                                convertedOutSize ) ;
      if ( SDB_OK == rc )
      {
         outString.assign( buff, convertedOutSize ) ;
      }

      if ( buff )
      {
         SDB_THREAD_FREE( buff ) ;
      }
      return rc ;
   }

   INT32 charsetICUConvertor::convert( const BSONObj& inObject,
                                       BSONObj& outObject ) const
   {
      INT32 rc = SDB_OK ;
      if ( getInCharset() == getOutCharset() ) 
      {
         outObject = inObject ;
         return SDB_OK ;
      }
      else if ( !_inConverter || !_outConverter )
      {
         outObject = inObject ;
         return SDB_OK ;
      }
      BSONObjBuilder builder ;
      try
      {
         rc = _convert( inObject, builder ) ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "Failed to convert charset of BSONObj, "
                 "unexpect error:%s", e.what() ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      if ( SDB_OK == rc )
      {
         outObject = builder.obj() ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 charsetICUConvertor::_convert( const BSONObj& inObject,
                                        BSONObjBuilder& outBuilder ) const
   {
      BSONObjIterator it(inObject ) ;
      INT32 rc = SDB_OK ;
      while ( it.more() )
      {
         BSONElement elem = it.next() ;
         StringData fieldName( elem.fieldName() ) ;
         std::string newFieldName ;
         rc = convert( fieldName, newFieldName ) ;
         PD_RC_CHECK( rc, PDERROR, "Failed to convert field name '%s', "
                      "rc: %d", fieldName.data(), rc ) ;

         switch ( elem.type() )
         {
            case bson::MinKey:
            case bson::Undefined:
            case bson::MaxKey:
            case bson::jstNULL:
            case bson::NumberDouble:
            case bson::BinData:
            case bson::jstOID:
            case bson::Bool:
            case bson::Date:
            case bson::NumberInt:
            case bson::Timestamp:
            case bson::NumberLong:
            case bson::NumberDecimal:
            {
               outBuilder.appendAs( elem, newFieldName );
               break;
            }
            case  bson::Code:
            {
               std::string outString ;
               rc = convert( StringData(elem.valuestr()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert 'Code' charset, "
                            "rc: %d", rc ) ;
               outBuilder.appendCode(newFieldName, outString) ;
               break;
            }
            case  bson::Symbol:
            {
               std::string outString;
               rc = convert( StringData(elem.valuestr()), outString ) ;
               outBuilder.appendSymbol( newFieldName, outString) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert 'Symbol' charset, "
                            "rc: %d", rc ) ;
               break;
            }
            case  bson::String:
            {
               std::string outString ;
               rc = convert( StringData(elem.valuestr()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert string charset, "
                            "rc: %d", rc ) ;
               outBuilder.append( newFieldName, outString ) ;
               break ;
            }
            case bson::Object:
            {
               BSONObjBuilder subBuilder(
                  outBuilder.subobjStart(newFieldName) ) ;
               rc = _convert( elem.embeddedObject(), subBuilder ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert BSON object charset, "
                            "rc: %d", rc ) ;
               subBuilder.doneFast() ;
               break ;
            }
            case bson::Array:
            {
               BSONArrayBuilder subBuilder(
                  outBuilder.subarrayStart(newFieldName) ) ;
               rc = _convert( elem.embeddedObject(), subBuilder ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert BSON Array charset, "
                            "rc: %d", rc ) ;
               subBuilder.doneFast() ;
               break ;
            }
            case RegEx:
            {
               std::string outString, outOptions ;
               rc = convert( StringData(elem.regex()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert regex string charset, "
                            "rc: %d", rc ) ;

               rc = convert( StringData(elem.regexFlags()), outOptions ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert regex options charset, "
                            "rc: %d", rc ) ;

               outBuilder.appendRegex( newFieldName,
                                       outString, outOptions ) ;
               break ;
            }
            case DBRef:
            {
               std::string outNS ;
               rc = convert( StringData(elem.dbrefNS()), outNS );
               PD_RC_CHECK( rc, PDERROR, "Failed to DBRef charset, rc: %d", rc ) ;
               outBuilder.appendDBRef( newFieldName, outNS, elem.dbrefOID() ) ;
               break ;
            }
            case CodeWScope:
            {
               std::string outCode ;
               BSONObj outCodeObj ;
               rc = convert( StringData(elem.codeWScopeCode()), outCode ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert CodeWScope charset, "
                            "rc: %d", rc ) ;

               rc = convert( elem.codeWScopeObject(), outCodeObj ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert CodeWScope object charset, "
                            "rc: %d", rc ) ;
               outBuilder.appendCodeWScope( newFieldName,
                                            outCode, outCodeObj ) ;
               break ;
            }
            default:
            {
               SDB_ASSERT( 0, "Unknown bson type" ) ;
               break ;
            }
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 charsetICUConvertor::_convert( const BSONObj& inObject,
                                        BSONArrayBuilder& outBuilder ) const
   {
      BSONObjIterator it( inObject ) ;
      INT32 rc = SDB_OK ;
      while ( it.more() )
      {
         BSONElement elem = it.next() ;
         switch ( elem.type() )
         {
            case bson::MinKey:
            case bson::Undefined:
            case bson::MaxKey:
            case bson::jstNULL:
            case bson::NumberDouble:
            case bson::BinData:
            case bson::jstOID:
            case bson::Bool:
            case bson::Date:
            case bson::NumberInt:
            case bson::Timestamp:
            case bson::NumberLong:
            case bson::NumberDecimal:
            {
               outBuilder.append( elem ) ;
               break;
            }
            case  bson::Code:
            {
               std::string outString ;
               rc = convert( StringData(elem.valuestr()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert 'Code' charset "
                            "in BSON Array, rc: %d", rc ) ;
               outBuilder.appendCode( StringData(outString) ) ;
               break ;
            }
            case  bson::Symbol:
            {
               std::string outString ;
               rc = convert( StringData(elem.valuestr()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert 'Symbol' charset "
                            "in BSON Array, rc: %d", rc ) ;
               outBuilder.appendSymbol( StringData(outString) ) ;
               break ;
            }
            case  bson::String:
            {
               std::string outString ;
               rc = convert( StringData(elem.valuestr()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert string charset "
                            "in BSON Array, rc: %d", rc ) ;
               outBuilder.append( outString ) ;
               break ;
            }
            case bson::Object:
            {
               BSONObjBuilder subBuilder( outBuilder.subobjStart() ) ;
               rc = _convert( elem.embeddedObject(), subBuilder ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert BSON object charset "
                            "in BSON Array, rc: %d", rc ) ;
               subBuilder.doneFast() ;
               break ;
            }
            case bson::Array:
            {
               BSONArrayBuilder subBuilder( outBuilder.subarrayStart() ) ;
               rc = _convert( elem.embeddedObject(), subBuilder ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert BSON Array charset "
                            "in BSON Array, rc: %d", rc ) ;

               subBuilder.doneFast() ;
               break ;
            }
            case RegEx:
            {
               std::string outString, outOptions ;
               rc = convert( StringData(elem.regex()), outString ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert regex string charset "
                            "in BSON Array, rc: %d", rc ) ;

               rc = convert( StringData(elem.regexFlags()), outOptions ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert regex options charset "
                            "in BSON Array, rc: %d", rc ) ;
               outBuilder.appendRegex( outString, outOptions ) ;
               break ;
            }
            case DBRef:
            {
               BSONObjBuilder builder ;
               std::string outNS ;
               rc = convert( StringData(elem.dbrefNS()), outNS ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert DBRef charset "
                            "in BSON Array, rc: %d", rc ) ;
               builder.append( outNS, elem.dbrefNS() ) ;
               {
                  BSONObj dbRef = builder.obj() ;
                  BSONElement convertedElem = dbRef.firstElement() ;
                  outBuilder.append( convertedElem ) ;
               }
               break ;
            }
            case CodeWScope:
            {
               std::string outCode ;
               BSONObj outCodeObj ;
               rc = convert( StringData(elem.codeWScopeCode()), outCode ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert CodeWScope charset, "
                            "in BSON Array, rc: %d", rc ) ;

               rc = convert( elem.codeWScopeObject(), outCodeObj ) ;
               PD_RC_CHECK( rc, PDERROR, "Failed to convert CodeWScope charset, "
                            "in BSON Array, rc: %d", rc ) ;

               outBuilder.appendCodeWScope( outCode, outCodeObj ) ;
               break ;
            }
            default:
            {
               SDB_ASSERT( 0, "Unknown bson type" ) ;
               break ;
            }
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }
}  // namespace engine
