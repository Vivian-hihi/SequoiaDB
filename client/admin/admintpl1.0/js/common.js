window.onload = function()
{
	getResolution() ;
}

function draw1(container,data1,data2,data3,data4,data5,data6,data7) {
	var placeholder = $(container);
	//sendMsg(url, data, draw);

	var subData = [];
	var metaDataInsert = [];
	var metaDataDelete = [];
	var metaDataUpdate = [];
	var metaDataInsert_cp = [];
	var metaDataDelete_cp = [];
	var metaDataUpdate_cp = [];
	var metaDataQuery = [];
	
	var insert = [0.5, data1];
	metaDataInsert.push(insert);
	var Delete = [2.5, data2];
	metaDataDelete.push(Delete);
	var update = [4.5, data3];
	metaDataUpdate.push(update);
	var insert_cp = [6.5, data4];
	metaDataInsert_cp.push(insert_cp);
	var Delete_cp = [8.5, data5];
	metaDataDelete_cp.push(Delete_cp);
	var update_cp = [10.5, data6];
	metaDataUpdate_cp.push(update_cp);
	var query = [12.5, data7];
	metaDataQuery.push(query);
	var json1 = {
	"label": "insert",
	"data": metaDataInsert,
	"color": "green"
  	};
  	var json2 = {
  		"label": "delete",
  		"data": metaDataDelete,
  		"color": "blue"
  	};
	var json3 = {
	"label": "update",
	"data": metaDataUpdate,
	"color": "yellow"
  	};
	var json4 = {
	"label": "insert(copy)",
	"data": metaDataInsert_cp,
	"color": "green"
  	};
  	var json5 = {
  		"label": "delete(copy)",
  		"data": metaDataDelete_cp,
  		"color": "blue"
  	};
	var json6 = {
	"label": "update(copy)",
	"data": metaDataUpdate_cp,
	"color": "yellow"
  	};
  	var json7 = {
  		"label": "query",
  		"data": metaDataQuery,
  		"color": "red"
  	};
		
		var options = {
  		bars : { show : true, horizontal : false, shadowSize : 0, barWidth : 1},
			grid: { hoverable: true, clickable: true },
			yaxis : { min : 0, autoscaleMargin : 0.2},
			xaxis : { min : 0, max : 17, autoscaleMargin : 1}
  	};
  	var plot = $.plot(placeholder, [json1,json2,json3,json4,json5,json6,json7], options);

  	var previousPoint = null;
  	// bind function to action(mouse over)
  	placeholder.bind("plothover", function (event, pos, item) {
  		if (item) {
         if (previousPoint != item.dataIndex) {
             previousPoint = item.dataIndex;
             $("#tooltip").remove();
             y = item.datapoint[1];
             showTooltip(item.pageX, item.pageY,
             item.series.label + " = " + y + "次");
         }
      }
      else {
         $("#tooltip").remove();
         previousPoint = null;            
      }
  	});
}

function showTooltip(x, y, contents) {
   $('<div id="tooltip">' + contents + '</div>').css( {
        position: 'absolute',
        display: 'none',
        top: y + 5,
        left: x + 5,
        border: '1px solid #fdd',
        padding: '2px',
        'background-color': '#fee',
        opacity: 0.80
    }).appendTo("body").fadeIn(100);
}

function getResolution()
{
	var str = "order=resolution&width=" + screen.width + "&height=" + screen.height ;
	getJsonFromServer ( false, str, "php/php_getmsg.php", noDoAnyThing ) ;
}

function noDoAnyThing( sdata )
{
	//alert ( sdata ) ;
}

function getJsonFromServer(isAsync ,sendData, serverUrl, funcReturn) {
			$.ajax({
				type: 'Post',//or 'get'
				contentType: "application/x-www-form-urlencoded",
				url: serverUrl, //PHP server url
				data: sendData,
				async: isAsync,
				dataType: "text", //i.e. XML ,Json jsonp script html text...
				success:function(sdata, textStatus, jqXHR){
					funcReturn(sdata);
				},
				error:function(){
					funcReturn("连接错误");
				}
			});
}

var plotArray = new Array();
var subDataArr = new Array() ;
var oldCount = new Array() ;
var json2 = new Array() ;

for ( i = 0; i < 11; i++ )
{
	subDataArr[i] = new Array() ;
}
for ( i = 0; i < 11; i++ )
{
	oldCount[i] = -1 ;
}

var number = 0 ;
var updateTime_all = 5 ;
var count_all = 13 ;
var orderCommand = "" ;

