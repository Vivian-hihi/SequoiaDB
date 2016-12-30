(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.NodesCharts.Ctrl', function( $scope, $location, SdbRest, SdbFunction ){
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
         $scope.DbInfo = { 'TotalInsert':0, 'TotalUpdate': 0, 'TotalDelete':0, 'TotalRead':0 } ;
           
         SdbRest.Exec( sql, {
            'success': function( DbInfo ){
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
                  $scope.charts['Query']['value'] = [ [ 0, ( DbInfo['TotalRead'] - SumInfo['TotalRead'] )/5, true, false ] ] ;

                  SumInfo['TotalInsert'] = DbInfo['TotalInsert'] ;
                  SumInfo['TotalUpdate'] = DbInfo['TotalUpdate'] ;
                  SumInfo['TotalDelete'] = DbInfo['TotalDelete'] ;
                  SumInfo['TotalRead'] = DbInfo['TotalRead'] ;
               }

            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getDbList() ;
                  return true ;
               } ) ;
            }
         },{
            'showLoading': false,
            'delay': 5000,
            'loop': true
         } ) ;
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

      //跳转至资源
      $scope.GotoResource = function(){
         $location.path( '/Monitor/SDB-Resources/Domain' ) ;
      } ;

      //跳转至主机列表
      $scope.GotoHosts = function(){
         $location.path( '/Monitor/Host-List/Index' ) ;
      } ;
      
      
      //跳转至节点列表
      $scope.GotoNodes = function(){
         $location.path( '/Monitor/SDB-Nodes/Nodes' ) ;
      } ;

      //跳转至监控主页
      $scope.GotoHosts = function(){
         $location.path( '/Monitor/Host-List/Index' ) ;
      } ;

      //跳转至分区组列表
      $scope.GotoGroups = function(){
         $location.path( '/Monitor/SDB-Nodes/Groups' ) ;
      } ;
   } ) ;

}());