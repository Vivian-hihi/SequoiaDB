


function checkresolution()
{
	var max_width = -1 ;
	var now_width = -1 ;
	if ( window.screen )
	{
		max_width = window.screen.width ;
		now_width = document.documentElement.clientWidth ;
		if ( max_width < 1024 )
		{
			document.getElementById("header_tips_1").style.display = "none" ;
			document.getElementById("header_tips_2").style.display = "block" ;
			$('#Modal_tips').modal('toggle');
			return 0 ;
		}
		else if ( max_width < 1280 )
		{
			document.getElementById("header_tips_1").style.display = "block" ;
			document.getElementById("header_tips_2").style.display = "none" ;
			$('#Modal_tips').modal('toggle');
		}
		else
		{
			return 2 ;
		}
	}
	return 1 ;
}

function showconnectmanager()
{
	ajax2send('connect_list_r','post','index.php?p=connectlist&m=ajax_f','connectmodel=list');
	$('#Modal_connect').modal('toggle');
}

function convert2post( str )
{
	/*
	str = replaceAllEx( str, '%', '%25' ) ;
	str = replaceAllEx( str, '&', '%26' ) ;
	str = replaceAllEx( str, '+', '%2B' ) ;
	str = replaceAllEx( str, ' ', '%20' ) ;
	str = replaceAllEx( str, '/', '%2F' ) ;
	str = replaceAllEx( str, '?', '%3F' ) ;
	str = replaceAllEx( str, '#', '%23' ) ;
	str = replaceAllEx( str, '=', '%3D' ) ;*/
	str = encodeURIComponent( str ) ;
	return str ;
}

function replaceAllEx(text, oldChar, newChar)
{
   var index = text.indexOf(oldChar);
   
   var json = "";
   while(index!=-1)
   {
	  json  = json + text.substring(0,index)+newChar;
	  text  = text.substring(index+1,text.length);
	  index = text.indexOf(oldChar);
   }
   json = json + text;
   return json;
}

function showandhidetool( obj, type )
{
	var tool = document.getElementById(obj) ;
	if ( type )
	{
		tool.style.display = "block" ;
	}
	else
	{
		tool.style.display = "none" ;
	}
}

function array_is_exists( array, value )
{
	for ( var i = 0,k = array.length; i < k; ++i )
	{
		if ( array[ i ] === value )
		{
			return true ;
		}
	}
	return false ;
}

function toggletable( name1, name2 )
{
	var obj1 = document.getElementById(name1) ;
	var obj2 = document.getElementById(name2) ;
	if ( !obj1 || !obj2 )
	{
		return false ;
	}
	if ( obj1.innerHTML == "[ + ]" )
	{
		obj1.innerHTML = "[ - ]" ;
		obj2.style.display = "block" ;
	}
	else
	{
		obj1.innerHTML = "[ + ]" ;
		obj2.style.display = "none" ;
	}
}