function initChar ( command1, command2, updatetime, count_in )
{
	updateTime_all = updatetime ;
	orderCommand = command1 ;
	count_all = count_in ;
	drawLine ( "#drow_pic_1", "insert"      , "" , "" ) ;
	drawLine ( "#drow_pic_2", "delete"      , "" , "" ) ;
	drawLine ( "#drow_pic_3", "update"      , "" , "" ) ;
	drawLine ( "#drow_pic_4", "insert(copy)", "" , "" ) ;
	drawLine ( "#drow_pic_5", "delete(copy)", "" , "" ) ;
	drawLine ( "#drow_pic_6", "update(copy)", "" , "" ) ;
	drawLine ( "#drow_pic_7", "query"       , "" , "" ) ;
	var url = "respond.php";
	var data = command2 ;
	var temp_arr ;
	getJsonFromServer (false, data, url, getMAD ) ;
	function getMAD ( sdata )
	{
		temp_arr = eval('('+sdata+')') ;
	}
	drawLine ( "#drow_pic_8", "Use(MB)"      , "Total(MB)" , temp_arr.MaxMemory ) ;
	drawLine ( "#drow_pic_9", "Use(GB)"      , "Total(GB)" , temp_arr.MaxDisk   ) ;
	number = 0 ;
}



function drawLine( container, labelName, labelName2, num2 )
{ // count of points
  var subData = [] ;
  var subData2 = [] ;
  var lineTimeOut = null;
  var initialData ;
  var placeholder = $(container);
  //var data = [[0.1, 0.1],[0.2, 0.2],[0.3, 0.3],[0.4, 0.4],[0.5, 0.5]];
	
  var xMax = (count_all-1)*updateTime_all;
  //initial subData
  for(var i = 0; i < count_all; ++i)
  {
  	var metaData = [];
  	metaData[0] = i*updateTime_all;
  	metaData[1] = 0;
  	subData[i] = metaData;
  }

  var lineData = [];
  var json = {
  		"label": labelName,
  		"data": subData,
		"color": "green"
  };
  
  if ( number > 6 )
  {
	  initialData = num2;
  	
	  for(var i = 0; i < count_all; ++i)
	  {
		var metaData = [];
		metaData[0] = i*updateTime_all;
		metaData[1] = initialData;
		subData2[i] = metaData;
	  }
	  json2[number] = {
		"label": labelName2,
		"data": subData2,
		"color": "red"
	  };
  }

	lineData.push(json);
	if ( number > 6 )
	{
		lineData.push(json2[number]);
	}
	var options = {
  	series: {
  		label: "Line",
  		lines: {show:true},
  		points: {show:false}
  	},
  	xaxis: {
  		tickDecimals: 0,	//decimal
  		color: "#E3C7E3",
  		min: 0,
  		max: xMax,
  		tickSize: updateTime_all
  	},
  	yaxis: {
  		tickDecimals: 1,	//decimal
  		position: "left",	//position
  		color: "#E2C7E2",
  		min: 0
  	}  	
  }; 
  plotArray[number] = $.plot(placeholder, lineData, options);
  subDataArr[number] = subData ;
  number++ ;
}

function reUpdate()
{
	var url = "respond.php";
	var data = orderCommand ;
	getJsonFromServer (true, data, url, getSdata ) ;
}

function getSdata(sdata)
{
	//alert(sdata);
	var temp_arr = eval('('+sdata+')') ;
	updata ( temp_arr.insert, "insert",       "","", 0 ) ;
	updata ( temp_arr.del,    "delete",       "","", 1 ) ;
	updata ( temp_arr.update, "update",       "","", 2 ) ;
	updata ( temp_arr.insertc,"insert(copy)", "","", 3 ) ;
	updata ( temp_arr.deletec,"delete(copy)", "","", 4 ) ;
	updata ( temp_arr.updatec,"update(copy)", "","", 5 ) ;
	updata ( temp_arr.query,  "query",        "","", 6 ) ;
	updata ( temp_arr.memory, "Use(MB)",      temp_arr.tmemory, "Total(MB)", 7 ) ;
	updata ( temp_arr.disk,   "Use(GB)",      temp_arr.tdisk,   "Total(GB)", 8 ) ;
}

