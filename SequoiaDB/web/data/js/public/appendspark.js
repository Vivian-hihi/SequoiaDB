var sdbjs = {
	_tab: null,
    appendSpark: function(){
        var obj = this ;
        var businessName = getUrlParam( 'businessName' ) ;
        var businessType = getUrlParam( 'businessType' ) ;
        var clusterName  = getUrlParam( 'clusterName' ) ;
        var SdbSessionID = getUrlParam( 'SdbSessionID' ) ;
        var BusinessInfo = window.frames[ 'spark' ].sdbjs.getMasterList() ;
        var data = { 'cmd': 'discovery business', configinfo: JSON.stringify( { 'BusinessType': businessType,
                                                                                'BusinessName': businessName,
                                                                                'ClusterName' : clusterName,
                                                                                'BusinessInfo': BusinessInfo } ) } ;
        $.ajax( { 'type': 'POST', 'async': true, 'url': '/', 'data': data, 'success': function( text, textStatus, jqXHR ){
            var json = parseJsons( text ) ;
            if( json[0]['errno'] == 0 )
            {
                $.ligerDialog.success( '添加完成','提示', function(){
                    window.location = '../index.html' ;
                } ) ;
            }
            else
            {
                $.ligerDialog.error( json[0]['description'] ) ;
            }
        }, 'error': function( XMLHttpRequest, textStatus, errorThrown ) {
            $.ligerDialog.error( '网络错误，请刷新网页，重新尝试', '添加Spark错误' ) ;
        }, 'complete': function ( XMLHttpRequest, textStatus ) {
            return;
        }, 'beforeSend': function( XMLHttpRequest ){
            XMLHttpRequest.setRequestHeader( 'SdbSessionID', SdbSessionID ) ;
        } } ) ;
    },
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
		obj._tab.addTabItem( { tabid: 'spark', text: '发现Spark', url: F( 'sdbspark/append.html' ), showClose: false } ) ;
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

function appendSpark()
{
    sdbjs.appendSpark() ;
}