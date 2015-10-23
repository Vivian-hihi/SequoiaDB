//动态加载模块文件
var resolveFun = function( files ){
   return {
      deps: function( $q, $rootScope ){
         var deferred = $q.defer();
         var dependencies = files ;
         var i = 0 ;
         function loadjs( fileName )
         {
            $.getScript( fileName, function(){
               ++i ;
               if( i == dependencies.length )
               {
                  $rootScope.$apply( function(){
                     deferred.resolve() ;
                  } ) ;
               }
               else
               {
                  loadjs( dependencies[i] ) ;
               }
            } ) ;
         }
         if( dependencies.length > 0 )
         {
            loadjs( dependencies[0] ) ;
         }
         return deferred.promise ;
      }
   }
} ;

//格式化
var sprintf = function( format )
{
	var len = arguments.length;
	var strLen = format.length ;
	var newStr = '' ;
	for( var i = 0, k = 1; i < strLen; ++i )
	{
		var chars = format.charAt( i ) ;
		if( chars == '\\' && ( i + 1 < strLen ) && format.charAt( i + 1 ) == '?' )
		{
			newStr += '?' ;
			++i ;
		}
		else if( chars == '?' && k < len )
		{
			newStr += ( '' + arguments[k] ) ;
			++k ;
		}
		else
		{
			newStr += chars ;
		}
	}
	return newStr ;
} ;

//保留多少位小数
function fixedNumber( x, num )
{
   var y = parseFloat( x );
   if( isNaN( y ) )
   {
      return x ;
   }
   var z = Math.pow( 10, num );
   y = Math.round( y * z ) / z ;
   return y ;
}

//字符串补位
function pad( num, n, chars )
{
   chars = ( typeof( chars ) == 'undefined' ? '0' : chars ) ;
   var len = num.toString().length;
   while( len < n )
   {
      num = '0' + num ;
      ++len ;
   }
   return num ;
}

//获取对象的属性数量
function getObjectSize( obj )
{
   var len = 0 ;
   if( typeof( obj ) == 'object' )
   {
      $.each( obj, function(){
         ++len ;
      } ) ;
   }
   return len ;
}

//格式化日期
function timeFormat( date, fmt )
{
   var o = {
      "M+": date.getMonth() + 1,
      "d+": date.getDate(),
      "h+": date.getHours(),
      "m+": date.getMinutes(),
      "s+": date.getSeconds(),
      "q+": Math.floor( ( date.getMonth() + 3 ) / 3 ),
      "S" : date.getMilliseconds()
   } ;
   if( /(y+)/.test( fmt ) )
   {
      fmt = fmt.replace( RegExp.$1, ( date.getFullYear() + "" ).substr( 4 - RegExp.$1.length ) ) ;
   }
   for ( var k in o )
   {
      if( new RegExp( "(" + k + ")" ).test( fmt ) )
      {
         fmt = fmt.replace( RegExp.$1, ( RegExp.$1.length == 1) ? ( o[k] ) : ( ( "00" + o[k] ).substr( ( "" + o[k] ).length ) ) ) ;
      }
   }
   return fmt ;
}

//删除两端空格
function trim( str )
{　　
   return str.replace( /(^\s*)|(\s*$)/g, '' ) ; 
}

//判断是不是数组
function isArray( object ) {
   //判断length属性是否是可枚举的 对于数组 将得到false
   return object && typeof( object ) === 'object' && typeof( object.length ) === 'number' &&
            typeof( object.splice ) === 'function' && !( object.propertyIsEnumerable( 'length' ) ) ;
}

//自动判断类型并转换
//hasQuotes 如果设置成true，那么如果带有 "xxx"，则转换成 xxxx 的字符串
function autoTypeConvert( val, hasQuotes )
{
   if( typeof( val ) == 'string' )
   {
      var valLen = val.length ;
      if( valLen > 0 )
      {
         if( hasQuotes == true )
         {
            if( valLen > 1 && val.charAt(0) == '"' && val.charAt(valLen - 1) == '"' )
            {
               return val.substr( 1, valLen - 2 ) ;
            }
         }
         if( val.toLowerCase() == 'null' )
         {
            val = null ;
         }
         else if( val.toLowerCase() == 'true' )
         {
            val = true ;
         }
         else if( val.toLowerCase() == 'false' )
         {
            val = false ;
         }
         else if( val == 'minKey' )
         {
            val = { '$minkey': 1 } ;
         }
         else if( val == 'maxKey' )
         {
            val = { '$maxkey': 1 } ;
         }
         else if( val == 'undefined' )
         {
            val = { '$undefined': 1 } ;
         }
         else if( !isNaN( val ) )
         {
            val = Number( val ) ;
         }
      }
   }
   return val ;
}

