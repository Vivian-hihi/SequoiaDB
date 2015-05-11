var sdbjs = {
	_layout: null,
	resize: function(){
		var obj = this ;
		obj._layout._onResize() ;
		//修正集合空间列表的高度
		var layoutLeftHeight = $( '.l-layout-left' ).height() ;
		var layoutLeftTitleHeight = $( '.l-layout-left > .l-layout-header' ).outerHeight() ;
		var layoutLeftContentHeight = layoutLeftHeight - layoutLeftTitleHeight ;
		$( '#csList > iframe' ).height( layoutLeftContentHeight ) ;
		//修改数据库总览的高度
		var layoutCenterHeight = $( '.l-layout-center' ).height() ;
		var layoutCenterTitleHeight = $( '#tab > .l-tab-links' ).height() ;
		var layoutCenterContentHeight = layoutCenterHeight - layoutCenterTitleHeight ;
		$( '#tab .l-tab-content-item' ).height( layoutCenterContentHeight ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		//设置左框标题
		$( '#layout > [position="left"]' ).attr( 'title', '集合空间列表' ) ;
		//创建布局
		obj._layout = $( '#layout' ).ligerLayout( { leftWidth: 300, allowLeftCollapse: true, allowLeftResize: true, heightDiff: -5 } ) ;
		//加载集合空间列表
		var csList = $( '#csList' ).ligerSdbIframe( { url: M( 'sdbbusiness/database/collectionspacelist.html' ) } ) ;
		//创建右框里面的分页
		obj._tab = $( '#tab' ).ligerTab(  { 
            onAfterSelectTabItem: function( targetID )
            {
			var sdbjsObj = window.frames[ targetID ].sdbjs ;
			if( typeof( sdbjsObj ) != 'undefined' )
			{
				sdbjsObj.resize() ;
			}
		} } ) ;
		//加载分页内容
		obj._tab.addTabItem( { tabid: 'overview', text: '数据库总览', url: M( 'sdbbusiness/database/overview/overview.html' ), showClose: false } ) ;
		obj._tab.addTabItem( { tabid: 'status', text: '状态', url: M( 'sdbbusiness/database/status/status.html' ), showClose: false } ) ;
		obj._tab.addTabItem( { tabid: 'config', text: '配置', url: M( 'sdbbusiness/database/configure/configure.html' ), showClose: false } ) ;
		obj._tab.selectTabItem( 'overview' ) ;
		obj.resize() ;
	},
	synData: function(){
		return ;
	}
} ;

$(document).ready( function(){
	if( sdbjs.check() ){
		sdbjs.render() ;
		sdbjs.synData() ;
	}
} ) ;

$( window ).resize( function(){
	sdbjs.resize() ;
} ) ;