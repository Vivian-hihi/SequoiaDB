// Global Constants
var SDB_PAGESIZE_4K              = 4096 ;
var SDB_PAGESIZE_8K              = 8192 ;
var SDB_PAGESIZE_16K             = 16384 ;
var SDB_PAGESIZE_32K             = 32768 ;
var SDB_PAGESIZE_64K             = 65536 ;
var SDB_PAGESIZE_DEFAULT         = SDB_PAGESIZE_64K ;

var SDB_SNAP_CONTEXTS            = 0 ;
var SDB_SNAP_CONTEXTS_CURRENT    = 1 ;
var SDB_SNAP_SESSIONS            = 2 ;
var SDB_SNAP_SESSIONS_CURRENT    = 3 ;
var SDB_SNAP_COLLECTIONS         = 4 ;
var SDB_SNAP_COLLECTIONSPACES    = 5 ;
var SDB_SNAP_DATABASE            = 6 ;
var SDB_SNAP_SYSTEM              = 7 ;
var SDB_SNAP_CATALOG             = 8 ;

var SDB_LIST_CONTEXTS            = 0 ;
var SDB_LIST_CONTEXTS_CURRENT    = 1 ;
var SDB_LIST_SESSIONS            = 2 ;
var SDB_LIST_SESSIONS_CURRENT    = 3 ;
var SDB_LIST_COLLECTIONS         = 4 ;
var SDB_LIST_COLLECTIONSPACES    = 5 ;
var SDB_LIST_STORAGEUNITS        = 6 ;
var SDB_LIST_GROUPS              = 7 ;
var SDB_LIST_STOREPROCEDURES     = 8 ;
var SDB_LIST_DOMAINS             = 9 ;
var SDB_LIST_TASKS               = 10 ;

var SDB_INSERT_CONTONDUP         = 1 ;
var SDB_INSERT_RETURN_ID         = 2 ; // only available when inserting only one document

var SDB_TRACE_FLW                = 0 ;
var SDB_TRACE_FMT                = 1 ;

var SDB_COORD_GROUP_NAME         = "SYSCoord" ;
var SDB_CATALOG_GROUP_NAME       = "SYSCatalogGroup" ;
var SDB_SPARE_GROUP_NAME         = "SYSSpare" ;

var SDB_PRINT_JSON_FORMAT        = true ;

var SDB_JSON_PARSE               = JSON.parse ;
// end Global Constants

// Global functions

// register json function
//JSON.parse JSON.stringify
function SDB_INIT(){

   function isArray( object ){
      return ( object &&
               typeof( object ) === 'object' &&
               typeof( object.length ) === 'number' &&
               typeof( object.splice ) === 'function' &&
               !( object.propertyIsEnumerable( 'length' ) ) ) ;
   }

   function filterInviChart(str) {
      var i = 0, len = str.length;
      var newStr = '';
      var chars, code;
      while (i < len) {
         chars = str.charAt(i);
         code = chars.charCodeAt();
         if (code < 0x20 || code == 0x7F) {
            chars = '?';
         }
         newStr += chars;
         ++i;
      }
      return newStr;
   }

   JSON.parse = function(str, func) {
      var json;
      try {
         json = SDB_JSON_PARSE(str, func);
      } catch (e) {
         try {
            var newStr = filterInviChart(str);
            json = SDB_JSON_PARSE(newStr, func);
         } catch (e) {
            json = eval('(' + str + ')');
         }
      }
      return json;
   } ;

   //var rx_escapable = /[\\\"\u0000-\u001f\u007f-\u009f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g;

   function f(n) {
      return n < 10
          ? "0" + n
          : n;
   }

   function this_value() {
      return this.valueOf();
   }

   if (typeof Date.prototype.toJSON !== "function") {

      Date.prototype.toJSON = function () {

         return isFinite(this.valueOf())
             ? this.getUTCFullYear() + "-" +
                     f(this.getUTCMonth() + 1) + "-" +
                     f(this.getUTCDate()) + "T" +
                     f(this.getUTCHours()) + ":" +
                     f(this.getUTCMinutes()) + ":" +
                     f(this.getUTCSeconds()) + "Z"
             : null;
      };

      Boolean.prototype.toJSON = this_value;
      Number.prototype.toJSON = this_value;
      String.prototype.toJSON = this_value;
   }

   var gap;
   var indent;
   var meta;
   var rep;


   function quote(str) {
      var newString = "" ;
      var length = str.length ;
      for( var i = 0; i < length; ++i )
      {
         switch( str.charAt( i ) )
         {
         case "\"":
            newString += "\\\"" ;
            break ;
         case "\\":
            newString += "\\\\" ;
            break ;
         case "\b":
            newString += "\\b" ;
            break ;
         case "\f":
            newString += "\\f" ;
            break ;
         case "\n":
            newString += "\\n" ;
            break ;
         case "\r":
            newString += "\\r" ;
            break ;
         case "\t":
            newString += "\\t" ;
            break ;
         default:
            newString += str.charAt( i ) ;
         }
      }
      return "\"" + newString + "\"" ;
   }


   function str(key, holder) {

      var i;
      var k;
      var v;
      var length;
      var mind = gap;
      var partial;
      var value = holder[key];

      if (value && typeof value === "object" &&
              typeof value.toJSON === "function") {
         value = value.toJSON(key);
      }

      if (typeof rep === "function") {
         value = rep.call(holder, key, value);
      }

      switch (typeof value) {
         case "string":
            return quote(value);

         case "number":
            if (value === Number.POSITIVE_INFINITY) {
               return 'Infinity';
            }
            else if (value === Number.NEGATIVE_INFINITY) {
               return '-Infinity';
            }
            else if (value === Number.NaN) {
               return '0';
            }
            else {
               return String(value);
            }

         case "boolean":
         case "null":
            return String(value);

         case "object":
            if (!value) {
               return "null";
            }
            gap += indent;
            partial = [];

            if( isArray( value ) ) {
               length = value.length;
               for (i = 0; i < length; i += 1) {
                  partial[i] = str(i, value) || "null";
               }

               v = partial.length === 0
                   ? "[]"
                   : gap
                       ? "[\n" + gap + partial.join(",\n" + gap) + "\n" + mind + "]"
                       : "[" + partial.join(",") + "]";
               gap = mind;
               return v;
            }

            if (rep && typeof rep === "object") {
               length = rep.length;
               for (i = 0; i < length; i += 1) {
                  if (typeof rep[i] === "string") {
                     k = rep[i];
                     v = str(k, value);
                     if (v) {
                        partial.push(quote(k) + (
                            gap
                                ? ": "
                                : ":"
                        ) + v);
                     }
                  }
               }
            } else {

               for (k in value) {
                  //if (Object.prototype.hasOwnProperty.call(value, k)) {
                     v = str(k, value);
                     if (v) {
                        partial.push(quote(k) + (
                            gap
                                ? ": "
                                : ":"
                        ) + v);
                     }
                  //}
               }
            }

            v = partial.length === 0
                ? "{}"
                : gap
                    ? "{\n" + gap + partial.join(",\n" + gap) + "\n" + mind + "}"
                    : "{" + partial.join(",") + "}";
            gap = mind;
            return v;
      }
   }

   meta = {
      "\b": "\\b",
      "\t": "\\t",
      "\n": "\\n",
      "\f": "\\f",
      "\r": "\\r",
      "\"": "\\\"",
      "\\": "\\\\"
   };

   JSON.stringify = function (value, replacer, space) {

      var i;
      gap = "";
      indent = "";

      if (typeof space === "number") {
         for (i = 0; i < space; i += 1) {
            indent += " ";
         }

      } else if (typeof space === "string") {
         indent = space;
      }

      rep = replacer;
      if (replacer && typeof replacer !== "function" &&
              (typeof replacer !== "object" ||
              typeof replacer.length !== "number")) {
         throw new Error("JSON.stringify");
      }

      return str("", { "": value });
   };

}
SDB_INIT() ;

function println ( val ) {
   if ( arguments.length > 0 )
      print ( val ) ;
   print ( '\n' ) ;
}
// return a double number between 0 and 1
function rand () {
   return Math.random() ;
}

// return merged json object
function mergeJsonObject(obj1, obj2) {
   var result = {};
   for (var attr in obj1) {
      result[attr] = obj1[attr];
   }
   for (var attr in obj2) {
      result[attr] = obj2[attr];
   }

   return result;
}