function updata( sdata, labelName, sdata2, labelName2, num )
{
	if(oldCount[num] < 0)
	{
		oldCount[num] = sdata;
		return;
	}
	var lineData = [];
	var metaData = [];
	var newCount = 0;
	for(var i = count_all-1; i > 0 ; --i)
	{
		var metaData = [];
		metaData[0] = subDataArr[num][i][0];
		metaData[1] = subDataArr[num][i-1][1];
		subDataArr[num][i] = metaData;
	}
	if ( num > 6 )
	{
		newCount = sdata ;
	}
	else
	{
		newCount = (sdata - oldCount[num])/updateTime_all;
	}
	oldCount[num] = sdata;
	
	var metaData = [];
	var json2 = [] ;
	if ( num > 6 )
	  {
		  var subData2 = [] ;
		  initialData = sdata2;
		  for(var i = 0; i < count_all; ++i)
		  {
			  var metaData2 = [];
			metaData2[0] = i*updateTime_all;
			metaData2[1] = initialData;
			subData2[i] = metaData2;
		  }
		    json2 = {
			"label": labelName2,
			"data": subData2,
			"color": "red"
		  };
	  }
	metaData[0] = 0;
	metaData[1] = newCount;
	subDataArr[num][0] = metaData;

	var json = {
	"label": labelName,
	"data": subDataArr[num],
	"color": "green"
	};

	lineData.push(json);
	if ( num > 6 )
	{
		lineData.push(json2);
	}
	var plot = plotArray[num] ;
	plot.setData(lineData);
	plot.setupGrid();
	plot.draw();
}

function newaweb ( json )
{
	var newW = window.open("", "edition" );
	newW.document.write( JsonUti.convertToString( json ) ) ;
}

//调用 format_str = JsonUti.convertToString(json);  
var JsonUti = {
    //定义换行符
    n: "<br/>",
    //定义制表符
    t: "&nbsp;&nbsp;&nbsp;",//&#9;
    //转换String
    convertToString: function(obj) {
		var temp_arr = eval('('+obj+')') ;
        var new_str = JsonUti.__writeObj(temp_arr, 1);
		new_str = "<PRE>" + new_str + "</PRE>" ;
		return new_str ;
    },
    //写对象
    __writeObj: function(obj //对象
    , level //层次（基数为1）
    , isInArray) { //此对象是否在一个集合内
        //如果为空，直接输出null
        if (obj == null) {
            return "null";
        }
        //为普通类型，直接输出值
        if (obj.constructor == Number || obj.constructor == Date || obj.constructor == String || obj.constructor == Boolean) {
            var v = obj.toString();
            var tab = isInArray ? JsonUti.__repeatStr(JsonUti.t, level - 1) : "";
            if (obj.constructor == String || obj.constructor == Date) {
                //时间格式化只是单纯输出字符串，而不是Date对象
                return tab + ("\"" + v + "\"");
            }
            else if (obj.constructor == Boolean) {
                return tab + v.toLowerCase();
            }
            else {
                return tab + (v);
            }
        }
        //写Json对象，缓存字符串
        var currentObjStrings = [];
        //遍历属性
        for (var name in obj) {
            var temp = [];
            //格式化Tab
            var paddingTab = JsonUti.__repeatStr(JsonUti.t, level);
            temp.push(paddingTab);
            //写出属性名
            temp.push(name + " : ");
            var val = obj[name];
            if (val == null) {
                temp.push("null");
            }
            else {
                var c = val.constructor;
                if (c == Array) { //如果为集合，循环内部对象
                    temp.push(JsonUti.n + paddingTab + "[" + JsonUti.n);
                    var levelUp = level + 2; //层级+2
                    var tempArrValue = []; //集合元素相关字符串缓存片段
                    for (var i = 0; i < val.length; i++) {
                        //递归写对象
                        tempArrValue.push(JsonUti.__writeObj(val[i], levelUp, true));
                    }
                    temp.push(tempArrValue.join("," + JsonUti.n));
                    temp.push(JsonUti.n + paddingTab + "]");
                }
                else if (c == Function) {
                    temp.push("[Function]");
                }
                else {
                    //递归写对象
                    temp.push(JsonUti.__writeObj(val, level + 1));
                }
            }
            //加入当前对象"属性"字符串
            currentObjStrings.push(temp.join(""));
        }
        return (level > 1 && !isInArray ? JsonUti.n: "") //如果Json对象是内部，就要换行格式化
        + JsonUti.__repeatStr(JsonUti.t, level - 1) + "{" + JsonUti.n //加层次Tab格式化
        + currentObjStrings.join("," + JsonUti.n) //串联所有属性值
        + JsonUti.n + JsonUti.__repeatStr(JsonUti.t, level - 1) + "}"; //封闭对象
    },
    __isArray: function(obj) {
        if (obj) {
            return obj.constructor == Array;
        }
        return false;
    },
    __repeatStr: function(str, times) {
        var newStr = [];
        if (times > 0) {
            for (var i = 0; i < times; i++) {
                newStr.push(str);
            }
        }
        return newStr.join("");
    }
};