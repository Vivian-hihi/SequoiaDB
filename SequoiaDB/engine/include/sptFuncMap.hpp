/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptFuncMap.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef SPT_FUNCMAP_HPP_
#define SPT_FUNCMAP_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "sptInvokeDef.hpp"


namespace engine
{
   using namespace JS_INVOKER ;

   class _sptFuncMap : public SDBObject
   {
   public:
      _sptFuncMap()
      :_construct(NULL),
       _destruct(NULL),
       _resolve(NULL)
      {

      }

      virtual ~_sptFuncMap()
      {
         _construct = NULL ;
         _destruct = NULL ;
         _resolve = NULL ;
         _normal.clear() ;
      }
   public:
      typedef std::map<std::string, JS_INVOKER::MEMBER_FUNC>
              NORMAL_FUNCS ;
   public:
      BOOLEAN isMemberFunc( const CHAR *funcName ) const
      {
         return NULL == funcName ?
                FALSE : 0 < _normal.count( funcName ) ;
      }

      JS_INVOKER::MEMBER_FUNC
      getMemberFunc( const CHAR *funcName ) const
      {
         JS_INVOKER::MEMBER_FUNC func = NULL ;
         if ( NULL != funcName )
         {
            NORMAL_FUNCS::const_iterator itr =
                        _normal.find( funcName ) ;
            if ( _normal.end() != itr )
            {
               func = itr->second ;
            }
         }

         return func ;
      }

      const NORMAL_FUNCS &getMemberFuncs()const
      {
         return _normal ;
      }

      BOOLEAN addMemberFunc( const CHAR *name,
                             JS_INVOKER::MEMBER_FUNC f )
      {
         return ( NULL != name && NULL != f ) ?
                _normal.insert( std::make_pair( name, f ) ).second :
                FALSE ;
      }

      void setConstructor( JS_INVOKER::MEMBER_FUNC f )
      {
         _construct = f ;
      }

      void setDestructor( JS_INVOKER::DESTRUCT_FUNC f )
      {
         _destruct = f ;
      }

      void setResolver( JS_INVOKER::RESLOVE_FUNC f )
      {
         _resolve = f ;
      }

      JS_INVOKER::MEMBER_FUNC getConstructor() const
      {
         return _construct ;
      }

      JS_INVOKER::DESTRUCT_FUNC getDestructor() const
      {
         return _destruct ;
      }

      JS_INVOKER::RESLOVE_FUNC getResolver() const
      {
         return _resolve ;
      }
   private:

      NORMAL_FUNCS  _normal ;
      JS_INVOKER::MEMBER_FUNC _construct ;
      JS_INVOKER::DESTRUCT_FUNC _destruct ;
      JS_INVOKER::RESLOVE_FUNC _resolve ;
   } ;
   typedef class _sptFuncMap sptFuncMap ;
}

#endif

