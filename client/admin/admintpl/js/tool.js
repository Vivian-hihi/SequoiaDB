function showtool( obj1, obj2 )
{
	var b1 = document.getElementById(obj1) ;
	var b2 = document.getElementById(obj2) ;
	
	if ( b1.innerHTML == "隐藏工具栏" )
	{
		b2.style.display = "none" ;
		b1.innerHTML = "显示工具栏" ;
		var strHeight = (window.screen.availHeight-200) + "px" ;
		document.getElementById("left_list").style.height = strHeight ;
		document.getElementById("right_context").style.height = strHeight ;
		document.getElementById("index").style.marginTop = "13px" ;
	}
	else
	{
		b2.style.display = "block" ;
		b1.innerHTML = "隐藏工具栏" ;
		var strHeight = (window.screen.availHeight-270) + "px" ;
		document.getElementById("left_list").style.height = strHeight ;
		document.getElementById("right_context").style.height = strHeight ;
		document.getElementById("index").style.marginTop = "0px" ;
	}
}

function loadconnection( add, represent, user, pwd )
{
	document.getElementById("connect_address").value    = add ;
	document.getElementById("connect_represent").value  = represent ;
	document.getElementById("connect_user").value       = user ;
	document.getElementById("connect_password").value   = pwd ;
}

function deleteconnect( address )
{
	ajax2send("connect_return","post","index.php?p=connectlist&m=ajax_f","connectmodel=delete&connectaddress=" + address, false ) ;
	ajax2send('connect_list_r','post','index.php?p=connectlist&m=ajax_f','connectmodel=list');
}

function updateconnect()
{
	var kep  = document.getElementById("keeppassword").checked ;
	var add  = document.getElementById("connect_address").value ;
	var user = document.getElementById("connect_user").value ;
	var pwd  = document.getElementById("connect_password").value ;
	var rep  = document.getElementById("connect_represent").value ;
	if ( kep == false )
	{
		pwd = "" ;
	}
	if ( add == "" )
	{
		document.getElementById("connect_return").innerHTML = '<div class="alert alert-danger alert-dismissable"><button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>请输入连接地址</div>' ;
	}
	else
	{
		ajax2send("connect_return","post","index.php?p=connectlist&m=ajax_f","connectmodel=update&connectaddress=" + add + "&connectuser=" + user + "&connectpwd=" + pwd + "&represent=" + rep, false ) ;
		ajax2send('connect_list_r','post','index.php?p=connectlist&m=ajax_f','connectmodel=list');
	}
}

function testconnect()
{
	var add  = document.getElementById("connect_address").value ;
	var user = document.getElementById("connect_user").value ;
	var pwd  = document.getElementById("connect_password").value ;
	if ( add == "" )
	{
		document.getElementById("connect_return").innerHTML = '<div class="alert alert-danger alert-dismissable"><button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>请输入连接地址</div>' ;
	}
	else
	{
		ajax2send("connect_return","post","index.php?p=connectlist&m=ajax_f","connectmodel=testlink&connectaddress=" + add + "&connectuser=" + user + "&connectpwd=" + pwd ) ;
	}
}

function sessionconnect()
{
	var add  = document.getElementById("connect_address").value ;
	var user = document.getElementById("connect_user").value ;
	var pwd  = document.getElementById("connect_password").value ;
	if ( add == "" )
	{
		document.getElementById("connect_return").innerHTML = '<div class="alert alert-danger alert-dismissable"><button type="button" class="close" data-dismiss="alert" aria-hidden="true">&times;</button>请输入连接地址</div>' ;
	}
	else
	{
		ajax2send("connect_return","post","index.php?p=connectlist&m=ajax_f","connectmodel=connect&connectaddress=" + add + "&connectuser=" + user + "&connectpwd=" + pwd, false ) ;
		tool_refresh_all ( location ) ;
	}
}

function sessionconnect2( add, user, pwd )
{
	ajax2send("connect_return","post","index.php?p=connectlist&m=ajax_f","connectmodel=connect&connectaddress=" + add + "&connectuser=" + user + "&connectpwd=" + pwd, false ) ;
	tool_refresh_all ( location ) ;
}

function convertChart ( data, step )
{
	var ary = [] ;
	if ( data.length == 0 )
	{
		for ( var i = 0 ; i <= 60 ; i+=step )
		{
			var temp = [i,0] ;
			data.push( temp ) ;
		}
	}
	for ( var i = 1 ; i < data.length ; ++i )
	{
		var temp = data[i] ;
		temp[0] -= step ;
		ary.push( temp ) ;
	}
	return ary ;
}
function tool_refresh_all ( lo )
{
	lo.reload() ;
}
function left_list_open_all ( b )
{
	b.openAll() ;
}

function left_list_close_all ( b )
{
	b.closeAll() ;
}
function sqlexecute ( obj1, obj2 )
{
	var str = document.getElementById(obj1).value ;
	ajax2send4 ( "context", "post", "index.php?p=sql&m=ajax_r", "sql=" + str ) ;
	$('#Modal_sql').modal('toggle');
}