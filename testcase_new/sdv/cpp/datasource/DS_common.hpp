#ifndef DS_COMMON_HPP
#define DS_COMMON_HPP

#if defined (_WINDOWS)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstdlib>
#include <client.hpp>
using namespace sdbclient ;

void ossSleep(int milliseconds)
{
	#if defined (_WINDOWS)
		Sleep(milliseconds) ;
	#else
		usleep(milliseconds*1000) ;
	#endif
}

bool isStandalone()
{
	sdb db ;
	db.connect("localhost","11810") ;
	sdbCursor cursor ;
	int rc = db.getList(cursor,7) ;
	if(rc == -159)
		return true ;
	else
		return false ; 
}

#endif 