/*
   数组结构 -> json
   例:
   [
      { key: 'Object', type: 'Object', level: 0, isOpen: false, val: [
         { key: 'a', type: 'Auto', level: 1, isOpen: false, val: '123' },
         { key: 'b', type: 'Object', level: 1, isOpen: false, val: [
            { key: 'c', type: 'String', level: 2, isOpen: false, val: 'hello' },
            { key: 'd', type: 'Auto', level: 2, isOpen: false, val: 'true' }
         ] },
         { key: 'e', type: 'Array', level: 1, isOpen: false, val: [
            { key: '', type: 'Auto', level: 2, isOpen: false, val: '7' },
            { key: '', type: 'Auto', level: 2, isOpen: false, val: '8' },
            { key: '', type: 'Auto', level: 2, isOpen: false, val: '9' }
         ] },
      ] }
   ]
   转成
   {
      a : 123,
      b : {
        c: "hello",
        d: true
      }
      e: [ 7, 8, 9 ]
   }
*/
function array2Json( array, parentType )
{
   if( typeof( parentType ) == 'undefined' || ( parentType != 'Object' && parentType != 'Array' ) ) parentType = 'Object' ;
   var json ;
   if( parentType == 'Object' )
   {
      json = {} ;
   }
   else
   {
      json = [] ;
   }
   $.each( array, function( index, field ){
      if( field['type'] == 'Object' && field['level'] == 0 )
      {
         json = array2Json( field['val'], field['type'] ) ;
         return false ;
      }
      else if( field['type'] == 'Object' || field['type'] == 'Array' )
      {
         var val = field['val'] ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = array2Json( val, field['type'] ) ;
         }
         else
         {
            json.push( array2Json( val, field['type'] ) ) ;
         }
      }
      else if( field['type'] == 'Auto' )
      {
         var val = field['val'] ;
         val = autoTypeConvert( val ) ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = val ;
         }
         else
         {
            json.push( val ) ;
         }
      }
      else if( field['type'] == 'String' )
      {
         var val = field['val'] ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = val ;
         }
         else
         {
            json.push( val ) ;
         }
      }
      else if( field['type'] == 'Binary' )
      {
         var binary = field['val'] ;
         var binType = '' ;
         if( binary.length > 0 && binary.charAt(0) == '(' && binary.indexOf( ')' ) >= 0 )
         {
            var right = binary.indexOf( ')' ) ;
            binType = binary.substr( 1, right - 1 ) ;
            binary = binary.substr( right + 1 ) ;
         }
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = { '$binary': binary, '$type': binType } ;
         }
         else
         {
            json.push( { '$binary': binary, '$type': binType } ) ;
         }
      }
      else if( field['type'] == 'Timestamp' )
      {
         var val = field['val'] ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = { '$timestamp': val } ;
         }
         else
         {
            json.push( { '$timestamp': val } ) ;
         }
      }
      else if( field['type'] == 'Date' )
      {
         var val = field['val'] ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = { '$date': val } ;
         }
         else
         {
            json.push( { '$date': val } ) ;
         }
      }
      /*
      else if( field['type'] == 'Code' )
      {
         var val = field['val'] ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = { '$code': val } ;
         }
         else
         {
            json.push( { '$code': val } ) ;
         }
      }
      */
      else if( field['type'] == 'ObjectId' )
      {
         var val = field['val'] ;
         val = pad( val, 24, '0' ) ;
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = { '$oid': val } ;
         }
         else
         {
            json.push( { '$oid': val } ) ;
         }
      }
      else if( field['type'] == 'Regex' )
      {
         var regex = field['val'] ;
         var options = '' ;
         if( regex.charAt(0) == '/' && regex.indexOf( '/', 1 ) > 0 )
         {
            var right = regex.indexOf( '/', 1 ) ;
            options = regex.substr( right + 1 ) ;
            regex = regex.substr( 1, right - 1 ) ;
         }
         if( parentType == 'Object' )
         {
            json[ field['key'] ] = { '$regex': regex, '$options': options } ;
         }
         else
         {
            json.push( { '$regex': regex, '$options': options } ) ;
         }
      }
   } ) ;
   return json ;
}

