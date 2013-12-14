/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pd.hpp

   Descriptive Name = Problem Determination Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PD_HPP_
#define PD_HPP_
#include <string>
#include <stdlib.h>
#include "core.h"
#include "oss.h"

#ifdef _DEBUG
#define SDB_ASSERT(cond,str)     {if(!(cond)) {pdassert(str,__FUNC__,__FILE__,__LINE__);}}
#define SDB_CHECK(cond,str)      {if(!(cond)) {pdcheck(str,__FUNC__,__FILE__,__LINE__);}}
#else
#define SDB_ASSERT(cond,str)     {if(cond){}}
#define SDB_CHECK(cond,str)      {if(cond){}}
#endif

#define SDB_VALIDATE_GOTOERROR(cond, ret, str) \
   {if(!(cond)) { pdLog(PDERROR, __FUNC__, __FILE__, __LINE__, str); \
   rc=ret; goto error; }}

#define PD_LOG_STRINGMAX 4096

#define PD_LOG(level, fmt, ...) \
   do { \
      if ( _curPDLevel >= level ) \
      { \
         pdLog(level, __FUNC__, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
      } \
   }while (0)

#define PD_LOG_MSG(level, fmt, ...) \
   do { \
      if ( level <= PDERROR ) \
      { \
         UINT32 tid = ossGetCurrentThreadID () ; \
         _pmdEDUCB *cb = pmdGetKRCB()->getEDUMgr()->getEDU( tid ) ; \
         if ( cb ) \
         { \
            cb->printInfo ( EDU_INFO_ERROR, fmt, ##__VA_ARGS__ ) ; \
         } \
      } \
      if ( _curPDLevel >= level ) \
      { \
         pdLog(level, __FUNC__, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
      } \
   } while ( 0 )

#define PD_CHECK(cond, retCode, gotoLabel, level, fmt, ...) \
   do {                                                     \
      if ( !(cond) )                                        \
      {                                                     \
         rc = (retCode) ;                                   \
         PD_LOG ( (level), fmt, ##__VA_ARGS__) ;            \
         goto gotoLabel ;                                   \
      }                                                     \
   } while ( 0 )                                            \


#define PD_RC_CHECK(rc, level, fmt, ...)                    \
   do {                                                     \
      PD_CHECK ( (SDB_OK == (rc)), (rc), error, (level),    \
                 fmt, ##__VA_ARGS__) ;                      \
   } while ( 0 )                                            \


enum PDLEVEL
{
   PDSEVERE = 0,
   PDERROR,
   PDEVENT,
   PDWARNING,
   PDINFO,
   PDDEBUG
};
const CHAR* getPDLevelDesp ( PDLEVEL level ) ;

typedef enum PDLEVEL PDLEVEL ;
#define PD_DFT_DIAGLEVEL PDWARNING
void pdLog(PDLEVEL level, const CHAR* func, const CHAR* file, UINT32 line, const CHAR* format, ...);
extern PDLEVEL _curPDLevel ;
#ifdef _DEBUG
void pdassert(const CHAR* string, const CHAR* func, const CHAR* file, UINT32 line) ;
void pdcheck(const CHAR* string, const CHAR* func, const CHAR* file, UINT32 line) ;
#else
#define pdassert(str1,str2,str3,str4)
#define pdcheck(str1,str2,str3,str4)
#endif

struct _pdGeneralException : public std::exception
{
   std::string s ;
   _pdGeneralException ( std::string ss ) : s (ss) {}
   ~_pdGeneralException() throw() {}
   const CHAR *what() const throw()
   {
      return s.c_str() ;
   }
} ;
typedef struct _pdGeneralException pdGeneralException ;

void pdLog(PDLEVEL level, const CHAR* func, const CHAR* file, UINT32 line, std::string message);
#endif
