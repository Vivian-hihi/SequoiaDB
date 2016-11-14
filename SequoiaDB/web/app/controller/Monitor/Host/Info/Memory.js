(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Memory.Index.Ctrl', function( $scope, $compile, $location, SdbRest, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var hostName = SdbFunction.LocalData( 'SdbHostName' ) ;
      var memoryInfo = {} ;
      //后期跳转到该页面时获取到的主机名
      $scope.hostName = hostName ;

      //图表配置
      $scope.charts = {}; 
      $scope.charts['Memory'] = {} ;
      $scope.charts['Memory']['options'] = window.SdbSacManagerConf.MemoryEchart ;
      $scope.charts['Memory']['value'] = [ [ 0, 0, true, false ], [ 0, 0, true, false ] ] ;

      $scope.charts['MemoryBar'] = {} ;
      $scope.charts['MemoryBar']['options'] = window.SdbSacManagerConf.MemoryLessEchart ;
      $scope.charts['MemoryBar']['value'] = [ [ 0, 15.54, true, false ], [ 1, 18.47, true, false ], [ 2, 31.80, true, false ], [ 3, 39.73, true, false ] ] ;

      //获取主机状态信息
      var getMemoryInfo = function(){
         var data = {
            'cmd':'query host status',
            'HostInfo': JSON.stringify( { "HostInfo": [ { 'HostName': hostName } ] } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( hostInfo ){
               memoryInfo = hostInfo[0]['HostInfo'][0]['Memory'] ;
               memoryPencent = ( memoryInfo['Used'] / memoryInfo['Size'] * 100 ).toFixed(2) ;
               $scope.charts['Memory']['value'] = [ [ 0, memoryPencent, true, false ] ] ;
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getMemoryInfo() ;
                  return true ;
               } ) ;
            }
         }, {
            'showLoading': false,
            'delay': 5000,
            'loop': true
         } ) ;
         
      } ;
      getMemoryInfo() ;

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