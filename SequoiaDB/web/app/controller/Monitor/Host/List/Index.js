(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.HostList.Index.Ctrl', function( $scope, SdbRest, $location, $compile, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.SearchName = {} ;
      var hostNameList = [] ;
      var DiskSize = 0 ;
      $scope.NewHostList = [] ;

      //新表格
      $scope.HostGridOptions = { 'titleWidth': [ 12,12,12,12,17,23,5] } ;

      //渲染网格显示的列


      $scope.HostListOptions = {
         'titleWidth': [ 15, 15, 15, 15, 15, 15, 10 ]
      } ;
      var gridData = {
         'title': [ 
            { "text": $scope.autoLanguage( '主机名' ) },
            { "text": $scope.autoLanguage( 'IP地址' )  },
            { "text": $scope.autoLanguage( '磁盘' ) },
            { "text": $scope.autoLanguage( '内存' ) },
            { "text": $scope.autoLanguage( '操作系统' ) },
            { "text": $scope.autoLanguage( 'CPU' ) },
            { "text": $scope.autoLanguage( '网卡数' ) }
         ],
         'body': [],
         'tool': {
            'position': 'bottom',
            'left': [ { 'text': '一共有 40 台主机' } ],
            'right': [ ]
         },
         'options': {
            'grid': {  'tdModel': 'fixed', 'gridModel': 'fixed', 'tdHeight': '19px', 'titleWidth': [ 15, 15, 15, 15, 15, 15, 15] }
         }
      } ;
      $scope.GridData = $.extend( true, {}, gridData ) ;

      
      var getHostList = function(){
         var data = {
            'cmd': 'query host'
         } ;
         SdbRest.OmOperation( data, function( hostList ){
            $.each( hostNameList, function( index, hostName ){
               $.each( hostList, function( index2, hostInfo ){
                  if( hostName == hostInfo['HostName'] )
                  {
                     DiskSize = 0 ;
                     $.each( hostInfo['Disk'], function( index3, diskInfo ){
                        DiskSize += diskInfo['Size'] ;
                     } ) ;
                     hostInfo['DiskSize'] = (  DiskSize/1024 ).toFixed(2) + 'GB' ;
                     hostInfo['MemorySize'] = (  hostInfo['Memory']['Size']/1024 ).toFixed(2) + 'GB' ;
                     $scope.NewHostList.push( hostInfo ) ;
                  }
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
               hostNameList.push(hostName['HostName'])
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
      
      $scope.queryList = function( data, success, failed, error, complete ){
         SdbRest._postTest( './test/hostList', success, failed, error ) ;
      }
      $scope.getList = function(){
         $scope.queryList( {}, function( test ){
            $scope.HostList = test ;
            $scope.HostGridTool = sprintf( $scope.autoLanguage( '一共?台主机' ), $scope.HostList.length ) ;
            $scope.bindResize() ;
         } ) ;
      }
      $scope.getList() ;
      
      $scope.change = function(){
         $.each( $scope.HostList, function( index, value ){
            if( $scope.SearchName['Host'].length > 0 && value['HostName'].indexOf( $scope.SearchName['Host'] ) < 0 )
            {
               $scope.HostList[index]['show'] = false ;
            }
            else
            {
               $scope.HostList[index]['show'] = true ;
            }
         } ) 
      }
      //跳转事件
      $scope.GotoHost = function(){
         $location.path( '/Monitor/Host-Info/Index' ) ;
      } ;

      $scope.GotoDisk = function(){
         $location.path( '/Monitor/Host-Info/Disk' ) ;
      } ;

      $scope.GotoNet = function(){
         $location.path( '/Monitor/Host-Info/Network' ) ;
      } ;

      $scope.GotoCPU = function(){
         $location.path( '/Monitor/Host-Info/CPU' ) ;
      } ;

      $scope.GotoMemory = function(){
         $location.path( '/Monitor/Host-Info/Memory' ) ;
      } ;
   } ) ;
}());