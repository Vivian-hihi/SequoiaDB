
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
