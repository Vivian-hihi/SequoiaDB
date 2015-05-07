var sdbjs = {
	_csName: null,
	addCLTab: function( tabId, tabTitle, url, showClose, param, options ){
		url = url + '?' + param ;
		parent.sdbjs.addCLTab( tabId, tabTitle, url, showClose, options ) ;
	},
	resize: function(){
		return;
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
		var items = [] ;
		for( var i = 1; i <= 50; ++i )
		{
			items.push( { text: 'bar_' + i, click: (function( num ){
				return function(){
					var options = { type: 'cl', name: 'bar_' + num } ;
					var param = setUrlParam( { 'csName': obj._csName, 'clName': 'bar_' + i } ) ;
					obj.addCLTab( 'bar_' + num, 'bar_' + num, 'sdbbusiness/collection.html', true, param, options ) ;
				}
			} )( i ), img: O( 'images/db/cl.png' ) } ) ;
		}
		$( '#clList' ).ligerSdbNorList( { items: items } ) ;
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