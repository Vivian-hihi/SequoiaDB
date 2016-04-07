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
      _strAreaSize = 0 ;
   }

   _utilLZWDictionary::~_utilLZWDictionary()
   {
      reset() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTIONARY_INIT, "_utilLZWDictionary::init" )
   INT32 _utilLZWDictionary::init( UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILLZWDICTIONARY_INIT ) ;

      // TBD: Need to calculate the maximum code number.
      UINT32 maxNodeNum = 150000 ;
      UINT32 totalSize =
            sizeof( utilLZWDictHead ) + sizeof( utilLZWNode ) * maxNodeNum ;

      _dictBuff = ( CHAR *)SDB_OSS_MALLOC( totalSize ) ;
      PD_CHECK( _dictBuff, SDB_OOM, error, PDERROR,
                "Failed to allocate memory for dictionary, requested size: %d",
                totalSize ) ;

      _head = ( utilLZWDictHead *)_dictBuff ;
      _nodes = ( utilLZWNode *)( (CHAR *)_head + sizeof( utilLZWDictHead ) ) ;
      _maxNodeNum = maxNodeNum ;

      for ( UINT32 i = 0; i < maxNodeNum; ++i )
      {
         _nodes[i]._prev = DICT_INVALID_NODE ;
         _nodes[i]._first = DICT_INVALID_NODE ;
         _nodes[i]._next = DICT_INVALID_NODE ;
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
      _strAreaSize = 0 ;

      PD_TRACE_EXIT( SDB__UTILLZWDICTIONARY_RESET ) ;
   }

   void _utilLZWDictionary::_formatOneGrp( UINT32 parentIdx, UINT32 &nextIdx )
   {
      LZW_CODE code ;

      /*
       * Set parent's child as nextIdx, if it does has child.
       * Copy all nodes of parent to the position starts from nextIdx.
       * Insert the code into the code map at the same time.
       */
      code = _codeMap[parentIdx] ;
      code = _nodes[code]._first ;

      if ( DICT_INVALID_NODE == code )
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
      } while ( DICT_INVALID_NODE != code ) ;

   done:
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
      LZW_CODE matchCode = DICT_INVALID_NODE ;
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

   INT32 _utilLZWDictionary::_initFinalEnv( CHAR *buff, UINT32 bufLen)
   {
      // TBD: Need to check the length.
      ossMemset( buff, 0, bufLen ) ;
      ossMemcpy( buff, _head, sizeof( utilLZWDictHead ) ) ;

      /*
       * The original space is managed using _dictBuff, _head can pointer to the
       * new position.
       */
      _head = ( utilLZWDictHead * )buff ;
      attach( (utilDictHandle)_head ) ;
      _strAreaSize = bufLen - ( _strArea - (CHAR*)_head ) ;

      return SDB_OK ;
   }

   void _utilLZWDictionary::_formatOneCode( UINT32 &offset, LZW_CODE code )
   {
      UINT32 len = _nodes[code]._len ;
      DST_ITEM *item = &_dst[code] ;
      CHAR buff[DST_MAX_LOCAL_LEN] = {0} ;
      LZW_CODE preCode ;
      BOOLEAN remote = FALSE ;

      if ( len > DST_MAX_LOCAL_LEN )
      {
         *item = 0 ;
         DST_SET_REMOTE_FLAG( *item ) ;
         DST_SET_REMOTE_POS( *item, offset ) ;
         getStr( code, (UINT8*)(_strArea + offset), _strAreaSize - offset ) ;
         DST_SET_REMOTE_LEN( *item, len ) ;
         remote = TRUE ;
      }
      else
      {
         *item = 0 ;
         DST_UNSET_REMOTE_FLAG( *item ) ;
         DST_SET_LOCAL_LEN( *item, _nodes[code]._len ) ;
         getStr( code, (UINT8*)buff, DST_MAX_LOCAL_LEN ) ;
         DST_SET_LOCAL_STR( *item, buff, _nodes[code]._len ) ;
      }

      /* Set all its substrings */
      preCode = _nodes[code]._prev ;
      while ( DICT_INVALID_NODE != preCode )
      {
         item = &_dst[preCode] ;
         *item = 0 ;
         if ( _nodes[preCode]._len > DST_MAX_LOCAL_LEN )
         {
            DST_SET_REMOTE_FLAG( *item ) ;
            DST_SET_REMOTE_POS( *item, offset ) ;
            DST_SET_REMOTE_LEN( *item, _nodes[preCode]._len ) ;
         }
         else
         {
            DST_UNSET_REMOTE_FLAG( *item ) ;
            DST_SET_LOCAL_LEN( *item, _nodes[preCode]._len ) ;
            getStr( preCode, (UINT8*)buff, DST_MAX_LOCAL_LEN ) ;
            DST_SET_LOCAL_STR( *item, buff, _nodes[preCode]._len ) ;
         }

         preCode = _nodes[preCode]._prev ;
      }

      if ( remote )
      {
         offset += len ;
      }
   }

   void _utilLZWDictionary::_formatDst()
   {
      LZW_CODE currCode ;
      UINT32 strOffset = 0 ;

      /*
       * Scan the nodes, find one longest string, format it and all its
       * substrings and put them into the decompression part.
       */
      for ( currCode = 0; currCode <= _head->_maxCode; ++currCode )
      {
         if ( DICT_INVALID_NODE != _nodes[currCode]._first )
         {
            continue ;
         }

         _formatOneCode( strOffset, currCode ) ;
      }

      _finalSize = _strArea - (CHAR *)_head + strOffset ;
   }

   INT32 _utilLZWDictionary::finalize( CHAR *stream, UINT32 &length )
   {
      // format the original dictionary into the final structure.
      INT32 rc = SDB_OK ;
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

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__UTILLZWDICTCREATOR_PREPARE, "_utilLZWDictionary::prepare" )
   INT32 _utilLZWDictCreator::prepare( UINT32 maxSize )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB__UTILLZWDICTCREATOR_PREPARE ) ;
      PD_CHECK( maxSize >= MIN_DICT_SIZE, SDB_INVALIDARG, error, PDERROR,
                "Dictionary size provided is too small: %d. The mininum requred"
                " size is: %d", maxSize, MIN_DICT_SIZE ) ;

      rc = _dictionary.init( maxSize ) ;
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
      LZW_CODE code = DICT_INVALID_NODE ;
      LZW_CODE nextCode = DICT_INVALID_NODE ;
      LZW_CODE maxCode = _dictionary.getMaxNodeNum() - 1 ;

      ch = source[0] ;
      code = ch ;
      pos++ ;
      strLen++ ;
      full = FALSE ;

      for ( ; pos < sourceLen; ++pos )
      {
         ch = source[pos] ;
         nextCode = _dictionary.findStr( code, ch ) ;
         if ( DICT_INVALID_NODE == nextCode )
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
         }
      }

   done:
      PD_TRACE_EXITRC( SDB__UTILLZWDICTCREATOR_BUILD, rc ) ;
      return rc ;
   }
}