function isEmptyObject(obj) {
   for (var name in obj) {
      return false;
   }

   return true;
}

function jsonFormat(pretty) {
   if (pretty == undefined){
      pretty = true;
   }
   SDB_PRINT_JSON_FORMAT = pretty;
}

// end Global functions

Object.defineProperty(Object.prototype,'_rawValueOf',{
   enumerable:false,
   value: Object.prototype.valueOf
});

Object.defineProperty(Object.prototype,'_rawToString',{
   enumerable:false,
   value: Object.prototype.toString
});

Object.defineProperty(Object.prototype,'_equality',{
   enumerable:false,
   value: function(rval) {
      if ( this.$numberLong != undefined ) {
         if ( rval.$numberLong != undefined ) {
            return this.valueOf() == rval.valueOf() ;
         }
         return rval == this.valueOf() ;
      }
      if ( rval.$numberLong != undefined ) {
         return this == rval.valueOf() ;
      }

      throw "condition not suitable for the function" ;
   }
});

Object.prototype.valueOf = function() {
   if (this.$numberLong != undefined) {
      if ( typeof(this.$numberLong ) == "string" )
      {
         return parseInt(this.$numberLong) ;
      }
      else if ( typeof(this.$numberLong ) == "number" )
      {
         return this.$numberLong ;
      }
      else
      {
         throw "invalid $numberLong" ;
      }
   }

   return this._rawValueOf() ;
}

Object.prototype.toString = function() {
   if (this.$numberLong != undefined) {
      try
      {
         return JSON.stringify ( this, undefined, 2 ) ;
      }
      catch ( e )
      {
      }
   }
   return this._rawToString() ;
}

// Bson
function _numberLongRevier(key, value) {
   if ( "number" === typeof(value) ) {
      // if we use +/-9007199254740992(+/-2^53) as the max/min valid integer
      // for sdb shell to input, we must use "<=" and ">=" to compare,
      // because, when value is greater then 9007199254740992,
      // value may show as 9007199254740992
      if (value < -9007199254740991 || value > 9007199254740991 ) {
         throw "can't display number, for it's too large" ;
      }
   }
   return value ;
}

Bson.prototype.toObj = function() {
   return JSON.parse( this.toJson() ) ;
}

Bson.prototype.toString = function() {
   if ( typeof(SDB_PRINT_JSON_FORMAT) == "undefined" ||
        SDB_PRINT_JSON_FORMAT )
   {
      try
      {
         var obj = this.toObj();
         var str = JSON.stringify ( obj, undefined, 2 ) ;
         return str ;
      }
      catch ( e )
      {
         return this.toJson() ;
      }
   }
   else
   {
      return this.toJson() ;
   }
}
// end Bson

