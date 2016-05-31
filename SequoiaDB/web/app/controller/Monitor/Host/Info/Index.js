(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Host.Performance.Index.Ctrl', function( $scope, SdbRest, $compile, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      $scope.hostInfo = [] ;
      var s = 0 ;
      var d = 0 ;
      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.hostInfo = test[0] ;
         } ) ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Storage']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Cpu']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*400) + 300 ;
            d = parseInt(Math.random()*300) + 300 ;
            $scope.charts['network']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*10) + 40 ;
            d = parseInt(Math.random()*10) + 30 ;
            $scope.charts['memory']['value'] = [ [ 0, s, true, false ],[ 1, d, true, false ] ] ;

         },2000)
      }
      $scope.getInfo();

      $scope.charts = {};
      $scope.charts['Storage'] = {} ;
      $scope.charts['Storage']['options'] = window.SdbSacManagerConf.DiskStorageEchart ;

      $scope.charts['network'] = {} ;
      $scope.charts['network']['options'] = window.SdbSacManagerConf.NetwordIOEchart ;

      $scope.charts['memory'] = {} ;
      $scope.charts['memory']['options'] = window.SdbSacManagerConf.MemoryEchart ;
      
      $scope.charts['Cpu'] = {} ;
      $scope.charts['Cpu']['options'] = window.SdbSacManagerConf.CpuEchart ;
      
      $scope.charts['Ram'] = {} ;
      $scope.charts['Ram']['options'] = window.SdbSacManagerConf.RamBarEchart ;
   } ) ;

}());