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
@description: create virtual coord
@modify list:
   2014-7-26 Zhaobo Tan  Init
*/
if ( typeof(OMA_HOST_NAME) == "undefined" ) {}
if ( typeof(OMA_SVC_NAME) == "undefined" ) {}
if ( typeof(V_COORD_SVC_NAME) == "undefined" )
{
   V_COORD_SVC_NAME = "10000" ;
}
if ( typeof(V_COORD_INSTALL_PATH) == "undefined" )
{
   V_COORD_INSTALL_PATH = "/tmp/omatmp/data/vCoord" ;
}
if ( typeof(V_COORD_CFG_OPTION) == "undefined" )
{
   V_COORD_CFG_OPTION = {} ;
}

var objRet = new Object() ;

objRet.Rc = 0 ;
objRet.detail = "" ;

function createVirtualCoord( oma )
{
   oma.createCoord( V_COORD_SVC_NAME,
                    V_COORD_INSTALL_PATH,
                    V_COORD_CFG_OPTION ) ;
}

function startVirtualCoord( oma )
{
   oma.startNode( V_COORD_SVC_NAME ) ;
}

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
         objRet.detail = "not specified host name or service name of omagent" ;
         return objRet ;
      }

      // new oma object
      oma = new Oma( OMA_HOST_NAME, OMA_SVC_NAME ) ;
print("111111111111\n") ;
      // create virtual coord
      createVirtualCoord( oma ) ;
 print("22222222222\n") ;
      // start virtual coord
      startVirtualCoord( oma ) ;
print("3333333333333333\n");
      // close connection
      oma.close() ;
      oma = null ;

      return objRet ;
   }
   catch ( e )
   {
print("44444444 e = " + e + "\n") ;
      if ( null != oma )
      {
         oma.removeNode( OMA_SVC_NAME ) ;
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

