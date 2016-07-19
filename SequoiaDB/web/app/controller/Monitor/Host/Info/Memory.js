(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Memory.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;

      var memoryInfo = {} ;
      //后期跳转到该页面时获取到的主机名
      var hostName = 'ubuntu-test-02' ;
      $scope.hostName = hostName ;

      //图表配置
      $scope.charts = {}; 
      $scope.charts['Memory'] = {} ;
      $scope.charts['Memory']['options'] = window.SdbSacManagerConf.MemoryEchart ;

      $scope.charts['MemoryBar'] = {} ;
      $scope.charts['MemoryBar']['options'] = window.SdbSacManagerConf.MemoryLessEchart ;
      $scope.charts['MemoryBar']['value'] = [ [ 0, 15.54, true, false ], [ 1, 18.47, true, false ], [ 2, 31.80, true, false ], [ 3, 39.73, true, false ] ] ;

      //获取主机状态信息
      var getMemoryInfo = function(){
         var data = {
            'cmd':'query host status',
            'HostInfo': JSON.stringify( { "HostInfo": [ { 'HostName': hostName } ] } )
         } ;
         SdbFunction.Interval( function(){
            SdbRest.OmOperation( data, function( hostInfo ){
               memoryInfo = hostInfo[0]['HostInfo'][0]['Memory'] ;
               memoryPencent = ( memoryInfo['Used'] / memoryInfo['Size'] * 100 ).toFixed(2) ;
               $scope.charts['Memory']['value'] = [ [ 0, memoryPencent, true, false ],[ 1, 35, true, false ] ] ;
            }, function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getMemoryInfo() ;
                  return true ;
               } ) ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            }, null, false ) ;
         }, 5000 ) ;
         
      } ;
      getMemoryInfo() ;
   } ) ;

}());