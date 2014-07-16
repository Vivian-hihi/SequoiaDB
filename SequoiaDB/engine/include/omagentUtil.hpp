#ifndef OMAGENT_UTIL_HPP_
#define OMAGENT_UTIL_HPP_

#include "core.hpp"
#include "ossUtil.hpp"

namespace CLSMGR
{

   INT32 checkBuffer ( CHAR **ppBuffer, INT32 *bufferSize,
                       INT32 packetLength ) ;

   INT32 readFile ( const CHAR * name , CHAR ** buf , UINT32 * bufSize,
                    UINT32 * readSize ) ;
}

#endif // OMAGENT_UTIL_HPP_
