
//集群名
var _clusterName = null ;

//业务列表
var _businessList = null ;

//业务类型
var _businessType = null ;

//删除业务
function removeBusiness( index )
{
	sdbjs.parts.modalBox.hide( 'isRemoveBusiness' ) ;
	var businessName = _businessList[index]['BusinessName'] ;
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	restRemoveBusiness( true, function( jsonArr, textStatus, jqXHR ){
		var taskID = jsonArr[0]['TaskID'] ;
		sdbjs.fun.saveData( 'SdbTaskID', taskID ) ;
		gotoPage( 'uninstsdb.html' ) ;
	}, function( json ){
		sdbjs.parts.loadingBox.hide( 'loading' ) ;
		showProcessError( json['detail'] ) ;
	}, null, businessName ) ;
}

//回到主页
function gotoIndex()
{
	gotoPage( 'index.html' ) ;
}

//打开添加业务模态框
function openAddBusinessModal()
{
	sdbjs.fun.setCSS( 'addBusinessFootAlert', { 'display': 'none' } ) ;
	sdbjs.parts.modalBox.show( 'addBusiness' ) ;
}

//打开删除业务模态框
function openDelBusinessModal( index )
{
	sdbjs.parts.buttonBox.update( 'isRemoveBusinessOK', htmlEncode( '确定' ), 'primary', null, 'removeBusiness(' + index + ')' ) ;
	sdbjs.parts.modalBox.show( 'isRemoveBusiness' ) ;
}

//给集群添加主机
function addBusiness()
{
	var rc = true ;
	var businessName = $( '#businessName_a' ).val() ;
	var businessType = $( '#businessType_a' ).val() ;
	var hostsNum = 0 ;

	if( !checkStrName( businessName ) )
	{
		showModalError( 'addBusinessFootAlert', '业务名格式错误，业务名只能由数字字母下划线组成，并且长度在 1 - 255 个字符内.' ) ;
		return;
	}
	
	$.each( _businessList, function( index, businessInfo ){
		if( businessName === businessInfo['BusinessName'] )
		{
			rc = false ;
			return false ;
		}
	} ) ;
	
	if( rc === false )
	{
		showModalError( 'addBusinessFootAlert', '业务名已经存在.' ) ;
		return ;
	}

	restListHost( false, function( jsonArr, textStatus, jqXHR ){
		hostsNum = jsonArr.length ;
	}, function( json ){
		showProcessError( json['detail'] ) ;
	}, null, _clusterName ) ;

	if( hostsNum <= 0 )
	{
		showModalError( 'addBusinessFootAlert', '该集群还没有主机，无法添加业务.' ) ;
		return ;
	}
	
	sdbjs.fun.saveData( 'SdbBusinessName', businessName ) ;
	sdbjs.fun.saveData( 'SdbBusinessType', businessType ) ;
	sdbjs.fun.saveData( 'SdbDeployModel', 'AddBusiness' ) ;
	
	if( businessType === 'sequoiadb' )
	{
		gotoPage( 'confsdb.html' ) ;
	}
}

//-------------------------- 初始化 --------------------------

//加载主机数据
function loadBusinessData()
{
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	restGetClusterBusinessInfo( false, function( jsonArr, textStatus, jqXHR ){
		_businessList = jsonArr ;
	}, function( json ){
		showProcessError( json['detail'] ) ;
	}, function(){
		sdbjs.parts.loadingBox.hide( 'loading' ) ;
	}, _clusterName ) ;
}

//加载主机列表
function loadBusinessList()
{
	$.each( _businessList, function( index, businessInfo ){
		sdbjs.parts.gridBox.addBody( 'businessInfoGrid', [{ 'text': htmlEncode( businessInfo['BusinessName'] ), 'width': '25%' },
																		  { 'text': htmlEncode( businessInfo['BusinessType'] ), 'width': '25%' },
																		  { 'text': htmlEncode( businessInfo['DeployMod'] ), 'width': '25%' },
																		  { 'text': '<button class="btn btn-default btn-lg" onclick="openDelBusinessModal(' + index + ')">' + htmlEncode( '删除业务' ) + '</button>', 'width': '25%' } ] ) ;
	} ) ;
}

//加载业务类型
function loadBusinessType()
{
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	restQueryBusinessType( true, function( jsonArr, textStatus, jqXHR ){
		var selectObj = $( '#businessType_a' ) ;
		_businessType = jsonArr ;
		$.each( _businessType, function( index, businessInfo ){
			selectObj.append( '<option value="' + htmlEncode( businessInfo['BusinessType'] ) + '"' + ( index === 0 ? ' select' : '' ) + '>' + htmlEncode( businessInfo['BusinessType'] ) + '</option>' )
			if( index === 0 )
			{
				sdbjs.parts.tableBox.updateBody( 'addBusinessTable', 1, 2, htmlEncode( businessInfo['BusinessDesc'] ) ) ;
			}
		} ) ;
		selectObj.change( function(){
			var index = this.selectedIndex ;
			sdbjs.parts.tableBox.updateBody( 'addBusinessTable', 1, 2, htmlEncode( _businessType[index]['BusinessDesc'] ) ) ;
		} ) ;
		sdbjs.parts.loadingBox.hide( 'loading' ) ;
	}, function( json ){
		showProcessError( json['detail'] ) ;
	} ) ;
}

function createDynamicHtml()
{
	sdbjs.parts.loadingBox.show( 'loading' ) ;
	loadBusinessType() ;
	loadBusinessData() ;
	loadBusinessList() ;
	sdbjs.parts.loadingBox.hide( 'loading' ) ;
}

