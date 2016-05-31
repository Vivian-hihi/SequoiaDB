(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Memory.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      $scope.hostName = '' ;
      $scope.hostInfo = [] ;
      
      //测试用
      var s = 0 ;
      var d = 0 ;

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      $scope.charts = {}; 
      $scope.charts['Memory'] = {} ;
      $scope.charts['Memory']['options'] = window.SdbSacManagerConf.MemoryEchart ;

      $scope.charts['MemoryBar'] = {} ;
      $scope.charts['MemoryBar']['options'] = window.SdbSacManagerConf.MemoryLessEchart ;
      $scope.charts['MemoryBar']['value'] = [ [ 0, 15.54, true, false ], [ 1, 18.47, true, false ], [ 2, 31.80, true, false ], [ 3, 39.73, true, false ] ] ;


      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.hostName = test[0]['HostName'] ;
         } ) ;

         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Memory']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;
         },2000)
      }
      $scope.getInfo();
   } ) ;

}());