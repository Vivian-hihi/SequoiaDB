(function(){
   var sacApp = window.SdbSacManagerModule ;
   var GridId ;
   sacApp.controllerProvider.register( 'Data.Lob.Lobs.Ctrl', function( $scope, $compile, SdbRest, InheritSize, SdbFunction, FormModal){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var csName = SdbFunction.LocalData( 'SdbCsName' ) ;
      var clName = SdbFunction.LocalData( 'SdbClName' ) ;
      var fullName = csName + '.' + clName ;
      printfDebug( 'Cluster: ' + clusterName + ', Module: ' + moduleName + ', cs: ' + csName + ', cl: ' + clName ) ;
      if( clusterName == null || moduleName == null || csName == null || clName == null )
      {
         $location.path( 'Data/Lob/Index' ) ;
         return;
      }

      //调整外层div宽高
      InheritSize.append( $( '#LobIndex' ) ) ;
      $( '#LobIndex > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;
      $( '#GridParent > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;

      $scope.fullName = csName + '.' + clName ;

      //初始化
      var isNotFilter = true ;
      var limit = 30 ;
      var lobContent = [] ;
      $scope.setCurrent = 1 ;
      $scope.current = 1 ;
      $scope.total = 0 ;

      //上一页
      $scope.previous = function()
      {
         --$scope.current ;
         $scope.setCurrent = $scope.current ;
         showPage( $scope.current ) ;
      }
      //检查输入的页数格式
      $scope.checkCurrent = function()
      {
         if( $scope.setCurrent.length == 0 )
         {
            $scope.setCurrent = 1 ;
         }
         else if( isNaN( $scope.setCurrent ) || parseInt( $scope.setCurrent ) != $scope.setCurrent )
         {
            $scope.setCurrent = parseInt( $scope.setCurrent ) ;
            if( isNaN( $scope.setCurrent ) )
            {
               $scope.setCurrent = 1 ;
            }
         }
         else if( $scope.setCurrent > $scope.total )
         {
            $scope.setCurrent = $scope.total ;
         }
      }
      //跳转到指定页
      $scope.gotoPate = function( event )
      {
         if( event.keyCode == 13 )
         {
            $scope.current = $scope.setCurrent ;
            showPage( $scope.current ) ;
         }
      }
      //下一页
      $scope.nextPage = function()
      {
         ++$scope.current ;
         $scope.setCurrent = $scope.current ;
         showPage( $scope.current ) ;
      }

      function showPage( pageNum )
      {
         var newLobs = [] ;
         var startOfNum = limit * ( pageNum - 1 ) ;
         var endOfNum = limit * pageNum ;
         var start = 0 ;
         var end = 0 ;
         endOfNum = ( endOfNum > lobContent.length ? lobContent.length : endOfNum ) ;
         for( var i = startOfNum; i < endOfNum ; ++i )
         {
            newLobs.push( lobContent[i] ) ;
         }
         if( newLobs.length > 0 )
         {
            start = startOfNum + 1 ;
            end = endOfNum ;
         }
         queryLobs( newLobs, start, end ) ;
      }

      function queryLobs( lobs, start, end )
      {
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 执行查询成功，显示 ? - ?，总计 ? 条记录' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, start, end, lobs.length ) ;
         $scope.execRc = true ;
         var gridData = {
            'title': [],
            'body': [],
            'tool':{
               'position': 'bottom',
               'left':
                  []
            },
            'options': {
               'order': { 'active': true },
               'grid': { 'tool': true, titleWidth: [ '60px', '75px', '250px', 40, 30, 30 ] } 
            }
         } ;
         if( isNotFilter )
         {
            gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play fa-flip-horizontal" ng-show="current > 1" ng-click="previous()"></i>' )( $scope ) } ) ;
            gridData.tool.left.push( { 'html': $compile( '<input style="width:100px;" ng-change="checkCurrent()" ng-model="setCurrent" ng-keypress="gotoPate($event)">' )( $scope ) } ) ;
            gridData.tool.left.push( { 'html': $compile( '<span>/<span>' )( $scope ) } ) ;
            gridData.tool.left.push( { 'html': $compile( '<span ng-bind="total"></span>' )( $scope ) } ) ;
            gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play" ng-click="nextPage()"></i>' )( $scope ) } ) ;
         }
         var keyList = [ '', '', 'Oid', 'CreateTime' ] ;
         $.each( lobs, function( index, record ){
             keyList = SdbFunction.getJsonKeys( record, 6, keyList ) ;
         } ) ;
         $.each( keyList, function( index, key ){
            gridData['title'].push( { 'text': key } );
         } ) ;
         $.each( lobs, function( index, record ){
            var id = index + 1 ;
            var line = SdbFunction.getJsonValues( record, keyList, [] ) ;
            var newRow = [] ;
            newRow[0] = { 'text': id } ;
            //构造一个删除按钮
            var removeIcon = $( '<i></i>' ).addClass( 'fa fa-remove' ).text( ' ' + $scope.autoLanguage( '删除' ) ) ;
            var removeBtn = $compile( '<a ng-click="LobDelete(' + ( index + start - 1 ) + ')"></div>' )( $scope ).addClass( 'linkButton' ).append( removeIcon ) ;
            newRow[1] = { 'html': removeBtn } ;
            newRow[2] = { 'html': $compile( '<a ng-click="LobContent(' + index + ')"></a>' )( $scope ).addClass( 'linkButton' ).text( line[2] ) } ;
            newRow[3] = { 'text': line[3] } ;
            newRow[4] = { 'text': line[4] } ;
            newRow[5] = { 'text': line[5] } ;
            gridData['body'].push( newRow ) ;
         } ) ;
         $scope.lobGridData = gridData ;
      }

      //Lob查询所有
      $scope.queryAll = function()
      {
         var data = { 'cmd': 'list lobs', 'name': csName + '.' + clName } ;
         SdbRest.DataOperation( data, function( records ){
            lobContent = records ;
            $scope.total = parseInt( lobContent.length / limit ) ;
            if( lobContent.length % limit > 0 )
            {
               ++$scope.total ;
            }
            $scope.setCurrent = 1 ;
            $scope.current = 1 ;
            showPage( 1 ) ;
         }, function( errorInfo ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 执行查询失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
               $scope.execRc = false ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }
      $scope.queryAll() ;

      //删除Lob记录
      $scope.LobDelete = function( index ){
         var oid = lobContent[index]['Oid']['$oid'] ;
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '是的，删除' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '要删除这条记录吗？' ) ;
         $scope.Components.Confirm.context = 'Oid : ' + oid ;
         $scope.Components.Confirm.ok = function(){
            var data = { 'cmd': 'delete lob', 'name': fullName, 'oid': oid } ;
            SdbRest.DataOperation( data, function( json ){
               $scope.execResult = sprintf( $scope.autoLanguage( '? ? 删除成功' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName ) ;
               $scope.execRc = true ;
               $scope.queryAll() ;
               showPage( $scope.current ) ;
            }, function( errorInfo ){
               $scope.execResult = sprintf( $scope.autoLanguage( '? ? 删除失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
               $scope.execRc = false ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            }, function(){
               //关闭弹窗
               $scope.Components.Modal.isShow = false ;
               $scope.$apply() ;
            } ) ;
            $scope.Components.Confirm.isShow = false ;
         }
      }

      //查找lob
      $scope.LobQuery = function(){
         $scope.Components.Modal.icon = 'fa-search' ;
         $scope.Components.Modal.title = $scope.autoLanguage( 'Lob查询' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "oid",
                  "webName": "Lob Oid",
                  "required": true,
                  "type": "string",
                  "value": "",
                  "valid": {
                     "min": 24,
                     "max": 24
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function( data ){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var newLobs = [] ;
               var value = $scope.Components.Modal.form.getValue() ;
               $.each( lobContent, function( index, lobInfo ){
                  if( lobInfo['Oid']['$oid'] == value['oid'] )
                  {
                     newLobs = [ lobInfo ] ;
                     return false ;
                  }
               } ) ;
               if( newLobs.length > 0 )
               {
                  queryLobs( newLobs, 1, 1 ) ;
               }
               else
               {
                  queryLobs( newLobs, 0, 0 ) ;
               }
            }
            return isAllClear ;
         } 
      }
      
      
      $scope.LobContent = function( index ){
         var records = lobContent[index] ;
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( 'Lob信息' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.Grid = lobContent[index] ;
         $scope.Components.Modal.Context = '\
<table class="table loosen border">\
<tr>\
   <td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
   <td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
</tr>\
<tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key == \'Oid\'">\
   <td>Oid</td>\
   <td>{{value[\'$oid\']}}</td>\
</tr>\
<tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key != \'Oid\'">\
   <td>{{key}}</td>\
   <td ng-if="key != \'CreateTime\'">{{value}}</td>\
   <td ng-if="key == \'CreateTime\'">{{value[\'$timestamp\']}}</td>\
</tr>\
</table>' ;
         $scope.Components.Modal.noOK = true ;
      }

   } ) ;
}()) ;