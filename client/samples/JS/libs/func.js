/* *****************************************************************************
@description: JavaScript common function library 
@modify list:
   2014-2-24 Jianhui Xu  Init
***************************************************************************** */

// begin global variable configuration
// CSPREFIX, COORDSVCNAME, COORDHOSTNAME  is input parameter
// UUID, UUNAME is input parameter
if ( typeof(CSPREFIX) == "undefined" ) { CSPREFIX = "local_test"; }
if ( typeof(COORDSVCNAME) == "undefined" ) { COORDSVCNAME = "50000"; }
if ( typeof(COORDHOSTNAME) == "undefined" ) { COORDHOSTNAME = 'localhost'; }
if ( typeof(UUID) == "undefined" ) { UUID = 1 ; }
if ( typeof(UUNAME) == "undefined" ) { UUNAME = "ID"+UUID+"NAME" ; }

var CATASVCNAME = 30000 ;
var COMMCSNAME = CSPREFIX + "_cs" ;
var COMMCLNAME = CSPREFIX + "_cl" ;
var DATA_GROUP_ID_BEGIN = 1000 ;
var CATALOG_GROUPNAME = "SYSCatalogGroup" ;
var COORD_GROUPNAME = "SYSCoord" ;
var CATALOG_GROUPID = 1 ;
var COORD_GROUPID = 2 ;
// end global variable configuration

// control variable
var funcCommDropCSTimes = 0 ;
var funcCommCreateCSTimes = 0 ;
var funcCommCreateCLTimes = 0 ;
var funcCommCreateCLOptTimes =  0 ;
var funcCommDropCLTimes = 0 ;
// end control variable

function RSize( CSname ){
//there are many groups , and I want to know the max nodes' size in one group
this.ReplSize_groups = function( db )
{
   var RGname ;
   var size = 0 ;

   try
   {
      RGname = db.listReplicaGroups() ;
   }
   catch ( e )
   {
      if ( e != -159 )
      {
         println( "list replica groups failed: " + e ) ;
         throw e ;
      }
   }

   for( var i = 1 ; i < RGname.size() ; ++i )
   {
      var eRGname = eval("("+RGname[i]+")");

      if( size < eRGname["Group"].length )
      {
         size = eRGname["Group"].length ; 
      }
   }
   return size ;
} ,

//get the group' node_number
this.ReplSize_one_group = function( db , CSname )
{
   try
   {
      var catadb = new Sdb('localhost', CATASVCNAME ) ;
      var data = catadb.SYSCAT.SYSCOLLECTIONSPACES.find() ;
      for ( var i = 0 ; i < data.size() ; ++i  )
      {
         var data1 = eval("("+data[i]+")") ;
         if ( CSname == data1["Name"] )
         {
            var groupID = data1["Group"][0]["GroupID"] ;  
            break ;
         }
      }
   
      if( !(i < data.size()) )
      {
         return -1 ;
      }
   
      var RGname = db.listReplicaGroups() ;
      for ( var i = 1 ; i < RGname.size() ; ++i )
      {
         eRGname = eval("("+RGname[i]+")") ;
         if ( groupID == eRGname["GroupID"] )
         {
            return eRGname["Group"].length ;
         }
      }
   }
  catch( e )
  {
     println( "Get replica one group size failed: " + e ) ;
     return 1 ;
  }
} ,
// if you want to get the max nodes' number when there are groups,
//please enter "split" in the first var when you use this function()
this.ReplSize = function( db )
{
   return 0 ;
   /*
   //return this.ReplSize_groups( db ) ; 

   switch( arguments[0] )
   {
      case "split":
         return this.ReplSize_groups( db );
      break;
      default:
         return this.ReplSize_one_group( db , CSname );

      break;
   }
   */
}

}  //end RSize object

/* *****************************************************************************
@discretion: check database mode is standalone
@author: Jianhui Xu
***************************************************************************** */
function commIsStandalone( db )
{
   try
   {
      db.listReplicaGroups() ;
      return false ;
   }
   catch( e )
   {
      if( e == -159 )
      {
         return true ;
      }
      else
      {
         println("execute listReplicaGroups happen error , e =" +e ) ;
         throw e ;
      }
   }
}

