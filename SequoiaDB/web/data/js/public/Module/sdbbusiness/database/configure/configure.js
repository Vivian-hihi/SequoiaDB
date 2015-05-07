var sdbjs = {
	_layout: null,
	_grid: null,
	resize: function(){
		var obj = this ;
		var height = $( window ).height() - 24 ;
		obj._grid.resize( height ) ;
	},
	init: function(){
		return ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		var height = $( window ).height() ;
		var columns = [ { name: 'id',	display: '序号',	width: '30%', heightAlign:'left', align:'left' },
							 { name: 'name', display: '名称', width: '70%', heightAlign:'left', align:'left' }]
		var griddata = [ { id: 'dbpath', name: '/opt/sequoiadb/database/standalone' },
							  { id: 'svcname', name: '11810' }, 
							  { id: 'diaglevel', name: '3' }, 
							  { id: 'role', name: 'standalone' }, 
							  { id: 'logfilesz', name: '64' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false' }, 
							  { id: 'preferdinstance', name: 'A' }, 
							  { id: 'numpagecleaners', name: '1' }, 
							  { id: 'pagecleaninterval', name: '10000' }, 
							  { id: 'numpreload', name: '0' }, 
							  { id: 'maxprefpool', name: '200' }];
		
		obj._grid = $( '#maingrid' ).ligerGrid( { columns: columns,
															   data: { Rows: griddata }, usePager:false, isScroll: true, height: height } ) ;
		obj.resize() ;
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