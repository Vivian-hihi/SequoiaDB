function addMaster()
{
    var address = $( '#address' ).val() ;
    var masterPort = $( '#mport' ).val() ;
    var webPort = $( '#wport' ).val() ;
    $( '#address' ).val( '' ) ;
    parent.sdbjs.addMaster( address, masterPort, webPort ) ;
}
        
var sdbjs = {
	resize: function(){
		return ;
	},
	check: function(){
		return true ;
	},
	render: function(){
		var obj = this ;
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