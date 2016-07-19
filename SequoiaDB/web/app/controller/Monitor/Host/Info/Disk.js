(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Disk.Index.Ctrl', function( $scope, $compile, $location, SdbRest, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      
      $scope.hostInfo = [] ;
      var s = 0 ;
      var d = 0 ;

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.hostName = test[0]['HostName'] ;
            $scope.hostInfo = test[0]['Disk'] ;
         } ) ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Storage']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*50) ;
            d = parseInt(Math.random()*50) ;
            $scope.charts['IO']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

         },2000)
      }
      $scope.getInfo();

      $scope.charts = {}; 
      $scope.charts['Storage'] = {} ;
      $scope.charts['Storage']['options'] = window.SdbSacManagerConf.DiskStorageEchart ;
      $scope.charts['IO'] = {} ;
      $scope.charts['IO']['options'] = window.SdbSacManagerConf.DiskIOEchart ;
   } ) ;

}());