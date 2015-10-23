// --------------------- Data.Database.Index ---------------------
var _DataDatabaseIndex = {} ;

//获取所有cs信息
_DataDatabaseIndex.getCSInfo = function( $scope, SdbRest ){
   var sql ;
   if( $scope.moduleMode == 'standalone' )
   {
      sql = 'SELECT Name, PageSize/1024, LobPageSize/1024, GroupName, TotalRecords, FreeDataSize/1048576, FreeIndexSize/1048576, FreeLobSize/1048576, FreeSize/1048576, MaxDataCapSize/1073741824, MaxIndexCapSize/1073741824, MaxLobCapSize/1073741824, TotalDataSize/1048576, TotalIndexSize/1048576, TotalLobSize/1048576, TotalSize/1048576 FROM $SNAPSHOT_CS' ;
   }
   else
   {
      sql = 'SELECT Name, PageSize/1024, LobPageSize/1024, GroupName, TotalRecords, FreeDataSize/1048576, FreeIndexSize/1048576, FreeLobSize/1048576, FreeSize/1048576, MaxDataCapSize/1073741824, MaxIndexCapSize/1073741824, MaxLobCapSize/1073741824, TotalDataSize/1048576, TotalIndexSize/1048576, TotalLobSize/1048576, TotalSize/1048576 FROM $SNAPSHOT_CS WHERE NodeSelect="master"' ;
   }

   //获取cs的信息
   SdbRest.Exec( sql, function( csList ){
      $scope.csList = [] ;
      if( csList.length == 1 && csList[0]['Name'] == null ) csList = [] ;
      $scope.csInfo = csList ;
      $.each( csList, function( index, csInfo ){
         var csListIndex = -1 ;
         //查找cs列表是否已经存在该cs
         $.each( $scope.csList, function( index2, csInfo2 ){
            if( csInfo['Name'] == csInfo2['Name'] )
            {
               csListIndex = index2 ;
               return false ;
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
            $scope.csList[csListIndex]['Info']['MaxDataCapSize']  += csInfo['MaxDataCapSize'] ;
            $scope.csList[csListIndex]['Info']['MaxIndexCapSize'] += csInfo['MaxIndexCapSize'] ;
            $scope.csList[csListIndex]['Info']['MaxLobCapSize']   += csInfo['MaxLobCapSize'] ;
            $scope.csList[csListIndex]['Info']['TotalSize']       += csInfo['TotalSize'] ;
            $scope.csList[csListIndex]['Info']['FreeSize']        += csInfo['FreeSize'] ;

            if( csInfo['GroupName'] != null )
            {
               $scope.csList[csListIndex]['GroupName'].push( { 'key': csInfo['GroupName'], 'value': index } ) ;
            }
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
               'clNum': 0,
               'GroupName': ( csInfo['GroupName'] == null ? [] : [ { 'value': index, 'key': csInfo['GroupName'] } ] ),
               'hide': false,
               'color': color,
               'show': false,
               'Info': {
                  'Name':            csInfo['Name'],
                  'PageSize':        csInfo['PageSize'],
                  'LobPageSize':     csInfo['LobPageSize'],
                  'TotalRecords':    csInfo['TotalRecords'],
                  'FreeDataSize':    csInfo['FreeDataSize'],
                  'FreeIndexSize':   csInfo['FreeIndexSize'],
                  'FreeLobSize':     csInfo['FreeLobSize'],
                  'FreeSize':        csInfo['FreeSize'],
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
         csInfo['PageSize']        = csInfo['PageSize'] + 'KB' ;
         csInfo['LobPageSize']     = csInfo['LobPageSize'] + 'KB' ;
         csInfo['TotalRecords']    = fixedNumber( csInfo['TotalRecords'], 2 ) + 'MB' ;
         csInfo['TotalDataSize']   = fixedNumber( csInfo['TotalDataSize'], 2 ) + 'MB' ;
         csInfo['FreeDataSize']    = fixedNumber( csInfo['FreeDataSize'], 2 ) + 'MB' ;
         csInfo['TotalIndexSize']  = fixedNumber( csInfo['TotalIndexSize'], 2 ) + 'MB' ;
         csInfo['FreeIndexSize']   = fixedNumber( csInfo['FreeIndexSize'], 2 ) + 'MB' ;
         csInfo['TotalLobSize']    = fixedNumber( csInfo['TotalLobSize'], 2 ) + 'MB' ;
         csInfo['FreeLobSize']     = fixedNumber( csInfo['FreeLobSize'], 2 ) + 'MB' ;
         csInfo['MaxDataCapSize']  = fixedNumber( csInfo['MaxDataCapSize'], 2 ) + 'GB' ;
         csInfo['MaxIndexCapSize'] = fixedNumber( csInfo['MaxIndexCapSize'], 2 ) + 'GB' ;
         csInfo['MaxLobCapSize']   = fixedNumber( csInfo['MaxLobCapSize'], 2 ) + 'GB' ;
         csInfo['TotalSize']       = fixedNumber( csInfo['TotalSize'], 2 ) + 'MB' ;
         csInfo['FreeSize']        = fixedNumber( csInfo['FreeSize'], 2 ) + 'MB' ;
      } ) ;
      $.each( $scope.csList, function( index, csInfo ){
         csInfo['Info']['PageSize']        = csInfo['Info']['PageSize'] + 'KB' ;
         csInfo['Info']['LobPageSize']     = csInfo['Info']['LobPageSize'] + 'KB' ;
         csInfo['Info']['TotalRecords']    = fixedNumber( csInfo['Info']['TotalRecords'], 2 ) + 'MB' ;
         csInfo['Info']['TotalDataSize']   = fixedNumber( csInfo['Info']['TotalDataSize'], 2 ) + 'MB' ;
         csInfo['Info']['FreeDataSize']    = fixedNumber( csInfo['Info']['FreeDataSize'], 2 ) + 'MB' ;
         csInfo['Info']['TotalIndexSize']  = fixedNumber( csInfo['Info']['TotalIndexSize'], 2 ) + 'MB' ;
         csInfo['Info']['FreeIndexSize']   = fixedNumber( csInfo['Info']['FreeIndexSize'], 2 ) + 'MB' ;
         csInfo['Info']['TotalLobSize']    = fixedNumber( csInfo['Info']['TotalLobSize'], 2 ) + 'MB' ;
         csInfo['Info']['FreeLobSize']     = fixedNumber( csInfo['Info']['FreeLobSize'], 2 ) + 'MB' ;
         csInfo['Info']['MaxDataCapSize']  = fixedNumber( csInfo['Info']['MaxDataCapSize'], 2 ) + 'GB' ;
         csInfo['Info']['MaxIndexCapSize'] = fixedNumber( csInfo['Info']['MaxIndexCapSize'], 2 ) + 'GB' ;
         csInfo['Info']['MaxLobCapSize']   = fixedNumber( csInfo['Info']['MaxLobCapSize'], 2 ) + 'GB' ;
         csInfo['Info']['TotalSize']       = fixedNumber( csInfo['Info']['TotalSize'], 2 ) + 'MB' ;
         csInfo['Info']['FreeSize']        = fixedNumber( csInfo['Info']['FreeSize'], 2 ) + 'MB' ;
      } ) ;
      $scope.showCSInfo( 0 ) ;
      $scope.$apply() ;
      _DataDatabaseIndex.getCLInfo( $scope, SdbRest ) ;
   }, function( errorInfo ){
      $scope.Components.Confirm.isShow = true ;
      $scope.Components.Confirm.type = 1 ;
      $scope.Components.Confirm.title = $scope.autoLanguage( '获取数据失败' ) ;
      $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
      $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
      $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
      $scope.Components.Confirm.ok = function(){
         $scope.Components.Confirm.isShow = false ;
         _DataDatabaseIndex.getCSInfo( $scope, SdbRest ) ;
      }
   }, function(){
      _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
   } ) ;
}

//获取所有cl信息
_DataDatabaseIndex.getCLInfo = function( $scope, SdbRest )
{
   if( $scope.moduleMode == 'standalone' )
   {
      sql = 'SELECT t1.Name, t1.Details.ID, t1.Details.LogicalID, t1.Details.Sequence, t1.Details.GroupName, t1.Details.Status, t1.Details.Indexes, t1.Details.TotalRecords, t1.Details.TotalDataPages, t1.Details.TotalIndexPages, t1.Details.TotalLobPages, t1.Details.TotalDataFreeSpace/1048576, t1.Details.TotalIndexFreeSpace/1048576 FROM (SELECT * FROM $SNAPSHOT_CL split BY Details) AS t1' ;
   }
   else
   {
      sql = 'SELECT t2.Name, t1.Details.ID, t1.Details.LogicalID, t1.Details.Sequence, t1.Details.GroupName, t1.Details.Status, t1.Details.Indexes, t1.Details.TotalRecords, t1.Details.TotalDataPages, t1.Details.TotalIndexPages, t1.Details.TotalLobPages, t1.Details.TotalDataFreeSpace/1048576, t1.Details.TotalIndexFreeSpace/1048576, t2.IsMainCL, t2.MainCLName, t2.ShardingKey, t2.ShardingType FROM (SELECT * FROM $SNAPSHOT_CL WHERE NodeSelect="master" split BY Details) AS t1 RIGHT OUTER JOIN $SNAPSHOT_CATA AS t2 ON t1.Name = t2.Name' ;
   }

   //获取cl信息
   SdbRest.Exec( sql, function( clList ){
      $scope.clList = [] ;
      if( clList.length == 1 && clList[0]['Name'] == null ) clList = [] ;
      $.each( $scope.csList, function( index2, csInfo ){
         csInfo['clNum'] = 0 ;
      } ) ;
      $scope.clInfo = clList ;
      $.each( clList, function( index, clInfo ){
         var fullName = clInfo['Name'].split( '.' ) ;
         var csName = fullName[0] ;
         var clName = fullName[1] ;
         var clListIndex = -1 ;
         //查找cl列表是否已经存在该cl
         $.each( $scope.clList, function( index2, clInfo2 ){
            if( clName == clInfo2['Name'] && csName == clInfo2['csName'] )
            {
               clListIndex = index2 ;
               return false ;
            }
         } ) ;
         //存在则累加
         if( clListIndex >= 0 )
         {
            $scope.clList[clListIndex]['Record']                      += clInfo['TotalRecords'] ;
            $scope.clList[clListIndex]['Index']                       += clInfo['Indexes'] ;
            $scope.clList[clListIndex]['Info']['Indexes']             += clInfo['Indexes'] ;
            $scope.clList[clListIndex]['Info']['TotalRecords']        += clInfo['TotalRecords'] ;
            $scope.clList[clListIndex]['Info']['TotalDataPages']      += clInfo['TotalDataPages'] ;
            $scope.clList[clListIndex]['Info']['TotalIndexPages']     += clInfo['TotalIndexPages'] ;
            $scope.clList[clListIndex]['Info']['TotalLobPages']       += clInfo['TotalLobPages'] ;
            $scope.clList[clListIndex]['Info']['TotalDataFreeSpace']  += clInfo['TotalDataFreeSpace'] ;
            $scope.clList[clListIndex]['Info']['TotalIndexFreeSpace'] += clInfo['TotalIndexFreeSpace'] ;
            if( clInfo['GroupName'] != null )
            {
               $scope.clList[clListIndex]['GroupName'].push( { 'key': clInfo['GroupName'], 'value': index } ) ;
            }
         }
         //不存在则创建
         else
         {
            var shardingType, shardingTypeDesc ;
            if( clInfo['IsMainCL'] == true )
            {
               shardingType = $scope.autoLanguage( '垂直' ) ;
               shardingTypeDesc = $scope.autoLanguage( '垂直分区' ) ;
            }
            else
            {
               if( clInfo['ShardingType'] == 'range' )
               {
                  shardingType = $scope.autoLanguage( '水平' ) ;
                  shardingTypeDesc = $scope.autoLanguage( '水平范围分区' ) ;
               }
               else if( clInfo['ShardingType'] == 'hash' )
               {
                  shardingType = $scope.autoLanguage( '水平' ) ;
                  shardingTypeDesc = $scope.autoLanguage( '水平散列分区' ) ;
               }
               else
               {
                  shardingType = $scope.autoLanguage( '普通' ) ;
                  shardingTypeDesc = $scope.autoLanguage( '普通' ) ;
               }
            }
            var isHide = false ;
            var pageSize = 0 ;
            var lobPageSize = 0 ;
            //计算cl数量
            $.each( $scope.csList, function( index2, csInfo ){
               if( csInfo['Name'] == csName )
               {
                  ++csInfo['clNum'] ;
                  if( csInfo['clNum'] > 5 )
                  {
                     isHide = true ;
                  }
                  pageSize = parseInt( csInfo['Info']['PageSize'] ) ;
                  lobPageSize = parseInt( csInfo['Info']['LobPageSize'] ) ;
                  return false ;
               }
            } ) ;
            $scope.clList.push( {
               'csName': csName,
               'Name': clName,
               'type': shardingType,
               'typeDesc': shardingTypeDesc,
               'GroupName': ( clInfo['GroupName'] == null ? [] : [ { 'value': index, 'key': clInfo['GroupName'] } ] ),
               'Lob': 0,
               'Record': clInfo['TotalRecords'],
               'Index': clInfo['Indexes'],
               'Ratio': 0,
               'IsMainCL': clInfo['IsMainCL'],
               'hide': isHide,
               'pageSize': pageSize,
               'lobPageSize': lobPageSize,
               'Info': {
                  'Name':                clName,
                  'IsMainCL':            clInfo['IsMainCL'],
                  'MainCLName':          clInfo['MainCLName'],
                  'ShardingKey':         clInfo['ShardingKey'],
                  'ShardingType':        clInfo['ShardingType'],
                  'ID':                  clInfo['ID'],
                  'LogicalID':           clInfo['LogicalID'],
                  'Sequence':            clInfo['Sequence'],
                  'GroupName':           clInfo['GroupName'],
                  'Status':              clInfo['Status'],
                  'Indexes':             clInfo['Indexes'],
                  'TotalRecords':        clInfo['TotalRecords'],
                  'TotalDataPages':      clInfo['TotalDataPages'],
                  'TotalIndexPages':     clInfo['TotalIndexPages'],
                  'TotalLobPages':       clInfo['TotalLobPages'],
                  'TotalDataFreeSpace':  clInfo['TotalDataFreeSpace'],
                  'TotalIndexFreeSpace': clInfo['TotalIndexFreeSpace']
               }
            } ) ;
         }
         clInfo['Name'] = clName ;
         clInfo['TotalDataFreeSpace'] = fixedNumber( clInfo['TotalDataFreeSpace'], 2 ) + 'MB' ;
         clInfo['TotalIndexFreeSpace'] = fixedNumber( clInfo['TotalIndexFreeSpace'], 2 ) + 'MB' ;
      } ) ;
      $.each( $scope.clList, function( index, clInfo ){
         if( typeof( clInfo['Info']['TotalDataFreeSpace'] ) == 'number' )
         {
            clInfo['Info']['TotalDataFreeSpace'] = fixedNumber( clInfo['Info']['TotalDataFreeSpace'], 2 ) + 'MB' ;
         }
         if( typeof( clInfo['Info']['TotalIndexFreeSpace'] ) == 'number' )
         {
            clInfo['Info']['TotalIndexFreeSpace'] = fixedNumber( clInfo['Info']['TotalIndexFreeSpace'], 2 ) + 'MB' ;
         }
         if( typeof( clInfo['Info']['TotalIndexPages'] ) == 'number' && typeof( clInfo['Info']['TotalDataPages'] ) == 'number' && typeof( clInfo['Info']['TotalLobPages'] ) == 'number' && clInfo['Info']['TotalDataPages'] + clInfo['Info']['TotalLobPages'] > 0 )
         {
            clInfo['Ratio'] = fixedNumber( ( clInfo['Info']['TotalIndexPages'] * 100 * clInfo['pageSize'] ) / ( clInfo['Info']['TotalDataPages'] * clInfo['pageSize'] + clInfo['Info']['TotalLobPages'] * clInfo['lobPageSize'] ), 2 ) ;
         }
      } ) ;
      $scope.$apply() ;
   }, function( errorInfo ){
      $scope.Components.Confirm.isShow = true ;
      $scope.Components.Confirm.type = 1 ;
      $scope.Components.Confirm.title = $scope.autoLanguage( '获取数据失败' ) ;
      $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
      $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
      $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
      $scope.Components.Confirm.ok = function(){
         $scope.Components.Confirm.isShow = false ;
         _DataDatabaseIndex.getCLInfo( $scope, SdbRest ) ;
      }
   }, function(){
      _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
   } ) ;
}

//添加自动调节宽高
_DataDatabaseIndex.auto2SetWidthAndHeight = function( InheritSize ){
   InheritSize.append( $( '#MetadataIndex' ) ) ;
   InheritSize.append( $( '#MetadataList' ) ) ;
   $( '#MetadataList > div' ).each( function( index, ele ){
      InheritSize.append( ele ) ;
   } ) ;
}

//初始化参数
_DataDatabaseIndex.init = function( $scope, moduleName, moduleMode ){
   //分区组列表
   $scope.GroupList = [] ;
   //业务名
   $scope.moduleName = moduleName ;
   //业务模式
   $scope.moduleMode = moduleMode ;
   //收起cl列表时，最大显示cl数
   $scope.maxShowCLNum = 5 ;
   //cs列表
   $scope.csList = [] ;
   //cs详细信息
   $scope.csInfo = [] ;
   //cl列表
   $scope.clList = [] ;
   //cl详细信息
   $scope.clInfo = [] ;
   //参数列表
   $scope.attr = {} ;
   //当前所选的group
   $scope.selectGroup = null ;
   //搜索内容
   $scope.find = '' ;
   //符合搜索的数量
   $scope.findNum = 0 ;
}

//获取分区组列表
_DataDatabaseIndex.getGroupList = function( $scope, SdbRest ){
   if( $scope.moduleMode == 'distribution' )
   {
      //获取分区组列表
      var exec = function(){
         var data = { 'cmd': 'list groups' } ;
         SdbRest.DataOperation( data, function( groups ){
            $.each( groups, function( index, groupInfo ){
               if( groupInfo['Role'] == 0 )
               {
                  $scope.GroupList.push( groupInfo ) ;
               }
            } ) ;
         }, function( errorInfo ){
            $scope.Components.Confirm.isShow = true ;
            $scope.Components.Confirm.type = 1 ;
            $scope.Components.Confirm.title = $scope.autoLanguage( '获取数据失败' ) ;
            $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
            $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
            $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
            $scope.Components.Confirm.ok = function(){
               $scope.Components.Confirm.isShow = false ;
               exec() ;
            }
         }, function(){
            _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
         } ) ;
      } ;
      exec() ;
   }
}

//跳到记录页面
_DataDatabaseIndex.gotoRecord = function( $scope, $location, SdbFunction, csIndex, clIndex ){
   var csName = $scope.csList[csIndex]['Name'] ;
   var clName = $scope.clList[clIndex]['Info']['Name'] ;
   var clType = '' ;
   if( $scope.clList[clIndex]['Info']['IsMainCL'] == true )
   {
      clType = 'main' ;
   }
   SdbFunction.LocalData( 'SdbCsName', csName ) ;
   SdbFunction.LocalData( 'SdbClName', clName ) ;
   SdbFunction.LocalData( 'SdbClType', clType ) ;
   $location.path( 'Data/Operate/Record' ) ;
}

//搜索cs和cl
_DataDatabaseIndex.searchCSAndCL = function( $scope, fullName ){
   $scope.findNum = 0 ;
   $scope.find = fullName ;
   var isNumber = !isNaN( fullName ) ;
   var len = fullName.length ;
   var pointIndex = fullName.indexOf( '.' ) ;
   fullName = trim( fullName ) ;
   if( len > 0 )
   {
      //xxx.???
      if( pointIndex > 0 )
      {
         //xxx.xxx
         if( pointIndex < len - 1 )
         {
            var csName = '', clName = '' ;
            var tmp = fullName.split( '.' ) ;
            csName = tmp[0] ;
            clName = tmp[1] ;
            $.each( $scope.csList, function( index, csInfo ){
               csInfo.hide = !( csInfo['Name'] == csName ) ;
            } ) ;
            $.each( $scope.clList, function( index, clInfo ){
               clInfo.hide = !( clInfo['Name'] == clName && clInfo['csName'] == csName ) ;
            } ) ;
         }
         //xxx.
         else
         {
            var csName = '' ;
            var tmp = fullName.split( '.' ) ;
            csName = tmp[0] ;
            $.each( $scope.csList, function( index, csInfo ){
               csInfo.hide = !( csInfo['Name'] == csName ) ;
            } ) ;
            $.each( $scope.clList, function( index, clInfo ){
               clInfo.hide = !( clInfo['csName'] == csName ) ;
            } ) ;
         }
      }
      //.xxx
      else if( pointIndex == 0 )
      {
         //.
         if( len == 0 )
         {
            $.each( $scope.clList, function( index, clInfo ){
               clInfo.hide = false ;
            } ) ;
            $.each( $scope.csList, function( index, csInfo ){
               csInfo.hide = false ;
            } ) ;
         }
         //.xx
         else
         {
            var clName = '' ;
            var tmp = fullName.split( '.' ) ;
            clName = tmp[1] ;
            $.each( $scope.csList, function( index, csInfo ){
               csInfo.hide = true ;
            } ) ;
            $.each( $scope.clList, function( index, clInfo ){
               clInfo.hide = true
               if( clInfo['Name'] == clName )
               {
                  clInfo.hide = false ;
                  $.each( $scope.csList, function( index, csInfo ){
                     if( csInfo['Name'] == clInfo['csName'] )
                     {
                        csInfo.hide = false ;
                        return false ;
                     }
                  } ) ;
               }
            } ) ;
         }
      }
      //xxx
      else
      {
         $.each( $scope.csList, function( index, csInfo ){
            csInfo.hide = true ;
         } ) ;
         $.each( $scope.clList, function( index, clInfo ){
            clInfo.hide = true ;
            if( clInfo['Name'].indexOf( fullName ) >= 0 )
            {
               clInfo.hide = false ;
               $.each( $scope.csList, function( index, csInfo ){
                  if( csInfo['Name'] == clInfo['csName'] )
                  {
                     csInfo.hide = false ;
                     return false ;
                  }
               } ) ;
            }
         } ) ;
         $.each( $scope.csList, function( index, csInfo ){
            if( csInfo['Name'].indexOf( fullName ) >= 0 )
            {
               csInfo.hide = false ;
               $.each( $scope.clList, function( index, clInfo ){
                  if( clInfo['csName'] == csInfo['Name'] )
                  {
                     clInfo.hide = false ;
                  }
               } ) ;
            }
         } ) ;
         if( isNumber )
         {
            var num = parseInt( fullName ) ;
            $.each( $scope.clList, function( index, clInfo ){
               if( clInfo['Lob'] == num || clInfo['Record'] == num || clInfo['Index'] == num || clInfo['Ratio'] == num )
               {
                  clInfo.hide = false ;
                  $.each( $scope.csList, function( index, csInfo ){
                     if( csInfo['Name'] == clInfo['csName'] )
                     {
                        csInfo.hide = false ;
                        return false ;
                     }
                  } ) ;
               }
            } ) ;
         }
      }
   }
   else
   {
      $.each( $scope.clList, function( index, clInfo ){
         clInfo.hide = false ;
      } ) ;
      $.each( $scope.csList, function( index, csInfo ){
         csInfo.hide = false ;
         $scope.clTableHide( csInfo['Name'] ) ;
      } ) ;
   }
   $.each( $scope.csList, function( index, csInfo ){
      if( csInfo.hide == false )
      {
         ++$scope.findNum ;
      }
   } ) ;
}

//展开cs下所有的cl
_DataDatabaseIndex.clTableShow = function( $scope, csName ){
   $.each( $scope.clList, function( index, clInfo ){
      if( clInfo.csName == csName )
      {
         clInfo.hide = false ;
      }
   } ) ;
   $.each( $scope.csList, function( index, csInfo ){
      if( csInfo.Name == csName )
      {
         csInfo.show = true ;
         return false ;
      }
   } ) ;
}

//收起cs下所有的cl
_DataDatabaseIndex.clTableHide = function( $scope, csName ){
   var showNum = 0 ;
   $.each( $scope.clList, function( index, clInfo ){
      if( clInfo.csName == csName )
      {
         ++showNum ;
         clInfo.hide = ( showNum > $scope.maxShowCLNum ) ;
         if( $scope.showType == 'cl' && $scope.clID == index )
         {
            clInfo.hide = false ;
         }
      }
   } ) ;
   $.each( $scope.csList, function( index, csInfo ){
      if( csInfo.Name == csName )
      {
         csInfo.show = false ;
         return false ;
      }
   } ) ;
}

//展示CS属性
_DataDatabaseIndex.showCSInfo = function( $scope, index ){
   $scope.showType = 'cs' ;
   $scope.csID = index ;
   $scope.selectGroup = null ;
   $scope.attr = $.extend( true, {}, $scope.csList[index] ) ;
}

//展示CL属性
_DataDatabaseIndex.showCLInfo = function( $scope, csIndex, clIndex ){
   $scope.showType = 'cl' ;
   $scope.csID = csIndex ;
   $scope.clID = clIndex ;
   $scope.selectGroup = null ;
   $scope.attr = $.extend( true, {}, $scope.clList[clIndex] ) ;
}

//打开 创建集合空间 的窗口
_DataDatabaseIndex.showCreateCS = function( $scope, SdbRest ){
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
               "max": 127,
               "ban": [ ".", "$" ]
            }
         },
         {
            "name": "pageSize",
            "webName": $scope.autoLanguage( '数据页大小' ),
            "type": "select",
            "desc": $scope.autoLanguage( '数据页大小创建后不可更改。' ),
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
            "desc": $scope.autoLanguage( '所属域必须已经存在。' ),
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
            "desc": $scope.autoLanguage( 'Lob数据页大小创建后不可更改。' ),
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
         var exec = function(){
            SdbRest.DataOperation( data, function( json ){
               _DataDatabaseIndex.getCSInfo( $scope, SdbRest ) ;
            }, function( errorInfo ){
               $scope.Components.Confirm.isShow = true ;
               $scope.Components.Confirm.type = 1 ;
               $scope.Components.Confirm.title = $scope.autoLanguage( '创建集合空间失败' ) ;
               $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
               $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
               $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
               $scope.Components.Confirm.ok = function(){
                  $scope.Components.Confirm.isShow = false ;
                  exec() ;
               }
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            } ) ;
         } ;
         exec() ;
      }
      return isAllClear ;
   }
}

//打开 删除集合空间 的窗口
_DataDatabaseIndex.showRemoveCS = function( $scope, SdbRest ){
   var csValid = [] ;
   //获取集合空间名显示在列表中
   $.each( $scope.csList, function( index, csInfo ){
      csValid.push( { 'key': csInfo['Name'], 'value': index } ) ;
   } ) ;
   $scope.Components.Modal.icon = 'fa-trash-o' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '删除集合空间' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "name",
            "webName": $scope.autoLanguage( '集合空间名' ),
            "type": "select",
            "value": csValid[ $scope.csID ]['value'],
            "desc": $scope.autoLanguage( '选择集合空间' ),
            "valid": csValid,
            "onChange": function( name, key, value ){
               $scope.Components.Modal.Table = $scope.csList[value]['Info'] ;
            }
         }
      ]
   } ;
   $scope.Components.Modal.Table = $scope.csList[ $scope.csID ]['Info'] ;
   $scope.Components.Modal.Context = '\
<div form-create para="data.form"></div>\
<table class="table loosen border">\
<tr>\
<td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
<td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
</tr>\
<tr ng-repeat="(key, value) in data.Table track by $index" ng-if="key != \'Name\'">\
<td>{{key}}</td>\
<td>{{value}}</td>\
</tr>\
</table>' ;
   $scope.Components.Modal.ok = function(){
      var isAllClear = $scope.Components.Modal.form.check() ;
      if( isAllClear )
      {
         var value = $scope.Components.Modal.form.getValue() ;
         var csName = $scope.csList[ value['name'] ]['Name'] ;
         var data = { 'cmd': 'drop collectionspace', 'name': csName } ;
         var exec = function(){
            SdbRest.DataOperation( data, function( json ){
               if( $scope.csID == value['name'] )
               {
                  $scope.showCSInfo( 0 ) ;
               }
               _DataDatabaseIndex.getCSInfo( $scope, SdbRest ) ;
            }, function( errorInfo ){
               $scope.Components.Confirm.isShow = true ;
               $scope.Components.Confirm.type = 1 ;
               $scope.Components.Confirm.title = $scope.autoLanguage( '删除集合空间失败' ) ;
               $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
               $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
               $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
               $scope.Components.Confirm.ok = function(){
                  $scope.Components.Confirm.isShow = false ;
                  exec() ;
               }
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            } ) ;
         } ;
         exec() ;
      }
      return isAllClear ;
   }
}