/* *****************************************************************************
@discretion: create collection space
@author: Jianhui Xu
@parameter
   ignoreExisted: default = false, value: true/false
   message: user define message, default:""
   domain: create CS specify domain, default:"";[by  xiaojun Hu ]
           exp : {"Domain":"domName"}
***************************************************************************** */
function commCreateCS( db, csName, ignoreExisted, message, domain )
{
   ++funcCommCreateCSTimes ;
   if ( ignoreExisted == undefined ) { ignoreExisted = false ; }
   if ( message == undefined ) { message = "" ; }
   if ( domain == undefined ) { domain = "" ; }
   try
   {
      if( "" == domain )
         return db.createCS( csName ) ;
      else
         return db.createCS( csName, domain ) ;
   }
   catch ( e )
   {
      if ( e != -33 || !ignoreExisted )
      {
         println( "commCreateCS[" + funcCommCreateCSTimes + "] Create collection space[" + csName + "] failed: " + e + ", message: " + message ) ;
         throw e ;
      }
   }
   // get collection space object
   try
   {
      return db.getCS( csName ) ;
   }
   catch ( e )
   {
      println( "commCreateCS[" + funcCommCreateCSTimes + "] Get existed collection space[" + csName + "] failed: " + e + ",message: " + message  ) ;
      throw e ;
   }
}

/* *****************************************************************************
@discretion: create collection
@author: Jianhui Xu
@parameter
   replSize: default = RSize().ReplSize(), value 0, 1, 2,...
   compressed: default = true, value: true/false
   autoCreateCS: default = true, value: true/false
   ignoreExisted: default = false, value: true/false
   message: default = "", value: user defined message string
***************************************************************************** */
function commCreateCL( db, csName, clName, replSize, compressed, autoCreateCS, ignoreExisted, message )
{
   ++funcCommCreateCLTimes ;
   if ( replSize == undefined || replSize < 0 ) { replSize = new RSize( csName ).ReplSize( db ) ; }
   if ( compressed == undefined ) { compressed = true ; }
   if ( autoCreateCS == undefined ) { autoCreateCS = true ; }
   if ( ignoreExisted == undefined ) { ignoreExisted = false ; }
   if ( message == undefined ) { message = ""; }

   if ( autoCreateCS )
   {
      commCreateCS( db, csName, true, "commCreateCL auto to create collection space" ) ;
   }
   try
   {
      return eval( 'db.'+csName+'.createCL("' + clName + '", {"ReplSize":' + replSize +', "Compressed":' + compressed +'})' ) ;
   }
   catch( e )
   {
      if ( e != -22 || !ignoreExisted )
      {
         println( "commCreateCL[" + funcCommCreateCLTimes + "] create collection[" + csName + "." + clName + "] failed: " + e + ",message: " + message ) ;
         throw e ;
      }
   }
   //get collection
   try
   {
      return eval( 'db.' +csName+'.getCL("' +clName+ '")' ) ;
   }
   catch ( e )
   {
      println( "commCreateCL[" + funcCommCreateCLTimes + "] get collection[" + csName + "." + clName + "] failed: " + e + ",message: " + message ) ;
      throw e ;
   }
}

/* *****************************************************************************
@discretion: create collection by user option
@author: Jianhui Xu
@parameter
   optionObj: option object, default {}
   compressed: default = true, value: true/false
   autoCreateCS: default = true, value: true/false
   ignoreExisted: default = false, value: true/false
   message: default = "", value: user defined message string
***************************************************************************** */
function commCreateCLByOption( db, csName, clName, optionObj, autoCreateCS, ignoreExisted, message )
{
   ++funcCommCreateCLOptTimes ;
   if ( optionObj == undefined ) { optionObj = {} ; }
   if ( autoCreateCS == undefined ) { autoCreateCS = true ; }
   if ( ignoreExisted == undefined ) { ignoreExisted = false ; }
   if ( message == undefined ) { message = ""; }

   if ( typeof( optionObj ) != "object" )
   {
      throw "commCreateCLByOption: optionObj is not object" ;
   }
   var csObj ;
   if ( autoCreateCS )
   {
      csObj = commCreateCS( db, csName, true, "commCreateCLByOption auto to create collection space" ) ;
   }
   else
   {
      try
      {
         csObj = db.getCS( csName ) ;
      }
      catch( e )
      {
         println( "commCreateCLByOption[" + funcCommCreateCLOptTimes + "] get collection space[" + csName + "] failed: " + e ) ;
         throw e ;
      }
   }

   try
   {
      return csObj.createCL( clName, optionObj ) ;
   }
   catch( e )
   {
      if ( e != -22 || !ignoreExisted )
      {
         println( "commCreateCLByOption[" + funcCommCreateCLOptTimes + "] create collection[" + csName + "." + clName + "] failed: " + e + ",message: " + message ) ;
         throw e ;
      }
   }
   //get collection
   try
   {
      return eval( 'db.' +csName+'.getCL("' +clName+ '")' ) ;
   }
   catch ( e )
   {
      println( "commCreateCLByOption[" + funcCommCreateCLOptTimes + "] get collection[" + csName + "." + clName + "] failed: " + e + ",message: " + message ) ;
      throw e ;
   }
}

