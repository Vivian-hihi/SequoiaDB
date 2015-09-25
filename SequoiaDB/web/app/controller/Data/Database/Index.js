(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Database.Index.Ctrl', function( $scope, $location, SdbFunction, SdbRest, InheritSize, FormModal ){
      //添加自动调节宽高
      InheritSize.append( $( '#MetadataIndex' ) ) ;
      InheritSize.append( $( '#MetadataList' ) ) ;
      $( '#MetadataList > div' ).each( function( index, ele ){
         InheritSize.append( ele ) ;
      } ) ;

      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleMode = SdbFunction.LocalData( 'SdbModuleMode' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      printfDebug( 'Cluster: ' + clusterName + ', Module: ' + moduleName + ', Mode: ' + moduleMode ) ;
      if( clusterName == null )
      {
         $scope.selectCluster( function( clusterName ){
            SdbFunction.LocalData( 'SdbClusterName', clusterName ) ;
            location.reload( false ) ;
         } ) ;
      }
      else if( moduleName == null || moduleMode == null )
      {
         $scope.selcetModule( function( moduleName ){
            SdbFunction.LocalData( 'SdbModuleName', moduleName ) ;
            location.reload( false ) ;
         } ) ;
      }

      $scope.moduleName = moduleName ;
      $scope.maxShowCLNum = [] ;
      
      //跳到记录页面
      $scope.gotoRecord = function( csName, clName )
      {
         SdbFunction.LocalData( 'SdbCsName', csName ) ;
         SdbFunction.LocalData( 'SdbClName', clName ) ;
         $location.path( 'Data/Operate/Record' ) ;
      }

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
               var clSearchNum = 0 ;
               $.each( csInfo['Collection'], function( index2, clInfo ){
                  if( clName == '' || clName == '*' )
                  {
                     csList[index]['Collection'][index2]['hide'] = false ;
                     ++clSearchNum ;
                  }
                  else
                  {
                     csList[index]['Collection'][index2]['hide'] = ( clInfo['Name'] != clName ) ;
                     if( clInfo['Name'] == clName )
                     {
                        ++clSearchNum ;
                     }
                  }
               } ) ;
               if( clSearchNum == 0 )
               {
                  csList[index]['hide'] = true ;
               }
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
         $scope.attr = $scope.csList[index] ;
      }

      //显示CL属性
      $scope.showCLInfo = function( csIndex, clIndex ){
         $scope.bold = { 'font-weight':'bold' } ;
         $scope.showType = 'cl' ;
         $scope.clID = [ csIndex, clIndex ] ;
      }

      //打开 创建集合空间 的窗口
      $scope.showCreateCS = function(){
         $scope.Components.Modal.icon = 'fa-plus' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '创建集合空间' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "name",
                  "webName": $scope.autoLanguage( '集合空间名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                              "min": 1,
                              "ban": "."
                           }
               },
               {
                  "name": "pageSize",
                  "webName": $scope.autoLanguage( '数据页大小' ),
                  "type": "select",
                  "desc": $scope.autoLanguage( '数据页大小创建后不可更改' ),
                  "value": "4096",
                  "valid": [
                              { "key": "4KB", "value": "4096" },
                              { "key": "8KB", "value": "8192" },
                              { "key": "16KB", "value": "16384" },
                              { "key": "32KB", "value": "32768" },
                              { "key": "64KB", "value": "65536" }
                           ]
               },
               {
                  "name": "Domain",
                  "webName": $scope.autoLanguage( '所属域' ),
                  "type": "string",
                  "desc": $scope.autoLanguage( '所属域必须已经存在，且不能为 SYSDOMAIN。' ),
                  "value": "",
                  "valid": {
                              "min": 0,
                              "ban": "SYSDOMAIN"
                           }
               },
               {
                  "name": "LobPageSize",
                  "webName": $scope.autoLanguage( 'Lob数据页大小' ),
                  "type": "select",
                  "desc": $scope.autoLanguage( 'Lob数据页大小创建后不可更改' ),
                  "value": "262144",
                  "valid": [
                              { "key": "4KB", "value": "4096" },
                              { "key": "8KB", "value": "8192" },
                              { "key": "16KB", "value": "16384" },
                              { "key": "32KB", "value": "32768" },
                              { "key": "64KB", "value": "65536" },
                              { "key": "128KB", "value": "131072" },
                              { "key": "256KB", "value": "262144" },
                              { "key": "512KB", "value": "524288" },
                           ]
               },
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form.getValue() ;
               var data = { 'cmd': 'create collectionspace', 'name': value['name'] } ;
               SdbRest.DataOperation( data, function( json ){

               } ) ;
            }
            return isAllClear ;
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
         var maskContent = '\
<div form-create para="data.form"></div>\
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
         var data = { 'cmd': 'list indexes', 'collectionname': 'runWater.2014' } ;
         SdbRest.DataOperation( data, function( indexList ){
            $scope.indexList = indexList ;
            var indexName = [] ; 
            var indexContent = [] ;
            var indexs = $scope.indexList ;
            $.each( indexs, function( index, indexInfo ){
               indexName.push( { 'key': indexInfo['Name'] , 'value': index } ) ;
               indexContent.push(indexInfo) ;
            } ) ;
            $scope.Components.Modal.icon = '' ;
            $scope.Components.Modal.title = '索引详细' ;
            $scope.Components.Modal.noOK = true ;
            $scope.Components.Modal.isShow = true ;
            $scope.Components.Modal.form = {
               inputList: [
                  {
                     "name": "index",
                     "webName": "索引名",
                     "type": "select",
                     "value":indexName[0]['value'] ,
                     "valid": indexName,
                     "onChange": function( name, key, value ){
                        $scope.Components.Modal.Grid = indexContent[ value ] ;
                     }
                  }
               ]
            } ;
            $scope.Components.Modal.Grid = indexContent[0] ;
            $scope.Components.Modal.Context = '\
<div form-create para="data.form"></div>\
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
         } ) ;
      }

      //获取所有cs和cl的信息 索引信息
      function getCSCLInfo()
      {
         var sql ;
         if( moduleMode == 'standalone' )
         {
            sql = 'select Name,TotalRecords,PageSize,LobPageSize,FreeDataSize/1048576,FreeIndexSize/1048576,FreeLobSize/1048576,FreeSize/1048576,MaxCapacitySize/1048576,MaxDataCapSize/1048576,MaxIndexCapSize/1048576,MaxLobCapSize/1048576,TotalDataSize/1048576,TotalIndexSize/1048576,TotalLobSize/1048576,TotalSize/1048576 from $SNAPSHOT_CS' ;
         }
         else
         {
            sql = 'SELECT Name, PageSize/1024, LobPageSize, GroupName, TotalRecords, FreeDataSize/1048576, FreeIndexSize/1048576, FreeLobSize/1048576, FreeSize/1048576, MaxCapacitySize/1048576, MaxDataCapSize/1048576, MaxIndexCapSize/1048576, MaxLobCapSize/1048576, TotalDataSize/1048576, TotalIndexSize/1048576, TotalLobSize/1048576, TotalSize/1048576 FROM $SNAPSHOT_CS WHERE NodeSelect="master"' ;
         }

         $scope.csList = [] ;
         $scope.attr = {} ;

         //获取cs的信息
         SdbRest.Exec( sql, function( csList ){
            if( csList.length == 1 && csList[0]['Name'] == null ) csList = [] ;
            
            $.each( csList, function( index, csInfo ){
               var csListIndex = -1 ;
               //查找cs列表是否已经存在该cs
               $.each( $scope.csList, function( index2, csInfo2 ){
                  if( csInfo['Name'] == csInfo2['Name'] )
                  {
                     csListIndex = index2 ;
                  }
               } ) ;
               //存在则累加
               if( csListIndex >= 0 )
               {
                  $scope.csList[csListIndex]['Info']['TotalRecords']    += csInfo['TotalRecords'] ;
                  $scope.csList[csListIndex]['Info']['TotalDataSize']   += csInfo['TotalDataSize'] ;
                  $scope.csList[csListIndex]['Info']['FreeDataSize']    += csInfo['FreeDataSize'] ;
                  $scope.csList[csListIndex]['Info']['TotalIndexSize']  += csInfo['TotalIndexSize'] ;
                  $scope.csList[csListIndex]['Info']['FreeIndexSize']   += csInfo['FreeIndexSize'] ;
                  $scope.csList[csListIndex]['Info']['TotalLobSize']    += csInfo['TotalLobSize'] ;
                  $scope.csList[csListIndex]['Info']['FreeLobSize']     += csInfo['FreeLobSize'] ;
                  $scope.csList[csListIndex]['Info']['MaxCapacitySize'] += csInfo['MaxCapacitySize'] ;
                  $scope.csList[csListIndex]['Info']['MaxDataCapSize']  += csInfo['MaxDataCapSize'] ;
                  $scope.csList[csListIndex]['Info']['MaxIndexCapSize'] += csInfo['MaxIndexCapSize'] ;
                  $scope.csList[csListIndex]['Info']['MaxLobCapSize']   += csInfo['MaxLobCapSize'] ;
                  $scope.csList[csListIndex]['Info']['TotalSize']       += csInfo['TotalSize'] ;
                  $scope.csList[csListIndex]['Info']['FreeSize']        += csInfo['FreeSize'] ;
                  $scope.csList[csListIndex]['GroupName'].push( csInfo['GroupName'] ) ;
               }
               //不存在则创建
               else
               {
                  var color = '' ;
                  var i = $scope.csList.length ;
                  if( i > 3 ) i = i % 4 ;
                  if( i == 0 ) color = 'green' ;
                  if( i == 1 ) color = 'yellow' ;
                  if( i == 2 ) color = 'blue' ;
                  if( i == 3 ) color = 'violet' ;
                  $scope.csList.push( {
                     'Name': csInfo['Name'],
                     'CLNum': 0,
                     'GroupName': [ csInfo['GroupName'] ],
                     'hide': false,
                     'color': color,
                     'Info': {
                        'Name':            csInfo['Name'],
                        'PageSize':        csInfo['PageSize'],
                        'LobPageSize':     csInfo['LobPageSize'],
                        'TotalRecords':    csInfo['TotalRecords'],
                        'FreeDataSize':    csInfo['FreeDataSize'],
                        'FreeIndexSize':   csInfo['FreeIndexSize'],
                        'FreeLobSize':     csInfo['FreeLobSize'],
                        'FreeSize':        csInfo['FreeSize'],
                        'MaxCapacitySize': csInfo['MaxCapacitySize'],
                        'MaxDataCapSize':  csInfo['MaxDataCapSize'],
                        'MaxIndexCapSize': csInfo['MaxIndexCapSize'],
                        'MaxLobCapSize':   csInfo['MaxLobCapSize'],
                        'TotalDataSize':   csInfo['TotalDataSize'],
                        'TotalIndexSize':  csInfo['TotalIndexSize'],
                        'TotalLobSize':    csInfo['TotalLobSize'],
                        'TotalSize':       csInfo['TotalSize']
                     }
                  } ) ;
               }
            } ) ;
            $.each( $scope.csList, function( index, csInfo ){
               csInfo['Info']['PageSize']        = csInfo['Info']['PageSize'] + 'KB' ;
               csInfo['Info']['TotalRecords']    = parseInt( csInfo['Info']['TotalRecords'] ) + 'MB' ;
               csInfo['Info']['TotalDataSize']   = parseInt( csInfo['Info']['TotalDataSize'] ) + 'MB' ;
               csInfo['Info']['FreeDataSize']    = parseInt( csInfo['Info']['FreeDataSize'] ) + 'MB' ;
               csInfo['Info']['TotalIndexSize']  = parseInt( csInfo['Info']['TotalIndexSize'] ) + 'MB' ;
               csInfo['Info']['FreeIndexSize']   = parseInt( csInfo['Info']['FreeIndexSize'] ) + 'MB' ;
               csInfo['Info']['TotalLobSize']    = parseInt( csInfo['Info']['TotalLobSize'] ) + 'MB' ;
               csInfo['Info']['FreeLobSize']     = parseInt( csInfo['Info']['FreeLobSize'] ) + 'MB' ;
               csInfo['Info']['MaxCapacitySize'] = parseInt( csInfo['Info']['MaxCapacitySize'] ) + 'MB' ;
               csInfo['Info']['MaxDataCapSize']  = parseInt( csInfo['Info']['MaxDataCapSize'] ) + 'MB' ;
               csInfo['Info']['MaxIndexCapSize'] = parseInt( csInfo['Info']['MaxIndexCapSize'] ) + 'MB' ;
               csInfo['Info']['MaxLobCapSize']   = parseInt( csInfo['Info']['MaxLobCapSize'] ) + 'MB' ;
               csInfo['Info']['TotalSize']       = parseInt( csInfo['Info']['TotalSize'] ) + 'MB' ;
               csInfo['Info']['FreeSize']        = parseInt( csInfo['Info']['FreeSize'] ) + 'MB' ;
            } ) ;
            $scope.$apply() ;

            /*
            if( moduleMode == 'standalone' )
            {
               sql = 'select * from $SNAPSHOT_CL' ;
            }
            else
            {
               sql = 'SELECT t5.Name, t5.IsMainCL, t5.MainCLName, t5.ShardingKey, t5.ShardingType, t4.TotalRecords, t4.TotalDataPages, t4.TotalIndexPages, t4.TotalLobPages, t4.TotalDataFreeSpace, t4.TotalIndexFreeSpace, t4.Groups, t4.Status FROM (SELECT sum(t3.TotalRecords) AS TotalRecords, sum(t3.TotalDataPages) AS TotalDataPages, sum(t3.TotalIndexPages) AS TotalIndexPages, sum(t3.TotalLobPages) AS TotalLobPages, sum(t3.TotalDataFreeSpace) AS TotalDataFreeSpace, sum(t3.TotalIndexFreeSpace) AS TotalIndexFreeSpace, t3.Name, push(t3.GroupName) AS Groups, t3.Status FROM (SELECT * FROM (SELECT t1.Name AS Name, t1.Details.GroupName AS GroupName, t1.Details.TotalRecords AS TotalRecords, t1.Details.TotalDataPages AS TotalDataPages, t1.Details.TotalIndexPages AS TotalIndexPages, t1.Details.TotalLobPages AS TotalLobPages, t1.Details.TotalDataFreeSpace AS TotalDataFreeSpace, t1.Details.TotalIndexFreeSpace AS TotalIndexFreeSpace, t1.Details.Status AS Status FROM (SELECT * FROM $SNAPSHOT_CL WHERE NodeSelect="master" split BY Details) AS t1) AS t2) AS t3 GROUP BY t3.Name) AS t4 RIGHT OUTER JOIN $SNAPSHOT_CATA AS t5 ON t4.Name = t5.Name' ;
            }

            //获取cl信息
            SdbRest.Exec( sql, function( clList ){
               if( clList.length == 1 && clList[0]['Name'] == null ) clList = [] ;
               $.each( csList, function( index, csInfo ){
                  csList[index]['hide'] = false ;
                  csList[index]['Collection'] = [] ;
                  csList[index]['GroupName'] = [] ;
                  $scope.maxShowCLNum.push( 3 ) ;
                  var i = index ;
                  if( index > 3 ) i = index % 4 ;
                  if( i == 0 ) csList[index]['color'] = 'green' ;
                  if( i == 1 ) csList[index]['color'] = 'yellow' ;
                  if( i == 2 ) csList[index]['color'] = 'blue' ;
                  if( i == 3 ) csList[index]['color'] = 'violet' ;
               } ) ;


               $.each( clList, function( index, clInfo ){
                  var fullName = clInfo['Name'].split( '.' ) ;
                  var csName = fullName[0] ;
                  var clName = fullName[1] ;
                  $.each( csList, function( index2, csInfo ){
                     if( csInfo['Name'] == csName )
                     {
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
                        var appendCLInfo = {} ;
                        appendCLInfo['Name'] = clName ;
                        appendCLInfo['hide'] = false ;
                        appendCLInfo['Details'] = clInfo['Details'] ;
                        appendCLInfo['ShardingType'] = shardingType ;
                        if( appendCLInfo['Details'] != null )
                        {
                           if( appendCLInfo['Details'][0]['TotalDataPages'] > 0 )
                           {
                              appendCLInfo['Details'][0]['MetadataRatio'] = parseInt( appendCLInfo['Details'][0]['TotalIndexPages'] * 100 / appendCLInfo['Details'][0]['TotalDataPages'] ) ;
                           }
                           else
                           {
                              appendCLInfo['Details'][0]['MetadataRatio'] = 0 ;
                           }
                        }
                        if( clInfo['Details'] != null )
                        {
                           if( typeof( clInfo['Details'][0]['GroupName'] ) == 'string' && $.inArray( clInfo['Details'][0]['GroupName'], csList[index2]['GroupName'] ) == -1 )
                           {
                              csList[index2]['GroupName'].push( clInfo['Details'][0]['GroupName'] ) ;
                           }
                        }
                        csList[index2]['Collection'].push( appendCLInfo ) ;
                        return false ;
                     }
                  } ) ;
               } ) ;

               //$scope.csList = csList ;
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
            */
         } ) ;
         //获取索引信息
         SdbRest.QueryIndexes( {}, function( indexList ){
            $scope.indexList = indexList ;
         } ) ;
      }
      getCSCLInfo() ;
   } ) ;
}());