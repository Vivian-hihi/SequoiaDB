
// --------------------- Index ---------------------
var _IndexPublic = {} ;

//创建错误弹窗
_IndexPublic.createErrorModel = function( $scope, context ){
   $scope.Components.Confirm.type = 3 ;
   $scope.Components.Confirm.noOK = true ;
   $scope.Components.Confirm.noClose = true ;
   $scope.Components.Confirm.context = context ;
   $scope.Components.Confirm.isShow = true ;
}

//语言控制器
_IndexPublic.languageCtrl = function( $scope, text ){
   var newText = text ;
   if( $scope.Language == 'en' )
   {
      function setLanguage()
      {
         if( typeof( window.SdbSacLanguage[text] ) == 'undefined' )
         {
            printfDebug( '"' + text + '" 没翻译！' ) ;
         }
         else
         {
            newText = window.SdbSacLanguage[text] ;
         }
      }
      if( typeof( window.SdbSacLanguage ) == 'undefined' )
      {
         //获取语言
         $.ajax( './app/language/English.json', { 'async': false, 'success': function( reqData ){
            window.SdbSacLanguage = JSON.parse( reqData ) ;
            setLanguage() ;
         }, 'error': function( XMLHttpRequest, textStatus, errorThrown ){
            window.SdbSacLanguage = {} ;
            _IndexPublic.createErrorModel( $scope, 'Can not find the language file, please try to refresh your browser by pressing F5.' ) ;
         } } ) ;
      }
      else
      {
         setLanguage() ;
      }
   }
   return newText ;
}

