(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.HostPerformance.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      
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