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

namespace engine
{
   /*
      _coordOrderKey define
   */
   class _coordOrderKey : public SDBObject
   {
      typedef std::vector< BSONElement >  OrderKeyList;
      typedef std::vector< BSONElement >  OrderKeyEleList;
      typedef std::vector< BSONObj >      OrderKeyObjList;
      public:
         _coordOrderKey( const _coordOrderKey &orderKey ) ;
         _coordOrderKey() ;

      public:
         BOOLEAN operator<( const _coordOrderKey &rhs ) const ;
         void clear() ;
         void setOrderBy( const BSONObj &orderBy ) ;
         INT32 generateKey( const BSONObj &record,
                           _ixmIndexKeyGen *keyGen ) ;

      private:
         BSONObj              _orderBy ;
         ixmHashValue         _hash ;
         BSONObj              _keyObj ;
         BSONElement          _arrEle ;
   } ;
   typedef _coordOrderKey coordOrderKey ;

   /*
      _rtnSubCLBuf
   */
   class _rtnSubCLBuf : public SDBObject
   {
   public:
      _rtnSubCLBuf();
      _rtnSubCLBuf( BSONObj &orderBy,
                    _ixmIndexKeyGen *keyGen );
      virtual ~_rtnSubCLBuf();
      const CHAR *front();
      INT32 pop();
      INT32 popN( SINT32 num );
      INT32 popAll();
      INT32 recordNum();
      INT32 getOrderKey( coordOrderKey &orderKey );
      rtnContextBuf buffer();
      void setBuffer( rtnContextBuf &buffer );

   private:
      coordOrderKey        _orderKey;
      BOOLEAN              _isOrderKeyChange;
      rtnContextBuf        _buffer;
      INT32                _remainNum;
      _ixmIndexKeyGen      *_keyGen;
   };
   typedef class _rtnSubCLBuf rtnSubCLBuf;

   /*
      _rtnContextMainCL define
   */
   class _rtnContextMainCL : public _rtnContextBase
   {
   typedef _utilMap< SINT64, _rtnSubCLBuf, 20 >    SubCLBufList ;
      DECLARE_RTN_CTX_AUTO_REGISTER()
   public:
      _rtnContextMainCL( SINT64 contextID, UINT64 eduID ) ;
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

      virtual INT32 getMore( INT32 maxNumToReturn, rtnContextBuf &buffObj,
                             _pmdEDUCB *cb ) ;

      INT32 addSubContext( SINT64 contextID );

      BOOLEAN requireOrder () const;
   protected:
      virtual INT32 _prepareData( _pmdEDUCB *cb );
      virtual void  _toString( stringstream &ss ) ;

   private:
      INT32 _prepareSubCTXData( SINT64 contextID,
                                _pmdEDUCB * cb,
                                INT32 maxNumToReturn = -1 );
      INT32 _prepareDataByOrder( _pmdEDUCB *cb );

      INT32 _initCLBuf( _pmdEDUCB *cb ) ;

      INT32 _getNextContext( _pmdEDUCB *cb,
                             SINT64 &contextID ) ;

      INT32 _initArgs( const _rtnQueryOptions &options,
                       const std::vector<string> &subs,
                       BOOLEAN shardSort ) ;

   private:
      _rtnQueryOptions  _options ;
      SubCLBufList      _subCLBufList;
      BOOLEAN           _includeShardingOrder;
      _ixmIndexKeyGen   *_keyGen;
      std::list< std::string > _subs ;
      INT64             _numToReturn ;
      INT64             _numToSkip ;
   };
   typedef class _rtnContextMainCL rtnContextMainCL;
}

#endif /* RTN_CONTEXT_MAIN_CL_HPP_ */

