(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Charts.Ctrl', function( $scope, $location, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      $scope.charts = {} ; 
      $scope.DbInfo = {} ;

      //获取数据库快照
      var getDbList = function(){
         var sql = '' ;
         var SumInfo = {} ;
         //var SumInfo = { 'TotalInsert':0, 'TotalUpdate': 0, 'TotalDelete':0, 'TotalRead':0 } ;
         
         
         //只获取主节点
         sql = 'select TotalInsert, TotalUpdate, TotalDelete, TotalRead from $SNAPSHOT_DB where NodeSelect="master"';
         SdbFunction.Interval( function(){
            $scope.DbInfo = { 'TotalInsert':0, 'TotalUpdate': 0, 'TotalDelete':0, 'TotalRead':0 } ;
           
            SdbRest.Exec( sql, function( DbList ){
               $scope.DbList = DbList ;
               $.each( DbList, function( index, DbInfo ){
                  $scope.DbInfo['TotalInsert'] += DbInfo['TotalInsert'] ;
                  $scope.DbInfo['TotalUpdate'] += DbInfo['TotalUpdate'] ;
                  $scope.DbInfo['TotalDelete'] += DbInfo['TotalDelete'] ;
                  $scope.DbInfo['TotalRead'] += DbInfo['TotalRead'] ;
               } ) ;

               if( typeof( SumInfo['TotalInsert'] ) == 'undefined' )
               {
                  SumInfo['TotalInsert'] = $scope.DbInfo['TotalInsert'] ;
                  SumInfo['TotalUpdate'] = $scope.DbInfo['TotalUpdate'] ;
                  SumInfo['TotalDelete'] = $scope.DbInfo['TotalDelete'] ;
                  SumInfo['TotalRead'] = $scope.DbInfo['TotalRead'] ;
               }
               else
               {
                  $scope.charts['Insert']['value'] = [ [ 0, ( $scope.DbInfo['TotalInsert'] - SumInfo['TotalInsert'] )/5, true, false ] ] ;
                  $scope.charts['Update']['value'] = [ [ 0, ( $scope.DbInfo['TotalUpdate'] - SumInfo['TotalUpdate'] )/5, true, false ] ] ;
                  $scope.charts['Delete']['value'] = [ [ 0, ( $scope.DbInfo['TotalDelete'] - SumInfo['TotalDelete'] )/5, true, false ] ] ;
                  $scope.charts['Query']['value'] = [ [ 0, ( $scope.DbInfo['TotalRead'] - SumInfo['TotalRead'] )/5, true, false ] ] ;

                  SumInfo['TotalInsert'] = $scope.DbInfo['TotalInsert'] ;
                  SumInfo['TotalUpdate'] = $scope.DbInfo['TotalUpdate'] ;
                  SumInfo['TotalDelete'] = $scope.DbInfo['TotalDelete'] ;
                  SumInfo['TotalRead'] = $scope.DbInfo['TotalRead'] ;
               }
               
            }, function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getDbList() ;
                  return true ;
               } ) ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            }, null, false ) ;
            

         },5000 ) ;
      } ;

      getDbList() ;

      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;

      $scope.charts['Update'] = {} ;
      $scope.charts['Update']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;

      $scope.charts['Delete'] = {} ;
      $scope.charts['Delete']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;

      $scope.charts['Query'] = {} ;
      $scope.charts['Query']['options'] = window.SdbSacManagerConf.RecordReadEchart ;

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