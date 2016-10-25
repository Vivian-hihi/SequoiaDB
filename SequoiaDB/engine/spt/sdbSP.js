
// BSONObj
BSONObj.prototype.toObj = function() {
   return JSON.parse( this.toJson() ) ;
}

BSONObj.prototype.toString = function() {
   try
   {
      var obj = this.toObj() ;
      var str = JSON.stringify( obj, undefined, 2 ) ;
      return str ;
   }
   catch( e )
   {
      return this.toJson() ;
   }
}
// end BSONObj

// BSONArray
BSONArray.prototype.toArray = function() {
   if ( this._arr )
      return this._arr;

   var a = [];
   while ( true ) {
      var bs = this.next();
      if ( ! bs ) break ;
      var json = bs.toJson () ;
      try
      {
         var stf = JSON.stringify(JSON.parse(json), undefined, 2) ;
         a.push ( stf ) ;
      }
      catch ( e )
      {
         a.push ( json ) ;
      }
   }
   this._arr = a ;
   return this._arr ;
}

BSONArray.prototype.arrayAccess = function( idx ) {
   return this.toArray()[idx] ;
}

BSONArray.prototype.toString = function() {
   //return this.toArray().join('\n') ;
   var array = this ;
   var record = undefined ;
   var returnRecordNum = 0 ;
   while ( ( record = array.next() ) != undefined )
   {
      returnRecordNum++ ;
      try
      {
         println ( record ) ;
      }
      catch ( e )
      {
         var json = record.toJson () ;
         println ( json ) ;
      }
   }
   println("Return "+returnRecordNum+" row(s).") ;
   return "" ;
}

BSONArray.prototype._formatStr = function() {

   var bsonObj = this.toArray() ;
   var objArr = new Array() ; 
   var strArr = new Array() ;
   var maxSizeArr = new Array() ;
   var outStr = "" ;
   var eleArr = new Array() ;
   
   var objNum ;
   var eleNum ;
   
   for ( var i in bsonObj )
   {
      objArr.push( JSON.parse( bsonObj[i] ) ) ;
   }
   
   var objNum = objArr.length ;

   if ( objNum > 0 )
   {
		var eleNum = Object.keys( objArr[0] ).length ;
	  
	  	for ( var eleKey in objArr[0] )
      {
	     	eleArr.push( eleKey ) ;
	  	}
      strArr.push( eleArr ) ;
	   
	  	for ( var obj in  objArr )
	  	{
	     	eleArr = new Array() ;
	     	for( var ele in  objArr[obj] )
	     	{
	     	   eleArr.push( objArr[obj][ele] ) ;
	     	}
    	 	strArr.push( eleArr ) ;
	  	}
	   
		for ( var i = 0; i < eleNum; i++ )
		{
			var max = 0 ;
			for ( var j = 0; j < objNum + 1; j++ )
			{
				if ( strArr[j][i].length > max )
				{
					max = strArr[j][i].length ;
				}
			}
			maxSizeArr.push( max + 1 ) ;
		}
	   
	  for ( var i = 0; i < strArr.length; i++ )
	  {
	   	var arr = strArr[i] ;
	     	for ( var j = 0; j < arr.length; j++ )
	     	{
	        	outStr += " " + arr[j] ;
				for ( var k = 0; k < maxSizeArr[j] - arr[j].length; k++ )
				{
			   	outStr += " " ;
				}
	     	}
		 	outStr += "\n" ;
	  }
   }
   return outStr ;
}
// end BSONArray

// Oma
Oma.prototype.help = function( val ) {
   if ( val == undefined )
   {
      println("OMA methods:") ;
      println("   oma.help(<method>)          help on specified method of oma, e.g. oma.help(\'createData\')");
      man( "oma" ) ;
   }
   else
   {
      man( "oma", val ) ;
   }
}
// end Oma
