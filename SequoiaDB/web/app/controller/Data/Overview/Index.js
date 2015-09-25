(function(){
   var sacApp = window.SdbSacManagerModule ;
   var chartEle = [] ;
   var lastRecord = [] ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Overview.Index.Ctrl', function( $scope, $location, SdbRest, SdbFunction, InheritSize, FormModal ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      if( clusterName == null )
      {
         $scope.selectCluster( function( clusterName ){
            SdbFunction.LocalData( 'SdbClusterName', clusterName ) ;
            location.reload( false ) ;
         } ) ;
      }
      InheritSize.append( $( '#ModuleBox' ) ) ;
      var isFrist = true ;
      var moduleTemplate = [] ;

      //获取业务模板
      function getModuleTemplate()
      {
         var data = { 'cmd': 'get business template', 'BusinessType': 'sequoiadb' } ;
         SdbRest.OmOperation( data, function( json ){
            moduleTemplate = json ;
            getModuleList() ;
         }, function( errorInfo ){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '获取业务列表失败。' ) ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      getModuleTemplate() ;

      //获取业务列表
      function getModuleList()
      {
         //获取集合列表的详细信息
         function getCLInfo( index, moduleName, moduleMode )
         {
            var sql ;
            if( moduleMode == 'standalone' )
            {
               sql = 'select Name as FullName, IsMainCL, MainCLName, ShardingKey, ShardingType, Details from $SNAPSHOT_CL' ;
            }
            else
            {
               sql = 'select T2.Name as FullName,T2.IsMainCL,T2.MainCLName,T2.ShardingKey,T2.ShardingType,T1.Details from $SNAPSHOT_CL as T1 right outer join $SNAPSHOT_CATA as T2 on T1.Name = T2.Name group by T2.Name' ;
            }
            //获取集合列表
            SdbRest.Exec2( clusterName, moduleName, sql, function( data ){
               if( data.length == 1 && data[0]['Name'] == null ) data = [] ;
               var sumRecords = 0 ;
               var clList = data ;
               var lobNum = 0 ;
               var dataNum = 0 ;
               var indexNum = 0 ;
               $.each( clList, function( index2, clInfo ){
                  try
                  {
                     sumRecords += clInfo['Details'][0]['TotalRecords'] ;
                     lobNum += clInfo['Details'][0]['TotalLobPages'] ;
                     dataNum += clInfo['Details'][0]['TotalDataPages'] ;
                     indexNum += clInfo['Details'][0]['TotalIndexPages'] ;
                  }
                  catch( e )
                  {
                  }
               } ) ;
               var sum = lobNum + dataNum + indexNum ;
               var lobPercent = 0 ;
               var dataPercent = 0 ;
               var indexPercent = 0 ;
               if( sum > 0 )
               {
                  lobPercent = ( lobNum / sum ).toFixed( 2 ) ;
                  dataPercent = ( dataNum / sum ).toFixed( 2 ) ;
                  indexPercent = ( indexNum / sum ).toFixed( 2 ) ;
                  $scope.QueryModule[index]['info'] = [] ;
                  if( lobPercent > 0 )
                  {
                     $scope.QueryModule[index]['info'].push( { name: 'Lob', color: '#68DEAB', percent: lobPercent } ) ;
                  }
                  if( dataPercent > 0 )
                  {
                     $scope.QueryModule[index]['info'].push( { name: 'Data', color: '#84BAE7', percent: dataPercent } ) ;
                  }
                  if( indexPercent > 0 )
                  {
                     $scope.QueryModule[index]['info'].push( { name: 'Index', color: '#EDCC96', percent: indexPercent } ) ;
                  }
               }
               else
               {
                  $scope.QueryModule[index]['info'] = [ { name: 'Empty', color: '#DCDCDC', percent: 1 } ] ;
               }
               if( typeof( chartEle[index] ) != 'undefined' )
               {
                  if( lastRecord[index] >= 0 && sumRecords - lastRecord[index] >= 0 )
                  {
                     chartEle[index].addData( [ [ 0, sumRecords - lastRecord[index], true, false ] ] ) ;
                  }
               }
               lastRecord[index] = sumRecords ;
               $scope.QueryModule[index]['detail'] = sprintf( $scope.autoLanguage( '一共 ? 个集合， ? 条记录' ), clList.length, sumRecords ) ;
               $scope.$apply() ;
               //查询到最后一个，重新刷新业务列表
               if( index + 1 == $scope.QueryModule.length )
               {
                  if( $scope.Url.Module == 'Data' && $scope.Url.Action == 'Overview' && $scope.Url.Method == 'Index' )
                  {
                     setTimeout( function(){
                        $.each( $scope.QueryModule, function( index2, moduleInfo ){
                           getCLInfo( index2, moduleInfo.BusinessName, moduleInfo.DeployMod ) ;
                        } ) ;
                     }, 5000 ) ;
                  }
               }
            }, function( errorInfo ){
               if( errorInfo['errno'] == -6 )
               {
                  getModuleList() ;
               }
               else
               {
                  getCLInfo( index, moduleName, moduleMode ) ;
               }
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            }, null, false ) ;
         }
         var data = { 'cmd': 'query business', 'filter': JSON.stringify( { 'ClusterName' : clusterName } ) } ;
         SdbRest.OmOperation( data, function( json ){
            $.each( json, function( index ){
               var i = index ;
               if( index > 3 ) i = index % 4 ;
               if( i == 0 ) json[index]['color'] = 'green' ;
               if( i == 1 ) json[index]['color'] = 'yellow' ;
               if( i == 2 ) json[index]['color'] = 'blue' ;
               if( i == 3 ) json[index]['color'] = 'violet' ;
               if( isFrist )
               {
                  json[index]['detail'] = $scope.autoLanguage( '正在加载...' ) ;
               }
               json[index]['info'] = [] ;
               lastRecord.push( -1 ) ;
            } ) ;
            isFrist = false ;
            $scope.QueryModule = json ;
            $.each( $scope.QueryModule, function( index, moduleInfo ){
               moduleInfo['WebDeployMod'] = moduleInfo.DeployMod ;
               $.each( moduleTemplate, function( index2, templateInfo ){
                  if( moduleInfo.DeployMod == templateInfo.DeployMod )
                  {
                     moduleInfo['WebDeployMod'] = templateInfo.WebName ;
                     return false ;
                  }
               } ) ;
               getCLInfo( index, moduleInfo.BusinessName, moduleInfo.DeployMod ) ;
            } ) ;
            $scope.$apply() ;
         }, function( errorInfo ){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '获取业务列表失败。' ) ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         }, null, false ) ;
      }

      $scope.gotoDatabase = function( moduleIndex )
      {
         var moduleName = $scope.QueryModule[moduleIndex].BusinessName ;
         var moduleMode = $scope.QueryModule[moduleIndex].DeployMod ;
         SdbFunction.LocalData( 'SdbModuleMode', moduleMode ) ;
         SdbFunction.LocalData( 'SdbModuleName', moduleName ) ;
         $location.path( 'Data/Database/Index' ) ;
      }
   } ) ;
   //记录视图
   sacApp.compileProvider.directive( 'createRecordChart', function(){
      var option = window.SdbSacManagerConf.recordEchart ;
      return {
         link: function( scope, element, attrs ){
            setTimeout( function(){
               var ele = $( element ).get(0) ;
               var newOption = option ;
               var chart = echarts.init( ele ).setOption( newOption ) ;
               chartEle.push( chart ) ;
            } ) ;
         }
      } ;
   } ) ;
}());