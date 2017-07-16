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

   INT32 omDatabaseTool::updateBusinessInfo( const string &businessName,
                                             const BSONObj &newBusinessInfo,
                                             INT64 &updateNum )
   {
      INT32 rc = SDB_OK ;
      BSONObj condition = BSON( OM_BUSINESS_FIELD_NAME << businessName ) ;
      BSONObj updator = BSON( "$replace" << newBusinessInfo ) ;
      BSONObj hint ;
   
      rc = rtnUpdate( OM_CS_DEPLOY_CL_BUSINESS, condition, updator, hint,
                      FLG_UPDATE_RETURNNUM,
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
      set<string> removeHostList ;
      string businessType = "" ;
      string deployMod = "" ;

      rc = getBusinessInfo( businessName, buzInfo ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to get business info,name=%s,rc=%d",
                 businessName.c_str(), rc ) ;
         goto error ;
      }

      businessType = buzInfo.getStringField( OM_BUSINESS_FIELD_TYPE );
      if ( businessType.length() == 0 )
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

      //business info location hosts
      {
         BSONObj hostInfoList = buzInfo.getObjectField(
                                                OM_BUSINESS_FIELD_LOCATION ) ;
         BSONObjIterator iter( hostInfoList ) ;
         while ( iter.more() )
         {
            BSONElement ele = iter.next() ;
            BSONObj tmpHostInfo = ele.embeddedObject() ;
            string hostName = tmpHostInfo.getStringField(
                                                OM_CONFIGURE_FIELD_HOSTNAME ) ;

            if( newHostList.find( hostName ) == newHostList.end() )
            {
               //find all the hosts that have been deleted
               removeHostList.insert( hostName ) ;
            }
         }
      }


      {
         BSONObj condition = BSON( OM_BUSINESS_FIELD_ADDTYPE << 0 <<
                                   OM_BUSINESS_FIELD_NAME << "" <<
                                   OM_BUSINESS_FIELD_TYPE << "" <<
                                   OM_BUSINESS_FIELD_CLUSTERNAME << "" <<
                                   OM_BUSINESS_FIELD_TIME << "" ) ;
         BSONObj businessInfo = buzInfo.filterFieldsUndotted( condition, true ) ;
         builder.appendElements( businessInfo ) ;
      }
      builder.append( OM_BUSINESS_FIELD_DEPLOYMOD, deployMod ) ;
      builder.append( OM_BUSINESS_FIELD_LOCATION, hostLocation.arr() ) ;

      rc = updateBusinessInfo( businessName, builder.obj(), updateNum ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "failed to update business info,name=%s,rc=%d",
                 businessName.c_str(), rc ) ;
         goto error ;
      }

      //delete the host config that does not exist
      for ( set<string>::iterator iter = removeHostList.begin();
            iter != removeHostList.end(); ++iter )
      {
         string hostName = *iter ;
         rc = removeConfigure( businessName, hostName ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "failed to remove configure,"
                             "businessName=%s,hostName=%s,rc=%d",
                    businessName.c_str(), hostName.c_str(), rc ) ;
            goto error ;
         }
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
      BSONObj hint ;

      rc = rtnDelete( OM_CS_DEPLOY_CL_CONFIGURE, condition, hint, 0, _cb ) ;
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

      sendResponse( SDB_OK, "" ) ;
   }

   void omRestTool::sendOkResonse()
   {
      sendResponse( SDB_OK, "" ) ;
   }

   void omRestTool::sendResponse( INT32 rc, const string &detail )
   {
      sendResponse( rc, detail.c_str() ) ;
   }

   void omRestTool::sendResponse( INT32 rc, const char *pDetail )
   {
      BSONObj res = BSON( OM_REST_RES_RETCODE << rc <<
                          OM_REST_RES_DESP << getErrDesp( rc ) <<
                          OM_REST_RES_DETAIL << pDetail ) ;

      _pRestAdaptor->setOPResult( _pRestSession, rc, res ) ;
      _pRestAdaptor->sendResponse( _pRestSession, HTTP_OK ) ;

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
