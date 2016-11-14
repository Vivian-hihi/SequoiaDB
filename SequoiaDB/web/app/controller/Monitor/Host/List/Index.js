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

     
     

      
      var getHostList = function(){
         var data = {
            'cmd': 'query host'
         } ;
         SdbRest.OmOperation( data, {
            'success': function( hostList ){
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
            },
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getHostList() ;
                  return true ;
               } ) ;
            }
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
                  hostNameList.push(hostName['HostName'])
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
      
      //$scope.change = function(){
      //   $.each( $scope.HostList, function( index, value ){
      //      if( $scope.SearchName['Host'].length > 0 && value['HostName'].indexOf( $scope.SearchName['Host'] ) < 0 )
      //      {
      //         $scope.HostList[index]['show'] = false ;
      //      }
      //      else
      //      {
      //         $scope.HostList[index]['show'] = true ;
      //      }
      //   } ) 
      //}

      //跳转事件
      $scope.GotoHost = function( HostName ){
         SdbFunction.LocalData( 'SdbHostName', HostName ) ;
         $location.path( '/Monitor/Host-Info/Index' ) ;
      } ;

      $scope.GotoDisk = function( HostName ){
         SdbFunction.LocalData( 'SdbHostName', HostName ) ;
         $location.path( '/Monitor/Host-Info/Disk' ) ;
      } ;

      $scope.GotoNet = function( HostName ){
         SdbFunction.LocalData( 'SdbHostName', HostName ) ;
         $location.path( '/Monitor/Host-Info/Network' ) ;
      } ;

      $scope.GotoCPU = function( HostName ){
         SdbFunction.LocalData( 'SdbHostName', HostName ) ;
         $location.path( '/Monitor/Host-Info/CPU' ) ;
      } ;

      $scope.GotoMemory = function( HostName ){
         SdbFunction.LocalData( 'SdbHostName', HostName ) ;
         $location.path( '/Monitor/Host-Info/Memory' ) ;
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