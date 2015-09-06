var sdbjs = {
	_type: 'cs',
	_csName: null,
	_layout: null,
	resize: function(){
		var obj = this ;
		obj._layout._onResize() ;
		//修正集合空间列表的高度
		var layoutLeftHeight = $( '.l-layout-left' ).height() ;
		var layoutLeftTitleHeight = $( '.l-layout-left > .l-layout-header' ).outerHeight() ;
		var layoutLeftContentHeight = layoutLeftHeight - layoutLeftTitleHeight ;
		$( '#clList > iframe' ).height( layoutLeftContentHeight ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		//设置左框标题
		$( '#layout > [position="left"]' ).attr( 'title', '集合列表' ) ;
		//创建布局
		obj._layout = $( '#layout' ).ligerLayout( { leftWidth: 300, allowLeftCollapse: true, allowLeftResize: true, heightDiff: -60 } ) ;
		//加载集合列表
		var clList = $( '#clList' ).ligerSdbIframe( { url: M( 'sdbbusiness/collectionlist.html' ), callback: function(){
			clList.getIframeObj().sdbjs._csName = obj._csName ;
		} } ) ;
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