// SdbCursor
SdbCursor.prototype.toArray = function() {
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

SdbCursor.prototype.arrayAccess = function( idx ) {
   return this.toArray()[idx] ;
}

SdbCursor.prototype.size = function() {
   //return this.toArray().length ;
   var cursor = this ;
   var size = 0 ;
   var record = undefined ;
   while( ( record = cursor.next() ) != undefined )
   {
      size++ ;
   }
   return size ;
}

SdbCursor.prototype.toString = function() {
   //return this.toArray().join('\n') ;
   var csr = this ;
   var record = undefined ;
   var returnRecordNum = 0 ;
   while ( ( record = csr.next() ) != undefined )
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
// end SdbCursor


// CLCount
CLCount.prototype.toString = function() {
   this._exec() ;
   return this._count ;
}
CLCount.prototype.valueOf = function() {
   this._exec() ;
   return this._count ;
}
CLCount.prototype.hint = function( hint ) {
   this._hint = hint ;
   return this ;
}
CLCount.prototype._exec = function() {
   this._count = this._collection._count ( this._condition,
                                           this._hint ) ;
}
// end CLCount

// SdbCollection
SdbCollection.prototype.count = function( condition ) {
   return new CLCount( this, condition ) ;
}

SdbCollection.prototype.find = function( query, select ) {
   return new SdbQuery( this , query, select );
}

SdbCollection.prototype.findOne = function( query, select ) {
   return new SdbQuery( this , query, select ).limit( 1 ) ;
}

SdbCollection.prototype.getIndex = function( name ) {
   if ( ! name )
      throw "SdbCollection.getIndex(): 1st parameter should be valid string" ;
   var obj = this._getIndexes(name).next();
   if (undefined == obj)
   {
      setLastError( SDB_IXM_NOTEXIST ) ;
      setLastErrMsg( getErr( SDB_IXM_NOTEXIST ) ) ;
      throw SDB_IXM_NOTEXIST ;
   }

   return obj ;
}

SdbCollection.prototype.listIndexes = function() {
   return this._getIndexes() ;
}

SdbCollection.prototype.toString = function() {
   return this._cs.toString() + "." + this._name;
}

SdbCollection.prototype.insert = function ( data , flags )
{
   if ( (typeof data) != "object" )
      throw "SdbCollection.insert(): the 1st param should be obj or array of objs";
   var newFlags = 0 ;
   if ( flags != undefined )
   {
      if ( (typeof flags) != "number" ||
            ( flags != 0 &&
              flags != SDB_INSERT_RETURN_ID &&
              flags != SDB_INSERT_CONTONDUP ) )
         throw "SdbCollection.insert(): the 2nd param if existed should be 0 or SDB_INSERT_RETURN_ID or SDB_INSERT_CONTONDUP only";
      newFlags = flags ;
   }

   if ( data instanceof Array )
   {
      if ( 0 == data.length ) return ;
      if ( newFlags != 0 && newFlags != SDB_INSERT_CONTONDUP )
         throw "SdbCollection.insert(): when insert more than 1 records, the 2nd param if existed should be 0 or SDB_INSERT_CONTONDUP only";
      return this._bulkInsert ( data , newFlags ) ;
   }
   else
   {
      if ( newFlags != 0 && newFlags != SDB_INSERT_RETURN_ID )
         throw "SdbCollection.insert(): when insert 1 record, the 2nd param if existed should be 0 or SDB_INSERT_RETURN_ID only";
      return this._insert ( data , SDB_INSERT_RETURN_ID == flags ) ;
   }
}

/*
SdbCollection.prototype.rename = function ( newName ) {
   this._rename ( newName ) ;
   this._name = newName ;
}
*/
// end SdbCollection

// SdbQuery
SdbQuery.prototype._checkExecuted = function() {
   if ( this._cursor )
      throw "query already executed";
}

SdbQuery.prototype._exec = function() {
   if ( ! this._cursor ) {
      this._cursor = this._collection.rawFind( this._query,
                                               this._select,
                                               this._sort,
                                               this._hint,
                                               this._skip,
                                               this._limit,
                                               this._flags );
   }
   return this._cursor;
}

SdbQuery.prototype.sort = function( sort ) {
   this._checkExecuted();
   this._sort = sort;
   return this;
}

SdbQuery.prototype.hint = function( hint ) {
   this._checkExecuted();
   if (undefined == this._hint) {
      this._hint = hint;
   } else {
      this._hint = mergeJsonObject(hint, this._hint);
   }
   return this;
}

SdbQuery.prototype.skip = function( skip ) {
   this._checkExecuted();
   this._skip = skip;
   return this;
}

SdbQuery.prototype.limit = function( limit ) {
   this._checkExecuted();
   this._limit = limit;
   return this;
}

SdbQuery.prototype.flags = function( flags ) {
   this._checkExecuted();
   this._flags = flags;
   return this;
}

SdbQuery.prototype.next = function() {
   this._exec();
   return this._cursor.next();
}

SdbQuery.prototype.current = function() {
   this._exec();
   return this._cursor.current();
}

SdbQuery.prototype.close = function() {
   this._exec();
   return this._cursor.close();
}

SdbQuery.prototype.update = function( rule, returnNew ) {
   if ((typeof rule) != "object" || isEmptyObject(rule)) {
      throw "SdbQuery.update(): the 1st param should be non-empty object";
   }
   if (undefined != returnNew && (typeof returnNew) != "boolean") {
      throw "SdbQuery.update(): the 2nd param should be boolean";
   }

   this._checkExecuted();

   if (undefined == this._hint) {
      this._hint = {};
   } else if (undefined != this._hint.$Modify) {
      throw "SdbQuery.update(): duplicate modification";
   }

   var modify = {};
   modify.OP = "update";
   modify.Update = rule;
   modify.ReturnNew = (returnNew != undefined) ? returnNew : false;
   this._hint.$Modify = modify;

   return this;
}

SdbQuery.prototype.remove = function() {
   this._checkExecuted();
   if (undefined == this._hint) {
      this._hint = {};
   } else if (undefined != this._hint.$Modify) {
      throw "SdbQuery.remove(): duplicate modification";
   }

   var modify = {};
   modify.OP = "remove";
   modify.Remove = true;
   this._hint.$Modify = modify;

   return this;
}

/*
SdbQuery.prototype.updateCurrent = function ( rule ) {
   this._exec();
   return this._cursor.updateCurrent( rule ) ;
}

SdbQuery.prototype.deleteCurrent = function () {
   this._exec();
   return this._cursor.deleteCurrent();
}
*/
SdbQuery.prototype.toArray = function() {
   this._exec();
   return this._cursor.toArray();
}

SdbQuery.prototype.arrayAccess = function( idx ) {
   return this.toArray()[idx];
}

SdbQuery.prototype.count = function() {
   if (undefined != this._hint && undefined != this._hint.$Modify) {
      throw "count() cannot be executed with update() or remove()";
   }
   var countObj = this._collection.count( this._query ) ;
   if ( undefined != this._hint ) {
      countObj.hint( this._hint ) ;
   }
   return countObj ;
}

SdbQuery.prototype.explain = function( options ) {
   return this._collection.explain( this._query,
                                    this._select,
                                    this._sort,
                                    this._hint,
                                    this._skip,
                                    this._limit,
                                    options ) ;
}

SdbQuery.prototype.size = function() {
//   return this.toArray().length;
   this._exec();
   return this._cursor.size() ;
}

SdbQuery.prototype.toString = function() {
   this._exec();
   var csr = this._cursor ;
   var record = undefined ;
   var returnRecordNum = 0 ;
   while ( ( record = csr.next() ) != undefined )
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
   //return this._cursor.toString();
}
// end SdbQuery

// SdbNode
SdbNode.prototype.toString = function() {
   return this._hostname + ":" +
          this._servicename ;
}

SdbNode.prototype.getHostName = function() {
   return this._hostname ;
}

SdbNode.prototype.getServiceName = function() {
   return this._servicename ;
}

SdbNode.prototype.getNodeDetail = function() {
   return this._nodeid + ":" + this._hostname + ":" +
          this._servicename + "(" +
          this._rg.toString() + ")" ;
}
// end SdbNode

// SdbReplicaGroup
SdbReplicaGroup.prototype.toString = function() {
   return this._name;
}

SdbReplicaGroup.prototype.getDetail = function() {
   return this._conn.list( SDB_LIST_GROUPS,
                           {GroupName: this._name } ) ;
}
// end SdbReplicaGroup

// SdbCS
SdbCS.prototype.toString = function() {
   return this._conn.toString() + "." + this._name;
}

SdbCS.prototype._resolveCL = function(clName) {
   this.getCL(clName) ;
}
// end SdbCS


// SdbDomain
SdbDomain.prototype.toString = function() {
   return this._domainname ;
}
// end SdbDomain

// SdbDc
SdbDC.prototype.toString = function() {
   return this._name ;
}
// end SdbDc

// Sdb
Sdb.prototype.toString = function() {
   return this._host + ":" + this._port;
}

Sdb.prototype.listCollectionSpaces = function() {
   return this.list( SDB_LIST_COLLECTIONSPACES ) ;
}

Sdb.prototype.listCollections = function() {
   return this.list( SDB_LIST_COLLECTIONS ) ;
}

Sdb.prototype.listReplicaGroups = function() {
   return this.list( SDB_LIST_GROUPS ) ;
}

Sdb.prototype._resolveCS = function(csName) {
   this.getCS( csName ) ;
}

Sdb.prototype.getCatalogRG = function() {
   return this.getRG( SDB_CATALOG_GROUP_NAME ) ;
}

Sdb.prototype.removeCatalogRG = function() {
   return this.removeRG( SDB_CATALOG_GROUP_NAME ) ;
}

Sdb.prototype.createCoordRG = function() {
   return this.createRG( SDB_COORD_GROUP_NAME ) ;
}

Sdb.prototype.removeCoordRG = function() {
   return this.removeRG( SDB_COORD_GROUP_NAME ) ;
}

Sdb.prototype.getCoordRG = function() {
   return this.getRG( SDB_COORD_GROUP_NAME ) ;
}

Sdb.prototype.createSpareRG = function() {
   return this.createRG(SDB_SPARE_GROUP_NAME) ;
}

Sdb.prototype.getSpareRG = function() {
   return this.getRG(SDB_SPARE_GROUP_NAME) ;
}

Sdb.prototype.removeSpareRG  = function() {
   return this.removeRG( SDB_SPARE_GROUP_NAME ) ;
}

// end Sdb

function printCallStack()
{
   try
   {
      throw new Error( "print ErrStack" ) ;
   }
   catch ( e )
   {
      print( e.stack ) ;
   }
}

function assert( condition )
{
   if ( !condition )
   {
      printCallStack() ;
   }
}

// ObjectId
if ( !ObjectId.prototype )
   ObjectId.prototype = {}

ObjectId.prototype.toString = function() {
   return "ObjectId(\"" + this._str + "\")" ;
}
// end ObjectId

// BinData
if ( !BinData.prototype )
   BinData.prototype = {}

BinData.prototype.toString = function() {
   return "BinData(\"" + this._data + "\", \"" + this._type + "\")"  ;
}


// end BinData

// Timestamp
if ( !Timestamp.prototype )
   Timestamp.prototype = {}

Timestamp.prototype.toString = function() {
   return "Timestamp(\"" + this._t + "\")" ;
}
// end Timestamp

// Regex
if ( !Regex.prototype )
   Regex.prototype = {}

Regex.prototype.toString = function () {
   return "Regex(\"" + this._regex + "\", \"" + this._option + "\")" ;
}
// end Regex

// MinKey
if ( !MinKey.prototype )
   MinKey.prototype = {}

MinKey.prototype.toString = function() {
   return "MinKey()" ;
}
// end MinKey

// MaxKey
if ( !MaxKey.prototype )
   MaxKey.prototype = {}

MaxKey.prototype.toString = function() {
   return "MaxKey()" ;
}
// end MaxKey

// NumberLong
if ( !NumberLong.prototype )
   NumberLong.prototype = {}

NumberLong.prototype.toString = function() {
   if ( typeof(this._v ) == "string" )
   {
      return "NumberLong(\"" + this._v + "\")" ;
   }
   return "NumberLong(" + this._v + ")" ;
}

NumberLong.prototype.valueOf = function() {
   if ( typeof(this._v ) == "string" )
   {
      return parseInt(this._v) ;
   }
   return this._v ;
}

// end NumberLong

// SdbDate
if ( !SdbDate.prototype )
   SdbDate.prototype = {}

SdbDate.prototype.toString = function() {
   return "SdbDate(\"" + this._d + "\")" ;
}
// end SdbDate


// Remote


Remote.prototype.getSystem = function() {
	var system = System.getObj() ;
	system._remote = this ;
	return system ;
}

Remote.prototype.getFile = function( filename, mode ) {
	var file = File._getFileObj() ;
	file._remote = this ;

	if ( undefined != filename )
	{
	   if ( undefined != mode )
	   {
	   	this._runCommand( "file open",{ "mode": mode }, {},
							   	{ "filename": filename } ) ;
	   }
		else
		{
			this._runCommand( "file open",{}, {},
							   	{ "filename": filename } ) ;
		}
		file._filename = filename ;
		file._location = 0 ;
		file._isOpened = true ;
	}

	return file ;
}

Remote.prototype.getCmd = function() {
	var cmd = new Cmd() ;
	cmd._remote = this ;
	cmd._retCode = SDB_OK ;
	cmd._strOut = '' ;
	cmd._command = '' ;
	return cmd ;
}

Remote.prototype._runCommand = function( command, optionObj, matchObj, valueObj ) {
   var bsonObj ;
   var retObj ;
   if( 6 < arguments.length )
   {
	   setLastErrMsg( "too much arguments" ) ;
      throw SDB_INVALIDARG ;
   }
   else if ( undefined != valueObj )
   {
      bsonObj = this.__runCommand( command, optionObj, matchObj, valueObj ) ;
   }
   else if ( undefined != matchObj )
   {
      bsonObj = this.__runCommand( command, optionObj, matchObj ) ;
   }
   else if ( undefined != optionObj  )
   {
      bsonObj = this.__runCommand( command, optionObj ) ;
   }
   else
   {
      bsonObj = this.__runCommand( command ) ;
   }

   return bsonObj ;
}

// end Remote

// _Filter
_Filter.prototype.match = function( BSONArrObj ) {

   if ( BSONArrObj instanceof Object )
   {
      return this._match( BSONArrObj ) ;
   }
   else
   {
      setLastErrMsg( "argument must be objArray" ) ;
	   throw SDB_INVALIDARG ;
   }
}
// end _Filter

// System
System.prototype.getInfo = function()
{
   var result ;
	var infoObj ;
	if ( undefined != this._remote )
	{
		var _remoteInfo = this._remote.getInfo() ;
		infoObj = _remoteInfo.toObj() ;
		infoObj.isRemote = true ;
	}
	else
	{
		infoObj = new Object() ;
		infoObj.isRemote = false ;
	}
	result = this._getInfo( infoObj ) ;
	return result ;
}

System.prototype.ping = function( hostname ) {
	var retObj ;

	if ( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "ping", {}, { "hostname" : hostname } ) ;
	}
	else
	{
		retObj = System.ping( hostname ) ;
	}
	return retObj ;

}

System.prototype.type = function() {

	var retStr ;
	if ( 0 < arguments.length )
	{
		setLastErrMsg( getErr( SDB_INVALIDARG ) ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined != this._remote )
	{
	    var retObj = this._remote._runCommand( "system type" ) ;
		retStr = retObj.toObj().type ;
	}
	else
	{
		retStr = System.type() ;
	}
	return retStr ;

}

System.prototype.getReleaseInfo = function() {
	var retObj ;

	if ( 0 < arguments.length )
	{
		setLastErrMsg( getErr( SDB_INVALIDARG ) ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "system get release info" ) ;
	}
	else
	{
		retObj = System.getReleaseInfo() ;
	}
	return retObj ;
}

System.prototype.getHostsMap = function() {
	var retObj ;

	if ( 0 < arguments.length )
	{
		setLastErrMsg( "getHostsMap() should have non arguments" ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get hosts map" ) ;
	}
	else
	{
		retObj = System.getHostsMap() ;
	}
   return retObj ;
}

System.prototype.getAHostMap = function( hostname ) {
	var retStr ;

	if ( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get a host map", {}, { "hostname": hostname } ) ;
		retStr = retObj.toObj().ip ;
	}
	else
	{
		retStr = System.getAHostMap() ;
	}
	return retStr ;
}

System.prototype.addAHostMap = function( hostname, ip, isReplace ) {
	if ( undefined != this._remote )
	{
		this._remote._runCommand( "add a host map", {}, {},
								 		 { "hostname": hostname,
								   		"ip": ip,
								   		"isReplace": isReplace } ) ;
	}
	else
	{
		System.addAHostMap( hostname, ip, isReplace ) ;
	}
}

System.prototype.delAHostMap = function( hostname ) {
	if ( undefined != this._remote )
	{
		this._remote._runCommand( "delete a host map", {}, { "hostname": hostname } ) ;

	}
	else
	{
		System.delAHostMap( hostname ) ;
	}
}

System.prototype.getCpuInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get cpu info" ) ;
	}
	else
	{
		retObj = System.getCpuInfo() ;
	}
	return retObj ;
}

System.prototype.snapshotCpuInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "snapshot cpu info" ) ;
	}
	else
	{
		retObj = System.snapshotCpuInfo() ;
	}
	return retObj ;
}

