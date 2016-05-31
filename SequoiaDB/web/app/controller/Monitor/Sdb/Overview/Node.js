(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Node.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var isOpenSelectMenu = false ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;

      $scope.NodeList = [] ;
      $scope.NodeGridOptions = { 'titleWidth': [], 'order': true } ;
      $scope.ShowKeyList = [ 'NodeName', 'GroupName', 'IsPrimary', 'Role', 'TotalRecord', 'TotalLob', 'LSN' ] ;
      $scope.ShowKey = [] ;
      $scope.SelectMenu = [] ;

      $scope.queryList = function( data, success, failed, error, complete ){
         SdbRest._postTest( './test/nodeList', success, failed, error ) ;
      }
      $scope.OrderByField = [] ;
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
      
      //打开 网格显示列 的下拉菜单
      $scope.OpenSelecMenu = function(){
         if( $scope.Timer.status == 'start' )
         {
            isOpenSelectMenu = true ;
            $scope.Timer.status = 'stop' ;
         }
      }

      //渲染网格显示的列
      var gridShowColumn = function(){
         $scope.NodeGridOptions['titleWidth'] = [] ;
         $scope.NodeGridOptions['titleWidth'].push( '50px' ) ;
         var widthPercent = 100 / $scope.ShowKeyList.length ;
         $.each( $scope.ShowKeyList, function( index, keyName ){
            $scope.NodeGridOptions['titleWidth'].push( widthPercent ) ;
         } ) ;
         //$scope.NodeGridOptions.onResize() ;
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

      $scope.getNodeList = function(){
         $scope.queryList( {}, function( test ){
            $scope.NodeList = test ;
            var keyList = [] ;
            $.each( $scope.NodeList, function( index, value ){
               keyList = SdbFunction.getJsonKeys( value, 0, keyList ) ;
            } ) ;
            $scope.ShowKey = [] ;
            $scope.SelectMenu = [] ;
            $.each( keyList, function( index, key ){
               $scope.ShowKey.push( { 'key': key, 'show': $scope.ShowKeyList.indexOf( key ) >= 0 } ) ;
               $scope.SelectMenu.push( { 
                  'html': $compile( '<label><div style="padding:5px 10px"><input type="checkbox" ng-model="ShowKey[\'' + index + '\'][\'show\']"/>&nbsp;' + key + '</div></label>' )( $scope ),
                  'onClick': function(){}
               } ) ;
            } ) ;
            $scope.SelectMenu.push( { 
               'html': $compile( '<button class="btn btn-primary" ng-click="SaveShowKeyList()" style="width:100%;">确定</button>' )( $scope )
            } ) ;
            gridShowColumn() ;
            $scope.Timer.complete = true ;
         } ) ;
       }

       $scope.Timer = {
         status: 'stop',
         interval: 5,
         currentTimer: 0,
         complete: false,
         fn: $scope.getNodeList
      } ;

      $scope.getNodeList() ;
   } ) ;
}());