//打开 创建集合 的窗口
_DataDatabaseIndex.showCreateCL = function( $scope, SdbRest ){
   //把获取表单的值转换成请求参数
   var modalValue2Create = function( valueJson ){
      var fullName = $scope.csList[ valueJson['csName'] ]['Name'] + '.' + valueJson['clName'] ;
      var rv = { 'name': fullName, 'options': {} } ;
      if( $scope.moduleMode == 'distribution' )
      {
         if( valueJson['type'] == '' )
         {
            rv['options']['Compressed'] = valueJson['Compressed'] ;
            if( valueJson['Group'] > 0 )
            {
               rv['options']['Group'] = $scope.GroupList[ valueJson['Group'] - 1 ]['GroupName'] ;
            }
         }
         else if( valueJson['type'] == 'range' )
         {
            rv['options']['ShardingType'] = 'range' ;
            rv['options']['ShardingKey'] = {} ;
            $.each( valueJson['ShardingKey'], function( index, field ){
               rv['options']['ShardingKey'][ field['field'] ] = field['sort'] ;
            } ) ;
            rv['options']['ReplSize'] = valueJson['ReplSize'] ;
            rv['options']['Compressed'] = valueJson['Compressed'] ;
            if( valueJson['Group'] > 0 )
            {
               rv['options']['Group'] = $scope.GroupList[ valueJson['Group'] - 1 ]['GroupName'] ;
            }
            rv['options']['EnsureShardingIndex'] = valueJson['EnsureShardingIndex'] ;
         }
         else if( valueJson['type'] == 'hash' )
         {
            rv['options']['ShardingType'] = 'hash' ;
            rv['options']['ShardingKey'] = {} ;
            $.each( valueJson['ShardingKey'], function( index, field ){
               rv['options']['ShardingKey'][ field['field'] ] = field['sort'] ;
            } ) ;
            rv['options']['Partition'] = valueJson['Partition'] ;
            rv['options']['ReplSize'] = valueJson['ReplSize'] ;
            rv['options']['Compressed'] = valueJson['Compressed'] ;
            rv['options']['AutoSplit'] = valueJson['AutoSplit'] ;
            if( valueJson['Group'] > 0 )
            {
               rv['options']['Group'] = $scope.GroupList[ valueJson['Group'] - 1 ]['GroupName'] ;
            }
            rv['options']['EnsureShardingIndex'] = valueJson['EnsureShardingIndex'] ;
         }
         else if( valueJson['type'] == 'main' )
         {
            rv['options']['IsMainCL'] = true ;
            rv['options']['ShardingKey'] = {} ;
            $.each( valueJson['ShardingKey'], function( index, field ){
               rv['options']['ShardingKey'][ field['field'] ] = field['sort'] ;
            } ) ;
            rv['options']['ReplSize'] = valueJson['ReplSize'] ;
         }
      }
      else if( $scope.moduleMode == 'standalone' )
      {
         rv['options']['Compressed'] = valueJson['Compressed'] ;
      }
      return rv ;
   }
   var createCLExec = function( data ){
      SdbRest.DataOperation( data, function( json ){
         _DataDatabaseIndex.getCLInfo( $scope, SdbRest ) ;
      }, function( errorInfo ){
         $scope.Components.Confirm.isShow = true ;
         $scope.Components.Confirm.type = 1 ;
         $scope.Components.Confirm.title = $scope.autoLanguage( '创建集合失败' ) ;
         $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
         $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
         $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
         $scope.Components.Confirm.ok = function(){
            $scope.Components.Confirm.isShow = false ;
            createCLExec( data ) ;
         }
         _DataDatabaseIndex.getCLInfo( $scope, SdbRest ) ;
      }, function(){
         _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
      } ) ;
   }
   var csValid = [] ;
   //获取集合空间名显示在列表中
   $.each( $scope.csList, function( index, csInfo ){
      csValid.push( { 'key': csInfo['Name'], 'value': index } ) ;
   } ) ;
   var groupValid = [ { "key": "Auto", "value": 0 } ] ;
   //获取分区组列表
   $.each( $scope.GroupList, function( index, groupInfo ){
      groupValid.push( { 'key': groupInfo['GroupName'], 'value': index + 1 } ) ;
   } ) ;
   $scope.Components.Modal.icon = 'fa-plus' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '创建集合' ) ;
   $scope.Components.Modal.isShow = true ;
   if( $scope.moduleMode == 'distribution' )
   {
      $scope.Components.Modal.formShow = 1 ;
      $scope.Components.Modal.form1 = {
         inputList: [
            {
                 
               "name": "csName",
               "webName": $scope.autoLanguage( '集合空间' ),
               "type": "select",
               "value": $scope.csID,
               'valid': csValid
            },
            {
                 
               "name": "type",
               "webName": $scope.autoLanguage( '集合类型' ),
               "type": "select",
               "value": "",
               "valid": [
                  { "key": $scope.autoLanguage( '普通' ), "value": "" },
                  { "key": $scope.autoLanguage( '水平范围分区' ), "value": "range" },
                  { "key": $scope.autoLanguage( '水平散列分区' ), "value": "hash" },
                  { "key": $scope.autoLanguage( '垂直分区' ), "value": "main" }         
               ],
               "onChange": function( name, key, value ){
                  if( value == '' )
                  {
                     $scope.Components.Modal.formShow = 1 ;
                  }
                  else if( value == 'range' )
                  {
                     $scope.Components.Modal.formShow = 2 ;
                  }
                  else if( value == 'hash' )
                  {
                     $scope.Components.Modal.formShow = 3 ;
                  }
                  else if( value == 'main' )
                  {
                     $scope.Components.Modal.formShow = 4 ;
                  }
                  $scope.Components.Modal.form1.inputList[1]['value'] = '' ;
               }
            },
            {
               "name": "clName",
               "webName": $scope.autoLanguage( '集合名' ),
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
               "name": "Compressed",
               "webName":  $scope.autoLanguage( '数据压缩' ),
               "type": "select",
               "required": false,
               "value": false,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
               ]
            },
            {
               "name": "Group",
               "webName":  $scope.autoLanguage( '分区组' ),
               "type": "select",
               "value": 0,
               "valid": groupValid
            }
         ]
      } ;
      $scope.Components.Modal.form2 = {
         inputList: [
            {
               "name": "csName",
               "webName": $scope.autoLanguage( '集合空间' ),
               "type": "select",
               "value": $scope.csID,
               'valid': csValid
            },
            {
               "name": "type",
               "webName": $scope.autoLanguage( '集合类型' ),
               "type": "select",
               "value": "range",
               "valid": [
                  { "key": $scope.autoLanguage( '普通' ), "value": "" },
                  { "key": $scope.autoLanguage( '水平范围分区' ), "value": "range" },
                  { "key": $scope.autoLanguage( '水平散列分区' ), "value": "hash" },
                  { "key": $scope.autoLanguage( '垂直分区' ), "value": "main" }         
               ],
               "onChange": function( name, key, value ){
                  if( value == '' )
                  {
                     $scope.Components.Modal.formShow = 1 ;
                  }
                  else if( value == 'range' )
                  {
                     $scope.Components.Modal.formShow = 2 ;
                  }
                  else if( value == 'hash' )
                  {
                     $scope.Components.Modal.formShow = 3 ;
                  }
                  else if( value == 'main' )
                  {
                     $scope.Components.Modal.formShow = 4 ;
                  }
                  $scope.Components.Modal.form2.inputList[1]['value'] = 'range' ;
               }
            },
            {
               "name": "clName",
               "webName": $scope.autoLanguage( '集合名' ),
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
               "name": "ShardingKey",
               "webName":  $scope.autoLanguage( '分区键' ),
               "required": true,
               "desc": $scope.autoLanguage( '分区键是一个或多个集合列的有续集和，其中的值用来确定每个集合所述的数据分区。' ),
               "type": "list",
               "valid": {
                  "min": 1
               },
               "child":[
                  [
                     {
                        "name": "field",
                        "webName": $scope.autoLanguage( "字段名" ),
                        "type": "string",
                        "value": "",
                        "valid": {
                           "min": 1,
                           "regex": "^[^/$].*",
                           "ban": "."
                        }
                     },
                     {
                        "name": "sort",
                        "type": "select",
                        "value": 1,
                        "valid": [
                           { "key": $scope.autoLanguage( '升序' ), "value": 1 },
                           { "key": $scope.autoLanguage( '降序' ), "value": -1 }
                        ]
                     }
                  ]
               ]
            },
            {
               "name": "ReplSize",
               "webName":  $scope.autoLanguage( '副本数' ),
               "type": "int",
               "required": true,
               "value": 1,
               "valid": {
                  "min": 0,
                  "max": 7
               }
            },
            {
               "name": "Compressed",
               "webName":  $scope.autoLanguage( '数据压缩' ),
               "type": "select",
               "value": false,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
               ]
            },
            {
               "name": "Group",
               "webName":  $scope.autoLanguage( '分区组' ),
               "type": "select",
               "value": 0,
               "valid": groupValid
            },
            {
               "name": "EnsureShardingIndex",
               "webName":  $scope.autoLanguage( '分区索引' ),
               "type": "select",
               "required": false,
               "desc": "",
               "value": true,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
               ]
            }
         ]
      } ;
      $scope.Components.Modal.form3 = {
         inputList: [
            {
               "name": "csName",
               "webName": $scope.autoLanguage( '集合空间' ),
               "type": "select",
               "value": $scope.csID,
               'valid': csValid
            },
            {
               "name": "type",
               "webName": $scope.autoLanguage( '集合类型' ),
               "type": "select",
               "value": "hash",
               "valid": [
                  { "key": $scope.autoLanguage( '普通' ), "value": "" },
                  { "key": $scope.autoLanguage( '水平范围分区' ), "value": "range" },
                  { "key": $scope.autoLanguage( '水平散列分区' ), "value": "hash" },
                  { "key": $scope.autoLanguage( '垂直分区' ), "value": "main" }         
               ],
               "onChange": function( name, key, value ){
                  if( value == '' )
                  {
                     $scope.Components.Modal.formShow = 1 ;
                  }
                  else if( value == 'range' )
                  {
                     $scope.Components.Modal.formShow = 2 ;
                  }
                  else if( value == 'hash' )
                  {
                     $scope.Components.Modal.formShow = 3 ;
                  }
                  else if( value == 'main' )
                  {
                     $scope.Components.Modal.formShow = 4 ;
                  }
                  $scope.Components.Modal.form3.inputList[1]['value'] = 'hash' ;
               }
            },
            {
               "name": "clName",
               "webName": $scope.autoLanguage( '集合名' ),
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
               "name": "ShardingKey",
               "webName":  $scope.autoLanguage( '分区键' ),
               "required": true,
               "desc": $scope.autoLanguage( '分区键是一个或多个集合列的有续集和，其中的值用来确定每个集合所述的数据分区。' ),
               "type": "list",
               "valid": {
                  "min": 1
               },
               "child":[
                  [
                     {
                        "name": "field",
                        "webName": $scope.autoLanguage( "字段名" ),
                        "type": "string",
                        "value": "",
                        "valid": {
                           "min": 1,
                           "regex": "^[^/$].*",
                           "ban": "."
                        }
                     },
                     {
                        "name": "sort",
                        "type": "select",
                        "value": 1,
                        "valid": [
                           { "key": $scope.autoLanguage( '升序' ), "value": 1 },
                           { "key": $scope.autoLanguage( '降序' ), "value": -1 }
                        ]
                     }
                  ]
               ]
            },
            {
               "name": "Partition",
               "webName":  $scope.autoLanguage( '分区数' ),
               "type": "int",
               "required": true,
               "value": 4096,
               "valid": {
                  "min": 8,
                  "max": 1048576,
                  "step": 2
               }
            },
            {
               "name": "ReplSize",
               "webName":  $scope.autoLanguage( '副本数' ),
               "type": "int",
               "required": true,
               "value": 1,
               "valid": {
                  "min": 0,
                  "max": 7
               }
            },
            {
               "name": "Compressed",
               "webName":  $scope.autoLanguage( '数据压缩' ),
               "type": "select",
               "value": false,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
               ]
            },
            {
               "name": "AutoSplit",
               "webName":  $scope.autoLanguage( '自动切分' ),
               "type": "select",
               "value": false,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
               ]
            },
            {
               "name": "Group",
               "webName":  $scope.autoLanguage( '分区组' ),
               "type": "select",
               "value": 0,
               "valid": groupValid
            },
            {
               "name": "EnsureShardingIndex",
               "webName":  $scope.autoLanguage( '分区索引' ),
               "type": "select",
               "required": false,
               "desc": "",
               "value": true,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
               ]
            }
         ]
      } ;
      $scope.Components.Modal.form4 = {
         inputList: [
            {
                 
               "name": "csName",
               "webName": $scope.autoLanguage( '集合空间' ),
               "type": "select",
               "value": $scope.csID,
               'valid': csValid
            },
            {
                 
               "name": "type",
               "webName": $scope.autoLanguage( '集合类型' ),
               "type": "select",
               "value": "main",
               "valid": [
                  { "key": $scope.autoLanguage( '普通' ), "value": "" },
                  { "key": $scope.autoLanguage( '水平范围分区' ), "value": "range" },
                  { "key": $scope.autoLanguage( '水平散列分区' ), "value": "hash" },
                  { "key": $scope.autoLanguage( '垂直分区' ), "value": "main" }         
               ],
               "onChange": function( name, key, value ){
                  if( value == '' )
                  {
                     $scope.Components.Modal.formShow = 1 ;
                  }
                  else if( value == 'range' )
                  {
                     $scope.Components.Modal.formShow = 2 ;
                  }
                  else if( value == 'hash' )
                  {
                     $scope.Components.Modal.formShow = 3 ;
                  }
                  else if( value == 'main' )
                  {
                     $scope.Components.Modal.formShow = 4 ;
                  }
                  $scope.Components.Modal.form4.inputList[1]['value'] = '' ;
               }
            },
            {
               "name": "clName",
               "webName": $scope.autoLanguage( '集合名' ),
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
               "name": "ShardingKey",
               "webName":  $scope.autoLanguage( '分区键' ),
               "required": true,
               "desc": $scope.autoLanguage( '分区键是一个或多个集合列的有续集和，其中的值用来确定每个集合所述的数据分区。' ),
               "type": "list",
               "valid": {
                  "min": 1
               },
               "child":[
                  [
                     {
                        "name": "field",
                        "webName": $scope.autoLanguage( "字段名" ),
                        "type": "string",
                        "value": "",
                        "valid": {
                           "min": 1,
                           "regex": "^[^/$].*",
                           "ban": "."
                        }
                     },
                     {
                        "name": "sort",
                        "type": "select",
                        "value": 1,
                        "valid": [
                           { "key": $scope.autoLanguage( '升序' ), "value": 1 },
                           { "key": $scope.autoLanguage( '降序' ), "value": -1 }
                        ]
                     }
                  ]
               ]
            },
            {
               "name": "ReplSize",
               "webName":  $scope.autoLanguage( '副本数' ),
               "type": "int",
               "required": true,
               "value": 1,
               "valid": {
                  "min": 0,
                  "max": 7
               }
            }
         ]
      } ;
      $scope.Components.Modal.Context = '\
<div ng-if="data.formShow == 1" form-create para="data.form1"></div>\
<div ng-if="data.formShow == 2" form-create para="data.form2"></div>\
<div ng-if="data.formShow == 3" form-create para="data.form3"></div>\
<div ng-if="data.formShow == 4" form-create para="data.form4"></div>' ;
      $scope.Components.Modal.ok = function(){
         if( $scope.Components.Modal.formShow == 1 )
         {
            var isAllClear = $scope.Components.Modal.form1.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form1.getValue() ;
               var data = modalValue2Create( value ) ;
               data['cmd'] = 'create collection' ;
               data['options'] = JSON.stringify( data['options'] ) ;
               createCLExec( data ) ;
            }
         }
         else if( $scope.Components.Modal.formShow == 2 )
         {
            var isAllClear = $scope.Components.Modal.form2.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form2.getValue() ;
               var data = modalValue2Create( value ) ;
               data['cmd'] = 'create collection' ;
               data['options'] = JSON.stringify( data['options'] ) ;
               createCLExec( data ) ;
            }
         }
         else if( $scope.Components.Modal.formShow == 3 )
         {
            var isAllClear = $scope.Components.Modal.form3.check( function( valueJson ){
               var rv = [] ;
               if( valueJson['AutoSplit'] == true && valueJson['Group'] > 0 )
               {
                  rv.push( { 'name': 'AutoSplit', 'error': sprintf( $scope.autoLanguage( '指定了?，不能开启?。' ), $scope.autoLanguage( '分区组' ), $scope.autoLanguage( '自动切分' ) ) } ) ;
                  rv.push( { 'name': 'Group', 'error': sprintf( $scope.autoLanguage( '开启了?，不能指定?。' ), $scope.autoLanguage( '自动切分' ), $scope.autoLanguage( '分区组' ) ) } ) ;
               }
               return rv ;
            } ) ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form3.getValue() ;
               var data = modalValue2Create( value ) ;
               data['cmd'] = 'create collection' ;
               data['options'] = JSON.stringify( data['options'] ) ;
               createCLExec( data ) ;
            }
         }
         else if( $scope.Components.Modal.formShow == 4 )
         {
            var isAllClear = $scope.Components.Modal.form4.check() ;
            if( isAllClear )
            {
               var value = $scope.Components.Modal.form4.getValue() ;
               var data = modalValue2Create( value ) ;
               data['cmd'] = 'create collection' ;
               data['options'] = JSON.stringify( data['options'] ) ;
               createCLExec( data ) ;
            }
         }
         return isAllClear ;
      }
   }
   else if( $scope.moduleMode == 'standalone' )
   {
      $scope.Components.Modal.form = {
         inputList: [
            {
                 
               "name": "csName",
               "webName": $scope.autoLanguage( '集合空间' ),
               "type": "select",
               "value": $scope.csID,
               'valid': csValid
            },
            {
               "name": "clName",
               "webName": $scope.autoLanguage( '集合名' ),
               "type": "string",
               "required": true,
               "value": "",
               "valid": {
                  "min": 1,
                  "ban": "."
               }
            },
            {
               "name": "Compressed",
               "webName":  $scope.autoLanguage( '数据压缩' ),
               "type": "select",
               "required": false,
               "value": false,
               "valid": [
                  { "key": $scope.autoLanguage( '开' ), "value": true },
                  { "key": $scope.autoLanguage( '关' ), "value": false }
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
            var data = modalValue2Create( value ) ;
            data['cmd'] = 'create collection' ;
            data['options'] = JSON.stringify( data['options'] ) ;
            createCLExec( data ) ;
         }
         return isAllClear ;
      }
   }
}

//打开 删除集合 的窗口
_DataDatabaseIndex.showRemoveCL = function( $scope, SdbRest ){
   var clValid = [] ;
   var clIndex = -1 ;
   $.each( $scope.clList, function( index, clInfo ){
      if( $scope.showType == 'cs' )
      {
         if( $scope.csList[ $scope.csID ]['Name'] == clInfo['csName'] && clIndex < 0 )
         {
            clIndex = index ;
         }
      }
      else
      {
         if( index == $scope.clID )
         {
            clIndex = index ;
         }
      }
      clValid.push( { 'key' : clInfo['csName'] + '.' + clInfo['Name'] , 'value' : index } );
   } ) ;
   $scope.Components.Modal.icon = 'fa-trash-o' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '删除集合' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "name",
            "webName": $scope.autoLanguage( '集合名' ),
            "type": "select",
            "value": clIndex,
            "valid": clValid,
            "onChange": function( name, key, value ){
               $scope.Components.Modal.Table = $scope.clList[value]['Info'] ;
            }
         }
      ]
   } ;
   $scope.Components.Modal.Table = $scope.clList[ clIndex ]['Info'] ;
   var maskContext = '\
<div form-create para="data.form"></div>\
<table class="table loosen border">\
<tr>\
<td style="width:40%;background-color:#F1F4F5;"><b>Key</b></td>\
<td style="width:60%;background-color:#F1F4F5;"><b>Value</b></td>\
</tr>\
<tr ng-repeat="(key, value) in data.Table track by $index" ng-if="key != \'Name\' && value != undefined">\
<td>{{key}}</td>\
<td>{{value}}</td>\
</tr>\
</table>';
   $scope.Components.Modal.Context = maskContext ;
   $scope.Components.Modal.ok = function(){
      var isAllClear = $scope.Components.Modal.form.check() ;
      if( isAllClear )
      {
         var value = $scope.Components.Modal.form.getValue() ;
         var fullName = $scope.clList[ value['name'] ]['csName'] + '.' + $scope.clList[ value['name'] ]['Name'] ;
         var data = { 'cmd': 'drop collection', 'name': fullName } ;
         var exec = function(){
            SdbRest.DataOperation( data, function( json ){
               if( $scope.clID == value['name'] )
               {
                  $scope.clID = 0 ;
                  $scope.showCSInfo( $scope.csID ) ;
               }
               _DataDatabaseIndex.getCLInfo( $scope, SdbRest ) ;
            }, function( errorInfo ){
               $scope.Components.Confirm.isShow = true ;
               $scope.Components.Confirm.type = 1 ;
               $scope.Components.Confirm.title = $scope.autoLanguage( '删除集合失败' ) ;
               $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
               $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
               $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
               $scope.Components.Confirm.ok = function(){
                  $scope.Components.Confirm.isShow = false ;
                  exec() ;
               }
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            } ) ;
         } ;
         exec() ;
      }
      return isAllClear ;
   }
}

