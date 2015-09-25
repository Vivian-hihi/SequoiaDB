(function(){
   var sacApp = window.SdbSacManagerModule ;
   var GridId ;
   sacApp.controllerProvider.register( 'Data.Operate.Index.Ctrl', function( $scope, $compile, $location, SdbRest, InheritSize, SdbFunction ){
      SdbFunction.LocalData( 'SdbCsName', null ) ;
      SdbFunction.LocalData( 'SdbClName', null ) ;
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      printfDebug( 'Cluster: ' + clusterName + ', Module: ' + moduleName + ', Mode: ' + moduleMode ) ;
      if( clusterName == null )
      {
         $scope.selectCluster( function( clusterName ){
            SdbFunction.LocalData( 'SdbClusterName', clusterName ) ;
            location.reload( false ) ;
         } ) ;
      }
      else if( moduleName == null || moduleMode == null )
      {
         $scope.selcetModule( function( moduleName ){
            SdbFunction.LocalData( 'SdbModuleName', moduleName ) ;
            location.reload( false ) ;
         } ) ;
      }
      
      $scope.moduleName = moduleName ;

      //修正宽高
      InheritSize.append( $( '#OperateIndex' ) ) ;
      $( '#OperateIndex > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;

      var limit = 30 ;
      var clsInfo = [] ;
      $scope.setCurrent = 1 ;
      $scope.current = 1 ;
      $scope.total = 0 ;

      //页面跳转
      $scope.gotoRecord = function( listIndex ){
         var fullName = clsInfo[listIndex]['FullName'] ;
         var csName = fullName.split( '.' )[0] ;
         var clName = fullName.split( '.' )[1] ;
         printfDebug( 'cs: ' + csName + ', cl: ' + clName ) ;
         SdbFunction.LocalData( 'SdbCsName', csName ) ;
         SdbFunction.LocalData( 'SdbClName', clName ) ;
         $location.path( 'Data/Operate/Record' ) ;
      }

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
         var newClList = [] ;
         var startOfNum = limit * ( pageNum - 1 ) ;
         var endOfNum = limit * pageNum ;
         endOfNum = ( endOfNum > clsInfo.length ? clsInfo.length : endOfNum ) ;
         for( var i = startOfNum; i < endOfNum ; ++i )
         {
            newClList.push( clsInfo[i] ) ;
         }
         var gridData = {
            'title': [],
            'body': [],
            'tool': {
               'position': 'bottom',
               'left': [
                  { 'html': $compile( '<i class="fa fa-refresh" ng-click="getCLList()"></i>' )( $scope ).attr( 'data-desc', $scope.autoLanguage( '刷新' ) ) },
                  { 'html': $compile( '<i class="fa fa-play fa-flip-horizontal" ng-show="current > 1" ng-click="previous()"></i>' )( $scope ) },
                  { 'html': $compile( '<input style="width:100px;" ng-change="checkCurrent()" ng-model="setCurrent" ng-keypress="gotoPate($event)">' )( $scope ) },
                  { 'html': $compile( '<span>/<span>' )( $scope ) },
                  { 'html': $compile( '<span ng-bind="total"></span>' )( $scope ) },
                  { 'html': $compile( '<i class="fa fa-play" ng-show="current < total" ng-click="nextPage()"></i>' )( $scope ) }
               ],
               'right': [
                  { 'text': sprintf( $scope.autoLanguage( '一共 ? 个集合。' ), clsInfo.length ) }
               ]
            },
            'options': {
               'order': { 'active': true },
               'grid': {
                  'tdModel': 'fixed',
                  'tdHeight': '19px'
               }
            }
         } ;
         var gridTitle = [
            "",
            $scope.autoLanguage( '集合空间' ),
            $scope.autoLanguage( '集合' ),
            $scope.autoLanguage( '分区类型' ),
            $scope.autoLanguage( '主集合' ),
            $scope.autoLanguage( '记录数' ),
            $scope.autoLanguage( '索引数' )
         ] ;
         $.each( gridTitle, function( index, titleText ){
            gridData['title'].push( { 'text': titleText } ) ;
         } ) ;
         $.each( newClList, function( index, clInfo ){
            var shardingType ;
            if( clInfo['IsMainCL'] == true )
            {
               shardingType = $scope.autoLanguage( '垂直分区' ) ;
            }
            else
            {
               if( clInfo['ShardingType'] == 'range' )
               {
                  shardingType = $scope.autoLanguage( '水平范围分区' ) ;
               }
               else if( clInfo['ShardingType'] == 'hash' )
               {
                  shardingType = $scope.autoLanguage( '水平散列分区' ) ;
               }
               else
               {
                  shardingType = $scope.autoLanguage( '普 通' ) ;
               }
            }
            var fullName = clInfo['FullName'].split( '.' ) ;
            var csName = fullName[0] ;
            var clName = fullName[1] ;
            var newEle = $compile( '<a class="linkButton" ng-click="gotoRecord(' + index + ')"></a>' )( $scope ).text( clName ) ;
            var shardingTypeEle = $( '<span></span>' ).addClass( 'badge badge-info' ).text( shardingType ) ;
            gridData['body'].push( [
               { 'text': index + 1 },
               { 'text': csName },
               { 'html': newEle },
               { 'html': shardingTypeEle },
               { 'text': clInfo['MainCLName'] == null ? '' : clInfo['MainCLName'] },
               { 'text': clInfo['TotalRecords'] == null ? '' : clInfo['TotalRecords'] },
               { 'text': clInfo['TotalIndexes'] == null ? '' : clInfo['TotalIndexes'] }
            ] ) ;
         } ) ;
         $scope.clGridData = gridData ;
      }

      //获取集合列表
      $scope.getCLList = function()
      {
         var sql ;
         if( moduleMode == 'standalone' )
         {
            sql = 'SELECT T1.Name as FullName, T1.IsMainCL, T1.MainCLName, T1.ShardingType, T1.Details.TotalRecords as TotalRecords, T1.Details.Indexes AS TotalIndexes FROM (SELECT * FROM $SNAPSHOT_CL split BY Details) AS T1' ;
            //sql = 'select * from $SNAPSHOT_CL' ;
         }  
         else
         {
            sql = 'SELECT T4.Name AS FullName, T4.IsMainCL, T4.MainCLName, T4.ShardingType, T3.TotalRecords, T3.TotalIndexes FROM (SELECT T2.Name, sum(T2.SUM1) AS TotalRecords, sum(T2.SUM2) AS TotalIndexes FROM (SELECT T1.Details.TotalRecords AS SUM1, T1.Details.Indexes AS SUM2, T1.Name FROM (SELECT * FROM $SNAPSHOT_CL split BY Details) AS T1 GROUP BY T1.Details.GroupName) AS T2 GROUP BY T2.Name) AS T3 RIGHT OUTER JOIN $SNAPSHOT_CATA AS T4 ON T3.Name = T4.Name GROUP BY T4.Name' ;
         }
         //获取集合列表
         SdbRest.Exec( sql, function( data ){
            if( data.length == 1 && data[0]['FullName'] == null ) data = [] ;
            $scope.execRc = true ;
            clsInfo = data ;
            $scope.total = parseInt( clsInfo.length / limit ) ;
            if( clsInfo.length % limit > 0 )
            {
               ++$scope.total ;
            }
            $scope.current = 1 ;
            $scope.setCurrent = 1 ;
            showPage( 1 ) ;
            $scope.$apply() ;
         }, function( errorInfo ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 获取集合列表失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), moduleName, errorInfo['errno'], errorInfo['description'], errorInfo['detail']) ;
            $scope.execRc = false ;
            showPage( 1 ) ;
            $scope.$apply() ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }
      $scope.getCLList() ;
   } ) ;
}());