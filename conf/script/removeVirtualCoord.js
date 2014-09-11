/*******************************************************************************

   Copyright (C) 2012-2014 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*******************************************************************************/
/*
@description: remove virtual coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(OMA_HOST_NAME) == "undefined" )
{
   OMA_HOST_NAME = "127.0.0.1" ;
}
if ( typeof(OMA_SVC_NAME)  == "undefined" )
{
   OMA_SVC_NAME = "11790" ;
}
if ( typeof(V_COORD_SVC_NAME)  == "undefined" )
{
}

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.detail = "" ;

function main()
{
   var oma = null ;
   try
   {
      // check argument
      if ( typeof(OMA_HOST_NAME) == "undefined" ||
           typeof(OMA_SVC_NAME) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "not specified sdbom's hostname or svcname" ;
         return objRet ;
      }
      if ( typeof(V_COORD_SVC_NAME) == "undefined" )
      {
         objRet.Rc = -6 ;
         objRet.detail = "not specified virtual coord's svcname" ;
         return objRet ;
      }
      // new oma object
      oma = new Oma( OMA_HOST_NAME, OMA_SVC_NAME ) ;
      
      // stop virtual coord
      oma.stopNode( V_COORD_SVC_NAME ) ;

      // remomve virtual coord
      oma.removeCoord( V_COORD_SVC_NAME ) ;
 
      // close connection
      oma.close() ;
      oma = null ;

      return objRet ;
   }
   catch ( e )
   {
      if ( null != oma )
      {
         oma.close() ;
      }
      if ( typeof(e) != "number" )
      {
         objRet.Rc = -10 ;
         objRet.detail = "system error" ;
      }
      else
      {
         var errMsg = "" ;
         objRet.Rc = e ;
         errMsg = getLastErrMsg() ;
         if ( "" != errMsg && null != errMsg && undefined != errMsg )
         {
            objRet.detail = eval( '(' + errMsg + ')' ) ;
         }
      }
      return objRet ;
   }
}

// execute
   main() ;