/* *****************************************************************************
@discretion: drop collection space
@author: Jianhui Xu
@parameter
   ignoreNotExist: default = true, value: true/false
   message: default = ""
***************************************************************************** */
function commDropCS( db, csName, ignoreNotExist, message )
{
   ++funcCommDropCSTimes ;
   if ( ignoreNotExist == undefined ) { ignoreNotExist = true ; }
   if ( message == undefined ) { message = "" ; }

   try
   {
      db.dropCS( csName ) ;
   }
   catch( e )
   {
      if ( e == -34 && ignoreNotExist )
      {
         // think right
      }
      else
      {
         println( "commDropCS[" + funcCommDropCSTimes + "] Drop collection space[" + csName + "] failed: " + e + ",message: " + message ) ;
         throw e ;
      }
   }
}

/* *****************************************************************************
@discretion: drop collection
@author: Jianhui Xu
@parameter
   ignoreCSNotExist: default = true, value: true/false
   ignoreCLNotExist: default = true, value: true/false
   message: default = ""
***************************************************************************** */
function commDropCL( db, csName, clName, ignoreCSNotExist, ignoreCLNotExist, message )
{
   ++funcCommDropCLTimes ;
   if ( message == undefined ) { message = ""; }
   if ( ignoreCSNotExist == undefined ) { ignoreCSNotExist = true ; }
   if ( ignoreCLNotExist == undefined ) { ignoreCLNotExist = true ; }

   try
   {
      eval( 'db.' +csName+ '.dropCL("' +clName+ '")' ) ;
   }
   catch( e )
   {
      if ( ( e == -34 && ignoreCSNotExist ) || ( e == -23 && ignoreCLNotExist ) )
      {
         // think right
      }
      else
      {
         println( "commDropCL[" + funcCommDropCLTimes + "] Drop collection[" + csName + "." + clName + "] failed: " + e + ",message: " + message ) ;
         throw e ;
      }
   }
}

/* *****************************************************************************
@discretion: create index
@author: Jianhui Xu
@parameter
   indexDef: index define object
   isUnique: true/false, default is false
   ignoreExist: default is false
***************************************************************************** */
function commCreateIndex( cl, name, indexDef, isUnique, ignoreExist )
{
   if ( isUnique == undefined ) { isUnique = false ; }
   if ( ignoreExist == undefined ) { ignoreExist = false ; }
   
   if ( typeof( indexDef ) != "object" )
   {
      throw "commCreateIndex: indexDef is not object" ;
   }

   try
   {
      cl.createIndex( name, indexDef, isUnique ) ;
   }
   catch( e )
   {
      println( "commCreateIndex: create index[" + name + "] failed: " + e ) ;
      if ( ignoreExist && ( e == -46 || e == -247 ) )
      {
         // ok
      }
      else
      {
         throw e ;
      }
   }
}

function commDropIndex( cl, name, ignoreNotExist )
{
   if ( ignoreNotExist == undefined ) { ignoreNotExist = false ; }
   
   try
   {
      cl.dropIndex( name ) ;
   }
   catch( e )
   {
      println( "commDropIndex: drop index[" + name + "] failed: " + e ) ;
      if ( ignoreNotExist && e == -47 )
      {
         // ok
      }
      else
      {
         throw e ;
      }
   }
}

/* *****************************************************************************
@discretion: check index
@author: Jianhui Xu
@parameter
   exist: true/false, if true check index exist, else check index not exist, default is true
   timeout: default 10 secs
***************************************************************************** */
function commCheckIndex( cl, name, exist, timeout )
{
   if ( exist == undefined ) { exist = true ; }
   if ( timeout == undefined ) { timeout = 10 ;}

   var timecount = 0 ;
   while ( true )
   {
      try
      {
         var tmpInfo = cl.getIndex( name ) ;
         if ( ( exist && tmpInfo == undefined ) ||
              ( !exist && tmpInfo != undefined ) )
         {
            if ( timecount < timeout )
            {
               ++timecount ;
               sleep( 1000 ) ;
               continue ;
            }
            println( "commCheckIndex: check index[" + name + "] time out" ) ;
            throw "check index time out" ;
         }

         if ( tmpInfo != undefined )
         {
            var tmpObj =  eval( "(" + tmpInfo.toString() + ")" ) ;
            if ( tmpObj.IndexDef.name != name )
            {
               println("commCheckIndex: get index name[" + tmpObj.IndexDef.name + "] is not the same with name[" + name + "]" ) ;
               println( tmpInfo ) ;
               throw "check name error" ;
            }
         }
         break ;
      }
      catch( e )
      {
         println( "commCheckIndex: get index[" + name + "] failed: " + e ) ;
         throw "get index failed" ;
      }
   }
}

