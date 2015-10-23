
// --------------------- Index ---------------------
var _IndexPublic = {} ;

//创建错误弹窗
_IndexPublic.createErrorModel = function( $scope, context ){
   $scope.Components.Confirm.type = 3 ;
   $scope.Components.Confirm.noOK = true ;
   $scope.Components.Confirm.noClose = true ;
   $scope.Components.Confirm.context = context ;
   $scope.Components.Confirm.isShow = true ;
}

//语言控制器
_IndexPublic.languageCtrl = function( $scope, text ){
   var newText = text ;
   if( $scope.Language == 'en' )
   {
      function setLanguage()
      {
         if( typeof( window.SdbSacLanguage[text] ) == 'undefined' )
         {
            printfDebug( '"' + text + '" 没翻译！' ) ;
         }
         else
         {
            newText = window.SdbSacLanguage[text] ;
         }
      }
      if( typeof( window.SdbSacLanguage ) == 'undefined' )
      {
         //获取语言
         $.ajax( './app/language/English.json', { 'async': false, 'success': function( reqData ){
            window.SdbSacLanguage = JSON.parse( reqData ) ;
            setLanguage() ;
         }, 'error': function( XMLHttpRequest, textStatus, errorThrown ){
            window.SdbSacLanguage = {} ;
            _IndexPublic.createErrorModel( $scope, 'Can not find the language file, please try to refresh your browser by pressing F5.' ) ;
         } } ) ;
      }
      else
      {
         setLanguage() ;
      }
   }
   return newText ;
}

