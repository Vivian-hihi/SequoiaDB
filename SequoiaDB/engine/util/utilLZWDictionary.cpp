/*******************************************************************************


   Copyright (C) 2011-2016 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = utilLZWDictionary.cpp

   Descriptive Name = Implementation of LZW dictionary.

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2015  YSD Initial Draft

   Last Changed =

*******************************************************************************/
#include "pd.hpp"
#include "pdTrace.hpp"
#include "utilTrace.hpp"
#include "utilCompressor.hpp"
#include "utilLZWDictionary.hpp"

namespace engine
{
   _utilLZWDictionary::_utilLZWDictionary()
   {
      _dictBuff = NULL ;
      _head = NULL ;
      _nodes = NULL ;
      _maxNodeNum = 0 ;
      _finalSize = 0 ;
      _cst = NULL ;
      _codeMap = NULL ;
      _dst = NULL ;
      _strArea = NULL ;
   }

   _utilLZWDictionary::~_utilLZWDictionary()
   {
      reset() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY_INIT, "_utilLZWDictionary::init" )
   INT32 _utilLZWDictionary::init()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY_INIT ) ;

      UINT32 totalSize = sizeof( utilLZWDictHead )
                         + sizeof( utilLZWNode ) * UTIL_MAX_DICT_ITEM_NUM ;

      _dictBuff = ( CHAR *)SDB_OSS_MALLOC( totalSize ) ;
      PD_CHECK( _dictBuff, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary, requested size: %d",
                totalSize ) ;

      _head = ( utilLZWDictHead *)_dictBuff ;
      _nodes = ( utilLZWNode *)( (CHAR *)_head + sizeof( utilLZWDictHead ) ) ;
      _maxNodeNum = UTIL_MAX_DICT_ITEM_NUM ;

      for ( UINT32 i = 0; i < UTIL_MAX_DICT_ITEM_NUM; ++i )
      {
         _nodes[i]._prev = UTIL_INVALID_DICT_CODE ;
         _nodes[i]._first = UTIL_INVALID_DICT_CODE ;
         _nodes[i]._next = UTIL_INVALID_DICT_CODE ;
         /*
          * 1 byte(8 bits) can represent 256 characters. Every search will start
          * with them.
          */
         if ( i < 256 )
         {
            _nodes[i]._ch = i ;
            _nodes[i]._len = 1 ;
         }
         else
         {
            _nodes[i]._len = 0 ;
         }
      }

      _head->_maxCode = 255 ;
      _head->_codeSize = 8 ;

   done:
      PD_TRACE_EXITRC( SDB__UTILLZWDICTIONARY_INIT, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY_RESET, "_utilLZWDictionary::reset" )
   void _utilLZWDictionary::reset()
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY_RESET ) ;
      if ( _dictBuff )
      {
         SDB_OSS_FREE( _dictBuff ) ;
      }

      _dictBuff = NULL ;
      _head = NULL ;
      _nodes = NULL ;
      _maxNodeNum = 0 ;
      _finalSize = 0 ;
      _cst = NULL ;
      _codeMap = NULL ;
      _dst = NULL ;
      _strArea = NULL ;

      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY_RESET ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY__FORMATONEGRP, "_utilLZWDictionary::_formatOneGrp" )
   void _utilLZWDictionary::_formatOneGrp( UINT32 parentIdx, UINT32 &nextIdx )
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY__FORMATONEGRP ) ;
      LZW_CODE code ;

      /*
       * Set parent's child as nextIdx, if it does has child.
       * Copy all nodes of parent to the position starts from nextIdx.
       * Insert the code into the code map at the same time.
       */
      code = _codeMap[parentIdx] ;
      code = _nodes[code]._first ;

      if ( UTIL_INVALID_DICT_CODE == code )
      {
         CST_SET_CHILD( _cst[parentIdx], CST_INVALID_CHILD ) ;
         goto done ;
      }

      CST_SET_CHILD( _cst[parentIdx], nextIdx ) ;

      do
      {
         _codeMap[nextIdx] = code ;
         CST_SET_CHAR( _cst[nextIdx], _nodes[code]._ch ) ;
         code = _nodes[code]._next ;
         nextIdx++ ;
      } while ( UTIL_INVALID_DICT_CODE != code ) ;

   done:
      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY__FORMATONEGRP ) ;
      return ;
   }

