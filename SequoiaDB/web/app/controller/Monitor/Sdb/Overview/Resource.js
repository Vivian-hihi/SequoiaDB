(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
   sacApp.controllerProvider.register( 'Monitor.SdbOverview.Resource.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){

      var clusterName = SdbFunction.LocalData( 'SdbClusterName' ) ;
      var moduleType = SdbFunction.LocalData( 'SdbModuleType' ) ;
      var moduleName = SdbFunction.LocalData( 'SdbModuleName' ) ;
      $scope.clusterName = clusterName ;
      $scope.moduleName = moduleName ;
      $scope.moduleType =  moduleType ;
      var statusIcon = {} ;
      var gridData = {
         'title': [ 
            { "text": $scope.autoLanguage( '状态' ) },
            { "text": $scope.autoLanguage( '会话ID' ) },
            { "text": $scope.autoLanguage( '上下文ID' ) },
            { "text": $scope.autoLanguage( '上下文类型' ) },
            { "text": $scope.autoLanguage( '所属' ) },
            { "text": $scope.autoLanguage( '所读数据' ) },
            { "text": $scope.autoLanguage( '所读索引' ) },
            { "text": $scope.autoLanguage( '查询时间' ) },
            { "text": $scope.autoLanguage( '缓存' ) },
            { "text": $scope.autoLanguage( '操作' ) }
         ],
         'body': [],
         'tool': {
            'position': 'bottom',
            'left': [ { 'text': '一共有20个会话，48个上下文' } ],
            'right': [ ]
         },
         'options': {
            'grid': {  'tdModel': 'fixed', 'gridModel': 'fixed', 'tdHeight': '19px', 'titleWidth': [ 4, 21, 8, 8, 24, 7, 8, 6, 6, 6 ] },
            'order': {
               'active': true
            }
         }
      } ; 
      $scope.GridData = $.extend( true, {}, gridData ) ;
      

      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/resourceList', success, failed, error ) ;
      }
      $scope.getResourceList = function(){
         $scope.queryList( {}, function( test ){
            $scope.test = test ;
            $scope.GridData = $.extend( true, {}, gridData ) ;
            $.each( test, function( index, value ){
               if( value['Status'] == 'Running' )
               {
                  statusIcon = { 'html': $compile( '<i class="fa fa-circle" style="color:#00CC66;" data-desc="正常运行中"></i>' )( $scope ) } ;
               }
               else if( value['Status'] == 'Waiting' )
               {
                  statusIcon = { 'html': $compile( '<i class="fa fa-circle" style="color:#F9A937;" data-desc="该会话处于waiting状态"></i>' )( $scope ) } ;
               }
               else if( value['Status'] == 'Destroying' )
               {
                  statusIcon = { 'html': $compile( '<i class="fa fa-circle" style="color:#D9534F;" data-desc="该会话处于销毁状态"></i>' )( $scope ) } ;
               }
               $scope.GridData['body'].push( [
                  statusIcon,
                  { 'html': $compile( '<a class="linkButton" ng-click="showContext()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['ContextID'] },
                  { 'text': value['Contexts'][0]['Type'] },
                  { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Node/Index">group1:Host-test-11810</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['DataRead'] },
                  { 'text': value['Contexts'][0]['IndexRead'] },
                  { 'text': value['Contexts'][0]['QueryTimeSpent'] },
                  { 'text': '' },
                  { 'html': $compile( '<div class="linkButton Ellipsis" ng-click="stopContext()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断上下文</div>' )( $scope ) }
               ] ) ;
            } ) ;
         } ) ;
      }
      $scope.getResourceList() ;

      $scope.showContext = function(){
         $scope.Components.Modal.contextInfo = {
            '会话ID' : 'Host-test-02:11810:10' ,
            '上下文ID': 6451 ,
            '对应系统线程ID': 854 ,
            '会话状态' : 'Running' ,
            'EDU类型' : 'Agent' ,
            '等待请求的队列长度' : 0 ,
            '已经处理请求的数量' : 150 ,
            '上下文ID数组' : 199 ,
            '数据记录读' : 0 ,
            '索引读' : 0 ,
            '数据记录写' : 0 ,
            '索引写' : 0 ,
            '总更新记录数量' : 0 ,
            '总删除记录数量' : 0 ,
            '总插入记录数量' : 0 ,
            '总读取记录数量' : 0 ,
            '总数据读' : 0 ,
            '总数据读时间' : 0 ,
            '总数据写时间' : 0 ,
            '读取记录的时间' : 0 ,
            '写入记录的时间' : 0 ,
            '连接发起时间' : "2016-04-07-19.19.42.932665",
            '最后一次操作类型' : 'COMMAND',
            'LastOpInfo' : 'Command:$SNAPSHOT_SESSION_CUR, Collection:, Match:{}, Selector:{}, OrderBy:{ \"SessionID\": 1 }, Hint:{}, Skip:0, Limit:-1, Flag:0x00000000(0)',
            "UserCPU" : 0.03 ,
            "SysCPU" : 0.02
         } ;
         $scope.Components.Modal.icon = '' ;
         $scope.Components.Modal.title = '上下文详细' ;
         $scope.Components.Modal.noOK = true ;
         $scope.Components.Modal.isShow = true ;
         $scope.Components.Modal.indexList = '' ;
         $scope.Components.Modal.Context = '\
<table class="table loosen border">\
<tr>\
<td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
<td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
</tr>\
<tr>\
<td>会话ID</td>\
<td>{{data.contextInfo["会话ID"]}}</td>\
</tr>\
<tr ng-repeat="(key, value) in data.contextInfo" ng-if="key != \'会话ID\'">\
<td>{{key}}</td>\
<td>{{value}}</td>\
</tr>\
</table>';
      }

      $scope.stopContext = function( ){
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '确定' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '要中断该上下文吗？' ) ;
         $scope.Components.Confirm.context = '会话id : Host-test-02:11810:10' ;
         $scope.Components.Confirm.ok = function(){
         }
      }

      $scope.ResourceMenu = [
         { 
            'html': $compile( '<div style="padding:5px 10px"><i class="pull-left fa fa-square-o" ng-click="changeSelect($event)" style="font-size:120%;width:16px;line-height:20px;cursor:pointer;"></i>上下文</div>' )( $scope ),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<div style="padding:5px 10px"><i class="pull-left fa fa-square-o" ng-click="changeSelect($event)" style="font-size:120%;width:16px;line-height:20px;cursor:pointer;"></i>缓存</div>' )( $scope ),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<div style="padding:5px 10px"><i class="pull-left fa fa-square-o" ng-click="changeSelect($event)" style="font-size:120%;width:16px;line-height:20px;cursor:pointer;"></i>锁</div>' )( $scope ),
            'onClick': function(){}
         }
      ]
      $scope.screenResult = {
         'Role':'all',
         'Running': true,
         'Waiting': true,
         'Destroying': true
      } ;

      $scope.changeScreen = function(){
         $scope.GridData = $.extend( true, {}, gridData ) ;
         $.each( $scope.test, function( index, value ){
            if( $scope.screenResult['Running'] == true && value['Status'] == 'Running' )
            {
               $scope.GridData['body'].push( [
                  { 'html': $compile( '<i class="fa fa-circle" style="color:#00CC66;" data-desc="正常运行中"></i>' )( $scope ) },
                  { 'html': $compile( '<a class="linkButton" ng-click="showContext()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['ContextID'] },
                  { 'text': value['Contexts'][0]['Type'] },
                  { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Node/Index">group1:Host-test-11810</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['DataRead'] },
                  { 'text': value['Contexts'][0]['IndexRead'] },
                  { 'text': value['Contexts'][0]['QueryTimeSpent'] },
                  { 'text': '' },
                  { 'html': $compile( '<div class="linkButton" ng-click="stopContext()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            }
            else if( $scope.screenResult['Waiting'] == true && value['Status'] == 'Waiting' )
            {
               $scope.GridData['body'].push( [
                  { 'html': $compile( '<i class="fa fa-circle" style="color:#F9A937;" data-desc="该会话处于waiting状态"></i>' )( $scope ) },
                  { 'html': $compile( '<a class="linkButton" ng-click="showContext()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['ContextID'] },
                  { 'text': value['Contexts'][0]['Type'] },
                  { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Node/Index">group1:Host-test-11810</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['DataRead'] },
                  { 'text': value['Contexts'][0]['IndexRead'] },
                  { 'text': value['Contexts'][0]['QueryTimeSpent'] },
                  { 'text': '' },
                  { 'html': $compile( '<div class="linkButton" ng-click="stopContext()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            }
            else if( $scope.screenResult['Destroying'] == true && value['Status'] == 'Destroying' )
            {
               $scope.GridData['body'].push( [
                  { 'html': $compile( '<i class="fa fa-circle" style="color:#D9534F;" data-desc="该会话处于销毁状态"></i>' )( $scope ) },
                  { 'html': $compile( '<a class="linkButton" ng-click="showContext()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['ContextID'] },
                  { 'text': value['Contexts'][0]['Type'] },
                  { 'html': $compile( '<a class="linkButton" href="#/Monitor/SDB-Node/Index">group1:Host-test-11810</a>' )( $scope ) },
                  { 'text': value['Contexts'][0]['DataRead'] },
                  { 'text': value['Contexts'][0]['IndexRead'] },
                  { 'text': value['Contexts'][0]['QueryTimeSpent'] },
                  { 'text': '' },
                  { 'html': $compile( '<div class="linkButton" ng-click="stopContext()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            }
         } )
      } ;

      $scope.ScreenMenu = [
         { 
            'html': $compile( '<div style="padding:5px 10px"><input type="radio" name="a" value="all" ng-model="screenResult[\'Role\']" />所有会话</div>' )( $scope ),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<div style="padding:5px 10px"><input type="radio" name="a" value="current" ng-model="screenResult[\'Role\']"/>当前会话</div>' )( $scope ),
            'onClick': function(){}
         },
         {},
         { 
            'html': $compile( '<div style="padding:5px 10px"><input type="checkbox" ng-model="screenResult[\'Running\']"/>运行状态</div>' )( $scope ),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<div style="padding:5px 10px"><input type="checkbox" ng-model="screenResult[\'Waiting\']"/>等待状态</div>' )( $scope ),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<div style="padding:5px 10px"><input type="checkbox" ng-model="screenResult[\'Destroying\']"/>销毁状态</div>' )( $scope ),
            'onClick': function(){}
         },
         { 
            'html': $compile( '<button class="btn btn-primary" ng-click="changeScreen()" style="width:100%;">确定</button>' )( $scope )
         }
      ] ;

      
   } ) ;
}());