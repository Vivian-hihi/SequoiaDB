/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdStartup.hpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          26/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef PMD_STARTUP_HPP_
#define PMD_STARTUP_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossIO.hpp"
#include "string"

#define PMD_STARTUP_FILE_NAME          ".SEQUOIADB_STARTUP"
namespace engine
{

   class _pmdStartup : public SDBObject
   {
      public:
         _pmdStartup () ;
         ~_pmdStartup () ;

         INT32 init () ;
         INT32 final () ;
         void  ok ( BOOLEAN bOK = TRUE ) ;
         BOOLEAN isOK () const ;

      private:
         OSSFILE      _file ;
         std::string  _fileName ;
         BOOLEAN      _ok ;
         BOOLEAN      _fileOpened ;
         BOOLEAN      _fileLocked ;
   };

   typedef _pmdStartup pmdStartup ;

   pmdStartup& pmdGetStartup () ;

}

#endif //PMD_STARTUP_HPP_

