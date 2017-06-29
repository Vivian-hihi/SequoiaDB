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

   Source File Name = rtnContextMainCL.hpp

   Descriptive Name = RunTime MainCL Context Header

   When/how to use: this program may be used on binary and text-formatted
   versions of Runtime component. This file contains structure for Runtime
   Context.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          5/26/2017   David Li  Split from rtnContext.hpp

   Last Changed =

*******************************************************************************/
#ifndef RTN_CONTEXT_MAIN_CL_HPP_
#define RTN_CONTEXT_MAIN_CL_HPP_

#include "rtnContext.hpp"
#include "rtnQueryOptions.hpp"
#include "rtnSubContext.hpp"

namespace engine
{
   /*
      _rtnSubCLContext
   */
   class _rtnSubCLContext : public _rtnSubContext
   {
   public:
      _rtnSubCLContext( BSONObj &orderBy,
                        _ixmIndexKeyGen *keyGen,
                        INT64 contextId );
      virtual ~_rtnSubCLContext() ;
      
   public:
      const CHAR *front() ;
      INT32 pop() ;
      INT32 popN( INT32 num ) ;
      INT32 popAll() ;
      INT32 recordNum() ;
      INT32 remainLength() ;
      INT32 getOrderKey( rtnOrderKey &orderKey );
      rtnContextBuf buffer() ;
      void setBuffer( rtnContextBuf &buffer ) ;

   private:
      rtnContextBuf        _buffer ;
      INT32                _remainNum ;
   };
   typedef class _rtnSubCLContext rtnSubCLContext ;

   /*
      _rtnContextMainCL define
   */
   class _rtnContextMainCL : public _rtnContextBase
   {
   typedef _utilMap< INT64, _rtnSubCLContext*, 20 >    SUBCL_CTX_MAP ;
      DECLARE_RTN_CTX_AUTO_REGISTER()
   public:
      _rtnContextMainCL( INT64 contextID, UINT64 eduID ) ;
      ~_rtnContextMainCL();
      virtual std::string      name() const ;
      virtual RTN_CONTEXT_TYPE getType () const;
      virtual _dmsStorageUnit* getSU () { return NULL ; }

      INT32 open( const bson::BSONObj & orderBy,
                  INT64 numToReturn,
                  INT64 numToSkip ) ;

      INT32 open( const _rtnQueryOptions &options,
                  const std::vector<string> &subs,
                  BOOLEAN shardSort,
                  _pmdEDUCB *cb ) ;

      INT32 addSubContext( SINT64 contextID );

      BOOLEAN requireOrder () const;
   protected:
      virtual INT32 _prepareData( _pmdEDUCB *cb );
      virtual void  _toString( stringstream &ss ) ;

   private:
      INT32 _prepareSubCLData( SINT64 contextID,
                                _pmdEDUCB * cb,
                                INT32 maxNumToReturn = -1 );
      INT32 _prepareDataByOrder( _pmdEDUCB *cb );
      INT32 _prepareDataNormal( _pmdEDUCB *cb ) ;

      INT32 _initSubCLContext( _pmdEDUCB *cb ) ;

      INT32 _getNextContext( _pmdEDUCB *cb,
                             INT64 &contextID ) ;

      INT32 _initArgs( const _rtnQueryOptions &options,
                       const std::vector<string> &subs,
                       BOOLEAN shardSort ) ;

   private:
      _rtnQueryOptions  _options ;
      SUBCL_CTX_MAP     _subContextMap ;
      BOOLEAN           _includeShardingOrder ;
      _ixmIndexKeyGen*  _keyGen ;
      std::list< std::string > _subs ;
      INT64             _numToReturn ;
      INT64             _numToSkip ;
   };
   typedef class _rtnContextMainCL rtnContextMainCL;
}

#endif /* RTN_CONTEXT_MAIN_CL_HPP_ */

