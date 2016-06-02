(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Domain.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){

      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      $scope.GridData = [] ;
      $scope.domainList = [] ;
      var gridData = {} ;

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/domainList', success, failed, error ) ;
      }

      $scope.getDomainList = function(){
         $scope.queryList( {}, function( test ){
            $.each( test, function( index, domainList ){
               $scope.domainList.push( domainList['Name'] ) ;
               gridData = {
                  'title': [
                     { "text": $scope.autoLanguage( '分区组名' ) },
                     { "text": $scope.autoLanguage( '组ID' ) },
                     { "text": $scope.autoLanguage( '集合空间' ) },
                     { "text": $scope.autoLanguage( '节点数' ) },
                     { "text": $scope.autoLanguage( '主节点' ) }
                  ],
                  'body': [],
                  'options': {
                     'grid': {  'tdModel': 'fixed', 'gridModel': 'fixed', 'tdHeight': '19px', 'titleWidth': [ 25, 15, 15, 10, 35 ] 
                     },
                     'order': {
                        'active': true
                     }
                  }
               }
               $.each( domainList['Groups'], function( key, value ){
                  gridData['body'].push( [
                     { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Group/Index">' + value['GroupName'] + '</a>' )( $scope ) },
                     { 'text': value['GroupID'] },
                     { 'text': value['TotalCS'] },
                     { 'text': value['NodeNumber'] },
                     { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Node/Index">' + value['PrimaryNode'] + '</a>' )( $scope ) }
                  ] )
               } )
               $scope.GridData.push(gridData) ;
            } )
         } )
      }
      $scope.getDomainList();
      
      //显示域详细
      $scope.showDomain = function(){
         $scope.Components.Modal.domainInfo = {
            "域名":"myDomain",
            "分区组数":"7",
            "集合空间数":"21",
            "集合数":"162",
            "记录数":"6549874",
            "是否自动切分":"true",
            "集合空间":"foo,foo1,foo2,foo3,foo4,foo5,foo1,foo2,foo3,foo4,foo5,foo1,foo2,foo3,foo4,foo5"
         }
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '域详细' ;
         $scope.Components.Modal.noOK = true ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.Context = '\
<table class="table loosen border">\
<tr>\
<td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
<td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
</tr>\
<tr>\
<td>会话ID</td>\
<td>{{data.domainInfo["域名"]}}</td>\
</tr>\
<tr ng-repeat="(key, value) in data.domainInfo">\
<td>{{key}}</td>\
<td>{{value}}</td>\
</tr>\
</table>' ;
      } ;

      //新增域
      $scope.addDomian = function(){
         //1域名 2选择分区组 3属性（是否自动切分）
         $scope.Components.Modal.icon = 'fa-plus' ;
         $scope.Components.Modal.title = '新增域' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "name",
                  "webName": $scope.autoLanguage( '域名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1,
                     "max": 127,
                     "ban": [ ".", "$" ]
                  }
               },
               {
                  "name": "autoSplit",
                  "webName": $scope.autoLanguage( '是否自动切分' ),
                  "type": "select",
                  "value": "false",
                  "valid": [
                     { "key": "false", "value": "false" },
                     { "key": "true", "value": "true" }   
                  ]
               },
               {
                  "name": "group",
                  "webName": $scope.autoLanguage( '选择分区组' ),
                  "desc": '',
                  "type": "list",
                  "valid": {
                     "min": 1
                  },
                  "child":[
                     [
                        {
                           "name": "group",
                           "type": "select",
                           "value": 'group2',
                           "valid":[
                              { "key": "group2", "value": "group2" },
                              { "key": "group3", "value": "group3" },
                              { "key": "group4", "value": "group4" },
                              { "key": "group6", "value": "group6" },
                              { "key": "group7", "value": "group7" },
                              { "key": "group8", "value": "group8" }
                           ]
                        }
                     ]
                  ]
               },
               {
                  "name": "conf",
                  "webName": $scope.autoLanguage( '配置' ),
                  "type": "string",
                  "value": ""
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            var value = $scope.Components.Modal.form.getValue() ;
            if( isAllClear  == true )
            {
               return true
            }
         }
      } ;

      //删除域
      $scope.deleteDomain = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '删除域' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList:[
               {
                  "name": "domain",
                  "webName": "域名",
                  "type": "select",
                  "value": 'myDomain1',
                  "valid":[
                     { "key": "myDomain1", "value": "myDomain1" },
                     { "key": "myDomain2", "value": "myDomain2" },
                     { "key": "myDomain3", "value": "myDomain3" },
                     { "key": "myDomain4", "value": "myDomain4" },
                     { "key": "myDomain5", "value": "myDomain5" },
                     { "key": "myDomain6", "value": "myDomain6" }
                  ]   
               }
            ]
         };
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            $scope.Components.Modal.isShow = false ;
         }
      }

      //编辑域
      $scope.editDomain = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '编辑域' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList:[
               {
                  "name": "domain",
                  "webName": "域名",
                  "type": "select",
                  "value": 'myDomain1',
                  "valid":[
                     { "key": "myDomain1", "value": "myDomain1" },
                     { "key": "myDomain2", "value": "myDomain2" },
                     { "key": "myDomain3", "value": "myDomain3" },
                     { "key": "myDomain4", "value": "myDomain4" },
                     { "key": "myDomain5", "value": "myDomain5" },
                     { "key": "myDomain6", "value": "myDomain6" }
                  ]   
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            $scope.Components.Modal.isShow = false ;
         }
      }

   } ) ;
}());