//创建选择cluster弹窗
_IndexPublic.createSelectClusterModel = function( $scope, selectEvent ){
   $scope.Components.Modal.icon = '' ;
   $scope.Components.Modal.title = '选择集群' ;
   $scope.Components.Modal.noOK = false ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.clusterList = [
      { "ClusterName": "myCluster", "Desc": "这是某某业务的集群", "SdbUser": "sdbadmin", "sdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "InstallPath": "/opt/sequoiadb/" },
      { "ClusterName": "cluster_2", "Desc": "这是某某业务的集群", "SdbUser": "sdbadmin", "sdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "InstallPath": "/opt/sequoiadb/" },
      { "ClusterName": "cluster_3", "Desc": "这是某某业务的集群", "SdbUser": "sdbadmin", "sdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "InstallPath": "/opt/sequoiadb/" },
      { "ClusterName": "cluster_4", "Desc": "这是某某业务的集群", "SdbUser": "sdbadmin", "sdbPasswd": "sdbadmin", "SdbUserGroup": "sdbadmin_group", "InstallPath": "/opt/sequoiadb/" }
   ] ;
   $scope.Components.Modal.setCluster = function( name ){
      printfDebug( '当前选择Cluster为： ' + name ) ;
      if( typeof( selectEvent ) == 'function' )
      {
         selectEvent( name ) ;
      }
   }
   $scope.Components.Modal.Context = '<table class="table loosen border">\
   <tbody>\
      <tr style="font-weight:bold;">\
         <td style="width:40%;background-color:#F1F4F5;">Cluster</td>\
         <td style="width:30%;background-color:#F1F4F5;">描述</td>\
         <td style="width:15%;background-color:#F1F4F5;">主机数</td>\
         <td style="width:15%;background-color:#F1F4F5;">业务数</td>\
      </tr>\
      <tr ng-repeat="clusterInfo in data.clusterList track by $index">\
         <td><a class="linkButton" ng-click="data.setCluster(clusterInfo.ClusterName)">{{clusterInfo.ClusterName}}</a></td>\
         <td>{{clusterInfo.Desc}}</td>\
         <td>3</td>\
         <td>3</td>\
      </tr>\
   </tbody>\
</table>' ;
}

//创建选择module弹窗
_IndexPublic.createSelectModuleModel = function( $scope, selectEvent ){
   $scope.Components.Modal.icon = '' ;
   $scope.Components.Modal.title = '选择业务' ;
   $scope.Components.Modal.noOK = false ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.moduleList = [
      { "BusinessName": "myModule", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_1" },
      { "BusinessName": "TestModule_2", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_1" },
      { "BusinessName": "TestModule_3", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_2" },
      { "BusinessName": "TestModule_4", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_2" },
      { "BusinessName": "TestModule_5", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_2" },
      { "BusinessName": "TestModule_6", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" },
      { "BusinessName": "TestModule_7", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" },
      { "BusinessName": "TestModule_8", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" },
      { "BusinessName": "TestModule_9", "BusinessType": "sequoiadb", "DeployMod": "distribution", "ClusterName": "cluster_3" }
   ] ;
   $scope.Components.Modal.setModule = function( name ){
      printfDebug( '当前选择业务为： ' + name ) ;
      if( typeof( selectEvent ) == 'function' )
      {
         selectEvent( name ) ;
      }
   }
   $scope.Components.Modal.Context = '<table class="table loosen border">\
   <tbody>\
      <tr style="font-weight:bold;">\
         <td style="width:40%;background-color:#F1F4F5;">Module Name</td>\
         <td style="width:30%;background-color:#F1F4F5;">DeployMod</td>\
         <td style="width:30%;background-color:#F1F4F5;">Cluster</td>\
      </tr>\
      <tr ng-repeat="moduleInfo in data.moduleList track by $index">\
         <td><a class="linkButton" ng-click="data.setModule(moduleInfo.BusinessName)">{{moduleInfo.BusinessName}}</a></td>\
         <td>{{moduleInfo.DeployMod}}</td>\
         <td>{{moduleInfo.ClusterName}}</td>\
      </tr>\
   </tbody>\
</table>' ;
}

// --------------------- Index.Bottom ---------------------
var _IndexBottom = {} ;

//获取系统时间
_IndexBottom.getSystemTime = function( $scope )
{
   var times = $.now() ;
   setInterval( function(){
      var date = new Date( times ) ;
      var year = date.getFullYear() ;
      var hour = date.getHours() ;
      var minute = date.getMinutes() ;
      var second = date.getSeconds() ;
      $scope.Bottom.year = year ;
      $scope.Bottom.nowtime = pad( hour, 2 ) + ':' + pad( minute, 2 ) + ':' + pad( second, 2 ) ;
      $scope.$apply() ;
      times += 1000 ;
   }, 1000 ) ;
}

//获取ping值
_IndexBottom.checkPing = function( $scope, SdbRest )
{
   SdbRest.getPing( function( times ){
      if( times >= 0 )
      {
         $scope.Bottom.sysStatus = $scope.autoLanguage( '良好' ) ;
         $scope.Bottom.statusColor = 'success' ;
         setTimeout( function(){
            _IndexBottom.checkPing( $scope, SdbRest ) ;
         }, 5000 ) ;
      }
      else
      {
         $scope.Bottom.sysStatus = $scope.autoLanguage( '网络错误' ) ;
         $scope.Bottom.statusColor = 'error' ;
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }
      $scope.$apply() ;
   } ) ;
}

// --------------------- Data.Operate.Record ---------------------
var _DataOperateRecord = {} ;

//构造json
_DataOperateRecord.buildJsonGrid = function( $scope, $compile, records, isNotFilter )
{
   $scope.showType = 1 ;
   var gridData = {
      'title': [ { 'text': '' }, { 'text': '' }, { 'text': 'Record' } ],
      'body': [],
      'tool': {
         'position': 'bottom',
         'left': [
            { 'html': $compile( '<i class="fa fa-font"  ng-class="{ active: showType == 1 }" ng-click="show(1)"></i>' )( $scope ) },
            { 'html': $compile( '<i class="fa fa-list" ng-class="{ active: showType == 2 }" ng-click="show(2)"></i>' )( $scope ) },
            { 'html': $compile( '<i class="fa fa-table"  ng-class="{ active: showType == 3 }" ng-click="show(3)"></i>' )( $scope ) }
         ],
         'right': [
            { 'html': $compile( '<span ng-bind="recordTotal"></span>' )( $scope ) }
         ]
      },
      'options': {
         'grid': { 'tdModel': 'auto', 'titleWidth': [ '60px', '80px', 100 ] }
      }
   } ;
   if( isNotFilter )
   {
      gridData.tool.left.push( {} ) ;
      gridData.tool.left.push( {} ) ;
      gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play fa-flip-horizontal" ng-show="current > 1" ng-click="previous()"></i>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<input style="width:100px;" ng-change="checkCurrent()" ng-model="setCurrent" ng-keypress="gotoPate($event)">' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<span>/<span>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<span ng-bind="total"></span>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play" ng-show="current < total" ng-click="nextPage()"></i>' )( $scope ) } ) ;
   }
   $.each( records, function( index, record ){
      var tmpJson = JSON.stringify( record, null, 3 ) ;
      var editBtn = $compile( '<a ng-click="Edit(' + index + ')"></a>' )( $scope ).addClass( 'linkButton' ).append( $( '<i class="fa fa-edit"></i>' ).attr( 'data-desc', $scope.autoLanguage( '编辑' ) ) ) ;
      var copyBtn = $compile( '<a ng-click="Insert(' + index + ')"></a>' )( $scope ).addClass( 'linkButton' ).append( $( '<i class="fa fa-copy"></i>' ).attr( 'data-desc', $scope.autoLanguage( '复制' ) ) ) ;
      var deleteBtn = $compile( '<a ng-click="DeleteRecord(' + index + ')" ></a>' )( $scope ).addClass( 'linkButton' ).append( $( '<i class="fa fa-remove"></i>' ).attr( 'data-desc', $scope.autoLanguage( '删除' ) ) ) ;
      gridData['body'].push( [
         { 'text': ( index + 1 ) },
         { 'html': $compile( '<span></span>' )( $scope ).append( editBtn ).append( '&nbsp;&nbsp;' ).append( copyBtn ).append( '&nbsp;&nbsp;' ).append( deleteBtn ), 'ellipsis': false },
         { 'text': tmpJson, 'ellipsis': false } ] ) ;
   } ) ;
   $scope.GridData = gridData ;
}

//构造树
_DataOperateRecord.buildTreeGrid = function( $scope, $compile, records, isNotFilter )
{
   $scope.showType = 2 ;
   var gridData = {
      'title': [
         { 'text': 'Key' }, { 'text': 'Value' }, { 'text': 'Type' }
      ],
      'body': [],
      'tool': {
         'position': 'bottom',
         'left': [
            { 'html': $compile( '<i class="fa fa-font" ng-class="{ active: showType == 1 }" ng-click="show(1)"></i>' )( $scope ) },
            { 'html': $compile( '<i class="fa fa-list" ng-class="{ active: showType == 2 }" ng-click="show(2)"></i>' )( $scope ) },
            { 'html': $compile( '<i class="fa fa-table" ng-class="{ active: showType == 3 }" ng-click="show(3)"></i>' )( $scope ) }
         ],
         'right': [
            { 'html': $compile( '<span ng-bind="recordTotal"></span>' )( $scope ) }
         ]
      },
      'options': {
         'grid': { 'tdModel': 'dynamic' },
         'event': {
            'onResize': function( column, line, width, height ){
               if( column == 0 )
               {
                  var id = line ;
                  gridData['data'][id]['width'] = parseInt( width ) ;
               }
            }
         }
      },
      'data': []
   } ;
   if( isNotFilter )
   {
      gridData.tool.left.push( {} ) ;
      gridData.tool.left.push( {} ) ;
      gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play fa-flip-horizontal" ng-show="current > 1" ng-click="previous()"></i>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<input type="text" style="width:100px;" ng-change="checkCurrent()" ng-model="setCurrent" ng-keypress="gotoPate($event)">' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<span>/<span>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<span ng-bind="total"></span>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play" ng-show="current < total" ng-click="nextPage()"></i>' )( $scope ) } ) ;
   }
   $.each( records, function( index, record ){
      var index2 = gridData['data'].length ;
      var line = json2Array( record, 0, true ) ;
      gridData['data'].push( { 'Json': line, 'index': index + 1, width: 100 } ) ;
      gridData['body'].push( [ { 'html': '<div tree-key para="data.data[' + index2 + ']"></div>', 'ellipsis': false }, { 'html': '<div tree-value para="data.data[' + index2 + ']"></div>', 'ellipsis': false }, { 'html': '<div tree-type para="data.data[' + index2 + ']"></div>', 'ellipsis': false } ] ) ;
   } ) ;
   $scope.GridData = gridData ;
}

//构造表格
_DataOperateRecord.buildTableGrid = function( $scope, $compile, SdbFunction, records, isNotFilter )
{
   $scope.showType = 3 ;
   var gridData = {
      'title': [],
      'body': [],
      'tool': {
         'position': 'bottom',
         'left': [
            { 'html': $compile( '<i class="fa fa-font"  ng-class="{ active: showType == 1 }" ng-click="show(1)"></i>' )( $scope ) },
            { 'html': $compile( '<i class="fa fa-list"  ng-class="{ active: showType == 2 }" ng-click="show(2)"></i>' )( $scope ) },
            { 'html': $compile( '<i class="fa fa-table" ng-class="{ active: showType == 3 }" ng-click="show(3)"></i>' )( $scope ) }
         ],
         'right': [
            { 'html': $compile( '<span ng-bind="recordTotal"></span>' )( $scope ) }
         ]
      },
      'options': {
         'grid': { 'tdModel': 'fixed', 'tdHeight': '19px', 'titleWidth': [ '60px' ] }
      }
   } ;
   if( isNotFilter )
   {
      gridData.tool.left.push( {} ) ;
      gridData.tool.left.push( {} ) ;
      gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play fa-flip-horizontal" ng-show="current > 1" ng-click="previous()"></i>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<input style="width:100px;" ng-change="checkCurrent()" ng-model="setCurrent" ng-keypress="gotoPate($event)">' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<span>/<span>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<span ng-bind="total"></span>' )( $scope ) } ) ;
      gridData.tool.left.push( { 'html': $compile( '<i class="fa fa-play" ng-show="current < total" ng-click="nextPage()"></i>' )( $scope ) } ) ;
   }
   var keyList = [ '' ] ;
   //取得json的所有键
   $.each( records, function( index, record ){
      keyList = SdbFunction.getJsonKeys( record, 0, keyList ) ;
   } ) ;
   //计算列宽
   var titleWidth = parseInt( 100 / ( keyList.length - 1 ) ) ;
   $.each( keyList, function( index, key ){
      //填写标题
      gridData['title'].push( { 'text': key } ) ;
      if( index > 0 )
      {
         //设置宽度
         gridData.options.grid.titleWidth.push( titleWidth ) ;
      }
   } ) ;
   //取得json的所有值
   $.each( records, function( index, record ){
      var line = [] ;
      line = SdbFunction.getJsonValues( record, keyList, line ) ;
      line[0] = index + 1 ;
      var newRow = [] ;
      $.each( line, function( index, value ){
         newRow.push( { 'text': value } ) ;
      } ) ;
      gridData['body'].push( newRow ) ;
   } ) ;
   $scope.GridData = gridData ;
}

//创建插入操作弹窗
_DataOperateRecord.createInsertModel = function( $scope, SdbRest, queryFilter, recordIndex, fullName, queryRecord, records ){
   $scope.Components.Modal.icon = 'fa-plus' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '插入记录' ) ;
   $scope.Components.Modal.isShow = true ;
   if( typeof( recordIndex ) == 'undefined' )
   {
      $scope.Components.Modal.jsonEdit = { Json: {}, Height: 0 } ;
   }
   else
   {
      var newObject = jQuery.extend( true, {}, records[recordIndex] ) ;
      delete newObject['_id'] ;
      $scope.Components.Modal.jsonEdit = { Json: newObject, Height: 0 } ;
   }
   $scope.Components.Modal.Context = '<div json-edit para="data.jsonEdit"></div>' ;
   $scope.Components.Modal.ok = function(){
      var str = JSON.stringify( $scope.Components.Modal.jsonEdit.Callback.getJson() ) ;
      var data = { 'cmd': 'insert', 'name': fullName, 'insertor': str } ;
      SdbRest.DataOperation( data, function( json ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 插入记录成功' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName ) ;
         $scope.execRc = true ;
         queryRecord( queryFilter, $scope.showType, false ) ;
      }, function( errorInfo ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 插入记录失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
         $scope.execRc = false ;
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }, function(){
         //关闭弹窗
         $scope.Components.Modal.isShow = false ;
         $scope.$apply() ;
      } ) ;
      return false ;
   }
   $scope.Components.Modal.onResize = function( width, height ){
      $scope.Components.Modal.jsonEdit.Height = height ;
   }
}

//创建编辑操作弹窗
_DataOperateRecord.createEditModel = function( $scope, SdbRest, queryFilter, recordIndex, fullName, queryRecord, records ){
   $scope.Components.Modal.icon = 'fa-edit' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '编辑记录' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.jsonEdit = { Json: records[recordIndex], Height: 0 } ;
   $scope.Components.Modal.Context = '<div json-edit para="data.jsonEdit"></div>' ;
   $scope.Components.Modal.ok = function(){
      var filter = JSON.stringify( { '_id': records[recordIndex]['_id'] } ) ;
      var newRecord = $scope.Components.Modal.jsonEdit.Callback.getJson() ;
      delete newRecord['_id'] ;
      var updator = JSON.stringify( { '$replace': newRecord } ) ;
      var data = { 'cmd': 'update', 'name': fullName, 'updator': updator, 'filter': filter } ;
      SdbRest.DataOperation( data, function( json ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 更新成功' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName ) ;
         $scope.execRc = true ;
         queryRecord( queryFilter, $scope.showType, false ) ;
      }, function( errorInfo ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 更新失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
         $scope.execRc = false ;
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }, function(){
         //关闭弹窗
         $scope.Components.Modal.isShow = false ;
         $scope.$apply() ;
      } ) ;
      return false ;
   }
   $scope.Components.Modal.onResize = function( width, height ){
      $scope.Components.Modal.jsonEdit.Height = height ;
      $scope.$apply() ;
   }
}

//创建查询操作弹窗
_DataOperateRecord.createQueryModel = function( $scope, fieldList, fullName, queryRecord )
{
   $scope.Components.Modal.icon = 'fa-search' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '查询' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "filter",
            "webName": $scope.autoLanguage( "查询条件" ),
            "type": "group",
            "child": [
               {
                  "name": "model",
                  "webName": $scope.autoLanguage( "匹配模式" ),
                  "type": "select",
                  "value": "and",
                  "valid": [
                     { "key": $scope.autoLanguage( "满足所有条件" ), "value": "and" },
                     { "key": $scope.autoLanguage( "满足任一条件" ), "value": "or" }
                  ]
               },
               {
                  "name": "condition",
                  "webName": $scope.autoLanguage( "匹配条件" ),
                  "type": "list",
                  "valid": {
                     "min": 0
                  },
                  "child": [
                     [
                        {
                           "name": "field",
                           "webName": $scope.autoLanguage( "字段名" ),
                           "placeholder": $scope.autoLanguage( "字段名" ),
                           "type": "string",
                           "value": "",
                           "valid": {
                              "min": 1,
                              "regex": "^[^/$].*",
                              "ban": "."
                           },
                           "selectList": fieldList
                        },
                        {
                           "name": "logic",
                           "type": "select",
                           "value": ">",
                           "valid": [
                              { "key": ">", "value": ">" },
                              { "key": ">=", "value": ">=" },
                              { "key": "<", "value": "<" },
                              { "key": "<=", "value": "<=" },
                              { "key": "!=", "value": "!=" },
                              { "key": "=", "value": "=" }
                           ]
                        },
                        {
                           "name": "value",
                           "webName": $scope.autoLanguage( "值" ),
                           "placeholder": $scope.autoLanguage( "值" ),
                           "type": "string",
                           "value": "",
                           "valid": {
                              "min": 0,
                              "max": 127
                           }
                        }
                     ]
                  ]
               }
            ]
         },
         {
            "name": "selector",
            "webName": $scope.autoLanguage( "选择字段" ),
            "type": "list",
            "valid": {
               "min": 0
            },
            "child": [
               [
                  {
                     "name": "field",
                     "webName": $scope.autoLanguage( "字段名" ), 
                     "placeholder": $scope.autoLanguage( "字段名" ),
                     "type": "string",
                     "valid": {
                        "min": 1,
                        "regex": "^[^/$].*",
                        "ban": "."
                     },
                     "value": "",
                     "selectList": fieldList
                  }
               ]
            ]
         },
         {
            "name": "sort",
            "webName": $scope.autoLanguage( "排序字段" ),
            "type": "list",
            "valid": {
               "min": 0,
               "max": 100
            },
            "child": [
               [
                  {
                     "name": "field",
                     "webName": $scope.autoLanguage( "字段名" ),
                     "placeholder": $scope.autoLanguage( "字段名" ),
                     "type": "string",
                     "valid": {
                        "min": 1,
                        "regex": "^[^/$].*",
                        "ban": "."
                     },
                     "value": "",
                     "selectList": fieldList
                  },
                  {
                     "name": "order",
                     "type": "select",
                     "value": "1",
                     "valid": [
                        { "key": $scope.autoLanguage( "升序" ), "value": "1" },
                        { "key": $scope.autoLanguage( "降序" ), "value": "-1" }
                     ]
                  }
               ]
            ]
         },
         {
            "name": "hint",
            "webName": $scope.autoLanguage( "扫描方式" ),
            "type": "select",
            "value": "1",
            "valid": [
               { "key": "表扫描", "value": "1" },
               { "key": "索引扫描", "value": "0" }
            ]
         },
         {
            "name": "returnnum",
            "webName": $scope.autoLanguage( "返回记录数" ),
            "required": true,
            "type": "int",
            "valid": {
               "min": 1,
               "max": 100
            },
            "value": 30
         },
         {
            "name": "skip",
            "webName": $scope.autoLanguage( "跳过记录数" ),
            "required": true,
            "type": "int",
            "valid": {
               "min": 0
            },
            "value": 0
         }
      ]
   } ;
   $scope.Components.Modal.form2 = {
      inputList: [
         {
            "name": "returnnum",
            "webName": $scope.autoLanguage( "返回记录数" ),
            "required": true,
            "type": "int",
            "valid": {
               "min": 1,
               "max": 100
            },
            "value": 30
         },
         {
            "name": "skip",
            "webName": $scope.autoLanguage( "跳过记录数" ),
            "required": true,
            "type": "int",
            "valid": {
               "min": 0
            },
            "value": 0
         }
      ]
   } ;
   $scope.Components.Modal.tab = 1 ;
   $scope.Components.Modal.filter = { Json: {}, Height: 0 } ;
   $scope.Components.Modal.selector = { Json: {}, Height: 0 } ;
   $scope.Components.Modal.sort = { Json: {}, Height: 0 } ;
   $scope.Components.Modal.hint = { Json: {}, Height: 0 } ;
   $scope.Components.Modal.Context = '\
<div class="underlineTab" style="padding-bottom:20px;">\
   <ul class="left">\
      <li ng-class="{active:data.tab == 1}">\
         <a ng-click="data.tab = 1">' + $scope.autoLanguage( '快速查询' ) + '</a>\
      </li>\
      <li ng-class="{active:data.tab == 2}">\
         <a ng-click="data.tab = 2">' + $scope.autoLanguage( '高级查询' ) + '</a>\
      </li>\
   </ul>\
</div>\
<div ng-show="data.tab == 1" form-create para="data.form"></div>\
<table ng-show="data.tab == 2" class="table loosen">\
   <tr>\
      <td style="width:130px;vertical-align:top;">' + $scope.autoLanguage( '查询条件' ) + '</td>\
      <td><div json-edit para="data.filter"></div></td>\
   </tr>\
   <tr>\
      <td style="width:130px;vertical-align:top;">' + $scope.autoLanguage( '选择字段' ) + '</td>\
      <td><div json-edit para="data.selector"></div></td>\
   </tr>\
   <tr>\
      <td style="width:130px;vertical-align:top;">' + $scope.autoLanguage( '排序字段' ) + '</td>\
      <td><div json-edit para="data.sort"></div></td>\
   </tr>\
   <tr>\
      <td style="width:130px;vertical-align:top;">' + $scope.autoLanguage( '扫描方式' ) + '</td>\
      <td><div json-edit para="data.hint"></div></td>\
   </tr>\
   <tr>\
      <td colspan="2"><div form-create para="data.form2"></div></td>\
   </tr>\
</table>' ;
   $scope.Components.Modal.ok = function(){
      if( $scope.Components.Modal.tab == 1 )
      {
         var isAllClear = $scope.Components.Modal.form.check() ;
         if( isAllClear )
         {
            function modalValue2Query( valueJson )
            {
               var filter = {} ;
               var selector = {} ;
               var sort = {} ;
               var hint = {} ;
               var returnnum = 30 ;
               var skip = 0 ;
               //filter
               {
                  var jobj = valueJson['filter'] ;
                  if( jobj['condition'].length > 1 || ( jobj['condition'].length == 1 && jobj['condition'][0]['field'].length > 0 ) )
                  {
                     $.each( jobj['condition'], function( index, field ){
                        var fieldValue = autoTypeConvert( field['value'], true ) ;
                        switch( field['logic'] )
                        {
                        case '>':
                           filter[ field['field'] ] = { '$gt': fieldValue } ;
                           break ;
                        case '>=':
                           filter[ field['field'] ] = { '$gte': fieldValue } ;
                           break ;
                        case '<':
                           filter[ field['field'] ] = { '$lt': fieldValue } ;
                           break ;
                        case '<=':
                           filter[ field['field'] ] = { '$lte': fieldValue } ;
                           break ;
                        case '!=':
                           filter[ field['field'] ] = { '$ne': fieldValue } ;
                           break ;
                        case '=':
                           filter[ field['field'] ] = fieldValue ;
                           break ;
                        }
                     } ) ;
                     if( jobj['model'] == 'or' )
                     {
                        filter = { '$or': filter } ;
                     }
                  }
               }
               //selector
               {
                  var jobj = valueJson['selector'] ;
                  if( jobj.length > 1 || ( jobj.length == 1 && jobj[0]['field'].length > 0 ) )
                  {
                     $.each( jobj, function( index, field ){
                        selector[ field['field'] ] = 1 ;
                     } ) ;
                  }
               }
               //sort
               {
                  var jobj = valueJson['sort'] ;
                  if( jobj.length > 1 || ( jobj.length == 1 && jobj[0]['field'].length > 0 ) )
                  {
                     $.each( jobj, function( index, field ){
                        sort[ field['field'] ] = autoTypeConvert( field['order'], false )  ;
                     } ) ;
                  }
               }
               // number to return
               {
                  returnnum = valueJson['returnnum'] ;
               }
               // skip
               {
                  skip = valueJson['skip'] ;
               }
               //组装
               var returnJson = {} ;
               if( $.isEmptyObject( filter ) == false )
               {
                  returnJson['filter'] = JSON.stringify( filter ) ;
               }
               if( $.isEmptyObject( selector ) == false )
               {
                  returnJson['selector'] = JSON.stringify( selector ) ;
               }
               if( $.isEmptyObject( sort ) == false )
               {
                  returnJson['sort'] = JSON.stringify( sort ) ;
               }
               returnJson['returnnum'] = returnnum ;
               returnJson['skip'] = skip ;
               return returnJson ;
            }
            var value = $scope.Components.Modal.form.getValue() ;
            alert( JSON.stringify( value ) )
            var data = modalValue2Query( value ) ;
            data['name'] = fullName ;
            queryRecord( data, $scope.showType ) ;
         }
         return isAllClear ;
      }
      else
      {
         var isAllClear = $scope.Components.Modal.form2.check() ;
         if( isAllClear )
         {
            var filter = $scope.Components.Modal.filter.Callback.getJson() ;
            var selector = $scope.Components.Modal.selector.Callback.getJson() ;
            var sort = $scope.Components.Modal.sort.Callback.getJson() ;
            var hint = $scope.Components.Modal.hint.Callback.getJson() ;
            var data = $scope.Components.Modal.form2.getValue() ;
            if( $.isEmptyObject( filter ) == false )
            {
               data['filter'] = JSON.stringify( filter ) ;
            }
            if( $.isEmptyObject( selector ) == false )
            {
               data['selector'] = JSON.stringify( selector ) ;
            }
            if( $.isEmptyObject( sort ) == false )
            {
               data['sort'] = JSON.stringify( sort ) ;
            }
            if( $.isEmptyObject( hint ) == false )
            {
               data['hint'] = JSON.stringify( hint ) ;
            }
            data['name'] = fullName ;
            queryRecord( data, $scope.showType ) ;
         }
         return isAllClear ;
      }
   }
   $scope.Components.Modal.onResize = function( width, height ){
      height = height - 30 ;
      height = parseInt( height / 4 ) ;
      if( height < 160 ) height = 160 ;
      $scope.Components.Modal.filter.Height = height ;
      $scope.Components.Modal.selector.Height = height ;
      $scope.Components.Modal.sort.Height = height ;
      $scope.Components.Modal.hint.Height = height ;
   }
}

//创建更新操作弹窗
_DataOperateRecord.createUpdateModel = function( $scope, SdbRest, queryFilter, fieldList, fullName, queryRecord ){
   $scope.Components.Modal.icon = 'fa-edit' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '更新' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "filter", "webName": $scope.autoLanguage( "更新条件" ), "type": "group",
            "child": [
               {
                  "name": "model",
                  "webName": $scope.autoLanguage( "匹配模式" ),
                  "type": "select",
                  "value": "and",
                  "valid": [
                     { "key": $scope.autoLanguage( "满足所有条件" ), "value": "and" },
                     { "key": $scope.autoLanguage( "满足任意条件" ), "value": "or" }
                  ]
               },
               {
                  "name": "condition",
                  "webName": $scope.autoLanguage( "匹配条件" ),
                  "type": "list",
                  "valid": {
                     "min": 0
                  },
                  "child": [
                     [
                        {
                           "name": "field",
                           "webName": $scope.autoLanguage( "字段名" ),
                           "placeholder": $scope.autoLanguage( "字段名" ),
                           "type": "string",
                           "value": "",
                           "valid": {
                              "min": 1,
                              "regex": "^[^/$].*",
                              "ban": "."
                           },
                           "selectList": fieldList
                        },
                        {
                           "name": "logic",
                           "type": "select",
                           "value": ">",
                           "valid": [
                              { "key": ">", "value": ">" },
                              { "key": ">=", "value": ">=" },
                              { "key": "<", "value": "<" },
                              { "key": "<=", "value": "<=" },
                              { "key": "!=", "value": "!=" },
                              { "key": "=", "value": "=" }
                           ]
                        },
                        {
                           "name": "value",
                           "webName": $scope.autoLanguage( "值" ),
                           "placeholder": $scope.autoLanguage( "值" ),
                           "type": "string",
                           "value": "",
                           "valid": {
                              "min": 0,
                              "max": 127
                           }
                        }
                     ]
                  ]
               }
            ]
         },
         {
            "name": "updator",
            "webName": $scope.autoLanguage( "更新操作" ),
            "type": "list",
            "desc": "",
            "required": true,
            "valid": {
               "min": 1
            },
            "child": [
               [
                  {
                     "name": "field",
                     "webName": $scope.autoLanguage( "字段名" ),
                     "placeholder": $scope.autoLanguage( "字段名" ),
                     "type": "string",
                     "value": "",
                     "valid": {
                        "min": 1,
                        "regex": "^[^/$].*",
                        "ban": "."
                     },
                     "selectList": fieldList
                  },
                  {
                     "name": "value",
                     "webName": $scope.autoLanguage( "值" ),
                     "placeholder": $scope.autoLanguage( "值" ),
                     "type": "string",
                     "value": "",
                     "valid": {}
                  }
               ]
            ]
         }
      ]
   } ;
   $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
   $scope.Components.Modal.ok = function(){
      var isAllClear = $scope.Components.Modal.form.check() ;
      if( isAllClear )
      {
         function modalValue2Update( valueJson )
         {
            var filter = {} ;
            var updator = {} ;
            //filter
            {
               var jobj = valueJson['filter'] ;
               if( jobj['condition'].length > 1 ||( jobj['condition'].length == 1 && jobj['condition'][0]['field'].length > 0 ) )
               {
                  $.each( jobj['condition'], function( index, field ){
                     switch( field['logic'] )
                     {
                     case '>':
                        filter[ field['field'] ] = { '$gt': field['value'] } ;
                        break ;
                     case '>=':
                        filter[ field['field'] ] = { '$gte': field['value'] } ;
                        break ;
                     case '<':
                        filter[ field['field'] ] = { '$lt': field['value'] } ;
                        break ;
                     case '<=':
                        filter[ field['field'] ] = { '$lte': field['value'] } ;
                        break ;
                     case '!=':
                        filter[ field['field'] ] = { '$ne': field['value'] } ;
                        break ;
                     case '=':
                        filter[ field['field'] ] = field['value'] ;
                        break ;
                     }
                  } ) ;
                  if( jobj['model'] == 'or' )
                  {
                     filter = { '$or': filter } ;
                  }
               }
            }
            //updator
            {
               var jobj = valueJson['updator'] ;
               $.each( jobj, function( index, field ){
                  updator[ field['field'] ] = field['value'] ;
               } ) ;
               updator = { '$set': updator } ;
            }
            //组装
            var returnJson = {} ;
            if( $.isEmptyObject( filter ) == false )
            {
               returnJson['filter'] = JSON.stringify( filter ) ;
            }
            if( $.isEmptyObject( updator ) == false )
            {
               returnJson['updator'] = JSON.stringify( updator ) ;
            }
            return returnJson ;
         }
         var value = $scope.Components.Modal.form.getValue() ;
         var data = modalValue2Update( value ) ;
         data['cmd'] = 'update' ;
         data['name'] = fullName ;
         SdbRest.DataOperation( data, function( json ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 更新成功' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName ) ;
            $scope.execRc = true ;
            queryRecord( queryFilter, $scope.showType, false ) ;
         }, function( errorInfo ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 更新失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
            $scope.execRc = false ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         }, function(){
            //关闭弹窗
            $scope.Components.Modal.isShow = false ;
            $scope.$apply() ;
         } ) ;
      }
      return false ;
   }
}

//创建删除操作弹窗
_DataOperateRecord.createDeleteModel = function( $scope, SdbRest, queryFilter, fieldList, fullName, queryRecord ){
   $scope.Components.Modal.icon = 'fa-edit' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '更新' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "filter", "webName": $scope.autoLanguage( "删除条件" ), "type": "group",
            "child": [
               {
                  "name": "model", "webName": $scope.autoLanguage( "匹配模式" ), "type": "select", "value": "and",
                  "valid": [
                     { "key": $scope.autoLanguage( "满足所有条件" ), "value": "and" },
                     { "key": $scope.autoLanguage( "满足任意条件" ), "value": "or" }
                  ]
               },
               {
                  "name": "condition", "webName": $scope.autoLanguage( "匹配条件" ), "type": "list", "valid": { "min": 0 },
                  "child": [
                     [
                        {
                           "name": "field", "webName": $scope.autoLanguage( "字段名" ),
                           "placeholder": $scope.autoLanguage( "字段名" ), "type": "string", "value": "",
                           "valid": {
                              "min": 1, "regex": "^[^/$].*", "ban": "."
                           },
                           "selectList": fieldList
                        },
                        {
                           "name": "logic", "type": "select", "value": ">",
                           "valid": [
                              { "key": ">", "value": ">" },
                              { "key": ">=", "value": ">=" },
                              { "key": "<", "value": "<" },
                              { "key": "<=", "value": "<=" },
                              { "key": "!=", "value": "!=" },
                              { "key": "=", "value": "=" }
                           ]
                        },
                        {
                           "name": "value", "webName": $scope.autoLanguage( "值" ),
                           "placeholder": $scope.autoLanguage( "值" ), "type": "string"
                        }
                     ]
                  ]
               }
            ]
         }
      ]
   } ;
   $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
   $scope.Components.Modal.ok = function(){
      var isAllClear = $scope.Components.Modal.form.check() ;
      if( isAllClear )
      {
         function modalValue2Delete( valueJson )
         {
            var filter = {} ;
            //filter
            {
               var jobj = valueJson['filter'] ;
               if( jobj['condition'].length > 1 ||( jobj['condition'].length == 1 && jobj['condition'][0]['field'].length > 0 ) )
               {
                  $.each( jobj['condition'], function( index, field ){
                     switch( field['logic'] )
                     {
                     case '>':
                        filter[ field['field'] ] = { '$gt': field['value'] } ;
                        break ;
                     case '>=':
                        filter[ field['field'] ] = { '$gte': field['value'] } ;
                        break ;
                     case '<':
                        filter[ field['field'] ] = { '$lt': field['value'] } ;
                        break ;
                     case '<=':
                        filter[ field['field'] ] = { '$lte': field['value'] } ;
                        break ;
                     case '!=':
                        filter[ field['field'] ] = { '$ne': field['value'] } ;
                        break ;
                     case '=':
                        filter[ field['field'] ] = field['value'] ;
                        break ;
                     }
                  } ) ;
                  if( jobj['model'] == 'or' )
                  {
                     filter = { '$or': filter } ;
                  }
               }
            }
            //组装
            var returnJson = {} ;
            if( $.isEmptyObject( filter ) == false )
            {
               returnJson['deletor'] = JSON.stringify( filter ) ;
            }
            return returnJson ;
         }
         var value = $scope.Components.Modal.form.getValue() ;
         var data = modalValue2Delete( value ) ;
         data['cmd'] = 'delete' ;
         data['name'] = fullName ;
         SdbRest.DataOperation( data, function( json ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 删除成功' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName ) ;
            $scope.execRc = true ;
            queryRecord( queryFilter, $scope.showType, false ) ;
         }, function( errorInfo ){
            $scope.execResult = sprintf( $scope.autoLanguage( '? ? 删除失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
            $scope.execRc = false ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         }, function(){
            //关闭弹窗
            $scope.Components.Modal.isShow = false ;
            $scope.$apply() ;
         } ) ;
      }
      return false ;
   }
}

//创建删除记录操作弹窗
_DataOperateRecord.createDeleteRecordModel = function( $scope, SdbRest, queryFilter, recordIndex, fullName, queryRecord, records ){
   var _id = records[recordIndex]['_id']['$oid'] ;
   $scope.Components.Confirm.isShow = true ;
   $scope.Components.Confirm.type = 1 ;
   $scope.Components.Confirm.okText = $scope.autoLanguage( '是的，删除' ) ;
   $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
   $scope.Components.Confirm.title = $scope.autoLanguage( '要删除这条记录吗？' ) ;
   $scope.Components.Confirm.context = '_id : ' + _id ;
   $scope.Components.Confirm.ok = function(){
      var deletor = JSON.stringify( { '_id': { '$oid': _id } } ) ;
      var data = { 'cmd': 'delete', 'name': fullName, 'deletor': deletor } ;
      SdbRest.DataOperation( data, function( json ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 删除成功' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName ) ;
         $scope.execRc = true ;
         queryRecord( queryFilter, $scope.showType, false ) ;
      }, function( errorInfo ){
         $scope.execResult = sprintf( $scope.autoLanguage( '? ? 删除失败，错误码: ?，?. ?' ), timeFormat( new Date(), 'hh:mm:ss' ), fullName, errorInfo['errno'], errorInfo['description'], errorInfo['detail'] ) ;
         $scope.execRc = false ;
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      }, function(){
         //关闭弹窗
         $scope.Components.Modal.isShow = false ;
         $scope.$apply() ;
      } ) ;
      $scope.Components.Confirm.isShow = false ;
   }
}