/* *****************************************************************************
@discretion: check whether enable transaction function
@author: Jianhui Xu
***************************************************************************** */
function commIsTransEnabled( db )
{
   var isTrans = false ;
   var COMMTMPCS = COMMCSNAME + "_tmp" ;
   var COMMTMPCL = COMMCLNAME + "_tmp" ;
   var tmpCL = commCreateCL( db, COMMTMPCS, COMMTMPCL, 1, false, true, true, "Judge transaction create collection" ) ;
   try
   {
      db.transBegin() ;
      tmpCL.update( {$unset:{a:1}}, {a:{$exists:0}} ) ; // do nothing
      isTrans = true ;
   }
   catch( e )
   {
      if ( e == -253 )
      {
      }
      else
      {
         println( "execute transBegin happen error, e = " + e ) ;
      }
   }
   // clear when all use cases finished
   return isTrans ;
}

/* *****************************************************************************
@discretion: print object function
@author: Jianhui Xu
***************************************************************************** */
function commPrintIndent( str, deep, withRN )
{
   if ( undefined == deep ) { deep = 0 ; }
   if ( undefined == withRN ) { withRN = true ; }
   for ( var i = 0 ; i < 3*deep; ++i )
   {
      print( " " ) ;
   }
   if ( withRN )
   {
      println( str ) ;
   }
   else
   {
      print( str ) ;
   }
}

function commPrint( obj, deep )
{
   var isArray = false ;
   if ( typeof( obj ) != "object" )
   {
      println( obj ) ;
      return ;
   }
   if ( undefined == deep ) { deep = 0 ; }
   if ( typeof( obj.length ) == "number" ) { isArray = true ; }

   if ( 0 == deep )
   {
      if ( !isArray )
      {
         commPrintIndent( "{", deep, false ) ;
      }
      else
      {
         commPrintIndent( "[", deep, false ) ;
      }
   }

   var i = 0 ;
   for ( var p in obj )
   {
      if ( i == 0 ) { commPrintIndent( "", 0, true ); }
      else if ( i > 0 ) { commPrintIndent( ",", 0, true ) ; }
      ++i ;

      if ( typeof( obj[p] ) == "object" )
      {
         if ( typeof( obj[p].length ) == "undefined" )
         {
            if ( !isArray )
            {
               commPrintIndent( p + ": {", deep + 1, false ) ;
            }
            else
            {
               commPrintIndent( "{", deep + 1, false ) ;
            }
         }
         else if ( !isArray )
         {
            commPrintIndent( p + ": [", deep + 1, false ) ;
         }
         else
         {
            commPrintIndent( "[", deep + 1, false ) ;
         }
         commPrint( obj[p], deep + 1 ) ;
      }
      else if ( typeof( obj[p] ) == "function" )
      {
         // not print
      }
      else if ( typeof( obj[p] ) == "string" )
      {
         if ( !isArray )
         {
            commPrintIndent( p + ': "' + obj[p] + '"', deep + 1, false ) ;
         }
         else
         {
            commPrintIndent( '"' + obj[p] + '"', deep + 1, false ) ;
         }
      }
      else
      {
         if ( !isArray )
         {
            commPrintIndent( p + ': ' + obj[p], deep + 1, false ) ;
         }
         else
         {
            commPrintIndent( obj[p], deep + 1, false ) ;
         }
      }
   }
   if ( i == 0 )
   {
      if ( !isArray )
      {
         commPrintIndent( "}", 0, false ) ;
      }
      else
      {
         commPrintIndent( "]", 0, false ) ;
      }
   }
   else
   {
      commPrintIndent( "", 0, true ) ;
      if ( !isArray )
      {
         commPrintIndent( "}", deep, false ) ;
      }
      else
      {
         commPrintIndent( "]", deep ,false ) ;
      }
   }
   if ( deep == 0 ) { commPrintIndent( "", deep, true ) } ;
}

