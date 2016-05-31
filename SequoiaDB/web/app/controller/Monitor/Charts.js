(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.Charts.Index.Ctrl', function( $scope, SdbRest, SdbFunction ){
      $scope.charts = {}; 

      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;
      $scope.charts['Insert']['value'] = [ [ 0, 0, true, false ] ] ;

      $scope.charts['Update'] = {} ;
      $scope.charts['Update']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;
      $scope.charts['Update']['value'] = [ [ 0, 0, true, false ] ] ;

      $scope.charts['Delete'] = {} ;
      $scope.charts['Delete']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;
      $scope.charts['Delete']['value'] = [ [ 0, 0, true, false ] ] ;

      $scope.charts['Read'] = {} ;
      $scope.charts['Read']['options'] = window.SdbSacManagerConf.RecordReadEchart ;
      $scope.charts['Read']['value'] = [ [ 0, 0, true, false ] ] ;

   } ) ;
   //记录视图

}());