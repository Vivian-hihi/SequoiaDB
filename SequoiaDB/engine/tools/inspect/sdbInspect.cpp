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

   Source File Name = sdbConsistencyInspect.cpp

   Descriptive Name = N/A

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains code logic for
   data insert/update/delete. This file does NOT include index logic.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/17/2014  LZ  Initial Draft

   Last Changed =

*******************************************************************************/
#include "sdbInspect.hpp"
#include "ossUtil.hpp"
#include "utilParam.hpp"
#include "ossVer.hpp"
#include "client.hpp"

using namespace engine;

namespace
{
   /**
    ** get the oid bson object
    ***/
   void getOID( const bson::BSONElement &e, bson::OID &id )
   {
      bson::BSONObj o;
      id = e.OID() ;
   }

   /**
    ** get the index of min bson object
    ***/
   INT32 getMinObjectIndex( ciBson &doc, const INT32 nodeCount )
   {
      bson::OID oid ;
      bson::BSONElement e ;

      INT32 idx = 0 ;
      INT32 minIndex = 0 ;
      // find the first valid object
      while ( doc.objs[idx].isEmpty() )
      {
         ++idx ;
         minIndex = idx ;
      }
      // get the id of object
      if ( doc.objs[0].getObjectID( e ) )
      {
         getOID( e, oid ) ;
      }
      // compare to other object
      for ( ; idx < nodeCount ; ++idx )
      {
         if ( !doc.objs[idx].isEmpty() )
         {
            bson::OID id ;
            if ( doc.objs[idx].getObjectID( e ) )
            {
               getOID( e, id );
               if ( id < oid )
               {
                  oid = id ;
                  minIndex = idx ;
               }
            }
         }
      }

      return minIndex ;
   }