/*******************************************************************************
@description : get collection groups
@author : xiaojun Hu
@parameter:
   clname: collection name, such as : "foo.bar"
@return array[] ex:
   array[0] group1
   ...
***************************************************************************** */
function commGetCLGroups( db, clName )
{
   if ( typeof(clName) != "string" || clName.length == 0 )
   {
      throw "commGetCLGroups: Invalid clName parameter or clName is empty" ;
   }
   if ( commIsStandalone( db ) )
   {
      return new Array() ;
   }

   var tmpArray = new Array() ;
   var tmpInfo ;
   try
   {
      tmpInfo = db.snapshot(8).toArray() ;
   }
   catch( e )
   {
      println( "commGetCLGroups: snapshot collection space failed: " + e ) ;
      throw e ;
   }
   for ( var i = 0 ; i < tmpInfo.length ; ++i )
   {
      var tmpObj = eval( "(" + tmpInfo[i] + ")" ) ;
      if ( tmpObj.Name != clName )
      {
         continue ;
      }
      for ( var j = 0 ; j < tmpObj.CataInfo.length; ++j )
      {
         tmpArray.push( tmpObj.CataInfo[j].GroupName ) ;
      }
   }
   return tmpArray ;
}

/* *****************************************************************************
@discretion: get collection space groups
@author: Jianhui Xu
@parameter:
   csname: collection space name
@return array[] ex:
   array[0] group1
   ...
***************************************************************************** */
function commGetCSGroups( db, csname )
{
   if ( typeof(csname) != "string" || csname.length == 0 )
   {
      throw "commGetCSGroups: Invalid csname parameter or csname is empty" ;
   }
   if ( commIsStandalone( db ) )
   {
      return new Array() ;
   }

   var tmpArray = new Array() ;
   var tmpInfo ;
   try
   {
      tmpInfo = db.snapshot(5).toArray() ;
   }
   catch( e )
   {
      println( "commGetCSGroups: snapshot collection space failed: " + e ) ;
      throw e ;
   }

   for ( var i = 0 ; i < tmpInfo.length; ++i )
   {
      var tmpObj = eval( "(" + tmpInfo[i] + ")" ) ;
      if ( tmpObj.Name != csname )
      {
         continue ;
      }
      for ( var j = 0 ; j < tmpObj.Group.length; ++j )
      {
         tmpArray.push( tmpObj.Group[j] ) ;
      }
   }
   return tmpArray ;
}

/* *****************************************************************************
@discretion: get all groups
@author: Jianhui Xu
@parameter:
   filter: group name filter
   exceptCata : default true
   exceptCoord: default true
@return array[][] ex:
        [0]
           [0] {"GroupName":"XXXX", "GroupID":XXXX, "Status":XX, "Version":XX, "Role":XX, "PrimaryNode":XXXX, "Length":XXXX, "PrimaryPos":XXXX}
           [1] {"HostName":"XXXX", "dbpath":"XXXX", "svcname":"XXXX", "NodeID":XXXX}
           [N] ...
        [N]
           ...
***************************************************************************** */
function commGetGroups( db, print, filter, exceptCata, exceptCoord )
{
   if ( filter == undefined ) { filter = "" ; }
   if ( undefined == print ) { print = false ; }
   if ( undefined == exceptCata ) { exceptCata = true ; }
   if ( undefined == exceptCoord ) { exceptCoord = true ;}

   var tmpArray = new Array() ;
   var tmpInfo ;
   try
   {
      tmpInfo = db.listReplicaGroups().toArray() ;
   }
   catch( e )
   {
      if ( e == -159 )
      {
         return tmpArray ;
      }
      else
      {
         if( true == print )
            println( "commGetGroups failed: " + e ) ;
         throw e ;
      }
   }

   var index = 0 ;
   for ( var i = 0 ; i < tmpInfo.length; ++i )
   {
      var tmpObj = eval( "(" + tmpInfo[i] + ")" ) ;
      if ( filter.length != 0 && tmpObj.GroupName.indexOf( filter, 0 ) == -1 )
      {
         continue ;
      }
      if ( true == exceptCata && tmpObj.GroupID == CATALOG_GROUPID )
      {
         continue ;
      }
      if ( true == exceptCoord && tmpObj.GroupID == COORD_GROUPID )
      {
         continue ;
      }
      tmpArray[index] = Array() ;
      tmpArray[index][0] = new Object() ;
      tmpArray[index][0].GroupName = tmpObj.GroupName ;         // GroupName
      tmpArray[index][0].GroupID = tmpObj.GroupID ;             // GroupID
      tmpArray[index][0].Status = tmpObj.Status ;               // Status
      tmpArray[index][0].Version = tmpObj.Version ;             // Version
      tmpArray[index][0].Role = tmpObj.Role ;                   // Role
      if ( typeof(tmpObj.PrimaryNode) == "undefined" )
      {
         tmpArray[index][0].PrimaryNode = -1 ;
      }
      else
      {
         tmpArray[index][0].PrimaryNode = tmpObj.PrimaryNode ;     // PrimaryNode
      }
      tmpArray[index][0].PrimaryPos = 0 ;                       // PrimaryPos

      var tmpGroupObj = tmpObj.Group ;
      tmpArray[index][0].Length = tmpGroupObj.length ;          // Length

      for ( var j = 0 ; j < tmpGroupObj.length; ++j )
      {
         var tmpNodeObj = tmpGroupObj[j] ;
         tmpArray[index][j+1] = new Object() ;
         tmpArray[index][j+1].HostName = tmpNodeObj.HostName ;           // HostName
         tmpArray[index][j+1].dbpath = tmpNodeObj.dbpath ;               // dbpath
         tmpArray[index][j+1].svcname = tmpNodeObj.Service[0].Name ;     // svcname
         tmpArray[index][j+1].NodeID = tmpNodeObj.NodeID ;

         if ( tmpNodeObj.NodeID == tmpObj.PrimaryNode )
         {
            tmpArray[index][0].PrimaryPos = j+1 ;                        // PrimaryPos
         }
      }
      ++index ;
   }

   return tmpArray ;
}

