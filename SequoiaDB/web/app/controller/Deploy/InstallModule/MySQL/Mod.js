//@ sourceURL=Mod.js
//"use strict" ;
(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Deploy.MySQL.Mod.Ctrl', function( $scope, $compile, $location, $rootScope, $interval, SdbRest, SdbFunction, Loading ){
      
      var configure      = $rootScope.tempData( 'Deploy', 'ModuleConfig' ) ;
      $scope.ModuleName  = $rootScope.tempData( 'Deploy', 'ModuleName' ) ;
      var deployType     = $rootScope.tempData( 'Deploy', 'Model' ) ;
      var clusterName    = $rootScope.tempData( 'Deploy', 'ClusterName' ) ;
      if( deployType == null || clusterName == null || $scope.ModuleName == null || configure == null )
      {
         $location.path( '/Deploy/Index' ).search( { 'r': new Date().getTime() } ) ;
         return ;
      }
      $scope.stepList = _Deploy.BuildSdbMysqlStep( $scope, $location, $scope['Url']['Action'] ) ;
      if( $scope.stepList['info'].length == 0 )
      {
         $location.path( '/Deploy/Index' ).search( { 'r': new Date().getTime() } ) ;
         return ;
      }

      $scope.ShowType  = 1 ;
      
      var isDeployPackage = false ; //是否需要部署包
      var allHostSelectList = [] ;
      var hostSelectList = [] ;
      var installConfig = [] ;
      var buildConf = [] ;
      //切换分类
      $scope.SwitchParam = function( type ){
         $scope.ShowType = type ;
      }

      //生成form表单
      function buildConfForm( config )
      {
         installConfig = config[0] ;
         buildConf = config[0]['Config'] ;
         $scope.Template = config[0]['Property'] ;
         $scope.Form1 = {} ;
         $scope.Form1 = {
            'keyWidth': '200px',
            'inputList': _Deploy.ConvertTemplate( $scope.Template, 0 )
         } ;
         if( isDeployPackage == false )
         {
            $scope.Form1['inputList'].splice( 0, 0, {
               "name": "HostName",
               "webName": $scope.autoLanguage( '主机名' ),
               "type": "select",
               "value": hostSelectList[0]['key'],
               "valid": hostSelectList
            } ) ;
         }
         else
         {
            $scope.Form1['inputList'].splice( 0, 0, {
               "name": "HostName",
               "webName": $scope.autoLanguage( '主机名' ),
               "type": "select",
               "required": true,
               "value": allHostSelectList[0]['key'],
               "valid": allHostSelectList,
               "onChange": function( name, key, value ){
                  var index2= 0 ;
                  $.each( allHostSelectList, function( index, hostInfo ){
                     if( hostInfo['key'] == key )
                     {
                        index2 = index ;
                        return false ;
                     }
                  } ) ;
                  var rootPath = allHostSelectList[index2]['Disk'][0]['Mount'] ;
                  if( rootPath == '/' )
                  {
                     rootPath = '/opt/sequoiasql/mysql' ;
                  }
                  else if( rootPath.indexOf( 'sequoiasql/mysql' ) < 0 )
                  {
                     rootPath = catPath( rootPath, 'sequoiasql/mysql' ) ;
                  }
                  $.each( $scope.Form1['inputList'], function( index ){
                     var name = $scope.Form1['inputList'][index]['name'] ;
                     if ( name == 'dbpath' )
                     {
                        var tmp = catPath( rootPath, 'database/3306' ) ;
                        $scope.Form1['inputList'][index]['value'] = tmp ;
                     }
                  } ) ;
               }
            } ) ;

            $scope.Form1['inputList'].splice( 1, 0, {
               "name": "InstallPath",
               "webName": $scope.autoLanguage( '安装路径' ),
               "type": "string",
               "required": true,
               "value": '/opt/sequoiasql/mysql/',
               "valid": {
                  "min": 1
               }
            } ) ;

            $scope.Form1['inputList'].splice( 2, 0, {
               "name": "SystemAdmin",
               "webName": $scope.autoLanguage( '系统管理员' ),
               "type": "group",
               "child": [
                  {
                     "name": "User",
                     "webName": $scope.autoLanguage( '用户名' ),
                     "type": "string",
                     "required": true,
                     "value": "root",
                     "valid": {
                        "min": 1
                     }
                  },
                  {
                     "name": "Passwd",
                     "webName": $scope.autoLanguage( '密码' ),
                     "type": "password",
                     "required": true,
                     "value": "",
                     "valid": {
                        "min": 1
                     }
                  } 
               ]
            } ) ;
         }

         $scope.Form1['inputList'].splice( isDeployPackage ? 3 : 1, 0, {
            "name": "DatabaseAuth",
            "webName": $scope.autoLanguage( '新建数据库鉴权' ),
            "type": "group",
            "child": [
               {
                  "name": "AuthUser",
                  "webName": $scope.autoLanguage( '用户名' ),
                  "type": "string",
                  "required": true,
                  "value": '',
                  "valid": {
                     "min": 1,
                     "max": 127,
                     "regex": '^[0-9a-zA-Z]+$'
                  }
               },
               {
                  "name": "AuthPasswd",
                  "webName": $scope.autoLanguage( '密码' ),
                  "type": "password",
                  "required": true,
                  "value": '',
                  "valid": {
                     "min" : 1
                  }
               }
            ]
         } ) ;

         var rootPath = allHostSelectList[0]['Disk'][0]['Mount'] ;
         if( rootPath == '/' )
         {
            rootPath = '/opt/sequoiasql/mysql' ;
         }
         else if( rootPath.indexOf( 'sequoiasql/mysql' ) < 0 )
         {
            rootPath = catPath( rootPath, 'sequoiasql/mysql' ) ;
         }
         $.each( $scope.Form1['inputList'], function( index ){
            var name = $scope.Form1['inputList'][index]['name'] ;
            if ( hasKey( buildConf[0], name ) )
            {
               var tmp = buildConf[0][name] ;
               if ( name == 'dbpath' && isDeployPackage )
               {
                  tmp = catPath( rootPath, 'database/3306' ) ;
               }

               $scope.Form1['inputList'][index]['value'] = tmp ;
            }
         } ) ;
      }

      //获取配置
      var getModuleConfig = function(){
         var data = { 'cmd': 'get business config', 'TemplateInfo': JSON.stringify( configure ) } ;
         SdbRest.OmOperation( data, {
            'success': function( config ){
               buildConfForm( config ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getModuleConfig() ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      //获取主机列表
      var getHostList = function(){
         var data = { 'cmd': 'query host' } ;
         SdbRest.OmOperation( data, {
            'success': function( hostList ){
               configure['HostInfo'] = [] ;
               $.each( hostList, function( index, hostInfo ){
                  if( hostInfo['ClusterName'] == clusterName )
                  {
                     $.each( hostInfo['Packages'], function( packageIndex, packageInfo ){
                        allHostSelectList.push( { 'key': hostInfo['HostName'], 'value': hostInfo['HostName'], 'Disk': hostInfo['Disk'] } ) ;
                        if( packageInfo['Name'] == 'sequoiasql-mysql' )
                        {
                           hostSelectList.push( { 'key': hostInfo['HostName'], 'value': hostInfo['HostName'] } ) ;
                           configure['HostInfo'].push( { 'HostName': hostInfo['HostName'] } ) ; 
                        }
                     } ) ;
                  }
               } ) ;

               if ( hostSelectList.length == 0 )
               {
                  isDeployPackage = true ;
                  configure['HostInfo'].push( { 'HostName': allHostSelectList[0]['value'] } ) ; 
               }

               getModuleConfig() ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getHostList() ;
                  return true ;
               } ) ;
            }
         } ) ;
      }
      getHostList() ;
      
      $scope.GotoDeploy = function(){
         $location.path( '/Deploy/Index' ).search( { 'r': new Date().getTime() } ) ;
      }

      var installMysql = function( installConfig ){
         var data = { 'cmd': 'add business', 'Force': true, 'ConfigInfo': JSON.stringify( installConfig ) } ;
         SdbRest.OmOperation( data, {
            'success': function( taskInfo ){
               $rootScope.tempData( 'Deploy', 'ModuleTaskID', taskInfo[0]['TaskID'] ) ;
               $location.path( '/Deploy/Task/Module' ).search( { 'r': new Date().getTime() } ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  installMysql( installConfig ) ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      function convertConfig()
      {
         var config = {} ;
         config['ClusterName']  = installConfig['ClusterName'] ;
         config['BusinessType'] = installConfig['BusinessType'] ;
         config['BusinessName'] = installConfig['BusinessName'] ;
         config['DeployMod']    = installConfig['DeployMod'] ;

         var allFormVal = {} ;

         var isAllClear1 = $scope.Form1.check( function( formVal ){
            var error = [] ;
            if( checkPort( formVal['port'] ) == false )
            {
               error.push( { 'name': 'port', 'error': sprintf( $scope.autoLanguage( '?格式错误。' ), $scope.autoLanguage( '端口' ) ) } ) ;
            }
            if( formVal['dbpath'].length == 0 )
            {
               error.push( { 'name': 'dbpath', 'error': sprintf( $scope.autoLanguage( '?长度不能小于?。' ), $scope.autoLanguage( '数据路径' ), 1 ) } ) ;
            }
            return error ;
         } ) ;

         if( isAllClear1 == false )
         {
            return false ;
         }

         var filterDeployKey = [ 'InstallPath', 'SystemAdmin' ] ;

         var formVal1 = $scope.Form1.getValue() ;

         formVal1['GrantType'] = 'create new user' ;

         $.each( formVal1, function( key2, value ){
            if ( isDeployPackage == true && filterDeployKey.indexOf( key2 ) >= 0 )
            {
               return true ;
            }
            if( key2 == 'DatabaseAuth' )
            {
               allFormVal['AuthUser'] = value['AuthUser'] ;
               allFormVal['AuthPasswd'] = value['AuthPasswd'] ;
            }
            else if( value.length > 0 || ( typeof( value ) == 'number' && isNaN( value ) == false ) )
            {
               allFormVal[key2] = value ;
            }
         } ) ;

         config['Config'] = [ {} ] ;
         $.each( allFormVal, function( key, value ){
            config['Config'][0][key] = value ;
         } ) ;

         return config ;
      }

      function convertDeployConfig()
      {
         var allFormVal = {} ;

         var formVal = $scope.Form1.getValue() ;
         $.each( formVal, function( key2, value ){
            if( key2 == 'SystemAdmin' )
            {
               allFormVal['User'] = value['User'] ;
               allFormVal['Passwd'] = value['Passwd'] ;
            }
            else if(  value.length > 0 || ( typeof( value ) == 'number' && isNaN( value ) == false ) )
            {
               allFormVal[key2] = value ;
            }
         } ) ;

         var config = {} ;
         config['ClusterName'] = installConfig['ClusterName'] ;
         config['PackageName'] = 'sequoiasql-mysql' ;
         config['InstallPath'] = allFormVal['InstallPath'] ;
         config['HostInfo'] = JSON.stringify( { "HostInfo" : [ {
            "HostName": allFormVal['HostName'],
            "User":     allFormVal['User'],
            "Passwd":   allFormVal['Passwd']
         } ] } ) ;
         config['User']       = allFormVal['User'] ;
         config['Passwd']     = '-' ;
         config['Enforced']   = false;

         return config ;
      }

      function deployPackage( addBuzConfig )
      {
         var config = convertDeployConfig() ;
         var data = {
            'cmd': 'deploy package',
            'ClusterName': config['ClusterName'],
            'PackageName': config['PackageName'],
            'InstallPath': config['InstallPath'],
            'HostInfo':    config['HostInfo'],
            'User':        config['User'],
            'Passwd':      config['Passwd'],
            'Enforced':    config['Enforced']
         } ;
         SdbRest.OmOperation( data, {
            'success': function( taskInfo ){
               $rootScope.tempData( 'Deploy', 'SecondTask', addBuzConfig ) ;
               $rootScope.tempData( 'Deploy', 'ModuleTaskID', taskInfo[0]['TaskID'] ) ;
               $location.path( '/Deploy/Task/Module' ).search( { 'r': new Date().getTime() } ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  deployPackage( packageInfo ) ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      $scope.GotoInstall = function(){
         var oldConfigure = convertConfig() ;
         if( oldConfigure == false )
         {
            return ;
         }
         var configure = {} ;
         $.each( oldConfigure, function( key, value ){
            configure[key] = value ;
         } ) ;
         configure['Config'] = [] ;
         $.each( oldConfigure['Config'], function( nodeIndex, nodeInfo ){
            var nodeConfig = {} ;
            $.each( nodeInfo, function( key, value ){
               if( value.length > 0 )
               {
                   nodeConfig[key] = value ;
               }
            } ) ;
            configure['Config'].push( nodeConfig ) ;
         } ) ;
         if( configure )
         {
            if ( isDeployPackage )
            {
               deployPackage( configure ) ;
            }
            else
            {
               installMysql( configure ) ;
            }
         }
      }

   } ) ;
}());