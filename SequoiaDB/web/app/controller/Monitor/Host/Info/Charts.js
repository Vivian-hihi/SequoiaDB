(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Charts.Index.Ctrl', function( $scope, SdbRest, $location, SdbFunction ){
      var ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var HostName = SdbFunction.LocalData( 'SdbHostName' ) ;
      $scope.ClusterName = ClusterName ;
      $scope.ModuleName = ModuleName ;
      $scope.ModuleType = ModuleType ;
      $scope.HostName = HostName ;

      $scope.charts = {};
      $scope.charts['Storage'] = {} ;
      $scope.charts['Storage']['options'] = window.SdbSacManagerConf.DiskStorageEchart ;

      $scope.charts['network'] = {} ;
      $scope.charts['network']['options'] = window.SdbSacManagerConf.NetwordIOEchart ;

      $scope.charts['memory'] = {} ;
      $scope.charts['memory']['options'] = window.SdbSacManagerConf.MemoryEchart ;

      $scope.charts['Cpu'] = {} ;
      $scope.charts['Cpu']['options'] = window.SdbSacManagerConf.CpuEchart ;

      $scope.getData = function(){
         var s = 0 ;
         var d = 0 ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Storage']['value'] = [ [ 0, 40, true, false ],[ 1, 30, true, false ] ] ;
            
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Cpu']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*20) + 200 ;
            d = parseInt(Math.random()*5) + 200 ;
            $scope.charts['network']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;
            
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['memory']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

         },5000)
      }
      $scope.getData() ;
      
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
   } ) ;

}());