/* *****************************************************************************
@description: test whether the port has been used
@modify list:
   2014-7-26 Zhaobo Tan  Init
***************************************************************************** */
if ( typeof(PORT) == "undefined" ) {}
var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.Detail = "" ;
objRet.result = null ;

function main()
{
   try
   {
      // check arguments
      if ( typeof(PORT) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.Detail = "Invalid argument" ;
         return objRet ;
      }
      // test port status
//      var obj = System.sniffPort( PORT ) ;
//      objRet.result = eval( '(' + obj + ')' ) ;
      objRet.result = "************test for get port status****************" ;
      return objRet ;
   }
   catch ( e )
   {
      objRet.Rc = e ;
      objRet.Detail = getLastErrMsg() ;
      return objRet ;
   }
}

// execute
   main() ;