/* *****************************************************************************
@discretion: get cs and cl
@author: Jianhui Xu
@parameter:
   csfilter : collection space filter
   clfilter : collection filter
@return array[] ex:
        [0] {"cs":"XXXX", "cl":["XXXX", "XXXX",...] }
        [N] ...
***************************************************************************** */
function commGetCSCL( db, csfilter, clfilter )
{
   if ( csfilter == undefined ) { csfilter = ""; }
   if ( clfilter == undefined ) { clfilter = ""; }

   var tmpCSCL = new Array() ;
   var tmpInfo ;
   try
   {
      tmpInfo = db.snapshot( 5 ).toArray() ;
   }
   catch( e )
   {
      println( "commGetCSCL failed: " + e ) ;
      return tmpCSCL ;
   }
   
   var m = 0 ;
   for ( var i = 0 ; i < tmpInfo.length; ++i )
   {
      var tmpObj = eval( "(" + tmpInfo[i] + ")" ) ;
      if ( csfilter.length != 0 && tmpObj.Name.indexOf( csfilter, 0 ) == -1 )
      {
         continue ;
      }
      tmpCSCL[ m ] = new Object() ;
      tmpCSCL[ m ].cs = tmpObj.Name ;
      tmpCSCL[ m ].cl = new Array() ;
      
      var tmpCollection = tmpObj.Collection ;
      for ( var j = 0 ; j < tmpCollection.length; ++j )
      {
         if ( clfilter.length != 0 && tmpCollection[ j ].Name.indexOf( clfilter, 0 ) == -1 )
         {
            continue ;
         }
         tmpCSCL[ m ].cl.push( tmpCollection[ j ].Name ) ;
      }
      ++m ;
   }
   return tmpCSCL ;
}

/* *****************************************************************************
@discretion: get backup
@author: Jianhui Xu
@parameter:
   filter : backup name filter
   path : backup path
   isSubDir: true/false, default is false
   condObj : condition object
   grpNameArray : group name array
@return array[] ex:
        [0] "XXXX"
        [N] ...
***************************************************************************** */
function commGetBackups( db, filter, path, isSubDir, cond, grpNameArray )
{
   if ( filter == undefined ) { filter = "" ; }
   if ( path == undefined ) { path = "" ; }
   if ( isSubDir == undefined ) { isSubDir = false ; }
   if ( cond == undefined ) { cond = {} ; }
   if ( grpNameArray == undefined ) { grpNameArray = new Array() ; }

   if ( typeof( cond ) != "object" )
   {
      throw "commGetBackups: cond is not a object" ;
   }
   if ( typeof( grpNameArray ) != "object" )
   {
      throw "commGetBackups: grpNameArray is not a array" ;
   }

   var tmpBackup = new Array() ;
   var tmpInfo ;
   try
   {
      if ( path.length == 0 )
      {
         tmpInfo = db.listBackup({GroupName:grpNameArray}).toArray() ;
      }
      else
      {
         tmpInfo = db.listBackup( {Path:path, IsSubDir:isSubDir, GroupName:grpNameArray} ).toArray() ;
      }
   }
   catch( e )
   {
      println( "commGetBackups failed: " + e ) ;
      return tmpBackup ;
   }

   for ( var i = 0 ; i < tmpInfo.length; ++i )
   {
      var tmpBackupObj = eval( "(" + tmpInfo[i] + ")" ) ;
      if ( typeof( tmpBackupObj.Name ) == "undefined" )
      {
         continue ;
      }
      if ( filter.length != 0 && tmpBackupObj.Name.indexOf( filter, 0 ) == -1 )
      {
         continue ;
      }
      var exist = false ;
      for ( var j = 0 ; j < tmpBackup.length; ++j )
      {
         if ( tmpBackupObj.Name == tmpBackup[j] )
         {
            exist = true ;
            break ;
         }
      }
      if ( exist )
      {
         continue ;
      }
      tmpBackup.push( tmpBackupObj.Name ) ;
   }
   return tmpBackup ;
}

