
//集群名
var _clusterName = null ;

//主机列表
var _hostList = null ;


//编辑节点列表
function editHostList( btnObj )
{
	if( $( btnObj ).text() === '编辑' )
	{
		$( btnObj ).text( '完成' ) ;
		sdbjs.parts.gridBox.updateTitle( 'hostInfoGrid', 0, 0, function( tdObj ){
			$( tdObj ).children( 'input' ).show( 200 ) ;
		} ) ;
		$.each( _hostList, function( index ){
			sdbjs.parts.gridBox.updateBody( 'hostInfoGrid', index, 0, function( tdObj ){
				$( tdObj ).children( 'input' ).get(0).checked = false ;
				$( tdObj ).children( 'input' ).show( 200 ) ;
			} ) ;
		} ) ;
	}
	else
	{
		$( btnObj ).text( '编辑' ) ;
		sdbjs.parts.gridBox.updateTitle( 'hostInfoGrid', 0, 0, function( tdObj ){
			$( tdObj ).children( 'input' ).hide( 200 ) ;
		} ) ;
		$.each( _hostList, function( index ){
			sdbjs.parts.gridBox.updateBody( 'hostInfoGrid', index, 0, function( tdObj ){
				$( tdObj ).children( 'input' ).hide( 200 ) ;
			} ) ;
		} ) ;
	}
}

//全选
function selectAll()
{
	if( $( '#editButton' ).text() === '编辑' )
	{
		editHostList( $( '#editButton' ) ) ;
	}
	$.each( _hostList, function( index ){
		sdbjs.parts.gridBox.updateBody( 'hostInfoGrid', index, 0, function( tdObj ){
			if( !$( tdObj ).children( 'input' ).is( ':disabled' ) )
			{
				$( tdObj ).children( 'input' ).get(0).checked = true ;
			}
		} ) ;
	} ) ;
}

//反选
function unSelectAll()
{
	if( $( '#editButton' ).text() === '编辑' )
	{
		editHostList( $( '#editButton' ) ) ;
	}
	else
	{
		$.each( _hostList, function( index ){
			sdbjs.parts.gridBox.updateBody( 'hostInfoGrid', index, 0, function( tdObj ){
				if( !$( tdObj ).children( 'input' ).is( ':disabled' ) )
				{
					$( tdObj ).children( 'input' ).get(0).checked = !$( tdObj ).children( 'input' ).get(0).checked ;
				}
			} ) ;
		} ) ;
	}
}

//打开删除主机的提示模态框
function openRemoveHostModal()
{
	if( $( '#editButton' ).text() === '编辑' )
	{
		editHostList( $( '#editButton' ) ) ;
		showFootStatus( 'info', 'Tip：删除主机至少选择一台主机.' ) ;
	}
	else
	{
		var checkHostNum = 0 ;
		$.each( _hostList, function( index ){
			sdbjs.parts.gridBox.updateBody( 'hostInfoGrid', index, 0, function( tdObj ){
				if( $( tdObj ).children( 'input' ).get(0).checked === true )
				{
					++checkHostNum ;
				}
			} ) ;
		} ) ;
		
		if( checkHostNum > 0 )
		{
			sdbjs.parts.modalBox.show( 'isRemoveHost' ) ;
		}
		else
		{
			showFootStatus( 'info', 'Tip：删除主机至少选择一台主机.' ) ;
		}
	}
}

//删除主机
function removeHost()
{
	sdbjs.parts.modalBox.hide( 'isRemoveHost' ) ;
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	
	var hostList = [] ;
	//获取要删除的主机
	$.each( _hostList, function( index ){
		sdbjs.parts.gridBox.updateBody( 'hostInfoGrid', index, 0, function( tdObj ){
			if( $( tdObj ).children( 'input' ).get(0).checked === true )
			{
				hostList.push( { 'HostName': _hostList[index]['HostName'] } ) ;
			}
		} ) ;
	} ) ;
	
	restRemoveHost( true, function( jsonArr, textStatus, jqXHR ){
		var taskID = jsonArr[0]['TaskID'] ;
		sdbjs.fun.saveData( 'SdbTaskID', taskID ) ;
		sdbjs.fun.saveData( 'SdbDeployModel', 'taskRemoveHost' ) ;
		gotoPage( 'uninsthost.html' ) ;
	}, function( json ){
		sdbjs.parts.loadingBox.hide( 'loading' ) ;
		showProcessError( json['detail'] ) ;
	}, null, hostList ) ;
}

//回到主页
function gotoIndex()
{
	gotoPage( 'index.html' ) ;
}

