(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.Preview.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
     
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;

      //初始化
      $scope.charts = {}; 
      $scope.charts['Host'] = {} ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleInfo = {} ;
      $scope.hostInfo = {} ;
      $scope.HostList = [] ;
      $scope.charts['Module'] = {} ;
      $scope.chartName = 'Record Insert' ;
      $scope.charts['Module']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;
      $scope.DiskNum = 0 ;

      //测试用
      var s = 0 ;
      var sql = '' ;
      var RecordsNum = 0 ;

      sql = 'SELECT Name, PageSize/1024, LobPageSize/1024, GroupName, TotalRecords, FreeDataSize/1048576, FreeIndexSize/1048576, FreeLobSize/1048576, FreeSize/1048576, MaxDataCapSize/1073741824, MaxIndexCapSize/1073741824, MaxLobCapSize/1073741824, TotalDataSize/1048576, TotalIndexSize/1048576, TotalLobSize/1048576, TotalSize/1048576 FROM $SNAPSHOT_CS WHERE NodeSelect="master" ORDER BY Name' ;

      var queryHost = function(){
         var data = { 
            'cmd': 'query host', 
            'HostInfo': JSON.stringify( {"HostInfo":[ {"HostName":"ubuntu-test-02"},{"HostName":"ubuntu-test-03"}]} )
         } ;
         SdbRest.OmOperation( data, function( hostList ){
            $scope.HostList = hostList ;
         }, function( errorInfo ){
               
         }, function(){

         } ) ;
      }

      $scope.queryModule = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/moduleInfo', success, failed, error ) ;
         SdbRest.Exec( sql, function( csList ){
         } ) ;
      }
      

      $scope.moduleList = function(){
         queryHost() ;

         $scope.queryModule( {}, function( test ){

            $scope.moduleInfo = test[0] ;

            $scope.charts['Host']['CPU'] = { 'percent': $scope.moduleInfo['cpuUse'], 'style': { 'progress': { 'background': '#FF9933' } } } ;   
            $scope.charts['Host']['Memory'] = { 'percent': $scope.moduleInfo['memoryUse'], 'style': { 'progress': { 'background': '#D9534F' } } } ;   
            $scope.charts['Host']['Disk'] = { 'percent': $scope.moduleInfo['diskUse'] } ;
            $.each( $scope.HostList, function( index, value ){
               $scope.DiskNum = $scope.DiskNum + value['Disk'].length ;
            } ) ;
         } ) ; 

         SdbFunction.Interval(function(){
            s = parseInt(Math.random()*100000) + 100000 ;
            $scope.charts['Module']['value'] = [ [ 0, s, true, false ] ] ;
         },5000);
      }
      
      //选择图表
      $scope.changeCharts = function( type ){
         $scope.charts['Module'] = {} ;
         if( type == 'Insert' )
         {
            $scope.chartName = 'Record Insert' ;
            $scope.charts['Module']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;
         }
         else if( type == 'Read' )
         {
            $scope.chartName = 'Record Read' ;
            $scope.charts['Module']['options'] = window.SdbSacManagerConf.RecordReadEchart ;
         }
         else if( type == 'Delete' )
         {
            $scope.chartName = 'Record Delete' ;
            $scope.charts['Module']['options'] = window.SdbSacManagerConf.RecordDeleteEchart ;
         }
         else if( type == 'Update' )
         {
            $scope.chartName = 'Record Update' ;
            $scope.charts['Module']['options'] = window.SdbSacManagerConf.RecordUpdateEchart ;
         }
      }

      //图表下拉选项
      $scope.DropdownMenu = [ 
         { 'html': $compile( '<div style="padding:5px 10px" ng-click="changeCharts(\'Insert\')">Record Insert</div>' )( $scope ) },
         { 'html': $compile( '<div style="padding:5px 10px" ng-click="changeCharts(\'Read\')">Record Read</div>' )( $scope ) },
         { 'html': $compile( '<div style="padding:5px 10px" ng-click="changeCharts(\'Delete\')">Record Delete</div>' )( $scope ) },
         { 'html': $compile( '<div style="padding:5px 10px" ng-click="changeCharts(\'Update\')">Record Update</div>' )( $scope ) }
      ]

      $scope.moduleList() ;
      
      $scope.result = {} ;
      var result = [] ;
      for( var i = 1; i < 2; ++i )
      {
         result.push( 
             '2016-05-23 10:06:22 [Error] - 主机Ubuntu-12-02连接失败'
         )
      }

      $scope.result = result ;
   } ) ;

}());

