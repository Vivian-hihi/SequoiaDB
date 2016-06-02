(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbGroup.Index.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
      
      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType = moduleType ;
      $scope.hostList = [] ;
      $scope.PrimaryNode = '' ;
      var i = 0 ;

      $scope.queryList = function( data, success, failed, error, complete ){
         SdbRest._postTest( './test/groupInfo', success, failed, error ) ;
      }

      $scope.getGroupInfo = function(){
         //var data = { 
         //   'cmd': 'get node configure',
         //   'HostName':'ubuntu-test-02',
         //   "ServiceName":11820 } ;
         //SdbRest.OmOperation( data, function( test ){
         //   alert(JSON.stringify(test))
         //} ) ;
         $scope.queryList( {}, function( test ){
            $.each( test, function( index, value ){
               if( value['Role'] == 0 )
               {
                  value['Role'] = '数据组' ;
               }
               else if ( value['Role'] == 2 )
               {
                  value['Role'] = '编目组' ;
               }
               $scope.groupInfo = value ;
            } ) ;
            $.each( $scope.groupInfo['Group'], function( index, value ){
               $scope.hostList.push( { "key": value['HostName'] + ':' + value['Service'][0]['Name'], "value": index } ) ;
               //获取主节点节点名
               if( $scope.groupInfo['PrimaryNode'] == value['NodeID'] )
               {
                  $scope.PrimaryNode = value['HostName'] + ':' + value['Service'][0]['Name'] ;
               }
            } ) ;
         } ) ;
      }
      
      $scope.getGroupInfo() ;

      //创建节点
      $scope.addNode = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '创建节点' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "hostName",
                  "webName": $scope.autoLanguage( '选择主机' ),
                  "type": "select",
                  "required": true,
                  "value": "1",
                  "valid": [
                     { "key":"ubuntu-test-02", "value": "1" },
                     { "key":"ubuntu-test-03", "value": "2" },
                     { "key":"ubuntu-test-04", "value": "3" },
                     { "key":"ubuntu-test-05", "value": "4" }
                  ]                  
               },
               {
                  "name": "svcname",
                  "webName": $scope.autoLanguage( '节点端口号' ),
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
                  "name": "svcname",
                  "webName": $scope.autoLanguage( '节点路径' ),
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
                  "name": "config",
                  "webName": $scope.autoLanguage( '配置' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1,
                     "max": 127,
                     "ban": [ ".", "$" ]
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            $scope.Components.Modal.isShow = false ;
         }
      }

      //删除节点
      $scope.deleteNode = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '删除节点' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "hostName",
                  "webName": $scope.autoLanguage( '选择节点' ),
                  "type": "select",
                  "required": true,
                  "value": 0,
                  "valid": $scope.hostList             
               }
            ]
         };
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            $scope.Components.Confirm.isShow = true ;
            $scope.Components.Confirm.type = 1 ;
            $scope.Components.Confirm.okText = $scope.autoLanguage( '确定' ) ;
            $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
            $scope.Components.Confirm.title = $scope.autoLanguage( '要删除该节点吗？' ) ;
            $scope.Components.Confirm.context = '节点名：ubuntu-test-02:11830' ;
            $scope.Components.Confirm.ok = function(){
               return true ;
            }
            return true ;
         }
      }

      //启动节点
      $scope.startNode = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '启动节点' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "hostName",
                  "webName": $scope.autoLanguage( '选择节点' ),
                  "type": "select",
                  "required": true,
                  "value": 0,
                  "valid": $scope.hostList             
               }
            ]
         };
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>';
         $scope.Components.Modal.ok = function(){
            return true ;
         }
      }

      //停止节点
      $scope.stopNode = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '停止节点' ;
         $scope.Components.Modal.noOK = false ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            inputList: [
               {
                  "name": "hostName",
                  "webName": $scope.autoLanguage( '选择节点' ),
                  "type": "select",
                  "required": true,
                  "value": 0,
                  "valid": $scope.hostList             
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