/* *****************************************************************************
@discretion: get procedure
@author: Jianhui Xu
@parameter:
   filter : procedure name filter
@return array[] ex:
        [0] "XXXX"
        [N] ...
***************************************************************************** */
function commGetProcedures( db, filter )
{
   if ( filter == undefined ) { filter = "" ; }

   var tmpProcedure = new Array() ;
   var tmpInfo ;
   try
   {
      tmpInfo = db.listProcedures().toArray() ;
   }
   catch( e )
   {
      println( "commGetProcedures failed: " + e ) ;
      return tmpProcedure ;
   }

   for ( var i = 0 ; i < tmpInfo.length; ++i )
   {
      var tmpProcedureObj = eval( "(" + tmpInfo[i] + ")" ) ;
      if ( typeof( tmpProcedureObj.name ) == "undefined" )
      {
         continue ;
      }
      if ( filter.length != 0 && tmpProcedureObj.name.indexOf( filter, 0 ) == -1 )
      {
         continue ;
      }
      tmpProcedure.push( tmpProcedureObj.name ) ;
   }
   return tmpProcedure ;
}

/* *****************************************************************************
@discretion: check nodes function
@author: Jianhui Xu
@return array[] ex:
        [0] {"HostName":"XXXX", "dbpath":"XXXX", "svcname":"XXXX", "NodeID":XXXX}
        [N] ...
***************************************************************************** */
function commCheckNodes( groups )
{
   var tmpFailedGrp = new Array() ;
   for ( var i = 0 ; i < groups.length; ++i )
   {
      for ( var j = 1; j < groups[i].length; ++j )
      {
         var tmpDB ;
         try
         {
            tmpDB = new Sdb( groups[i][j].HostName, groups[i][j].svcname ) ;
            tmpDB.close() ;
         }
         catch ( e )
         {
            tmpFailedGrp.push( groups[i][j] ) ;
         }
      }
   }
   return tmpFailedGrp ;
}