//给集群添加主机
function gotoAddHost()
{
	sdbjs.fun.saveData( 'SdbDeployModel', 'AddHost' ) ;
	gotoPage( 'scanhost.html' ) ;
}

//-------------------------- 初始化 --------------------------

//加载主机数据
function loadHostData()
{
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	restGetClusterHostsInfo( false, function( jsonArr, textStatus, jqXHR ){
		_hostList = jsonArr ;
	}, function( json ){
		showProcessError( json['detail'] ) ;
	}, function(){
		sdbjs.parts.loadingBox.hide( 'loading' ) ;
	}, _clusterName ) ;

	sdbjs.parts.loadingBox.show( 'loading' ) ;
	$.each( _hostList, function( index, hostInfo ){
		restListHostBusiness( false, function( jsonArr, textStatus, jqXHR ){
			if( jsonArr.length > 0 )
			{
				hostInfo['BusinessName'] = jsonArr[0]['BusinessName'] ;
				hostInfo['BusinessType'] = jsonArr[0]['BusinessType'] ;
			}
			else
			{
				hostInfo['BusinessName'] = '' ;
				hostInfo['BusinessType'] = '' ;
			}
		}, function( json ){
			showProcessError( json['detail'] ) ;
		}, function(){
		}, hostInfo['HostName'] ) ;
	} ) ;
	sdbjs.parts.loadingBox.hide( 'loading' ) ;
}

//加载主机列表
function loadHostList()
{
	$.each( _hostList, function( index, hostInfo ){
		var inputStr = null ;
		if( hostInfo['BusinessName'] === '' && hostInfo['BusinessType'] === '' )
		{
			inputStr = '<input style="display:none;" type="checkbox">' ;
		}
		else
		{
			inputStr = '<input style="display:none;" type="checkbox" disabled="disabled">' ;
		}
		sdbjs.parts.gridBox.addBody( 'hostInfoGrid', [{ 'text': inputStr, 'width': '0%' },
																	 { 'text': htmlEncode( hostInfo['HostName'] ), 'width': '20%' },
																	 { 'text': htmlEncode( hostInfo['IP'] ), 'width': '15%' },
																	 { 'text': htmlEncode( hostInfo['OS']['Distributor'] + ' ' + hostInfo['OS']['Release'] + ' x' +  hostInfo['OS']['Bit'] ), 'width': '20%' },
																	 { 'text': htmlEncode( hostInfo['CPU'].length ), 'width': '10%' },
																	 { 'text': htmlEncode( hostInfo['Memory']['Size'] + ' MB' ), 'width': '10%' },
																	 { 'text': htmlEncode( hostInfo['BusinessName'] ), 'width': '15%' },
																	 { 'text': htmlEncode( hostInfo['BusinessType'] ), 'width': '10%' } ] ) ;
	} ) ;
}

function createDynamicHtml()
{
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	loadHostData() ;
	loadHostList() ;
	sdbjs.parts.loadingBox.hide( 'loading' ) ;
}

