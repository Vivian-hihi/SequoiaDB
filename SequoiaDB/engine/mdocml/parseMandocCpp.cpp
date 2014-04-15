#include "parseMandocCpp.hpp"
//#include "ossUtil.h"
#include "ossErr.h"
#include <string.h>

namespace engine
{
   parseMandoc* parseMandoc::m_pInstance = NULL;

   parseMandoc* parseMandoc::createInstance()
   {
      if ( !m_pInstance )
      {
         m_pInstance = new parseMandoc();
         if ( !m_pInstance )
//            ossPrintf( "Failed to new parseMandoc."OSS_NEWLINE );
         ;
      }
      return m_pInstance;
   }

   void parseMandoc::destroyInstance()
   {
      if ( m_pInstance )
      {
         delete m_pInstance;
         m_pInstance = NULL;
      }
   }

   // class parseMandoc
   parseMandoc::parseMandoc()
   {
      memset(&curp, 0, sizeof(struct curparse));
      type = MPARSE_AUTO;
      curp.outtype = OUTT_ASCII;
      curp.wlevel = MANDOCLEVEL_FATAL;
      curp.mp = mparse_alloc(type, curp.wlevel, mmsg, &curp, NULL);
      if (OUTT_MAN == curp.outtype)
         mparse_keep(curp.mp);
   }

   parseMandoc::~parseMandoc()
   {
      if (curp.outfree)
         (*curp.outfree)(curp.outdata);
      if (curp.mp)
         mparse_free(curp.mp);
   }

   INT32 parseMandoc::parse(const CHAR* filename)
   {
      INT32 rc = SDB_OK ;
      enum mandoclevel ret = MANDOCLEVEL_OK;
      // parse the file
      ::parse(&curp, -1, filename, &ret);
      if  ( ret != MANDOCLEVEL_OK )
      {
         rc = SDB_INVALIDARG ;
      }
      return rc ;
   }
}
