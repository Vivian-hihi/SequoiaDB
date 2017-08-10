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

   Source File Name = omCommandTool.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/17/2017  HJW Initial Draft

   Last Changed =

*******************************************************************************/

#include "omCommandTool.hpp"
#include "omDef.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"

using namespace bson ;

namespace engine
{

   INT32 omXmlTool::readXml2Bson( const string &fileName, BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      try
      {
         ptree pt ;
         read_xml( fileName.c_str(), pt ) ;
         _xml2Bson( pt, obj ) ;
      }
      catch( std::exception &e )
      {
         rc = SDB_INVALIDPATH ;
         PD_LOG_MSG( PDERROR, "%s", e.what() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN omXmlTool::_isStringValue( ptree &pt )
   {
      BOOLEAN isStringV = FALSE ;
      if( _isArray( pt ) )
      {
         isStringV = FALSE ;
         goto done ;
      }

      if( pt.size() == 0 )
      {
         isStringV = TRUE ;
         goto done ;
      }

      if( pt.size() > 1 )
      {
         isStringV = FALSE ;
         goto done ;
      }

      // in this case pt.size() == 1
      {
         ptree::iterator ite = pt.begin() ;
         string key          = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            isStringV = TRUE ;
            goto done ;
         }
      }

   done:
      return isStringV ;
   }

   BOOLEAN omXmlTool::_isArray( ptree &pt )
   {
      BOOLEAN isArr = FALSE ;
      string type ;
      try
      {
         type = pt.get<string>( OM_XMLATTR_TYPE ) ;
      }
      catch( std::exception &e )
      {
         isArr = FALSE ;
         goto done ;
      }

      if ( ossStrcasecmp( type.c_str(), OM_XMLATTR_TYPE_ARRAY ) == 0 )
      {
         isArr = TRUE ;
         goto done ;
      }

   done:
      return isArr ;
   }

   void omXmlTool::_recurseParseObj( ptree &pt, BSONObj &out )
   {
      BSONObjBuilder builder ;
      ptree::iterator ite = pt.begin() ;
      for( ; ite != pt.end() ; ite++ )
      {
         string key = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            continue ;
         }

         ptree child = ite->second ;
         if ( _isArray( child ) )
         {
            BSONArrayBuilder arrayBuilder ;
            _parseArray( child, arrayBuilder ) ;
            builder.append( key, arrayBuilder.arr() ) ;
         }
         else if ( _isStringValue( child ) )
         {
            string value = ite->second.data() ;
            builder.append( key, value ) ;
         }
         else
         {
            // obj
            BSONObj obj ;
            _recurseParseObj( child, obj ) ;
            builder.append(key, obj ) ;
         }
      }
      out = builder.obj() ;
   }


   void omXmlTool::_parseArray( ptree &pt, BSONArrayBuilder &arrayBuilder )
   {
      ptree::iterator ite = pt.begin() ;
      for( ; ite != pt.end() ; ite++ )
      {
         string key    = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            continue ;
         }

         BSONObj obj ;
         ptree child = ite->second ;
         _recurseParseObj( child, obj ) ;
         arrayBuilder.append( obj ) ;
      }
   }

   void omXmlTool::_xml2Bson( ptree &pt, BSONObj &out )
   {
      BSONObjBuilder builder ;
      ptree::iterator ite = pt.begin() ;
      for( ; ite != pt.end() ; ite++ )
      {
         string key = ite->first ;
         if ( ossStrcasecmp( key.c_str(), OM_XMLATTR_KEY ) == 0 )
         {
            continue ;
         }

         ptree child = ite->second ;
         if ( _isArray( child ) )
         {
            BSONArrayBuilder arrayBuilder ;
            _parseArray( child, arrayBuilder ) ;
            builder.append( key, arrayBuilder.arr() ) ;
         }
         else if ( _isStringValue( child ) )
         {
            string value = ite->second.data() ;
            builder.append( key, value ) ;
         }
         else
         {
            // obj
            BSONObj obj ;
            _recurseParseObj( child, obj ) ;
            builder.append(key, obj ) ;
         }
      }
      out = builder.obj() ;
   }

   string omConfigTool::getBuzTemplatePath( const string &businessType,
                                            const string &operationType )
   {
      string templateFile ;

      templateFile = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR
                           + OSS_FILE_SEP + businessType ;

      if( operationType == OM_REST_OPERATION_EXTEND )
      {
         templateFile = templateFile + "_"OM_REST_OPERATION_EXTEND ;
      }

      templateFile = templateFile + OM_TEMPLATE_FILE_NAME + _languageFileSep
                           + OM_CONFIG_FILE_TYPE ;

      return templateFile ;
   }

   string omConfigTool::getBuzConfigPath( const string &businessType,
                                          const string &deployMod,
                                          BOOLEAN isSeparateConfig )
   {
      stringstream path ;

      path << _rootPath ;
      if( OSS_FILE_SEP != _rootPath.substr( _rootPath.length() - 1, 1 ) )
      {
         path << OSS_FILE_SEP ;
      }
      path << OM_BUSINESS_CONFIG_SUBDIR
           << OSS_FILE_SEP
           << businessType ;
      if( isSeparateConfig )
      {
         path << "_" << deployMod ;
      }
      path << OM_CONFIG_ITEM_FILE_NAME
           << _languageFileSep
           << OM_CONFIG_FILE_TYPE ;

      return path.str() ;
   }

   string omConfigTool::getBuzConfigPath( const string &businessType,
                                          const string &deployMod,
                                          const string &isSeparateConfig )
   {
      BOOLEAN sepCfg = FALSE ;
      ossStrToBoolean( isSeparateConfig.c_str(), &sepCfg ) ;
      return getBuzConfigPath( businessType, deployMod, sepCfg ) ;
   }

   INT32 omConfigTool::readBuzTypeList( list<BSONObj> &businessList )
   {
      INT32 rc = SDB_OK ;
      string businessFile ;
      BSONObj fileContent ;

      businessFile = _rootPath + OSS_FILE_SEP + OM_BUSINESS_CONFIG_SUBDIR
                     + OSS_FILE_SEP + OM_BUSINESS_FILE_NAME
                     + _languageFileSep + OM_CONFIG_FILE_TYPE ;

      rc = readXml2Bson( businessFile, fileContent ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "read business file failed:file=%s",
                 businessFile.c_str() ) ;
         goto error ;
      }

