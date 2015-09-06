(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Overview.Index.Ctrl', function( $scope, SdbRest, SdbFunction, InheritSize, FormModal ){
      var clusterName = SdbFunction.getLocalData( 'SdbClusterName' ) ;
      if( clusterName == null )
      {
         $scope.selectCluster( function( clusterName ){
            SdbFunction.setLocalData( 'SdbClusterName', clusterName ) ;
            location.reload( false ) ;
         } ) ;
      }
      InheritSize.append( $( '#ModuleBox' ) ) ;
      //获取业务列表
      /*
      SdbRest.QueryAllBusiness( {}, function( json ){
         //给业务列表增加颜色属性
         $.each( json, function( index ){
            var i = index ;
            if( index > 3 ) i = index % 4 ;
            if( i == 0 ) json[index]['color'] = 'green' ;
            if( i == 1 ) json[index]['color'] = 'yellow' ;
            if( i == 2 ) json[index]['color'] = 'blue' ;
            if( i == 3 ) json[index]['color'] = 'violet' ;
         } ) ;
         $scope.QueryBusiness = json ;
         $scope.$apply() ;
      } ) ;
      */
      var data = { 'cmd': 'query business', 'filter': JSON.stringify( { 'ClusterName' : clusterName } ) } ;
      SdbRest.OmOperation( data, function( json ){
         $.each( json, function( index ){
            var i = index ;
            if( index > 3 ) i = index % 4 ;
            if( i == 0 ) json[index]['color'] = 'green' ;
            if( i == 1 ) json[index]['color'] = 'yellow' ;
            if( i == 2 ) json[index]['color'] = 'blue' ;
            if( i == 3 ) json[index]['color'] = 'violet' ;
         } ) ;
         $scope.QueryModule = json ;
         $scope.$apply() ;
      }, function( errorInfo ){}, function(){} ) ;
   } ) ;
   var boxEle = [] ;
   var chartEle = [] ;
   var databaseEle = [] ;

   //记录视图
   sacApp.compileProvider.directive( 'createRecordChart', function(){
      var option = window.SdbSacManagerConf.recordEchart ;
      return {
         link: function( scope, element, attrs ){
            var ele = $( element ).get(0) ;
            var newOption = option ;
            var chart = echarts.init( ele ).setOption( newOption ) ;
            chartEle.push( { chart: chart, ele: ele } ) ;
         }
      } ;
   } ) ;

   //数据库视图
   sacApp.compileProvider.directive( 'createDatabaseChart', function( Graphics ){
      return {
         link: function( scope, element, attrs ){
            var newDatabase = Graphics.drawCylinder( $( element ).get(0), 5, 5, 20, [ { name: 'Lob', color: '#68DEAB', percent: 0.5 },
                                                                                      { name: 'Data', color: '#84BAE7', percent: 0.2 },
                                                                                      { name: 'Index', color: '#EDCC96', percent: 0.3 } ] ) ;
            databaseEle.push( newDatabase ) ;
         }
      } ;
   } ) ;

   //盒子内响应框架
   sacApp.compileProvider.directive( 'createResponse', function( $window, Graphics, ResponseLayout ){
      //重绘
      function ModuleListResize()
      {
         $.each( boxEle, function( index, value ){
            ResponseLayout.resize( value ) ;
         } ) ;
         $.each( chartEle, function( index, value ){
            value['chart'].resize() ;
         } ) ;
         $.each( databaseEle, function( index, value ){
            Graphics.resizeCylinder( value ) ;
         } ) ;
      }
      angular.element( $window ).bind( 'resize', function () {
         ModuleListResize() ;
      } ) ;
      return {
         link: function( scope, element, attrs ){
            var layout = ResponseLayout.render( element ) ;
            boxEle.push( layout ) ;
            if( scope.$last == true )
            {
               layout = ResponseLayout.render( $( '#DataBoxList' ) ) ;
               boxEle.unshift( layout ) ;
               ModuleListResize() ;
            }
         }
      } ;
   } ) ;
}());