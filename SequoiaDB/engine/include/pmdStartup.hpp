/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

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

