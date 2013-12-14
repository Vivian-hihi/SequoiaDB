function ajax2send( obj, style, url, context, ajax, functions )
{
	if ( ajax == undefined )
	{
		ajax = true ;
	}
	if ( functions == undefined )
	{
		functions = "" ;
	}
	document.getElementById(obj).innerHTML = "<img src=\"images/loading.gif\" /> 载入中..." ;
	
	var xmlhttp;
	try
	{
		xmlhttp = new XMLHttpRequest();
	}
	catch (e)
	{
		try
		{
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e)
		{
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
	}

	xmlhttp.onreadystatechange = function()
	{
		if (4 == xmlhttp.readyState)
		{
			if (200 == xmlhttp.status)
			{
				var Bodys = xmlhttp.responseText;
				document.getElementById(obj).innerHTML = Bodys ;
				if ( functions )
				{
					eval('(' + functions + ')');
				}
			}
		}
	}

	xmlhttp.open( style, url, ajax ) ;
	xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xmlhttp.send( context ) ;
}

function ajax2send2( style, url, context )
{
	var record ;
	var xmlhttp;
	try
	{
		xmlhttp = new XMLHttpRequest();
	}
	catch (e)
	{
		try
		{
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e)
		{
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
	}

	xmlhttp.onreadystatechange = function()
	{
		if (4 == xmlhttp.readyState)
		{
			if (200 == xmlhttp.status)
			{
				record = xmlhttp.responseText;
				if ( record )
				{
					record = eval('(' + record + ')');
				}
			}
		}
	}

	xmlhttp.open( style, url, false ) ;
	xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xmlhttp.send( context ) ;
	return record ;
}

function ajax2send3( obj, style, url, context, ajax, functions )
{
	var record ;
	var xmlhttp;
	if ( ajax == undefined )
	{
		ajax = true ;
	}
	document.getElementById(obj).innerHTML = "<img src=\"images/loading.gif\" /> 载入中..." ;
	try
	{
		xmlhttp = new XMLHttpRequest();
	}
	catch (e)
	{
		try
		{
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e)
		{
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
	}

	xmlhttp.onreadystatechange = function()
	{
		if (4 == xmlhttp.readyState)
		{
			if (200 == xmlhttp.status)
			{
				record = xmlhttp.responseText;
				if ( record )
				{
					try
					{
						record = eval('(' + record + ')');
						eval( functions );
					}
					catch(e)
					{
						document.getElementById(obj).innerHTML = "" ;
					}
				}
				else
				{
					document.getElementById(obj).innerHTML = "" ;
				}
			}
		}
	}

	xmlhttp.open( style, url, ajax ) ;
	xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xmlhttp.send( context ) ;
	return record ;
}

function ajax2send4( obj, style, url, context, ajax, functions )
{
	if ( ajax == undefined )
	{
		ajax = true ;
	}
	if ( functions == undefined )
	{
		functions = "" ;
	}
	//document.getElementById(obj).innerHTML = "<img src=\"images/loading.gif\" /> 载入中..." ;
	
	var xmlhttp;
	try
	{
		xmlhttp = new XMLHttpRequest();
	}
	catch (e)
	{
		try
		{
			xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch (e)
		{
			xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
		}
	}

	xmlhttp.onreadystatechange = function()
	{
		if (4 == xmlhttp.readyState)
		{
			if (200 == xmlhttp.status)
			{
				var Bodys = xmlhttp.responseText;
				Bodys = Bodys.replace( new RegExp( "\r\n", "gm" ), "\\n" ) ;
				Bodys = Bodys.replace( new RegExp( "\r", "gm" ), "" ) ;
				Bodys = Bodys.replace( new RegExp( "\n", "gm" ), "\\n" ) ;
				if ( Bodys )
				{
					Bodys = eval('(' + Bodys + ')');
				}
				else
				{
					Bodys = eval( '( { } )' ) ;
				}
				
				if ( Bodys["context"] != "" )
				{
					document.getElementById(obj).innerHTML = Bodys["context"] ;
				}
				if ( Bodys["respond"] != "" )
				{
					var temp ;
					if ( Bodys["rc"] == 0 )
					{
						temp = '<div>' + Bodys["respond"] + "</div>" ;
					}
					else
					{
						temp = '<div style="color:#F00;">' + Bodys["respond"] + "</div>" ;
					}
					document.getElementById("context_respond").innerHTML += temp ;
					document.getElementById("context_respond").scrollTop = document.getElementById("context_respond").scrollHeight ;
				}
				if ( functions )
				{
					eval('(' + functions + ')');
				}
			}
		}
	}

	xmlhttp.open( style, url, ajax ) ;
	xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xmlhttp.send( context ) ;
}