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

   Source File Name = charsetICUConvertorFactory.cpp

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
#include "charsetConvertorFactory.hpp"
#include "boost/move/unique_ptr.hpp"
#include "charsetDef.hpp"
#include "charsetICUConvertor.hpp"
#include "charsetUtils.hpp"
#include "ossTypes.h"
#include "pd.hpp"

using namespace bson ;
namespace engine
{
   static charsetConvertorInterface*
      convertors[CHARSET_SUPPORTED_CHARSET_NUM][CHARSET_SUPPORTED_CHARSET_NUM] ;
   static bool isConvertorInitialized = false ;

   // Initialize charset convertor
   void charsetConvertorFactory::init() 
   {
      PD_LOG( PDINFO, "Init charset convertor, charset number: %d",
              CHARSET_SUPPORTED_CHARSET_NUM ) ;
      for ( uint32_t in = 0 ; in < CHARSET_SUPPORTED_CHARSET_NUM ; ++in )
      {
         for ( uint32_t out = 0 ; out < CHARSET_SUPPORTED_CHARSET_NUM ; ++out )
         {
            if ( in == out )
            {
               convertors[in][out] = NULL;
               continue;
            }

            Charset inCharset = static_cast<Charset>(in) ;
            Charset outCharset = static_cast<Charset>(out) ;
            convertors[in][out] = _create( inCharset, outCharset ) ;
            if ( !convertors[in][out] )
            {
               StringData in = charsetSerializer( inCharset ) ;
               StringData out = charsetSerializer( outCharset ) ;
               PD_LOG( PDWARNING,
                       "Failed to build convertor from %s to %s",
                       in.data(), out.data() ) ;
            }
         }
      }
      isConvertorInitialized = true ;
   }

   void charsetConvertorFactory::deinit()
   {
      for ( uint32_t in = 0 ; in < CHARSET_SUPPORTED_CHARSET_NUM ; ++in )
      {
         for ( uint32_t out = 0 ; out < CHARSET_SUPPORTED_CHARSET_NUM ; ++out )
         {
            if ( convertors[in][out] )
            {
               delete convertors[in][out] ;
            }
         }
      }
   }

   // Get charset convertor
   boost::movelib::unique_ptr<charsetConvertorInterface>
      charsetConvertorFactory::get( Charset inCharset, Charset outCharset )
   {
      uint32_t in = static_cast<uint32_t>(inCharset) ;
      uint32_t out = static_cast<uint32_t>(outCharset) ;
      SDB_ASSERT( in < CHARSET_SUPPORTED_CHARSET_NUM, 
                  "Invalid input charset number" ) ;
      SDB_ASSERT( out < CHARSET_SUPPORTED_CHARSET_NUM,
                  "Invalid output charset number" ) ;
      charsetConvertorInterface *cnv = NULL;
      if ( convertors[in][out] )
      {
         cnv = convertors[in][out]->make_clone() ;
      }
      return boost::movelib::unique_ptr< charsetConvertorInterface > ( cnv ) ;
   }

   // Build charset convertor
   charsetConvertorInterface*
      charsetConvertorFactory::_create( Charset inCharset, Charset outCharset )
   {
      return charsetICUConvertor::make( inCharset, outCharset ) ;
   }
}  // namespace engine
