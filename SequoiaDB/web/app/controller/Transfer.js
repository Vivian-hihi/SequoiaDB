(function(){
   var sacApp = window.SdbSacManagerModule ;
   function loginTransfer( $scope, SdbRest, SdbFunction ){
      var data = { 'cmd': 'query business', 'sort': JSON.stringify( { 'BusinessName': 1, 'ClusterName': 1 } ) } ;
      SdbRest.OmOperation( data, function( moduleList ){
         if( moduleList.length > 0 )
         {
            var moduleInfo = moduleList[0] ;
            SdbFunction.LocalData( 'SdbClusterName', moduleInfo['ClusterName'] ) ;
            SdbFunction.LocalData( 'SdbModuleName', moduleInfo['BusinessName'] ) ;
            SdbFunction.LocalData( 'SdbModuleType', moduleInfo['BusinessType'] ) ;
            SdbFunction.LocalData( 'SdbModuleMode', moduleInfo['DeployMod'] ) ;
            if( moduleInfo['BusinessType'] == 'sequoiadb' )
            {
               window.location.href = '/#/Data/SDB-Database/Index' ;
            }
            else if( moduleInfo['BusinessType'] == 'sequoiasql' )
            {
               window.location.href = '/#/Data/SQL-Metadata/Index' ;
            }
            else if( moduleInfo['BusinessType'] == 'spark' )
            {
               window.location.href = '/#/Data/SPARK-web/Index' ;
            }
            else if( moduleInfo['BusinessType'] == 'hdfs' )
            {
               window.location.href = '/#/Data/HDFS-web/Index' ;
            }
            else if( moduleInfo['BusinessType'] == 'yarn' )
            {
               window.location.href = '/#/Data/YARN-web/Index' ;
            }
            else
            {
               window.location.href = '/deployment/index.html' ;
            }
         }
         else
         {
            window.location.href = '/deployment/index.html' ;
         }
      }, function( errorInfo ){
         window.location.href = '/login.html' ;
      }, function(){
         window.location.href = '/login.html' ;
      } ) ;
   }
   //控制器
   sacApp.controllerProvider.register( 'Transfer', function( $scope, $rootScope, $location, SdbRest, SdbFunction ){
      setTimeout( function(){
         loginTransfer( $scope, SdbRest, SdbFunction ) ;
      } ) ;
      //$location.path( '/Deploy/Index' ) ;
   } ) ;
}());