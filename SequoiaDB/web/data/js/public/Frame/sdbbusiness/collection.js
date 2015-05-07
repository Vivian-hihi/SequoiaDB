var sdbjs = {
	_type: 'cs',
	_csName: null,
	_layout: null,
	resize: function(){
		//获取总高度
        var height = $( window ).height() - 20 ;
        $( '#tab' ).height( height ) ;
        //修正集合数据网格的高度
		$( '#tab > iframe' ).height( height ) ;
	},
	check: function(){
		return true ;
	},
	resize: function(){
		//获取总高度
        var height = $( window ).height() - 55 ;
        $( '.l-tab-content' ).height( height ) ;
        //修正集合数据网格的高度
		$( '.l-tab-content > iframe' ).height( height ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;

		obj._tab = $( '#tab' ).ligerTab(  { 
            onAfterSelectTabItem: function( targetID )
            {
			var sdbjsObj = window.frames[ targetID ].sdbjs ;
			if( typeof( sdbjsObj ) != 'undefined' )
			{
				sdbjsObj.resize() ;
			}
		} } ) ;
		obj._tab.addTabItem( { tabid: 'browse', text: '浏览', url: M( 'sdbbusiness/structure/browse.html' ), showClose: false } ) ;
		obj._tab.addTabItem( { tabid: 'insert', text: '插入', url: M( 'sdbbusiness/structure/export.html' ), showClose: false } ) ;
		obj._tab.addTabItem( { tabid: 'export', text: '导出', url: M( 'sdbbusiness/structure/insert.html' ), showClose: false } ) ;
		obj._tab.selectTabItem( 'browse' ) ;
		
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