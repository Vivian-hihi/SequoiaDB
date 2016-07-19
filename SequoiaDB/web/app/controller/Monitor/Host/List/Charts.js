(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.HostPerformance.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
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
         SdbRest.OmOperation( data, function( hostList ){
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
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               getHostList() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      } ;
      
      var getModuleInfo = function(){
         var data = {
            'cmd': 'query business',
            'filter' : JSON.stringify( { 'BusinessName': $scope.ModuleName } )
         } ;
         SdbRest.OmOperation( data, function( moduleInfo ){
            $.each( moduleInfo[0]['Location'], function( index, hostName ){
               hostNameList.push(hostName)
            } ) ;
            getHostList() ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               getModuleInfo() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      } ;
      getModuleInfo() ;



      $scope.charts = {}; 
      $scope.getData = function(){
         var s = 0 ;
         var d = 0 ;
         SdbFunction.Timeout(function(){
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

      $scope.hostList = {} ;
      var gridData = {
            'title': [ 
               { "text": $scope.autoLanguage( '主机名' ) },
               { "text": $scope.autoLanguage( 'IP地址' ) } ,
               { "text": $scope.autoLanguage( '操作系统' )},
               { "text": $scope.autoLanguage( '内存' ) } ,
               { "text": $scope.autoLanguage( '磁盘' ) } ,
               { "text": $scope.autoLanguage( 'CPU' ) } ,
               { "text": $scope.autoLanguage( '网卡数' ) }
            ],
            'body': [],
            'tool': {
               'position': 'bottom',
               'left': [ { 'text': '' } ],
               'right': [ ]
            },
            'options': {
               'grid': {  'tdModel': 'fixed', 'gridModel': 'fixed', 'tdHeight': '19px', 'titleWidth': [ 15, 15, 15, 15, 15, 15, 15] }
            }
         } ;
      for( i=1, index = 201; index < 222 ; index++, i++)
      {
         gridData['body'].push( [
            { 'html': $compile( '<a href="#/Monitor/Host/Index" class="linkButton">Ubuntu-1' + index + '</a>')( $scope ) },
            { 'text': '192.168.1.' + index },
            { 'text': 'Ubuntu' },
            { 'text': '3451MB/8192MB' },
            { 'text': '41GB/120GB' },
            { 'text': '32%' },
            { 'text': '4' }
         ] )
         $scope.i = i ;
      } ;
      $scope.GridData = gridData ;
   } ) ;
   //记录视图

}());