   INT32 dumpCiHeader( const ciHeader *header, CHAR *&buffer,
                       INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc  = SDB_OK ;
      INT64 len = 0 ;

   retry:
      if ( bufferSize - 1 <= len )
      {
         bufferSize += CI_BUFFER_BLOCK ;
         buffer = ( CHAR *)SDB_OSS_REALLOC( buffer, bufferSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer, size = "
                      << bufferSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      len = 0 ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "Tool Name    : sdbInspect"OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "Tool Version : %d.%d"OSS_NEWLINE,
                          header->_mainVersion, header->_subVersion ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "------------------------------"
                          OSS_NEWLINE""OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;

      len += ossSnprintf( buffer + len, bufferSize - len,
                          "Parameters:"OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "Loop        : %d"OSS_NEWLINE,
                          header->_loop ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "action      : %s"OSS_NEWLINE,
                          header->_action ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "coorAddress : %s"OSS_NEWLINE,
                          header->_coordAddr ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "serviceName : %s"OSS_NEWLINE,
                          header->_serviceName ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "group       : %s"OSS_NEWLINE,
                          header->_groupName ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "cs name     : %s"OSS_NEWLINE,
                          header->_csName ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "cl name     : %s"OSS_NEWLINE,
                          header->_clName ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "file path   : %s"OSS_NEWLINE,
                          header->_filepath ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "output file : %s"OSS_NEWLINE,
                          header->_outfile ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "view        : %s"OSS_NEWLINE,
                          header->_view ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "------------------------------"
                          OSS_NEWLINE""OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      validSize = len ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   INT32 dumpCiGroupHeader( const ciGroupHeader *header, CHAR *&buffer,
                            INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc  = SDB_OK ;
      INT64 len = 0 ;

   retry:
      if ( bufferSize - 1 <= len )
      {
         bufferSize += CI_BUFFER_BLOCK ;
         buffer = ( CHAR *)SDB_OSS_REALLOC( buffer, bufferSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer, size = "
               << bufferSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      len = 0 ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "  Replica Group:"OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "  Group ID     : %d"OSS_NEWLINE,
                          header->_groupID ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "  Group Name   : %s"OSS_NEWLINE,
                          header->_groupName ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "  Nodes count  : %d"OSS_NEWLINE,
                          header->_nodeCount ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;

      validSize = len ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   INT32 dumpCiNode( ciLinkList< ciNode > &link, CHAR *&buffer,
                      INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc     = SDB_OK ;
      ciNode *node = NULL ;
      INT64 len    = 0 ;

   retry:
      if ( bufferSize - 1 <= len )
      {
         bufferSize += CI_BUFFER_BLOCK ;
         buffer = ( CHAR *)SDB_OSS_REALLOC( buffer, bufferSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer, size = "
               << bufferSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      len = 0 ;
      link.resetCurrentNode() ;
      node = link.getHead() ;
      while ( NULL != node )
      {
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "    Node index       : %d"OSS_NEWLINE,
                             node->_index) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "    Node ID          : %d"OSS_NEWLINE,
                             node->_nodeID ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "    Node HostName    : %s"OSS_NEWLINE,
                             node->_hostname ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "    Node ServiceName : %s"OSS_NEWLINE,
                             node->_serviceName ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         len += ossSnprintf( buffer + len, bufferSize - len, OSS_NEWLINE ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;

         node = link.next() ;
      }

      validSize = len ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   INT32 dumpCiClHeader( const ciClHeader *header, CHAR *&buffer,
                         INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc  = SDB_OK ;
      INT64 len = 0 ;

   retry:
      if ( bufferSize - 1 <= len )
      {
         bufferSize += CI_BUFFER_BLOCK ;
         buffer = ( CHAR *)SDB_OSS_REALLOC( buffer, bufferSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer, size = "
                      << bufferSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      len = 0 ;
      len += ossSnprintf( buffer + len, bufferSize - len,
                          "  Collection Full Name  : %s"OSS_NEWLINE,
                          header->_fullname ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      if ( header->_recordCount <= 0 )
      {
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "    There is no record different"OSS_NEWLINE ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      }
      len += ossSnprintf( buffer + len, bufferSize - len, OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;

      validSize = len ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   INT32 dumpCiRecord( const INT32 nodeCount, ciLinkList< ciRecord > &link,
                       CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc     = SDB_OK ;
      ciRecord *rd = NULL ;
      INT64 len    = 0 ;

   retry:
      if ( bufferSize - 1 <= len )
      {
         bufferSize += CI_BUFFER_BLOCK ;
         buffer = ( CHAR *)SDB_OSS_REALLOC( buffer, bufferSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer, size = "
               << bufferSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
      }

      len = 0 ;
      if ( link.count() > 0 )
      {
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "  # Node state 1 means node has the record,"
                             " or 0 means not."OSS_NEWLINE
                             "  # The order is ascended by node index."OSS_NEWLINE
                             "    There is [%d] piece of records that haven't been"
                             " synchronized."
                             OSS_NEWLINE""OSS_NEWLINE, link.count() ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      }
      else
      {
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "   There is no record different"
                             OSS_NEWLINE""OSS_NEWLINE ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
      }

      link.resetCurrentNode() ;
      rd = link.getHead() ;
      while ( NULL != rd )
      {
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "  -record     : %s"OSS_NEWLINE,
                             rd->_bson.toString().c_str() ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         len += ossSnprintf( buffer + len, bufferSize - len,
                             "  -Node State : " ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         for ( INT32 idx = 0 ; idx < nodeCount ; ++idx )
         {
            ciState st( rd->_state ) ;
            len += ossSnprintf( buffer + len, bufferSize - len,
                                   st.hit( idx ) ? " 1" : " 0" ) ;
            CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         }
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         len += ossSnprintf( buffer + len, bufferSize - len, OSS_NEWLINE ) ;
         CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;
         rd = link.next() ;
      }
      len += ossSnprintf( buffer + len, bufferSize - len, OSS_NEWLINE ) ;
      CHECK_VALUE( ( bufferSize - 1 <= len ), retry ) ;

      validSize = len ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   BOOLEAN findCiOffset( ciLinkList< ciOffset > &clOffset, const INT64 offset )
   {
      BOOLEAN find = FALSE ;

      clOffset.resetCurrentNode() ;
      ciOffset *off = clOffset.getHead() ;

      while ( NULL != off )
      {
         if ( offset == off->_offset )
         {
            find = TRUE ;
            goto done ;
         }
         off = clOffset.next() ;
      }

   done:
      return find ;
   }

   /**
    ** read data from file
    ***/
   INT32 readFromFile( OSSFILE &in, INT64 &offset,
                       CHAR *buffer, const INT64 readSize )
   {
      INT32 rc       = SDB_OK ;
      ///< read from file
      INT64 restLen  = readSize ;
      INT64 readPos  = 0 ;
      INT64 readLen  = 0 ;

      while( restLen > 0 )
      {
         rc = ossSeekAndRead( &in, offset, buffer + readPos,
                                           restLen, &readLen ) ;
         if ( SDB_OK != rc && SDB_INTERRUPT != rc )
         {
            std::cout << "Failed to read data from file" << std::endl ;
            goto error ;
         }

         rc = SDB_OK ;
         restLen -= readLen ;
         readPos += readLen ;
      }

      offset += readSize ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write buffer to file
    ***/
   INT32 writeToFile( OSSFILE &out, const CHAR *buffer, const INT64 bufferSize )
   {
      INT32 rc        = SDB_OK ;
      ///< write buffer
      INT64 restLen   = bufferSize ;
      INT64 writePos  = 0 ;
      INT64 writeSize = 0 ;
      while( restLen > 0 )
      {
         rc = ossWrite( &out, buffer + writePos, restLen, &writeSize ) ;
         if ( SDB_OK != rc && SDB_INTERRUPT != rc )
         {
            std::cout << "Failed to write data to file" << std::endl ;
            goto error ;
         }

         rc = SDB_OK ;
         restLen -= writeSize ;
         writePos += writeSize ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write padding to file
    ***/
   INT32 writePadding( OSSFILE &out, CHAR *&buffer,
                       INT64 &bufferSize, const INT32 paddingSize  )
   {
      INT32 rc = SDB_OK ;
      INT64 validSize = paddingSize ;

      if ( validSize > bufferSize )
      {
         buffer = ( CHAR * )SDB_OSS_REALLOC( buffer, validSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer. size = "
                      << validSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         bufferSize = validSize ;
      }

      ossMemset( buffer, 0, validSize ) ;
      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** read ciHeader from file
    ***/
   INT32 readCiHeader( OSSFILE &in, ciHeader *header )
   {
      INT32 rc          = SDB_OK ;
      INT32 len         = 0 ;
      INT32 mainVersion = 0 ;
      INT32 subVersion  = 0 ;
      INT64 tmpOffset   = 0 ;
      CHAR  eyeCatcher[ CI_EYECATCHER_SIZE ] = { 0 } ;
      CHAR  buffer[ CI_HEADER_SIZE ] = { 0 } ;

      rc = readFromFile( in, tmpOffset, buffer, CI_HEADER_SIZE ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      // try to copy 
      ossMemcpy( eyeCatcher, buffer, CI_EYECATCHER_SIZE ) ;
      if ( 0 != ossStrncmp( CI_HEADER_EYECATCHER,
                            eyeCatcher, CI_EYECATCHER_SIZE ) )
      {
         std::cout << "Error: eyeCatcher is invalid" << std::endl ;
         goto error ;
      }
      len += CI_EYECATCHER_SIZE ;

      // try copy main version
      ossMemcpy( &mainVersion, buffer + len, sizeof( INT32 ) ) ;
      if ( mainVersion > header->_mainVersion )
      {
         std::cout << "Error: the main version in file header is larger "
                   << "than the main version of current tools" << std::endl ;
         goto error ;
      }
      len += sizeof( INT32 ) ;

      //try copy sub version
      ossMemcpy( &subVersion, buffer + len, sizeof( INT32 ) ) ;
      if ( ( mainVersion == header->_mainVersion ) &&
           ( subVersion > header->_subVersion ) )
      {
         std::cout << "Error: the sub version in file header is larger "
                   << "than the sub version of current tools" << std::endl ;
         goto error ;
      }
      len += sizeof( INT32 ) ;

      // copy loop
      ossMemcpy( &header->_loop, buffer + len, sizeof(INT32) ) ;
      if ( 0 >= header->_loop )
      {
         std::cout << "Warning: the value of loop is invalid. "
                   << "Modify it to default value(5)." << std::endl ;
         header->_loop = 5 ;
      }
      len += sizeof( INT32 ) ;

      // copy actions
      ossMemcpy( header->_action, buffer + len, CI_ACTION_SIZE ) ;
      if ( 0 != ossStrncmp( CI_ACTION_INSPECT,
                            header->_action, CI_ACTION_SIZE ) &&
           0 != ossStrncmp( CI_ACTION_REPORT,
                            header->_action, CI_ACTION_SIZE ) )
      {
         std::cout << "Error: action is invalid" << std::endl ;
         goto error ;
      }
      len += CI_ACTION_SIZE ;

      // copy coord hostname
      ossMemcpy( header->_coordAddr, buffer + len, CI_HOSTNAME_SIZE + 1 ) ;
      len += CI_HOSTNAME_SIZE + 1 ;
      // copy coord service name
      ossMemcpy( header->_serviceName, buffer + len, CI_SERVICENAME_SIZE + 1 ) ;
      len += CI_SERVICENAME_SIZE + 1 ;
      // copy group name
      ossMemcpy( header->_groupName, buffer + len, CI_GROUPNAME_SIZE + 1 ) ;
      len += CI_GROUPNAME_SIZE + 1 ;
      // copy collection space name
      ossMemcpy( header->_csName, buffer + len, CI_CS_NAME_SIZE + 1 ) ;
      len += CI_CS_NAME_SIZE + 1 ;
      // copy collection name
      ossMemcpy( header->_clName, buffer + len, CI_CL_NAME_SIZE + 1) ;
      len += CI_CL_NAME_SIZE + 1 ;
      // skip file path
      ossMemcpy( header->_filepath, buffer + len, OSS_MAX_PATHSIZE + 1) ;
      len += OSS_MAX_PATHSIZE + 1 ;
      // skip out file
      ossMemcpy( header->_outfile, buffer + len, OSS_MAX_PATHSIZE + 1) ;
      len += OSS_MAX_PATHSIZE + 1 ;
      // copy view format string
      ossMemcpy( header->_view, buffer + len, CI_VIEWOPTION_SIZE + 1 ) ;
      len += CI_VIEWOPTION_SIZE + 1 ;
      // copy padding? it seems useless
      // skip first

   done:
      return rc ;
   error:
      rc = HEADER_PARSE_ERROR ;
      goto done ;
   }

   /**
    ** copy ciHeader data to buffer
    ***/
   INT32 ciHeaderToBuffer( const ciHeader *header, CHAR *&buffer,
                           INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc = SDB_OK ;
      INT32 pos = 0 ;

      validSize = CI_HEADER_SIZE ;
      if ( validSize > bufferSize )
      {
         buffer = ( CHAR * )SDB_OSS_REALLOC( buffer, validSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer. size = "
                      << validSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         bufferSize = validSize ;
      }

      ossMemcpy( buffer + pos, header->_eyeCatcher, CI_EYECATCHER_SIZE ) ;
      pos += CI_EYECATCHER_SIZE ;
      ossMemcpy( buffer + pos, &header->_mainVersion, sizeof(INT32) ) ;
      pos += sizeof( INT32 ) ;
      ossMemcpy( buffer + pos, &header->_subVersion, sizeof(INT32) ) ;
      pos += sizeof( INT32 ) ;
      ossMemcpy( buffer + pos, &header->_loop, sizeof(INT32) ) ;
      pos += sizeof( INT32 ) ;
      ossMemcpy( buffer + pos, header->_action, CI_ACTION_SIZE ) ;
      pos += CI_ACTION_SIZE ;
      ossMemcpy( buffer + pos, header->_coordAddr, CI_HOSTNAME_SIZE + 1 ) ;
      pos += CI_HOSTNAME_SIZE + 1 ;
      ossMemcpy( buffer + pos, header->_serviceName, CI_SERVICENAME_SIZE + 1 ) ;
      pos += CI_SERVICENAME_SIZE + 1 ;
      ossMemcpy( buffer + pos, header->_groupName, CI_GROUPNAME_SIZE + 1 ) ;
      pos += CI_GROUPNAME_SIZE + 1 ;
      ossMemcpy( buffer + pos, header->_csName, CI_CS_NAME_SIZE + 1 ) ;
      pos += CI_CS_NAME_SIZE + 1 ;
      ossMemcpy( buffer + pos, header->_clName, CI_CL_NAME_SIZE + 1 ) ;
      pos += CI_CL_NAME_SIZE + 1 ;
      ossMemcpy( buffer + pos, header->_filepath, OSS_MAX_PATHSIZE + 1 ) ;
      pos += OSS_MAX_PATHSIZE + 1 ;
      ossMemcpy( buffer + pos, header->_outfile, OSS_MAX_PATHSIZE + 1 ) ;
      pos += OSS_MAX_PATHSIZE + 1 ;
      ossMemcpy( buffer + pos, header->_view, CI_VIEWOPTION_SIZE + 1 ) ;
      pos += CI_VIEWOPTION_SIZE + 1 ;
      ossMemcpy( buffer + pos, header->_padding, CI_HEAD_PADDING_SIZE ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciHeader to file
    ***/
   INT32 writeCiHeader( OSSFILE &out, const ciHeader *header, CHAR *&buffer,
                                      INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc = SDB_OK ;

      rc = ciHeaderToBuffer( header, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** read ciGroupHeader from file
    ***/
   INT32 readCiGroupHeader( OSSFILE &in, INT64 &offset, ciGroupHeader *header )
   {
      INT32 rc  = SDB_OK ;
      INT32 len = 0 ;
      CHAR buffer[ CI_GROUP_HEADER_SIZE ] = { 0 } ;

      rc = readFromFile( in, offset, buffer, CI_GROUP_HEADER_SIZE ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      //offset += CI_GROUP_HEADER_SIZE ;

      ossMemcpy( &header->_groupID, buffer + len, sizeof( INT32 ) ) ;
      len += sizeof( INT32 ) ;
      ossMemcpy( &header->_nodeCount, buffer + len, sizeof( UINT32 ) ) ;
      len += sizeof( UINT32 ) ;
      ossMemcpy( &header->_clCount, buffer + len, sizeof( UINT32 ) ) ;
      len += sizeof( UINT32 ) ;
      ossMemcpy( &header->_groupName, buffer + len, CI_GROUPNAME_SIZE ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciGroupHeader to file
    ***/
   INT32 writeCiGroupHeader( OSSFILE &out, const ciGroupHeader *header )
   {
      INT32 rc  = SDB_OK ;
      INT32 len = 0 ;
      CHAR buffer[ CI_GROUP_HEADER_SIZE ] = { 0 } ;

      ossMemcpy( buffer + len, &header->_groupID, sizeof( INT32 ) ) ;
      len += sizeof( INT32 ) ;
      ossMemcpy( buffer + len, &header->_nodeCount, sizeof( UINT32 ) ) ;
      len += sizeof( UINT32 ) ;
      ossMemcpy( buffer + len, &header->_clCount, sizeof( UINT32 ) ) ;
      len += sizeof( UINT32 ) ;
      ossMemcpy( buffer + len, header->_groupName, CI_GROUPNAME_SIZE ) ;

      rc = writeToFile( out, buffer, CI_GROUP_HEADER_SIZE ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** read ciClHeader from file
    ***/
   INT32 readCiClHeader( OSSFILE &in, INT64 &offset, ciClHeader *header )
   {
      INT32 rc = SDB_OK ;
      INT32 len = 0 ;
      CHAR buffer[ CI_CL_HEADER_SIZE ] = { 0 } ;

      rc = readFromFile( in, offset, buffer, CI_CL_HEADER_SIZE ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      //offset += CI_CL_HEADER_SIZE ;

      ossMemcpy( &header->_recordCount, buffer + len, sizeof( UINT32 ) ) ;
      len += sizeof( UINT32 ) ;
      ossMemcpy( header->_fullname, buffer + len, CI_CL_FULLNAME_SIZE ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciClHeader to file
    ***/
   INT32 writeCiClHeader( OSSFILE &out, const ciClHeader *header )
   {
      INT32 rc  = SDB_OK ;
      INT32 len = 0 ;
      CHAR buffer[ CI_CL_HEADER_SIZE ] = { 0 } ;

      ossMemcpy( buffer + len, &header->_recordCount, sizeof( UINT32 ) ) ;
      len += sizeof( UINT32 ) ;
      ossMemcpy( buffer + len, header->_fullname, CI_GROUPNAME_SIZE ) ;

      rc = writeToFile( out, buffer, CI_CL_HEADER_SIZE ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** read ciNode from file
    ***/
   INT32 readCiNode( OSSFILE &in, INT64 &offset,
                     const ciGroupHeader &header, ciLinkList< ciNode > &nodes )
   {
      INT32 rc                    = SDB_OK ;
      CHAR buffer[ CI_NODE_SIZE ] = { 0 } ;
      INT32 pos                   = 0 ;
      UINT32 idx                  = 0 ;
      while ( idx < header._nodeCount )
      {
         pos = 0 ;
         ossMemset( buffer, 0, CI_NODE_SIZE ) ;

         rc = readFromFile( in, offset, buffer, CI_NODE_SIZE ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;
         //offset += CI_NODE_SIZE ;
         ciNode *node = nodes.createNode() ;
         if ( NULL == node )
         {
            std::cout << "Error: failed to allocate ciNode" << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }

         ossMemcpy( &node->_index, buffer + pos, sizeof( INT32 ) ) ;
         pos += sizeof( INT32 ) ;
         ossMemcpy( &node->_nodeID, buffer + pos, sizeof( INT32 ) ) ;
         pos += sizeof( INT32 ) ;
         ossMemcpy( node->_hostname, buffer + pos, CI_HOSTNAME_SIZE + 1) ;
         pos += CI_HOSTNAME_SIZE + 1 ;
         ossMemcpy( node->_serviceName, buffer + pos, CI_SERVICENAME_SIZE + 1 );

         nodes.add( node ) ;
         ++idx ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** copy ciNode data to buffer
    ***/
   INT32 ciNodeToBuffer( ciLinkList< ciNode > &nodes, CHAR *&buffer, 
                         INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc        = SDB_OK ;
      INT64 pos       = 0 ;
      ciNode *curNode = NULL ;
      UINT32 count    = nodes.count() ;
      INT32 unitLen   = CI_NODE_SIZE ;

      validSize = count * unitLen ;

      if ( validSize > bufferSize )
      {
         buffer = ( CHAR * )SDB_OSS_REALLOC( buffer, validSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer. size = "
                      << validSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         bufferSize = validSize ;
      }

      nodes.resetCurrentNode() ;
      curNode = nodes.getHead() ;
      while ( NULL != curNode )
      {
         ossMemcpy( buffer + pos, &curNode->_index, sizeof( INT32 ) ) ;
         pos += sizeof( INT32 ) ;
         ossMemcpy( buffer + pos, &curNode->_nodeID, sizeof( INT32 ) ) ;
         pos += sizeof( INT32 ) ;
         ossMemcpy( buffer + pos, curNode->_hostname, CI_HOSTNAME_SIZE + 1) ;
         pos += CI_HOSTNAME_SIZE + 1 ;
         ossMemcpy( buffer + pos, curNode->_serviceName,
                                  CI_SERVICENAME_SIZE + 1 ) ;
         pos += CI_SERVICENAME_SIZE + 1 ;
         curNode = nodes.next() ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciNode to file
    ***/
   INT32 writeCiNode( OSSFILE &out, ciLinkList< ciNode > &nodes,
                      CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc = SDB_OK ;

      rc = ciNodeToBuffer( nodes, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** get next record
    ***/
   void getNext( ciState &st, ciLinkList< ciCursor > &cursors,
                 const INT32 minIndex, ciBson &docs, BOOLEAN inited = TRUE )
   {
      INT32 rc = SDB_OK ;
      cursors.resetCurrentNode() ;
      ciCursor *cursor = cursors.getHead() ;
      INT32 idx = 0 ;
      while ( NULL != cursor )
      {
         // check last compare is all the same.
         // if not, get next record of cursor which contains min bson object.
         if ( st.hit( ALL_THE_SAME_BIT ) || ( st.hit( idx ) ) )
         {
            if ( NULL != cursor->_cursor )
            {
               rc = cursor->_cursor->next( docs.objs[ idx ] ) ;
               if ( SDB_OK != rc )
               {
                  docs.objs[ idx ] = sdbclient::_sdbStaticObject ;
                  rc = SDB_OK ; // need reset the return value
               }
            }
            else
            {
               if ( !inited )
               {
                  docs.objs[ idx ] = sdbclient::_sdbStaticObject ;
               }
            }
         }
         ++idx ;
         cursor = cursors.next() ;
      }

      if ( !inited )
      {
         for ( ; idx < MAX_NODE_COUNT ; ++idx )
         {
            docs.objs[ idx ] = sdbclient::_sdbStaticObject ;
         }
      }
   }

   /**
    ** get the cursor of records, the collection full name specified
    ***/
   INT32 getCiCursor( ciLinkList< ciNode > &nodes, const CHAR* clName,
                      ciLinkList< ciCursor > &cursors, bson::BSONObj &con,
                      BOOLEAN orderCon = FALSE )
   {
      INT32 rc = SDB_OK ;

      nodes.resetCurrentNode() ;
      ciNode *curNode = nodes.getHead() ;
      while ( NULL != curNode )
      {
         ciCursor *cursor = cursors.createNode() ;
         if ( NULL == cursor )
         {
            std::cout << "Error: failed to allocate ciCursor" << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }

         cursor->_nodeID = curNode->_nodeID ;
         cursor->_index = curNode->_index ;


         sdbclient::sdb *db = new sdbclient::sdb() ;
         if ( NULL == db )
         {
            std::cout << "Error: failed to allocate sdbclient::sdb"
                      << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }

         sdbclient::sdbCollection cl ;

         rc = db->connect( curNode->_hostname, curNode->_serviceName ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to connect to " << curNode->_hostname
                      << ":" << curNode->_serviceName << std::endl ;
            // goto error ;
            // we should not goto error, and we considerate that the
            // node is empty.
         }

         rc = db->getCollection( clName, cl ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to get collection:"
                      << clName << std::endl ;
            // goto error ;
            // we should not goto error, and we considerate that the
            // node is empty.
         }

         sdbclient::sdbCursor *cr = new sdbclient::sdbCursor() ;
         if ( NULL == cr )
         {
            std::cout << "Error: failed to allocate sdbclient::sdbCursor"
                      << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }

         if ( orderCon )
         {
            rc = cl.query( *cr, sdbclient::_sdbStaticObject,
                            sdbclient::_sdbStaticObject, con ) ;
         }
         else
         {
            rc = cl.query( *cr, con ) ;
         }
         if ( SDB_OK != rc )
         {
            std::cout << "Warning: failed to query" << std::endl ;
            // goto error ;
            // we should not goto error, and we considerate that the
            // node is empty.
         }
         else
         {
            cursor->_db = db ;
            cursor->_cursor = cr ;
         }

         cursors.add( cursor ) ;
         curNode = nodes.next() ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** query bson record in each cursor
    ***/
   BOOLEAN recordQuery( ciLinkList< ciCursor > &cursors,
                      ciState &state, bson::BSONObj &obj )
   {
      BOOLEAN same = FALSE ;
      INT32 idx = 0 ;
      INT32 count = cursors.count() ;
      ciBson docs ;

      state.reset() ;
      // it's a trick to make sure that all cursors can get next.
      state.set( ALL_THE_SAME_BIT ) ;
      getNext( state, cursors, 0, docs, FALSE ) ;
      state.reset() ;
      while ( idx < count )
      {
         if ( !docs.objs[ idx ].isEmpty() && docs.objs[ idx ].equal( obj ) )
         {
            state.set( idx ) ;
         }
         ++idx ;
      }

      if ( state._state == ( ( 1 << count ) - 1 ) || state._state == 0 )
      {
         same = TRUE ;
      }

      return same ;
   }

   /**
    ** read ciRecord from file
    ***/
   INT32 readCiRecord( OSSFILE &in, INT64 &offset,
                       ciLinkList< ciNode > &nodes,
                       const ciClHeader &header,
                       ciLinkList< ciRecord > &records, BOOLEAN dump = FALSE )
   {
      INT32 rc         = SDB_OK ;
      CHAR *bsonBuffer = NULL ;
      INT32 bufferLen  = 0 ;

      UINT32 idx = 0 ;
      while ( idx < header._recordCount )
      {
         INT32 recordLen = 0 ;
         CHAR  state ;
         rc = readFromFile( in, offset, (CHAR *)&recordLen, sizeof( INT32 ) ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;

         if ( recordLen > bufferLen )
         {
            bsonBuffer = (CHAR *)SDB_OSS_REALLOC( bsonBuffer, recordLen ) ;
            if ( NULL == bsonBuffer )
            {
               std::cout << "Error: failed to allocate buffer. size = "
                  << recordLen << std::endl ;
               rc = SDB_OOM ;
               goto error ;
            }
            bufferLen = recordLen ;
         }
         // read bson
         rc = readFromFile( in, offset, ( CHAR * )bsonBuffer, recordLen ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;

         // read state
         rc = readFromFile( in, offset, ( CHAR * )&state, sizeof( CHAR ) ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;

         // make a condition of query
         bson::BSONObj obj( bsonBuffer ) ;
         bson::BSONElement e ;
         obj.getObjectID( e ) ;
         bson::BSONObj con = bob().append( e ).obj() ;

         ciLinkList< ciCursor > cursors ;
         if ( !dump )
         {
            rc = getCiCursor( nodes, header._fullname, cursors, con ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;
         }

         ciState st;
         if ( dump || !recordQuery( cursors, st, obj ) )
         {
            ciRecord *record = records.createNode() ;
            if ( NULL == record )
            {
               std::cout << "Error: failed to allocate ciRecord "
                         << std::endl ;
               rc = SDB_OOM ;
               goto error ;
            }
            record->_bson = obj.copy() ;
            record->_len = obj.objsize() ;
            record->_state = dump ? state : st._state ;
            records.add( record ) ;
         }
         ++idx ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** copy record data to buffer
    ***/
   INT32 ciRecordToBuffer( ciLinkList< ciRecord > &records, CHAR *&buffer,
                           INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc            = SDB_OK ;
      INT64 pos           = 0 ;
      ciRecord *curRecord = NULL ;

      validSize = 0 ;

      records.resetCurrentNode() ;
      curRecord = records.getHead() ;
      while ( NULL != curRecord )
      {
         validSize += sizeof( INT32 ) ;
         validSize += curRecord->_len ;
         validSize += sizeof( CHAR ) ;
         curRecord = records.next() ;
      }

      if ( validSize > bufferSize )
      {
         buffer = ( CHAR * )SDB_OSS_REALLOC( buffer, validSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer. size = "
                      << validSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         bufferSize = validSize ;
      }

      records.resetCurrentNode() ;
      curRecord = records.getHead() ;
      while ( NULL != curRecord )
      {
         ossMemcpy( buffer + pos, &curRecord->_len, sizeof( INT32 ) ) ;
         pos += sizeof( INT32 ) ;
         INT32 size = curRecord->_bson.objsize() ;
         ossMemcpy( buffer + pos, curRecord->_bson.objdata(), size ) ;
         pos += size ;
         ossMemcpy( buffer + pos, &curRecord->_state, sizeof( CHAR ) ) ;
         pos += 1 ;

         curRecord = records.next() ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciRecord to file
    ***/
   INT32 writeCiRecord( OSSFILE &out, ciLinkList< ciRecord > &records,
                        CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc = SDB_OK ;

      rc = ciRecordToBuffer( records, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** read ciOffset from file
    ***/
   INT32 readCiOffset( OSSFILE &in, INT64 &offset,
                       const INT32 count, ciLinkList< ciOffset > &clo )
   {
      INT32 rc = SDB_OK ;
      INT32 idx = 0 ;
      INT32 pos = 0 ;
      CHAR *buffer = NULL ;

      INT32 readSize = count * sizeof( INT64 ) ;
      buffer = ( CHAR * )SDB_OSS_MALLOC( readSize ) ;
      if ( NULL == buffer )
      {
         std::cout << "Error: failed to allocate memory. size = "
                   << readSize << std::endl ;
         rc = SDB_OOM ;
         goto error ;
      }

      rc = readFromFile( in, offset, buffer, readSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      while ( idx < count )
      {
         ciOffset *clOff = clo.createNode() ;
         if ( NULL == clOff )
         {
            std::cout << "Error: failed to allocate ciClOffset" << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }

         ossMemcpy( &clOff->_offset, buffer + pos, sizeof( INT64 ) ) ;
         pos += sizeof( INT64 ) ;

         clo.add( clOff ) ;
         ++idx ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** copy ciClOffset data to buffer
    ***/
   INT32 ciOffsetToBuffer( ciLinkList< ciOffset > &offsets,
                           CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc            = SDB_OK ;
      INT64 pos           = 0 ;
      ciOffset *curNode = NULL ;
      UINT32 count        = offsets.count() ;
      INT32 unitLen       = sizeof( INT64 ) ;

      validSize = count * unitLen ;

      if ( validSize > bufferSize )
      {
         buffer = ( CHAR * )SDB_OSS_REALLOC( buffer, validSize ) ;
         if ( NULL == buffer )
         {
            std::cout << "Error: failed to allocate buffer. size = "
                      << validSize << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         bufferSize = validSize ;
      }

      //ossMemcpy( buffer + pos, &count, sizeof( INT32 ) ) ;
      //pos += sizeof( INT32 ) ;
      offsets.resetCurrentNode() ;
      curNode = offsets.getHead() ;
      while ( NULL != curNode )
      {
         ossMemcpy( buffer + pos, &curNode->_offset, sizeof( INT64 ) ) ;
         pos += sizeof( INT64 ) ;
         curNode = offsets.next() ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciClOffset to file
    ***/
   INT32 writeCiClOffset( OSSFILE &out, ciLinkList< ciOffset > &offsets,
                          CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc = SDB_OK ;

      rc = ciOffsetToBuffer( offsets, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** read ciTail from file
    ***/
   INT32 readCiTail( OSSFILE &in, const INT64 offset, ciTail *tail )
   {
      INT32 rc        = SDB_OK ;
      INT64 tmpOffset = offset ;

      rc = readFromFile( in, tmpOffset,
                             ( CHAR * )&tail->_exitCode, sizeof( INT32 ) ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      rc = readFromFile( in, tmpOffset,
                         ( CHAR * )&tail->_groupCount, sizeof( UINT32 ) ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      rc = readCiOffset( in, tmpOffset, tail->_groupCount, tail->_groupOffset );
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** write ciTail to file
    ***/
   INT32 writeCiTail( OSSFILE &out, ciTail *tail,
                      CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc = SDB_OK ;
      INT32 paddingSize = 0 ;
      INT32 len = 0 ;

      SDB_ASSERT( ( tail->_groupCount == tail->_groupOffset.count() ),
                    "count of group is valid" ) ;

      rc = writeToFile( out,
                        ( const CHAR * )&tail->_exitCode, sizeof( INT32 ) ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      len += sizeof( INT32 ) ;

      rc = writeToFile( out,
                        ( const CHAR * )&tail->_groupCount, sizeof( UINT32 ) ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      len += sizeof( UINT32 ) ;

      rc = writeCiClOffset( out, tail->_groupOffset,
                                 buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      validSize += len ;

      paddingSize = CI_TAIL_SIZE - validSize ;
      rc = writePadding( out, buffer, bufferSize, paddingSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      validSize += paddingSize ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   INT32 dumpOneCl( OSSFILE &in, OSSFILE &out, ciOffset *groupOffset,
                    ciLinkList< ciOffset > &clOffsets, const CHAR *clName,
                    CHAR *&buffer, INT64 &bufferSize, INT64 &validSize )
   {
      INT32 rc     = SDB_OK ;
      UINT32 idx   = 0 ;
      INT64 offset = 0 ;
      ciGroupHeader header ;
      ciLinkList< ciNode > nodes ;

      if ( NULL == groupOffset )
      {
         goto done ;
      }

      offset = groupOffset->_offset ;
      // read group
      rc = readCiGroupHeader( in, offset, &header ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      // read nodes
      nodes.clear() ;
      rc = readCiNode( in, offset, header, nodes ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      while ( idx < header._clCount )
      {
         ciClHeader clHeader ;
         ciLinkList< ciRecord > records ;
         if ( !findCiOffset ( clOffsets, offset ) )
         {
            INT64 clOffset = offset ;
            rc = readCiClHeader( in, offset, &clHeader ) ;
            if ( SDB_OK != rc )
            {
               std::cout << "Error: failed to get ciClHeader" << std::endl ;
               goto error ;
            }

            if ( NULL == clName || 0 == ossStrncmp( clName, clHeader._fullname,
               CI_CL_NAME_SIZE ) )
            {
               // find and remember the offset
               ciOffset *cl = clOffsets.createNode() ;
               if ( NULL == cl )
               {
                  std::cout << "Error: failed to allocate ciOffset"
                     << std::endl;
                  rc = SDB_OOM ;
                  goto error ;
               }
               cl->_offset = clOffset ;
               clOffsets.add( cl ) ;

               // dump
               rc = dumpCiClHeader( &clHeader, buffer, bufferSize, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               rc = writeToFile( out, buffer, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               // dump group and node, if view option is "collection"
               // dump group
               rc = dumpCiGroupHeader( &header, buffer, bufferSize, validSize );
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               rc = writeToFile( out, buffer, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               // dump nodes
               rc = dumpCiNode( nodes, buffer, bufferSize, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               rc = writeToFile( out, buffer, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;

               if ( clHeader._recordCount > 0 )
               {
                  records.clear() ;

                  rc = readCiRecord( in, offset, nodes,
                                     clHeader, records, TRUE ) ;
                  CHECK_VALUE( ( SDB_OK != rc ), error ) ;
                  rc = dumpCiRecord( nodes.count(), records,
                     buffer, bufferSize, validSize ) ;
                  CHECK_VALUE( ( SDB_OK != rc ), error ) ;
                  rc = writeToFile( out, buffer, validSize ) ;
                  CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               }

               rc = dumpOneCl( in, out, groupOffset->_next, clOffsets,
                                        clHeader._fullname, buffer,
                                        bufferSize, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;

               // collection found and dumped, then exit
               //goto done ;
            }
         }

         ++idx ;
      }
   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** initialize header with file specified
    ***/
   INT32 initialize( ciHeader *header )
   {
      INT32 rc                            = SDB_OK ;
      INT64 fileSize                      = 0 ;
      BOOLEAN opened                      = FALSE ;
      OSSFILE file ;
      ciHeader oldheader ;

      rc = ossOpen( header->_filepath, OSS_READONLY, OSS_RU, file ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to open file specified" << std::endl ;
         goto error ;
      }
      opened = TRUE ;

      rc = ossGetFileSize( &file, &fileSize ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: fail to get file size" << std::endl ;
         goto error ;
      }

      if ( SDB_OK != rc )
      {
         std::cout << "Error: file size is less than " << CI_HEADER_SIZE
                   << std::endl ;
         goto error ;
      }

      rc = readCiHeader( file, &oldheader ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      // skip eye catcher
      // skip main version
      // skip sub version
      // copy loop
      ossMemcpy( &header->_loop, &oldheader._loop, sizeof(INT32) ) ;
      // copy actions
      ossMemcpy( header->_action, oldheader._action, CI_ACTION_SIZE ) ;
      // copy coord hostname
      ossMemcpy( header->_coordAddr,
                 oldheader._coordAddr, CI_HOSTNAME_SIZE + 1 ) ;
      // copy coord service name
      ossMemcpy( header->_serviceName,
                 oldheader._serviceName, CI_SERVICENAME_SIZE + 1 ) ;
      // copy group name
      ossMemcpy( header->_groupName,
                 oldheader._groupName, CI_GROUPNAME_SIZE + 1 ) ;
      // copy collection space name
      ossMemcpy( header->_csName, oldheader._csName, CI_CS_NAME_SIZE + 1 ) ;
      // copy collection name
      ossMemcpy( header->_clName, oldheader._clName, CI_CL_NAME_SIZE + 1) ;
      // skip file path
      // skip out file
      // copy view format string
      ossMemcpy( header->_view, oldheader._view, CI_VIEWOPTION_SIZE + 1 ) ;

   done:
      if ( opened )
      {
         ossClose( file ) ;
      }

      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** get groups through coord
    ***/
   INT32 getCiGroup( sdbclient::sdb *coord, const CHAR *groupName,
                                            ciLinkList< ciGroup > &groupList )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN hasGroup = ( 0 != ossStrncmp( "", groupName,
                                            CI_GROUPNAME_SIZE ) ) ;
      bson::BSONObj obj ;
      sdbclient::sdbCursor cursor ;

      rc = coord->listReplicaGroups( cursor ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to list replica groups" << std::endl ;
         goto error ;
      }

      rc = cursor.next( obj ) ;
      while( SDB_DMS_EOC != rc )
      {
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to get current record." << std::endl ;
            goto error ;
         }
         else
         {
            bson::BSONElement name = obj.getField( "GroupName" ) ;
            std::string beginWith = name.String().substr( 0, 3 ) ;
            if ( 0 != ossStrncmp( beginWith.c_str(),
                                  "SYS", ossStrlen( "SYS") ) )
            {
               // fill group item
               if ( !hasGroup || ( 0 == ossStrncmp( name.String().c_str(),
                                   groupName, CI_GROUPNAME_SIZE ) ) )
               {
                  ciGroup *group = groupList.createNode() ;
                  if ( NULL == group )
                  {
                     std::cout << "Error: failed to allocate ciGroup"
                               << std::endl ;
                     rc = SDB_OOM ;
                     goto error ;
                  }

                  ossMemcpy( group->_groupName, name.String().c_str(),
                                                CI_GROUPNAME_SIZE ) ;
                  group->_groupID = obj.getField( "GroupID" ).Int() ;

                  groupList.add( group ) ;
               }
            }
         }
         rc = cursor.next( obj ) ;
      }

      if ( 0 >= groupList.count() )
      {
         std::cout << "Error: Cannot get replica group: "
                   << groupName << std::endl ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      rc = SDB_OK ;
   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** get nodes info through group
    ***/
   INT32 getCiNode( sdbclient::sdb *coord, ciGroup *group,
                  ciGroupHeader &header, ciLinkList<ciNode> &nodeList )
   {
      INT32 rc = SDB_OK ;

      if ( NULL != group )
      {
         INT32 index = 0 ;
         // fill member of group
         header._groupID = group->_groupID ;
         ossMemset( header._groupName, 0, CI_GROUPNAME_SIZE ) ;
         ossMemcpy( header._groupName, group->_groupName, CI_GROUPNAME_SIZE ) ;
         // query replica group
         sdbclient::sdbReplicaGroup rg ;
         rc = coord->getReplicaGroup( group->_groupName, rg ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to get replica group: "
                      << group->_groupName << std::endl ;
            goto error ;
         }

         //get master node to make sure master is the head node of list
         sdbclient::sdbNode master ;
         rc = rg.getMaster( master ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to get master node" << std::endl ;
            goto error ;
         }

         ciNode *masterNode = nodeList.createNode() ;
         if ( NULL == masterNode )
         {
            std::cout << "Error: failed to allocate ciNode"
                      << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         ossMemcpy( masterNode->_hostname, master.getHostName(),
                                           CI_HOSTNAME_SIZE ) ;
         ossMemcpy( masterNode->_serviceName, master.getServiceName(),
                                           CI_SERVICENAME_SIZE ) ;
         ++index ;
         masterNode->_index = index ;
         nodeList.add( masterNode ) ;

         // query slave nodes of group
         bson::BSONObj result ;
         rc = rg.getDetail( result ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to get detail of replica group: "
                      << rg.getName() << std::endl ;
            goto error ;
         }

         bson::BSONElement eGroup = result.getField("Group") ;
         if( bson::Array == eGroup.type() )
         {
            std::vector<bson::BSONElement> nodes = eGroup.Array() ;
            std::vector<bson::BSONElement>::iterator cit = nodes.begin() ;
            while ( nodes.end() != cit )
            {
               bson::BSONObj bsonNode ;
               cit->Val( bsonNode ) ;

               ciNode *node = nodeList.createNode() ;
               if ( NULL == node )
               {
                  std::cout << "Error: failed to allocate ciNode" << std::endl ;
                  rc = SDB_OOM ;
                  goto error ;
               }
               // get hostname of node
               std::string hostname = bsonNode.getField( "HostName" ).String() ;
               // get servicename of node
               std::vector<bson::BSONElement> service ;
               service = bsonNode.getField( "Service" ).Array() ;
               std::string servicename = service[0][ "Name" ].String() ;
               INT32 nodeID = bsonNode.getField("NodeID").Int() ;

               if ( ( 0 == ossStrncmp( master.getHostName(),
                                       hostname.c_str(),
                                        CI_HOSTNAME_SIZE ) ) &&
                    ( 0 == ossStrncmp( master.getServiceName(),
                                       servicename.c_str(),
                                       CI_SERVICENAME_SIZE ) ) )
               {
                  masterNode->_nodeID = nodeID ;
                  ++cit ;
                  continue ;
               }

               sdbclient::sdb db;
               rc = db.connect( hostname.c_str(), servicename.c_str() ) ;
               if ( SDB_OK != rc )
               {
                  std::cout << "Warning: connot connect to " << hostname
                            << ":" << servicename << std::endl ;
                  std::cout << "Node is invalid, it will not be inspected"
                            << std::endl ;
                  rc = SDB_OK ;
                  ++cit ;
                  continue ;
               }

               // not master node
               db.disconnect() ;
               ossMemcpy( node->_hostname, hostname.c_str(),
                                           CI_HOSTNAME_SIZE ) ;
               ossMemcpy( node->_serviceName, servicename.c_str(),
                                              CI_SERVICENAME_SIZE ) ;
               node->_nodeID = nodeID ;
               ++index ;
               node->_index = index ;
               // add to group
               nodeList.add( node ) ;
               ++cit ;
            }
            header._nodeCount = nodeList.count() ;
         }
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** get collection space in nodes
    ***/
   INT32 getCiCollection( ciNode *master, const CHAR *csName,
                          const CHAR *clName,
                          ciLinkList< ciCollection > &collections )
   {
      INT32 rc              = SDB_OK ;
      BOOLEAN hasCollection = FALSE ;
      sdbclient::sdb db ;
      sdbclient::sdbCursor cursor ;
      bson::BSONObj collection ;

      SDB_ASSERT( NULL != master, "Error: master node cannot be NULL" ) ;

      hasCollection = ( 0 != ossStrncmp( "", clName, CI_CL_FULLNAME_SIZE ) ) ;

      // get collections of master node
      rc = db.connect( master->_hostname, master->_serviceName ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to connect to master node" << std::endl ;
         goto error ;
      }

      rc = db.listCollections( cursor ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to list collections" << std::endl ;
         goto error ;
      }

      rc = cursor.next( collection ) ;
      while ( SDB_DMS_EOC != rc )
      {
         if ( SDB_OK != rc )
         {
            std::cout << "Waring: failed to get record in cursor" << std::endl ;
            if ( collections.count() > 0)
            {
               // inspect with collections already exist.
               goto done ;
            }
            else
            {
               goto error ;
            }
         }
         else
         {
            std::string cs ;
            std::string cl ;
            std::string name = collection.getField( "Name" ).String() ;
            std::size_t dot = name.find( '.' ) ;
            if ( std::string::npos == dot )
            {
               std::cout << "Error: cannot split collection fullname: "
                         << name << std::endl ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
            cs = name.substr( 0, dot ) ;
            cl = name.substr( dot + 1 ) ;
            if ( !hasCollection ||
                ( 0 == ossStrncmp( csName, cs.c_str(), CI_CS_NAME_SIZE ) &&
                  0 == ossStrncmp( clName, cl.c_str(), CI_CL_NAME_SIZE ) ) )
            {
               ciCollection *cl = collections.createNode() ;
               if ( NULL == cl )
               {
                  std::cout << "Error: failed to allocate ciCollection"
                            << std::endl ;
                  rc = SDB_OOM ;
                  goto error ;
               }
               ossMemcpy( cl->_clName, name.c_str(), CI_CL_FULLNAME_SIZE ) ;
               collections.add( cl ) ;
            }
            else
            {
               std::cout << "Error: cannot find collection: "
                         << clName << std::endl ;
               rc = SDB_INVALIDARG ;
               goto error ;
            }
         }
         rc = cursor.next( collection ) ;
      }

      rc = SDB_OK ;

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** check if reach end of condition
    ***/
   BOOLEAN reachEnd( const ciBson &doc, const INT32 nodeCount )
   {
      BOOLEAN end = TRUE ;
      for ( INT32 idx = 0 ; idx < nodeCount ; ++idx )
      {
         if ( !doc.objs[idx].isEmpty() )
         {
            end = FALSE ;
            break;
         }
      }

      return end ;
   }

   /**
    ** compare each record among nodes
    ***/
   BOOLEAN compare( const INT32 nodeCount, const bson::BSONObj &obj,
                                           ciState &state, ciBson &doc )
   {
      BOOLEAN equal = FALSE ;
      state.reset() ;

      for ( INT32 idx = 0 ; idx < nodeCount ; ++idx )
      {
         if ( doc.objs[idx].equal( obj ) )
         {
            state.set( idx ) ;
         }
      }

      if ( state._state == ( ( 1 << nodeCount ) - 1 ) || 0 == state._state )
      {
         state.reset() ;
         equal = TRUE ;
         // for next round
         state.set( ALL_THE_SAME_BIT ) ;
      }

      return equal ;
   }

   /**
    ** compare each record among nodes
    ***/
   INT32 getCiRecord( ciLinkList< ciCursor > &cursors,
                        ciLinkList< ciRecord > &records )
   {
      INT32 rc        = SDB_OK ;
      BOOLEAN equal   = FALSE ;
      INT32 min       = 0 ;
      INT32 nodeCount = 0 ;
      ciBson record ;
      ciState state ;
      state.reset() ;
      cursors.resetCurrentNode() ;

      nodeCount = cursors.count() ;
      // get first record and compare
      state.set( ALL_THE_SAME_BIT ) ;
      getNext( state, cursors, 0, record, FALSE ) ;
      while ( !reachEnd( record, nodeCount ) )
      {
         min = getMinObjectIndex( record, nodeCount ) ;
         equal = compare( nodeCount, record.objs[ min ], state, record ) ;

         if ( !equal )
         {
            ciRecord *rd = records.createNode() ;
            if ( NULL == rd )
            {
               std::cout << "Error: failed to allocate ciRecord"
                         << std::endl ;
               rc = SDB_OOM ;
               goto error ;
            }
            rd->_bson = record.objs[min].copy() ;
            rd->_state = state._state ;
            rd->_len = rd->_bson.objsize() ;
            records.add( rd ) ;
         }
         getNext( state, cursors, min, record ) ;
      }

   done:
      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** inspect node without file specified
    ***/
   INT32 inspectWithoutFile( sdbclient::sdb *coord, ciHeader *header,
                             const CHAR *outFile, UINT64 &count )
   {
      INT32 rc                                 = SDB_OK ;
      BOOLEAN hasGroup                         = FALSE ;
      BOOLEAN opened                           = FALSE ;
      ciGroup *curGroup                        = NULL ;
      ciCollection *curCollection              = NULL ;
      CHAR *buffer                             = NULL ;
      INT64 offset                             = 0 ;
      INT64 bufferSize                         = 0 ;
      INT64 validSize                          = 0 ;
      CHAR fullName[ CI_CL_FULLNAME_SIZE + 1 ] = { 0 } ;
      ciLinkList< ciGroup > groupList ;
      ciLinkList< ciNode > nodeList ;
      ciLinkList< ciCollection > collections ;
      ciLinkList< ciCursor > cursors ;
      ciLinkList< ciRecord > records ;
      ciGroupHeader groupHeader ;
      ciClHeader clHeader ;
      ciTail tail ;
      OSSFILE file ;

      count = 0 ;
      rc = ossOpen( outFile, OSS_REPLACE | OSS_READWRITE,
                             OSS_RU | OSS_WU | OSS_RG, file ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to open file, rc = " << rc << std::endl ;
         goto error ;
      }
      opened = TRUE ;

      // write header to file
      rc = writeCiHeader( file, header, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      offset += validSize ;

      rc = getCiGroup( coord, header->_groupName, groupList ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      hasGroup = ( 0 != ossStrncmp( "", header->_groupName,
                                        CI_GROUPNAME_SIZE ) ) ;
      // combine collection full name
      ossSnprintf( fullName, CI_CL_FULLNAME_SIZE, "%s.%s",
                             header->_csName, header->_clName ) ;

      groupList.resetCurrentNode() ;
      curGroup = groupList.getHead() ;
      while( NULL != curGroup )
      {
         if ( !hasGroup || 0 == ossStrncmp( curGroup->_groupName,
                                            header->_groupName,
                                            CI_GROUPNAME_SIZE ) )
         {
            nodeList.clear() ;
            collections.clear() ;

            rc = getCiNode( coord, curGroup, groupHeader, nodeList ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;

            // get collections
            rc = getCiCollection( nodeList.getHead(), header->_csName,
                                  header->_clName, collections ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;

            groupHeader._clCount = collections.count() ;

            // remember the offset
            ciOffset *off = tail._groupOffset.createNode() ;
            if ( NULL == off )
            {
               std::cout << "Error: failed to allocate ciClOffset"
                  << std::endl ;
               rc = SDB_OOM ;
               goto error ;
            }
            off->_offset = offset ;
            tail._groupOffset.add( off ) ;
            ++tail._groupCount ;

            // write group header to file
            rc = writeCiGroupHeader( file, &groupHeader ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;
            offset += CI_GROUP_HEADER_SIZE ;

            // write group header to file
            rc = writeCiNode( file, nodeList, buffer, bufferSize, validSize ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;
            offset += validSize ;

            curCollection = collections.getHead() ;
            while ( NULL != curCollection )
            {
               cursors.clear() ;
               bson::BSONObj order = bob().append("_id", 1).obj();
               rc = getCiCursor( nodeList, curCollection->_clName,
                                           cursors, order, TRUE ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;

               ossMemset( clHeader._fullname, 0, CI_CL_FULLNAME_SIZE ) ;
               ossMemcpy( clHeader._fullname, curCollection->_clName,
                                              CI_CL_FULLNAME_SIZE ) ;
               records.clear() ;
               rc = getCiRecord( cursors, records ) ;
               if ( SDB_OK != rc )
               {
                  std::cout << "Error: failed to re record among nodes"
                            << std::endl ;
                  goto error ;
               }

               clHeader._recordCount = records.count() ;

               // write collection header to file
               rc = writeCiClHeader( file, &clHeader ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               offset += CI_CL_HEADER_SIZE ;

               if ( clHeader._recordCount > 0 )
               {
                  count += clHeader._recordCount ;
                  rc = writeCiRecord( file, records, buffer,
                                            bufferSize, validSize ) ;
                  CHECK_VALUE( ( SDB_OK != rc ), error ) ;
                  offset += validSize ;
               }

               curCollection = collections.next() ;
            }
         }

         curGroup = groupList.next() ;
      }
      
      if ( count == 0 )
      {
         tail._exitCode = 0 ; // no records
      }
      else
      {
         tail._exitCode = 2 ;// loop count over
      }

      // tail 65536 bytes
      rc = writeCiTail( file, &tail, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      offset += validSize ;

   done:
      // close file
      if ( opened )
      {
         ossClose( file ) ;
      }

      if ( NULL != buffer )
      {
         SDB_OSS_FREE( buffer ) ;
         buffer = NULL ;
      }

      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }

   /**
    ** inspect node with file specified
    ***/
   INT32 inspectWithFile( ciHeader *header, const CHAR *inFile,
                          const CHAR *outFile, UINT64 &count, BOOLEAN &finish )
   {
      INT32 rc           = SDB_OK ;
      BOOLEAN inOpened   = FALSE ;
      BOOLEAN outOpened  = FALSE ;
      CHAR *buffer       = NULL ;
      INT64 bufferSize   = 0 ;
      INT64 validSize    = 0 ;
      UINT64 totalRecord = 0 ;
      INT64 fileSize     = 0 ;
      INT64 offset       = 0 ;
      INT64 tailOffset   = 0 ;
      INT64 writeOffset  = 0 ;
      ciLinkList< ciNode > ciNodes ;
      ciGroupHeader groupHeader ;
      ciClHeader clHeader ;
      ciTail tail ;
      OSSFILE in ;
      OSSFILE out ;
      // open in file
      rc = ossOpen( inFile, OSS_RO, OSS_RU | OSS_WU | OSS_RG, in ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to open file: " << inFile
                   << ", rc = " << rc << std::endl ;
         goto error ;
      }
      inOpened = TRUE ;
      // open out file
      rc = ossOpen( outFile, OSS_REPLACE | OSS_READWRITE,
                             OSS_RU | OSS_WU | OSS_RG, out ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to open file: " << outFile
                   << ", rc = " << rc << std::endl ;
         goto error ;
      }
      outOpened = TRUE ;

      rc = ossGetFileSize( &in, &fileSize ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to get file size" << std::endl ;
         goto error ;
      }

      if ( SDB_OK != rc )
      {
         std::cout << "Error: filesize is lt " << CI_HEADER_SIZE << std::endl ;
         goto error ;
      }

      tailOffset = fileSize - CI_TAIL_SIZE ;
      rc = readCiTail( in, tailOffset, &tail ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      tail._groupCount = 0 ;
      tail._groupOffset.clear() ;

      rc = writeCiHeader( out, header, buffer, bufferSize, validSize ) ;
      if ( SDB_OK != rc )
      {
         std::cout << "Error: failed to write header to file" << std::endl ;
         goto error ;
      }
      writeOffset = CI_HEADER_SIZE ;

      //skip 65536 bytes
      offset = CI_HEADER_SIZE ;
      while ( offset < tailOffset )
      {
         rc = readCiGroupHeader( in, offset, &groupHeader ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;

         // remember the offset
         ciOffset *off = tail._groupOffset.createNode() ;
         if ( NULL == off )
         {
            std::cout << "Error: failed to allocate ciOffset"
               << std::endl ;
            rc = SDB_OOM ;
            goto error ;
         }
         off->_offset = writeOffset ;
         tail._groupOffset.add( off ) ;
         ++tail._groupCount ;

         // write to out file
         rc = writeCiGroupHeader( out, &groupHeader ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to write ciGroupHeader to file"
                      << std::endl ;
            goto error ;
         }

         writeOffset += CI_GROUP_HEADER_SIZE ;
         // read nodes
         ciNodes.clear() ;
         rc = readCiNode( in, offset, groupHeader, ciNodes ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;

         rc = writeCiNode( out, ciNodes, buffer, bufferSize, validSize ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to write ciNodes to file" << std::endl ;
            goto error ;
         }
         writeOffset += validSize ;

         UINT32 idx = 0 ;
         while ( idx < groupHeader._clCount )
         {
            ciClHeader clHeader ;
            ciLinkList< ciRecord > records ;
            rc = readCiClHeader( in, offset, &clHeader ) ;
            if ( SDB_OK != rc )
            {
               std::cout << "Error: failed to get ciClHeader" << std::endl ;
               goto error ;
            }

            if ( clHeader._recordCount > 0 )
            {
               records.clear() ;

               rc = readCiRecord( in, offset, ciNodes, clHeader, records ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
            }

            clHeader._recordCount = records.count() ;

            rc = writeCiClHeader( out, &clHeader ) ;
            if ( SDB_OK != rc )
            {
               std::cout << "Error: failed to write ciClHeader to file"
                         << std::endl ;
               goto error ;
            }
            writeOffset += CI_CL_HEADER_SIZE ;

            if ( records.count() > 0 )
            {
               totalRecord += records.count() ;

               rc = writeCiRecord( out, records, buffer,
                                        bufferSize, validSize ) ;
               CHECK_VALUE( ( SDB_OK != rc ), error ) ;
               writeOffset += validSize ;
            }
            ++idx ;
         }
      }

      if ( totalRecord == 0 )
      {
         finish = TRUE ;
         tail._exitCode = 0 ; // no records
      }
      else
      {
         double precent = ( totalRecord / (double)count ) * 100 ;
         if ( precent <= 1 )
         {
            finish = TRUE ;
            tail._exitCode = 1 ; // lt 1%
         }
      }

      if ( !finish )
      {
         count = totalRecord ;
         tail._exitCode = 2 ; // assume loop it over
      }

      // write header to file
      rc = writeCiTail( out, &tail, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   done:
      if ( inOpened )
      {
         ossClose( in ) ;
      }

      if ( outOpened )
      {
         ossClose( out ) ;
      }

      if ( NULL != buffer )
      {
         SDB_OSS_FREE( buffer ) ;
         buffer = NULL ;
      }

      return rc ;
   error:
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      OUTPUT_FUNCTION( "Error occurs in ", __FUNCTION__ ) ;
      goto done ;
   }
}

_sdbCi::_sdbCi()
{
   ossMemset( _coordAddr, 0, CI_ADDRESS_SIZE + 1 ) ;
}

_sdbCi::~_sdbCi()
{

}

void _sdbCi::displayArgs( const po::options_description &desc )
{
   std::cout << desc << std::endl ;
}

INT32 _sdbCi::init( INT32 argc, CHAR **argv,
                                    po::options_description &desc,
                                    po::variables_map &vm )
{
   INT32 rc = SDB_OK ;

   INSPECT_ADD_OPTIONS_BEGIN( desc )
      INSPECT_OPTIONS
   INSPECT_ADD_OPTIONS_END

   rc = utilReadCommandLine( argc, argv, desc, vm ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Invalid parameters" << std::endl ;
      displayArgs( desc ) ;
      goto error ;
   }

   rc = _pmdCfgRecord::init( NULL, &vm ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Invalid parameters" << std::endl ;
      displayArgs( desc ) ;
      goto error ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 _sdbCi::handle( const po::options_description &desc,
                                      const po::variables_map &vm )
{
   INT32 rc = SDB_OK ;
   BOOLEAN byGroup = TRUE ;
   BOOLEAN useOutput = FALSE ;
   CHAR outReport[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

   if ( vm.empty() || vm.count( CONSISTENCY_INSPECT_HELP ) )
   {
      displayArgs(desc) ;
      goto done ;
   }

   if ( vm.count( CONSISTENCY_INSPECT_VER ) )
   {
      ossPrintVersion( "SequoiaDB version" ) ;

      CHAR version[ 64 ] = { 0 } ;
      std::cout << "sdbConsistencyInspect version : " ;
      ossSnprintf( version, 64, "%d.%d", CI_MAIN_VERSION, CI_SUB_VERSION ) ;
      std::cout << version << std::endl ;
      goto done ;
   }

   if ( 0 != ossStrncmp( CI_ACTION_INSPECT, _header._action, CI_ACTION_SIZE ) &&
        0 != ossStrncmp( CI_ACTION_REPORT, _header._action, CI_ACTION_SIZE ) )
   {
      std::cout << "Invalid parameters:" << std::endl
                << "Unknown action : " << _header._action << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   else if ( 0 == ossStrncmp( CI_ACTION_REPORT,
                              _header._action, CI_ACTION_SIZE ) &&
             !vm.count( CONSISTENCY_INSPECT_FILE ) )
   {
      std::cout << "Invalid parameters:" << std::endl
                << "a existed file need to be specified "
                   "when ACTION is \"report\"" << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if (    vm.count( CONSISTENCY_INSPECT_CL )
       && !vm.count( CONSISTENCY_INSPECT_CS ) )
   {
      std::cout << "Invalid parameters:" << std::endl
                << "collection space name should be specified when collection "
                   "name is specified" << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( 0 != ossStrncmp( CI_VIEW_CL, _header._view, CI_VIEWOPTION_SIZE ) &&
        0 != ossStrncmp( CI_VIEW_GROUP, _header._view, CI_VIEWOPTION_SIZE ) )
   {
      std::cout << "Invalid parameters:" << std::endl
                << "Unknown action : " << _header._action << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   byGroup = ( 0 != ossStrncmp( CI_VIEW_CL, _header._view,
                                CI_VIEWOPTION_SIZE ) ) ? TRUE : FALSE ;
   useOutput = ( 0 != ossStrncmp( "", _header._outfile, OSS_MAX_PATHSIZE ) ) ;

   if ( 0 == ossStrncmp( CI_ACTION_REPORT,
                         _header._action, CI_ACTION_SIZE ) &&
        vm.count( CONSISTENCY_INSPECT_FILE ) )
   {
      // report file
      if ( !useOutput )
      {
         ossMemcpy( outReport, _header._filepath, OSS_MAX_PATHSIZE ) ;
         ossStrncat( outReport, CI_FILE_REPORT, ossStrlen( CI_FILE_REPORT ) ) ;
      }
      else
      {
         ossMemcpy( outReport, _header._outfile, OSS_MAX_PATHSIZE ) ;
      }
      rc = byGroup ? report ( _header._filepath, outReport )
                   : report2( _header._filepath, outReport );
      //rc = report2( _header._filepath ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      std::cout << _header._action << " successfully" << std::endl ;

      goto done ;
   }

   if ( vm.count( CONSISTENCY_INSPECT_FILE ) &&
        0 == ossStrncmp( CI_ACTION_INSPECT, _header._action, CI_ACTION_SIZE ) )
   {
      std::cout << "file is specified, initialize all options according to file"
                << std::endl ;
      rc = initialize( &_header ) ;
   }
   else
   {
      rc = splitAddr() ;
   }
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   if ( 0 != ossStrncmp( CI_VIEW_GROUP, _header._view, CI_VIEWOPTION_SIZE ) &&
        0 != ossStrncmp( CI_VIEW_CL, _header._view, CI_VIEWOPTION_SIZE ) )
   {
      std::cout << "Invalid parameters:" << std::endl
                << "Unknown action : " << _header._action << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   rc = inspect() ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;
   // report file
   if ( !useOutput )
   {
      ossMemcpy( outReport, CI_FILE_NAME, OSS_MAX_PATHSIZE ) ;
   }
   else
   {
      ossMemcpy( outReport, _header._outfile, OSS_MAX_PATHSIZE ) ;
   }
   ossStrncat( outReport, CI_FILE_REPORT, ossStrlen( CI_FILE_REPORT ) ) ;
   rc = byGroup ? report ( _header._outfile, outReport )
                : report2( _header._outfile, outReport ) ;

   CHECK_VALUE( (SDB_OK != rc ), error ) ;
   std::cout << _header._action << " successfully" << std::endl ;

done:
   return rc ;
error:
   goto done ;
}

INT32 _sdbCi::inspect()
{
   INT32 rc           = SDB_OK ;
   INT32 curLoop      = 0 ;
   UINT64 totalRecord = 0 ;
   BOOLEAN finish     = FALSE ;
   CHAR inFile[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;
   CHAR tmpFile[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

   sdbclient::sdb *coord = new sdbclient::sdb() ;
   if( NULL == coord )
   {
      std::cout << "Error: failed to allocate sdbclient::sdb" << std::endl ;
      rc = SDB_OOM ;
      goto error ;
   }

   rc = coord->connect( _header._coordAddr, _header._serviceName ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to connect to " << _header._coordAddr
         << ":" << _header._serviceName << std::endl ;
      goto error ;
   }

   if ( 0 == ossStrncmp( _header._filepath, "", OSS_MAX_PATHSIZE ) )
   {
      curLoop = 1 ;

      ossMemset( tmpFile, 0, OSS_MAX_PATHSIZE ) ;
      ossSnprintf( tmpFile, OSS_MAX_PATHSIZE, CI_TMP_FILE, curLoop ) ;

      rc = inspectWithoutFile( coord, &_header, tmpFile, totalRecord ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      if ( 0 == totalRecord )
      {
         finish = TRUE ;
      }

      if ( _header._loop > 1 )
      {
         // use out file as input file for next loop
         ossMemcpy( inFile, tmpFile, OSS_MAX_PATHSIZE ) ;
      }
   }
   else
   {
      ossMemcpy( inFile, _header._filepath, OSS_MAX_PATHSIZE ) ;
   }

   for (INT32 idx = curLoop ; idx < _header._loop && !finish ; ++idx)
   {
      ossMemset( tmpFile, 0, OSS_MAX_PATHSIZE ) ;
      ossSnprintf( tmpFile, OSS_MAX_PATHSIZE, CI_TMP_FILE, idx + 1 ) ;

      rc = inspectWithFile( &_header, inFile, tmpFile, totalRecord, finish ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      // use out file as input file for next loop
      ossMemset( inFile, 0, OSS_MAX_PATHSIZE ) ;
      ossMemcpy( inFile, tmpFile, OSS_MAX_PATHSIZE ) ;
   }

   if ( 0 != ossStrncmp( "", _header._outfile, OSS_MAX_PATHSIZE ) )
   {
      rc = ossRenamePath( tmpFile, _header._outfile ) ;
   }
   else
   {
      rc = ossRenamePath( tmpFile, CI_FILE_NAME ) ;
   }
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to rename temp file to \""
                << _header._outfile << "\"" << std::endl ;
      goto error ;
   }

   // delete temp file
   for ( INT32 idx = 0 ; idx < _header._loop ; ++idx )
   {
      ossMemset( tmpFile, 0, OSS_MAX_PATHSIZE ) ;
      ossSnprintf( tmpFile, OSS_MAX_PATHSIZE, CI_TMP_FILE, idx + 1 ) ;

      ossDelete( tmpFile ) ;
   }

done:
   if ( NULL != coord )
   {
      delete coord ;
      coord = NULL ;
   }
   return rc ;
error:
   std::cout << "Error occurs in " << __FUNCTION__ << std::endl ;
   goto done ;
}

INT32 _sdbCi::report ( const CHAR *inFile, const CHAR *reportFile )
{
   INT32 rc           = SDB_OK ;
   BOOLEAN inOpened   = FALSE ;
   BOOLEAN outOpened  = FALSE ;
   CHAR *buffer       = NULL ;
   INT64 bufferSize   = 0 ;
   INT64 validSize    = 0 ;
   INT64 fileSize     = 0 ;
   INT64 offset       = 0 ;
   INT64 tailOffset   = 0 ;
   CHAR exitTip[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

   ciHeader header ;
   ciLinkList< ciOffset > Offset ;
   ciLinkList< ciNode > ciNodes ;
   ciGroupHeader groupHeader ;
   ciClHeader clHeader ;
   ciTail tail ;
   OSSFILE in ;
   OSSFILE out ;
   // open in file
   rc = ossOpen( inFile, OSS_RO, OSS_RU | OSS_WU | OSS_RG, in ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to open file: " << inFile
         << ", rc = " << rc << std::endl ;
      goto error ;
   }
   inOpened = TRUE ;

   rc = ossGetFileSize( &in, &fileSize ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to get file size" << std::endl ;
      goto error ;
   }

   if ( SDB_OK != rc )
   {
      std::cout << "Error: filesize is lt " << CI_HEADER_SIZE << std::endl ;
      goto error ;
   }
   // read tail
   tailOffset = fileSize - CI_TAIL_SIZE ;
   rc = readCiTail( in, tailOffset, &tail ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   rc = ossOpen( reportFile, OSS_REPLACE | OSS_READWRITE,
                 OSS_RU | OSS_WU | OSS_RG, out ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to open file, rc = " << rc << std::endl ;
      goto error ;
   }
   outOpened = TRUE ;

   // dump header
   rc = readCiHeader( in, &header ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;
   rc = dumpCiHeader( &header, buffer, bufferSize, validSize ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;
   rc = writeToFile( out, buffer, validSize ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   //skip 65536 bytes
   offset = CI_HEADER_SIZE ;
   while ( offset < tailOffset )
   {
      // dump group
      rc = readCiGroupHeader( in, offset, &groupHeader ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      rc = dumpCiGroupHeader( &groupHeader, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      // read nodes
      ciNodes.clear() ;
      rc = readCiNode( in, offset, groupHeader, ciNodes ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      rc = dumpCiNode( ciNodes, buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      rc = writeToFile( out, buffer, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;

      UINT32 idx = 0 ;
      while ( idx < groupHeader._clCount )
      {
         ciClHeader clHeader ;
         ciLinkList< ciRecord > records ;
         rc = readCiClHeader( in, offset, &clHeader ) ;
         if ( SDB_OK != rc )
         {
            std::cout << "Error: failed to get ciClHeader" << std::endl ;
            goto error ;
         }
         rc = dumpCiClHeader( &clHeader, buffer, bufferSize, validSize ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;
         rc = writeToFile( out, buffer, validSize ) ;
         CHECK_VALUE( ( SDB_OK != rc ), error ) ;

         if ( clHeader._recordCount > 0 )
         {
            records.clear() ;

            rc = readCiRecord( in, offset, ciNodes,
                               clHeader, records, TRUE ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;
            rc = dumpCiRecord( ciNodes.count(), records,
                               buffer, bufferSize, validSize ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;
            rc = writeToFile( out, buffer, validSize ) ;
            CHECK_VALUE( ( SDB_OK != rc ), error ) ;
         }

         ++idx ;
      }
   }

   if ( 0 == tail._exitCode )
   {
      validSize = ossSnprintf( exitTip, OSS_MAX_PATHSIZE,
                               OSS_NEWLINE"Result: exit with no records "
                               "different"OSS_NEWLINE ) ;
   }
   else if ( 1 == tail._exitCode )
   {
      validSize = ossSnprintf( exitTip, OSS_MAX_PATHSIZE,
                               OSS_NEWLINE"Result: exit with less than 1% of "
                               "records not synchronized"OSS_NEWLINE ) ;
   }
   else
   {
      validSize = ossSnprintf( exitTip, OSS_MAX_PATHSIZE,
                               OSS_NEWLINE"Result: loop is limited"
                               OSS_NEWLINE ) ;
   }

   rc = writeToFile( out, exitTip, validSize ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

done:
   if ( inOpened )
   {
      ossClose( in ) ;
   }

   if ( outOpened )
   {
      ossClose( out ) ;
   }

   if ( NULL != buffer )
   {
      SDB_OSS_FREE( buffer ) ;
      buffer = NULL ;
   }

   return rc ;
error:
   std::cout << "Error occurs in " << __FUNCTION__ << std::endl ;
   goto done ;
}



INT32 _sdbCi::report2( const CHAR *inFile, const CHAR *reportFile )
{
   INT32 rc              = SDB_OK ;
   BOOLEAN inOpened      = FALSE ;
   BOOLEAN outOpened     = FALSE ;
   ciOffset *groupOffset = NULL ;
   CHAR *buffer          = NULL ;
   INT64 bufferSize      = 0 ;
   INT64 validSize       = 0 ;
   INT64 fileSize        = 0 ;
   INT64 tailOffset      = 0 ;
   CHAR exitTip[ OSS_MAX_PATHSIZE + 1 ] = { 0 } ;

   ciHeader header ;
   ciLinkList< ciNode > ciNodes ;
   ciLinkList< ciOffset > clOffsets ;
   ciGroupHeader groupHeader ;
   ciClHeader clHeader ;
   ciTail tail ;
   OSSFILE in ;
   OSSFILE out ;
   // open in file
   rc = ossOpen( inFile, OSS_RO, OSS_RU | OSS_WU | OSS_RG, in ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to open file: " << inFile
                << ", rc = " << rc << std::endl ;
      goto error ;
   }
   inOpened = TRUE ;

   rc = ossGetFileSize( &in, &fileSize ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to get file size" << std::endl ;
      goto error ;
   }

   if ( SDB_OK != rc )
   {
      std::cout << "Error: filesize is lt " << CI_HEADER_SIZE << std::endl ;
      goto error ;
   }
   // read tail
   tailOffset = fileSize - CI_TAIL_SIZE ;
   rc = readCiTail( in, tailOffset, &tail ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   rc = ossOpen( reportFile, OSS_REPLACE | OSS_READWRITE,
                 OSS_RU | OSS_WU | OSS_RG, out ) ;
   if ( SDB_OK != rc )
   {
      std::cout << "Error: failed to open file, rc = " << rc << std::endl ;
      goto error ;
   }
   outOpened = TRUE ;

   // dump header
   rc = readCiHeader( in, &header ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;
   rc = dumpCiHeader( &header, buffer, bufferSize, validSize ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;
   rc = writeToFile( out, buffer, validSize ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

   tail._groupOffset.resetCurrentNode() ;
   groupOffset = tail._groupOffset.getHead() ;
   while ( NULL != groupOffset )
   {
      rc = dumpOneCl( in, out, groupOffset, clOffsets, NULL,
                                            buffer, bufferSize, validSize ) ;
      CHECK_VALUE( ( SDB_OK != rc ), error ) ;
      groupOffset = tail._groupOffset.next() ;
   }

   if ( 0 == tail._exitCode )
   {
      validSize = ossSnprintf( exitTip, OSS_MAX_PATHSIZE,
                               OSS_NEWLINE"Result: exit with no records "
                               "different"OSS_NEWLINE ) ;
   }
   else if ( 1 == tail._exitCode )
   {
      validSize = ossSnprintf( exitTip, OSS_MAX_PATHSIZE,
                               OSS_NEWLINE"Result: exit with less than 1% of"
                               "records not synchronized"OSS_NEWLINE ) ;
   }
   else
   {
      validSize = ossSnprintf( exitTip, OSS_MAX_PATHSIZE,
                               OSS_NEWLINE"Result: loop is limited"
                               OSS_NEWLINE ) ;
   }
   rc = writeToFile( out, exitTip, validSize ) ;
   CHECK_VALUE( ( SDB_OK != rc ), error ) ;

done:
   if ( inOpened )
   {
      ossClose( in ) ;
   }

   if ( outOpened )
   {
      ossClose( out ) ;
   }

   if ( NULL != buffer )
   {
      SDB_OSS_FREE( buffer ) ;
      buffer = NULL ;
   }

   return rc ;
error:
   std::cout << "Error occurs in " << __FUNCTION__ << std::endl ;
   goto done ;
}

INT32 _sdbCi::doDataExchange( engine::pmdCfgExchange *pEx )
{
   resetResult() ;

   rdxString( pEx, CONSISTENCY_INSPECT_COORD, _coordAddr,
                   CI_ADDRESS_SIZE , FALSE, FALSE, "", FALSE ) ;

   rdxString( pEx, CONSISTENCY_INSPECT_ACTION, _header._action,
                   CI_ACTION_SIZE , FALSE, FALSE, CI_ACTION_INSPECT, FALSE ) ;

   rdxInt( pEx, CONSISTENCY_INSPECT_LOOP, _header._loop, FALSE, TRUE, 5 ) ;

   rdxString( pEx, CONSISTENCY_INSPECT_GROUP, _header._groupName,
                   CI_GROUPNAME_SIZE, FALSE, FALSE, "", FALSE ) ;

   rdxString( pEx, CONSISTENCY_INSPECT_CS, _header._csName,
                   CI_CS_NAME_SIZE, FALSE, FALSE, "", FALSE ) ;

   rdxString( pEx, CONSISTENCY_INSPECT_CL, _header._clName,
                   CI_CL_NAME_SIZE, FALSE, FALSE, "", FALSE ) ;

   rdxPath( pEx, CONSISTENCY_INSPECT_FILE, _header._filepath,
                   OSS_MAX_PATHSIZE, FALSE, FALSE, "" ) ;

   rdxString( pEx, CONSISTENCY_INSPECT_OUTPUT, _header._outfile,
                   OSS_MAX_PATHSIZE, FALSE, FALSE, CI_FILE_NAME ) ;

   rdxString( pEx, CONSISTENCY_INSPECT_VIEW, _header._view,
                   CI_VIEWOPTION_SIZE, FALSE, FALSE, CI_VIEW_GROUP, FALSE ) ;

   return getResult() ;
}

INT32 _sdbCi::postLoaded()
{
   return SDB_OK ;
}

INT32 _sdbCi::preSaving()
{
   return SDB_OK ;
}

INT32 _sdbCi::splitAddr()
{
   INT32 rc        = SDB_OK ;
   INT32 length    = ossStrlen( _coordAddr ) ;
   CHAR *begin     = _coordAddr ;
   CHAR *end       = begin + length ;
   const CHAR *pch = NULL ;

   if ( begin == end )
   {
      std::cout << "Invalid parameters" << std::endl ;
      std::cout << " Hostname and servicename of coord must be specified"
                << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   pch = ossStrrchr( _coordAddr, ':' ) ;
   if ( begin == pch )
   {
      std::cout << "Invalid parameters" << std::endl ;
      std::cout << " Hostname of coord must be specified" << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( NULL == pch || end == pch + 1 )
   {
      std::cout << "Invalid parameters" << std::endl ;
      std::cout << " Service Name must be specified after the hostname, "
                   " split by \":\"" << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   // initialize hostname and servicename in _header
   ossMemcpy( _header._coordAddr, _coordAddr, pch - begin ) ;
   ossMemcpy( _header._serviceName, pch + 1, end - pch ) ;

done:
   return rc ;
error:
   goto done ;
}

//////////////////////////////////////////////////////////////////////////
///< main function
INT32 main(INT32 argc, CHAR** argv)
{
   INT32 rc  = SDB_OK ;
   sdbCi *ci = NULL ;
   po::options_description desc( "Command options" ) ;
   po::variables_map vm ;

   ci = SDB_OSS_NEW sdbCi() ;
   if ( NULL == ci )
   {
      std::cout << "Error: failed to allocate sdbCi" << std::endl ;
      rc = SDB_OOM ;
      goto done ;
   }

   rc = ci->init( argc, argv, desc, vm ) ;
   CHECK_VALUE( ( SDB_OK != rc ), done ) ;

   rc = ci->handle( desc, vm ) ;
   CHECK_VALUE( ( SDB_OK != rc ), done ) ;

done:
   if ( NULL != ci )
   {
      SDB_OSS_DEL ci ;
      ci = NULL ;
   }
   return rc ;
}
