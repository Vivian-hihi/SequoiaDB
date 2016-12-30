(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.CPU.Index.Ctrl', function( $scope, $compile, $location, SdbRest, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.CpuList = [] ;
      var diskUsed = 0 ;
      var diskSize = 0 ;
      var chartDetail = {} ;
      var sumCpuOld = 0 ;
      var sumCpuOld1 = 0 ;
      var sumCpuOld2 = 0 ;
      var idleCpuOld = 0 ;
      var sumCpu = 0 ;
      var idleCpu = 0 ;
      //后期跳转到该页面时获取到的主机名
      var hostName = 'ubuntu-test-02' ;
      $scope.HostName = hostName ;
      var getCpuInfo = function(){
         var data = {
            'cmd': 'query host',
            'filter': JSON.stringify( { 'HostName': hostName } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( hostInfo ){
               $.each( hostInfo[0]['CPU'], function( index, cpuInfo ){
                  cpuInfo['Freq'] = parseFloat( cpuInfo['Freq'].substr(0,4) ) ;
                  //数据暂无，待补充
                  cpuInfo['L1Cache'] = '1MB' ;
                  cpuInfo['L2Cache'] = '2MB' ;
                  cpuInfo['L3Cache'] = '8MB' ;
                  //逻辑处理器
                  cpuInfo['Processor'] = 4 ;
                  //进程数
                  cpuInfo['Processes'] = 126 ;
                  $scope.CpuList.push( cpuInfo ) ;
               } ) ;
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getCpuInfo() ;
                  return true ;
               } ) ;
            }
         } ) ;
      } ;

      var getChartInfo = function(){
         var data = {
            'cmd':'query host status',
            'HostInfo': JSON.stringify( { "HostInfo": [ { 'HostName': hostName } ] } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( chartInfo ){
               diskUsed = 0 ;
               diskSize = 0 ;
               chartDetail = chartInfo[0]['HostInfo'][0] ;
               //CPU
               sumCpuOld1 = chartDetail['CPU']['Sys']['Megabit'] + chartDetail['CPU']['Idle']['Megabit'] + chartDetail['CPU']['Other']['Megabit'] + chartDetail['CPU']['User']['Megabit'] ;
               sumCpuOld2 = chartDetail['CPU']['Sys']['Unit'] + chartDetail['CPU']['Idle']['Unit'] + chartDetail['CPU']['Other']['Unit'] + chartDetail['CPU']['User']['Unit'] ;
               
               //总量
               sumCpuOld = sumCpuOld1 * 1024 * 1024 + sumCpuOld2 ;

               //总空闲
               idleCpuOld = chartDetail['CPU']['Idle']['Megabit'] * 1024 * 1024 + chartDetail['CPU']['Idle']['Unit'] ;

               if( sumCpu == 0 && idleCpu == 0 )
               {
                  $scope.charts['Cpu']['value'] = [ [ 0, 0, true, false ],[ 1, s, true, false   ] ] ;
                  sumCpu = sumCpuOld ;
                  idleCpu = idleCpuOld ;
               }
               else
               {

                  $scope.charts['Cpu']['value'] = [ [ 0, 100 - ( ( idleCpuOld - idleCpu ) / ( sumCpuOld - sumCpu ) ).toFixed(2) * 100, true, false ],[ 1, s, true, false ] ] ;

                  sumCpu = sumCpuOld ;
                  idleCpu = idleCpuOld ;
               }

               //alert(( idleCpuOld/sumCpuOld ).toFixed(2) * 100)
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
      getCpuInfo() ;
      getChartInfo() ;


      $scope.hostInfo = [] ;
      var s = 0 ;
      var d = 0 ;
      $scope.charts = {}; 
      $scope.charts['Cpu'] = {} ;
      $scope.charts['Cpu']['options'] = window.SdbSacManagerConf.CpuEchart ;

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            $scope.hostName = test[0]['HostName'] ;
            $scope.hostInfo = test[0]['CPU'] ;
         } ) ;
      }
      $scope.getInfo();

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