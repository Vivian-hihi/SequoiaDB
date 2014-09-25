// this file is only intened to be included by sdb.cpp and sdbbp.cpp
#ifndef UTILPIPE_HPP__
#define UTILPIPE_HPP__

#include "core.h"


#define SH_VERIFY_RC                                             \
   if ( rc != SDB_OK ) {                                          \
      PD_LOG ( PDERROR , "%s, rc = %d" , getErrDesp(rc) , rc ) ;  \
      goto error ;                                                \
   }

#define SH_VERIFY_COND(cond, ret)  \
   if (!(cond)) {                   \
      rc=ret;                       \
      SH_VERIFY_RC                 \
   }

INT32 getWaitPipeName ( const OSSPID & ppid , CHAR * buf , UINT32 bufSize ) ;
INT32 getPipeNames( const OSSPID & ppid , CHAR * f2bName , UINT32 f2bSize ,
                    CHAR * b2fName , UINT32 b2fSize ) ;
INT32 getPipeNames2( const OSSPID & ppid , const OSSPID & pid ,
                     CHAR * f2bName , UINT32 f2bSize ,
                     CHAR * b2fName , UINT32 b2fSize ) ;

INT32 getPipeNames1( CHAR * bpf2bName , UINT32 bpf2bSize ,
                     CHAR * bpb2fName , UINT32 bpb2fSize ,
                     CHAR * f2bName , CHAR * b2fName ) ;


#endif //UTILPIPE_HPP__

