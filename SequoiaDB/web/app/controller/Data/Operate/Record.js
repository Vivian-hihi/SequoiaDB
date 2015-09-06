(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Operate.Record.Ctrl', function( $scope, $compile, $location, Loading, SdbRest, InheritSize, SdbFunction, FormModal ){
      var clusterName = SdbFunction.getLocalData( 'SdbClusterName' ) ;
      var moduleName = SdbFunction.getLocalData( 'SdbModuleName' ) ;
      var csName = SdbFunction.getLocalData( 'SdbCsName' ) ;
      var clName = SdbFunction.getLocalData( 'SdbClName' ) ;
      var fullName = csName + '.' + clName ;
      printfDebug( 'Cluster: ' + clusterName + ', Module: ' + moduleName + ', cs: ' + csName + ', cl: ' + clName ) ;
      if( clusterName == null || moduleName == null || csName == null || clName == null )
      {
         $location.path( 'Data/Operate/Index' ) ;
         return;
      }

      //修正宽高
      InheritSize.append( $( '#OperateRecord' ) ) ;
      $( '#OperateRecord > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;
      $( '#GridParent > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;

      //初始化
      var isNotFilter = true ;
      var records = [] ;
      var limit = 30 ;
      var fieldList = [] ;
      $scope.setCurrent = 1 ;
      $scope.current = 1 ;
      $scope.total = 0 ;
      var queryFilter = { 'name': fullName, 'returnnum': limit, 'skip': 0 } ;
      $scope.GridData = { 'title': [], 'body': [], 'tool': {}, 'options': { 'grid': {} } } ;

      //查询所有
      $scope.queryAll = function()
      {
         isNotFilter = true ;
         $scope.setCurrent = 1 ;
         $scope.current = 1 ;
         var skipNum = ( $scope.current - 1 ) * limit ;
         queryFilter = { 'name': fullName, 'returnnum': limit, 'skip': skipNum } ;
         queryRecord( queryFilter, $scope.showType ) ;
      }
      //上一页
      $scope.previous = function()
      {
         --$scope.current ;
         $scope.setCurrent = $scope.current ;
         var skipNum = ( $scope.current - 1 ) * limit ;
         queryFilter['skip'] = skipNum ;
         queryRecord( queryFilter, $scope.showType ) ;
      }
      //检查输入的页数格式
      $scope.checkCurrent = function()
      {
         if( $scope.setCurrent.length == 0 )
         {
            $scope.setCurrent = 1 ;
         }
         else if( isNaN( $scope.setCurrent ) || parseInt( $scope.setCurrent ) != $scope.setCurrent )
         {
            $scope.setCurrent = parseInt( $scope.setCurrent ) ;
         }
      }
      //跳转到指定页
      $scope.gotoPate = function( event )
      {
         if( event.keyCode == 13 )
         {
            $scope.current = $scope.setCurrent ;
            var skipNum = ( $scope.current - 1 ) * limit ;
            queryFilter['skip'] = skipNum ;
            queryRecord( queryFilter, $scope.showType ) ;
         }
      }
      //下一页
      $scope.nextPage = function()
      {
         ++$scope.current ;
         $scope.setCurrent = $scope.current ;
         var skipNum = ( $scope.current - 1 ) * limit ;
         queryFilter['skip'] = skipNum ;
         queryRecord( queryFilter, $scope.showType ) ;
      }

      //显示方式
      $scope.show = function( type ){
         if( type == 1 )
         {
            _DataOperateRecord.buildJsonGrid( $scope, $compile, records, isNotFilter ) ;
         }
         else if( type == 2 )
         {
            _DataOperateRecord.buildTreeGrid( $scope, $compile, records, isNotFilter ) ;
         }
         else
         {
            _DataOperateRecord.buildTableGrid( $scope, $compile, SdbFunction, records, isNotFilter ) ;
         }
      }

      //查询
      function queryRecord( data, type, showSuccess )
      {
         if( typeof( data['filter'] ) != 'undefined' || typeof( data['selector'] ) != 'undefined' || typeof( data['sort'] ) != 'undefined' || typeof( data['hint'] ) != 'undefined' )
         {
            isNotFilter = false ;
         }
         data['cmd'] = 'query' ;
         SdbRest.ClOperation( data, function( json ){
            records = json ;
            //获取所有字段
            $.each( records, function( index, record ){
               fieldList = SdbFunction.getJsonKeys( record, 0, fieldList ) ;
            } ) ;
            if( showSuccess != false )
            {
               $scope.execResult = sprintf( $scope.autoLanguage( '? ? 执行查询成功，显示 ? - ?，总计 ? 条记录' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, data['skip'] + 1, data['skip'] + records.length, records.length ) ;
               $scope.execRc = true ;
            }
            queryFilter = data ;
         }, function( errorInfo ){
            records = [] ;
            fieldList = [] ;
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 执行查询失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
            $scope.execRc = false ;
         }, function(){
            records = [] ;
            fieldList = [] ;
         }, function(){
            $scope.$apply() ;
            $scope.show( type ) ;
         } ) ;
         if( isNotFilter )
         {
            var newdata = { 'cmd': 'get count', 'name': fullName } ;
            SdbRest.ClOperation( newdata, function( countData ){
                $scope.total = countData['Total'];
                $scope.$apply();
            } ) ;
         }
      }

      queryRecord( queryFilter, 1 ) ;

      //创建插入弹窗
      $scope.Insert = function( recordIndex ){
         _DataOperateRecord.createInsertModel( $scope, SdbRest, queryFilter, recordIndex, fullName, queryRecord, records ) ;
      }

      //创建编辑记录弹窗
      $scope.Edit = function( recordIndex ){
         _DataOperateRecord.createEditModel( $scope, SdbRest, queryFilter, recordIndex, fullName, queryRecord, records ) ;
      }

      //创建快速查询弹窗
      $scope.QuickQuery = function(){
         _DataOperateRecord.createQuickQueryModel( $scope, fieldList, fullName, queryRecord ) ;
      }

      //创建查询弹窗
      $scope.Query = function(){
         _DataOperateRecord.createQueryModel( $scope, fieldList, fullName, queryRecord ) ;
      }

      //创建更新弹窗
      $scope.Update = function(){
         _DataOperateRecord.createUpdateModel( $scope, SdbRest, queryFilter, fieldList, fullName, queryRecord ) ;
      }

      //创建删除弹窗
      $scope.Delete = function(){
         _DataOperateRecord.createDeleteModel( $scope, SdbRest, queryFilter, fieldList, fullName, queryRecord ) ;
      }

      //创建删除记录提示
      $scope.DeleteRecord = function( recordIndex ){
         _DataOperateRecord.createDeleteRecordModel( $scope, SdbRest, queryFilter, recordIndex, fullName, queryRecord, records ) ;
      }
   } ) ;
}());