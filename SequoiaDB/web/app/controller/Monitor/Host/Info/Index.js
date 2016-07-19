(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Host.Performance.Index.Ctrl', function( $scope, SdbRest, $location, $compile, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.hostInfo = [] ;
      $scope.NodeList = {'coord':[],'catalog':[],'data':[]} ;
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
      var hostName = 'ubuntu-test-02' ;

      var getNodeList = function(){
         var data = {
            'cmd': 'list nodes',
            'BusinessName': $scope.ModuleName
         } ;
         SdbRest.OmOperation( data, function( nodeList ){
            $.each( nodeList, function( index, nodeInfo ){
               if( nodeInfo['HostName'] == hostName )
               {
                  if( nodeInfo['Role'] == 'coord' )
                  {                  
                     $scope.NodeList['coord'].push( nodeInfo['HostName'] + ':' + nodeInfo['ServiceName'] ) ;
                  }
                  else if( nodeInfo['Role'] == 'catalog' )
                  {                  
                     $scope.NodeList['catalog'].push( nodeInfo['HostName'] + ':' + nodeInfo['ServiceName'] ) ;
                  }
                   else if( nodeInfo['Role'] == 'data' )
                  {                  
                     $scope.NodeList['data'].push( nodeInfo['HostName'] + ':' + nodeInfo['ServiceName'] ) ;
                  }
               }
            } ) ;

         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               getNodeList() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      } ;
      

      var getHostInfo = function(){
         var data = {
            'cmd': 'query host',
            'filter': JSON.stringify( { 'HostName': hostName } )
         } ;
         SdbRest.OmOperation( data, function( hostInfo ){
            $.each( hostInfo[0]['Disk'], function( index3, diskInfo ){
               diskSize += diskInfo['Size'] ;
            } ) ;
            hostInfo[0]['DiskSize'] = (  diskSize/1024 ).toFixed(2) + 'GB' ;
            hostInfo[0]['MemorySize'] = (  hostInfo[0]['Memory']['Size']/1024 ).toFixed(2) + 'GB' ;
            $scope.hostInfo = hostInfo[0]  ;
            getChartInfo() ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               getHostInfo() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      
      var getChartInfo = function(){
         var data = {
            'cmd': 'query host status',
            'HostInfo': JSON.stringify( { "HostInfo": [ { 'HostName': hostName } ] } )
         } ;
         SdbFunction.Interval( function(){
            SdbRest.OmOperation( data, function( chartInfo ){
               diskUsed = 0 ;
               diskSize = 0 ;
               chartDetail = chartInfo[0]['HostInfo'][0] ;
               //当前磁盘占用率
               $.each( chartDetail['Disk'], function( index, diskInfo ){
                  diskUsed += diskInfo['Size'] - diskInfo['Free'] ;
                  diskSize += diskInfo['Size'] ;
               } ) ;
               diskPercent = ( ( diskUsed/diskSize )*100 ).toFixed(2) ;
               //网卡
               $.each( chartDetail['Net']['Net'], function( index, netInfo){
                  networkIn += netInfo['RXBytes']['Megabit'] * 1024  + netInfo['RXBytes']['Unit'] / 1024 ;
                  networkOut += netInfo['TXBytes']['Megabit'] * 1024  + netInfo['RXBytes']['Unit'] / 1024 ;
               } ) ;

               //CPU
               sumCpu1 = chartDetail['CPU']['Sys']['Megabit'] + chartDetail['CPU']['Idle']['Megabit'] + chartDetail['CPU']['Other']['Megabit'] + chartDetail['CPU']['User']['Megabit'] ;
               sumCpu2 = chartDetail['CPU']['Sys']['Unit'] + chartDetail['CPU']['Idle']['Unit'] + chartDetail['CPU']['Other']['Unit'] + chartDetail['CPU']['User']['Unit'] ;
            
               //当前内存占用率

               memoryPencent = ( chartDetail['Memory']['Used'] / chartDetail['Memory']['Size'] * 100 ).toFixed(2) ;
               $scope.charts['Storage']['value'] = [ [ 0, diskPercent, true, false ],[ 1, 10, true, false ] ] ;
               $scope.charts['memory']['value'] = [ [ 0, memoryPencent, true, false ],[ 1, 10, true, false ] ] ;
               
            
            }, function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getChartInfo() ;
                  return true ;
               } ) ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            }, null, false ) ;
         }, 2000 )
         
      } ;
      getHostInfo();
      getNodeList() ;

      $scope.getInfo = function(){
         SdbFunction.Interval(function(){

            s = parseInt(Math.random()*10) + 30 ;
            d = parseInt(Math.random()*10) + 40 ;
            $scope.charts['Cpu']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*400) + 300 ;
            d = parseInt(Math.random()*300) + 300 ;
            $scope.charts['network']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;
         },2000)
      }
      $scope.getInfo();

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
      $scope.GotoNode = function(){
         $location.path( '/Monitor/SDB-Node/Index' ) ;
      } ;

   } ) ;
}());