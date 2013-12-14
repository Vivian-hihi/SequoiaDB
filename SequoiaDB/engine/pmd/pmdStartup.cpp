/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdStartup.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdStartup.hpp"
#include "pmd.hpp"
#include "ossUtil.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{

#define PMD_STARTUP_INVALID_CHAR       "SEQUOIADB:STARTUP"
#define PMD_STARTUP_INVALID_CHAR_LEN   ossStrlen(PMD_STARTUP_INVALID_CHAR)

   _pmdStartup::_pmdStartup () :
   _ok(FALSE),
   _fileOpened ( FALSE ),
   _fileLocked ( FALSE )
   {
   }

   _pmdStartup::~_pmdStartup ()
   {
   }

   void _pmdStartup::ok ( BOOLEAN bOK )
   {
      _ok = bOK ;

      if ( _ok )
      {
         pmdGetKRCB()->setStartType ( SDB_START_NORMAL ) ;
      }
   }

   BOOLEAN _pmdStartup::isOK () const
   {
      return _ok ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__PMDSTARTUP_INIT, "_pmdStartup::init" )
   INT32 _pmdStartup::init ()
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__PMDSTARTUP_INIT );
      BOOLEAN startUpFromCrash = FALSE ;
      pmdKRCB *krcb = pmdGetKRCB() ;
      CHAR dbPath[OSS_MAX_PATHSIZE+1] = {0} ;
      _fileOpened = FALSE ;
      SINT64 written = 0 ;
      if ( krcb->getDBPath( dbPath, OSS_MAX_PATHSIZE ) )
      {
         _fileName =  dbPath ;
         _fileName += OSS_FILE_SEP ;
         _fileName += PMD_STARTUP_FILE_NAME ;

         // attempt to access the file
         rc = ossAccess ( _fileName.c_str() ) ;
         // if the file does not exist, that means we were normally shutdown
         if ( SDB_FNE == rc )
         {
            krcb->setStartType ( SDB_START_NORMAL ) ;
            startUpFromCrash = FALSE ;
            _ok = TRUE ;
         }
         // if we get permission error, we can't continue
         else if ( SDB_PERM == rc )
         {
            PD_LOG ( PDSEVERE,
                     "Permission denied when creating startup file" ) ;
            goto error ;
         }
         // if we can find the file, that means there were unexpected outage
         else if ( SDB_OK == rc )
         {
            krcb->setStartType ( SDB_START_CRASH ) ;
            startUpFromCrash = TRUE ;
            _ok = FALSE ;
         }
         // for unknown error, let's stop starting up the engine
         else
         {
            PD_LOG ( PDSEVERE, "Failed to access startup file, rc = %d", rc ) ;
            goto error ;
         }

         rc = ossOpen ( _fileName.c_str(), OSS_REPLACE|OSS_READWRITE,
                        OSS_RU|OSS_WU|OSS_RG, _file ) ;
         if ( SDB_OK != rc )
         {
#if defined (_WINDOWS)
            if ( SDB_PERM == rc )
            {
               PD_LOG ( PDERROR, "Failed to open startup file due to perm "
                        "error, please check if the directory is granted "
                        "with the right permission, or if there is another "
                        "instance is running with the directory" ) ;
            }
            else
#endif
            {
               PD_LOG ( PDERROR,
                        "Failed to create startup file, rc = %d", rc ) ;
            }
            goto error ;
         }
         _fileOpened = TRUE ;
         // lock the file
         rc = ossLockFile ( &_file, OSS_LOCK_EX ) ;
         if ( SDB_PERM == rc )
         {
            PD_LOG ( PDERROR, "The startup file is already locked, most likely "
                     "there is another instance running in the directory" ) ;
            goto error ;
         }
         else if ( rc )
         {
            PD_LOG ( PDERROR, "Failed to lock startup file, rc = %d", rc ) ;
            goto error ;
         }
         _fileLocked = TRUE ;
         //write char
         rc = ossSeekAndWrite ( &_file, 0, PMD_STARTUP_INVALID_CHAR,
            PMD_STARTUP_INVALID_CHAR_LEN, &written ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }
      // print startup from crash/normal after locked the file
      if ( startUpFromCrash )
      {
         PD_LOG ( PDEVENT, "Start up from crash" ) ;
      }
      else
      {
         PD_LOG ( PDEVENT, "Start up from normal" ) ;
      }
   done:
      /*if ( fileOpened )
      {
         ossClose( _file ) ;
      }*/
      PD_TRACE_EXITRC ( SDB__PMDSTARTUP_INIT, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdStartup::final ()
   {
      if ( _fileLocked )
      {
         ossLockFile ( &_file, OSS_LOCK_UN ) ;
      }
      if ( _fileOpened )
      {
         ossClose( _file ) ;
      }
      if ( _ok )
      {
         ossDelete ( _fileName.c_str() ) ;
      }
      return SDB_OK ;
   }

   pmdStartup & pmdGetStartup ()
   {
      static pmdStartup _startUp ;
      return _startUp ;
   }

}


