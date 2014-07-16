#include "omagentTest.hpp"
#include "msgMessage.hpp"

using namespace bson ;
using namespace engine ;

namespace CLSMGR
{
   INT32 testScanHost ( CHAR **ppBuffer, INT32 *bufferSize )
   {
      INT32 rc = SDB_OK ;

      const CHAR* cmd = "$scan host" ;

      BSONObj cond ;
      BSONObj sel ;
      BSONObj orderBy ;
      BSONObj hint ;
      BSONObj sub1 = BSON( OMA_FIELD_NAME_IP << "192.168.20.40" <<
                           OMA_FIELD_NAME_USERNAME << "tanzhaobo" <<
                           OMA_FIELD_NAME_PASSWORD << "tanzb_2012" ) ;
      BSONObj sub2 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME <<"sequoiadb" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" ) ;
      BSONObj sub3 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME<< "root" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" ) ;
      BSONArrayBuilder bab ;
      BSONObjBuilder bob ;
      bab.append( sub1 ) ;
      bab.append( sub2 ) ;
      bab.append( sub3 ) ;

      bob.appendArray( OMA_FIELD_NAME_HOSTS, bab.arr() ) ;
      cond = bob.obj() ;

      rc = msgBuildQueryMsg ( ppBuffer, bufferSize, cmd,
                              0, 0, 0, 1,
                              cond.isEmpty()?NULL:&cond,
                              sel.isEmpty()?NULL:&sel,
                              orderBy.isEmpty()?NULL:&orderBy,
                              hint.isEmpty()?NULL:&hint ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to build query message, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;

   }

   INT32 testInstallRemoteAgent ( CHAR **ppBuffer, INT32 *bufferSize )
   {
      INT32 rc = SDB_OK ;

      const CHAR* cmd = "$install remote agent" ;

      BSONObj cond ;
      BSONObj sel ;
      BSONObj orderBy ;
      BSONObj hint ;
      BSONObj sub1 = BSON( OMA_FIELD_NAME_IP << "192.168.20.40" <<
                           OMA_FIELD_NAME_USERNAME << "tanzhaobo" <<
                           OMA_FIELD_NAME_PASSWORD << "tanzb_2012" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           << "/opt/sequoiadb/omagent" ) ;

      BSONObj sub2 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME <<"sequoiadb" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           << "/opt/sequoiadb/omagent" ) ;

      BSONObj sub3 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME<< "root" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           << "/opt/sequoiadb/omagent" ) ;

      BSONArrayBuilder bab ;
      BSONObjBuilder bob ;
      bab.append( sub1 ) ;
      bab.append( sub2 ) ;
      bab.append( sub3 ) ;

      bob.appendArray( OMA_FIELD_NAME_HOSTS, bab.arr() ) ;
      cond = bob.obj() ;

      rc = msgBuildQueryMsg ( ppBuffer, bufferSize, cmd,
                              0, 0, 0, 1,
                              cond.isEmpty()?NULL:&cond,
                              sel.isEmpty()?NULL:&sel,
                              orderBy.isEmpty()?NULL:&orderBy,
                              hint.isEmpty()?NULL:&hint ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to build query message, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;

   }

   INT32 testInstallAgentProcess ( CHAR **ppBuffer, INT32 *bufferSize )
   {
      INT32 rc = SDB_OK ;

      const CHAR* cmd = "$install agent process" ;

      BSONObj cond ;
      BSONObj sel ;
      BSONObj orderBy ;
      BSONObj hint ;
      BSONObj sub1 = BSON( OMA_FIELD_NAME_IP << "192.168.20.40" <<
                           OMA_FIELD_NAME_USERNAME << "tanzhaobo" <<
                           OMA_FIELD_NAME_PASSWORD << "tanzb_2012" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent/sdbomagent"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           <<"/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent/sdbomagentlocal" ) ;

      BSONObj sub2 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME <<"sequoiadb" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent/sdbomagent"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           << "/opt/sequoiadb/omagent/omagentopt" ) ;

      BSONObj sub3 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME<< "root" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/SequoiaDB/engine/omagent/sdbomagent"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           << "/tmp/sdbomagenttmp" ) ;
/*
      BSONObj sub4 = BSON( OMA_FIELD_NAME_IP << "192.168.20.112" <<
                           OMA_FIELD_NAME_USERNAME<< "root" <<
                           OMA_FIELD_NAME_PASSWORD << "sequoiadb" <<
                           OMA_FIELD_NAME_LOCAL_PACKET_PATH <<
                           "/home/users/tanzhaobo/sequoiadb/bin/sdb"
                           << OMA_FIELD_NAME_REMOTE_PACKET_PATH
                           << "/opt/sequoiadb/omagent/sdb123" ) ;
*/

      BSONArrayBuilder bab ;
      BSONObjBuilder bob ;
      bab.append( sub1 ) ;
      bab.append( sub2 ) ;
      bab.append( sub3 ) ;
//      bab.append( sub4 ) ;

      bob.appendArray( OMA_FIELD_NAME_HOSTS, bab.arr() ) ;
      cond = bob.obj() ;

      rc = msgBuildQueryMsg ( ppBuffer, bufferSize, cmd,
                              0, 0, 0, 1,
                              cond.isEmpty()?NULL:&cond,
                              sel.isEmpty()?NULL:&sel,
                              orderBy.isEmpty()?NULL:&orderBy,
                              hint.isEmpty()?NULL:&hint ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to build query message, rc = %d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;

   }

}


