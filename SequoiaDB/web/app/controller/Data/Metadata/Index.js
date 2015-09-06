(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Metadata.Index.Ctrl', function( $scope, SdbRest, InheritSize, FormModal ){
      //添加自动调节宽高
      InheritSize.append( $( '#MetadataIndex' ) ) ;
      InheritSize.append( $( '#MetadataList' ) ) ;
      $( '#MetadataList > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;

      $scope.maxShowCLNum = [] ;
      
      //过滤CS和cl
      $scope.findCLName = '' ;
      $scope.search = function( fullName ){
         var csName = '', clName = '';
         var tmp = fullName.split( '.' ) ;
         csName = tmp[0] ;
         clName = tmp[1] ;
         var csList = $scope.csList ;
         $.each( csList, function( index, csInfo ){
            if( typeof( clName ) == 'string' )
            {
               if( csName == '' || csName == '*' )
               {
                  csList[index]['hide'] = false ;
               }
               else
               {
                  csList[index]['hide'] = ( csInfo['Name'] != csName ) ;
               }
               $.each( csInfo['Collection'], function( index2, clInfo ){
                  if( clName == '' || clName == '*' )
                  {
                     csList[index]['Collection'][index2]['hide'] = false ;
                  }
                  else
                  {
                     csList[index]['Collection'][index2]['hide'] = ( clInfo['Name'] != clName ) ;
                  }
               } ) ;
               $scope.findCLName = clName ;
            }
            else
            {
               if( csName == '' || csName == '*' )
               {
                  csList[index]['hide'] = false ;
               }
               else
               {
                  csList[index]['hide'] = ( csInfo['Name'].indexOf( csName ) == -1 ) ;
               }
               $scope.findCLName = '' ;
            }
         } ) ;
         $scope.csList = csList ;
      }
      //展开cs下所有的cl
      $scope.clTableShow = function( index ){
         $scope.maxShowCLNum[index] = 9999 ;
      }
      //收起cs下所有的cl
      $scope.clTableHide = function( index ){
         $scope.maxShowCLNum[index] = 3 ;
      }
      //显示CS属性
      $scope.showCSInfo = function( index ){
         $scope.showType = 'cs' ;
         $scope.csID = index ;
      }
      //显示CL属性
      $scope.showCLInfo = function( csIndex, clIndex ){
         $scope.bold = {'font-weight':'bold'};
         $scope.showType = 'cl' ;
         $scope.clID = [ csIndex, clIndex ] ;
         SdbRest.QueryIndexes( {}, function( indexList ){
            $scope.indexList = indexList ;
         } ) ;
      }

      //打开 创建集合空间 的窗口
      $scope.showCreateCS = function(){
         $scope.Components.Modal.icon = 'fa-plus' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '创建集合空间' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "model",
                  "webName": $scope.autoLanguage( '集合空间名' ),
                  "type": "string",
                  "required": true
               },
               {
                  "name": "pageSize",
                  "webName": $scope.autoLanguage( '数据页大小' ),
                  "type": "select",
                  "desc": $scope.autoLanguage( '数据页大小创建后不可更改' ),
                  "value": "4096",
                  "valid": [
                           { "key": "4096", "value": "4096" },
                           { "key": "8192", "value": "8192" },
                           { "key": "16384", "value": "16384" },
                           { "key": "32768", "value": "32768" },
                           { "key": "65536", "value": "65536" }
                        ]
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form.getValue() ;
               alert( JSON.stringify( value ) ) ;
            }
            return false ;
         }
      }

         //打开 删除集合空间 的窗口
         $scope.showRemoveCS = function(){
         var csValid = [] ;
         var csContent = [] ;
         var csID = '' ;
         var type = $scope.showType ;
         //获取csID
         if( type == 'cl' )
         {
            var clID = $scope.clID ;
            csID = clID[0] ;
         }
         else if( type == 'cs' )
         {
            csID = $scope.csID ;
         } ;
         var csList = $scope.csList ;
         //获取集合空间名显示在列表中
         $.each( csList, function( index, csInfo ){
            csValid.push( { 'key' : csInfo['Name'] , 'value' : index } );
            csContent.push(csInfo);
         } ) ;         
         $scope.Components.Modal.icon = 'fa-remove' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '删除集合空间' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "csName",
                  "webName": $scope.autoLanguage( '集合空间名' ),
                  "type": "select",
                  "value": csValid[csID]['value'],
                  "desc": $scope.autoLanguage( '选择集合空间' ),
                  "valid": csValid,
                  "onChange": function( name, key, value ){
                     $scope.Components.Modal.Grid = csContent[value] ;
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Grid = csContent[csID] ;
         var maskContent = '<div form-create para="data.form"></div>\
<table class="table loosen border">\
   <tr>\
      <td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
      <td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key != \'Collection\'&&key != \'Group\'&&key != \'Name\'&&key != \'color\'&&key != \'hide\'">\
      <td>{{key}}</td>\
      <td>{{value}}</td>\
   </tr>\
</table>' ;
         $scope.Components.Modal.Context = maskContent ;
         $scope.Components.Modal.ok = function(){
            return false ;
         }
      }

      //打开 删除集合 的窗口
      $scope.showRemoveCL = function(){
         var clValid = [] ;
         var clContent = [] ;
         var clID = $scope.clID ;
         var type = $scope.showType ;
         if( type == 'cl' )
         {
            var csID = clID[0] ;
            clID = clID[1] ;
         }
         else if( type == 'cs' )
         {
            var csID = $scope.csID ;
            csID = csID;
            clID = 0 ;
         }
         var csList = $scope.csList ;
         var clName = '' ;
         $.each( csList[csID]['Collection'], function( index, clInfo ){
            if( index == clID )
            {
               clName = index ;
            }
            clContent.push( clInfo ) ;
            clValid.push( { 'key' : csList[csID]['Name'] + '.' + clInfo['Name'] , 'value' : index } );
         } ) ;
         $scope.Components.Modal.icon = 'fa-remove' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '删除集合' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "clName",
                  "webName": $scope.autoLanguage( '集合名' ),
                  "type": "select",
                  "value": clName,
                  // "desc": "选择集合",
                  // "required": true,
                  "valid": clValid,
                  "onChange": function( name, key, value ){
                     $scope.Components.Modal.Grid = clContent[value] ;
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Grid = clContent[clID] ;
         var maskContext = '<div form-create para="data.form"></div>\
<table class="table loosen border">\
   <tr>\
      <td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
      <td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key == \'Name\'">\
      <td>Name</td>\
      <td>{{value}}</td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid[\'Details\'][0] track by $index">\
      <td>{{key}}</td>\
      <td>{{value}}</td>\
   </tr>\
</table>';
         $scope.Components.Modal.Context = maskContext ;
         $scope.Components.Modal.ok = function(){
            return false ;
         }
      }

      //打开 创建集合 的窗口
      $scope.showCreateCL = function(){
         $scope.Components.Modal.icon = 'fa-plus' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '创建集合' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                 
                  "name": "clName",
                  "webName": $scope.autoLanguage( '集合名' ),
                  "type": "string",
                  // "value": "foo",
                  // "desc": "选择集合空间",
                  "required": true
                  
               },
               {
                 
                  "name": "clType",
                  "webName": $scope.autoLanguage( '集合类型' ),
                  "type": "select",
                  "value": "普通",
                  // "desc": "选择集合空间",
                  // "required": true
                  'valid': [
                     { "key": $scope.autoLanguage( '普 通' ), "value": "普通" },
                     { "key": $scope.autoLanguage( '水平范围分区' ), "value": "水平范围分区" },
                     { "key": $scope.autoLanguage( '水平散列分区' ), "value": "水平散列分区" },
                     { "key": $scope.autoLanguage( '垂直分区' ), "value": "垂直分区" }         
                  ]
               },
               {
                  "neme":"",
                  "webName":  $scope.autoLanguage( '分区键' ),
                  "required": true,
                  "desc": "分区键，指定一个或多个字段正向或逆向排序",
                  "type": "list",
                  "child":[
                     [
                        {
                           "name": "ShardingKey",
                           "type":"string"
                        },
                        {
                           "name": "sort",
                           "type": "select",
                           "value": "1",
                           "valid": [
                              { "key": $scope.autoLanguage( '正向排序' ), "value": "1" },
                              { "key": $scope.autoLanguage( '逆向排序' ), "value": "-1" }
                           ]
                        }
                     ]
                  ]
               },
               {
                  "name": "ReplSize",
                  "webName":  $scope.autoLanguage( '副本数' ),
                  "type": "select",
                  "value": "2",
                  "valid": [
                     { "key": "0", "value": "0" },
                     { "key": "1", "value": "1" },
                     { "key": "2", "value": "2" },
                     { "key": "3", "value": "3" },
                     { "key": "4", "value": "4" },
                     { "key": "5", "value": "5" },
                     { "key": "6", "value": "6" },
                     { "key": "7", "value": "7" }
                  ]
               },
               {
                  "name": "",
                  "webName":  $scope.autoLanguage( '数据压缩' ),
                  "type": "select",
                  "required": false,
                  "desc": "",
                  "value": "关",
                  "valid": [
                     { "key": $scope.autoLanguage( '开' ), "value": "开" },
                     { "key": $scope.autoLanguage( '关' ), "value": "关" }
                  ]
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form.getValue() ;
               alert( JSON.stringify( value ) ) ;
            }
            return false ;
         }
      }

      //打开 创建索引 的窗口
      $scope.showCreateIndex = function(){
         $scope.Components.Modal.icon = 'fa-plus' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '创建索引' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                 
                  "name": "indexName",
                  "webName": $scope.autoLanguage( '索引名' ),
                  "type": "string",
                  "desc": $scope.autoLanguage( '索引名，同一个集合中的索引名必须唯一' ),
                  "required": true
               },
               {
                  "name":"indexKey",
                  "webName": $scope.autoLanguage( '索引键' ),
                  "required": true,
                  "desc": $scope.autoLanguage( '索引键，包含一个或多个指定索引字段与方向的对象' ),
                  "type": "list",
                  "child":[
                     [
                        {
                           "name": "indexDef",
                           "type":"string"
                        },
                        {
                           "name": "order",
                           "type": "select",
                           "value": "1",
                           "valid": [
                              { "key": $scope.autoLanguage( '正向排序' ), "value": "1" },
                              { "key": $scope.autoLanguage( '逆向排序' ), "value": "-1" }
                           ]
                        }
                     ]
                  ]
               },
               {
                  "name": "isUnique",
                  "webName": $scope.autoLanguage( '索引是否唯一' ),
                  "type": "select",
                  "value": "yes",
                  "valid": [
                     { "key": $scope.autoLanguage( '是' ), "value": "yes" },
                     { "key": $scope.autoLanguage( '否' ), "value": "no" }
                  ]
               },
               {
                  "name": "enforced",
                  "webName": $scope.autoLanguage( '索引是否强制唯一' ),
                  "type": "select",
                  "desc": "",
                  "value": "no",
                  "valid": [
                     { "key": $scope.autoLanguage( '是' ), "value": "yes" },
                     { "key": $scope.autoLanguage( '否' ), "value": "no" }
                  ]
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form.getValue() ;
               alert( JSON.stringify( value ) ) ;
            }
            return false ;
         }
      }




      //打开 删除索引 的窗口
      $scope.showRemoveIndex = function(){
         var indexName = [] ; 
         var indexContent = [] ;
         var indexs = $scope.indexList ;
         $.each( indexs, function( index, indexInfo ){
            indexName.push( { 'key': indexInfo['Name'] , 'value': index } ) ;
            indexContent.push(indexInfo) ;
         } ) ;
         $scope.Components.Modal.icon = 'fa-remove' ;
         $scope.Components.Modal.title =  $scope.autoLanguage( '删除索引' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "index",
                  "webName":  $scope.autoLanguage( '索引名' ),
                  "type": "select",
                  "value":indexName[0]['value'] ,
                  // "desc": "选择集合",
                  // "required": true,
                  "valid": indexName,
                  "onChange": function( name, key, value ){
                     $scope.Components.Modal.Grid = indexContent[ value ] ;
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Grid = indexContent[0] ;
         $scope.Components.Modal.qwe = $scope.isShow = true ;
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>\
<table class="table loosen border">\
   <tr>\
      <td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
      <td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key == \'Name\'">\
      <td>Name</td>\
      <td>{{value}}</td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key != \'Name\'&&key != \'_id\'">\
      <td>{{key}}</td>\
      <td>{{value}}</td>\
   </tr>\
</table>' ;
         $scope.Components.Modal.ok = function(){
            return false ;
         }    
      }

      //打开 索引详细 的窗口
      $scope.showIndex = function(){
         var indexName = [] ; 
         var indexContent = [] ;
         var indexs = $scope.indexList ;
         $.each( indexs, function( index, indexInfo ){
            indexName.push( { 'key': indexInfo['Name'] , 'value': index } ) ;
            indexContent.push(indexInfo) ;
         } ) ;
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '索引详细' ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "index",
                  "webName": "索引名",
                  "type": "select",
                  "value":indexName[0]['value'] ,
                  // "desc": "选择集合",
                  // "required": true,
                  "valid": indexName,
                  "onChange": function( name, key, value ){
                     $scope.Components.Modal.Grid = indexContent[ value ] ;
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Grid = indexContent[0] ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>\
<table class="table loosen border">\
   <tr>\
      <td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
      <td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key == \'Name\'">\
      <td>Name</td>\
      <td>{{value}}</td>\
   </tr>\
   <tr ng-repeat="(key, value) in data.Grid track by $index" ng-if="key != \'Name\'&&key != \'_id\'">\
      <td>{{key}}</td>\
      <td>{{value}}</td>\
   </tr>\
</table>' ;
         $scope.Components.Modal.ok = function(){
            return false ;
         }      
      }

      //获取所有cs和cl的信息 索引信息
      function getCSCLInfo()
      {
         //获取cs和cl的信息
         SdbRest.QueryCS( {}, function( csList ){
            SdbRest.QueryCL( {}, function( clList ){
               //合并cs和cl的信息
               $.each( csList, function( index, csInfo ){
                  csList[index]['hide'] = false ;
                  $.each( csInfo['Collection'], function( index2, clInfo ){
                     var fullName = csInfo['Name'] + '.' + clInfo['Name'] ;
                     $.each( clList, function( index3, clInfo2 ){
                        if( clInfo2['Name'] == fullName )
                        {
                           csList[index]['Collection'][index2]['hide'] = false ;
                           csList[index]['Collection'][index2]['Details'] = clInfo2['Details'] ;
                           if( csList[index]['Collection'][index2]['Details'][0]['TotalDataPages'] > 0 )
                           {
                              csList[index]['Collection'][index2]['Details'][0]['MetadataRatio'] = parseInt( csList[index]['Collection'][index2]['Details'][0]['TotalIndexPages'] * 100 / csList[index]['Collection'][index2]['Details'][0]['TotalDataPages'] ) ;
                           }
                           else
                           {
                              csList[index]['Collection'][index2]['Details'][0]['MetadataRatio'] = 0 ;
                           }
                           return false ;
                        }
                     } ) ;
                  } ) ;
                  $scope.maxShowCLNum.push( 3 ) ;
                  var i = index ;
                  if( index > 3 ) i = index % 4 ;
                  if( i == 0 ) csList[index]['color'] = 'green' ;
                  if( i == 1 ) csList[index]['color'] = 'yellow' ;
                  if( i == 2 ) csList[index]['color'] = 'blue' ;
                  if( i == 3 ) csList[index]['color'] = 'violet' ;
               } ) ;
               $scope.csList = csList ;
               $scope.showType = 'cs' ;
               if( csList.length > 0 )
               {
                  $scope.csID = 0 ;
               }
               else
               {
                  $scope.csID = -1 ;
               }
               $scope.$apply() ;
            } ) ;
         } ) ;
         //获取索引信息
         SdbRest.QueryIndexes( {}, function( indexList ){
            $scope.indexList = indexList ;
         } ) ;
      }
      getCSCLInfo() ;
   } ) ;
}());