function createHtml()
{
	createPublicHtml() ;

	/* 分页 */
	sdbjs.parts.tabPageBox.create( 'top2', 'tab' ) ;
	sdbjs.fun.setCSS( 'tab', { 'padding-top': 5 } ) ;
	sdbjs.parts.tabPageBox.add( 'tab', '<img width="14" src="./images/smallicon/blacks/16x16/home.png"> ' + htmlEncode( '总览' ), false, 'gotoIndex()' ) ;
	sdbjs.parts.tabPageBox.add( 'tab', '<img width="14" src="./images/smallicon/blacks/16x16/layers_1.png"> ' + htmlEncode( '业务列表' ), true, null ) ;
	
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
		sdbjs.parts.divBox.create( panelBody['name'], 'businessTopDiv', 'auto', 43 ) ;
		sdbjs.parts.divBox.update( 'businessTopDiv', function( divObj ){
			$( divObj ).append( '<button class="btn btn-default" onclick="openAddBusinessModal()">添加业务</button>' ) ;
		} ) ;
		sdbjs.parts.divBox.create( panelBody['name'], 'businessBottomDiv', 'auto', 'variable' ) ;
		sdbjs.parts.gridBox.create( 'businessBottomDiv', 'businessInfoGrid', 'auto', 'variable' ) ;
		sdbjs.parts.gridBox.addTitle( 'businessInfoGrid', [{ 'text': htmlEncode( '业务名' ), 'width': '25%' },
																			{ 'text': htmlEncode( '业务类型' ), 'width': '25%' },
																			{ 'text': htmlEncode( '业务模式' ), 'width': '25%' },
																			{ 'text': htmlEncode( '操作' ), 'width': '25%' } ] ) ;
	} ) ;
	
	/* ** */
	sdbjs.parts.divBox.create( 'middle', 'middle-clear', 0, 0 ) ;
	sdbjs.fun.setClass( 'middle-clear', 'clear-float' ) ;
	
	/* 确认是否要卸载业务 */
	sdbjs.parts.modalBox.create( $( document.body ), 'isRemoveBusiness' ) ;
	sdbjs.parts.modalBox.update( 'isRemoveBusiness', htmlEncode( '提示' ), function( bodyObj ){
		sdbjs.parts.alertBox.create( bodyObj, 'isRemoveBusinessAlert' ) ;
		sdbjs.parts.alertBox.update( 'isRemoveBusinessAlert', htmlEncode( 'Warning：该操作是不可恢复操作，并且不会保留该业务数据.' ), 'warning' )
	}, function( footObj ){
		$( footObj ).css( 'text-align', 'right' ) ;
		sdbjs.parts.buttonBox.create( footObj, 'isRemoveBusinessOK' ) ;
		$( footObj ).append( '&nbsp;' ) ;
		sdbjs.parts.buttonBox.create( footObj, 'isRemoveBusinessClose' ) ;
		sdbjs.parts.buttonBox.update( 'isRemoveBusinessOK', htmlEncode( '确定' ), 'primary', null, '' ) ;
		sdbjs.parts.buttonBox.update( 'isRemoveBusinessClose', function( buttonObj ){
			$( buttonObj ).text( '关闭' ).attr( 'data-toggle', 'modalBox' ).attr( 'data-target', 'isRemoveBusiness' ) ;
		}, 'primary' ) ;
	} ) ;
	
	/* 添加业务的弹窗 */
	sdbjs.parts.modalBox.create( $( document.body ), 'addBusiness' ) ;
	sdbjs.parts.modalBox.update( 'addBusiness', htmlEncode( '添加业务' ), function( bodyObj ){
		sdbjs.parts.tableBox.create( bodyObj, 'addBusinessTable' ) ;
		sdbjs.parts.tableBox.update( 'addBusinessTable', 'loosen' ) ;
		
		sdbjs.parts.tableBox.addBody( 'addBusinessTable', [{ 'text': htmlEncode( '业务名：' ), 'width': 100 },
																			{ 'text': '<input class="form-control" type="text" id="businessName_a" value="myBusiness">' },
																			{ 'text': htmlEncode( '安装的业务名' ) } ] ) ;

		sdbjs.parts.tableBox.addBody( 'addBusinessTable', [{ 'text': htmlEncode( '业务类型：' ), 'width': 100 },
																			{ 'text': '<select class="form-control" id="businessType_a"></select>' },
																			{ 'text': '' } ] ) ;
	}, function( footObj ){
		sdbjs.parts.tableBox.create( footObj, 'addBusinessFootTable' ) ;
		sdbjs.parts.tableBox.addBody( 'addBusinessFootTable', [{ 'text': function( tdObj ){
																				sdbjs.parts.alertBox.create( tdObj, 'addBusinessFootAlert' ) ;
																				sdbjs.fun.setCSS( 'addBusinessFootAlert', { 'display': 'none', 'padding': '8px', 'text-align': 'left' } ) ;
																			} },
																			{ 'text': function( tdObj ){
																				sdbjs.parts.buttonBox.create( tdObj, 'addBusinessOK' ) ;
																				$( tdObj ).append( '&nbsp;' ) ;
																				sdbjs.parts.buttonBox.create( tdObj, 'addBusinessClose' ) ;
																				sdbjs.parts.buttonBox.update( 'addBusinessOK', htmlEncode( '确定' ), 'primary', null, 'addBusiness()' ) ;
																				sdbjs.parts.buttonBox.update( 'addBusinessClose', function( buttonObj ){
																					$( buttonObj ).text( '关闭' ).attr( 'data-toggle', 'modalBox' ).attr( 'data-target', 'addBusiness' ) ;
																				}, 'primary' ) ;
																			}, 'width': 120  } ] ) ;
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