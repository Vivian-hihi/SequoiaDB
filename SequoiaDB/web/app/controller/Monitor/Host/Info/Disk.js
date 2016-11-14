(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Disk.Index.Ctrl', function( $scope, $compile, $location, SdbRest, SdbFunction ){
      $scope.ClusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      $scope.ModuleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      $scope.ModuleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var hostName = SdbFunction.LocalData( 'SdbHostName' ) ;
      $scope.hostName = hostName ;
      
      $scope.hostInfo = [] ;
      var s = 0 ;
      var d = 0 ;


      var getDiskInfo = function(){
         var data = {
            'cmd':'query host status',
            'HostInfo': JSON.stringify( { "HostInfo": [ { 'HostName': hostName } ] } )
         } ;
         SdbRest.OmOperation( data, {
            'success': function( hostInfo ){
               $scope.hostInfo = hostInfo[0]['HostInfo'][0]['Disk'] ;
               $.each( $scope.hostInfo, function( index, diskInfo ){
                  $scope.hostInfo[index]['Used'] = ( ( diskInfo['Size'] - diskInfo['Free'] )/1024 ).toFixed(2) ;
                  $scope.hostInfo[index]['Size'] = ( diskInfo['Size']/1024 ).toFixed(2) ;
                  $scope.hostInfo[index]['Free'] = ( diskInfo['Free']/1024 ).toFixed(2) ;

               } ) ;
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  getDiskInfo() ;
                  return true ;
               } ) ;
            }
         }, {
            'showLoading': false,
            'delay': 5000,
            'loop': true
         } ) ;
         
      } ;
      getDiskInfo() ;



      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/hostInfo', success, failed, error ) ;
      }

      //虚拟数据
      $scope.getInfo = function(){
         $scope.queryList( {}, function( test ){
            //$scope.hostName = test[0]['HostName'] ;
            //$scope.hostInfo = test[0]['Disk'] ;
         } ) ;
         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*5) + 30 ;
            d = parseInt(Math.random()*5) + 40 ;
            $scope.charts['Storage']['value'] = [ [ 0, d, true, false ],[ 1, s, true, false ] ] ;

            s = parseInt(Math.random()*50) ;
            d = parseInt(Math.random()*50) ;
            $scope.charts['IO']['value'] = [ [ 0, 0, true, false ],[ 1, 0, true, false ] ] ;

         },2000)
      }
      $scope.getInfo();

      $scope.charts = {}; 
      $scope.charts['Storage'] = {} ;
      $scope.charts['Storage']['options'] = window.SdbSacManagerConf.DiskStorageEchart ;
      $scope.charts['IO'] = {} ;
      $scope.charts['IO']['options'] = window.SdbSacManagerConf.DiskIOEchart ;

      //跳转至资源
      $scope.GotoResource = function(){
         $location.path( '/Monitor/SDB-Resources/Domain' ) ;
      } ;

      //跳转至主机
      $scope.GotoHosts = function(){
         $location.path( '/Monitor/Host-List/Index' ) ;
      } ;
      
      
      //跳转至节点
      $scope.GotoNodes = function(){
         $location.path( '/Monitor/SDB-Nodes/Nodes' ) ;
      } ;
   } ) ;

}());