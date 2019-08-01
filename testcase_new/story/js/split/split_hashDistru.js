/***************************************************
@description: jira801：不同数据类型在hash分区中分布是否合理
@modify list:
              2015-8-6 Ting YU init          
***************************************************/
main();

function main()
{	  
	try
	{	   
      var domName= CHANGEDPREFIX;
      var csName = CHANGEDPREFIX;
      var clName = CHANGEDPREFIX;
      var SDKEYNAME = "a";
      var recSum = 1000;
      var partition = getRandomPartition();
      var groupsName = []; //all groups name in cluster
            
		if (true == commIsStandalone(db))
      {
      	println("Mode is standalone!");
      }
      else if ( commGetGroups(db).length < 2)
      {
      	println("data groups number is less than 2! You need at least 2 groups to split!");
      }
      else
      {  
         ready( csName, domName );
         var groupsName = createDomainCSCL( csName, clName, domName, SDKEYNAME, partition );
             						
			intTest      ( csName, clName, groupsName, SDKEYNAME, recSum );
      	longTest     ( csName, clName, groupsName, SDKEYNAME, recSum );
      	floatTest    ( csName, clName, groupsName, SDKEYNAME, recSum );
      	arrayTest    ( csName, clName, groupsName, SDKEYNAME, recSum );
      	stringTest   ( csName, clName, groupsName, SDKEYNAME, recSum );
      	objectTest   ( csName, clName, groupsName, SDKEYNAME, recSum );
      	regexTest    ( csName, clName, groupsName, SDKEYNAME, recSum );
      	binaryTest   ( csName, clName, groupsName, SDKEYNAME, recSum );
      	oidTest      ( csName, clName, groupsName, SDKEYNAME, recSum );
      	dateTest     ( csName, clName, groupsName, SDKEYNAME, recSum );
      	timestampTest( csName, clName, groupsName, SDKEYNAME, recSum );
				
         clean( csName, domName );
      }
	}
	catch(e)
	{
		throw e;
	}
}




function intTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'int';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 
   var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = i;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function longTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'long';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	var expRecArr = [];
	for(var i = 0; i < recSum; i++)
	{
	   var sdkeyVal = i + 10000000000;
	   
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = NumberLong( sdkeyVal );
		recArr.push( rec );
		
		var recExp = {};   
		recExp["type"]    = dataType;
		recExp[SDKEYNAME] = sdkeyVal;
      expRecArr.push( recExp );		
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, expRecArr, SDKEYNAME );
}

function floatTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'float';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{  
	   //change int to float, eg: 1.0 --> 1.95
	   var sdkeyVal = i/9; 
		var str = sdkeyVal.toString();
		if (-1 == str.indexOf('.') )
		sdkeyVal = sdkeyVal + 0.95;
		
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = sdkeyVal;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function arrayTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'array';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = [i];
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function stringTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'string';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = "sting"+i;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}
		
function objectTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'object';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
	   //generate random object
	   var rd = parseInt( Math.random()*26,10) + 65;//'A'-'Z'	
		var fieldname = String.fromCharCode(rd);
		var sdkeyVal = {};  
		sdkeyVal[fieldname] = i;
	   
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = sdkeyVal;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function binaryTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'binary';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
	   //generate random binary
	   var tmp = '';
		for ( var j = 0; j < 16; j++)
		{	
			tmp += parseInt( Math.random()*10,10);
		}
	 //var tmp = (1E15+i).toString();
		var sdkeyVal = {"$binary":tmp, "$type":"1"}; 
		
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = sdkeyVal;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function dateTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'date';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
	   //generate random date
	   var year= parseInt( Math.random()*8089,   10) + 1901; //1901-9999
		var mon = parseInt( Math.random()*12,     10) + 1;    //1-12
		var day = parseInt( Math.random()*28,     10) + 1;    //1-28
		var tmp = year + '-' + mon + '-' + day;
		var sdkeyVal = {"$date":tmp}; 
		
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = sdkeyVal;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
//	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function timestampTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'timestamp';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
	   //generate random timestamp
	   var year= parseInt( Math.random()*135,    10) + 1902; //1902-2037
		var mon = parseInt( Math.random()*12,     10) + 1;    //1-12
		var day = parseInt( Math.random()*28,     10) + 1;    //1-28
		var hour= parseInt( Math.random()*24,     10);        //0-23
		var min = parseInt( Math.random()*60,     10);        //0-59
		var sec = parseInt( Math.random()*60,     10);        //0-59
		var fse = parseInt( Math.random()*900000,10) +100000;//100000-999999
		var tmp = year + '-' + mon + '-' + day + '-' + hour + '.'+ min + '.'+ sec + '.'+ fse;
		var sdkeyVal = {"$timestamp":tmp}; 
		
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = sdkeyVal;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
//	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function regexTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'regex';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 	
	var recArr = [];	
	var optionsArr = ['i', 'm', 'x', 's'];
	for(var i = 0; i < recSum; i++)
	{
	   //generate random regex
	   var para1 = '^' + (1E15+i).toString();
		var para2 = optionsArr[ parseInt( Math.random()*4,10) ];
		var sdkeyVal ={"$regex":para1, "$options":para2}; 
		
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = sdkeyVal;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

function oidTest( csName, clName, groupsName, SDKEYNAME, recSum )
{	
	var dataType = 'oid';
	println("\n----begin to insert: " + recSum + " records, data type: "+dataType);
	
	//insert records 
	var sdkeyValArr = new Array();
	
	var recArr = [];	
	for(var i = 0; i < recSum; i++)
	{
		var rec = {};   
		rec["type"]    = dataType;
		rec[SDKEYNAME] = ObjectId();   ;
		recArr.push( rec );
	}
	db.getCS(csName).getCL(clName).insert( recArr );
	//check hash
	checkHashDistru ( csName, clName, groupsName, recSum, dataType );
	checkHashEachRec( csName, clName, recArr, SDKEYNAME );
}

/******************************************
检验每个组的记录数的总和是否等于总数
检验任意两个组的差值是否小于每组预期值的30%
******************************************/
function checkHashDistru( csName, clName, groupsName, expSum, dataType )
{
	print("------begin to check data distrubution: ");
	
	//get count in all groups
	var cntGroups = [];
	for( var i in groupsName )
	{
	   var masterNode = db.getRG(groupsName[i]).getMaster();
	   var cnt = new Sdb(masterNode).getCS(csName).getCL(clName).count({type:dataType});
	   cntGroups.push( cnt );
	}	
	
	//get sum and check
	var sum = 0;	
	for(var i in cntGroups)
	{
		sum += cntGroups[i];
		print( cntGroups[i] + ", " );
	}
	println("");
		
	if( sum != expSum )
	{
		println("ERROR! sum of all groups rec num is wrong! expect="+expSum+", actual="+sum);
		throw "ERROR!";
	}
	
	//get difference and check
	var max = Math.max.apply(null, cntGroups);
	var min = Math.min.apply(null, cntGroups);
	var dif = max - min;
	var hashLimit = (expSum / cntGroups.length) * 0.3;
	
	if( dif > hashLimit )
	{
		println("ERROR! diff of 2 groups rec num is out of limit!");
		throw "ERROR!";
	}
}

/**********************************
对每条记录，用分区键字段count
**********************************/
function checkHashEachRec( csName, clName, recArr, SDKEYNAME )
{
	println("------begin to find each rec with ShardingKey condition");
	
	for(var i = 0; i < recArr.length; i++)
	{
      var cond = {}
      cond["type"] = recArr[i]["type"]
      var etCond = {}
      etCond["$et"] = recArr[i][SDKEYNAME]
      cond[SDKEYNAME] = etCond
		var cnt = db.getCS(csName).getCL(clName).count(cond);
		if( cnt != 1)
		{
			println("ERROR! i="+i+", count="+cnt+", count( "+JSON.stringify(recArr[i])+" )");
			throw "ERROR!";
		}
	}
}
function createDomainCSCL( csName, clName, domName, SDKEYNAME, partition )
{
   //get all group name to array
   var groupsName = [];
	var tmpGroups = commGetGroups(db);
	for(var i in tmpGroups)
	{
		groupsName.push( tmpGroups[i][0]["GroupName"] );
	}
	
   //create domain
   println( "\n----begin to create domain in groups: "+groupsName );
   commDropDomain( db, domName);
   var dom = commCreateDomain( db, domName, groupsName, {AutoSplit:true});
	
	//create cs
	println( "\n----begin to create cs in domain" );
   commCreateCS( db, csName, false, "create cs", {"Domain": domName} );
   
   //create cl
   var order = getRandomOrder();
   
   var tmpSK = {};
   tmpSK[SDKEYNAME] = order;
   var opt = {'ShardingKey':tmpSK, 'ShardingType':'hash', 'Partition':partition, 'ReplSize':0};
   println( "\n----begin to create cl: " + JSON.stringify(opt) );
   commCreateCLByOption( db, csName, clName, opt, false, false, "create cl" )
   
   return groupsName;
}

function ready( csName, domName )
{
   println( "\n----begin to execute ready()" );
   
   commDropCS( db, csName, true, "drop cs in ready" );
	commDropDomain( db, domName);
}

function clean( csName, domName )
{
   println( "\n----begin to execute clean()" );
   
   commDropCS( db, csName, true, "drop cs in finally");
	commDropDomain( db, domName);
}

function getRandomPartition()
{
   var partiArr = [];
   for( var n = 6; n < 21; n++ )  // 2^6~2^20      2^3 2^4 2^5 distribute unevenly
   {
      var tmp = Math.pow( 2, n );
      partiArr.push( tmp );
   }  
   
   var len = partiArr.length;
   var index = parseInt( Math.random() * len );
   
   var partition = partiArr[ index ];
   return partition;  
}

function getRandomOrder()
{
   var arr = [ -1, 1];
   
   var len = arr.length;
   var index = parseInt( Math.random() * len );
   
   var order = arr[ index ];
   return order;  
}
