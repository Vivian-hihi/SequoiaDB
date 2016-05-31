(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbNode.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      var s = 0 ;
      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/nodeInfo', success, failed, error ) ;
      }
      $scope.getGroupInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.nodeInfo = test[0] ;
         } )
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*2500) ;
            $scope.charts['Insert']['value'] = [ [ 0, s, true, false ] ] ;

            s = parseInt(Math.random()*2500) ;
            $scope.charts['Update']['value'] = [ [ 0, s, true, false ] ] ;

            s = parseInt(Math.random()*2500) ;
            $scope.charts['Delete']['value'] = [ [ 0, s, true, false ] ] ;

            s = parseInt(Math.random()*2500) ;
            $scope.charts['Query']['value'] = [ [ 0, s, true, false ] ] ;

         },3000)
      }
      $scope.getGroupInfo() ;

      $scope.charts = {};
      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;

      $scope.charts['Update'] = {} ;
      $scope.charts['Update']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;

      $scope.charts['Delete'] = {} ;
      $scope.charts['Delete']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;

      $scope.charts['Query'] = {} ;
      $scope.charts['Query']['options'] = window.SdbSacManagerConf.RecordReadEchart ;
   } ) ;
}());