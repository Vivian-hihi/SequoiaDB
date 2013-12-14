#ifndef NETWORK_H__
#define NETWORK_H__

#include "core.h"
SDB_EXTERN_C_START
INT32 clientConnect ( const CHAR *pHostName,
                      const CHAR *pServiceName,
                      SOCKET *sock ) ;

void clientDisconnect ( SOCKET sock ) ;

// timeout for microsecond (1/1000000 sec )
INT32 clientSend ( SOCKET sock, const CHAR *pMsg, INT32 len, INT32 timeout ) ;
// timeout for microsecond ( 1/1000000 sec )
INT32 clientRecv ( SOCKET sock, CHAR *pMsg, INT32 len, INT32 timeout ) ;

INT32 disableNagle( SOCKET sock ) ;
SDB_EXTERN_C_END
#endif