/* *****************************************************************************
@discretion: check business right function
@author: Jianhui Xu
@return array[][] ex:
        [0]
           [0] {"GroupName":"XXXX", "GroupID":XXXX, "PrimaryNode":XXXX, "ConnCheck":t/f, "PrimaryCheck":t/f, "LSNCheck":t/f, "ServiceCheck":t/f, "DiskCheck":t/f }
           [1] {"HostName":"XXXX", "svcname":"XXXX", "NodeID":XXXX, "Connect":t/f, "IsPrimay":t/f, "LSN":XXXX, "ServiceStatus":t/f, "FreeSpace":XXXX }
           [N] ...
        [N]
           ...
***************************************************************************** */
function commCheckBusiness( groups, checkLSN, diskThreshold )
{
   if ( checkLSN == undefined ) { checkLSN = false ;}
   if ( diskThreshold == undefined ) { diskThreshold = 134217728 ; }
   var tmpCheckGrp = new Array() ;
   var grpindex = 0 ;
   var ndindex = 1 ;
   var error = false ;

   for ( var i = 0 ; i < groups.length; ++i )
   {
      error = false ;

      tmpCheckGrp[ grpindex ] = new Array() ;
      tmpCheckGrp[ grpindex ][0] = new Object() ;
      tmpCheckGrp[ grpindex ][0].GroupName = groups[i][0].GroupName ;
      tmpCheckGrp[ grpindex ][0].GroupID = groups[i][0].GroupID ;
      tmpCheckGrp[ grpindex ][0].PrimaryNode = groups[i][0].PrimaryNode ;
      tmpCheckGrp[ grpindex ][0].ConnCheck = true ;
      tmpCheckGrp[ grpindex ][0].PrimaryCheck = true ;
      tmpCheckGrp[ grpindex ][0].LSNCheck = true ;
      tmpCheckGrp[ grpindex ][0].ServiceCheck = true ;
      tmpCheckGrp[ grpindex ][0].DiskCheck = true ;

      // if no primary
      if ( groups[i][0].PrimaryNode == -1 )
      {
         error = true ;
         tmpCheckGrp[ grpindex ][0].PrimaryCheck = false ;
      }
      
      for ( var j = 1; j < groups[i].length; ++j )
      {
         ndindex = j ;
         tmpCheckGrp[grpindex][ndindex] = new Object() ;
         tmpCheckGrp[grpindex][ndindex].HostName = groups[i][j].HostName ;
         tmpCheckGrp[grpindex][ndindex].svcname = groups[i][j].svcname ;
         tmpCheckGrp[grpindex][ndindex].NodeID = groups[i][j].NodeID ;
         tmpCheckGrp[grpindex][ndindex].Connect = true ;
         tmpCheckGrp[grpindex][ndindex].IsPrimay = false ;
         tmpCheckGrp[grpindex][ndindex].LSN = -1 ;
         tmpCheckGrp[grpindex][ndindex].ServiceStatus = true ;
         tmpCheckGrp[grpindex][ndindex].FreeSpace = -1 ;

         var tmpDB ;
         // check connection
         try
         {
            tmpDB = new Sdb( groups[i][j].HostName, groups[i][j].svcname ) ;
         }
         catch ( e )
         {
            error = true ;
            tmpCheckGrp[ grpindex ][0].ConnCheck = false ;
            tmpCheckGrp[grpindex][ndindex].Connect = false ;
            continue ;
         }
         // check primary and lsn
         var tmpSysInfo ;
         try
         {
            tmpSysInfo = tmpDB.snapshot( 6 ).toArray() ;
         }
         catch( e )
         {
            error = true ;
            tmpCheckGrp[ grpindex ][0].ConnCheck = false ;
            tmpCheckGrp[grpindex][ndindex].Connect = false ;
            tmpDB.close() ;
            continue ;
         }
         //delete tmpCheckGrp[grpindex][ndindex].Connect ;
         // check primary
         var tmpSysInfoObj = eval( "(" + tmpSysInfo[0] + ")" ) ;
         if ( tmpSysInfoObj.IsPrimary == true )
         {
            tmpCheckGrp[grpindex][ndindex].IsPrimay = true ;
            if ( groups[i][0].PrimaryNode != groups[i][j].NodeID )
            {
               error = true ;
               tmpCheckGrp[ grpindex ][0].PrimaryCheck = false ;
            }
         }
         else
         {
            if ( groups[i][0].PrimaryNode == groups[i][j].NodeID )
            {
               error = true ;
               tmpCheckGrp[ grpindex ][0].PrimaryCheck = false ;
            }
            //delete tmpCheckGrp[grpindex][ndindex].IsPrimay ;
         }
         // check ServiceStatus
         if ( tmpSysInfoObj.ServiceStatus == false )
         {
            error = true ;
            tmpCheckGrp[ grpindex ][ndindex].ServiceStatus = false ;
            tmpCheckGrp[ grpindex ][0].ServiceCheck = false ;
         }
         // check disk
         tmpCheckGrp[grpindex][ndindex].FreeSpace = tmpSysInfoObj.Disk.FreeSpace ;
         if ( tmpCheckGrp[grpindex][ndindex].FreeSpace < diskThreshold )
         {
            error = true ;
            tmpCheckGrp[ grpindex ][0].DiskCheck = false ;
         }
         // check LSN
         tmpCheckGrp[ grpindex ][ndindex].LSN = tmpSysInfoObj.CurrentLSN.Offset ;
         if ( checkLSN && j > 1 && tmpCheckGrp[ grpindex ][ndindex].LSN != tmpCheckGrp[ grpindex ][1].LSN  )
         {
            error = true ;
            tmpCheckGrp[ grpindex ][0].LSNCheck = false ;
         }
         
         tmpDB.close() ;
      }
      
      if ( error == true )
      {
         ++grpindex ;
      }
   }
   // no error
   if ( error == false )
   {
      if ( grpindex == 0 )
      {
         tmpCheckGrp = new Array() ;
      }
      else
      {
         delete tmpCheckGrp[grpindex] ;
      }
   }
   return tmpCheckGrp ;
}


//example
//var db = new Sdb(COORDHOSTNAME,COORDSVCNAME ) ;

//var aa = new RSize( 'CSname' );
//println(aa.ReplSize());

