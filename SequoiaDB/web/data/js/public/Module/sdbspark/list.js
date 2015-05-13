var sdbjs = {
	_grid: null,
    _data: [],
	resize: function(){
		var obj = this ;
		var height = $( window ).height() - 50 ;
		obj._grid.resize( height ) ;
	},
    addMaster: function( address, masterPort, webPort ){
        var obj = this ;
        var height = $( window ).height() - 50 ;
        var newRow = { address: address,
                       masterPort: masterPort,
                       webPort: webPort } ;
        obj._grid.addRow( newRow ) ;
        obj._data.push( newRow ) ;
        obj._grid.resize( height ) ;
        obj._grid.reload() ;
    },
	init: function(){
		return ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
		var columns = [ { name: 'address',	display: 'IP/HostName',	width: '50%', heightAlign:'left', align:'left' },
                        { name: 'masterPort', display: 'Master Port', width: '25%', heightAlign:'left', align:'left' },
                        { name: 'webPort', display: 'Web Port', width: '25%', heightAlign:'left', align:'left' } ] ;
		
		obj._grid = $( '#list' ).ligerGrid( { columns: columns,
                                              data: { Rows: obj._data },
                                              pageSize:30,
                                              isScroll: true,
                                              height: 'auto',
                                              rownumbers: true,
                                              fixedCellHeight: false } ) ;
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