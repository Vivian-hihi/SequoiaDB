(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Data.Database.Index.Ctrl', function( $scope, $location, SdbFunction, SdbRest, InheritSize, FormModal ){
      //添加自动调节宽高
      _DataDatabaseIndex.auto2SetWidthAndHeight( InheritSize ) ;

      $scope.selectCluster( function( clusterName ){
         SdbFunction.LocalData( 'SdbClusterName', clusterName ) ;
         $scope.selcetModule( clusterName, function( moduleMode, moduleName ){
            SdbFunction.LocalData( 'SdbModuleMode', moduleMode ) ;
            SdbFunction.LocalData( 'SdbModuleName', moduleName ) ;
            //初始化参数
            _DataDatabaseIndex.init( $scope, moduleName, moduleMode ) ;

            //获取分区组列表
            _DataDatabaseIndex.getGroupList( $scope, SdbRest ) ;
      
            //跳到记录页面
            $scope.gotoRecord = function( csIndex, clIndex ){
               _DataDatabaseIndex.gotoRecord( $scope, $location, SdbFunction, csIndex, clIndex ) ;
            }

            //过滤CS和cl
            $scope.search = function( fullName ){
               _DataDatabaseIndex.searchCSAndCL( $scope, fullName ) ;
            }

            //展开cs下所有的cl
            $scope.clTableShow = function( csName ){
               _DataDatabaseIndex.clTableShow( $scope, csName ) ;
            }

            //收起cs下所有的cl
            $scope.clTableHide = function( csName ){
               _DataDatabaseIndex.clTableHide( $scope, csName ) ;
            }

            //展示CS属性
            $scope.showCSInfo = function( index ){
               _DataDatabaseIndex.showCSInfo( $scope, index ) ;
            }

            //展示CL属性
            $scope.showCLInfo = function( csIndex, clIndex ){
               _DataDatabaseIndex.showCLInfo( $scope, csIndex, clIndex ) ;
            }

            //打开 创建集合空间 的窗口
            $scope.showCreateCS = function(){
               _DataDatabaseIndex.showCreateCS( $scope, SdbRest ) ;
            }

            //打开 删除集合空间 的窗口
            $scope.showRemoveCS = function(){
               _DataDatabaseIndex.showRemoveCS( $scope, SdbRest ) ;
            }

            //打开 创建集合 的窗口
            $scope.showCreateCL = function(){
               _DataDatabaseIndex.showCreateCL( $scope, SdbRest ) ;
            }

            //打开 删除集合 的窗口
            $scope.showRemoveCL = function(){
               _DataDatabaseIndex.showRemoveCL( $scope, SdbRest ) ;
            }

            //打开 挂载集合 的窗口
            $scope.showAttachCL = function(){
               _DataDatabaseIndex.showAttachCL( $scope, SdbRest ) ;
            }

            //打开 切分数据 的窗口
            $scope.showSplit = function(){
               _DataDatabaseIndex.showSplit( $scope, SdbRest ) ;
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
               $scope.Components.Modal.icon = 'fa-trash-o' ;
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

            //显示不同分区组的信息
            $scope.showGroupInfo = function( index ){
               _DataDatabaseIndex.showGroupInfo( $scope, index ) ;
            }

            _DataDatabaseIndex.getCSInfo( $scope, SdbRest ) ;
         } ) ;
      } ) ;

   } ) ;
}());