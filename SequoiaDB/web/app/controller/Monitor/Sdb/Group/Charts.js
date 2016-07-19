(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbGroup.Charts.Ctrl', function( $scope, SdbRest, $location, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      $scope.charts = {}; 
      //组信息
      var GroupName = 'group1' ;
      
      var sql = 'select * from $SNAPSHOT_DB where GroupName="' + GroupName + '" and NodeSelect = "master"' ;

      var getChartInfo = function(){
         var chartInfo = { 'TotalInsert':0, 'TotalUpdate': 0, 'TotalDelete':0, 'TotalRead':0 } ;
         var SumInfo = {} ;
         sql = 'SELECT TotalInsert, TotalRead, TotalDelete, TotalUpdate FROM $SNAPSHOT_DB WHERE GroupName="' + GroupName + '" AND NodeSelect = "master"' ;
         SdbFunction.Interval( function(){
            SdbRest.Exec( sql, function( DbInfo ){
               DbInfo = DbInfo[0] ;
               if( typeof( SumInfo['TotalInsert'] ) == 'undefined' )
               {
                  SumInfo['TotalInsert'] = DbInfo['TotalInsert'] ;
                  SumInfo['TotalUpdate'] = DbInfo['TotalUpdate'] ;
                  SumInfo['TotalDelete'] = DbInfo['TotalDelete'] ;
                  SumInfo['TotalRead'] = DbInfo['TotalRead'] ;
               }
               else
               {
                  $scope.charts['Insert']['value'] = [ [ 0, ( DbInfo['TotalInsert'] - SumInfo['TotalInsert'] )/5, true, false ] ] ;
                  $scope.charts['Update']['value'] = [ [ 0, ( DbInfo['TotalUpdate'] - SumInfo['TotalUpdate'] )/5, true, false ] ] ;
                  $scope.charts['Delete']['value'] = [ [ 0, ( DbInfo['TotalDelete'] - SumInfo['TotalDelete'] )/5, true, false ] ] ;
                  $scope.charts['Read']['value'] = [ [ 0, ( DbInfo['TotalRead'] - SumInfo['TotalRead'] )/5, true, false ] ] ;

                  SumInfo['TotalInsert'] = DbInfo['TotalInsert'] ;
                  SumInfo['TotalUpdate'] = DbInfo['TotalUpdate'] ;
                  SumInfo['TotalDelete'] = DbInfo['TotalDelete'] ;
                  SumInfo['TotalRead'] = DbInfo['TotalRead'] ;
               }

            }, function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getDbInfo() ;
                  return true ;
               } ) ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            }, null, false ) ;
         }, 2000 ) ;
      } ;
      
      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;

      $scope.charts['Update'] = {} ;
      $scope.charts['Update']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;

      $scope.charts['Delete'] = {} ;
      $scope.charts['Delete']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;

      $scope.charts['Read'] = {} ;
      $scope.charts['Read']['options'] = window.SdbSacManagerConf.RecordReadEchart ;
      getChartInfo() ;

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