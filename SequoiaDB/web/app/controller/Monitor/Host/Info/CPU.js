(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.CPU.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      $scope.hostName = '' ;
      $scope.hostInfo = [] ;
      var s = 0 ;
      var d = 0 ;
      $scope.charts = {}; 
      $scope.charts['Cpu'] = {} ;
      $scope.charts['Cpu']['options'] = window.SdbSacManagerConf.CpuEchart ;

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.hostName = test[0]['HostName'] ;
            $scope.hostInfo = test[0]['CPU'] ;
         } ) ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Cpu']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;
         },2000)
      }
      $scope.getInfo();
   } ) ;

}());