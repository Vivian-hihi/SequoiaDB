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
		var columns = [ { name: 'id',	display: '序号',	width: '10%', heightAlign:'left', align:'left' , editor:{ type:'text'} },
                       { name: 'user',	display: '用户',	width: '30%', heightAlign:'left', align:'left' , editor:{ type:'text'}},
							 { name: 'name', display: '名称', width: '60%', heightAlign:'left', align:'left' , editor:{ type:'text'}}];
		var griddata = [ { id: 'dbpath', name: '/opt/sequoiadb/database/standalone' },
							  { name: '11810', user:'afilesd' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', user:'ad' }, 
							  { id: 'logfilesz', name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name:'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', user:'ad' }, 
							  { id: 'numpagecleaners', name: '1', user:'ad' }, 
							  { name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', user:'ad' }, 
							  {  name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', }, 
							  { id: 'numpagecleaners',  user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', user:'ad' }, 
							  {  name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', }, 
							  { id: 'numpagecleaners',  user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', user:'ad' }, 
							  {  name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', }, 
							  { id: 'numpagecleaners',  user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { id: 'diaglevel', name: '3', user:'ad' }, 
							  { id: 'role', user:'ad' }, 
							  {  name: '64', user:'ad' }, 
							  { id: 'logfilenum', name: '20' }, 
							  { id: 'transactionon', name: 'false', user:'ad' }, 
							  { id: 'preferdinstance', name: 'A', }, 
							  { id: 'numpagecleaners',  user:'ad' }, 
							  { id: 'pagecleaninterval', name: '10000', user:'ad' }, 
							  { id: 'numpreload', name: '0' , user:'ad'}, 
							  { id: 'svcname', name: '11810', user:'ad' }, 
							  { name: '3', user:'ad' }, 
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
		
		obj._grid = $( '#maingrid1' ).ligerGrid( { 	columns: columns,enabledEdit:true,
												  	data: { Rows: griddata }, 
			 										pageSize:30,  
			 										height: height,
												  	checkbox:true,
												  	
			 										onEndChangePage: function(){
														obj.resize() ; 
													} } ) ;	
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