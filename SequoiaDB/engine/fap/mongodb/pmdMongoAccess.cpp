#include "pmdMongoAccess.hpp"
#include "ossUtil.h"
#include "pmdOptions.h"
#include "pmdMongoSession.hpp"

INT32 pmdMongoAccess::init( engine::IResource *pResource )
{
   INT32 rc = SDB_OK ;
   ossMemset( (void *)_svcName, 0, OSS_MAX_SERVICENAME + 1 ) ;

   if ( NULL != pResource )
   {
      _resource = pResource ;
   }

done:
   return rc ;
error:
   goto done ;
}

INT32 pmdMongoAccess::active()
{
   return SDB_OK ;
}

INT32 pmdMongoAccess::deactive()
{
   return SDB_OK ;
}

INT32 pmdMongoAccess::fini()
{
   return SDB_OK ;
}

const CHAR * pmdMongoAccess::getServiceName() const
{
   UINT16 basePort = 0 ;
   if ( '\0' == _svcName[0] )
   {
      ossMemset( (void *)_svcName, 0, OSS_MAX_SERVICENAME ) ;
      if ( NULL != _resource )
      {
         basePort = _resource->getLocalPort() ;
         ossItoa( basePort + PORT_OFFSET, _svcName, OSS_MAX_SERVICENAME ) ;
      }
   }

   return _svcName ;
}

engine::pmdSession * pmdMongoAccess::getSession( SOCKET fd,
                                                 engine::IProcessor *pProcessor )
{
   pmdMongoSession *session = NULL ;
   if ( NULL != pProcessor )
   {
      session = SDB_OSS_NEW pmdMongoSession( fd ) ;
      session->attachProcessor( pProcessor ) ;
   }

   return session ;
}

void pmdMongoAccess::releaseSession( engine::pmdSession *pSession )
{
   pmdMongoSession *session = dynamic_cast< pmdMongoSession *>( pSession ) ;
   if ( NULL == session )
   {
      session->detachProcessor() ;
      SDB_OSS_DEL session ;
      session = NULL ;
   }
}

void pmdMongoAccess::_release()
{
   ossMemset( _svcName, 0, OSS_MAX_SERVICENAME + 1 ) ;

   if ( NULL != _resource )
   {
      // should not delete here, just make it point to nullptr
      _resource = NULL ;
   }
}