//打开 挂载集合 的窗口 ( 未完成 )
_DataDatabaseIndex.showAttachCL = function( $scope, SdbRest ){
   var mainCL = [] ;
   var childCL = [] ;
   //获取主分区集合
   $.each( $scope.clList, function( index, clInfo ){
      if( clInfo['IsMainCL'] == true )
      {
         var id = mainCL.length ;
         mainCL.push( { 'key' : clInfo['csName'] + '.' + clInfo['Name'] , 'value' : id } ) ;
      }
      else
      {
         var id = childCL.length ;
         childCL.push( { 'key' : clInfo['csName'] + '.' + clInfo['Name'] , 'value' : id } ) ;
      }
   } ) ;
   $scope.Components.Modal.icon = 'fa-paperclip' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '挂载' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.form = {
      inputList: [
         {
            "name": "name",
            "webName": $scope.autoLanguage( '集合' ),
            "type": "select",
            "value": 0,
            "valid": mainCL
         },
         {
            "name": "attachName",
            "webName": $scope.autoLanguage( '分区' ),
            "type": "select",
            "value": 0,
            "desc": $scope.autoLanguage( '选择在主分区集合下挂载的分区。' ),
            "valid": childCL
         },
         {
            "name": "LowBound",
            "webName":  $scope.autoLanguage( '分区范围' ),
            "required": true,
            "desc": $scope.autoLanguage( '分区范围，包含两个字段“LowBound”（区间左值）以及“UpBound”（区间右值），例如：{LowBound:{a:0},UpBound:{a:100}表示取字段“a”的范围区间：[0, 100)。' ),
            "type": "list",
            "valid": {
               "min": 1
            },
            "child":[
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
                     }
                  },
                  {
                     "name": "LowBound",
                     "webName": $scope.autoLanguage( "区间左值" ),
                     "placeholder": $scope.autoLanguage( "区间左值" ),
                     "type": "string",
                     "value": ""
                  },
                  {
                     "name": "UpBound",
                     "webName": $scope.autoLanguage( "区间右值" ),
                     "placeholder": $scope.autoLanguage( "区间右值" ),
                     "type": "string",
                     "value": ""
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
         var value = $scope.Components.Modal.form.getValue() ;
         alert( JSON.stringify( value ) )
         /*
         var csName = $scope.csList[ value['name'] ]['Name'] ;
         var data = { 'cmd': 'drop collectionspace', 'name': csName } ;
         var exec = function(){
            SdbRest.DataOperation( data, function( json ){
               if( $scope.csID == value['name'] )
               {
                  $scope.showCSInfo( 0 ) ;
               }
               _DataDatabaseIndex.getCSInfo( $scope, SdbRest ) ;
            }, function( errorInfo ){
               $scope.Components.Confirm.isShow = true ;
               $scope.Components.Confirm.type = 1 ;
               $scope.Components.Confirm.title = $scope.autoLanguage( '删除集合空间失败' ) ;
               $scope.Components.Confirm.okText = $scope.autoLanguage( '重试' ) ;
               $scope.Components.Confirm.closeText = $scope.autoLanguage( '取消' ) ;
               $scope.Components.Confirm.context = sprintf( $scope.autoLanguage( '错误码: ?, ?。需要重试吗?' ), errorInfo['errno'], errorInfo['description'] ) ;
               $scope.Components.Confirm.ok = function(){
                  $scope.Components.Confirm.isShow = false ;
                  exec() ;
               }
            }, function(){
               _IndexPublic.createErrorModel( $scope, $scope.autoLanguage( '网络连接错误，请尝试按F5刷新浏览器。' ) ) ;
            } ) ;
         } ;
         exec() ;
         */
      }
      return isAllClear ;
   }
}

//打开 切分数据 的窗口 ( 未完成 )
_DataDatabaseIndex.showSplit = function( $scope, SdbRest ){
   var clValid = [] ;
   var clIndex = -1 ;
   var sourceGroupValid = [] ;
   var sourceGroupValue = 0 ;
   var groupValid = [] ;
   $.each( $scope.GroupList, function( index, groupInfo ){
      groupValid.push( { 'key': groupInfo['GroupName'], 'value': index } ) ;
   } ) ;
   $.each( $scope.clList, function( index, clInfo ){
      if( $scope.showType == 'cs' )
      {
         if( $scope.csList[ $scope.csID ]['Name'] == clInfo['csName'] && clIndex < 0 )
         {
            clIndex = index ;
            sourceGroupValid = $scope.clList[ clIndex ]['GroupName'] ;
            if( sourceGroupValid.length > 0 )
            {
               sourceGroupValue = sourceGroupValid[0]['value'] ;
            }
         }
      }
      else
      {
         if( index == $scope.clID )
         {
            clIndex = index ;
            sourceGroupValid = $scope.clList[ clIndex ]['GroupName'] ;
            if( sourceGroupValid.length > 0 )
            {
               sourceGroupValue = sourceGroupValid[0]['value'] ;
            }
         }
      }
      clValid.push( { 'key' : clInfo['csName'] + '.' + clInfo['Name'] , 'value' : index } );
   } ) ;
   $scope.Components.Modal.icon = 'fa-scissors' ;
   $scope.Components.Modal.title = $scope.autoLanguage( '切分数据' ) ;
   $scope.Components.Modal.isShow = true ;
   $scope.Components.Modal.formShow = 0 ;
   $scope.Components.Modal.form1 = {
      inputList: [
         {
            "name": "type",
            "webName": $scope.autoLanguage( '切分方式' ),
            "type": "select",
            "value": 0,
            "valid": [
               { 'key': $scope.autoLanguage( '百分比切分' ), 'value': 0 },
               { 'key': $scope.autoLanguage( '条件切分' ), 'value': 1 }
            ],
            "onChange": function( name, key, value ){
               if( value == 0 )
               {
                  $scope.Components.Modal.formShow = 0 ;
               }
               else if( value == 1 )
               {
                  $scope.Components.Modal.formShow = 1 ;
               }
               $scope.Components.Modal.form1.inputList[0]['value'] = 0 ;
            }
         },
         {
            "name": "name",
            "webName": $scope.autoLanguage( '集合名' ),
            "type": "select",
            "value": clIndex,
            "valid": clValid
         },
         {
            "name": "source",
            "webName": $scope.autoLanguage( '源分区组' ),
            "type": "select",
            "value": sourceGroupValue,
            "valid": sourceGroupValid
         },
         {
            "name": "target",
            "webName": $scope.autoLanguage( '目标分区组' ),
            "type": "select",
            "value": 0,
            "valid": groupValid
         },
         {
            "name": "percent",
            "webName": $scope.autoLanguage( '百分比切分' ),
            "type": "int",
            "value": 50,
            "valid": {
               'min': 0,
               'max': 100
            }
         }
      ]
   } ;
   $scope.Components.Modal.form2 = {
      inputList: [
         {
            "name": "type",
            "webName": $scope.autoLanguage( '切分方式' ),
            "type": "select",
            "value": 1,
            "valid": [
               { 'key': $scope.autoLanguage( '百分比切分' ), 'value': 0 },
               { 'key': $scope.autoLanguage( '条件切分' ), 'value': 1 }
            ],
            "onChange": function( name, key, value ){
               if( value == 0 )
               {
                  $scope.Components.Modal.formShow = 0 ;
               }
               else if( value == 1 )
               {
                  $scope.Components.Modal.formShow = 1 ;
               }
               $scope.Components.Modal.form2.inputList[0]['value'] = 1 ;
            }
         },
         {
            "name": "name",
            "webName": $scope.autoLanguage( '集合名' ),
            "type": "select",
            "value": clIndex,
            "valid": clValid
         },
         {
            "name": "source",
            "webName": $scope.autoLanguage( '源分区组' ),
            "type": "select",
            "value": 0,
            "valid": groupValid
         },
         {
            "name": "target",
            "webName": $scope.autoLanguage( '目标分区组' ),
            "type": "select",
            "value": 0,
            "valid": groupValid
         },
         {
            "name": "condition",
            "webName":  $scope.autoLanguage( '起始切分条件' ),
            "required": true,
            "type": "list",
            "valid": {
               "min": 1
            },
            "child":[
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
                     }
                  },
                  {
                     "name": "value",
                     "webName": $scope.autoLanguage( "值" ),
                     "placeholder": $scope.autoLanguage( "值" ),
                     "type": "string",
                     "value": ""
                  }
               ]
            ]
         },
         {
            "name": "endcondition",
            "webName":  $scope.autoLanguage( '结束范围条件' ),
            "required": true,
            "type": "list",
            "valid": {
               "min": 1
            },
            "child":[
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
                     }
                  },
                  {
                     "name": "value",
                     "webName": $scope.autoLanguage( "值" ),
                     "placeholder": $scope.autoLanguage( "值" ),
                     "type": "string",
                     "value": ""
                  }
               ]
            ]
         }
      ]
   } ;
   $scope.Components.Modal.Context = '\
<div ng-if="data.formShow == 0" form-create para="data.form1"></div>\
<div ng-if="data.formShow == 1" form-create para="data.form2"></div>' ;
   $scope.Components.Modal.ok = function(){

   }
}

//显示不同分区组下的CS或CL信息
_DataDatabaseIndex.showGroupInfo = function( $scope, index ){
   $scope.selectGroup = index ;
   if( $scope.showType == 'cs' )
   {
      if( index == null )
      {
         $scope.attr['Info'] = $scope.csList[ $scope.csID ]['Info'] ;
      }
      else
      {
         $scope.attr['Info'] = $scope.csInfo[ index ] ;
      }
   }
   else
   {
      if( index == null )
      {
         $scope.attr['Info'] = $scope.clList[ $scope.clID ]['Info'] ;
      }
      else
      {
         $scope.attr['Info'] = $scope.clInfo[ index ] ;
      }
   }
}