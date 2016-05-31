(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;

      //初始化
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      $scope.coordInfo = [] ;
      $scope.catalogInfo = [] ;
      $scope.dataInfo = [] ;
      $scope.dataNumber = 0 ;
      $scope.groupList = [] ;
      $scope.waitingGroupList = [] ;
      $scope.runningGroupList = [] ;
      var statusIcon = '' ;

      
      $scope.queryList = function( data, success, failed, error, complete ){
         SdbRest._postTest( './test/groupList', success, failed, error ) ;
      } 
      var gridData = {
         'title': [
            { "text": $scope.autoLanguage( '状态' ) },
            { "text": $scope.autoLanguage( '分区组名' ) },
            { "text": $scope.autoLanguage( '节点数' ) },
            { "text": $scope.autoLanguage( '主节点' ) },
            { "text": $scope.autoLanguage( '集合数' ) },
            { "text": $scope.autoLanguage( '记录数' ) }
         ],
         'body': [],
         'tool': {
            'position': 'bottom',
            'left': [ { 'text': '一共有 9 个数据组' } ],
            'right': [ ]
         },
         'options': {
            'grid': {
               'tdModel': 'auto', 
               'gridModel': 'fix', 
               'tdHeight': '19px', 
               'titleWidth': [ '60px', 35, '60px', 35, 15, 15 ] 
            },
            'order': {
               'active': true
            }
         }
      } ;
      $scope.GridData = $.extend( true, {}, gridData ) ;

      $scope.getGroupList = function(){
         $scope.queryList( {}, function( test ){
            $scope.GridData = $.extend( true, {}, gridData ) ;
            $.each( test, function( index, value ){
               if( value['GroupType'] == 'coord' )
               {
                  $scope.coordInfo.push( value ) ;
               }
               else if( value['GroupType'] == 'catalog' )
               {
                  $scope.catalogInfo.push( value ) ;
               }
               else if( value['GroupType'] == 'data' )
               {
                  if( value['Status'] == 'Running' )
                  {
                     statusIcon = { 'html': $compile( '<i class="fa fa-circle" style="color:#00CC66;" data-desc="正常运行中"></i>' )( $scope ) }
                  }
                  else if( value['Status'] == 'Waiting' )
                  {
                     statusIcon = { 'html': $compile( '<i class="fa fa-circle" style="color:#F9A937;" data-desc="该分区组处于waiting状态"></i>' )( $scope ) }
                  }
                  $scope.GridData['body'].push( [
                     statusIcon ,
                     { 'html': $compile( '<a href="#/Monitor/SDB-Group/Index" class="linkButton">' + value['GroupName'] + '</a>' )( $scope ) },
                     { 'text': value['NodeNumber'] },
                     { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Node/Index">' + value['PrimaryNode'] +'</a>' )( $scope ) },
                     { 'text': value['NumCollections'] },
                     { 'text': value['TotalRecord'] }
                  ] ) ;
               }
               $scope.groupList.push(
                  { "key":value['GroupName'], "value": index }
               ) ;
               
               if( value['Status'] == 'Waiting' )
               {
                  $scope.waitingGroupList.push(
                     { "key":value['GroupName'], "value": index }
                  ) ;
               }
               else if( value['Status'] == 'Running' )
               {
                  $scope.runningGroupList.push(
                     { "key":value['GroupName'], "value": index }
                  ) ;
               }
               
               $scope.dataNumber = index + 1 ;
            } ) ;
         } ) ;
      }
      $scope.getGroupList()




      $scope.charts = {}; 
      $scope.charts['Storage'] = {} ;
      $scope.charts['Storage']['options'] = window.SdbSacManagerConf.DiskStorageEchart ;
      $scope.charts['Storage']['value'] = [ [ 0, 0, true, false ] ] ;

      $scope.charts['Insert'] = {} ;
      $scope.charts['Insert']['options'] = window.SdbSacManagerConf.RecordInsertEchart ;
      $scope.charts['Insert']['value'] = [ [ 0, 0, true, false ] ] ;

      $scope.charts['Ram'] = {} ;
      $scope.charts['Ram']['options'] = window.SdbSacManagerConf.RamBarEchart ;
      $scope.charts['Ram']['value'] = [ [ 0, 0, true, false ] ] ;

      

      //创建分区组
      $scope.addGroup = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '创建分区组' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "name",
                  "webName": $scope.autoLanguage( '分区组名' ),
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
                  "name": "name",
                  "webName": $scope.autoLanguage( '角色' ),
                  "type": "select",
                  "value": 0,
                  "valid": [
                     { "key": "coord", "value": 0 },
                     { "key": "catalog", "value": 1 },
                     { "key": "data", "value": 2 }
                  ]
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            //$scope.Components.Modal.isShow = false ;
         } ;
      }
      //删除分区组  
      $scope.deleteGroup = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '删除分区组' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "groupName",
                  "webName": $scope.autoLanguage( '选择分区组' ),
                  "type": "select",
                  "required": true,
                  "value": 1,
                  "valid": $scope.groupList
               }
            ]
         };
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            $scope.Components.Confirm.isShow = true ;
            $scope.Components.Confirm.type = 1 ;
            $scope.Components.Confirm.okText = $scope.autoLanguage( '确定' ) ;
            $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
            $scope.Components.Confirm.title = $scope.autoLanguage( '要删除该分区组吗？' ) ;
            $scope.Components.Confirm.context = '分区组名：datagroup1' ;
            $scope.Components.Confirm.ok = function(){
               return true ;
            }
            return true ;
         }
      }
      //启动分区组
      $scope.startGroup = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '启动分区组' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "groupName",
                  "webName": $scope.autoLanguage( '选择分区组' ),
                  "type": "select",
                  "required": true,
                  "value": 6,
                  "valid": $scope.waitingGroupList
               }
            ]
         };
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            return true ;
         }
      }

      //停止分区组
      $scope.stopGroup = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '停止分区组' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "groupName",
                  "webName": $scope.autoLanguage( '选择分区组' ),
                  "type": "select",
                  "required": true,
                  "value": 1,
                  "valid": $scope.runningGroupList
               }
            ]
         };
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            return true ;
         }
      }
   } ) ;
}());

