/** \file Base64.h
    \brief Encode binary data using printable characters.
*/
#ifndef ALAN_BASE64_H
#define ALAN_BASE64_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
    #define SDB_INLINE static __inline__
    #define SDB_EXPORT
#else
    #define SDB_INLINE static
    #ifdef SDB_STATIC_BUILD
        #define SDB_EXPORT
    #elif defined(SDB_DLL_BUILD)
        #define SDB_EXPORT __declspec(dllexport)
    #else
        #define SDB_EXPORT __declspec(dllimport)
    #endif
#endif

#ifdef __cplusplus
#define SDB_EXTERN_C_START extern "C" {
#define SDB_EXTERN_C_END }
#else
#define SDB_EXTERN_C_START
#define SDB_EXTERN_C_END
#endif

SDB_EXTERN_C_START

/** \fn
*/


/*********** Test example *************

* ========== Encode ===========
* char *str = "1234567" ;
* int len = getEnBase64Size ( str ) ;
* char *out = (char *)malloc(len) ;
* base64Encode( str, out, len ) ;
* print ( out ) ;
* free( out ) ;

* ========== Decode ===========
* char *str = "1234567" ;
* int len = getDeBase64Size ( str ) ;
* char *out = (char *)malloc(len) ;
* base64Decode( str, out, len ) ;
* print ( out ) ;
* free( out ) ;

***************************************/


/** \fn int base64Encode ( char *s, int in_size, char *out, int out_size )
    \brief String convert base64
    \param [in] s Input buffer
    \param [in] in_size Input buffer size
    \param [in] out_size The size of the buffer that 'out' pointing to ,
                                   make sure it's large enough
    \param [out] out Output string
    \return If successful return the length of base64,else return 0
*/
SDB_EXPORT int base64Encode ( char *s, int in_size, char *out, int out_size ) ;

/** \fn int base64Decode ( char *s, char *out, int out_size ) 
    \brief Base64 convert string
    \param [in] s Input string
    \param [in] out_size The size of the buffer that 'out' pointing to ,
                                   make sure it's large enough
    \param [out] out Output string
    \return If successful return the length of string,else return 0
*/
SDB_EXPORT int base64Decode ( char *s, char *out, int out_size ) ;

/** \fn int getEnBase64Size ( int size )
    \brief Get string convert base64 need size
    \param [in] size original data size
    \return Base64 size
*/
SDB_EXPORT int getEnBase64Size ( int size ) ;

/** \fn int getDeBase64Size ( char *s )
    \brief Get base64 convert string need size
    \param [in] s Base64
    \return String size
*/
SDB_EXPORT int getDeBase64Size ( char *s ) ;

SDB_EXTERN_C_END

#endif
