#ifndef CIPHER_H_
#define CIPHER_H_

#include "ossTypes.h"

SDB_EXTERN_C_START

INT32 decryptUserCipher( const CHAR* pUsername, const CHAR *pToken,
                         const CHAR *pPath, CHAR *pPasswd, UINT32 passwdLen ) ;
SDB_EXTERN_C_END

#endif /* CIPHER_H_ */