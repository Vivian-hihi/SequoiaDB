/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = coordSequenceAgent.hpp

   Descriptive Name = Coordinator Sequence Agent

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2018  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef COORD_SEQUENCE_AGENT_HPP_
#define COORD_SEQUENCE_AGENT_HPP_

#include "oss.hpp"
#include "ossUtil.hpp"
#include "msg.h"
#include "utilConcurrentMap.hpp"
#include "../bson/bsonobj.h"
#include <string>

namespace engine
{
   class _coordResource ;
   class _coordSequence ;
   class _pmdEDUCB ;

   class _coordSequenceAgent: public SDBObject
   {
   private:
      typedef utilConcurrentMap<std::string, _coordSequence*, 64> COORD_SEQ_MAP ;
   public:
      _coordSequenceAgent() ;
      ~_coordSequenceAgent() ;

      INT32 init( _coordResource* resource ) ;
      void fini() ;

   public:
      INT32 getNextValue( const std::string& sequenceName, INT64& nextValue, _pmdEDUCB* eduCB ) ;
      BOOLEAN removeCache( const std::string& sequenceName ) ;
      void clear() ;

   private:
      INT32 _getNextValueByXLock( const std::string& sequenceName, INT64& nextValue, _pmdEDUCB* eduCB ) ;
      INT32 _getNextValueBySLock( const std::string& sequenceName, INT64& nextValue, BOOLEAN& noCache, bson::OID& oid, _pmdEDUCB* eduCB ) ;
      INT32 _getNextValueFromCache( _coordSequence& seq, INT64& nextValue, _pmdEDUCB* eduCB ) ;
      INT32 _acquireSequence( _coordSequence& seq, _pmdEDUCB* eduCB ) ;
      INT32 _processAcquireReply( MsgHeader* msg, _coordSequence& seq ) ;
      BOOLEAN _removeCacheByOID( const std::string& sequenceName, bson::OID& oid ) ;

   private:
      _coordResource*   _resource ;
      COORD_SEQ_MAP     _sequenceCache ;
   } ;
   typedef _coordSequenceAgent coordSequenceAgent ;

   INT32 coordSequenceInvalidateCache( const std::string& sequenceName, _pmdEDUCB* eduCB ) ;
}

#endif /* COORD_SEQUENCE_AGENT_HPP_ */

