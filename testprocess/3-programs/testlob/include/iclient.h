#ifndef __ICLIENT_H__
#define __ICLIENT_H__

#include <string>

using namespace std ;

struct IClient
{
   public:
         virtual int Connect ( ) = 0 ;
         virtual int Putfile ( int id ) = 0 ;
         virtual int Getfile ( int id ) = 0 ;
         virtual int Mix ( int id ) = 0 ;
	 virtual void Release ( ) = 0 ;
} ;

extern "C" IClient* CreateClient ( const char* pSrvAddr, const char* pDbName, int startID ) ;

#endif

