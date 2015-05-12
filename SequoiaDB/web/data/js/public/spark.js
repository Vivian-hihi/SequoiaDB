function openAboutus(){
    $.ligerDialog.open( { height: 240,
                          width: 630,
                          title: '关于SAC',
                          url: M('public/aboutus.html'),
                          showMax: false,
                          showToggle: false,
                          showMin: false,
                          isResize: false,
                          slide: false,
                          show: false,
                          cls: 'ext-modal' } ) ;
}

var sdbjs = {
	_tab: null,
	addCSTab: function( tabId, tabTitle, url, showClose, options ){
		var obj = this ;
		obj._tab.addTabItem( { tabid: tabId, text: tabTitle, url: F( url ), showClose: showClose } ) ;
	},
	resize: function(){
		var obj = this ;
		var height = $( window ).height() ;
		var menuHeight = $( '#menu' ).height() ;
		var footHeight = $( '#foot' ).height() ;
		obj._tab.setHeight( height - menuHeight - footHeight - 3 ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		$( '#menu' ).ligerSdbMenuBar( { items: [{ text: '首页', click: function(){ window.location = '../index.html' ; } },
                                                { text: '帮助', menu: { items: [ { text: '关于SAC系统', click: function(){ openAboutus(); } } ] } } ] } ) ;
		obj._tab = $( '#tab' ).ligerTab( { onAfterSelectTabItem: function( targetID ){
			var sdbjsObj = window.frames[ targetID ].sdbjs ;
			if( typeof( sdbjsObj ) != 'undefined' )
			{
				sdbjsObj.resize() ;
			}
		} } ) ;
		obj._tab.addTabItem( { tabid: 'database', text: 'Spark', url: 'http://192.168.20.56:8080/', showClose: false } ) ;
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