/*
   json -> 数组结构
   例:
   {
      a : 123,
      b : {
        c: "hello",
        d: true
      }
      e: [ 7, 8, 9 ]
   }
   转成
   [
      { key: 'Object', type: 'Object', level: 0, isOpen: false, val: [
         { key: 'a', type: 'Auto', level: 1, isOpen: false, val: '123' },
         { key: 'b', type: 'Object', level: 1, isOpen: false, val: [
            { key: 'c', type: 'String', level: 2, isOpen: false, val: 'hello' },
            { key: 'd', type: 'Auto', level: 2, isOpen: false, val: 'true' }
         ] },
         { key: 'e', type: 'Array', level: 1, isOpen: false, val: [
            { key: '', type: 'Auto', level: 2, isOpen: false, val: '7' },
            { key: '', type: 'Auto', level: 2, isOpen: false, val: '8' },
            { key: '', type: 'Auto', level: 2, isOpen: false, val: '9' }
         ] },
      ] }
   ]
*/
function json2Array( json, level, exact )
{
   if( isNaN( level ) ) level = 0 ;
   if( typeof( exact ) == 'undefined' ) exact = false ;
   var array = [] ;
   if( level == 0 )
   {
      var child = json2Array( json, level + 1, exact ) ;
      array.push( { key: 'Object', type: 'Object', level: 0, isOpen: false, val: child } ) ;
      return array ;
   }
   $.each( json, function( key, value ){
      key = key + '' ;
      var valueType = typeof( value ) ;
      if( valueType == 'object' )
      {
         if( value == null )
         {
            value = 'null' ;
            valueType = 'Auto' ;
            if( exact == true )
            {
               valueType = 'Null' ;
            }
         }
         else if( isArray( value ) )
         {
            value = json2Array( value, level + 1, exact ) ;
            valueType = 'Array' ;
         }
         else if( typeof( value['$binary'] ) == 'string' && typeof( value['$type'] ) == 'string' )
         {
            var binary = value['$binary'] ;
            var binType = value['$type'] ;
            value = binary ;
            if( binType.length > 0 )
            {
                value = '(' + binType + ')' + value ;
            }
            valueType = 'Binary' ;
         }
         else if( typeof( value['$timestamp'] ) == 'string' )
         {
            value = value['$timestamp'] ;
            valueType = 'Timestamp' ;
         }
         else if( typeof( value['$date'] ) == 'string' )
         {
            value = value['$date'] ;
            valueType = 'Date' ;
         }
         /*
         else if( typeof( value['$code'] ) == 'string' )
         {
            value = value['$code'] ;
            valueType = 'Code' ;
         }
         */
         else if( typeof( value['$minKey'] ) == 'number' )
         {
            value = 'minKey' ;
            valueType = 'Auto' ;
            if( exact == true )
            {
               valueType = 'MinKey' ;
            }
         }
         else if( typeof( value['$maxKey'] ) == 'number')
         {
            value = 'maxKey' ;
            valueType = 'Auto' ;
            if( exact == true )
            {
               valueType = 'MaxKey' ;
            }
         }
         else if( typeof( value['$undefined'] ) == 'number' )
         {
            value = 'undefined' ;
            valueType = 'Auto' ;
            if( exact == true )
            {
               valueType = 'Undefined' ;
            }
         }
         else if( typeof( value['$oid'] ) == 'string' )
         {
            value = value['$oid'] ;
            value = pad( value, 24, '0' ) ;
            valueType = 'ObjectId' ;
         }
         else if( typeof( value['$regex'] ) == 'string' && typeof( value['$options'] ) == 'string' )
         {
            value = '/' + value['$regex'] + '/' + value['$options'] ;
            valueType = 'Regex' ;
         }
         else
         {
            value = json2Array( value, level + 1, exact ) ;
            valueType = 'Object' ;
         }
      }
      else if( valueType == 'boolean' )
      {
         value = ( value ? 'true' : 'false' ) ;
         valueType = 'Auto' ;
         if( exact == true )
         {
            valueType = 'Bool' ;
         }
      }
      else if (valueType == 'number')
      {
         value = value + '' ;
         valueType = 'Auto' ;
         if( exact == true )
         {
            valueType = 'Number' ;
         }
      }
      else if (valueType == 'string')
      {
         value = value ;
         valueType = 'String' ;
      }
      else
      {
         
      }
      array.push( { key: key, type: valueType, level: level, isOpen: false, val: value } ) ;
   } ) ;
   return array ;
}

