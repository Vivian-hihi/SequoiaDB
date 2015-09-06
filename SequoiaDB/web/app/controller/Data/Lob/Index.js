(function(){
   var sacApp = window.SdbSacManagerModule ;
   var GridId ;
   sacApp.controllerProvider.register( 'Data.Lob.Index.Ctrl', function( $scope, $compile, $location, SdbRest, InheritSize, SdbFunction ){
      
      //修正宽高
      InheritSize.append( $( '#LobClList' ) ) ;
      $( '#LobClList > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;
      //获取语言包
      SdbRest.QueryCollection( {}, function( data ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 获取集合列表成功' ), timeFormat( new Date(), 'hh:mm:ss' ), 'myModule' ) ;
         $scope.execRc = true ;
         var clsInfo = data ;
         var gridData = {
            'title': [],
            'body': [],
            'tool': {
               'position': 'bottom',
               'left': [
                  { 'text': sprintf( $scope.autoLanguage('一共 ? 个集合。'), clsInfo.length ) }
               ]
            },
            'options': {
               'order': { 'active': true },
               'grid': {
                  'tdModel': 'fixed',
                  'tdHeight': '19px'
               }
            }
         } ;
         var gridTitle = [
            "",
            $scope.autoLanguage( '集合' ),
            $scope.autoLanguage( '分区类型' ),
            $scope.autoLanguage( '主集合' ),
            $scope.autoLanguage( '记录数' ),
            $scope.autoLanguage( '索引数' )
         ] ;
         $.each( gridTitle, function( index, titleText ){
            gridData['title'].push( { 'text': titleText } ) ;
         } ) ;
         $.each( clsInfo, function( index, clInfo ){
            var shardingType ;
            if( clInfo['IsMainCL'] == true )
            {
               shardingType = $scope.autoLanguage( '垂直分区' ) ;
            }
            else
            {
               if( clInfo['ShardingType'] == 'range' )
               {
                  shardingType = $scope.autoLanguage( '水平范围分区' ) ;
               }
               else if( clInfo['ShardingType'] == 'hash' )
               {
                  shardingType = $scope.autoLanguage( '水平散列分区' ) ;
               }
               else
               {
                  shardingType = $scope.autoLanguage( '普 通' ) ;
               }
            }
            var newEle = $( '<a></a>' ).addClass( 'linkButton' ).text( clInfo['FullName'] ).attr( 'href', './#/Data/Lob/Lobs' ) ;
            var shardingTypeEle = $( '<span></span>' ).addClass( 'badge badge-info' ).text( shardingType ) ;
            gridData['body'].push( [
               { 'text': index + 1 },
               { 'html': newEle },
               { 'html': shardingTypeEle },
               { 'text': clInfo['MainCLName'] == null ? '' : clInfo['MainCLName'] },
               { 'text': clInfo['Details'] == null ? '' : clInfo['Details'][0]['TotalRecords'] },
               { 'text': clInfo['Details'] == null ? '' : clInfo['Details'][0]['Indexes'] }
            ] ) ;
         } ) ;
         $scope.clGridData = gridData ;
         $scope.$apply() ;
      } ) ;
   } ) ;
}());