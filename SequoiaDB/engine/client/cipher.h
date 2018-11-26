#ifndef CIPHER_H_
#define CIPHER_H_

#include "ossTypes.h"

SDB_EXTERN_C_START

INT32 decryptUserCipher( const CHAR *userName, const CHAR *fullName,
                         const CHAR *token, const CHAR *path, 
                         CHAR *clearText ) ;
SDB_EXTERN_C_END

#endif /* CIPHER_H_ */