//打印调试
function printfDebug( text )
{
   try
   {
      if( window.SdbDebug == true )
         console.warn( text ) ;
   }
   catch( e ){}
}

//带小数就进位
function numberCarry( num )
{
   var intNum = parseInt( num ) ;
   if( intNum != num )
   {
      num = intNum + 1 ;
   }
   return num ;
}

//获取操作系统信息
function getSystemInfo()
{
   var nAgt = navigator.userAgent;
   var os = 'unknown' ;
   var clientStrings = [
       { s: 'Windows 10', r: /(Windows 10.0|Windows NT 10.0)/ },
       { s: 'Windows 8.1', r: /(Windows 8.1|Windows NT 6.3)/ },
       { s: 'Windows 8', r: /(Windows 8|Windows NT 6.2)/ },
       { s: 'Windows 7', r: /(Windows 7|Windows NT 6.1)/ },
       { s: 'Windows Vista', r: /Windows NT 6.0/ },
       { s: 'Windows Server 2003', r: /Windows NT 5.2/ },
       { s: 'Windows XP', r: /(Windows NT 5.1|Windows XP)/ },
       { s: 'Windows 2000', r: /(Windows NT 5.0|Windows 2000)/ },
       { s: 'Windows ME', r: /(Win 9x 4.90|Windows ME)/ },
       { s: 'Windows 98', r: /(Windows 98|Win98)/ },
       { s: 'Windows 95', r: /(Windows 95|Win95|Windows_95)/ },
       { s: 'Windows NT 4.0', r: /(Windows NT 4.0|WinNT4.0|WinNT|Windows NT)/ },
       { s: 'Windows CE', r: /Windows CE/ },
       { s: 'Windows 3.11', r: /Win16/ },
       { s: 'Android', r: /Android/ },
       { s: 'Open BSD', r: /OpenBSD/ },
       { s: 'Sun OS', r: /SunOS/ },
       { s: 'Linux', r: /(Linux|X11)/ },
       { s: 'iOS', r: /(iPhone|iPad|iPod)/ },
       { s: 'Mac OS X', r: /Mac OS X/ },
       { s: 'Mac OS', r: /(MacPPC|MacIntel|Mac_PowerPC|Macintosh)/ },
       { s: 'QNX', r: /QNX/ },
       { s: 'UNIX', r: /UNIX/ },
       { s: 'BeOS', r: /BeOS/ },
       { s: 'OS/2', r: /OS\/2/ },
       { s: 'Search Bot', r: /(nuhk|Googlebot|Yammybot|Openbot|Slurp|MSNBot|Ask Jeeves\/Teoma|ia_archiver)/ }
   ];
   for (var id in clientStrings)
   {
      var cs = clientStrings[id];
      if (cs.r.test(nAgt))
      {
         os = cs.s;
         break;
      }
   }
   var osVersion = 'unknown';
   if (/Windows/.test(os)) {
      osVersion = /Windows (.*)/.exec(os)[1];
      os = 'Windows';
   }
   switch( os )
   {
   case 'Mac OS X':
      osVersion = /Mac OS X (10[\.\_\d]+)/.exec(nAgt)[1];
      break;
   case 'Android':
      osVersion = /Android ([\.\_\d]+)/.exec(nAgt)[1];
      break;
   case 'iOS':
      osVersion = /OS (\d+)_(\d+)_?(\d+)?/.exec(nVer);
      osVersion = osVersion[1] + '.' + osVersion[2] + '.' + (osVersion[3] | 0);
      break;
   }
   return [os, osVersion] ;
}