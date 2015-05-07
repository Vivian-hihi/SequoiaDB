var sdbjs = {
	_layout: null,
	_grid: null,
	resize: function(){
		var obj = this ;
		var height = $( window ).height() - 14 ;
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
		var columns = [ { name: 'id',	display: '序号',	width: '20%', heightAlign:'left', align:'left' },
                       { name: 'user',	display: '用户',	width: '20%', heightAlign:'left', align:'left' },
							 { name: 'name', display: '名称', width: '60%', heightAlign:'left', align:'left' }]
		var griddata = [ { id: 'dbpath', name: '/opt/sequoiadb/database/standalone' },
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', name: 'standalone', user:'ad' }, 
							  { id: 'logfilesz', name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', user:'ad' }, 
							  { id: 'numpagecleaners', name: '1', user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', name: 'standalone', user:'ad' }, 
							  { id: 'logfilesz', name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', user:'ad' }, 
							  { id: 'numpagecleaners', name: '1', user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', name: 'standalone', user:'ad' }, 
							  { id: 'logfilesz', name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', user:'ad' }, 
							  { id: 'numpagecleaners', name: '1', user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', name: 'standalone', user:'ad' }, 
							  { id: 'logfilesz', name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', user:'ad' }, 
							  { id: 'numpagecleaners', name: '1', user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'maxprefpool', name: '200', user:'ad' }];
		
		obj._grid = $( '#maingrid' ).ligerGrid( { columns: columns,
                                                  data: { Rows: griddata },
                                                  pageSize:30,
                                                  isScroll: true,
                                                  height: height } ) ;
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