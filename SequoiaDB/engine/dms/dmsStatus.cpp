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

   Source File Name = dmsStatus.hpp

   Descriptive Name = Data Management Service Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   dms Reccord ID (RID).

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/08/2016  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#include "dmsStatus.hpp"


namespace engine
{

   /*
      _dmsStatus implement
   */
   _dmsStatus::_dmsStatus()
   {
   }

   _dmsStatus::~_dmsStatus()
   {
   }

   INT32 _dmsStatus::open( const CHAR *fileName )
   {
      return SDB_OK ;
   }

   void _dmsStatus::close()
   {
   }

   INT32 _dmsStatus::remove()
   {
      return SDB_OK ;
   }

   INT32 _dmsStatus::_write( UINT32 offset, const CHAR * pData, UINT32 len )
   {
      return SDB_OK ;
   }

   INT32 _dmsStatus::_read( UINT32 offset, CHAR * pData, UINT32 len )
   {
      return SDB_OK ;
   }

   /*
      _dmsPersistStatus implement
   */
   _dmsPersistStatus::_dmsPersistStatus()
   {
   }

   _dmsPersistStatus::~_dmsPersistStatus()
   {
   }

   INT32 _dmsPersistStatus::removeItem( const CHAR *name )
   {
      return SDB_OK ;
   }

   INT32 _dmsPersistStatus::addItem( const CHAR *name,
                                     UINT32 commitFlag,
                                     UINT64 commitLSN,
                                     UINT64 commitTime )
   {
      return SDB_OK ;
   }

   BOOLEAN _dmsPersistStatus::findItem( const CHAR *name,
                                        dmsStatusItem &item )
   {
      return FALSE ;
   }

   UINT32 _dmsPersistStatus::findCLItemsByCS( const CHAR *name,
                                              VEC_STATUS_ITEM &vecItems )
   {
      return 0 ;
   }

   UINT32 _dmsPersistStatus::getCount()
   {
      return 0 ;
   }

}


