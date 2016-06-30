(function(){
   var sacApp = window.SdbSacManagerModule ;
   //控制器
    sacApp.controllerProvider.register( 'Monitor.SdbNode.Session.Ctrl', function( $scope, $compile, SdbRest, SdbFunction ){
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
            { "text": $scope.autoLanguage( '线程ID' ) },
            { "text": $scope.autoLanguage( 'EDU类型' ) },
            { "text": $scope.autoLanguage( '数据记录写' ) },
            { "text": $scope.autoLanguage( '索引写' ) },
            { "text": $scope.autoLanguage( '更新记录数' ) },
            { "text": $scope.autoLanguage( '删除记录数' ) },
            { "text": $scope.autoLanguage( '插入记录数' ) },
            { "text": $scope.autoLanguage( '数据读' ) },
            { "text": $scope.autoLanguage( '操作' ) }
         ],
         'body': [],
         'tool': {
            'position': 'bottom',
            'left': [ { 'text': '一共有 48 个会话' } ],
            'right': [ ]
         },
         'options': {
            'grid': {  'tdModel': 'fixed', 'gridModel': 'fixed', 'tdHeight': '19px', 'titleWidth': [ 4, 21, 7, 13, 9, 6, 9, 9, 9,  7,9] },
            'order': {
               'active': true
            }
         }
      } ;
      $scope.GridData = $.extend( true, {}, gridData ) ;

      //获取会话列表数据
      $scope.queryList = function( data, success, failed, error, complete )
      {
         SdbRest._postTest( './test/sessionList', success, failed, error ) ;
      }

      $scope.getSessionList = function(){
         
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
                  { 'html': $compile( '<a class="linkButton" ng-click="showSession()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['TID'] },
                  { 'text': value['Type'] },
                  { 'text': value['TotalDataWrite'] },
                  { 'text': value['TotalIndexWrite'] },
                  { 'text': value['TotalUpdate'] },
                  { 'text': value['TotalDelete'] },
                  { 'text': value['TotalInsert'] },
                  { 'text': value['TotalRead'] },
                  { 'html': $compile( '<div class="linkButton Ellipsis" ng-click="stopSession()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            } )
         } )
      }

      $scope.getSessionList() ;

      //显示会话详细
      $scope.showSession = function(){
         $scope.Components.Modal.sessionInfo = {
         '会话ID' : 'Host-test-02:11810:10' ,
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
         $scope.Components.Modal.title = '会话详细' ;
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
<td>{{data.sessionInfo["会话ID"]}}</td>\
</tr>\
<tr ng-repeat="(key, value) in data.sessionInfo" ng-if="key != \'会话ID\'">\
<td>{{key}}</td>\
<td>{{value}}</td>\
</tr>\
</table>';
      }

      $scope.changeSelect = function( $event ){
         if( $( $event.target ).attr( 'class' ) == 'pull-left fa fa-check-square-o' )
         {
            $( $event.target ).removeClass('fa-check-square-o').addClass('fa-square-o') ;
         }
         else if( $( $event.target ).attr( 'class' ) == 'pull-left fa fa-square-o' )
         {
            $( $event.target ).removeClass('fa-square-o').addClass('fa-check-square-o') ;
         }
      } ;

      $scope.stopSession = function( ){
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '是的，中断会话' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '要中断该会话吗？' ) ;
         $scope.Components.Confirm.context = '会话id : Host-test-02:11810:10' ;
         $scope.Components.Confirm.ok = function(){
            return true ;
         }
      }
      
      $scope.screenResult = {
         'Role':'all',
         'Running': true,
         'Waiting': true,
         'Destroying': true
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

      $scope.changeScreen = function(){
         $scope.GridData = $.extend( true, {}, gridData ) ;
         $.each( $scope.test, function( index, value ){
            if( $scope.screenResult['Running'] == true && value['Status'] == 'Running' )
            {
               $scope.GridData['body'].push( [
                  { 'html': $compile( '<i class="fa fa-circle" style="color:#00CC66;" data-desc="正常运行中"></i>' )( $scope ) },
                  { 'html': $compile( '<a class="linkButton" ng-click="showSession()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['TID'] },
                  { 'text': value['Type'] },
                  { 'text': value['TotalDataWrite'] },
                  { 'text': value['TotalIndexWrite'] },
                  { 'text': value['TotalUpdate'] },
                  { 'text': value['TotalDelete'] },
                  { 'text': value['TotalInsert'] },
                  { 'text': value['TotalRead'] },
                  { 'html': $compile( '<div class="linkButton" ng-click="stopSession()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            }
            else if(  $scope.screenResult['Waiting'] == true && value['Status'] == 'Waiting' )
            {
               $scope.GridData['body'].push( [
                  { 'html': $compile( '<i class="fa fa-circle" style="color:#F9A937;" data-desc="该会话处于waiting状态"></i>' )( $scope ) },
                  { 'html': $compile( '<a class="linkButton" ng-click="showSession()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['TID'] },
                  { 'text': value['Type'] },
                  { 'text': value['TotalDataWrite'] },
                  { 'text': value['TotalIndexWrite'] },
                  { 'text': value['TotalUpdate'] },
                  { 'text': value['TotalDelete'] },
                  { 'text': value['TotalInsert'] },
                  { 'text': value['TotalRead'] },
                  { 'html': $compile( '<div class="linkButton" ng-click="stopSession()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            }
            else if( $scope.screenResult['Destroying'] == true && value['Status'] == 'Destroying' )
            {
               $scope.GridData['body'].push( [
                  { 'html': $compile( '<i class="fa fa-circle" style="color:#D9534F;" data-desc="该会话处于销毁状态"></i>' )( $scope ) },
                  { 'html': $compile( '<a class="linkButton" ng-click="showSession()">' + value['SessionID'] + '</a>' )( $scope ) },
                  { 'text': value['TID'] },
                  { 'text': value['Type'] },
                  { 'text': value['TotalDataWrite'] },
                  { 'text': value['TotalIndexWrite'] },
                  { 'text': value['TotalUpdate'] },
                  { 'text': value['TotalDelete'] },
                  { 'text': value['TotalInsert'] },
                  { 'text': value['TotalRead'] },
                  { 'html': $compile( '<div class="linkButton" ng-click="stopSession()" style="font-size:80%;"><i class="fa fa-stop" style="font-size:90%;padding-right:3px;"></i>中断会话</div>' )( $scope ) }
               ] )
            }
         } )   
      } ;
   } ) ;
}());