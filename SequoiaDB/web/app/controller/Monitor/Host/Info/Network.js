(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Network.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      $scope.hostName = '' ;
      $scope.hostInfo = [] ;

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.hostName = test[0]['HostName'] ;
            $scope.hostInfo = test[0]['Network'] ;
         } ) ;
         var s = 0 ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*10) + 30 ;
            $scope.charts['output']['value'] = [ [ 0, s, true, false ] ] ;
            s = parseInt(Math.random()*400) + 300 ;
            $scope.charts['input']['value'] = [ [ 0, s, true, false ] ] ;
            
         },2000)
      }
      $scope.getInfo();

      $scope.charts = {}; 
      $scope.charts['output'] = {} ;
      $scope.charts['output']['options'] = {} ;
      $.extend( true, $scope.charts['output']['options'], window.SdbSacManagerConf.NetwordEchart ) ;
      $scope.charts['output']['options']['title']['text'] = '已发送流量' ;

      $scope.charts['input'] = {} ;
      $scope.charts['input']['options'] = {} ;
      $.extend( true, $scope.charts['input']['options'], window.SdbSacManagerConf.NetwordEchart ) ;
      $scope.charts['input']['options']['title']['text'] = '已接收流量' ;
   } ) ;

}());