(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.HostList.Index.Ctrl', function( $scope, SdbRest, $compile, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      $scope.SearchName = {} ;
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
   } ) ;
   //记录视图
}());