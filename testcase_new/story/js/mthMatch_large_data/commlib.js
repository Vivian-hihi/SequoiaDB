/************************************
*@Description: insert data
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function insertData( dbcl, insertData )
{  
   try
   {
      dbcl.insert(insertData);
      println( "--insert data success" ) ;
   }
   catch(e)
   {
      throw buildException("insertData()",e,"insert", "insert success","insert fail");
   }
}

/**********************************************************************
@Description:  generate all kinds of types data randomly
@author:       Ting YU
@usage:        var rd = new commDataGenerator();
               1.var recs = rd.getRecords( 300, "int", ['a','b'] );
               2.var recs = rd.getRecords( 3, ["int", "string"] );
               3.rd.getValue("string");
***********************************************************************/
function dataGenerator()
{
   this.getRecords =
   function( recNum, dataTypes, fieldNames )
   {
      return getRandomRecords( recNum, dataTypes, fieldNames );
   }
   
   this.getValue = 
   function( dataType )
   {
      return getRandomValue( dataType );
   }
}

function getRandomRecords( recNum, dataTypes, fieldNames )
{
  if( fieldNames === undefined ) { fieldNames = getRandomFieldNames(); }
  if( dataTypes.constructor !== Array ) { dataTypes = [ dataTypes ]; }
  
  var recs = [];
  var i = 0;
  for( var i = 0; i < recNum; i++ )
  {          
	 // generate 1 record
	 var rec = {};
	 rec["No"] = i;
	 for( var j in fieldNames )
	 {  
		// generate 1 filed
		var rdn = parseInt( Math.random() * dataTypes.length );
		if( rdn % 2 == 0){
			var filedName = fieldNames[j];
			var dataType = dataTypes[rdn];  //randomly get 1 data type
			var filedVal =  getRandomValue( dataType );
		}
		if( filedVal !== undefined ) rec[filedName] = filedVal;
	 } 
	 
	 recs.push( rec );
  }   
  return recs;
}

function getRandomFieldNames( minNum, maxNum )
{
  if( minNum == undefined ) { minNum = 0; }
  if( maxNum == undefined ) { maxNum = 16; }
  
  var fieldNames = [];
  var fieldNum = getRandomInt( minNum, maxNum );
  
  for( var i = 0; i < fieldNum; i++ )
  {
	 //get 1 field name
	 var fieldName = "";
	 var fieldNameLen = getRandomInt( 1, 9 );
	 for( var j = 0; j < fieldNameLen; j++ )
	 {
		//get 1 char
		var ascii = getRandomInt( 97, 123 ); // 'a'~'z'
		var c = String.fromCharCode( ascii );
		fieldName += c;
	 }
	 fieldNames.push( fieldName );
  }
  
  return fieldNames;
}

/**********************************************************************
@Description:  generate all kinds of types data randomly
@author:       Ting YU
@usage:        var rd = new commDataGenerator();
               1.var recs = rd.getRecords( 300, "int", ['a','b'] );
               2.var recs = rd.getRecords( 3, ["int", "string"] );
               3.rd.getValue("string");
***********************************************************************/
function getRandomValue( dataType )
{
  var value = undefined;
  
  switch( dataType )
  {
	 case "int":
		value = getRandomInt( -2147483648, 2147483647 );
		break;
	 case "long":
		value = getRandomLong( -922337203685477600, 922337203685477600 );
		break;
	 case "float":
		value = getRandomFloat( -999999, 999999 );
		break;
	 case "array":
		value = getRandomArray();
		break; 
	 case "non-existed":
		break;                 
  }
  
  return value;
}

function getRandomInt( min, max ) // [min, max)
   {
      var range = max - min;
      var value = min + parseInt( Math.random() * range );
      return value;
   }
   
function getRandomLong( min, max )
{
  var value = getRandomInt( min, max );
  return NumberLong(value);
}

function getRandomFloat( min, max )
{
  var range = max - min;
  var value = min + Math.random() * range;
  return value;
}

function getRandomArray()
{
  var arr = [];   
  var dataTypes= [ "int", "long", "float" ];
				   
  var arrLen = getRandomInt( 1, 5 );  
  for( var i = 0; i < arrLen; i++ )
  {
	 var dataType = dataTypes[ parseInt( Math.random() * dataTypes.length ) ];  //randomly get 1 data type 
	 
	 var elem = getRandomValue( dataType );
	 arr.push( elem );
  }
  
  return arr;
}
 
function genRandomFindCondition( matches, fieldNames )
{
	var recs = [];
	// generate 1 record     
	for( var j in fieldNames )
	{  	
		var filedName = fieldNames[j];
		var rec = {}; 
		// generate 1 filed
		var rdn = parseInt( Math.random() * matches.length );
		if( rdn % 2 === 0){
			var match = matches[rdn];
			rec[filedName] = match;
			recs.push( rec ); 
		}
	}   
  return recs;
}

function getFindCondition( recNum, dataTypes, matches )
{
  if( dataTypes.constructor !== Array ) { dataTypes = [ dataTypes ]; }
  
  var recs = [];
  for( var i = 0; i < recNum; i++ )
  {          
	 // generate 1 record
	 var rec = {};      
	 for( var j in matches )
	 {  
		// generate 1 filed
		var match = matches[j];

		var dataType = dataTypes[ parseInt( Math.random() * dataTypes.length ) ];  //randomly get 1 data type
		var filedVal =  getRandomValue( dataType );
		
		if( filedVal !== undefined ) rec[match] = filedVal;       
	 } 
	 
	 recs.push( rec );
  }   
  return recs;
}

function mergeObj(left,right)
{
	var obj ={}
	for(var k in left)
		obj[k] = left[k];
	
	for (var k in right)
		obj[k] = right[k];
	
	return obj;
}