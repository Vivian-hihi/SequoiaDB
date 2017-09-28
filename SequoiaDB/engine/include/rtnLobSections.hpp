/*******************************************************************************

   Copyright (C) 2011-2017 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = rtnLobSections.hpp

   Descriptive Name = LOB section access management

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/27/2017  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTN_LOB_SECTIONS_HPP_
#define RTN_LOB_SECTIONS_HPP_

#include "oss.hpp"
#include "ossUtil.hpp"
#include <map>
#include <string>

namespace engine
{
   struct _rtnLobSection: public SDBObject
   {
      INT64 offset ;
      INT64 length ;
      INT64 accessId ;

      _rtnLobSection()
         : offset( -1 ),
           length( -1 ),
           accessId( -1 )
      {
      }

      _rtnLobSection( INT64 off, INT64 len, INT64 id )
         : offset( off ),
           length( len ),
           accessId( id )
      {
      }

      _rtnLobSection( const _rtnLobSection& other )
      {
         offset = other.offset ;
         length = other.length ;
         accessId = other.accessId ;
      }

      void operator=( const _rtnLobSection& other )
      {
         offset = other.offset ;
         length = other.length ;
         accessId = other.accessId ;
      }

      OSS_INLINE INT64 start() const
      {
         return offset ;
      }

      OSS_INLINE INT64 end() const
      {
         return ( offset + length - 1 ) ;
      }

      OSS_INLINE BOOLEAN valid() const
      {
         return ( ( offset >= 0 && length > 0 ) ? TRUE : FALSE ) ;
      }

      OSS_INLINE BOOLEAN include( INT64 offset ) const
      {
         if ( offset >= start() && offset <= end() )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }

      OSS_INLINE BOOLEAN overlapped( const _rtnLobSection& other ) const
      {
         if ( other.include( start() ) ||
              other.include( end() ) )
         {
            return TRUE ;
         }
         else if ( include( other.start() ) ||
                   include( other.end() ) )
         {
            return TRUE ;
         }
         else
         {
            return FALSE ;
         }
      }
   } ;

   class _rtnLobSections: public SDBObject
   {
   public:
      _rtnLobSections() ;
      ~_rtnLobSections() ;

   private:
      // disallow copy and assign
      _rtnLobSections( const _rtnLobSections& ) ;
      void operator=( const _rtnLobSections& ) ;

   public:
      std::string toString() const ;
      BOOLEAN overlapped( const _rtnLobSection& section ) const ;
      BOOLEAN completelyContains( const _rtnLobSection& section ) const ;
      BOOLEAN conflicted( INT64 accessId ) const ;
      _rtnLobSection find( INT64 offset ) const ;
      INT32   addSection( const _rtnLobSection& section ) ;
      void    delSectionById( INT64 accessId ) ;

      OSS_INLINE INT32 sectionNum() const { return (INT32)_sections.size() ; }

   private:
      INT32   _addSection( const _rtnLobSection& section ) ;

   private:
      typedef std::map<INT64, _rtnLobSection> LOB_SECTIONS_TYPE ;
      LOB_SECTIONS_TYPE       _sections ;
   } ;
}

#endif /* RTN_LOB_SECTIONS_HPP_ */

