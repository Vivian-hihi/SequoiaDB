var sdbjs = {
	addCS: function( tabId, tabTitle, url, showClose, param, options ){
		url = url + '?' + setUrlParam( param ) ;
		top.sdbjs.addCSTab( tabId, tabTitle, url, showClose, options ) ;
	},
	resize: function(){
		return;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		var items = [] ;
		for( var i = 1; i <= 50; ++i )
		{
			items.push( { text: 'foo_' + i, click: (function( num ){
				return function(){
					var options = { type: 'cs', name: 'foo_' + num } ;
					var param = { csName: 'foo_' + num } ;
					obj.addCS( 'foo_' + num, 'foo_' + num, 'sdbbusiness/collectionspace.html', true, param, options ) ;
				}
			} )( i ), img: O( 'images/db/cs.png' ) } ) ;
		}
		$( '#csList' ).ligerSdbNorList( { items: items } ) ;
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