function createdatalefttree( d, json )
{
	d.add ( 0, -1, json['name'], "", "", "", "images/tree/database.png", "images/tree/database.png" ) ;
	json = json['child'] ;
	d.add ( 1, 0, json['name'], "", "", "", "images/tree/file.png", "images/tree/file.png" ) ;
	json = json['child'] ;
	var num1 = 2 ;
	var num2 = 0 ;
	var num3 = 0 ;
	var num4 = 0 ;
	for ( var key in json )
	{
		num2 = num1 ;
		var temp = json[key] ;
		d.add ( num2, 1, temp["Name"], 'settempdata( "' + temp["Name"] + '", "" );', "", "", "images/tree/cs.png", "images/tree/cs.png" ) ;
		num1 = num1 + 1 ;
		d.add ( num1, num2, "属性", 'show_group_tool_1();settempdata( "' + temp["Name"] + '", "" );senddata( "' + temp["Name"] + '", "", "csnature" );', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
		num1 = num1 + 1 ;
		d.add ( num1, num2, "集合", 'settempdata( "' + temp["Name"] + '", "" );', "", "", "images/tree/file.png", "images/tree/file.png" ) ;
		num3 = num1 ;
		var temp2 = temp["Collection"] ;
		for ( var key2 in temp2 )
		{
			num1 = num1 + 1 ;
			var temp3 = temp2[key2] ;
			d.add ( num1, num3, temp3["Name"], 'settempdata( "' + temp["Name"] + '", "' + temp3["Name"] + '" );', "", "", "images/tree/cl.png", "images/tree/cl.png" ) ;
			num4 = num1 ;
			num1 = num1 + 1 ;
			d.add ( num1, num4, "属性", 'show_group_tool_2();settempdata( "' + temp["Name"] + '", "' + temp3["Name"] + '" );senddata( "' + temp["Name"] + '", "' + temp3["Name"] + '", "clnature" );', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num4, "数据", 'show_group_tool_3();settempdata( "' + temp["Name"] + '", "' + temp3["Name"] + '" );senddata( "' + temp["Name"] + '", "' + temp3["Name"] + '", "cldata" );', "", "", "images/tree/data.png", "images/tree/data.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num4, '索引', 'show_group_tool_4();settempdata( "' + temp["Name"] + '", "' + temp3["Name"] + '" );senddata( "' + temp["Name"] + '", "' + temp3["Name"] + '", "clindex" );', "", "", "images/tree/index.png", "images/tree/index.png" ) ;
		}
		num1 = num1 + 1 ;
	}
	return d ;
}

function creategrouplefttree( d, json )
{
	var errobj = null ;
	if ( json.ErrNodes )
	{
		errobj = json['ErrNodes'] ;
	}
	d.add ( 0, -1, json['name'], "", "", "", "images/tree/database.png", "images/tree/database.png" ) ;
	d.add ( 1, 0, json['name1'], 'changeChartModel2( "all", "", "", "" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
	d.add ( 2, 0, json['name2'], 'changeChartModel( "all", "", "", "" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
	json = json['child'] ;
	d.add ( 3, 0, json['name'], "", "", "", "images/tree/file.png", "images/tree/file.png" ) ;
	json = json['child'] ;
	var num1 = 4 ;
	var num2 = 0 ;
	var num3 = 0 ;
	for ( var key in json )
	{
		num2 = num1 ;
		var temp = json[key] ;
		var sumnodenum = 0 ;
		var errnodenum = 0 ;
		var temp2 = temp["Group"] ;
		for ( var key2 in temp["Group"] )
		{
			var temp3 = temp2[key2] ;
			var temp4 = temp3["Service"] ;
			var tempName = "" ;
			for ( var key4 in temp4 )
			{
				var temp5 = temp4[key4] ;
				if ( temp5["Type"] == 0 )
				{
					tempName = temp5["Name"] ;
					break ;
				}
			}
			if ( errobj != null )
			{
				for ( var key2 in errobj )
				{
					var tempstr = temp3["HostName"]+":"+tempName ;
					if ( tempstr == errobj[key2] )
					{
						iserrornode = true ;
						++errnodenum ;
						break ;
					}
				}
			}
			++sumnodenum ;
		}
		if ( sumnodenum > 0 && errnodenum == 0 )
		{
			d.add ( num2, 3, temp["GroupName"], '', "", "", "images/tree/groupok.png", "images/tree/groupok.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, "属性", 'senddata( "' + temp["GroupName"] + '", "", "", true )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
			if ( temp["GroupName"] != 'SYSCatalogGroup' )
			{
				num1 = num1 + 1 ;
				d.add ( num1, num2, '数据统计', 'changeChartModel2( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
				num1 = num1 + 1 ;
				d.add ( num1, num2, '性能监控', 'changeChartModel( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
			}
		}
		else if ( sumnodenum > 0 && errnodenum / sumnodenum < 0.5 )
		{
			d.add ( num2, 3, temp["GroupName"], '', "", "", "images/tree/groupwarning.png", "images/tree/groupwarning.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, "属性", 'senddata( "' + temp["GroupName"] + '", "", "", true )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
			if ( temp["GroupName"] != 'SYSCatalogGroup' )
			{
				num1 = num1 + 1 ;
				d.add ( num1, num2, '数据统计', 'changeChartModel2( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
				num1 = num1 + 1 ;
				d.add ( num1, num2, '性能监控', 'changeChartModel( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
			}
		}
		else if ( sumnodenum > 0 && errnodenum / sumnodenum < 1 )
		{
			d.add ( num2, 3, temp["GroupName"], '', "", "", "images/tree/groupdanger.png", "images/tree/groupdanger.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, "属性", 'senddata( "' + temp["GroupName"] + '", "", "", false )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
			if ( temp["GroupName"] != 'SYSCatalogGroup' )
			{
				num1 = num1 + 1 ;
				d.add ( num1, num2, '数据统计', 'changeChartModel2( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
				num1 = num1 + 1 ;
				d.add ( num1, num2, '性能监控', 'changeChartModel( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
			}
		}
		else
		{
			d.add ( num2, 3, temp["GroupName"], '', "", "", "images/tree/grouperror.png", "images/tree/grouperror.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, "属性", 'senddata( "' + temp["GroupName"] + '", "", "", false )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
		}
		num1 = num1 + 1 ;
		d.add ( num1, num2, "节点", '', "", "", "images/tree/file.png", "images/tree/file.png" ) ;
		num3 = num1 ;
		temp2 = temp["Group"] ;
		for ( var key2 in temp2 )
		{
			num1 = num1 + 1 ;
			var temp3 = temp2[key2] ;
			var temp4 = temp3["Service"] ;
			var tempName = "" ;
			for ( var key4 in temp4 )
			{
				var temp5 = temp4[key4] ;
				if ( temp5["Type"] == 0 )
				{
					tempName = temp5["Name"] ;
					break ;
				}
			}
			var num5 = num1 ;
			if ( errobj != null )
			{
				var iserrornode = false ;
				for ( var key2 in errobj )
				{
					var tempstr = temp3["HostName"]+":"+tempName ;
					if ( tempstr == errobj[key2] )
					{
						d.add ( num1, num3, temp3["HostName"]+":"+tempName, '', "", "", "images/tree/nodeerror.png", "images/tree/nodeerror.png" ) ;
						num1 = num1 + 1 ;
						d.add ( num1, num5, '属性', 'senddata( "' + temp["GroupName"] + '", "' + temp3["HostName"] + '", "' + tempName + '", true )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
						iserrornode = true ;
						break ;
					}
				}
				if ( !iserrornode )
				{
					d.add ( num1, num3, temp3["HostName"]+":"+tempName, '', "", "", "images/tree/nodeok.png", "images/tree/nodeok.png" ) ;
					num1 = num1 + 1 ;
					d.add ( num1, num5, '属性', 'senddata( "' + temp["GroupName"] + '", "' + temp3["HostName"] + '", "' + tempName + '", true )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
					if ( temp["GroupName"] != 'SYSCatalogGroup' )
					{
						num1 = num1 + 1 ;
						d.add ( num1, num5, '数据统计', 'changeChartModel2( "node", "", "' + temp3["HostName"] + '", "' + tempName + '" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
						num1 = num1 + 1 ;
						d.add ( num1, num5, '性能监控', 'changeChartModel( "node", "", "' + temp3["HostName"] + '", "' + tempName + '" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
					}
				}
			}
			else
			{
				d.add ( num1, num3, temp3["HostName"]+":"+tempName, '', "", "", "images/tree/nodeok.png", "images/tree/nodeok.png" ) ;
				num1 = num1 + 1 ;
				d.add ( num1, num5, '属性', 'senddata( "' + temp["GroupName"] + '", "' + temp3["HostName"] + '", "' + tempName + '", true )', "", "", "images/tree/attribute.png", "images/tree/attribute.png" ) ;
				if ( temp["GroupName"] != 'SYSCatalogGroup' )
				{
					num1 = num1 + 1 ;
					d.add ( num1, num5, '数据统计', 'changeChartModel2( "node", "", "' + temp3["HostName"] + '", "' + tempName + '" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
					num1 = num1 + 1 ;
					d.add ( num1, num5, '性能监控', 'changeChartModel( "node", "", "' + temp3["HostName"] + '", "' + tempName + '" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
				}
			}
		}
		num1 = num1 + 1 ;
	}
	return d ;
}

function createmonitorlefttree( d, json )
{
	d.add ( 0, -1, json['name'], "", "", "", "images/tree/database.png", "images/tree/database.png" ) ;
	json = json['child'] ;
	d.add ( 1, 0, json['name1'], 'changeChartModel2( "all", "", "", "" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
	d.add ( 2, 0, json['name2'], 'changeChartModel( "all", "", "", "" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
	if ( !json.name3 )
	{
		return d;
	}
	d.add ( 3, 0, json['name3'], "", "", "", "images/tree/file.png", "images/tree/file.png" ) ;
	json = json['child'] ;
	var num1 = 4 ;
	var num2 = 0 ;
	var num3 = 0 ;
	var num4 = 0 ;
	for ( var key in json )
	{
		var temp = json[key] ;
		if ( temp["GroupName"] != "SYSCatalogGroup" )
		{
			num2 = num1 ;
			d.add ( num2, 3, temp["GroupName"], 'senddata( "' + temp["GroupName"] + '", "" )', "", "", "images/tree/group.png", "images/tree/group.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, '数据统计', 'changeChartModel2( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, '性能监控', 'changeChartModel( "group", "' + temp["GroupName"] + '", "", "" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
			num1 = num1 + 1 ;
			d.add ( num1, num2, '节点', '', "", "", "images/tree/file.png", "images/tree/file.png" ) ;
			num3 = num1 ;
			var temp2 = temp["Group"] ;
			for ( var key2 in temp2 )
			{
				num1 = num1 + 1 ;
				var temp3 = temp2[key2] ;
				var temp4 = temp3["Service"] ;
				var tempName = "" ;
				for ( var key4 in temp4 )
				{
					var temp5 = temp4[key4] ;
					if ( temp5["Type"] == 0 )
					{
						tempName = temp5["Name"] ;
						break ;
					}
				}
				d.add ( num1, num3, temp3["HostName"]+":"+tempName, "", "", "", "images/tree/node.png", "images/tree/node.png" ) ;
				num4 = num1 ;
				num1 = num1 + 1 ;
				d.add ( num1, num4, '数据统计', 'changeChartModel2( "node", "", "' + temp3["HostName"] + '", "' + tempName + '" );', "", "", "images/tree/barchart.png", "images/tree/barchart.png" ) ;
				num1 = num1 + 1 ;
				d.add ( num1, num4, '性能监控', 'changeChartModel( "node", "", "' + temp3["HostName"] + '", "' + tempName + '" );', "", "", "images/tree/linechart.png", "images/tree/linechart.png" ) ;
			}
			num1 = num1 + 1 ;
		}
	}
	return d ;
}

function getleftlist( obj, name )
{
	var str = 'd = new dTree("d");'
	if ( name == "data" )
	{
		str += 'd = createdatalefttree ( d, record );'
	}
	else if ( name == "group" )
	{
		str += 'd = creategrouplefttree ( d, record );'
	}
	else if ( name == "monitor" )
	{
		str += 'd = createmonitorlefttree ( d, record );'
	}
	str += 'document.getElementById("pictree").innerHTML = d ;' ;
	ajax2send3 ( obj, "post", "index.php?p=" + name + "&m=ajax_l", "", true, str ) ;
}

function isArray(obj) {
    return (typeof(obj) =='object'); 
}

//数字字符串自动补零
function pad(num, n) {
    var len = num.toString().length;
    while(len < n) {
        num = "0" + num;
        len++;
    }
    return num;
}