var sdbjs = {
	_layout: null,
	resize: function(){
		var obj = this ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		$( '#databaseinfo' ).ligerPanel( { title: '数据库服务器', url: M( 'sdbbusiness/database/overview/databaseinfo.html' ), showToggle: false, width: 'auto', height: 262 } ) ;
		$( '#nodeinfo' ).ligerPanel( { title: '节点信息', url: M( 'sdbbusiness/database/overview/nodeinfo.html' ),showToggle: false, width: 'auto', height: 130 } ) ;
		$( '#document' ).ligerPanel( { title: '文档中心', url: M( 'sdbbusiness/database/overview/document.html' ),showToggle: false, width: 'auto', height: 130 } ) ;
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