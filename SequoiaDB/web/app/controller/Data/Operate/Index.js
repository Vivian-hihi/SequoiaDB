(function(){
   var sacApp = window.SdbSacManagerModule ;
   var GridId ;
   sacApp.controllerProvider.register( 'Data.Operate.Index.Ctrl', function( $scope, $compile, $location, SdbRest, InheritSize, SdbFunction ){
      var clusterName = SdbFunction.getLocalData( 'SdbClusterName' ) ;
      var moduleName = SdbFunction.getLocalData( 'SdbModuleName' ) ;
      var csName = SdbFunction.getLocalData( 'SdbCsName' ) ;
      var clName = SdbFunction.getLocalData( 'SdbClName' ) ;
      printfDebug( 'Cluster: ' + clusterName + ', Module: ' + moduleName + ', cs: ' + csName + ', cl: ' + clName ) ;
      if( clusterName == null )
      {
         $scope.selectCluster( function( clusterName ){
            SdbFunction.setLocalData( 'SdbClusterName', clusterName ) ;
            location.reload( false ) ;
         } ) ;
      }
      if( moduleName == null )
      {
         $scope.selcetModule( function( moduleName ){
            SdbFunction.setLocalData( 'SdbModuleName', moduleName ) ;
            location.reload( false ) ;
         } ) ;
      }
      
      //修正宽高
      InheritSize.append( $( '#OperateIndex' ) ) ;
      $( '#OperateIndex > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;

      var clsInfo = [] ;
      $scope.gotoRecord = function( listIndex ){
         var fullName = clsInfo[listIndex]['FullName'] ;
         var csName = fullName.split( '.' )[0] ;
         var clName = fullName.split( '.' )[1] ;
         printfDebug( 'cs: ' + csName + ', cl: ' + clName ) ;
         SdbFunction.setLocalData( 'SdbCsName', csName ) ;
         SdbFunction.setLocalData( 'SdbClName', clName ) ;
         $location.path( 'Data/Operate/Record' ) ;
      }
      $scope.getCLList = function()
      {
         //获取集合列表
         SdbRest.Exec( 'select T2.Name as FullName,T2.IsMainCL,T2.MainCLName,T2.ShardingKey,T2.ShardingType,T1.Details from $SNAPSHOT_CL as T1 right outer join $SNAPSHOT_CATA as T2 on T1.Name = T2.Name group by T2.Name', function( data ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 获取集合列表成功' ), timeFormat( new Date(), 'hh:mm:ss' ), moduleName ) ;
            $scope.execRc = true ;
            clsInfo = data ;
            var gridData = {
               'title': [],
               'body': [],
               'tool': {
                  'position': 'bottom',
                  'left': [
                     { 'html': $compile( '<i class="fa fa-refresh" ng-click="getCLList()"></i>' )( $scope ).attr( 'data-desc', $scope.autoLanguage( '刷新' ) ) },
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
               $scope.autoLanguage( '集合' ),
               $scope.autoLanguage( '分区类型' ),
               $scope.autoLanguage( '主集合' ),
               $scope.autoLanguage( '记录数' ),
               $scope.autoLanguage( '索引数' )
            ] ;
            $.each( gridTitle, function( index, titleText ){
               gridData['title'].push( { 'text': titleText } ) ;
            } ) ;
            $.each( clsInfo, function( index, clInfo ){
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
               var newEle = $compile( '<a class="linkButton" ng-click="gotoRecord(' + index + ')"></a>' )( $scope ).text( clInfo['FullName'] ) ;
               var shardingTypeEle = $( '<span></span>' ).addClass( 'badge badge-info' ).text( shardingType ) ;
               gridData['body'].push( [
                  { 'text': index + 1 },
                  { 'html': newEle },
                  { 'html': shardingTypeEle },
                  { 'text': clInfo['MainCLName'] == null ? '' : clInfo['MainCLName'] },
                  { 'text': clInfo['Details'] == null ? '' : clInfo['Details'][0]['TotalRecords'] },
                  { 'text': clInfo['Details'] == null ? '' : clInfo['Details'][0]['Indexes'] }
               ] ) ;
            } ) ;
            $scope.clGridData = gridData ;
            $scope.$apply() ;
         }, function( errorInfo ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 获取集合列表失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), moduleName, errorInfo['errno'], errorInfo['description'], errorInfo['detail']) ;
            $scope.execRc = false ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }
      $scope.getCLList() ;
   } ) ;
}());