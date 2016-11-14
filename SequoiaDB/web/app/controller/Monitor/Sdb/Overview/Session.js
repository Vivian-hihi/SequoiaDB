(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Session.Ctrl', function( $scope, $compile, SdbRest, $location, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;

      var isOpenSelectMenu = false ;
      $scope.SessionList = [] ;
      $scope.SessionGridOptions = { 'titleWidth': [] } ;
      $scope.ShowKeyList = [ 'SessionID', 'TID', 'Type', 'TotalInsert', 'TotalDelete', 'TotalUpdate', 'TotalRead' ] ;
      $scope.ShowKey = [] ;
      $scope.SelectMenu = [] ;
      $scope.OrderByField = [] ;
      $scope.SessionType = 'all' ;

      //设置排序字段
      $scope.SetOrderField = function( fieldName ){
         var normal  = $scope.OrderByField.indexOf( fieldName ) ;
         var reverse = $scope.OrderByField.indexOf( '-' + fieldName ) ;
         if( normal == -1 && reverse == -1 )
         {
            $scope.OrderByField.push( fieldName ) ;
         }
         else if( normal >= 0 )
         {
            $scope.OrderByField[normal] = '-' + fieldName ;
         }
         else if( reverse >= 0 )
         {
            $scope.OrderByField.splice( reverse, 1 ) ;
         }
      }
      
      //创建 设置实时刷新 的弹窗
      $scope.CreatePlayIntervalModel = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '实时刷新设置' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "play",
                  "webName": $scope.autoLanguage( '自动刷新' ),
                  "type": "select",
                  "value": $scope.Timer.status == 'start',
                  "valid": [
                     { 'key': '开启', 'value': true },
                     { 'key': '停止', 'value': false }
                  ]
               },
               {
                  "name": "interval",
                  "webName": $scope.autoLanguage( '刷新间距(秒)' ),
                  "type": "int",
                  "value": $scope.Timer.interval,
                  "valid": {
                     'min': 1
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               $scope.Timer.interval = formVal['interval'] ;
               if( formVal['play'] == true )
               {
                  $scope.Timer.status = 'start' ;
               }
               else
               {
                  $scope.Timer.status = 'stop' ;
               }
            }
            return isAllClear ;
         }
      }

      //渲染网格显示的列
      var gridShowColumn = function(){
         $scope.SessionGridOptions['titleWidth'] = [] ;
         $scope.SessionGridOptions['titleWidth'].push( '50px' ) ;
         var widthPercent = 100 / $scope.ShowKeyList.length ;
         $.each( $scope.ShowKeyList, function( index, keyName ){
            $scope.SessionGridOptions['titleWidth'].push( widthPercent ) ;
         } ) ;
         $scope.SessionGridOptions.onResize() ;
      }

      //保存显示列
      $scope.SaveShowKeyList = function(){
         if( isOpenSelectMenu == true && $scope.Timer.status == 'stop' )
         {
            isOpenSelectMenu = false ;
            $scope.Timer.status = 'start' ;
         }
         $scope.ShowKeyList = [] ;
         $.each( $scope.ShowKey, function( index, keyInfo ){
            if( keyInfo['show'] == true )
            {
               $scope.ShowKeyList.push( keyInfo['key'] ) ;
            }
         } ) ;
         gridShowColumn() ;
      }

      var sql = 'SELECT * FROM $SNAPSHOT_SESSION' ;

      var getSessionList = function(){
         SdbRest.Exec( sql, function( SessionList ){
            $scope.SessionList = [] ;
            var keyList = [] ;
            $.each( SessionList, function( index, value ){
               if( typeof( value['SessionID'] ) != 'undefined' )
               {
                  keyList = SdbFunction.getJsonKeys( value, 0, keyList ) ;
                  $scope.SessionList.push( value )
               }
            } ) ;
            $scope.ShowKey = [] ;
            $scope.SelectMenu = [] ;
            $.each( keyList, function( index, key ){
               $scope.ShowKey.push( { 'key': key, 'show': $scope.ShowKeyList.indexOf( key ) >= 0 } ) ;
               $scope.SelectMenu.push( { 
                  'html': $compile( '<label><div class="Ellipsis" style="padding:5px 10px"><input type="checkbox" ng-model="ShowKey[\'' + index + '\'][\'show\']"/>&nbsp;' + key + '</div></label>' )( $scope ),
                  'onClick': function(){}
               } ) ;
            } ) ;
            $scope.SelectMenu.push( { 
               'html': $compile( '<button class="btn btn-primary" ng-click="SaveShowKeyList()" style="width:100%;">确定</button>' )( $scope )
            } ) ;
            gridShowColumn() ;
            $scope.Timer.complete = true ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               getSessionList() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         }, null ,false ) ;

      } ;

      getSessionList() ;
   

      //显示会话详细
      $scope.ShowSession = function( sessionID ){
         var choose = {} ;
         $.each( $scope.SessionList, function( index, sessionInfo ){
            if( sessionInfo['SessionID'] == sessionID )
            {
               choose = sessionInfo ;
            }
         } )
         $scope.Components.Modal.sessionInfo = choose ;
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '会话详细' ;
         $scope.Components.Modal.noOK = true ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.indexList = '' ;
         $scope.Components.Modal.Context = '\
<table class="table loosen border">\
<tr>\
<td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
<td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
</tr>\
<tr>\
<td>SessionID</td>\
<td>{{data.sessionInfo["SessionID"]}}</td>\
</tr>\
<tr ng-repeat="(key, value) in data.sessionInfo" ng-if="key != \'SessionID\'">\
<td>{{key}}</td>\
<td>{{value}}</td>\
</tr>\
</table>' ;
      } 

      $scope.stopSession = function( ){
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '是的，中断会话' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '要中断该会话吗？' ) ;
         $scope.Components.Confirm.context = '会话id : Host-test-02:11810:10' ;
         $scope.Components.Confirm.ok = function(){
            return true ;
         }
      }
      
      $scope.screenResult = {
         'Role':'all'
      } ;

      $scope.ScreenMenu = [
         { 
             'html': $compile('<label><div style="padding:5px 10px"><input type="radio" name="a" value="all" ng-model="screenResult[\'Role\']" />{{autoLanguage("所有会话")}}</div></label>')($scope),
            'onClick': function(){}
         },
         { 
             'html': $compile('<label><div style="padding:5px 10px"><input type="radio" name="a" value="current" ng-model="screenResult[\'Role\']"/>{{autoLanguage("当前会话")}}</div></label>')($scope),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<button class="btn btn-primary" ng-click="changeScreen()" style="width:100%;">确定</button>' )( $scope )
         }
      ] ; 
         
      $scope.Timer = {
         status: 'stop',
         interval: 5,
         currentTimer: 0,
         complete: false,
         fn: getSessionList
      } ;

      //打开 网格显示列 的下拉菜单
      $scope.OpenSelecMenu = function () {
         if ($scope.Timer.status == 'start') {
            isOpenSelectMenu = true;
            $scope.Timer.status = 'stop';
         }
      } ;
    

      $scope.changeScreen = function(){
         if( isOpenSelectMenu == true && $scope.Timer.status == 'stop' )
         {
            isOpenSelectMenu = false ;
            $scope.Timer.status = 'start' ;
         }
         $scope.SessionType = $scope.screenResult['Role'] ;
         if( $scope.screenResult['Role'] == 'current' )
         {
            sql = 'SELECT * FROM $SNAPSHOT_SESSION_CUR' ;
         }
         else
         {
            sql = 'SELECT * FROM $SNAPSHOT_SESSION' ;
         }
         getSessionList() ;
      } ;



      

      //跳转至部署
      $scope.GotoDeploy = function(){
         $location.path( '/Deploy/Index' ) ;
      } ;

      //跳转至监控主页
      $scope.GotoModule = function(){
         $location.path( '/Monitor/Index' ) ;
      } ;

      //跳转至分区组列表
      $scope.GotoGroups = function(){
         $location.path( '/Monitor/SDB-Overview/Index' ) ;
      } ;
   } ) ;
}());