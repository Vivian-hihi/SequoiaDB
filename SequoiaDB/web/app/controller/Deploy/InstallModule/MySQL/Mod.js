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
               "valid": allHostSelectList
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
               "name": "User",
               "webName": $scope.autoLanguage( '用户名' ),
               "type": "string",
               "required": true,
               "value": "root",
               "valid": {
                  "min": 1
               }
            } ) ;

            $scope.Form1['inputList'].splice( 3, 0, {
               "name": "Passwd",
               "webName": $scope.autoLanguage( '密码' ),
               "type": "password",
               "required": true,
               "value": "",
               "valid": {
                  "min": 1
               }
            } ) ;
         }
         $scope.Form1['inputList'].splice( isDeployPackage ? 4 : 1, 0, {
            "name": "GrantType",
            "webName": $scope.autoLanguage( '访问授权' ),
            "type": "select",
            "value": 'grant root',
            "valid": [
               { 'key': $scope.autoLanguage( 'root账号授权任意主机访问' ), 'value': 'grant root' },
               { 'key': $scope.autoLanguage( '创建SAC访问的账号' ), 'value': 'create new user' },
               { 'key': $scope.autoLanguage( '不设置授权' ), 'value': 'do nothing' }
            ],
            "onChange": function( name, key, value ){
               if( value == 'create new user' )
               {
                  $scope.Form1['inputList'].splice( 2, 0, {
                     "name": "AuthUser",
                     "webName": $scope.autoLanguage( '账号' ),
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
                  } ) ;
               }
               else if( $scope.Form1['inputList'][2]['name'] == 'AuthUser' )
               {
                  $scope.Form1['inputList'].splice( 2, 2 ) ;
               }
            }
         } ) ;
         $.each( $scope.Form1['inputList'], function( index ){
            var name = $scope.Form1['inputList'][index]['name'] ;
            if ( hasKey( buildConf[0], name ) )
            {
               var tmp = buildConf[0][name] ;

               if ( name == 'dbpath' && isDeployPackage )
               {
                  tmp = catPath( '/opt/sequoiasql/mysql', tmp ) ;
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
                        allHostSelectList.push( { 'key': hostInfo['HostName'], 'value': hostInfo['HostName'] } ) ;
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

         var filterDeployKey = [ 'InstallPath', 'User', 'Passwd' ] ;

         var formVal1 = $scope.Form1.getValue() ;
         $.each( formVal1, function( key2, value ){
            if ( isDeployPackage == true && filterDeployKey.indexOf( key2 ) >= 0 )
            {
               return true ;
            }
            if(  value.length > 0 || ( typeof( value ) == 'number' && isNaN( value ) == false ) )
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
            if(  value.length > 0 || ( typeof( value ) == 'number' && isNaN( value ) == false ) )
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
            if( configure['Config'][0]['GrantType'] == 'do nothing' )
            {
               $scope.Components.Confirm.type = 2 ;
               $scope.Components.Confirm.context = $scope.autoLanguage( '选择不设置授权添加服务后，需要手工设置授权，SAC才能正常访问服务数据，是否继续？' ) ;
               $scope.Components.Confirm.isShow = true ;
               $scope.Components.Confirm.okText = $scope.pAutoLanguage( '继续' ) ;
               $scope.Components.Confirm.ok = function(){
                  $scope.Components.Confirm.isShow = false ;
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
            else
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
      }

   } ) ;
}());