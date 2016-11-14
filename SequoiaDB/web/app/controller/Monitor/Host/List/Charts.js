(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.HostPerformance.Index.Ctrl', function( $scope, $compile, $location, SdbRest, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var hostNameList = [] ;
      var cpuSum = 0 ;
      var memorySum = 0 ;
      var cpuUsed = 0 ;
      var memoryUsed = 0 ;
      var networkIn = 0 ;
      var networkOut = 0 ;
      var diskSum = 0 ;
      var diskFree = 0 ;
      var diskUsed = 0 ;
      var getHostList = function(){
         var data = {
            'cmd':'query host status',
            'HostInfo': JSON.stringify( {"HostInfo":hostNameList } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( hostList ){
               $.each( hostList[0]['HostInfo'], function( index, hostInfo ){
                  cpuSum =  hostInfo['CPU']['Sys']['Megabit'] +
                            hostInfo['CPU']['Idle']['Megabit'] +
                            hostInfo['CPU']['Other']['Megabit'] +
                            hostInfo['CPU']['User']['Megabit'] + cpuSum ;
                  memorySum += hostInfo['Memory']['Size'] ;
                  memoryUsed += hostInfo['Memory']['Used'] ;
                  $.each( hostInfo['Disk'], function( index2, diskInfo ){
                     diskSum += diskInfo['Size'] ;
                     diskFree += diskInfo['Free'] ;
                  } ) ;
               } ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getHostList() ;
                  return true ;
               } ) ;
            }
         },{
            'showLoading': false,
            'delay': 5000,
            'loop': true
         } ) ;
      } ;
      
      var getModuleInfo = function(){
         var data = {
            'cmd': 'query business',
            'filter' : JSON.stringify( { 'BusinessName': $scope.ModuleName } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( moduleInfo ){
               $.each( moduleInfo[0]['Location'], function( index, hostName ){
                  hostNameList.push(hostName)
               } ) ;
               getHostList() ;
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getModuleInfo() ;
                  return true ;
               } ) ;
            }
         } ) ;
      } ;
      getModuleInfo() ;



      $scope.charts = {}; 
      $scope.getData = function(){
         var s = 0 ;
         var d = 0 ;
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
            
            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['memory']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

         },2000)
      }
      $scope.getData() ;

      $scope.charts['Storage'] = {} ;
      $scope.charts['Storage']['options'] = window.SdbSacManagerConf.DiskStorageEchart ;

      $scope.charts['network'] = {} ;
      $scope.charts['network']['options'] = window.SdbSacManagerConf.NetwordIOEchart ;

      $scope.charts['memory'] = {} ;
      $scope.charts['memory']['options'] = window.SdbSacManagerConf.MemoryEchart ;

      $scope.charts['Cpu'] = {} ;
      $scope.charts['Cpu']['options'] = window.SdbSacManagerConf.CpuEchart ;


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