      {
         BSONObj businessArray = fileContent.getObjectField(
                                                      OM_BSON_BUSINESS_LIST ) ;
         BSONObjIterator iter( businessArray ) ;
         while( iter.more() )
         {
            BSONElement ele = iter.next() ;
            if( Object != ele.type() )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG( PDERROR, "field is not Object:field=%s,type=%d",
                           OM_BSON_BUSINESS_LIST, ele.type() ) ;
               goto error ;
            }

            BSONObj oneBusiness = ele.embeddedObject() ;
            businessList.push_back( oneBusiness.copy() ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigTool::readBuzTemplate( const string &businessType,
                                        const string &operationType,
                                        list<BSONObj> &objList )
   {
      INT32 rc = SDB_OK ;
      string templateFile ;
      BSONObj templateArray ;
      BSONObj deployMods ;

      templateFile = getBuzTemplatePath( businessType, operationType ) ;

      rc = readXml2Bson( templateFile, templateArray ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "read file failed:file=%s", templateFile.c_str() ) ;
         goto error ;
      }

      deployMods = templateArray.getObjectField( OM_BSON_DEPLOY_MOD_LIST ) ;
      {
         BSONObjIterator iter( deployMods ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            if ( ele.type() != Object )
            {
               rc = SDB_INVALIDARG ;
               PD_LOG_MSG( PDERROR, "field's element is not Object:field=%s"
                           ",type=%d", OM_BSON_DEPLOY_MOD_LIST, ele.type() ) ;
               goto error ;
            }
            BSONObj oneDeployMod = ele.embeddedObject() ;
            objList.push_back( oneDeployMod.copy()) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigTool::readBuzConfig( const string &businessType,
                                      const string &deployMod,
                                      BOOLEAN isSeparateConfig,
                                      BSONObj &obj )
   {
      INT32 rc = SDB_OK ;
      string templateFile ;

      templateFile = getBuzConfigPath( businessType,
                                       deployMod, isSeparateConfig ) ;

      rc = readXml2Bson( templateFile, obj ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "read file failed:file=%s", templateFile.c_str() ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omConfigTool::readBuzConfig( const string &businessType,
                                      const string &deployMod,
                                      const string &isSeparateConfig,
                                      BSONObj &obj )
   {
      BOOLEAN sepCfg = FALSE ;
      ossStrToBoolean( isSeparateConfig.c_str(), &sepCfg ) ;
      return readBuzConfig( businessType, deployMod, sepCfg, obj ) ;
   }

   /**
    * Get task id of thr running business.
    *
    * @param(in)  businessName
    *
    * @return     taskID, -1: not exist   >=0: exist
    *
   */
   INT64 omDatabaseTool::getTaskIdOfRunningBuz( const string &businessName )
   {
      INT32 rc = SDB_OK ;
      INT64 taskID = -1 ;
      SINT64 contextID = -1 ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      string businessKey ;
      rtnContextBuf buffObj ;

      //Status not in( OM_TASK_STATUS_FINISH, OM_TASK_STATUS_CANCEL )
      BSONArrayBuilder arrBuilder ;
      arrBuilder.append( OM_TASK_STATUS_FINISH ) ;
      arrBuilder.append( OM_TASK_STATUS_CANCEL ) ;
      BSONObj status = BSON( "$nin" << arrBuilder.arr() ) ;

      businessKey = OM_TASKINFO_FIELD_INFO"."OM_BSON_BUSINESS_NAME ;
      matcher = BSON( businessKey << businessName
                      << OM_TASKINFO_FIELD_STATUS << status ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_TASKINFO, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to query business:rc=%d,matcher=%s", rc, 
                 matcher.toString().c_str() ) ;
         goto error ;
      }

      rc = rtnGetMore( contextID, 1, buffObj, _cb, _pRTNCB ) ;
      if ( rc )
      {
         if( SDB_DMS_EOC != rc )
         {
            PD_LOG( PDERROR, "Failed to retreive record, rc = %d", rc ) ;
            goto error ;
         }

         // notice: if rc != SDB_OK, contextID is deleted in rtnGetMore
         goto done ;
      }

      {
         BSONObj result( buffObj.data() ) ;
         taskID = result.getField( OM_TASKINFO_FIELD_TASKID ).numberLong() ;
      }

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return taskID ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::addBusinessInfo( const INT32 addType,
                                          const string &clusterName,
                                          const string &businessName,
                                          const string &businessType,
                                          const string &deployMod,
                                          const BSONObj &businessInfo )
   {
      INT32 rc = SDB_OK ;
      time_t now = time( NULL ) ;
      BSONObj buzRecord ;
      BSONObjBuilder builder ;

      builder.append( OM_BUSINESS_FIELD_ADDTYPE, addType ) ;
      builder.append( OM_BUSINESS_FIELD_CLUSTERNAME, clusterName ) ;
      builder.append( OM_BUSINESS_FIELD_NAME, businessName ) ;
      builder.append( OM_BUSINESS_FIELD_TYPE, businessType ) ;
      builder.append( OM_BUSINESS_FIELD_DEPLOYMOD, deployMod ) ;
      builder.appendTimestamp( OM_BUSINESS_FIELD_TIME, (UINT64)now * 1000, 0 ) ;
   
      builder.appendElements( businessInfo ) ;
      buzRecord = builder.obj() ;

      if ( FALSE == clusterIsExist( clusterName ) )
      {
         rc = SDB_OM_CLUSTER_NOT_EXIST ;
         PD_LOG( PDERROR, "cluster does not exist,name:%s",
                 clusterName.c_str() ) ;
         goto error ;
      }

      if ( TRUE == businessIsExist( businessName ) )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "business already exist,name:%s",
                 businessName.c_str() ) ;
         goto error ;
      }

      rc = rtnInsert( OM_CS_DEPLOY_CL_BUSINESS, buzRecord, 1, 0, _cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "insert record failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /**
    * Get business info.
    *
    * @param(in)  businessName
    *
    * @param(out) businessType
    *
    * @param(out) deployMod
    *
    * @param(out) clusterName
    *
   */
   INT32 omDatabaseTool::getBusinessInfo( const string &businessName,
                                          BSONObj &businessInfo )
   {
      INT32 rc = SDB_OK ;
      SINT64 contextID  = -1 ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj condition = BSON( OM_BUSINESS_FIELD_NAME << businessName )  ;

      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, condition, order,
                     hint, 0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         BSONObj tmpConf ;
         rc = rtnGetMore ( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d",
                    OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         businessInfo = result.copy() ;
      }

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /**
    * Get all the business info of the cluster
    *
    * @param(in)  clusterName
    *
    * @param(out) clusterBusinessInfo
    *
   */
   INT32 omDatabaseTool::getBusinessInfoOfCluster(
                                                const string &clusterName,
                                                BSONObj &clusterBusinessInfo )
   {
      BSONObjBuilder bsonBuilder ;
      BSONArrayBuilder arrayBuilder ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj result ;
      SINT64 contextID = -1 ;
      INT32 rc         = SDB_OK ;

      matcher = BSON( OM_BUSINESS_FIELD_CLUSTERNAME << clusterName ) ;
      rc = rtnQuery( OM_CS_DEPLOY_CL_BUSINESS, selector, matcher, order, hint, 
                     0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         rc = rtnGetMore ( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d", 
                    OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
            goto error ;
         }

         BSONObj result( buffObj.data() ) ;
         arrayBuilder.append( result ) ;
      }

      bsonBuilder.append( OM_BSON_FIELD_BUSINESS_INFO, arrayBuilder.arr() ) ;
      clusterBusinessInfo = bsonBuilder.obj() ;

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   /**
    * Get all the address of the business
    *
    * @param(in)  businessName
    *
    * @param(out) addressList
    *
   */
   INT32 omDatabaseTool::getBusinessAddress(
                                       const string &businessName,
                                       vector<simpleAddressInfo> &addressList )
   {
      INT32 rc = SDB_OK ;
      SINT64 contextID  = -1 ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj condition = BSON( OM_BUSINESS_FIELD_NAME << businessName )  ;
      string businessType = "" ;
      string deployMod = "" ;

      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, condition, order,
                     hint, 0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         rtnContextBuf buffObj ;
         BSONObj tmpConf ;
         rc = rtnGetMore ( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d",
                    OM_CS_DEPLOY_CL_BUSINESS, rc ) ;
            goto error ;
         }

         {
            BSONObj result( buffObj.data() ) ;
            string hostName = result.getStringField(
                                                OM_CONFIGURE_FIELD_HOSTNAME ) ;

            if ( 0 == businessType.length() )
            {
               businessType = result.getStringField( OM_BUSINESS_FIELD_TYPE ) ;
            }

            if ( 0 == deployMod.length() )
            {
               deployMod = result.getStringField(
                                                OM_BUSINESS_FIELD_DEPLOYMOD ) ;
            }

            if ( OM_BUSINESS_SEQUOIADB == businessType )
            {
               BSONObj nodes = result.getObjectField(
                                                   OM_CONFIGURE_FIELD_CONFIG ) ;
               BSONObjIterator iterBson( nodes ) ;

               while ( iterBson.more() )
               {
                  simpleAddressInfo address ;
                  BSONElement ele = iterBson.next() ;
                  BSONObj oneNode = ele.embeddedObject() ;
                  string role = oneNode.getStringField( OM_CONF_DETAIL_ROLE ) ;

                  address.hostName = hostName ;
                  address.port = oneNode.getStringField(
                                                      OM_CONF_DETAIL_SVCNAME ) ;
                  if ( OM_DEPLOY_MOD_DISTRIBUTION == deployMod &&
                       OM_NODE_ROLE_COORD == role )
                  {
                     addressList.push_back( address ) ;
                  }
                  else if ( OM_DEPLOY_MOD_STANDALONE == deployMod &&
                            OM_NODE_ROLE_STANDALONE == role )
                  {
                     addressList.push_back( address ) ;
                  }
               }
            }
            
         }
      }

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN omDatabaseTool::businessIsExist( const string &businessName )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isExist = FALSE ;
      BSONObj buzInfo ;
      string businessType = "" ;

      rc = getBusinessInfo( businessName, buzInfo ) ;
      if ( rc )
      {
         isExist = FALSE ;
      }
      else
      {
         businessType = buzInfo.getStringField( OM_BUSINESS_FIELD_TYPE ) ;
         if ( businessType.length() > 0 )
         {
            isExist = TRUE ;
         }
      }

      return isExist ;
   }

   INT32 omDatabaseTool::upsertBusinessInfo( const string &businessName,
                                             const BSONObj &newBusinessInfo,
                                             INT64 &updateNum )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition = BSON( OM_BUSINESS_FIELD_NAME << businessName ) ;
      BSONObj updator = BSON( "$replace" << newBusinessInfo ) ;
      BSONObj hint ;
   
      rc = rtnUpdate( OM_CS_DEPLOY_CL_BUSINESS, condition, updator, hint,
                      FLG_UPDATE_UPSERT | FLG_UPDATE_RETURNNUM,
                      _cb, &updateNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "falied to update business info,"
                          "condition=%s,updator=%s,rc=%d",
                 condition.toString().c_str(),
                 updator.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::addCluster( const BSONObj &clusterInfo )
   {
      INT32 rc = SDB_OK ;
      string installPath ;
      BSONObjBuilder recordBuilder ;
      BSONElement grantConfEle ;
      BSONObj record ;
      BSONObj filter = BSON( OM_BSON_FIELD_CLUSTER_NAME  << "" <<
                             OM_BSON_FIELD_CLUSTER_DESC  << "" <<
                             OM_BSON_FIELD_SDB_USER      << "" <<
                             OM_BSON_FIELD_SDB_PASSWD    << "" <<
                             OM_BSON_FIELD_SDB_USERGROUP << "" ) ;
      BSONObj newClusterInfo = clusterInfo.filterFieldsUndotted( filter,
                                                                 TRUE ) ;

      // ClusterName Desc SdbUser SdbPasswd SdbUserGroup
      recordBuilder.appendElements( newClusterInfo ) ;

      // InstallPath
      installPath = clusterInfo.getStringField( OM_BSON_FIELD_INSTALL_PATH ) ;
      if ( installPath.empty() )
      {
         installPath = OM_DEFAULT_INSTALL_PATH ;
      }
      recordBuilder.append( OM_BSON_FIELD_INSTALL_PATH,
                            OM_DEFAULT_INSTALL_PATH ) ;

      // GrantConf
      grantConfEle = clusterInfo.getField( OM_BSON_FIELD_GRANTCONF ) ;
      if ( bson::Array == grantConfEle.type() )
      {
         BSONArrayBuilder grantConfBuilder ;
         BSONObjIterator iter( grantConfEle.embeddedObject() ) ;

         while ( iter.more() )
         {
            BSONObjBuilder privilegeBuilder ;
            BSONElement ele = iter.next() ;
            BSONObj grantInfo = ele.embeddedObject() ;
            string tmpName = grantInfo.getStringField(
                                                OM_CLUSTER_FIELD_GRANTNAME ) ;
            BOOLEAN tmpPrivilege = grantInfo.getBoolField(
                                                OM_CLUSTER_FIELD_PRIVILEGE ) ;

            privilegeBuilder.append( OM_CLUSTER_FIELD_GRANTNAME,
                                     tmpName ) ;
            privilegeBuilder.appendBool( OM_CLUSTER_FIELD_PRIVILEGE,
                                         tmpPrivilege ) ;
            grantConfBuilder.append( privilegeBuilder.obj() ) ;
         }

         recordBuilder.append( OM_BSON_FIELD_GRANTCONF,
                               grantConfBuilder.arr() ) ;
      }
      else
      {
         BSONArrayBuilder grantConfBuilder ;
         BSONObjBuilder hostFileBuilder ;
         BSONObjBuilder rootUserBuilder ;

         hostFileBuilder.append( OM_CLUSTER_FIELD_GRANTNAME,
                                 OM_CLUSTER_FIELD_HOSTFILE ) ;
         hostFileBuilder.appendBool( OM_CLUSTER_FIELD_PRIVILEGE, TRUE ) ;

         rootUserBuilder.append( OM_CLUSTER_FIELD_GRANTNAME,
                                 OM_CLUSTER_FIELD_ROOTUSER ) ;
         rootUserBuilder.appendBool( OM_CLUSTER_FIELD_PRIVILEGE, TRUE ) ;

         grantConfBuilder.append( hostFileBuilder.obj() ) ;
         grantConfBuilder.append( rootUserBuilder.obj() ) ;

         recordBuilder.append( OM_BSON_FIELD_GRANTCONF,
                               grantConfBuilder.arr() ) ;
      }
      record = recordBuilder.obj() ;

      rc = rtnInsert( OM_CS_DEPLOY_CL_CLUSTER, record, 1, 0, _cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "insert record failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /**
    * Get cluster info
    *
    * @param(in)  clusterName
    *
    * @param(out) clusterInfo
    *
   */
   INT32 omDatabaseTool::getClusterInfo( const string &clusterName,
                                         BSONObj &clusterInfo )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN hasFind   = FALSE ;
      SINT64 contextID  = -1 ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      BSONObj condition = BSON( OM_CLUSTER_FIELD_NAME << clusterName ) ;

      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_CLUSTER, selector, condition, order,
                     hint, 0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
         goto error ;
      }

      while( TRUE )
      {
         rtnContextBuf buffObj ;
         rc = rtnGetMore ( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d",
                    OM_CS_DEPLOY_CL_CLUSTER, rc ) ;
            goto error ;
         }
         BSONObj tmpConf( buffObj.data() ) ;
         clusterInfo = tmpConf ;
         hasFind = TRUE ;
      }

      if( FALSE == hasFind )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "cluster does not exist: %s", clusterName.c_str() ) ;
         goto error ;
      }

   done:
      if ( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::updateClusterInfo( const string &clusterName,
                                            const BSONObj &clusterInfo )
   {
      INT32 rc = SDB_OK ;
      INT64 updateNum = 0 ;
      BSONObj condition = BSON( OM_CLUSTER_FIELD_NAME << clusterName ) ;
      BSONObj updator = BSON( "$set" << clusterInfo ) ;
      BSONObj hint ;

      rc = rtnUpdate( OM_CS_DEPLOY_CL_CLUSTER, condition, updator, hint,
                      FLG_UPDATE_RETURNNUM, _cb, &updateNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "falied to update cluster info,"
                          "condition=%s,updator=%s,rc=%d",
                 condition.toString().c_str(),
                 updator.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN omDatabaseTool::clusterIsExist( const string &clusterName )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isExist = FALSE ;
      BSONObj clusterInfo ;

      rc = getClusterInfo( clusterName, clusterInfo ) ;
      if ( rc )
      {
         isExist= FALSE ;
      }
      else
      {
         isExist = TRUE ;
      }

      return isExist ;
   }

   INT32 omDatabaseTool::updateClusterGrantConf( const string &clusterName,
                                                 const string &grantName,
                                                 const BOOLEAN privilege )
   {
      INT32 rc = SDB_OK ;
      BSONObj clusterInfo ;
      BSONObj newClusterInfo ;
      BSONObj grantConf ;
      map<string,BOOLEAN> grantList ;

      rc = getClusterInfo( clusterName, clusterInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to get cluster info: cluster=%s, rc=%d",
                 clusterName.c_str(), rc ) ;
         goto error ;
      }

      grantConf = clusterInfo.getObjectField( OM_CLUSTER_FIELD_GRANTCONF ) ;
      {
         BSONObjIterator iter( grantConf ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObj grantInfo = ele.embeddedObject() ;
            string tmpName = grantInfo.getStringField(
                                                OM_CLUSTER_FIELD_GRANTNAME ) ;
            BOOLEAN tmpPrivilege = grantInfo.getBoolField(
                                                OM_CLUSTER_FIELD_PRIVILEGE ) ;

            grantList[tmpName] = tmpPrivilege ;
         }
      }

      grantList[grantName] = privilege ;

      {
         BSONObjBuilder grantConfBuilder ;
         BSONArrayBuilder grantArray ;
         map<string,BOOLEAN>::iterator iter ;

         for ( iter = grantList.begin(); iter != grantList.end(); ++iter )
         {
            BSONObjBuilder grantInfoBuilder ;

            grantInfoBuilder.append( OM_CLUSTER_FIELD_GRANTNAME, iter->first ) ;
            grantInfoBuilder.appendBool( OM_CLUSTER_FIELD_PRIVILEGE,
                                         iter->second ) ;

            grantArray.append( grantInfoBuilder.obj() ) ;
         }

         grantConfBuilder.append( OM_CLUSTER_FIELD_GRANTCONF,
                                  grantArray.arr() ) ;
         newClusterInfo = grantConfBuilder.obj() ;
      }

      rc = updateClusterInfo( clusterName, newClusterInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to get cluster info: cluster=%s, rc=%d",
                 clusterName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /**
    * Get one host config.
    *
    * @param(in)  hostName
    *
    * @param(out) config
    * {
    *    "Config": [
    *        {
    *           "BusinessName":"b1",
    *           "BusinessType": "sequoiadb",
    *           "DeployMod": "distribution",
    *           "dbpath":"",
    *           "svcname":"11810",
    *            ...
    *        },
    *        ...
    *     ]
    * }
   **/
   INT32 omDatabaseTool::getOneHostConfig( const string &hostName,
                                           BSONObj &config )
   {
      INT32 rc = SDB_OK ;
      SINT64 contextID = -1 ;
      BSONObj condition ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      BSONObjBuilder condBuilder ;
      BSONObjBuilder confBuilder ;
      BSONArrayBuilder arrayBuilder ;

      //build condition
      condBuilder.append( OM_CONFIGURE_FIELD_HOSTNAME, hostName ) ;
      condition = condBuilder.obj() ;

      // query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_CONFIGURE, selector, condition, order,
                     hint, 0, _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;
         goto error ;
      }

      while ( TRUE )
      {
         string businessName ;
         string businessType ;
         string deployMode ;
         BSONObj nodeConfig ;
         rtnContextBuf buffObj ;

         rc = rtnGetMore( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d",
                    OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;
            goto error ;
         }

         {
            /*
               hostConfig
               {
                  "HostName": "h1", "BusinessName":"b1",
                  "BusinessType": "sequoiadb", "DeployMod": "distribution",
                  "Config": [
                     { "dbpath":"xxxx", "svcname":"11810", ... },
                     ...
                  ]
               }
            */
            BSONObj hostConfig( buffObj.data() ) ;
            businessType = hostConfig.getStringField(
                                            OM_CONFIGURE_FIELD_BUSINESSTYPE ) ;
            deployMode   = hostConfig.getStringField(
                                            OM_CONFIGURE_FIELD_DEPLOYMODE ) ;
            businessName = hostConfig.getStringField(
                                            OM_CONFIGURE_FIELD_BUSINESSNAME ) ;
            nodeConfig = hostConfig.getObjectField(
                                                  OM_CONFIGURE_FIELD_CONFIG ) ;
            BSONObjIterator iter( nodeConfig ) ;
            while ( iter.more() )
            {
               BSONObjBuilder innerBuilder ;
               BSONElement ele = iter.next() ;

               innerBuilder.appendElements( ele.embeddedObject() ) ;
               innerBuilder.append( OM_BSON_BUSINESS_TYPE, businessType ) ;
               innerBuilder.append( OM_BUSINESS_FIELD_DEPLOYMOD, deployMode ) ;
               innerBuilder.append( OM_BSON_BUSINESS_NAME, businessName ) ;
               arrayBuilder.append( innerBuilder.obj() ) ;
            }
         }
      }

      /*
      arrayBuilder
         [
            {
               "BusinessName":"b1",
               "BusinessType": "sequoiadb",
               "DeployMod": "distribution",
               "dbpath":"",
               "svcname":"11810",
                ...
            },
            ...
         ]
      */
      confBuilder.append( OM_BSON_FIELD_CONFIG, arrayBuilder.arr() ) ;
      config = confBuilder.obj() ;

   done:
      if( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
         contextID = -1 ;
      }
      return rc ;
   error:
      goto done ;
   }

   /**
    * Get all the host config of the cluster.
    *
    * @param(in)  clusterName
    *
    * @param(out) config
    * {
    *    "HostInfo": [
    *       {
    *          "HostName": "host1", "ClusterName":"c1",
    *          "Disk": [ { "Name":"dev", "Mount":"/mnt", ... }, ... ],
    *          "Config": [
    *             { "dbpath": "xxxx", "svcname":"11810", ... },
    *             ...
    *          ],
    *          ...
    *       }
    *    ]
    * }
   */
   INT32 omDatabaseTool::getHostConfigOfCluster( const string &clusterName,
                                                 BSONObj &config )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isRecordFetched = FALSE ;
      SINT64 contextID = -1 ;
      BSONObj condition ;
      BSONObj selector ;
      BSONObj order ;
      BSONObj hint ;
      BSONObjBuilder hostsDetailBuilder ;
      BSONObjBuilder condBuilder ;
      BSONArrayBuilder hostArrBuilder ;

      //build condition
      condBuilder.append( OM_HOST_FIELD_CLUSTERNAME, clusterName ) ;
      condition = condBuilder.obj() ;

      //query table
      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, condition, order, hint, 0,
                     _cb, 0, -1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "fail to query table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_HOST, rc ) ;
         goto error ;
      }

      while( TRUE )
      {
         BSONObjBuilder hostBuilder ;
         rtnContextBuf buffObj ;

         rc = rtnGetMore( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            contextID = -1 ;
            PD_LOG( PDERROR, "failed to get record from table:%s,rc=%d",
                    OM_CS_DEPLOY_CL_HOST, rc ) ;
            goto error ;
         }

         {
            string hostName ;
            BSONObj hostConfig ;
            /*
               hostInfo:
               {
                  "HostName": "host1", "ClusterName":"c1",
                  "Disk":[ {"Name":"dev", "Mount":"/mnt", ... }, ... ],
                  ...
               }
            */
            BSONObj hostInfo( buffObj.data() ) ;

            hostName = hostInfo.getStringField( OM_HOST_FIELD_NAME ) ;
            rc = getOneHostConfig( hostName, hostConfig ) ;
            if( rc )
            {
               PD_LOG( PDERROR, "Failed to get host config, rc=%d", rc ) ;
               goto error ;
            }
            hostBuilder.appendElements( hostInfo ) ;
            hostBuilder.appendElements( hostConfig ) ;
         }

         /*
            hostBuilder.obj():
            {
               "HostName": "host1", "ClusterName":"c1",
               "Disk":[ {"Name":"dev", "Mount":"/mnt", ... }, ... ],
               "Config":[
                  { "BusinessName": "b1", "dbpath": "xxxx",
                    "svcname":"11810",
                    ...
                  },
                  ...
               ],
               ...
            }
         */
         hostArrBuilder.append( hostBuilder.obj() ) ;
         isRecordFetched = TRUE ;
      }

      if( isRecordFetched == FALSE )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         PD_LOG( PDERROR, "host is not exist: cluster=%s",
                 clusterName.c_str() ) ;
         goto error ;
      }

      hostsDetailBuilder.append( OM_BSON_FIELD_HOST_INFO, hostArrBuilder.arr() ) ;
      config = hostsDetailBuilder.obj() ;

   done:
      if( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
         contextID = -1 ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::getHostUsedPort( const string &hostName,
                                          vector<string> &portList )
   {
      INT32 rc = SDB_OK ;
      BSONObj hostConfig ;
      BSONObj config ;

      rc = getOneHostConfig( hostName, hostConfig ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to get host config, host=%s, rc=%d",
                 hostName.c_str(), rc ) ;
         goto error ;
      }

      /*
      hostConfig
      {
         "Config": [
             {
                "BusinessName":"b1",
                "BusinessType": "sequoiadb",
                "DeployMod": "distribution",
                "dbpath":"",
                "svcname":"11810",
                 ...
             },
             ...
          ]
      }
      */
      config = hostConfig.getObjectField( OM_BSON_FIELD_CONFIG ) ;

      {
         BSONObjIterator iter( config ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObj nodeConfig = ele.embeddedObject() ;
            string buzType = nodeConfig.getStringField(
                                             OM_CONFIGURE_FIELD_BUSINESSTYPE ) ;

            if ( OM_BUSINESS_SEQUOIADB == buzType )
            {
               portList.push_back( nodeConfig.getStringField(
                                                OM_CONFIGURE_FIELD_SVCNAME ) ) ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::upsertConfigure( const string &businessName,
                                          const string &hostName,
                                          const BSONObj &newConfig,
                                          INT64 &updateNum )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME <<
            businessName << OM_CONFIGURE_FIELD_HOSTNAME <<  hostName ) ;
      BSONObj updator = BSON( "$replace" << newConfig ) ;
      BSONObj hint ;

      rc = rtnUpdate( OM_CS_DEPLOY_CL_CONFIGURE, condition, updator, hint,
                      FLG_UPDATE_UPSERT | FLG_UPDATE_RETURNNUM,
                      _cb, &updateNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "falied to update host config,"
                          "condition=%s,updator=%s,rc=%d",
                 condition.toString().c_str(),
                 updator.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::addNodeConfigOfBusiness( const string &clusterName,
                                                  const string &businessName,
                                                  const string &businessType,
                                                  const BSONObj &newConfig )
   {
      INT32 rc = SDB_OK ;
      INT64 updateNum = 0 ;
      string deployMod ;
      BSONObj hostInfoList ;
      BSONObjBuilder builder ;
      BSONArrayBuilder hostLocation ;

      hostInfoList = newConfig.getObjectField( OM_BSON_FIELD_HOST_INFO ) ;
      BSONObjIterator iter( hostInfoList ) ;
      while ( iter.more() )
      {
         BSONElement ele = iter.next() ;
         BSONObj tmpConfig = ele.embeddedObject() ;
         string hostName = tmpConfig.getStringField(
                                             OM_CONFIGURE_FIELD_HOSTNAME ) ;

         if ( 0 == deployMod.length() )
         {
            deployMod = tmpConfig.getStringField(
                                          OM_CONFIGURE_FIELD_DEPLOYMODE ) ;
         }

         rc = upsertConfigure( businessName, hostName, tmpConfig,
                               updateNum ) ;
         if( rc )
         {
            PD_LOG( PDERROR, "failed to update configure,hostname=%s,rc=%d",
                    hostName.c_str(), rc ) ;
            goto error ;
         }

         //used to construct business info
         hostLocation.append( BSON( OM_CONFIGURE_FIELD_HOSTNAME <<
                                    hostName ) ) ;
      }

      //add business
      builder.append( OM_BUSINESS_FIELD_LOCATION, hostLocation.arr() ) ;
      rc = addBusinessInfo( OM_BUSINESS_ADDTYPE_INSTALL, clusterName,
                            businessName, businessType,
                            deployMod, builder.obj() ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to add business,name=%s,rc=%d",
                 businessName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   /**
    * update node config of the business
    *
    * @param(in) businessName
    *
    * @param(in) newConfig
    * {
    *    "HostInfo": [
    *       {
    *          "BusinessName": xxx,
    *          "BusinessType": xxx,
    *          "ClusterName": xxx,
    *          "DeployMod": xxx,
    *          "HostName": xxx,
    *          "Config": [
    *             { "role": xxx, "svcname": xxx, "dbpath": xxx, ... },
    *             ...
    *          ]
    *       },
    *       ...
    *    ]
    * }
   */
   INT32 omDatabaseTool::updateNodeConfigOfBusiness( const string &businessName,
                                                     const BSONObj &newConfig )
   {
      INT32 rc = SDB_OK ;
      INT64 updateNum = 0 ;
      BSONObj buzInfo ;
      BSONObjBuilder builder ;
      BSONArrayBuilder hostLocation ;
      set<string> newHostList ;
      string businessType = "" ;
      string deployMod = "" ;

      rc = getBusinessInfo( businessName, buzInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to get business info,name=%s,rc=%d",
                 businessName.c_str(), rc ) ;
         goto error ;
      }

      businessType = buzInfo.getStringField( OM_BUSINESS_FIELD_TYPE ) ;
      if ( businessType.empty() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG_MSG( PDERROR, "business does not exist: %s",
                     businessName.c_str() ) ;
         goto error ;
      }

      //host and node info by omagent
      {
         BSONObj hostInfoList = newConfig.getObjectField(
                                                   OM_BSON_FIELD_HOST_INFO ) ;
         BSONObjIterator iter( hostInfoList ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObj tmpConfig = ele.embeddedObject() ;
            string hostName = tmpConfig.getStringField(
                                                OM_CONFIGURE_FIELD_HOSTNAME ) ;

            if ( 0 == deployMod.length() )
            {
               deployMod = tmpConfig.getStringField(
                                             OM_CONFIGURE_FIELD_DEPLOYMODE ) ;
            }

            newHostList.insert( hostName ) ;

            rc = upsertConfigure( businessName, hostName, tmpConfig,
                                  updateNum ) ;
            if( rc )
            {
               PD_LOG( PDERROR, "failed to update configure,hostname=%s,rc=%d",
                       hostName.c_str(), rc ) ;
               goto error ;
            }

            //used to construct business info
            hostLocation.append( BSON( OM_CONFIGURE_FIELD_HOSTNAME <<
                                       hostName ) ) ;

         }
      }

      //upsert business
      {
         BSONObj condition = BSON( OM_BUSINESS_FIELD_LOCATION << "" <<
                                   OM_BUSINESS_FIELD_DEPLOYMOD << "" <<
                                   OM_BUSINESS_FIELD_ID << "" ) ;
         BSONObj businessInfo = buzInfo.filterFieldsUndotted( condition,
                                                              FALSE ) ;
         builder.appendElements( businessInfo ) ;
      }
      builder.append( OM_BUSINESS_FIELD_DEPLOYMOD, deployMod ) ;
      builder.append( OM_BUSINESS_FIELD_LOCATION, hostLocation.arr() ) ;
      rc = upsertBusinessInfo( businessName, builder.obj(), updateNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to update business info,name=%s,rc=%d",
                 businessName.c_str(), rc ) ;
         goto error ;
      }

      //delete the host config that does not exist
      {
         BSONObj deleteCondition ;
         BSONObjBuilder deleteBuilder ;
         BSONObjBuilder conditionBuilder ;
         BSONArrayBuilder notDeleteArray ;

         for ( set<string>::iterator iter = newHostList.begin();
               iter != newHostList.end(); ++iter )
         {
            string hostName = *iter ;
            notDeleteArray.append( hostName ) ;
         }
         conditionBuilder.append( "$nin", notDeleteArray.arr() ) ;

         deleteBuilder.append( OM_CONFIGURE_FIELD_BUSINESSNAME, businessName ) ;
         deleteBuilder.append( OM_CONFIGURE_FIELD_HOSTNAME,
                               conditionBuilder.obj() ) ;
         deleteCondition = deleteBuilder.obj() ;

         rc = removeConfigure( deleteCondition ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "failed to remove configure,condition=%s,rc=%d",
                    deleteCondition.toString().c_str(), rc ) ;
            goto error ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::removeConfigure( const BSONObj &condition )
   {
      INT32 rc = SDB_OK ;
      BSONObj hint ;
   
      rc = rtnDelete( OM_CS_DEPLOY_CL_CONFIGURE, condition, hint, 0, _cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to delete configure from table:%s,rc=%d",
                 OM_CS_DEPLOY_CL_CONFIGURE, rc ) ;
         goto error ;
      }
   
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::removeConfigure( const string &businessName,
                                          const string &hostName )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition = BSON( OM_CONFIGURE_FIELD_BUSINESSNAME <<
            businessName << OM_CONFIGURE_FIELD_HOSTNAME << hostName ) ;

      rc = removeConfigure( condition ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to delete configure from table:%s,"
                          "%s=%s,%s=%s,rc=%d",
                 OM_CS_DEPLOY_CL_CONFIGURE,
                 OM_CONFIGURE_FIELD_BUSINESSNAME, businessName.c_str(),
                 OM_CONFIGURE_FIELD_HOSTNAME, hostName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::upsertAuth( const string &businessName,
                                     const string &authUser,
                                     const string &authPasswd )
   {
      INT32 rc = SDB_OK ;
      INT64 updateNum = 0 ;
      BSONObj condition = BSON( OM_BUSINESSAUTH_BUSINESSNAME << businessName ) ;
      BSONObj updator = BSON( "$replace" <<
            BSON( OM_BUSINESSAUTH_BUSINESSNAME << businessName <<
                  OM_BUSINESSAUTH_USER << authUser <<
                  OM_BUSINESSAUTH_PASSWD << authPasswd ) ) ;
      BSONObj hint ;

      if ( authUser.empty() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "user cannot be empty" ) ;
         goto error ;
      }

      if ( authPasswd.empty() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "password cannot be empty" ) ;
         goto error ;
      }

      rc = rtnUpdate( OM_CS_DEPLOY_CL_BUSINESS_AUTH, condition, updator, hint,
                      FLG_UPDATE_UPSERT | FLG_UPDATE_RETURNNUM,
                      _cb, &updateNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "falied to update business auth,"
                          "condition=%s,updator=%s,rc=%d",
                 condition.toString().c_str(),
                 updator.toString().c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::removeAuth( const string &businessName )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition = BSON( OM_BUSINESSAUTH_BUSINESSNAME << businessName ) ;
      BSONObj hint ;

      rc = rtnDelete( OM_CS_DEPLOY_CL_BUSINESS_AUTH, condition, hint, 0, _cb ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to delete configure from table:%s,"
                          "%s=%s,%s=%s,rc=%d",
                 OM_CS_DEPLOY_CL_BUSINESS_AUTH,
                 OM_BUSINESSAUTH_BUSINESSNAME, businessName.c_str(), rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omDatabaseTool::_getOneHostInfo( const BSONObj &matcher,
                                          const BSONObj &selector,
                                          BSONObj &hostInfo )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isExist = FALSE ;
      SINT64 contextID = -1 ;
      BSONObj order ;
      BSONObj hint ;

      rc = rtnQuery( OM_CS_DEPLOY_CL_HOST, selector, matcher, order, hint, 0,
                     _cb, 0, 1, _pDMSCB, _pRTNCB, contextID );
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to query host:rc=%d,condition=%s", rc,
                 matcher.toString().c_str() ) ;
         goto error ;
      }

      while( TRUE )
      {
         rtnContextBuf buffObj ;

         rc = rtnGetMore ( contextID, 1, buffObj, _cb, _pRTNCB ) ;
         if ( rc )
         {
            if ( SDB_DMS_EOC == rc )
            {
               rc = SDB_OK ;
               break ;
            }

            PD_LOG( PDERROR, "Failed to retreive record, rc = %d", rc ) ;
            goto error ;
         }

         {
            BSONObj result( buffObj.data() ) ;
            hostInfo = result.copy() ;
            isExist = TRUE ;
         }
      }

      if ( FALSE == isExist )
      {
         rc = SDB_DMS_RECORD_NOTEXIST ;
         PD_LOG( PDERROR, "Failed to query host info:rc=%d", rc ) ;
         goto error ;
      }

   done:
      if( -1 != contextID )
      {
         _pRTNCB->contextDelete ( contextID, _cb ) ;
         contextID = -1 ;
      }
      return rc ;
   error:
      goto done ;

   }

   INT32 omDatabaseTool::getHostNameByAddress( const string &address,
                                               string &hostName )
   {
      INT32 rc = SDB_OK ;
      BSONObj matcher = BSON( "$or" << BSON_ARRAY(
                                    BSON( OM_HOST_FIELD_NAME << address ) <<
                                    BSON( OM_HOST_FIELD_IP   << address ) ) ) ;
      BSONObj selector ;
      BSONObj hostInfo ;

      rc = _getOneHostInfo( matcher, selector, hostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get host info:rc=%d", rc ) ;
         goto error ;
      }

      hostName = hostInfo.getStringField( OM_HOST_FIELD_NAME ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   BOOLEAN omDatabaseTool::isHostExistOfCluster( const string &hostName,
                                                 const string &clusterName )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN isExist = FALSE ;
      BSONObj selector ;
      BSONObj matcher ;
      BSONObj hostInfo ;

      matcher = BSON( OM_HOST_FIELD_NAME << hostName <<
                      OM_HOST_FIELD_CLUSTERNAME << clusterName ) ;

      rc = _getOneHostInfo( matcher, selector, hostInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to get host info:rc=%d", rc ) ;
         goto error ;
      }

      isExist = TRUE ;

   done:
      return isExist ;
   error:
      goto done ;
   }

   omRestTool::omRestTool( restAdaptor *pRestAdaptor,
                           pmdRestSession *pRestSession )
   {
      _pRestAdaptor = pRestAdaptor ;
      _pRestSession = pRestSession ;
   }

   void omRestTool::sendRecord2Web( list<BSONObj> &records,
                                    const BSONObj *pFilter,
                                    BOOLEAN inFilter )
   {
      list<BSONObj>::iterator iter ;
      for( iter = records.begin(); iter != records.end(); ++iter )
      {
         if( pFilter )
         {
            BSONObj tmp = iter->filterFieldsUndotted( *pFilter, inFilter ) ;
            _pRestAdaptor->appendHttpBody( _pRestSession, tmp.objdata(),
                                          tmp.objsize(), 1 ) ;
         }
         else
         {
            _pRestAdaptor->appendHttpBody( _pRestSession, iter->objdata(),
                                          iter->objsize(), 1 ) ;
         }
      }

      sendRespone( SDB_OK, "" ) ;
   }

   void omRestTool::sendOkRespone()
   {
      sendRespone( SDB_OK, "" ) ;
   }

   void omRestTool::sendRespone( INT32 rc, const string &detail )
   {
      sendRespone( rc, detail.c_str() ) ;
   }

   void omRestTool::sendRespone( INT32 rc, const char *pDetail )
   {
      list<BSONObj>::iterator iter ;
      BSONObj res ;
      BSONObj defaultRes ;
      BSONObjBuilder resBuilder ;

      defaultRes = BSON( OM_REST_RES_RETCODE << rc <<
                         OM_REST_RES_DESP << getErrDesp( rc ) <<
                         OM_REST_RES_DETAIL << pDetail ) ;

      resBuilder.appendElements( defaultRes ) ;
      for ( iter = _msgList.begin(); iter != _msgList.end(); ++iter )
      {
         resBuilder.appendElements( *iter ) ;
      }

      res = resBuilder.obj() ;
      _pRestAdaptor->setOPResult( _pRestSession, rc, res ) ;
      _pRestAdaptor->sendResponse( _pRestSession, HTTP_OK ) ;

   }

   void omRestTool::appendResponeMsg( const BSONObj &msg )
   {
      _msgList.push_back( msg.copy() ) ;
   }

   INT32 omTaskTool::createTask( INT32 taskType, INT64 taskID,
                                 const string &taskName,
                                 const BSONObj &taskInfo,
                                 const BSONArray &resultInfo )
   {
      INT32 rc = SDB_OK ;
      BSONObj record ;

      BSONObjBuilder builder ;
      time_t now = time( NULL ) ;
      builder.append( OM_TASKINFO_FIELD_TASKID, taskID ) ;
      builder.append( OM_TASKINFO_FIELD_TYPE, taskType ) ;
      builder.append( OM_TASKINFO_FIELD_TYPE_DESC,
                      getTaskTypeStr( taskType ) ) ;
      builder.append( OM_TASKINFO_FIELD_NAME, taskName ) ;
      builder.appendTimestamp( OM_TASKINFO_FIELD_CREATE_TIME,
                               (unsigned long long)now * 1000, 0 ) ;
      builder.appendTimestamp( OM_TASKINFO_FIELD_END_TIME,
                               (unsigned long long)now * 1000, 0 ) ;
      builder.append( OM_TASKINFO_FIELD_STATUS, OM_TASK_STATUS_INIT ) ;
      builder.append( OM_TASKINFO_FIELD_STATUS_DESC,
                      getTaskStatusStr( OM_TASK_STATUS_INIT ) ) ;
      builder.append( OM_TASKINFO_FIELD_AGENTHOST, _localAgentHost ) ;
      builder.append( OM_TASKINFO_FIELD_AGENTPORT, _localAgentService ) ;
      builder.append( OM_TASKINFO_FIELD_INFO, taskInfo ) ;
      builder.append( OM_TASKINFO_FIELD_ERRNO, SDB_OK ) ;
      builder.append( OM_TASKINFO_FIELD_DETAIL, "" ) ;
      builder.append( OM_TASKINFO_FIELD_PROGRESS, 0 ) ;
      builder.append( OM_TASKINFO_FIELD_RESULTINFO, resultInfo ) ;

      record = builder.obj() ;
      rc = rtnInsert( OM_CS_DEPLOY_CL_TASKINFO, record, 1, 0, _cb ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "insert task failed:rc=%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omTaskTool::notifyAgentMsg( const CHAR *pCmd,
                                     const BSONObj &request,
                                     string &errDetail,
                                     BSONObj &result )
   {
      INT32 rc          = SDB_OK ;
      SINT32 flag       = SDB_OK ;
      INT32 contentSize = 0 ;
      CHAR *pContent    = NULL ;
      MsgHeader *pMsg   = NULL ;
      omManager *om     = NULL ;
      pmdRemoteSession *remoteSession = NULL ;
      BSONObj bsonRequest ;
      SDB_ASSERT( pCmd != NULL , "" ) ;

      /*
         pContent has allocated memory. After the remote session ends,
         it is auto free by the destructor of the session
      */
      rc = msgBuildQueryMsg( &pContent, &contentSize, pCmd,
                             0, 0, 0, -1, &request, NULL, NULL, NULL ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "build msg failed:cmd=%s,rc=%d", OM_NOTIFY_TASK,
                 rc ) ;
         goto error ;
      }

      // create remote session
      om = sdbGetOMManager() ;
      remoteSession = om->getRSManager()->addSession( _cb,
                                                      OM_WAIT_SCAN_RES_INTERVAL,
                                                      NULL ) ;
      if( NULL == remoteSession )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "create remote session failed:rc=%d", rc ) ;
         goto error ;
      }

      // send message to agent
      pMsg = (MsgHeader *)pContent ;
      rc = _sendMsgToLocalAgent( om, remoteSession, pMsg ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "send message to agent failed:rc=%d", rc ) ;
         remoteSession->clearSubSession() ;
         goto error ;
      }

      // receiving for agent's response
      rc = _receiveFromAgent( remoteSession, flag, result ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "receive from agent failed:rc=%d", rc ) ;
         goto error ;
      }

      if ( flag )
      {
         rc = flag ;
         errDetail = result.getStringField( OM_REST_RES_DETAIL ) ;
         PD_LOG( PDERROR, "agent process failed:detail=(%s),rc=%d",
                 errDetail.c_str(), rc ) ;
         goto error ;
      }

   done:
      if( remoteSession )
      {
         _clearSession( om, remoteSession ) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 omTaskTool::notifyAgentTask( INT64 taskID, string &errDetail )
   {
      INT32 rc = SDB_OK ;
      BSONObj request ;
      BSONObj result ;

      request = BSON( OM_TASKINFO_FIELD_TASKID << taskID ) ;

      rc = notifyAgentMsg( CMD_ADMIN_PREFIX OM_NOTIFY_TASK,
                           request, errDetail, result ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to notify agent,rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omTaskTool::_sendMsgToLocalAgent( omManager *om,
                                           pmdRemoteSession *pRemoteSession,
                                           MsgHeader *pMsg )
   {
      INT32 rc = SDB_OK ;
      MsgRouteID localAgentID ;

      localAgentID = om->updateAgentInfo( _localAgentHost,
                                          _localAgentService ) ;
      if( NULL == pRemoteSession->addSubSession( localAgentID.value ) )
      {
         rc = SDB_OOM ;
         PD_LOG( PDERROR, "addSubSession failed:id=%ld", localAgentID.value ) ;
         goto error ;
      }

      rc = pRemoteSession->sendMsg( pMsg, PMD_EDU_MEM_ALLOC ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "send msg to localhost's agent failed:rc=%d", rc ) ;
         goto error ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 omTaskTool::_receiveFromAgent( pmdRemoteSession *pRemoteSession,
                                        SINT32 &flag,
                                        BSONObj &result )
   {
      INT32 rc           = SDB_OK ;
      SINT32 startFrom   = 0 ;
      SINT32 numReturned = 0 ;
      SINT64 contextID   = -1 ;
      MsgHeader *pRspMsg = NULL ;
      vector<BSONObj> objVec ;
      VEC_SUB_SESSIONPTR subSessionVec ;

      rc = pRemoteSession->waitReply( TRUE, &subSessionVec ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "wait reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if( 1 != subSessionVec.size() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "unexpected session size:size=%d", 
                 subSessionVec.size() ) ;
         goto error ;
      }

      if( subSessionVec[0]->isDisconnect() )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG(PDERROR, "session disconnected:id=%s,rc=%d", 
                routeID2String(subSessionVec[0]->getNodeID()).c_str(), rc ) ;
         goto error ;
      }

      pRspMsg = subSessionVec[0]->getRspMsg() ;
      if( NULL == pRspMsg )
      {
         rc = SDB_UNEXPECTED_RESULT ;
         PD_LOG( PDERROR, "receive null response:rc=%d", rc ) ;
         goto error ;
      }

      rc = msgExtractReply( (CHAR *)pRspMsg, &flag, &contextID, &startFrom, 
                            &numReturned, objVec ) ;
      if( rc )
      {
         PD_LOG( PDERROR, "extract reply failed:rc=%d", rc ) ;
         goto error ;
      }

      if( objVec.size() > 0 )
      {
         result = objVec[0] ;
         result = result.getOwned() ;
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   void omTaskTool::_clearSession( omManager *om,
                                   pmdRemoteSession *pRemoteSession )
   {
      if( NULL != pRemoteSession )
      {
         pRemoteSession->clearSubSession() ;
         om->getRSManager()->removeSession( pRemoteSession ) ;
      }
   }

   void omErrorTool::setError( BOOLEAN isCover, const CHAR *pFormat, ... )
   {
      if( _isSet == FALSE || isCover == TRUE )
      {
         va_list ap;

         va_start( ap, pFormat ) ;
         vsnprintf( _errorDetail, PD_LOG_STRINGMAX, pFormat, ap ) ;
         va_end( ap ) ;

         _isSet = TRUE ;
      }
   }

   const CHAR* omErrorTool::getError()
   {
      if( _isSet == FALSE )
      {
         _errorDetail[0] = 0 ;
      }
      return _errorDetail ;
   }

}
