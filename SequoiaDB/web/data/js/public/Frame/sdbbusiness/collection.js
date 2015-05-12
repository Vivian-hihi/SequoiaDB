var sdbjs = {
	_type: 'cs',
	_csName: null,
	_layout: null,
	resize: function(){
		//获取总高度
        var height = $( window ).height() -50 ;
        $( '#toolTab' ).height( height ) ;
        //修正集合数据网格的高度
		$( '#toolTab > iframe' ).height( height );
	},
	check: function(){
		return true ;
	},
	resize: function(){
		//获取总高度
        var height = $( window ).height() - 55 ;
        $( '.l-tab-content' ).height( height ) ;
        //修正集合数据网格的高度
		$( '.l-tab-content > iframe' ).height( height ) - 20 ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;

		obj._tab = $( '#toolTab' ).ligerTab(  { 
            onAfterSelectTabItem: function( targetID )
            {
			var sdbjsObj = window.frames[ targetID ].sdbjs ;
			if( typeof( sdbjsObj ) != 'undefined' )
			{	
				sdbjsObj.resize() ;
			}
		} } ) ;
		obj._tab.addTabItem( { tabid: 'browse', icon:I( 'img/b_browse.png' ), text: '浏览', url: M( 'sdbbusiness/structure/browse.html' ), showClose: false } ) ;
		
		obj._tab.addTabItem( { tabid: 'insert', icon:I( 'img/b_search.png' ), text: '搜索', url: M( 'sdbbusiness/structure/export.html' ), showClose: false } ) ;
		
		obj._tab.addTabItem( { tabid: 'insert4', icon:I( 'img/b_insrow.png' ), text: '插入', url: M( 'sdbbusiness/structure/export.html' ), showClose: false } ) ;
		
		obj._tab.addTabItem( { tabid: 'insert2', icon:I( 'img/b_import.png' ), text: '导入', url: M( 'sdbbusiness/structure/browse.html' ), showClose: false } ) ;	
		
		obj._tab.addTabItem( { tabid: 'export1', icon:I( 'img/b_export.png' ), text: '导出', url: M( 'sdbbusiness/structure/export.html' ), showClose: false } ) ;
		
		obj._tab.addTabItem( { tabid: 'export2', icon:I( 'img/b_tblops.png' ), text: '操作', url: M( 'sdbbusiness/structure/browse.html' ), showClose: false } ) ;
		
		obj._tab.addTabItem( { tabid: 'insert3', icon: I( 'img/b_triggers.png' ), text: '触发器', url: M( 'sdbbusiness/structure/export.html' ), showClose: false } ) ;
		
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