System.prototype.getMemInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get mem info" ) ;
	}
	else
	{
		retObj = System.getMemInfo() ;
	}
	return retObj ;
}

System.prototype.snapshotMemInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get mem info" ) ;
	}
	else
	{
		retObj = System.snapshotMemInfo() ;
	}
	return retObj ;
}

System.prototype.getDiskInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get disk info" ) ;
	}
	else
	{
		retObj = System.getDiskInfo() ;
	}
	return retObj ;
}

System.prototype.snapshotDiskInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get disk info" ) ;
	}
	else
	{
		retObj = System.snapshotDiskInfo() ;
	}
	return retObj ;
}

System.prototype.getNetcardInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get netcard info" ) ;
	}
	else
	{
		retObj = System.getNetcardInfo() ;
	}
	return retObj ;
}

System.prototype.snapshotNetcardInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "snapshot netcard info" ) ;
	}
	else
	{
		retObj = System.snapshotNetcardInfo() ;
	}
	return retObj ;
}

System.prototype.getIpTablesInfo = function() {
	var retObj ;
	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get ip tables info" ) ;
	}
	else
	{
		retObj = System.getIpTablesInfo() ;
	}
	return retObj ;
}

System.prototype.getHostName = function() {
	var result ;

	if ( 0 < arguments.length )
	{
		setLastErrMsg( getErr( SDB_INVALIDARG ) ) ;
		throw SDB_INVALIDARG ;
	}

	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "get hostname" ) ;
		result = retObj.toObj().hostname ;
	}
	else
	{
		result = System.getHostName() ;
	}
	return result ;
}

System.prototype.sniffPort = function( port ) {
	var retObj ;

	if( undefined == port )
	{
		setLastErrMsg( "not specified the port to sniff" ) ;
		throw SDB_INVALIDARG ;
	}

	if( undefined != this._remote )
	{
		var retObj = this._remote._runCommand( "sniff port", {}, { "port": port }) ;
	}
	else
	{
		retObj = System.sniffPort( port ) ;
	}
	return retObj ;
}

System.prototype.listProcess = function( optionObj, filterObj ) {
   var retObj ;
   var result ;

	if ( undefined != this._remote )
   {
		var displayMode = "obj" ;

		if ( undefined != optionObj )
		{
			if ( undefined != optionObj.displayMode  )
			{
				displayMode = optionObj.displayMode ;
				delete optionObj.displayMode ;
			}
		}

		if ( undefined != optionObj )
		{
			result = this._remote._runCommand( "list process", optionObj ) ;
		}
		else
		{
			result = this._remote._runCommand( "list process" ) ;
		}

		if ( undefined != filterObj )
		{
			var filter = new _Filter( filterObj ) ;
	   	retObj = filter.match( result.toObj() ) ;
		}
		else
		{
			var filter = new _Filter( {} ) ;
	   	retObj = filter.match( result.toObj() ) ;
		}

		if ( "text" == displayMode )
		{
		   return retObj._formatStr() ;
		}
		else
		{
		   return retObj ;
		}
   }
   else
   {
   	retObj = System.listProcess( optionObj, filterObj ) ;
		return retObj ;
   }
}

System.listProcess = function( optionObj, filterObj )
{
	var result ;
   var recvObj ;
	var displayMode = "obj" ;

	if ( undefined != optionObj )
	{
		if ( undefined != optionObj.displayMode )
		{
			displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;
		}
		recvObj = System._listProcess( optionObj ) ;
	}
	else
	{
		recvObj = System._listProcess() ;
	}

   if( undefined != filterObj )
   {
      var filter = new _Filter( filterObj ) ;
	   result = filter.match( recvObj.toObj() ) ;
   }
   else
   {
      var filter = new _Filter( {} ) ;
	  	result = filter.match( recvObj.toObj() ) ;
   }

   if( "text" == displayMode )
   {
      return result._formatStr() ;
   }
   else
   {
      return result ;
   }
}

