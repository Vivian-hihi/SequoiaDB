(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Host.Performance.Index.Ctrl', function( $scope, SdbRest, $location, $compile, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.HostName = SdbFunction.LocalData( 'SdbHostName' ) ;
      $scope.NodeList = {'coord':[],'catalog':[],'data':[]} ;
      $scope.hostInfo = [] ;
      var diskSize = 0 ;
      var diskUsed = 0 ;
      var diskPercent = 0 ;
      var networkIn = 0 ;
      var networkOut = 0 ;
      var chartDetail = {} ;
      var sumCpu = 0 ;
      var s = 0 ;
      var d = 0 ;

      //后期跳转到该页面时获取到的主机名
      var hostName = $scope.HostName ;

      var getNodeList = function(){
         var data = {
            'cmd': 'list nodes',
            'BusinessName': $scope.ModuleName
         } ;
         SdbRest.OmOperation( data, {
            'success': function( nodeList ){
               $.each( nodeList, function( index, nodeInfo ){
                  if( nodeInfo['HostName'] == hostName )
                  {
                     $scope.NodeList[ nodeInfo['Role'] ].push( { 'hostName': nodeInfo['HostName'], 'serviceName': nodeInfo['ServiceName'] } ) ;
                  }
               } ) ;

            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getNodeList() ;
                  return true ;
               } ) ;
            }
         } ) ;
      } ;
      
      var getChartInfo = function(){
         var data = {
            'cmd': 'query host status',
            'HostInfo': JSON.stringify( { "HostInfo": [ { 'HostName': hostName } ] } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( chartInfo ){
               diskUsed = 0 ;
               diskSize = 0 ;
               chartDetail = chartInfo[0]['HostInfo'][0] ;
               //当前磁盘占用率
               if( isArray( chartDetail['Disk'] ) )
               {
                  $.each( chartDetail['Disk'], function( index, diskInfo ){
                     diskUsed += diskInfo['Size'] - diskInfo['Free'] ;
                     diskSize += diskInfo['Size'] ;
                  } ) ;
               }
               diskPercent = ( ( diskUsed/diskSize )*100 ).toFixed(2) ;
               //网卡
               if( isArray( chartDetail['Net']['Net'] ) )
               {
                  $.each( chartDetail['Net']['Net'], function( index, netInfo){
                     networkIn += netInfo['RXBytes']['Megabit'] * 1024  + netInfo['RXBytes']['Unit'] / 1024 ;
                     networkOut += netInfo['TXBytes']['Megabit'] * 1024  + netInfo['RXBytes']['Unit'] / 1024 ;
                  } ) ;
               }
               

               //CPU
               sumCpu1 = chartDetail['CPU']['Sys']['Megabit'] + chartDetail['CPU']['Idle']['Megabit'] + chartDetail['CPU']['Other']['Megabit'] + chartDetail['CPU']['User']['Megabit'] ;
               sumCpu2 = chartDetail['CPU']['Sys']['Unit'] + chartDetail['CPU']['Idle']['Unit'] + chartDetail['CPU']['Other']['Unit'] + chartDetail['CPU']['User']['Unit'] ;
            
               //当前内存占用率

               memoryPencent = ( chartDetail['Memory']['Used'] / chartDetail['Memory']['Size'] * 100 ).toFixed(2) ;
               $scope.charts['Storage']['value'] = [ [ 0, diskPercent, true, false ],[ 1, 10, true, false ] ] ;
               $scope.charts['memory']['value'] = [ [ 0, memoryPencent, true, false ],[ 1, 10, true, false ] ] ;

               //虚拟数据
               s = parseInt(Math.random()*10) + 30 ;
               d = parseInt(Math.random()*10) + 40 ;
               $scope.charts['Cpu']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

               s = parseInt(Math.random()*30) + 200 ;
               d = parseInt(Math.random()*30) + 300 ;
               $scope.charts['network']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;
            
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getChartInfo() ;
                  return true ;
               } ) ;
            }
         }, {
            'showLoading': false,
            'delay': 5000,
            'loop': true
         } ) ;
         
      } ;

      var getHostInfo = function(){
         var data = {
            'cmd': 'query host',
            'filter': JSON.stringify( { 'HostName': hostName } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( hostInfo ){
               if( isArray( hostInfo[0]['Disk'] ) )
               {
                  $.each( hostInfo[0]['Disk'], function( index3, diskInfo ){
                     diskSize += diskInfo['Size'] ;
                  } ) ;
                  hostInfo[0]['DiskSize'] = (  diskSize/1024 ).toFixed(2) + 'GB' ;
                  hostInfo[0]['MemorySize'] = (  hostInfo[0]['Memory']['Size']/1024 ).toFixed(2) + 'GB' ;
                  $scope.hostInfo = hostInfo[0]  ;
                  getChartInfo() ;
               }
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getHostInfo() ;
                  return true ;
               } ) ;
            } 
         } ) ;
      }

      
      
      getHostInfo();
      getNodeList() ;

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

      //跳转至节点信息页面
      $scope.GotoNode = function( serviceName ){
         SdbFunction.LocalData( 'SdbServiceName', serviceName ) ;
         $location.path( '/Monitor/SDB-Node/Index' ) ;
      } ;

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