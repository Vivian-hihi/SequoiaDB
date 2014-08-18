/*******************************************************************************

   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnLobWindow.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_LOBWINDOW_HPP_
#define RTN_LOBWINDOW_HPP_

#include "dmsLobDef.hpp"

namespace engine
{
   class _rtnLobWindow : public SDBObject
   {
   public:
      _rtnLobWindow() ;
      virtual ~_rtnLobWindow() ;

   private:
      struct pieceFromUsr
      {
         SINT64 offset ;
         UINT32 len ;
         const CHAR *data ;
         pieceFromUsr()
         :offset( 0 ),
          len( 0 ),
          data( NULL )
         {

         }

         BOOLEAN empty() const
         {
            return 0 == offset &&
                   len == 0 && NULL == data ;
         }

         void clear()
         {
            offset = 0 ;
            len = 0 ;
            data = NULL ;
         }
      } ;

   public:
      /// do not destruct meta until rtnLobWindow is released.
      INT32 init( const bson::OID *oid, INT32 pageSize ) ;

      INT32 prepare2Write( SINT64 offset, UINT32 len, const CHAR *data ) ;

      BOOLEAN getNextWriteSequence( _dmsLobRecord &piece ) ;

      /// do not call this func until u have write all sequences.
      void cacheLastDataOrClearCache() ;

      BOOLEAN getCachedData( _dmsLobRecord &piece ) ;

      INT32 prepare2Read( SINT64 lobLen, SINT64 offset,
                          UINT32 len, UINT32 &readLen,
                          UINT32 maxPieceNum,
                          _dmsLobRecord *pieces,
                          UINT32 &pieceNum ) ;

//      BOOLEAN getNextReadSequence( _dmsLobRecord &piece ) ;

   private:
      const bson::OID *_oid ;
      SINT32 _pageSize ;
      UINT32 _logarithmic ;

      SINT64 _curOffset ;
      CHAR *_pool ;
      INT32 _cachedSz ;
      pieceFromUsr _writeData ;
      BOOLEAN _analysisCache ;
   } ;
   typedef class _rtnLobWindow rtnLobWindow ;
}

#endif

