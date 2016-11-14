(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Performance.Network.Index.Ctrl', function( $scope, $compile, $location, SdbRest, SdbFunction ){
      
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      var HostName = SdbFunction.LocalData( 'SdbHostName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      $scope.hostName = HostName ;
      $scope.hostInfo = [] ;

      var queryHostList = function(){
         var data = {
            'cmd': 'query host status',
            'HostInfo': JSON.stringify( { 'HostInfo': [ { 'HostName': HostName } ] } )
         } ;
         $scope.NetList = [] ;
         SdbRest.OmOperation( data, {
            'success': function( info ){
               $.each( info[0]['HostInfo'], function( Index, value ){
                  $.each( value['Net']['Net'], function( Index2, value2 ){
                     $scope.NetList[Index2] = {
                        'Name': value2['Name'],
                        'RXBytes': value2['RXBytes']['Megabit'] + ( value2['RXBytes']['Unit'] / 1024 / 1024 ).toFixed( 2 ) + 'MB',
                        'RXPackets': value2['RXPackets']['Megabit'] + ( value2['RXPackets']['Unit'] / 1024 / 1024 ).toFixed( 2 ) + 'MB',
                        'TXBytes': value2['TXBytes']['Megabit'] + ( value2['TXBytes']['Unit'] / 1024 / 1024 ).toFixed( 2 ) + 'MB',
                        'TXPackets': value2['TXPackets']['Megabit'] + ( value2['TXPackets']['Unit'] / 1024 / 1024 ).toFixed( 2 ) + 'MB',
                        'RXChart': { 'options': window.SdbSacManagerConf.NetwordInEchart },
                        'TXChart': { 'options': window.SdbSacManagerConf.NetwordOutEchart }
                     } ;
                  } ) ;
               } ) ;
              
               var SumInfo = [] ;
               SdbRest.OmOperation( data, {
                  'success': function( infomach ){
                     $.each( infomach[0]['HostInfo'], function( Index, value ){
                        $.each( value['Net']['Net'], function( Index2, value2 ){

                           if( typeof( SumInfo[Index2] ) == 'undefined' )
                           {
                              SumInfo[Index2] = {} ;
                              SumInfo[Index2]['RX'] = parseInt( value2['RXBytes']['Megabit'] * 1024 +  value2['RXBytes']['Unit'] / 1024 ) ;
                              SumInfo[Index2]['TX'] = parseInt( value2['TXBytes']['Megabit'] * 1024 + value2['TXBytes']['Unit'] / 1024 ) ;
                           }
                           else
                           {
                              $scope.NetList[Index2]['TXChart']['value'] = [ [ 0, ( parseInt( value2['TXBytes']['Megabit'] * 1024 + value2['TXBytes']['Unit'] / 1024 ) - SumInfo[Index2]['TX'] )/5, true, false ] ] ;
                              $scope.NetList[Index2]['RXChart']['value'] = [ [ 0, ( parseInt( value2['RXBytes']['Megabit'] * 1024 + value2['RXBytes']['Unit'] / 1024 ) - SumInfo[Index2]['RX'] )/5, true, false ] ] ;
                              
                              SumInfo[Index2]['RX'] = parseInt( value2['RXBytes']['Megabit'] * 1024 + value2['RXBytes']['Unit'] / 1024 ) ;
                              SumInfo[Index2]['TX'] = parseInt( value2['TXBytes']['Megabit'] * 1024 + value2['TXBytes']['Unit'] / 1024 ) ;
                           }
                        } ) ;
                     } ) ;
                  },
                  'failed': function( errorInfo ){
                     _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                        queryHostList() ;
                        return true ;
                     } ) ;
                  }
               }, {
                  'showLoading': false,
                  'delay': 5000,
                  'loop': true
               } ) ;
            }, 
            'failed': function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  queryHostList() ;
                  return true ;
               } ) ;
            }
         } ) ;
      }

      queryHostList() ;

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