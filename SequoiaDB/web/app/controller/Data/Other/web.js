(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Other.Index.Ctrl', function( $scope, $location, $compile, SdbFunction, SdbRest ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      if( clusterName == null || ( moduleType != 'hdfs' && moduleType != 'spark' && moduleType != 'yarn' )  || moduleName == null )
      {
         window.location.href = '/deployment/index.html' ;
         return;
      }

      printfDebug( 'Cluster: ' + clusterName + ', Type: ' + moduleType + ', Module: ' + moduleName ) ;

      $scope.url = '' ;

      var data = { 'cmd': 'query business', 'filter': JSON.stringify( { 'BusinessName': moduleName, 'ClusterName': clusterName } ) } ;
      SdbRest.OmOperation( data, function( moduleInfo ){
         if( moduleInfo.length > 0 )
         {
            moduleInfo = moduleInfo[0]['BusinessInfo'] ;
            $scope.url = 'http://' + moduleInfo['HostName'] + ':' + moduleInfo['WebServicePort'] ;
         }
      }, function( errorInfo ){
         _IndexPublic.createRetryModel( $scope, errorInfo, function(){
            return true ;
         } ) ;
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      } ) ;
   } ) ;
}());