(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Charts.Ctrl', function( $scope, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      $scope.charts = {}; 
      $scope.getData = function(){
         var s = 0 ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*2500) ;
            $scope.charts['Insert']['value'] = [ [ 0, s, true, false ] ] ;

            s = parseInt(Math.random()*2500) ;
            $scope.charts['Update']['value'] = [ [ 0, s, true, false ] ] ;

            s = parseInt(Math.random()*2500) ;
            $scope.charts['Delete']['value'] = [ [ 0, s, true, false ] ] ;

            s = parseInt(Math.random()*2500) ;
            $scope.charts['Query']['value'] = [ [ 0, s, true, false ] ] ;

         },5000)
      }
      $scope.getData() ;
      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;

      $scope.charts['Update'] = {} ;
      $scope.charts['Update']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;

      $scope.charts['Delete'] = {} ;
      $scope.charts['Delete']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;

      $scope.charts['Query'] = {} ;
      $scope.charts['Query']['options'] = window.SdbSacManagerConf.RecordReadEchart ;

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