function createHtml()
{
	createPublicHtml() ;

	/* 分页 */
	sdbjs.parts.tabPageBox.create( 'top2', 'tab' ) ;
	sdbjs.fun.setCSS( 'tab', { 'padding-top': 5 } ) ;
	sdbjs.parts.tabPageBox.add( 'tab', '<img width="14" src="./images/smallicon/blacks/16x16/home.png"> ' + htmlEncode( '总览' ), false, 'gotoIndex()' ) ;
	sdbjs.parts.tabPageBox.add( 'tab', '<img width="14" src="./images/smallicon/blacks/16x16/comp.png"> ' + htmlEncode( '主机列表' ), true, null ) ;
	
	/* 外框 */
	sdbjs.parts.divBox.create( 'middle', 'middleDiv', 'auto', 'variable' ) ;
	sdbjs.fun.setCSS( 'middleDiv', { 'padding': '10px' } ) ;
	
	/* 参数框 */
	sdbjs.parts.panelBox.create( 'middleDiv', 'groupListBar', 'auto', 'variable' ) ;
	sdbjs.parts.panelBox.update( 'groupListBar', function( panelTitle ){
		sdbjs.parts.divBox.create( panelTitle['name'], 'clusterNameDiv' ) ;
		sdbjs.fun.setCSS( 'clusterNameDiv', { 'text-overflow': 'ellipsis', 'overflow': 'hidden', 'white-space': 'nowrap' } ) ;
		sdbjs.parts.divBox.update( 'clusterNameDiv', function( divObj ){
			$( divObj ).text( '集群：' + _clusterName ) ;
		} ) ;
	}, function( panelBody ){
		sdbjs.parts.divBox.create( panelBody['name'], 'hostTopDiv', 'auto', 43 ) ;
		sdbjs.parts.divBox.update( 'hostTopDiv', function( divObj ){
			$( divObj ).append( '<button class="btn btn-default" onclick="editHostList(this)" id="editButton">编辑</button>' ) ;
			$( divObj ).append( '&nbsp;&nbsp;' ) ;
			sdbjs.parts.dropDownBox.create( divObj, 'hostSelect' ) ;
			sdbjs.parts.dropDownBox.update( 'hostSelect', '选择操作' ) ;
			sdbjs.parts.dropDownBox.add( 'hostSelect', htmlEncode( '全选' ), true, 'selectAll()' ) ;
			sdbjs.parts.dropDownBox.add( 'hostSelect', htmlEncode( '反选' ), true, 'unSelectAll()' ) ;
			$( divObj ).append( '&nbsp;&nbsp;' ) ;
			sdbjs.parts.dropDownBox.create( divObj, 'hostOperation' ) ;
			sdbjs.parts.dropDownBox.update( 'hostOperation', '已选定操作' ) ;
			sdbjs.parts.dropDownBox.add( 'hostOperation', htmlEncode( '删除主机' ), true, function( liObj ){
				sdbjs.fun.addClick( liObj, 'openRemoveHostModal()' ) ;
			} ) ;
			$( divObj ).append( '&nbsp;&nbsp;' ) ;
			$( divObj ).append( '<button class="btn btn-default" onclick="gotoAddHost()">添加主机</button>' ) ;
		} ) ;
		sdbjs.parts.divBox.create( panelBody['name'], 'hostBottomDiv', 'auto', 'variable' ) ;
		sdbjs.parts.gridBox.create( 'hostBottomDiv', 'hostInfoGrid', 'auto', 'variable' ) ;
		sdbjs.parts.gridBox.addTitle( 'hostInfoGrid', [{ 'text': function( tdObj ){
			$( tdObj ).css( 'height', 19 ).html( '<input style="display:none;visibility:hidden;" type="checkbox">' ) ;
		}, 'width': '0%' },
																	  { 'text': htmlEncode( '主机名' ), 'width': '20%' },
																	  { 'text': htmlEncode( 'IP' ), 'width': '15%' },
																	  { 'text': htmlEncode( '系统' ), 'width': '20%' },
																	  { 'text': htmlEncode( 'CPU核心数' ), 'width': '10%' },
																	  { 'text': htmlEncode( '内存容量' ), 'width': '10%' },
																	  { 'text': htmlEncode( '业务名' ), 'width': '15%' },
																	  { 'text': htmlEncode( '业务类型' ), 'width': '10%' } ] ) ;
	} ) ;
	
	/* ** */
	sdbjs.parts.divBox.create( 'middle', 'middle-clear', 0, 0 ) ;
	sdbjs.fun.setClass( 'middle-clear', 'clear-float' ) ;
	
	/* 确认是否要删除主机 */
	sdbjs.parts.modalBox.create( $( document.body ), 'isRemoveHost' ) ;
	sdbjs.parts.modalBox.update( 'isRemoveHost', htmlEncode( '提示' ), function( bodyObj ){
		sdbjs.parts.alertBox.create( bodyObj, 'isRemoveHostAlert' ) ;
		sdbjs.parts.alertBox.update( 'isRemoveHostAlert', htmlEncode( 'Warning：该操作是不可恢复操作，并且无法删除已经安装业务的主机.' ), 'warning' )
	}, function( footObj ){
		$( footObj ).css( 'text-align', 'right' ) ;
		sdbjs.parts.buttonBox.create( footObj, 'isRemoveHostOK' ) ;
		$( footObj ).append( '&nbsp;' ) ;
		sdbjs.parts.buttonBox.create( footObj, 'isRemoveHostClose' ) ;
		sdbjs.parts.buttonBox.update( 'isRemoveHostOK', htmlEncode( '确定' ), 'primary', null, 'removeHost()' ) ;
		sdbjs.parts.buttonBox.update( 'isRemoveHostClose', function( buttonObj ){
			$( buttonObj ).text( '关闭' ).attr( 'data-toggle', 'modalBox' ).attr( 'data-target', 'isRemoveHost' ) ;
		}, 'primary' ) ;
	} ) ;
	
}

function checkReady()
{
	var rc = true ;
	_clusterName = sdbjs.fun.getData( 'SdbClusterName' ) ;
	if( _clusterName === null )
	{
		rc = false ;
		gotoPage( 'index.html' ) ;
	}
	return rc ;
}

$(document).ready(function(){
	if( checkReady() === true )
	{
		createHtml() ;
		createDynamicHtml() ;
	}
} ) ;