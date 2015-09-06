(function(){
   var sacApp = window.SdbSacManagerModule ;
   var GridId ;
   sacApp.controllerProvider.register( 'Data.Lob.Lobs.Ctrl', function( $scope, $compile, SdbRest, InheritSize, SdbFunction, FormModal){
      //调整外层div宽高
      InheritSize.append( $( '#LobIndex' ) ) ;
      $( '#LobIndex > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;
      var lobContent ;
      SdbRest.QueryLob( {}, function( records ){
         lobContent = records ;
         var gridData = {
            'title': [],
            'body': [],
            'tool':{
               'position': 'bottom',
               'left': [
                  { 'html': $compile( '<i class="fa fa-step-backward"></i>' )( $scope ) },
                  { 'html': $compile( '<i class="fa fa-play fa-flip-horizontal"></i>' )( $scope ) },
                  { 'html': $compile( '<span><input type="text" value="1" style="width:20px;">&nbsp;/&nbsp;10</span>' )( $scope ) },
                  { 'html': $compile( '<i class="fa fa-play"></i>&nbsp;<i class="fa fa-step-forward"></i>' )( $scope ) }
               ]
            },
            'options': {
               'order': { 'active': true },
               'grid': { 'tool': true, titleWidth: [ 10, '240px', 15, 25, 15, 15, 20, '75px' ] } 
            }
         } ;
         var keyList = [ '' ] ;
         $.each( records, function( index, record ){
             keyList = SdbFunction.getJsonKeys( record, 8, keyList ) ;
         } ) ;
         $.each( keyList, function( index, key ){
            gridData['title'].push( { 'text': key } );
         } ) ;
         gridData['title'][7] = { 'text': '' } ;
         $.each( records, function( index, record ){
            var id = index + 1 ;
            var line = SdbFunction.getJsonValues( record, keyList, [] ) ;
            var newRow = [] ;
            newRow[0] = { 'text': id } ;
            newRow[1] = { 'html': $compile( '<a ng-click="LobContent(' + index + ')"></a>' )( $scope ).addClass( 'linkButton' ).text( line[1] ) } ;
            newRow[2] = { 'text': line[2] } ;
            newRow[3] = { 'text': line[3] } ;
            newRow[4] = { 'text': line[4] } ;
            newRow[5] = { 'text': line[5] } ;
            newRow[6] = { 'text': line[6] } ;
            //构造一个删除按钮
            var removeIcon = $( '<i></i>' ).addClass( 'fa fa-remove' ).text( ' ' + $scope.autoLanguage( '删除' ) ) ;
            var removeBtn = $compile( '<a ng-click="LobDelete()"></div>' )( $scope ).addClass( 'linkButton' ).append( removeIcon ) ;
            newRow[7] = { 'html': removeBtn } ;
            gridData['body'].push( newRow ) ;
         } ) ;
         $scope.lobGridData = gridData ;
         $scope.$apply() ; 
      } ) ;
   

      $scope.LobDelete = function(){
         $scope.Components.Confirm.isShow = true ;
         // $scope.Components.Confirm.noOK = true ;
         // $scope.Components.Confirm.noclose = true ;
         $scope.Components.Confirm.type = 3;
         $scope.Components.Confirm.ok = function(){ $scope.Components.Confirm.isShow = false ; } ;
         $scope.Components.Confirm.okText = 'Yes, delete it!' ;
         // $scope.Components.Confirm.closeText = 'Cancel' ;
         $scope.Components.Confirm.contentType2 = '5435e7b69487faa6630008971024 2015-07-16 10:10sequwishhicontrol Del' ;


      }
      $scope.LobQuery = function(){
         $scope.Components.Modal.icon = 'fa-search' ;
         $scope.Components.Modal.title = $scope.autoLanguage( 'Lob 查询' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "LobQuery",
                  "webName": "Lob Id",
                  "type": "string"
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){

            return false ;
         } 
      }
      
      
      $scope.LobContent = function( index ){
         var records = lobContent[index] ;
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( 'Lob 详细' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.Grid = lobContent[index] ;
         $scope.Components.Modal.Context = ' <table class="table loosen border">\
                                                <tr>\
                                                   <td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
                                                   <td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
                                                </tr>\
                                                <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key == \'Id\'">\
                                                   <td>Name</td>\
                                                   <td>{{value}}</td>\
                                                </tr>\
                                                <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key != \'Id\'">\
                                                   <td>{{key}}</td>\
                                                   <td>{{value}}</td>\
                                                </tr>\
                                             </table>' ;
         $scope.Components.Modal.ok = function(){
             return false ;
         }    
      }
      } ) ;
      //$scope.QueryLobs = QueryLob ;
   }()) ;