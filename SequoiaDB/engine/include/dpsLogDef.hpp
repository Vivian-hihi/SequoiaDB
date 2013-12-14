/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossTypes.hpp

   Descriptive Name = Operating System Services Types Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains declare for data types used in
   SequoiaDB.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGDEF_HPP_
#define DPSLOGDEF_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "dpsDef.hpp"

namespace engine
{

   class DPS_LSN : public SDBObject
   {
   public :
      /// 0x00 - 0x07
      DPS_LSN_OFFSET offset ;
      /// 0x08 - 0x0B
      DPS_LSN_VER  version ;

      DPS_LSN()
      {
         offset = DPS_INVALID_LSN_OFFSET ;
         version = DPS_INVALID_LSN_VERSION ;
      }

      DPS_LSN( const DPS_LSN &lsn )
      {
         offset = lsn.offset ;
         version = lsn.version ;
      }

      DPS_LSN &operator=( const DPS_LSN &lsn )
      {
         offset = lsn.offset ;
         version = lsn.version ;
         return *this ;
      }

      BOOLEAN invalid() const
      {
         return ( DPS_INVALID_LSN_OFFSET == offset ) ||
                ( DPS_INVALID_LSN_VERSION == version ) ;
      }

      INT32 compareVersion( const DPS_LSN_VER &version ) const
      {
         INT32 rc = 0 ;
         if ( DPS_INVALID_LSN_VERSION == this->version &&
              DPS_INVALID_LSN_VERSION == version )
         {
            goto done ;
         }
         else if ( DPS_INVALID_LSN_VERSION != this->version &&
                   DPS_INVALID_LSN_VERSION == version )
         {
            rc = 1 ;
            goto done ;
         }
         else if ( DPS_INVALID_LSN_VERSION == this->version &&
                   DPS_INVALID_LSN_VERSION != version )
         {
            rc = -1 ;
            goto done ;
         }
         else if ( this->version < version )
         {
            rc = -1 ;
            goto done ;
         }
         else if ( this->version > version )
         {
            rc = 1 ;
            goto done ;
         }
         else
         {
            goto done ;
         }
      done:
         return rc ;
      }

      INT32 compareOffset( const DPS_LSN_OFFSET &offset ) const
      {
         INT32 rc = 0 ;
         if ( DPS_INVALID_LSN_OFFSET == this->offset &&
              DPS_INVALID_LSN_OFFSET == offset )
         {
            goto done ;
         }
         else if ( DPS_INVALID_LSN_OFFSET != this->offset &&
                   DPS_INVALID_LSN_OFFSET == offset )
         {
            rc = 1 ;
            goto done ;
         }
         else if ( DPS_INVALID_LSN_OFFSET == this->offset &&
                   DPS_INVALID_LSN_OFFSET != offset )
         {
            rc = -1 ;
            goto done ;
         }
         else if ( this->offset < offset )
         {
            rc = -1 ;
            goto done ;
         }
         else if ( this->offset > offset )
         {
            rc = 1 ;
            goto done ;
         }
         else
         {
            goto done ;
         }
      done:
         return rc ;
      }

      /// 0 means this = lsn
      /// < 0 means this < lsn
      /// > 0 means this > lsn
      INT32 compare( const DPS_LSN &lsn ) const
      {
         INT32 rc = 0 ;
         rc = compareVersion( lsn.version ) ;
         if ( 0 != rc )
         {
            goto done ;
         }
         else
         {
            rc = compareOffset( lsn.offset ) ;
            goto done ;
         }
      done:
         return rc ;
      }
   } ;
}

#endif

