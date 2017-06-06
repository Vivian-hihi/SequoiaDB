#include <string.h>
#include "core.h"
#include "ossErr.h"
#include "parseMandocCpp.hpp"


// class parseMandoc
parseMandoc::parseMandoc()
{
   /*
   INT32 options = MPARSE_SO | MPARSE_UTF8 | MPARSE_LATIN1;  
   memset(&_conf, 0, sizeof(_conf));
   memset(&_curp, 0, sizeof(_curp));
   _curp.outtype = OUTT_UTF8;
   _curp.wlevel  = MANDOCLEVEL_BADARG;
   _curp.outopts = &_conf.output;

	mchars_alloc();
	_curp.mp = mparse_alloc(options, _curp.wlevel, mmsg, NULL);
	*/
}

parseMandoc::~parseMandoc()
{
   /*
   if ( _curp.outdata )
   {
      ascii_free(_curp.outdata) ;
   }
   if ( _curp.mp )
   {
      mparse_free(_curp.mp);
   }
   mchars_free();
   */
}

parseMandoc& parseMandoc::getInstance()
{
   static parseMandoc _instance ;
   return _instance ;
}

INT32 parseMandoc::parse(const CHAR* filename)
{
   INT32 rc = SDB_OK ;
#if defined _WIN32
   const CHAR *argv[6] = { "sdb", "-K", "utf-8", "-T", "locale", filename } ;
#else
   const CHAR *argv[6] = { "sdb", "-K", "utf-8", "-T", "utf8", filename } ;
#endif
   rc = parse_main(6, (const CHAR **)argv) ;
   if ( rc )
   {
      rc = SDB_SYS ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}
/*
INT32 parseMandoc::parse(const CHAR* filename)
{
   INT32 rc      = SDB_OK ;
   INT32 fd      = -1 ;
   
   if ( filename == NULL ) {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   mparse_reset(_curp.mp);
   fd = mparse_open(_curp.mp, filename) ;
   if ( fd == -1 ) {
      rc = SDB_SYS;
      goto error ;
   }
   // parse the file
   ::parse( &_curp, fd, filename ) ;
   
done:
   return rc ;
error:
   goto done ;
   
}
*/

/*
INT32 parseMandoc::parse(const CHAR* filename)
{
   INT32 rc      = SDB_OK ;
   INT32 fd      = -1 ;
   INT32 flag    = 0 ;
   INT32 options = MPARSE_SO | MPARSE_UTF8 | MPARSE_LATIN1;
   struct curparse curp ;
   struct manconf conf ;

   if ( filename == NULL ) {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   
   memset(&conf, 0, sizeof(conf));
   memset(&curp, 0, sizeof(curp));
   curp.outtype = OUTT_LOCALE;
   curp.wlevel  = MANDOCLEVEL_BADARG;
   curp.outopts = &conf.output;

	mchars_alloc();
   flag = 1;
	curp.mp = mparse_alloc(options, curp.wlevel, mmsg, NULL);
   fd = mparse_open(curp.mp, filename) ;
   if ( fd == -1 ) {
      rc = SDB_SYS;
      goto error ;
   }
   // parse the file
   ::parse( &curp, fd, filename ) ;
   
done:
   if ( curp.outdata )
   {
      ascii_free(curp.outdata) ;
   }
   if ( curp.mp )
   {
      mparse_free(curp.mp);
   }
   if ( flag )
   {
      mchars_free();
   }
   return rc ;
error:
   goto done ;
}
*/

