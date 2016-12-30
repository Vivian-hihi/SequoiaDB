(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbGroup.Charts.Ctrl', function( $scope, SdbRest, $location, SdbFunction ){
      
      _IndexPublic.checkMonitorEdition( $location ) ; //检测是不是企业版

      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;
      var groupName = SdbFunction.LocalData( 'SdbGroupName' ) ;
      if( clusterName == null || moduleType != 'sequoiadb' || moduleMode == null || moduleName == null )
      {
         $location.path( '/Transfer' ).search( { 'r': new Date().getTime() } ) ;
         return;
      }

      if( groupName == null || groupName == 'SYSCoord' )
      {
         $location.path( '/Monitor/SDB-Nodes/Groups' ).search( { 'r': new Date().getTime() } ) ;
         return;
      }

      //初始化
      $scope.charts = {}; 
      //组信息
      var GroupName = groupName ;
      
      var getChartInfo = function(){
         var chartInfo = { 'TotalInsert':0, 'TotalUpdate': 0, 'TotalDelete':0, 'TotalRead':0 } ;
         var SumInfo = {} ;
         var sql = 'SELECT TotalInsert, TotalRead, TotalDelete, TotalUpdate FROM $SNAPSHOT_DB WHERE GroupName="' + GroupName + '" AND NodeSelect = "master"' ;
         SdbRest.Exec( sql, {
            'success': function( dbInfo ){
               if( dbInfo )
               {
                  dbInfo = dbInfo[0] ;
                  if( typeof( SumInfo['TotalInsert'] ) == 'undefined' )
                  {
                     SumInfo['TotalInsert'] = dbInfo['TotalInsert'] ;
                     SumInfo['TotalUpdate'] = dbInfo['TotalUpdate'] ;
                     SumInfo['TotalDelete'] = dbInfo['TotalDelete'] ;
                     SumInfo['TotalRead'] = dbInfo['TotalRead'] ;
                  }
                  else
                  {
                     $scope.charts['Insert']['value'] = [ [ 0, ( dbInfo['TotalInsert'] - SumInfo['TotalInsert'] ) / 5, true, false ] ] ;
                     $scope.charts['Update']['value'] = [ [ 0, ( dbInfo['TotalUpdate'] - SumInfo['TotalUpdate'] ) / 5, true, false ] ] ;
                     $scope.charts['Delete']['value'] = [ [ 0, ( dbInfo['TotalDelete'] - SumInfo['TotalDelete'] ) / 5, true, false ] ] ;
                     $scope.charts['Read']['value'] = [ [ 0, ( dbInfo['TotalRead'] - SumInfo['TotalRead'] ) / 5, true, false ] ] ;

                     SumInfo['TotalInsert'] = dbInfo['TotalInsert'] ;
                     SumInfo['TotalUpdate'] = dbInfo['TotalUpdate'] ;
                     SumInfo['TotalDelete'] = dbInfo['TotalDelete'] ;
                     SumInfo['TotalRead'] = dbInfo['TotalRead'] ;
                  }
               }
            }
         }, {
            'showLoading': false,
            'delay': 5000,
            'loop': true
         } ) ;
      }

      getChartInfo() ;
      
      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;

      $scope.charts['Update'] = {} ;
      $scope.charts['Update']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;

      $scope.charts['Delete'] = {} ;
      $scope.charts['Delete']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;

      $scope.charts['Read'] = {} ;
      $scope.charts['Read']['options'] = window.SdbSacManagerConf.RecordReadEchart ;

      //跳转至部署
      $scope.GotoDeploy = function(){
         $location.path( '/Deploy/Index' ).search( { 'r': new Date().getTime() } ) ;
      } ;

      //跳转至监控主页
      $scope.GotoModule = function(){
         $location.path( '/Monitor/Index' ).search( { 'r': new Date().getTime() } ) ;
      } ;

      //跳转至分区组列表
      $scope.GotoGroups = function(){
         $location.path( '/Monitor/SDB-Nodes/Groups' ).search( { 'r': new Date().getTime() } ) ;
      } ;
       //跳转至资源
      $scope.GotoResources = function(){
         $location.path( '/Monitor/SDB-Resources/Session' ).search( { 'r': new Date().getTime() } ) ;
      } ;

      //跳转至主机列表
      $scope.GotoHosts = function(){
         $location.path( '/Monitor/SDB-Host/List/Index' ).search( { 'r': new Date().getTime() } ) ;
      } ;
      
      
      //跳转至节点列表
      $scope.GotoNodes = function(){
         $location.path( '/Monitor/SDB-Nodes/Nodes' ).search( { 'r': new Date().getTime() } ) ;
      } ;
   } ) ;

}());