//创建选择module弹窗
_IndexPublic.createSelectModuleModel = function( $scope, $location, SdbRest, SdbFunction, clusterName, main ){
   var defaultEvent = function( moduleMode, moduleName ){
      if( moduleMode == null || moduleName == null )
      {
         $location.path( 'Data/Overview/Index' ) ;
      }
      else
      {
         if( typeof( main ) == 'function' )
         {
            main( moduleMode, moduleName ) ;
         }
      }
   } ;
   var selectEvent = function( moduleMode, moduleName ){
      SdbFunction.LocalData( 'SdbModuleMode', moduleMode ) ;
      SdbFunction.LocalData( 'SdbModuleName', moduleName ) ;
      location.reload( false ) ;
   } ;
   $scope.Components.Modal.icon = '' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '选择业务' ) ;
   $scope.Components.Modal.noOK = false ;
   $scope.Components.Modal.setModule = function( moduleMode, moduleName ){
      printfDebug( '当前选择业务为： ' + moduleName ) ;
      if( typeof( selectEvent ) == 'function' )
      {
         selectEvent( moduleMode, moduleName ) ;
      }
   }
   $scope.Components.Modal.Context = '<table class="table loosen border">\
   <tbody>\
      <tr style="font-weight:bold;">\
         <td style="width:40%;background-color:#F1F4F5;">Module Name</td>\
         <td style="width:30%;background-color:#F1F4F5;">DeployMod</td>\
         <td style="width:30%;background-color:#F1F4F5;">Cluster</td>\
      </tr>\
      <tr ng-repeat="moduleInfo in data.moduleList track by $index">\
         <td><a class="linkButton" ng-click="data.setModule(moduleInfo.DeployMod,moduleInfo.BusinessName)">{{moduleInfo.BusinessName}}</a></td>\
         <td>{{moduleInfo.DeployMod}}</td>\
         <td>{{moduleInfo.ClusterName}}</td>\
      </tr>\
   </tbody>\
</table>' ;
   var quertModule = function(){
      SdbRest.OmOperation( { 'cmd': 'query business', 'filter': JSON.stringify( { 'ClusterName': clusterName } ) }, function( moduleList ){
         $scope.Components.Modal.moduleList = moduleList ;
         if( typeof( defaultEvent ) == 'function' )
         {
            if( moduleList.length > 0 )
            {
               defaultEvent( moduleList[0]['DeployMod'], moduleList[0]['BusinessName'] ) ;
            }
            else
            {
               defaultEvent( null, null ) ;
            }
         }
         if( moduleList.length > 1 )
         {
            $scope.Components.Modal.isShow = true ;
         }
      }, function( errorInfo ){
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '获取数据失败' ) ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
         $scope.Components.Confirm.ok = function(){
            $scope.Components.Confirm.isShow = false ;
            quertModule() ;
         }
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }, null, true ) ;
      $scope.Components.Modal.moduleList = [
         { "BusinessName": "myModule", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_1" },
         { "BusinessName": "TestModule_2", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_1" },
         { "BusinessName": "TestModule_3", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_2" },
         { "BusinessName": "TestModule_4", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_2" },
         { "BusinessName": "TestModule_5", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_2" },
         { "BusinessName": "TestModule_6", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" },
         { "BusinessName": "TestModule_7", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" },
         { "BusinessName": "TestModule_8", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" },
         { "BusinessName": "TestModule_9", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" }
      ] ;
   }
   var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;
   var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
   if( moduleMode == null || moduleName == null )
   {
      quertModule() ;
   }
   else
   {
      if( typeof( main ) == 'function' )
      {
         main( moduleMode, moduleName ) ;
      }
   }
}

//创建选择cluster弹窗
_IndexPublic.createSelectClusterModel = function( $scope, SdbRest, SdbFunction, main ){
   var defaultEvent = function( clusterName ){
      if( clusterName == null )
      {
         $location.path( '/deployment/index.html' ) ;
      }
      else
      {
         if( typeof( main ) == 'function' )
         {
            main( clusterName ) ;
         }
      }
   } ;
   var selectEvent = function( clusterName ){
      SdbFunction.LocalData( 'SdbClusterName', clusterName ) ;
      location.reload( false ) ;
   } ;
   $scope.Components.Modal.icon = '' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '选择集群' ) ;
   $scope.Components.Modal.noOK = true ;
   $scope.Components.Modal.Context = '<table class="table loosen border">\
   <tbody>\
      <tr style="font-weight:bold;">\
         <td style="width:40%;background-color:#F1F4F5;">' + $scope.autoLanguage( '集群名' ) + '</td>\
         <td style="width:30%;background-color:#F1F4F5;">' + $scope.autoLanguage( '描述' ) + '</td>\
         <td style="width:15%;background-color:#F1F4F5;">' + $scope.autoLanguage( '主机数' ) + '</td>\
         <td style="width:15%;background-color:#F1F4F5;">' + $scope.autoLanguage( '业务数' ) + '</td>\
      </tr>\
      <tr ng-repeat="clusterInfo in data.clusterList track by $index">\
         <td><a class="linkButton" ng-click="data.setCluster(clusterInfo.ClusterName)">{{clusterInfo.ClusterName}}</a></td>\
         <td>{{clusterInfo.Desc}}</td>\
         <td></td>\
         <td></td>\
      </tr>\
   </tbody>\
</table>' ;
   $scope.Components.Modal.setCluster = function( name ){
      printfDebug( '当前选择Cluster为： ' + name ) ;
      if( typeof( selectEvent ) == 'function' )
      {
         selectEvent( name ) ;
      }
   }
   var quertCluster = function(){
      SdbRest.OmOperation( { 'cmd': 'query cluster' }, function( clusterList ){
         $scope.Components.Modal.clusterList = clusterList ;
         if( typeof( defaultEvent ) == 'function' )
         {
            if( clusterList.length > 0 )
            {
               defaultEvent( clusterList[0]['ClusterName'] ) ;
            }
            else
            {
               defaultEvent( null ) ;
            }
         }
         if( clusterList.length > 1 )
         {
            $scope.Components.Modal.isShow = true ;
         }
      }, function( errorInfo ){
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '获取数据失败' ) ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
         $scope.Components.Confirm.ok = function(){
            $scope.Components.Confirm.isShow = false ;
            quertCluster() ;
         }
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }, null, true ) ;
   } ;

   var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
   if( clusterName == null )
   {
      SdbFunction.LocalData( 'SdbModuleMode', null ) ;
      SdbFunction.LocalData( 'SdbModuleName', null ) ;
      quertCluster() ;
   }
   else
   {
      if( typeof( main ) == 'function' )
      {
         main( clusterName ) ;
      }
   }
}

// --------------------- Index.Bottom ---------------------
var _IndexBottom = {} ;

//获取系统时间
_IndexBottom.getSystemTime = function( $scope )
{
   var times = $.now() ;
   setInterval( function(){
      var date = new Date( times ) ;
      var year = date.getFullYear() ;
      var hour = date.getHours() ;
      var minute = date.getMinutes() ;
      var second = date.getSeconds() ;
      $scope.Bottom.year = year ;
      $scope.Bottom.nowtime = pad( hour, 2 ) + ':' + pad( minute, 2 ) + ':' + pad( second, 2 ) ;
      $scope.$apply() ;
      times += 1000 ;
   }, 1000 ) ;
}

//获取ping值
_IndexBottom.checkPing = function( $scope, SdbRest )
{
   SdbRest.getPing( function( times ){
      if( times >= 0 )
      {
         $scope.Bottom.sysStatus = $scope.autoLanguage( '良好' ) ;
         $scope.Bottom.statusColor = 'success' ;
         setTimeout( function(){
            _IndexBottom.checkPing( $scope, SdbRest ) ;
         }, 5000 ) ;
      }
      else
      {
         $scope.Bottom.sysStatus = $scope.autoLanguage( '网络错误' ) ;
         $scope.Bottom.statusColor = 'error' ;
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }
      $scope.$apply() ;
   } ) ;
}

// --------------------- Index.Top ---------------------
var _IndexTop = {} ;

//创建修改密码弹窗
_IndexTop.createPasswdModel = function( $scope, SdbRest ){
   $scope.Components.Modal.icon = 'fa-lock' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '修改密码' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "password",
            "webName": $scope.autoLanguage( "密码" ),
            "type": "password",
            "required": true,
            "value": "",
            "valid": {
               "min": 1,
               "max": 1024
            }
         },
         {
            "name": "newPassword",
            "webName": $scope.autoLanguage( "新密码" ),
            "type": "password",
            "required": true,
            "value": "",
            "valid": {
               "min": 1,
               "max": 1024
            }
         }
      ]
   } ;
   $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
   $scope.Components.Modal.ok = function(){
      var isAllClear = $scope.Components.Modal.form.check() ;
      if( isAllClear )
      {
         var value = $scope.Components.Modal.form.getValue() ;
         SdbRest.ChangePasswd( 'admin', value['password'], value['newPassword'], function( json ){
            $scope.Components.Confirm.isShow = true ;
            $scope.Components.Confirm.type = 4 ;
            $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
            $scope.Components.Confirm.context = $scope.autoLanguage( '修改密码成功。' ) ;
            $scope.Components.Confirm.noOK = true ;
         }, function( errorInfo ){
            $scope.Components.Confirm.isShow = true ;
            $scope.Components.Confirm.type = 1 ;
            $scope.Components.Confirm.title = $scope.autoLanguage( '修改密码失败。' ) ;
            $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
            $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
            $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。' ), errorInfo['errno'], errorInfo['detail'] ) ;
            $scope.Components.Confirm.ok = function(){
               $scope.Components.Confirm.isShow = false ;
               _IndexTop.createPasswdModel( $scope, SdbRest ) ;
            }
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         }, function(){
            //关闭弹窗
            $scope.Components.Modal.isShow = false ;
         } ) ;
      }
      return isAllClear ;
   }
}

//登出
_IndexTop.logout = function( $location, SdbFunction ){
   //删除会话
   SdbFunction.LocalData( 'SdbSessionID', null ) ;
   //删除用户名
   SdbFunction.LocalData( 'SdbUser', null ) ;
   //删除选中的集群
   SdbFunction.LocalData( 'SdbClusterName', null ) ;
   //删除选中的业务
   SdbFunction.LocalData( 'SdbModuleType', null ) ;
   SdbFunction.LocalData( 'SdbModuleMode', null ) ;
   SdbFunction.LocalData( 'SdbModuleName', null ) ;
   //删除cs
   SdbFunction.LocalData( 'SdbCsName', null ) ;
   //删除cl
   SdbFunction.LocalData( 'SdbClType', null ) ;
   SdbFunction.LocalData( 'SdbClName', null ) ;
   $location.path( '/login.html#/Login' ) ;
}