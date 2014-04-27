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

   Source File Name = sptCmdRunner.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_CMDRUNNER_HPP_
#define SPT_CMDRUNNER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossNPipe.hpp"
#include "ossProc.hpp"

#include <vector>
#include <map>

namespace engine
{
   class _sptCmdRunner : public SDBObject
   {
   public:
      _sptCmdRunner() ;
      virtual ~_sptCmdRunner() ;

   public:
      INT32 exec( const CHAR *cmd, UINT32 &exit ) ;

      INT32 done() ;

      INT32 read( CHAR *buf, SINT64 len, SINT64 &got ) ;

   private:
      OSSNPIPE _in ;
      OSSNPIPE _out ;
      OSSPID _id ;
   } ;
   typedef class _sptCmdRunner sptCmdRunner ;
}

#endif