#ifdef _DEBUG
   void _utilLZWDictionary::healthCheck()
   {
      /* Check the dictionary for possible errors.
       * 1. Get the string from the decompression table;
       * 2. Find the string in the compression table, and get the code from code
       *    map.
       * 3. Compare if the codes are the same.
       */
      #define DICT_MAX_STR_LEN 256
      UINT32 len = 0 ;
      LZW_CODE matchCode = UTIL_INVALID_DICT_CODE ;
      UINT32 matchLen = 0 ;
      BYTE buff[DICT_MAX_STR_LEN] = { 0 } ;

      for ( LZW_CODE code = 0; code <= _head->_maxCode; ++code )
      {
         len = getStrExt( code, buff, DICT_MAX_STR_LEN ) ;
         matchLen = len ;
         matchCode = findStrExt( buff, matchLen ) ;
         SDB_ASSERT( len == matchLen, "Length not match" ) ;
         SDB_ASSERT( matchCode == code, "Code not match" ) ;
      }
   }
#endif   /* _DEBUG */

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY__INITFINALENV, "_utilLZWDictionary::_initFinalEnv" )
   void _utilLZWDictionary::_initFinalEnv( CHAR *buff, UINT32 bufLen)
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY__INITFINALENV ) ;
      ossMemset( buff, 0, bufLen ) ;
      ossMemcpy( buff, _head, sizeof( utilLZWDictHead ) ) ;

      /*
       * The original space is managed using _dictBuff, _head can pointer to the
       * new position.
       */
      _head = ( utilLZWDictHead * )buff ;
      attach( (utilDictHandle)_head ) ;

      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY__INITFINALENV ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY__FORMATREMOTESTR, "_utilLZWDictionary::_formatRemoteStr" )
   UINT32 _utilLZWDictionary::_formatRemoteStr( LZW_CODE code, UINT32 &offset )
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY__FORMATREMOTESTR ) ;
      BYTE strBuff[UTIL_MAX_DICT_STR_LEN] = { 0 } ;
      UINT32 preLen = 0 ;
      BOOLEAN link = FALSE ;
      UINT32 totalLen = 0 ;
      DST_ITEM *currItem = &_dst[code] ;
      LZW_CODE preCode = code ;
      DST_ITEM *tmpItem = NULL ;
      CHAR localStr[DST_MAX_LOCAL_LEN] = { 0 } ;

      UINT32 pos = UTIL_MAX_DICT_STR_LEN ;

      while ( TRUE )
      {
         strBuff[--pos] = _nodes[preCode]._ch ;
         preCode = _nodes[preCode]._prev ;
         if ( UTIL_INVALID_DICT_CODE == preCode )
         {
            /* The total string is now in the buffer */
            break ;
         }

         if ( 0 != _dst[preCode] )
         {
            preLen = _nodes[preCode]._len ;
            if ( preLen <= 4 )
            {
               /* Copy all the string */
               continue ;
            }
            else
            {
               /* Set the link information */
               pos -= 3 ;
               strBuff[pos] = (BYTE)((preCode & 0x00ff0000) >> 16) ;
               strBuff[pos + 1] = (BYTE)((preCode & 0x0000ff00) >> 8) ;
               strBuff[pos + 2] = (BYTE)(preCode & 0x000000ff) ;
               link = TRUE ;
               DST_SET_LINK_FLAG( *currItem ) ;
               break ;
            }
         }
      }

      totalLen = UTIL_MAX_DICT_STR_LEN - pos ;

      ossMemcpy( _strArea + offset, &strBuff[pos], totalLen ) ;

      DST_SET_REMOTE_FLAG( *currItem ) ;
      DST_SET_REMOTE_POS( *currItem, offset ) ;
      DST_SET_REMOTE_LEN( *currItem, totalLen ) ;

      preCode = _nodes[code]._prev ;
      for ( INT32 i = 1; preCode != UTIL_INVALID_DICT_CODE; ++i )
      {
         tmpItem = &_dst[preCode] ;
         if ( *tmpItem != 0 )
         {
            SDB_ASSERT( 0 != (*tmpItem) >> 29, "wrong value" ) ;
            break ;
         }

         if ( _nodes[preCode]._len > DST_MAX_LOCAL_LEN )
         {
            DST_SET_REMOTE_FLAG( *tmpItem ) ;
            if ( link )
            {
               DST_SET_LINK_FLAG( *tmpItem ) ;
            }
            DST_SET_REMOTE_POS( *tmpItem, offset ) ;
            DST_SET_REMOTE_LEN( *tmpItem, totalLen - i ) ;
         }
         else
         {
            DST_SET_LOCAL_LEN( *tmpItem, _nodes[preCode]._len ) ;
            {
               UINT32 length = 0 ;
               length = DST_GET_LOCAL_LEN( _dst[preCode] ) ;
               SDB_ASSERT( 0 != length, "invalid length" ) ;
            }
            getStr( preCode, (UINT8*)localStr, DST_MAX_LOCAL_LEN ) ;
            DST_SET_LOCAL_STR( *tmpItem, localStr, _nodes[preCode]._len ) ;
         }

         preCode = _nodes[preCode]._prev ;
      }

      offset += totalLen ;

      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY__FORMATREMOTESTR ) ;
      return totalLen ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY__FORMATONECODE, "_utilLZWDictionary::_formatOneCode" )
   void _utilLZWDictionary::_formatOneCode( UINT32 &offset, LZW_CODE code )
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY__FORMATONECODE ) ;
      UINT32 len = _nodes[code]._len ;
      DST_ITEM *item = &_dst[code] ;
      CHAR buff[DST_MAX_LOCAL_LEN] = {0} ;
      LZW_CODE preCode ;

      SDB_ASSERT( 0 == *item, "dst item value should be init to 0" ) ;

      if ( len > DST_MAX_LOCAL_LEN )
      {
         len = _formatRemoteStr( code, offset ) ;
      }
      else
      {
         DST_SET_LOCAL_LEN( *item, _nodes[code]._len ) ;
         getStr( code, (UINT8*)buff, DST_MAX_LOCAL_LEN ) ;
         DST_SET_LOCAL_STR( *item, buff, _nodes[code]._len ) ;

         /* Set all its substrings */
         preCode = _nodes[code]._prev ;
         while ( UTIL_INVALID_DICT_CODE != preCode )
         {
            item = &_dst[preCode] ;
            if ( 0 == *item )
            {
               if ( _nodes[preCode]._len > DST_MAX_LOCAL_LEN )
               {
                  SDB_ASSERT( false, "should not be here" ) ;
                  DST_SET_REMOTE_FLAG( *item ) ;
                  DST_SET_REMOTE_POS( *item, offset ) ;
                  DST_SET_REMOTE_LEN( *item, _nodes[preCode]._len ) ;
               }
               else
               {
                  DST_SET_LOCAL_LEN( *item, _nodes[preCode]._len ) ;
                  getStr( preCode, (UINT8*)buff, DST_MAX_LOCAL_LEN ) ;
                  DST_SET_LOCAL_STR( *item, buff, _nodes[preCode]._len ) ;
               }
            }

            preCode = _nodes[preCode]._prev ;
         }
      }

      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY__FORMATONECODE ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY__FORMATDST, "_utilLZWDictionary::_formatDst" )
   void _utilLZWDictionary::_formatDst()
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY__FORMATDST ) ;
      LZW_CODE currCode = _head->_maxCode + 1 ;
      UINT32 strOffset = 0 ;

      do
      {
         --currCode ;
         if ( UTIL_INVALID_DICT_CODE == _nodes[currCode]._first )
         {
            _formatOneCode( strOffset, currCode ) ;
         }
      } while ( currCode > 0 ) ;

      _finalSize = _strArea - (CHAR *)_head + strOffset ;
      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY__FORMATDST ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY_FINALIZE, "_utilLZWDictionary::finalize" )
   INT32 _utilLZWDictionary::finalize( CHAR *stream, UINT32 &length )
   {
      /* format the original dictionary into the final structure */
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY_FINALIZE ) ;
      UINT32 index = 0 ;

      _initFinalEnv( stream, length ) ;

      // fill the node in cst, and put its code in the code map at the same time
      /* 1. fill the first 256 codes */
      for ( LZW_CODE code = 0; code < 256; ++code, ++index )
      {
         CST_SET_CHAR( _cst[index], _nodes[code]._ch ) ;
         _codeMap[index] = code ;
      }

      UINT32 nextIdx = 256 ;
      for ( index = 0; index < nextIdx; ++index )
      {
         _formatOneGrp( index, nextIdx ) ;
      }

      /* format the decompress part */
      _formatDst() ;
      length = _finalSize ;

      PD_TRACE_EXITRC( SDB__UTILLZWDICTIONARY_FINALIZE, rc ) ;
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTCREATOR_PREPARE, "_utilLZWDictionary::prepare" )
   INT32 _utilLZWDictCreator::prepare()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILLZWDICTCREATOR_PREPARE ) ;

      rc = _dictionary.init() ;
      PD_RC_CHECK( rc, PDERROR,
                   "Failed to initialize dictionary, rc: %d", rc ) ;

   done:
      PD_TRACE_EXITRC( SDB__UTILLZWDICTCREATOR_PREPARE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTCREATOR_RESET, "_utilLZWDictionary::reset" )
   void _utilLZWDictCreator::reset()
   {
      PD_TRACE_ENTRY( SDB__UTILLZWDICTCREATOR_RESET ) ;

      _dictionary.reset() ;

      PD_TRACE_EXIT( SDB__UTILLZWDICTCREATOR_RESET ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTCREATOR_BUILD, "_utilLZWDictionary::build" )
   INT32 _utilLZWDictCreator::build( const CHAR *source, UINT32 sourceLen,
                                     BOOLEAN &full )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILLZWDICTCREATOR_BUILD ) ;
      UINT8 ch = 0 ;
      UINT32 pos = 0 ;
      UINT32 strLen = 0 ;
      BOOLEAN restart = FALSE ;
      LZW_CODE code = UTIL_INVALID_DICT_CODE ;
      LZW_CODE nextCode = UTIL_INVALID_DICT_CODE ;
      LZW_CODE maxCode = _dictionary.getMaxNodeNum() - 1 ;

      ch = source[0] ;
      code = ch ;
      pos++ ;
      strLen++ ;
      full = FALSE ;

      for ( ; pos < sourceLen; ++pos )
      {
         if ( !restart )
         {
            ch = source[pos] ;
         }
         else
         {
            code = source[pos] ;
            if ( pos + 1 == sourceLen )
            {
               break ;
            }

            ch = source[++pos] ;
            restart = FALSE ;
         }

         nextCode = _dictionary.findStr( code, ch ) ;
         if ( UTIL_INVALID_DICT_CODE == nextCode )
         {
            nextCode = _dictionary.addStr( code, ch ) ;
            if ( nextCode == maxCode )
            {
               /* Dictionary is full */
               full = TRUE ;
               //_dictionary.healthCheck() ;

               goto done ;
            }

            code = ch ;
            strLen = 1 ;
         }
         else
         {
            code = nextCode ;
            strLen++ ;
            if ( UTIL_MAX_DICT_STR_LEN == strLen )
            {
               restart = TRUE ;
            }
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__UTILLZWDICTCREATOR_BUILD, rc ) ;
      return rc ;
   }
}

