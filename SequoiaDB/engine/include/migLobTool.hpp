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

   Source File Name = migLobTool.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef MIG_LOBTOOL_HPP_
#define MIG_LOBTOOL_HPP_

#include "oss.hpp"
#include "client.hpp"
#include "ossIO.hpp"
#include "../bson/bson.hpp"
#include "migLobDef.hpp"

namespace lobtool
{
   class _migLobTool : public SDBObject
   {
   public:
      _migLobTool() ;
      virtual ~_migLobTool() ;

   public:
      INT32 exec( const bson::BSONObj &options ) ;

   private:
      INT32 _createFile( const CHAR *fullPath ) ;

      INT32 _openFile( const CHAR *fullPath ) ;

      INT32 _write( const CHAR *buf, UINT32 size ) ;

      void _initFileHeader( migFileHeader *header ) ;

      INT32 _initDB( const migOptions &ops ) ;

      void _closeDB() ;

      INT32 _exportLob( const migOptions &ops ) ;

      INT32 _append2File( const bson::BSONObj &obj ) ;

      INT32 _truncate( INT64 len ) ;

      INT32 _refreshHeader( UINT64 totalNum ) ;

   private:
      OSSFILE _file ;
      CHAR *_buf ;
      UINT32 _bufSize ;
      UINT32 _written ;

      sdbclient::sdb _db ;
      sdbclient::sdbCollection _cl ;
   } ;
   typedef class _migLobTool migLobTool ;
}
#endif

