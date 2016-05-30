(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Deploy.Index.Ctrl', function( $scope, $compile, $location, $rootScope, SdbFunction, SdbRest ){

      var defaultShow = $rootScope.tempData( 'Deploy', 'Index' ) ;

      //初始化
      //集群列表
      $scope.clusterList = [] ;
      //默认选的cluster
      $scope.currentCluster = 0 ;
      //默认显示业务还是主机列表
      $scope.currentPage = 'module' ;
      //业务列表
      $scope.moduleList = [] ;
      //主机列表
      $scope.HostList = [] ;
      //业务类型列表
      $scope.moduleType = [] ;
      //业务列表数量
      $scope.ModuleNum = 0 ;
      //主机列表数量
      $scope.HostNum = 0 ;
      //选择主机的网格选项
      $scope.HostGridOptions = { 'titleWidth': [ '30px', '60px', 30, 30, 40 ] } ;
      //临时
      $scope.charts = {} ;
      $scope.charts['Module'] = {} ;
      $scope.charts['Module']['options'] = window.SdbSacManagerConf.StorageScaleEchart ;

      $scope.charts['Host'] = {} ;
      $scope.charts['Host']['CPU'] = { 'percent': 60, 'style': { 'progress': { 'background': '#FF9933' } } } ;
      $scope.charts['Host']['Memory'] = { 'percent': 90, 'style': { 'progress': { 'background': '#D9534F' } } } ;
      $scope.charts['Host']['Disk'] = { 'percent': 40 } ;

      //清空Deploy域的数据
      $rootScope.tempData( 'Deploy' ) ;

      //查询主机状态
      var queryHostStatus = function(){
         var queryHostList = [] ;
         $.each( $scope.HostList, function( index, hostInfo ){
            queryHostList.push( { 'HostName': hostInfo['HostName'] } ) ;
         } ) ;
         var data = { 'cmd': 'query host status', 'HostInfo': JSON.stringify( queryHostList ) } ;
         SdbRest.OmOperation( data, function( hostStatus ){
            
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               queryHost() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //查询主机
      var queryHost = function(){
         var data = { 'cmd': 'query host' } ;
         SdbRest.OmOperation( data, function( hostList ){
            $scope.HostList = hostList ;
            $scope.SwitchCluster( $scope.currentCluster ) ;
            if( defaultShow == 'host' )
            {
               $scope.SwitchPage( defaultShow ) ;
            }
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               queryHost() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //查询业务
      var queryModule = function(){
         var data = { 'cmd': 'query business' } ;
         SdbRest.OmOperation( data, function( moduleList ){
            $.each( moduleList, function( index ){
               moduleList[index]['Chart'] = {} ;
               moduleList[index]['Chart']['options'] = $.extend( true, {}, window.SdbSacManagerConf.StorageScaleEchart )
            } ) ;
            $scope.moduleList = moduleList ;
            $scope.SwitchCluster( $scope.currentCluster ) ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               queryModule() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
         var data = { 'cmd': 'list businesses' } ;
         SdbRest.OmOperation( data, function( moduleList ){} ) ;
      }

      //查询集群
      var queryCluster = function(){
         var data = { 'cmd': 'query cluster' } ;
         SdbRest.OmOperation( data, function( clusterList ){
            $scope.clusterList = clusterList ;
            if( $scope.clusterList.length > 0 )
            {
               queryHost() ;
               queryModule() ;
            }
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               queryCluster() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //切换业务和主机
      $scope.SwitchPage = function( page ){
         $scope.currentPage = page ;
         $scope.bindResize() ;
      }

      //切换集群
      $scope.SwitchCluster = function( index ){
         $scope.currentCluster = index ;
         if( $scope.clusterList.length > 0 )
         {
            var clusterName = $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ;
            $scope.ModuleNum = 0 ;
            $.each( $scope.moduleList, function( index, moduleInfo ){
               if( moduleInfo['ClusterName'] == clusterName )
               {
                  ++$scope.ModuleNum ;
               }
            } ) ;
            $scope.HostNum = 0 ;
            $.each( $scope.HostList, function( index, hostInfo ){
               if( hostInfo['ClusterName'] == clusterName )
               {
                  ++$scope.HostNum ;
               }
            } ) ;
         }
         $scope.bindResize() ;
      }

      //获取业务类型列表
      var GetModuleType = function(){
         var data = { 'cmd': 'list business type' } ;
         SdbRest.OmOperation( data, function( moduleType ){
            $scope.moduleType = moduleType ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               GetModuleType() ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //全选
      $scope.SelectAll = function(){
         $.each( $scope.HostList, function( index ){
            if( $scope.clusterList[ $scope.currentCluster ]['ClusterName'] == $scope.HostList[index]['ClusterName'] )
            {
               $scope.HostList[index]['checked'] = true ;
            }
         } ) ;
      }

      //反选
      $scope.Unselect = function(){
         $.each( $scope.HostList, function( index ){
            if( $scope.clusterList[ $scope.currentCluster ]['ClusterName'] == $scope.HostList[index]['ClusterName'] )
            {
               $scope.HostList[index]['checked'] = !$scope.HostList[index]['checked'] ;
            }
         } ) ;
      }

      //添加主机
      $scope.AddHost = function(){
         if( $scope.clusterList.length > 0 )
         {
            $rootScope.tempData( 'Deploy', 'Model', 'Host' ) ;
            $rootScope.tempData( 'Deploy', 'Module', 'None' ) ;
            $rootScope.tempData( 'Deploy', 'ClusterName', $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ) ;
            $rootScope.tempData( 'Deploy', 'InstallPath', $scope.clusterList[ $scope.currentCluster ]['InstallPath'] ) ;
            $location.path( '/Deploy/ScanHost' ) ;
         }
      }

      //删除主机
      $scope.RemoveHost = function(){
         if( $scope.clusterList.length > 0 )
         {
            var hostList = [] ;
            $.each( $scope.HostList, function( index ){
               if( $scope.HostList[index]['checked'] == true && $scope.clusterList[ $scope.currentCluster ]['ClusterName'] == $scope.HostList[index]['ClusterName'] )
               {
                  hostList.push( { 'HostName': $scope.HostList[index]['HostName'] } ) ;
               }
            } ) ;
            if( hostList.length > 0 )
            {
               var data = { 'cmd': 'remove host', 'HostInfo': JSON.stringify( { 'HostInfo': hostList } ) } ;
               SdbRest.OmOperation( data, function( taskInfo ){
                  $rootScope.tempData( 'Deploy', 'Model', 'Task' ) ;
                  $rootScope.tempData( 'Deploy', 'Module', 'None' ) ;
                  $rootScope.tempData( 'Deploy', 'HostTaskID', taskInfo[0]['TaskID'] ) ;
                  $location.path( '/Deploy/InstallHost' ) ;
               }, function( errorInfo ){
                  _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                     $scope.RemoveHost() ;
                     return true ;
                  } ) ;
               }, function(){
                  _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
               } ) ;
            }
            else
            {
               $scope.Components.Confirm.type = 3 ;
               $scope.Components.Confirm.context = $scope.autoLanguage( '至少选择一台的主机。' ) ;
               $scope.Components.Confirm.isShow = true ;
               $scope.Components.Confirm.okText = $scope.autoLanguage( '好的' ) ;
            }
         }
      }

      //创建 添加业务 弹窗
      $scope.CreateInstallModuleModel = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '添加业务' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'moduleName',
                  "webName": $scope.autoLanguage( '业务名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1,
                     "max": 127,
                     "regex": '^[0-9a-zA-Z]+$'
                  }
               },
               {
                  "name": 'moduleType',
                  "webName": $scope.autoLanguage( '业务类型' ),
                  "type": "select",
                  "value": 0,
                  "valid": []
               }
            ]
         } ;
         var num = 1 ;
         var defaultName = '' ;
         while( true )
         {
            var isFind = false ;
            defaultName = sprintf( 'myModule?', num ) ;
            $.each( $scope.moduleList, function( index, moduleInfo ){
               if( defaultName == moduleInfo['BusinessName'] )
               {
                  isFind = true ;
                  return false ;
               }
            } ) ;
            if( isFind == false )
            {
               $.each( $rootScope.OmTaskList, function( index, taskInfo ){
                  if( defaultName == taskInfo['Info']['BusinessName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == false )
               {
                  break ;
               }
            }
            ++num ;
         }
         $scope.Components.Modal.form['inputList'][0]['value'] = defaultName ;
         $.each( $scope.moduleType, function( index, typeInfo ){
            $scope.Components.Modal.form['inputList'][1]['valid'].push( { 'key': typeInfo['BusinessDesc'], 'value': index } ) ;
         } ) ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check( function( formVal ){
               var isFind = false ;
               $.each( $scope.moduleList, function( index, moduleInfo ){
                  if( formVal['moduleName'] == moduleInfo['BusinessName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == true )
               {
                  return [ { 'name': 'moduleName', 'error': $scope.autoLanguage( '业务名已经存在' ) } ]
               }
               else
               {
                  return [] ;
               }
            } ) ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               $rootScope.tempData( 'Deploy', 'Model', 'Module' ) ;
               $rootScope.tempData( 'Deploy', 'Module', $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] ) ;
               $rootScope.tempData( 'Deploy', 'ModuleName', formVal['moduleName'] ) ;
               $rootScope.tempData( 'Deploy', 'ClusterName', $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ) ;
               if( $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] == 'sequoiadb' )
               {
                  $location.path( '/Deploy/SDB-Conf' ) ;
               }
               else if( $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] == 'sequoiasql' )
               {
                  var tempHostInfo = [] ;
			         $.each( $scope.HostList, function( index, value ){
                     if( $scope.clusterList[$scope.currentCluster]['ClusterName'] == value['ClusterName'] )
                     {
				            tempHostInfo.push( { 'HostName': value['HostName'] } ) ;
                     }
			         } ) ;
                  var businessConf = {} ;
                  businessConf['ClusterName']  = $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ;
                  businessConf['BusinessName'] = formVal['moduleName'] ;
                  businessConf['BusinessType'] = $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] ;
                  businessConf['DeployMod'] = 'olap' ;
                  businessConf['Property'] = [
                     { "Name": "deploy_standby", "Value": "false" },
                     { "Name": "segment_num", "Value": tempHostInfo.length + '' }
                  ] ;
                  businessConf['HostInfo'] = tempHostInfo ;
                  $rootScope.tempData( 'Deploy', 'ModuleConfig', businessConf ) ;
                  $location.path( '/Deploy/SSQL-Mod' ) ;
               }
               else if( $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] == 'zookeeper' )
               {
                  var tempHostInfo = [] ;
			         $.each( $scope.HostList, function( index, value ){
                     if( $scope.clusterList[$scope.currentCluster]['ClusterName'] == value['ClusterName'] )
                     {
				            tempHostInfo.push( { 'HostName': value['HostName'] } ) ;
                     }
			         } ) ;
                  var businessConf = {} ;
                  businessConf['ClusterName']  = $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ;
                  businessConf['BusinessName'] = formVal['moduleName'] ;
                  businessConf['BusinessType'] = $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] ;
                  businessConf['DeployMod'] = 'distribution' ;
                  businessConf['Property'] = [ { 'Name': 'zoonodenum', 'Value': '3' } ] ;
                  businessConf['HostInfo'] = tempHostInfo ;
                  $rootScope.tempData( 'Deploy', 'ModuleConfig', businessConf ) ;
                  $location.path( '/Deploy/ZKP-Mod' ) ;
               }
            }
            return isAllClear ;
         }
      }

      //发现业务
      var discoverModule = function( configure ){
         var data = { 'cmd': 'discover business', 'ConfigInfo': JSON.stringify( configure ) } ;
         SdbRest.OmOperation( data, function(){
            queryCluster() ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               discoverModule( configure ) ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //创建 发现sequoiasql 弹窗
      $scope.CreateAppendSSQLModel = function( moduleName ){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = 'SequoiaSQL' ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'HostName',
                  "webName": $scope.autoLanguage( '主机名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               },
               {
                  "name": 'ServiceName',
                  "webName": $scope.autoLanguage( '服务名' ),
                  "type": "port",
                  "required": true,
                  "value": '',
                  "valid": {}
               },
               {
                  "name": 'InstallPath',
                  "webName": $scope.autoLanguage( '安装路径' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               },
               {
                  "name": 'DbName',
                  "webName": $scope.autoLanguage( '数据库名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               },
               {
                  "name": 'User',
                  "webName": $scope.autoLanguage( '数据库用户名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               },
               {
                  "name": 'Passwd',
                  "webName": $scope.autoLanguage( '数据库密码' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               var configure = {} ;
               configure['ClusterName']  = $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ;
               configure['BusinessType'] = 'sequoiasql' ;
               configure['BusinessName'] = moduleName ;
               configure['BusinessInfo'] = formVal ;
               discoverModule( configure ) ;
            }
            return isAllClear ;
         }
      }

      //创建 发现其他业务 弹窗
      $scope.CreateAppendOtherModel = function( moduleName, moduleType ){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = moduleType ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'HostName',
                  "webName": $scope.autoLanguage( '主机名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1
                  }
               },
               {
                  "name": 'WebServicePort',
                  "webName": $scope.autoLanguage( '服务名' ),
                  "type": "port",
                  "required": true,
                  "value": '',
                  "valid": {}
               }
            ]
         } ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               var configure = {} ;
               configure['ClusterName']  = $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ;
               configure['BusinessType'] = moduleType ;
               configure['BusinessName'] = moduleName ;
               configure['BusinessInfo'] = formVal ;
               discoverModule( configure ) ;
            }
            return isAllClear ;
         }
      }

      //创建 发现业务 弹窗
      $scope.CreateAppendModuleModel = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '发现业务' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'moduleName',
                  "webName": $scope.autoLanguage( '业务名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     "min": 1,
                     "max": 127,
                     "regex": '^[0-9a-zA-Z]+$'
                  }
               },
               {
                  "name": 'moduleType',
                  "webName": $scope.autoLanguage( '业务类型' ),
                  "type": "select",
                  "value": 'sequoiasql',
                  "valid": [
                     { 'key': $scope.autoLanguage( 'SequoiaSQL引擎' ), 'value': 'sequoiasql' },
                     { 'key': 'Spark', 'value': 'spark' },
                     { 'key': 'Hdfs', 'value': 'hdfs' },
                     { 'key': 'Yarn', 'value': 'yarn' },
                  ]
               }
            ]
         } ;
         var num = 1 ;
         var defaultName = '' ;
         while( true )
         {
            var isFind = false ;
            defaultName = sprintf( 'myModule?', num ) ;
            $.each( $scope.moduleList, function( index, moduleInfo ){
               if( defaultName == moduleInfo['BusinessName'] )
               {
                  isFind = true ;
                  return false ;
               }
            } ) ;
            if( isFind == false )
            {
               $.each( $rootScope.OmTaskList, function( index, taskInfo ){
                  if( defaultName == taskInfo['Info']['BusinessName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == false )
               {
                  break ;
               }
            }
            ++num ;
         }
         $scope.Components.Modal.form['inputList'][0]['value'] = defaultName ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check( function( formVal ){
               var isFind = false ;
               $.each( $scope.moduleList, function( index, moduleInfo ){
                  if( formVal['moduleName'] == moduleInfo['BusinessName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == false )
               {
                  $.each( $rootScope.OmTaskList, function( index, taskInfo ){
                     if( formVal['moduleName'] == taskInfo['Info']['BusinessName'] )
                     {
                        isFind = true ;
                        return false ;
                     }
                  } ) ;
               }
               if( isFind == true )
               {
                  return [ { 'name': 'moduleName', 'error': $scope.autoLanguage( '业务名已经存在' ) } ]
               }
               else
               {
                  return [] ;
               }
            } ) ;
            if( isAllClear )
            {
               $scope.Components.Modal.isShow = false ;
               var formVal = $scope.Components.Modal.form.getValue() ;
               if( formVal['moduleType'] == 'sequoiasql' )
               {
                  setTimeout( function(){
                     $scope.CreateAppendSSQLModel( formVal['moduleName'] ) ;
                     $scope.$apply() ;
                  } ) ;
               }
               else
               {
                  setTimeout( function(){
                     $scope.CreateAppendOtherModel( formVal['moduleName'], formVal['moduleType'] ) ;
                     $scope.$apply() ;
                  } ) ;
               }
            }
            else
            {
               return false ;
            }
         }
      }

      //卸载业务
      var uninstallModule = function( index ){
         if( typeof( $scope.moduleList[index]['AddtionType'] ) == 'undefined' || $scope.moduleList[index]['AddtionType'] != 1 )
         {
            var data = { 'cmd': 'remove business', 'BusinessName': $scope.moduleList[index]['BusinessName'] } ;
            SdbRest.OmOperation( data, function( taskInfo ){
               $rootScope.tempData( 'Deploy', 'Model', 'Task' ) ;
               $rootScope.tempData( 'Deploy', 'Module', 'None' ) ;
               $rootScope.tempData( 'Deploy', 'ModuleTaskID', taskInfo[0]['TaskID'] ) ;
               $location.path( '/Deploy/InstallModule' ) ;
            }, function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  uninstallModule( moduleName ) ;
                  return true ;
               } ) ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            } ) ;
         }
         else
         {
            var data = { 'cmd': 'undiscover business', 'ClusterName': $scope.moduleList[index]['ClusterName'], 'BusinessName': $scope.moduleList[index]['BusinessName'] } ;
            SdbRest.OmOperation( data, function(){
               queryCluster() ;
            }, function( errorInfo ){
               _IndexPublic.createRetryModel( $scope, errorInfo, function(){
                  uninstallModule( moduleName ) ;
                  return true ;
               } ) ;
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            } ) ;
         }
      }

      //创建 卸载业务 弹窗
      $scope.CreateUninstallModuleModel = function(){
         var clusterName = $scope.clusterList[ $scope.currentCluster ]['ClusterName'] ;
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '卸载业务' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'moduleIndex',
                  "webName": $scope.autoLanguage( '业务名' ),
                  "type": "select",
                  "value": null,
                  "valid": []
               },
            ]
         } ;
         $.each( $scope.moduleList, function( index, moduleInfo ){
            if( clusterName == moduleInfo['ClusterName'] )
            {
               if( $scope.Components.Modal.form['inputList'][0]['value'] == null )
               {
                  $scope.Components.Modal.form['inputList'][0]['value'] = index ;
               }
               $scope.Components.Modal.form['inputList'][0]['valid'].push( { 'key': moduleInfo['BusinessName'], 'value': index } )
            }
         } ) ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               uninstallModule( formVal['moduleIndex'] ) ;
            }
            return isAllClear ;
         }
      }

      //创建集群
      var createCluster = function( clusterInfo, success ){
         var data = { 'cmd': 'create cluster', 'ClusterInfo': JSON.stringify( clusterInfo ) } ;
         SdbRest.OmOperation( data, function(){
            success() ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               createCluster( clusterInfo ) ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //创建 创建集群 弹窗
      $scope.CreateAddClusterModel = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '创建集群' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'ClusterName',
                  "webName": $scope.autoLanguage( '集群名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     'min': 1,
                     'max': 127,
                     'regex': '^[0-9a-zA-Z]+$'
                  }
               },
               {
                  'name': 'Desc',
                  'webName': $scope.autoLanguage( '描述' ),
                  'type': 'string',
                  'value': '',
                  'valid': {
                     'min': 0,
                     'max': 1024
                  }
               },
               {
                  "name": 'SdbUser',
                  "webName": $scope.autoLanguage( '用户名' ),
                  "type": "string",
                  "required": true,
                  "value": 'sdbadmin',
                  "valid": {
                     'min': 1,
                     'max': 1024
                  }
               },
               {
                  "name": 'SdbPasswd',
                  "webName": $scope.autoLanguage( '密码' ),
                  "type": "string",
                  "required": true,
                  "value": 'sdbadmin',
                  "valid": {
                     'min': 1,
                     'max': 1024
                  }
               },
               {
                  "name": 'SdbUserGroup',
                  "webName": $scope.autoLanguage( '用户组' ),
                  "type": "string",
                  "required": true,
                  "value": 'sdbadmin_group',
                  "valid": {
                     'min': 1,
                     'max': 1024
                  }
               },
               {
                  "name": 'InstallPath',
                  "webName": $scope.autoLanguage( '安装路径' ),
                  "type": "string",
                  "required": true,
                  "value": '/opt/sequoiadb/',
                  "valid": {
                     'min': 1,
                     'max': 2048
                  }
               }
            ]
         } ;
         var num = 1 ;
         var defaultName = '' ;
         while( true )
         {
            var isFind = false ;
            defaultName = sprintf( 'myCluster?', num ) ;
            $.each( $scope.clusterList, function( index, clusterInfo ){
               if( defaultName == clusterInfo['ClusterName'] )
               {
                  isFind = true ;
                  return false ;
               }
            } ) ;
            if( isFind == false )
            {
               break ;
            }
            ++num ;
         }
         $scope.Components.Modal.form['inputList'][0]['value'] = defaultName ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check( function( formVal ){
               var isFind = false ;
               $.each( $scope.clusterList, function( index, clusterInfo ){
                  if( formVal['ClusterName'] == clusterInfo['ClusterName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == true )
               {
                  return [ { 'name': 'ClusterName', 'error': $scope.autoLanguage( '集群名已经存在' ) } ]
               }
               else
               {
                  return [] ;
               }
            } ) ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               createCluster( formVal, function(){
                  queryCluster() ;
               } ) ;
            }
            return isAllClear ;
         }
      }

      //删除集群
      var removeCluster = function( clusterIndex ){
         var clusterName = $scope.clusterList[clusterIndex]['ClusterName'] ;
         var data = { 'cmd': 'remove cluster', 'ClusterName': clusterName } ;
         SdbRest.OmOperation( data, function(){
            if( clusterIndex == $scope.currentCluster )
            {
               $scope.currentCluster = 0 ;
            }
            queryCluster() ;
         }, function( errorInfo ){
            _IndexPublic.createRetryModel( $scope, errorInfo, function(){
               removeCluster( clusterIndex ) ;
               return true ;
            } ) ;
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      }

      //创建 删除集群 弹窗
      $scope.CreateRemoveClusterModel = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '删除集群' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'ClusterName',
                  "webName": $scope.autoLanguage( '集群名' ),
                  "type": "select",
                  "value": $scope.currentCluster,
                  "valid": []
               }
            ]
         } ;
         $.each( $scope.clusterList, function( index ){
            $scope.Components.Modal.form['inputList'][0]['valid'].push( { 'key': $scope.clusterList[index]['ClusterName'], 'value': index } ) ;
         } ) ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check() ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               removeCluster( formVal['ClusterName'] ) ;
            }
            return isAllClear ;
         }
      }

      //一键部署
      $scope.CreateDeployModuleModel = function(){
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = $scope.autoLanguage( '部署' ) ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.form = {
            'inputList': [
               {
                  "name": 'ClusterName',
                  "webName": $scope.autoLanguage( '集群名' ),
                  "type": "string",
                  "required": true,
                  "value": "",
                  "valid": {
                     'min': 1,
                     'max': 127,
                     'regex': '^[0-9a-zA-Z]+$'
                  }
               },
               {
                  'name': 'Desc',
                  'webName': $scope.autoLanguage( '描述' ),
                  'type': 'string',
                  'value': '',
                  'valid': {
                     'min': 0,
                     'max': 1024
                  }
               },
               {
                  "name": 'moduleName',
                  "webName": $scope.autoLanguage( '业务名' ),
                  "type": "string",
                  "required": true,
                  "value": "myModule",
                  "valid": {
                     "min": 1,
                     "max": 127,
                     "regex": '^[0-9a-zA-Z]+$'
                  }
               },
               {
                  "name": 'moduleType',
                  "webName": $scope.autoLanguage( '业务类型' ),
                  "type": "select",
                  "value": 0,
                  "valid": []
               },
               {
                  "name": 'SdbUser',
                  "webName": $scope.autoLanguage( '用户名' ),
                  "type": "string",
                  "required": true,
                  "value": 'sdbadmin',
                  "valid": {
                     'min': 1,
                     'max': 1024
                  }
               },
               {
                  "name": 'SdbPasswd',
                  "webName": $scope.autoLanguage( '密码' ),
                  "type": "string",
                  "required": true,
                  "value": 'sdbadmin',
                  "valid": {
                     'min': 1,
                     'max': 1024
                  }
               },
               {
                  "name": 'SdbUserGroup',
                  "webName": $scope.autoLanguage( '用户组' ),
                  "type": "string",
                  "required": true,
                  "value": 'sdbadmin_group',
                  "valid": {
                     'min': 1,
                     'max': 1024
                  }
               },
               {
                  "name": 'InstallPath',
                  "webName": $scope.autoLanguage( '安装路径' ),
                  "type": "string",
                  "required": true,
                  "value": '/opt/sequoiadb/',
                  "valid": {
                     'min': 1,
                     'max': 2048
                  }
               }
            ]
         } ;
         var num = 1 ;
         var defaultName = '' ;
         while( true )
         {
            var isFind = false ;
            defaultName = sprintf( 'myCluster?', num ) ;
            $.each( $scope.clusterList, function( index, clusterInfo ){
               if( defaultName == clusterInfo['ClusterName'] )
               {
                  isFind = true ;
                  return false ;
               }
            } ) ;
            if( isFind == false )
            {
               break ;
            }
            ++num ;
         }
         $scope.Components.Modal.form['inputList'][0]['value'] = defaultName ;
         num = 1 ;
         defaultName = '' ;
         while( true )
         {
            var isFind = false ;
            defaultName = sprintf( 'myModule?', num ) ;
            $.each( $scope.moduleList, function( index, moduleInfo ){
               if( defaultName == moduleInfo['BusinessName'] )
               {
                  isFind = true ;
                  return false ;
               }
            } ) ;
            if( isFind == false )
            {
               $.each( $rootScope.OmTaskList, function( index, taskInfo ){
                  if( defaultName == taskInfo['Info']['BusinessName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == false )
               {
                  break ;
               }
            }
            ++num ;
         }
         $scope.Components.Modal.form['inputList'][2]['value'] = defaultName ;
         $.each( $scope.moduleType, function( index, typeInfo ){
            $scope.Components.Modal.form['inputList'][3]['valid'].push( { 'key': typeInfo['BusinessDesc'], 'value': index } ) ;
         } ) ;
         $scope.Components.Modal.Context = '<div form-create para="data.form"></div>' ;
         $scope.Components.Modal.ok = function(){
            var isAllClear = $scope.Components.Modal.form.check( function( formVal ){
               var rv = [] ;
               var isFind = false ;
               $.each( $scope.clusterList, function( index, clusterInfo ){
                  if( formVal['ClusterName'] == clusterInfo['ClusterName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == true )
               {
                  rv.push( { 'name': 'ClusterName', 'error': $scope.autoLanguage( '集群名已经存在' ) } ) ;
               }
               isFind = false ;
               $.each( $scope.moduleList, function( index, moduleInfo ){
                  if( formVal['moduleName'] == moduleInfo['BusinessName'] )
                  {
                     isFind = true ;
                     return false ;
                  }
               } ) ;
               if( isFind == true )
               {
                  rv.push( { 'name': 'moduleName', 'error': $scope.autoLanguage( '业务名已经存在' ) } ) ;
               }
               return rv ;
            } ) ;
            if( isAllClear )
            {
               var formVal = $scope.Components.Modal.form.getValue() ;
               createCluster( formVal, function(){
                  $rootScope.tempData( 'Deploy', 'ClusterName', formVal['ClusterName'] ) ;
                  $rootScope.tempData( 'Deploy', 'ModuleName', formVal['moduleName'] ) ;
                  $rootScope.tempData( 'Deploy', 'Model', 'Deploy' ) ;
                  $rootScope.tempData( 'Deploy', 'Module', $scope.moduleType[ formVal['moduleType'] ]['BusinessType'] ) ;
                  $rootScope.tempData( 'Deploy', 'InstallPath', formVal['InstallPath'] ) ;
                  $location.path( '/Deploy/ScanHost' ) ;
               } ) ;
            }
            return isAllClear ;
         }
      }

      //执行
      GetModuleType() ;
      queryCluster() ;

      if( defaultShow == 'host' )
      {
         $scope.SwitchPage( defaultShow ) ;
      }
   } ) ;
}());