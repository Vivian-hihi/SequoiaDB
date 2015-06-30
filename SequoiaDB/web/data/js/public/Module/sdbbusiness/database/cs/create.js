var sdbjs = {
	_layout: null,
	resize: function(){
		var obj = this ;
		var height = $( window ).height() - 24 ;
		obj._grid.resize( height ) ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		var columns = [ { name: 'name', display: '集合空间',	width: '40%', heightAlign:'left', align:'left' },
                        { name: 'clNum', display: '集合数', width: '15%', heightAlign:'left', align:'left' },
                        { name: 'recordNum', display: '记录数', width: '15%', heightAlign:'left', align:'left' },
                        { name: 'dataSize', display: '数据大小', width: '15%', heightAlign:'left', align:'left' },
                        { name: 'indexSize', display: '索引大小', width: '15%', heightAlign:'left', align:'left' } ] ;
		var griddata = [ { name: 'foo_1', clNum: 5, recordNum: 10000, dataSize: '5MB', indexSize: '2MB' },
                         { name: 'foo_2', clNum: 5, recordNum: 10000, dataSize: '5MB', indexSize: '2MB' },
                         { name: 'foo_3', clNum: 5, recordNum: 10000, dataSize: '5MB', indexSize: '2MB' },
                         { name: 'foo_4', clNum: 5, recordNum: 10000, dataSize: '5MB', indexSize: '2MB' },
                         { name: 'foo_5', clNum: 5, recordNum: 10000, dataSize: '5MB', indexSize: '2MB' },
                         { name: 'foo_6', clNum: 5, recordNum: 10000, dataSize: '5MB', indexSize: '2MB' } ] ;
		
		obj._grid = $( '#csgrid' ).ligerGrid( { columns: columns,
                                                data: { Rows: griddata },
                                                pageSize:30,
                                                usePager: true,
                                                isScroll: true,
                                                height: 'auto',
                                                checkbox: true,
                                                toolbar: {
                                                    items: [
                                                        { text: '创建集合空间', click: function(){}, icon: 'add' },
                                                        { text: '删除集合空间', click: function(){}, icon: 'delete' }
                                                    ]
                                                },
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
	if( sdbjs.check() ){
		sdbjs.render() ;
		sdbjs.synData() ;
	}
} ) ;

$( window ).resize( function(){
	sdbjs.resize() ;
} ) ;