System.prototype.isProcExist = function( optionObj ) {

	var retArray ;
	var isExist = false ;

	if ( undefined != optionObj )
   {
      if( undefined == optionObj.value )
      {
			setLastErrMsg( "value must be config" ) ;
			throw SDB_OUT_OF_BOUND ;
      }
		else
		{
			if ( "name" == optionObj.type )
      	{
       		retArray = this.listProcess( { "detail": true }, { "cmd": optionObj.value } ) ;
     		}
			else
			{
				retArray = this.listProcess( { "detail": true }, { "pid": optionObj.value } ) ;
			}
		}

  	}
	else
	{
		setLastErrMsg( "optionObj must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( 0 != retArray.size() )
	{
		isExist = true ;
	}
	return isExist ;
}

System.isProcExist = function( optionObj ) {
	var retArray ;
	var isExist ;

	if ( undefined != optionObj )
   {
      if( undefined == optionObj.value )
      {
			setLastErrMsg( "value must be config" ) ;
			throw SDB_OUT_OF_BOUND ;
      }
		else
		{
			if ( optionObj.type == "name" )
      	{
       		retArray = System.listProcess( { "detail": true }, { "cmd": optionObj.value } ) ;
     		}
			else
			{
				retArray = System.listProcess( { "detail": true }, { "pid": optionObj.value } ) ;
			}
		}
  	}
	else
	{
		setLastErrMsg( "optionObj must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( 0 != retArray.size() )
	{
		isExist = true ;
	}
	else
	{
		isExist = false ;
	}
	return isExist ;
}

System.prototype.addUser = function( userObj ) {
	if ( undefined != this._remote )
	{
	   this._remote._runCommand( "add user", {}, {}, userObj ) ;
	}
	else
	{
		System.addUser( userObj ) ;
	}
}

System.prototype.setUserConfigs = function( optionObj ) {
	if ( undefined != this._remote )
	{
	   this._remote._runCommand( "set user configs", {}, {}, optionObj ) ;
	}
	else
	{
		System.setUserConfigs( optionObj ) ;
	}
}

System.prototype.delUser = function( optionObj ) {
	if ( undefined != this._remote )
	{
	   this._remote._runCommand( "del user", {}, optionObj ) ;
	}
	else
	{
		System.delUser( optionObj ) ;
	}
}

System.prototype.addGroup = function( optionObj ) {
	if ( undefined != this._remote )
	{
	   this._remote._runCommand( "add group", {}, {}, optionObj ) ;
	}
	else
	{
		System.addGroup( optionObj ) ;
	}
}

System.prototype.delGroup = function( name ) {
	if ( undefined != this._remote )
	{
	   this._remote._runCommand( "del group", {}, { "name" :name } ) ;
	}
	else
	{
		System.delGroup( name ) ;
	}
}

System.prototype.listLoginUsers = function( optionObj, filterObj ) {
	var retArray ;

	if ( undefined != this._remote )
	{
		var objArray ;
		var displayMode = "obj" ;
		if ( undefined != optionObj )
		{
			if ( undefined != optionObj.displayMode )
			{
				displayMode = optionObj.displayMode ;
				delete optionObj.displayMode ;
			}
			objArray = this._remote._runCommand( "list login users", optionObj ) ;
		}
		else
		{
			objArray = this._remote._runCommand( "list login users" ) ;
		}

		if( undefined != filterObj )
	   {
	      var filter = new _Filter( filterObj ) ;
		   retArray = filter.match( objArray.toObj() ) ;
	   }
	   else
	   {
	      var filter = new _Filter( {} ) ;
		  	retArray = filter.match( objArray.toObj() ) ;
	   }

	   if( "text" == displayMode )
	   {
	      retArray = retArray._formatStr() ;
	   }
	}
	else
	{
		retArray = System.listLoginUsers( optionObj, filterObj ) ;
	}

	return retArray ;
}

System.listLoginUsers = function( optionObj, filterObj ) {
	var objArray ;
   var retArray ;
	var displayMode = "obj" ;

	if ( undefined != optionObj )
	{
		if ( undefined != optionObj.displayMode )
		{
			displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;
		}
		objArray = System._listLoginUsers( optionObj ) ;
	}
	else
	{
		objArray = System._listLoginUsers() ;
	}

   if( undefined != filterObj )
   {
      var filter = new _Filter( filterObj ) ;
	   retArray = filter.match( objArray.toObj() ) ;
   }
   else
   {
      var filter = new _Filter( {} ) ;
	  	retArray = filter.match( objArray.toObj() ) ;
   }

   if( "text" == displayMode )
   {
      retArray = retArray._formatStr() ;
   }
   return retArray ;
}

System.prototype.listAllUsers = function( optionObj, filterObj ) {
	var retArray ;

	if ( undefined != this._remote )
	{
		var objArray ;
		var displayMode = "obj" ;
		if ( undefined != optionObj )
		{
			if ( undefined != optionObj.displayMode )
			{
				displayMode = optionObj.displayMode ;
				delete optionObj.displayMode ;
			}
			objArray = this._remote._runCommand( "list all users", optionObj ) ;
		}
		else
		{
			objArray = this._remote._runCommand( "list all users" ) ;
		}

		if( undefined != filterObj )
	   {
	      var filter = new _Filter( filterObj ) ;
		   retArray = filter.match( objArray.toObj() ) ;
	   }
	   else
	   {
	      var filter = new _Filter( {} ) ;
		  	retArray = filter.match( objArray.toObj() ) ;
	   }

	   if( "text" == displayMode )
	   {
	      retArray = retArray._formatStr() ;
	   }
	}
	else
	{
		retArray = System.listAllUsers( optionObj, filterObj ) ;
	}
	return retArray ;
}

System.listAllUsers = function( optionObj, filterObj ) {
	var objArray ;
   var retArray ;
	var displayMode = "obj" ;

	if ( undefined != optionObj )
	{
		if ( undefined != optionObj.displayMode )
		{
			displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;
		}
		objArray = System._listAllUsers( optionObj ) ;
	}
	else
	{
		objArray = System._listAllUsers() ;
	}

   if( undefined != filterObj )
   {
      var filter = new _Filter( filterObj ) ;
	   retArray = filter.match( objArray.toObj() ) ;
   }
   else
   {
      var filter = new _Filter( {} ) ;
	  	retArray = filter.match( objArray.toObj() ) ;
   }

   if( "text" == displayMode )
   {
      retArray = retArray._formatStr() ;
   }

   return retArray ;
}

System.prototype.listGroups = function( optionObj, filterObj ) {
	var retArray ;

	if ( undefined != this._remote )
	{
		var objArray ;
		var displayMode = "obj" ;
		if ( undefined != optionObj )
		{
			if ( undefined != optionObj.displayMode )
			{
				displayMode = optionObj.displayMode ;
				delete optionObj.displayMode ;
			}
			objArray = this._remote._runCommand( "list all groups", optionObj ) ;
		}
		else
		{
			objArray = this._remote._runCommand( "list all groups" ) ;
		}

		if( undefined != filterObj )
	   {
	      var filter = new _Filter( filterObj ) ;
		   retArray = filter.match( objArray.toObj() ) ;
	   }
	   else
	   {
	      var filter = new _Filter( {} ) ;
		  	retArray = filter.match( objArray.toObj() ) ;
	   }

	   if( "text" == displayMode )
	   {
	      retArray = retArray._formatStr() ;
	   }
	}
	else
	{
		retArray = System.listGroups( optionObj, filterObj ) ;
	}

	return retArray ;
}

System.listGroups = function( optionObj, filterObj ) {
	var objArray ;
   var retArray ;
	var displayMode = "obj" ;
	if ( undefined != optionObj )
	{
		if ( undefined != optionObj.displayMode )
		{
			displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;
		}
		objArray = System._listGroups( optionObj ) ;
	}
	else
	{
		objArray = System._listGroups() ;
	}

   if( undefined != filterObj )
   {
      var filter = new _Filter( filterObj ) ;
	   retArray = filter.match( objArray.toObj() ) ;
   }
   else
   {
      var filter = new _Filter( {} ) ;
	  	retArray = filter.match( objArray.toObj() ) ;
   }

   if( "text" == displayMode )
   {
      retArray = retArray._formatStr() ;
   }
   return retArray ;
}

System.prototype.getCurrentUser = function() {
	var retObj ;

	if ( undefined != this._remote )
	{
		retObj = this._remote._runCommand( "get current user" ) ;
	}
	else
	{
		retObj = System.getCurrentUser() ;
	}
	return retObj ;
}

System.prototype.isUserExist = function( userName ) {
	var isExist = false ;
	var retArray ;

	if( undefined == userName )
   {
		setLastErrMsg( "userName must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
   }

	if ( undefined != this._remote )
	{
		retArray = this.listAllUsers( { "detail": true }, { "user": userName } ) ;
	}
	else
	{
		retArray = System.listAllUsers( { "detail": true }, { "user": userName } ) ;
	}

	if ( 0 != retArray.size() )
	{
		isExist = true ;
	}
	return isExist ;
}

System.isUserExist = function( userName ) {
	var isExist = false ;
	var retArray ;

	if( undefined == userName )
   {
		setLastErrMsg( "userName must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
   }
	else
	{
		retArray = System.listAllUsers( { "detail": true }, { "user": userName } ) ;
	}

	if ( 0 != retArray.size() )
	{
		isExist = true ;
	}
	return isExist ;
}

System.prototype.isGroupExist = function( groupName ) {
	var isExist = false ;
	var retArray ;

	if( undefined == groupName )
   {
		setLastErrMsg( "groupName must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
   }

	if ( undefined != this._remote )
	{
		retArray = this.listGroups( { "detail": true }, { "name": groupName } ) ;
	}
	else
	{
		retArray = System.listGroups( { "detail": true }, { "name": groupName } ) ;
	}
	if ( 0 != retArray.size() )
	{
		isExist = true ;
	}
	return isExist ;
}

System.isGroupExist = function( groupName ) {
	var isExist = false ;
	var retArray ;

	if( undefined == groupName )
   {
		setLastErrMsg( "userName must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
   }

	retArray = System.listGroups( { "detail": true }, { "name": groupName } ) ;

	if ( 0 != retArray.size() )
	{
		isExist = true ;
	}
	return isExist ;
}

System.prototype.killProcess = function( optionObj ) {
	if ( undefined == optionObj )
	{
		setLastErrMsg( "optionObj must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}
	if ( "object" != typeof( optionObj ) )
	{
		setLastErrMsg( "optionObj must be BsonObj" ) ;
		throw SDB_INVALIDARG ;
	}
	if ( undefined != this._remote )
	{
		this._remote._runCommand( "kill process", { "sig" : optionObj.sig }, { "pid": optionObj.pid } ) ;
	}
	else
	{
		System.killProcess( optionObj ) ;
	}
}

System.prototype.getProcUlimitConfigs = function() {
	var retObj ;
	if ( 0 < arguments.length )
	{
		setLastErrMsg( "getUlimitConfigs() should have non arguments" ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined != this._remote )
	{
		retObj = this._remote._runCommand( "get proc ulimit configs" ) ;
	}
	else
	{
		retObj = System.getProcUlimitConfigs() ;
	}
	return retObj ;
}

System.prototype.setProcUlimitConfigs = function( configsObj ) {

	if ( undefined == configsObj )
	{
		setLastErrMsg( "configsObj must be configs" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != this._remote )
	{
		this._remote._runCommand( "set proc ulimit configs", {},{},{ "configs": configsObj } ) ;
	}
	else
	{
		System.setProcUlimitConfigs( configsObj ) ;
	}
}

System.prototype.getUserEnv = function() {
	var result ;
	if ( undefined != this._remote )
	{
		result = this._remote._runCommand( "get user env" ) ;
	}
	else
	{
		result = System.getUserEnv() ;
	}
	return result ;
}

System.prototype.getSystemConfigs = function( type ) {
	var retObj ;

	if ( undefined != this._remote )
	{
		if ( undefined != type )
		{
			retObj = this._remote._runCommand( "get system configs", { "type": type } ) ;
		}
		else
		{
			retObj = this._remote._runCommand( "get system configs" ) ;
		}
	}
	else
	{
		if ( undefined != type )
		{
			retObj = System.getSystemConfigs( type ) ;
		}
		else
		{
			retObj = System.getSystemConfigs( ) ;
		}
	}
	return retObj ;
}

System.prototype.runService = function ( serviceName, command, options ) {
	var result ;

	if ( undefined == serviceName )
	{
		setLastErrMsg( "serviceName must be config" ) ;
		throw SDB_INVALIDARG ;
	}
	if ( 'string' != typeof( serviceName ) )
	{
		setLastErrMsg( "serviceName must be string" ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined == command )
	{
		setLastErrMsg( "command must be config" ) ;
		throw SDB_INVALIDARG ;
	}
	if ( 'string' != typeof( command ) )
	{
		setLastErrMsg( "command must be string" ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined != this._remote )
	{
		var retObj ;
		if ( "undefined" != options )
		{
		   retObj = this._remote._runCommand( "run service", { "command": command, "options": options },
				        		                    { "serviceName": serviceName } ) ;
		}
		else
		{
			retObj = this._remote._runCommand( "run service", { "command": command },
				      			                 { "serviceName": serviceName } ) ;
		}
		result = retObj.toObj().outStr ;
	}
	else
	{
	   if ( "undefined" != options )
	   {
			result = System.runService( serviceName, command, options ) ;
	   }
		else
		{
			result = System.runService( serviceName, command ) ;
		}
	}
	return result ;
}



System.prototype.buildTrusty = function() {

   if ( "LINUX" == System.type() )
   {
   	if ( undefined != this._remote )
   	{

   		var homeDir = System._getHomePath() ;
   		System._createSshKey() ;
   		var pubKey = File._readFile( homeDir + "/.ssh/id_rsa.pub" ) ;
   		this._remote._runCommand( "build trusty", {}, {}, { "key": pubKey } ) ;
      }
	}
}

System.prototype.removeTrusty = function() {
   if ( "LINUX" == System.type() )
   {
   	if ( undefined != this._remote )
   	{
   		var homeDir = System._getHomePath() ;
   		var matchStr = File._readFile( homeDir + "/.ssh/id_rsa.pub" ) ;
   		this._remote._runCommand( "remove trusty", {}, {},
                                   { "matchStr": matchStr } ) ;
   	}
   }
}

System.prototype.getPID = function() {
   if ( 0 < arguments.length )
	{
		setLastErrMsg( "No need arguments" ) ;
		throw SDB_INVALIDARG ;
	}
   var pid ;
   if ( undefined != this._remote )
   {
      pid = this._remote._runCommand( "get pid" ).toObj().PID ;
   }
   else
   {
      pid = System.getPID() ;
   }
   return pid ;
}

System.prototype.getTID = function() {
   if ( 0 < arguments.length )
	{
		setLastErrMsg( "No need arguments" ) ;
		throw SDB_INVALIDARG ;
	}
   var tid ;
   if ( undefined != this._remote )
   {
      tid = this._remote._runCommand( "get tid" ).toObj().TID ;
   }
   else
   {
      tid = System.getTID() ;
   }
   return tid ;
}

System.prototype.getEWD = function() {
   if ( 0 < arguments.length )
	{
		setLastErrMsg( "No need arguments" ) ;
		throw SDB_INVALIDARG ;
	}
   var ewd ;
   if ( undefined != this._remote )
   {
      ewd = this._remote._runCommand( "get ewd" ).toObj().EWD ;
   }
   else
   {
      ewd = System.getEWD() ;
   }
   return ewd ;
}
// end System

// Cmd

Cmd.prototype.getInfo = function()
{
   var result ;
	var infoObj ;
	if ( undefined != this._remote )
	{
		var _remoteInfo = this._remote.getInfo() ;
		infoObj = _remoteInfo.toObj() ;
		infoObj.isRemote = true ;
	}
	else
	{
		infoObj = new Object() ;
		infoObj.isRemote = false ;
	}
	result = this._getInfo( infoObj ) ;
	return result ;
}

Cmd.prototype.getCommand = function()
{
	if ( undefined != this._remote )
	{
		return this._command ;
	}
	else
	{
		return this._getCommand() ;
	}
}

Cmd.prototype.getLastOut = function()
{
	if ( undefined != this._remote )
	{
		return this._strOut ;
	}
	else
	{
		return this._getLastOut() ;
	}
}

Cmd.prototype.getLastRet = function()
{
	if ( undefined != this._remote )
	{
		return this._retCode ;
	}
	else
	{
		return this._getLastRet() ;
	}
}

Cmd.prototype.run = function( cmd, args, timeout, useShell )
{
	var retStr ;
	if ( undefined == cmd )
	{
		setLastErrMsg( "cmd must be config" ) ;
		throw SDB_INVALIDARG ;
	}

	if ( 'string' != typeof( cmd ) )
	{
		setLastErrMsg( "cmd must be string" ) ;
		throw SDB_INVALIDARG ;
	}
	if ( undefined != this._remote )
	{

		var retObj = this._remote._runCommand( "cmd run", { "timeout": timeout, "useShell": useShell }, {},
										 				  { "command": cmd, "args": args } ).toObj() ;
		this._command = cmd ;
		this._retCode = retObj.retCode ;
		this._strOut = retObj.strOut ;

		if ( 0 != this._retCode )
		{
			setLastErrMsg( this._strOut ) ;
			throw this._retCode ;
		}
		else
		{
			retStr = this._strOut ;
		}
	}
	else
	{
		if ( undefined != useShell )
		{
			retStr = this._run( cmd, args, timeout, useShell ) ;
		}
		else if ( undefined != timeout )
		{
			retStr = this._run( cmd, args, timeout ) ;
		}
		else if ( undefined != args )
		{
			retStr = this._run( cmd, args ) ;
		}
		else
		{
			retStr = this._run( cmd ) ;
		}

	}
	return retStr ;
}

Cmd.prototype.start = function( cmd, args, useShell, timeout )
{
	var retStr ;
	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "cmd start", { "useShell": useShell, "timeout": timeout }, {},
			                      					{ "command": cmd, "args": args } ) ;
		var getObj = recvObj.toObj() ;

		this._command = cmd ;
		this._retCode = getObj.retCode ;
		this._strOut = getObj.strOut ;

		if ( 0 != this._retCode )
		{
			setLastErrMsg( getObj.strOut ) ;
			throw getObj.retCode ;
		}
		else
		{
			retStr = getObj.pid ;
		}
	}
	else
	{
		if ( undefined != timeout )
		{
			retStr = this._start( cmd, args, useShell, timeout ) ;
		}
		else if ( undefined != useShell )
		{
			retStr = this._start( cmd, args, useShell ) ;
		}
		else if ( undefined != args )
		{
			retStr = this._start( cmd, args ) ;
		}
		else
		{
			retStr = this._start( cmd ) ;
		}
	}

	return retStr ;
}

Cmd.prototype.runJS = function( code ) {
	var code ;
	if ( undefined == code )
	{
		setLastErrMsg( "code must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}
	if ( "string" != typeof( code ) )
	{
		setLastErrMsg( "code must be config" ) ;
		throw SDB_INVALIDARG ;
	}

	if ( undefined != this._remote )
	{
		var recvObj ;
		recvObj = this._remote._runCommand( "cmd run js", {}, {},
			                                   { "code": code } ) ;
		return recvObj.toObj().strOut ;
	}
	else
	{
		setLastErrMsg( "runJS() should be called by remote obj" ) ;
		throw SDB_SYS ;
	}
}

// end Cmd

// File

File.prototype.getInfo = function()
{
	var result ;
	var infoObj ;
	if ( undefined != this._remote )
	{
		var _remoteInfo = this._remote.getInfo() ;
		infoObj = _remoteInfo.toObj() ;
		infoObj.isRemote = true ;
		infoObj.filename = this._filename ;
	}
	else
	{
		infoObj = new Object() ;
		infoObj.isRemote = false ;
	}
	result = this._getInfo( infoObj ) ;
	return result ;
}

File.prototype.read = function( size ) {

	var str ;
	if ( undefined != this._remote )
	{
		if ( true != this._isOpened )
		{
			setLastErrMsg( "file is not opened" ) ;
			throw SDB_IO ;
		}

		var retObj ;
		if( undefined != size )
		{
			retObj= this._remote._runCommand( "file read",{}, {},
											   		{ "size":size,
											   		  "filename": this._filename,
											   		  "location": this._location } ) ;
		}
		else
		{
			retObj= this._remote._runCommand( "file read",{}, {},
											   		{ "filename": this._filename,
											   		  "location": this._location } ) ;
		}
		str = retObj.toObj().readStr ;
		this._location += str.length ;
	}
	else
	{
		if ( undefined != size )
		{
			str = this._read( size ) ;
		}
		else
		{
			str = this._read() ;
		}
	}
	return str ;
}

File.prototype.write = function( content ){

	if ( undefined != this._remote )
	{
		if ( true != this._isOpened )
		{
			setLastErrMsg( "file is not opened" ) ;
			throw SDB_IO ;
		}

		this._remote._runCommand( "file write", {}, {},
										  { "filename": this._filename,
										    "location": this._location,
										    "content": content } ) ;
		this._location += content.length ;
	}
	else
	{
		this._write( content ) ;
	}
}

File.prototype.seek = function( offset, where ) {
	if ( undefined == offset )
	{
		setLastErrMsg( "offset must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}
	if ( undefined == where )
	{
		where = 'b' ;
	}

	if ( undefined != this._remote )
	{
		if ( true != this._isOpened )
		{
			setLastErrMsg( "file is not opened" ) ;
			throw SDB_IO ;
		}

		if ( 'b' == where )
		{
			if( offset < 0 )
			{
				throw SDB_INVALIDARG ;
			}
			this._location = offset ;
		}
		else if ( 'c' == where )
		{
			if ( 0 > this._location + offset )
			{
				throw SDB_INVALIDARG ;
			}
			this._location += offset ;
		}
		else if ( 'e' == where )
		{
			var recvObj = this._remote._runCommand( "file get content size", {},
				                                     { "name": this._filename } ) ;
			var size = recvObj.toObj().size ;

			if ( 0 > size - 1 + offset )
			{
				throw SDB_INVALIDARG ;
			}
			this._location = size -1 + offset ;
		}
		else
		{
			setLastErrMsg( "where must be string(b/c/e)" ) ;
			throw SDB_INVALIDARG ;
		}
	}
	else
	{
		this._seek( offset, where ) ;
	}
}

File.prototype.close = function() {
   if ( undefined == this._remote )
   {
   	this._close() ;
   }
	else
	{
		this._isOpened = false ;
	}
}

File.prototype.remove = function( filepath ) {

	if ( undefined == filepath )
	{
		setLastErrMsg( "filepath must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != this._remote )
	{
		this._remote._runCommand( "file remove", {}, {},
			                      { "filepath" : filepath } ) ;
	}
	else
	{
		File.remove( filepath ) ;
	}
}

File.prototype.exist = function( filepath ) {
	var isExist ;

	if ( undefined == filepath )
	{
		setLastErrMsg( "filepath must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "file is exist", {}, {},
			                      					 { "filepath" : filepath } ) ;
		isExist = recvObj.toObj().isExist ;
	}
	else
	{
		isExist = File.exist( filepath ) ;
	}
	return isExist ;
}

File.prototype.copy = function( src, dst, replace, mode ) {
	if ( undefined == src )
	{
		setLastErrMsg( "src must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined == dst )
	{
		setLastErrMsg( "dst must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != mode )
	{
		if ( undefined != this._remote )
		{
	   	this._remote._runCommand( "file copy", { "replace": replace, "mode": mode },
	   		                      { "src": src }, { "dst": dst } ) ;
		}
		else
		{
			File.copy( src, dst, replace, mode ) ;
		}
	}
	else if ( undefined != replace )
	{
		if ( undefined != this._remote )
		{
			this._remote._runCommand( "file copy", { "replace": replace },
	   	                      	 { "src": src }, { "dst": dst } ) ;
		}
		else
		{
			File.copy( src, dst, replace ) ;
		}
	}
	else
	{
		if ( undefined != this._remote )
		{
			this._remote._runCommand( "file copy", {},
	   	                      	 { "src": src }, { "dst": dst } ) ;
		}
		else
		{
			File.copy( src, dst ) ;
		}
	}
}

File.prototype.move = function( src, dst ) {

	if ( undefined == src )
	{
		setLastErrMsg( "src must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined == dst )
	{
		setLastErrMsg( "dst must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != this._remote )
	{
		this._remote._runCommand( "file move", {}, { "src": src },
		                        { "dst": dst } ) ;
	}
	else
	{
		File.move( src, dst ) ;
	}
}

File.prototype.mkdir = function( name, mode ) {

	if ( undefined == name )
	{
		setLastErrMsg( "name must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != mode )
	{
		if ( undefined != this._remote )
		{
			this._remote._runCommand( "file mkdir", { "mode": mode }, {},
											 { "name": name } ) ;
		}
		else
		{
			File.mkdir( name, mode ) ;
		}
	}
	else
	{
		if ( undefined != this._remote )
		{
			this._remote._runCommand( "file mkdir", {}, {},
											 { "name": name } ) ;
		}
		else
		{
			File.mkdir( name ) ;
		}
	}
}

File.list = function( optionObj, filterObj ) {

	var retObj ;
	var objArr ;
	var result ;
	var displayMode = "obj" ;

	if ( undefined != optionObj )
	{
	   if ( undefined != optionObj.displayMode )
	   {
	   	displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;

	   }
		retObj = File._list( optionObj ) ;
	}
	else
	{
		retObj = File._list() ;
	}

	if ( undefined != filterObj )
	{
		var filter = new _Filter( filterObj ) ;
		objArr = filter.match( retObj.toObj() ) ;
	}
	else
	{
		var filter = new _Filter( {} ) ;
		objArr =  filter.match( retObj.toObj() ) ;
	}

	if ( "text" == displayMode )
	{
	   result = objArr._formatStr() ;
	}
	else
	{
		result = objArr ;
	}

	return result ;
}

File.isFile = function( pathname ) {
	if ( undefined == pathname )
	{
		setLastErrMsg( "pathname must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if( "FIL" == File._getPathType( pathname ) )
	{
		return true ;
	}
	else
	{
		return false ;
	}
}

File.isDir = function( pathname ) {
	if ( undefined == pathname )
	{
		setLastErrMsg( "pathname must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if( "DIR" == File._getPathType( pathname ) )
	{
		return true ;
	}
	else
	{
		return false ;
	}
}

File.find = function( optionObj, filterObj ) {
	var recvObj ;
	var retArr ;
	var result ;
	if ( undefined == optionObj )
	{
		setLastErrMsg( "optionObj must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	var displayMode = "obj" ;
	if ( undefined != optionObj.displayMode )
	{
		displayMode = optionObj.displayMode ;
		delete optionObj.displayMode ;
	}

	recvObj  = File._find( optionObj ) ;
	if ( undefined != filterObj )
	{
		var filter = new _Filter( filterObj ) ;
		retArr = filter.match( recvObj.toObj() ) ;
	}
	else
	{
		var filter = new _Filter( {} ) ;
		retArr = filter.match( recvObj.toObj() ) ;
	}

   if ( "text" == displayMode )
   {
   	result = retArr._formatStr() ;
   }
	else
	{
		result = retArr ;
	}
	return result ;
}

File.prototype.find = function( optionObj, filterObj ) {

	if ( undefined == optionObj )
	{
		setLastErrMsg( "optionObj must be config" ) ;
		throw SDB_OUT_OF_BOUND  ;
	}
	if ( false == optionObj instanceof Object )
	{
		setLastErrMsg( "optionObj must be Object" ) ;
		throw SDB_INVALIDARG  ;
	}

	var result ;
	if ( undefined != this._remote )
	{
		var matchResult ;
		var displayMode = "obj" ;
		if ( undefined != optionObj.displayMode )
		{
			displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;
		}

		var recvObj = this._remote._runCommand( "file find", optionObj ).toObj() ;
		if ( undefined != filterObj )
		{
			var filter = new _Filter( filterObj ) ;
			matchResult = filter.match( recvObj ) ;
		}
		else
		{
			var filter = new _Filter( {} ) ;
			matchResult = filter.match( recvObj ) ;
		}

		if ( "text" == displayMode )
		{
			result = matchResult._formatStr() ;
		}
		else
		{
			result = matchResult ;
		}
	}
	else
	{
		result = File.find( optionObj, filterObj ) ;
	}

	return result ;
}

File.prototype.chmod = function( filename, mode, recursive ) {

	if ( undefined != this._remote )
	{
		if ( undefined != recursive )
		{
			this._remote._runCommand( "file chmod", { "recursive": recursive }, { "pathname": filename },
				                      { "mode": mode } ) ;
		}
		else
		{
			this._remote._runCommand( "file chmod", {}, { "pathname": filename },
				                      { "mode": mode } ) ;
		}
	}
	else
	{
		if ( undefined != recursive )
		{
			File.chmod( filename, mode, recursive ) ;
		}
		else
		{
			File.chmod( filename, mode ) ;
		}
	}
}

File.prototype.toString = function()
{
	var result ;
	if ( undefined != this._remote )
	{
		if( undefined != this._filename )
		{
			result = this._filename ;
		}
		else
		{
			result = "" ;
		}
	}
	else
	{
		result = this._toString() ;
	}
	return result ;
}

File.prototype.chown = function( filename, optionObj, recursive ) {
	if ( undefined != this._remote )
	{
		if ( undefined != recursive )
		{
			this._remote._runCommand( "file chown", { "recursive": recursive }, { "filename": filename },
				                      optionObj ) ;
		}
		else
		{
			this._remote._runCommand( "file chown", {}, { "filename": filename },
				                      optionObj ) ;
		}
	}
	else
	{
		if ( undefined != recursive )
		{
			File.chown( filename, optionObj, recursive ) ;
		}
		else
		{
			File.chown( filename, optionObj ) ;
		}
	}
}

File.prototype.chgrp = function( filename, groupname, recursive ) {
	if ( undefined != this._remote )
	{
		if ( undefined != recursive )
		{
			this._remote._runCommand( "file chgrp", { "recursive": recursive }, { "filename": filename },
				                      { "groupname": groupname } ) ;
		}
		else
		{
			this._remote._runCommand( "file chgrp", {}, { "filename": filename },
				                      { "groupname": groupname } ) ;
		}
	}
	else
	{
		if ( undefined != recursive )
		{
			File.chgrp( filename, groupname, recursive ) ;
		}
		else
		{
			File.chgrp( filename, groupname ) ;
		}
	}
}

File.prototype.setUmask = function( mask ) {
	if ( undefined == mask )
	{
		setLastErrMsg( "mask must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	if ( undefined != this._remote )
	{
		this._remote._runCommand( "file set umask", {}, {}, { "mask": mask } ) ;
	}
	else
	{
		File.setUmask( mask ) ;
	}
}

File.prototype.getUmask = function() {
	var result ;
	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "file get umask" ) ;
		result = recvObj.toObj().mask ;
	}
	else
	{
		result = File.getUmask() ;
	}
	return result ;
}

File.prototype.list = function( optionObj, filterObj ) {
	var result ;
	if ( undefined == optionObj )
	{
		optionObj = {} ;
	}

	if ( undefined != this._remote )
	{
	   var displayMode = "obj" ;
		if ( undefined != optionObj.displayMode )
		{
			displayMode = optionObj.displayMode ;
			delete optionObj.displayMode ;
		}

		var recvObj = this._remote._runCommand( "file list", optionObj ).toObj() ;
		var matchResult ;
		if ( undefined != filterObj )
		{
			var filter = new _Filter( filterObj ) ;
			matchResult = filter.match( recvObj ) ;
		}
		else
		{
			var filter = new _Filter( {} ) ;
			matchResult = filter.match( recvObj ) ;
		}

		if ( "text" == displayMode )
		{
			result = matchResult._formatStr() ;
		}
		else
		{
			result = matchResult ;
		}
	}
	else
	{
		result = File.list( pathname, optionObj, filterObj, displayMode ) ;
	}
	return result ;
}

File.prototype.isFile = function( pathname ) {

	if ( undefined == pathname )
	{
		setLastErrMsg( "mask must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	var result = false ;
	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "file get path type", {},
			                                    { "pathname":pathname } ) ;
		if( "FIL" == recvObj.toObj().pathType )
		{
			result = true ;
		}
	}
	else
	{
		result = File.isFile( pathname ) ;
	}
	return result ;
}

File.prototype.isDir = function( pathname ) {

	if ( undefined == pathname )
	{
		setLastErrMsg( "mask must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	var result = false ;
	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "file get path type", {},
			                                    { "pathname":pathname } ) ;
		if( "DIR" == recvObj.toObj().pathType )
		{
			result = true ;
		}
	}
	else
	{
		result = File.isDir( pathname ) ;
	}
	return result ;
}

File.prototype.isEmptyDir = function( pathname ) {
	if ( undefined == pathname )
	{
		setLastErrMsg( "pathname must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	var result ;
	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "file is empty dir", {},
			                                    { "pathname":pathname } ) ;

		var isEmpty = recvObj.toObj().isEmpty ;
      if ( isEmpty )
      {
         result = true ;
      }
      else
      {
         result = false ;
      }
	}
	else
	{
		result = File.isDir( pathname ) ;
	}
	return result ;
}

File.prototype.stat = function( filename ) {

	if ( undefined == filename )
	{
		setLastErrMsg( "filename must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	var result ;
	if ( undefined != this._remote )
	{
		result = this._remote._runCommand( "file stat", {},
		                                  { "filename": filename } ) ;
	}
	else
	{
		result = File.stat( filename ) ;
	}
	return result ;
}

File.prototype.md5 = function( filename ) {
	if ( undefined == filename )
	{
		setLastErrMsg( "filename must be config" ) ;
		throw SDB_OUT_OF_BOUND ;
	}

	var result ;
	if ( undefined != this._remote )
	{
		var recvObj = this._remote._runCommand( "file md5", {},
		                                  	{ "filename": filename } ) ;
		result = recvObj.toObj().md5 ;
	}
	else
	{
		result = File.md5( filename ) ;
	}
	return result ;

}
// end File
