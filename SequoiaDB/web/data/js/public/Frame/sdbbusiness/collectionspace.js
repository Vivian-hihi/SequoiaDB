var sdbjs = {
	_csName: null,
	_layout: null,
	_tab: null,
	addCLTab: function( tabId, tabTitle, url, showClose, options ){
		var obj = this ;
		obj._tab.addTabItem( { tabid: tabId, text: tabTitle, url: F( url ), showClose: showClose } ) ;
        obj.resize() ;
	},
	resize: function(){
		var obj = this ;
		obj._layout._onResize() ;
		//修正集合列表的高度
		var layoutLeftHeight = $( '.l-layout-left' ).height() ;
		var layoutLeftTitleHeight = $( '.l-layout-left > .l-layout-header' ).outerHeight() ;
		var layoutLeftContentHeight = layoutLeftHeight - layoutLeftTitleHeight ;
		$( '#clList > iframe' ).height( layoutLeftContentHeight ) ;
		//修改集合信息的高度
		var layoutCenterHeight = $( '.l-layout-center' ).height() ;
		var layoutCenterTitleHeight = $( '#tab > .l-tab-links' ).height() ;
		var layoutCenterContentHeight = layoutCenterHeight - layoutCenterTitleHeight ;
		$( '#tab .l-tab-content-item' ).height( layoutCenterContentHeight ) ;
	},
	init: function(){
		var obj = this ;
		obj._csName = getUrlParam( 'csName' ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		//设置左框标题
		$( '#layout > [position="left"]' ).attr( 'title', '集合列表' ) ;
		//创建布局
		obj._layout = $( '#layout' ).ligerLayout( { leftWidth: 300, allowLeftCollapse: true, allowLeftResize: true, heightDiff: -15 } ) ;
		//加载集合列表
		var param = setUrlParam( { 'csName': obj._csName } ) ;
		var clList = $( '#clList' ).ligerSdbIframe( { url: M( 'sdbbusiness/collectionspace/collectionlist.html' ) + '?' + param, callback: function(){
			clList.getIframeObj().sdbjs._csName = obj._csName ;
		} } ) ;
		//创建右框里面的分页
		obj._tab = $( '#tab' ).ligerTab(  { onAfterSelectTabItem: function( targetID ){
			var sdbjsObj = window.frames[ targetID ].sdbjs ;
			if( typeof( sdbjsObj ) != 'undefined' )
			{
				sdbjsObj.resize() ;
			}
		} } ) ;
		//加载分页内容
		obj._tab.addTabItem( { tabid: 'overview', text: '集合空间信息', url: M( 'sdbbusiness/collectionspace/collectionspaceinfo.html' ), showClose: false } ) ;
		obj._tab.selectTabItem( 'overview' ) ;
		obj.resize() ;
	},
	synData: function(){
		return ;
	}
} ;

$(document).ready( function(){
	sdbjs.init() ;
	if( sdbjs.check() ){
		sdbjs.render() ;
		sdbjs.synData() ;
	}
} ) ;

$( window ).resize( function(){
	sdbjs.resize() ;
} ) ;