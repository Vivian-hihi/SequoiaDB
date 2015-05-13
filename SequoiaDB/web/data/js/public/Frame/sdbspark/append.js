var sdbjs = {
	_layout: null,
    _left: null,
    _right: null,
    addMaster: function( address, masterPort, webPort ){
        var obj = this ;
        obj._right.getIframeObj().sdbjs.addMaster( address, masterPort, webPort ) ;
    },
	resize: function(){
		var obj = this ;
		obj._layout._onResize() ;
		//修正集合空间列表的高度
		var layoutLeftHeight = $( '.l-layout-left' ).height() ;
		var layoutLeftTitleHeight = $( '.l-layout-left > .l-layout-header' ).outerHeight() ;
		var layoutLeftContentHeight = layoutLeftHeight - layoutLeftTitleHeight ;
		$( '#operatePanel > iframe' ).height( layoutLeftContentHeight ) ;
		//修改数据库总览的高度
		var layoutCenterHeight = $( '.l-layout-center' ).height() ;
		var layoutCenterTitleHeight = $( '#tab > .l-tab-links' ).height() ;
		var layoutCenterContentHeight = layoutCenterHeight - layoutCenterTitleHeight ;
		$( '#masterList > iframe' ).height( layoutCenterContentHeight ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		//设置标题
		$( '#layout > [position="left"]' ).attr( 'title', '添加Master' ) ;
        $( '#layout > [position="center"]' ).attr( 'title', 'Master列表' ) ;
		//创建布局
		obj._layout = $( '#layout' ).ligerLayout( { leftWidth: 400, allowLeftCollapse: true, allowLeftResize: true, heightDiff: -5 } ) ;
		//加载控制面板
		obj._left = $( '#operatePanel' ).ligerSdbIframe( { url: M( 'sdbspark/operate.html' ) } ) ;
        //加载master表格
        obj._right = $( '#masterList' ).ligerSdbIframe( { url: M( 'sdbspark/list.html' ) } ) ;
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