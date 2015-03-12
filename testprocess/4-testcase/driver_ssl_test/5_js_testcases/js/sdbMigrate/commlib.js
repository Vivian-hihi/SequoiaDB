/*******************************************************************************
*@Description:common function for sdb import and export tool
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
*******************************************************************************/

var hostName = COORDHOSTNAME ;
var coordPort = COORDSVCNAME ;
var user = "lobtooltest" ;
var passwd = "lobtooltest" ;
var db = new SecureSdb( hostName, coordPort, user, passwd ) ;
var cmd = new Cmd() ;
var LocalPath = null ;
var InstallPath = null ;

// put easy data
function cmdInsert( cl, genNum, isHaveErr, record, isCheck )
{
   if( undefined == genNum ){ genNum = 1 ; }
   if( undefined == isHaveErr ){ isHaveErr = false ;}
   if( undefined == isCheck ){ isCheck = true ;}
   try
   {
      if( undefined == record )
      {
         var record = new Array( {"key":123}, {"key":3000000000}, {"key":123.456},
                                 {"key":123e+50}, {"key":"value"},
                                 {"key":{ "$oid":"123abcd00ef12358902300ef" }},
                                 {"key":true}, {"key":{ "$date":"2012-01-01" }},
                                 {"key":{ "$timestamp":"2012-01-01-13.14.26.124233" }},
                                 {"key":{"$binary":"aGVsbG8gd29ybGQ=", "$type":"1" }},
                                 {"key":{ "$regex":"^张", "$options":"i" }},
                                 {"key":{"subobj":"value"}},
                                 {"key":["abc",0,"def"]},
                                 {"key":null} ) ;
      }
      var errRecord = '{"key":{"子对象":"错误的字段记录"}' ;
      if( false == isHaveErr )
      {
         var loopTimes = parseInt( genNum/record.length ),
             addTimes = genNum%record.length ;
         //println( "loop times:" + loopTimes + " add times:" + addTimes ) ;
         for( var i = 0 ; i < loopTimes ; ++i )
         {
            for( var j = 0  ; j < record.length ; ++j )
            {
               //var errRecord = record[j] ;
               //println( "error record: " + JSON.stringify( errRecord ) ) ;
               cl.insert( record[j] ) ;
            }
         }
         for( var i = 0 ; i < addTimes ; ++i )
         {
            cl.insert( record[i] ) ;
         }
      }
      else
      {
         var loopTimes = parseInt( genNum/(record.length+1) ),
             addTimes = genNum%(record.length+1) ;
         for( var i = 0 ; i < loopTimes ; ++i )
         {
            cl.insert( errRecord ) ;
            for( var j = 0  ; j < record.length ; ++j )
            {
               cl.insert( record[j] ) ;
            }
         }
         for( var i = 0 ; i < addTimes ; ++i )
         {
            cl.insert( errRecord ) ;
            cl.insert( record[i] ) ;
         }
      }
      if( true == isCheck )
      {
         cnt = 0 ;
         while( genNum != cl.count() && 1000 > cnt )
         {
            cnt++ ;
            sleep(2) ;
         }
         if( genNum != cl.count() )
         {
            println( "expect record number: " + genNum +
                     ", actual record number: " + cl.count() ) ;
            throw "ErrRecordNumber" ;
         }
      }
   }
   catch( e )
   {
      println( "error record: " + JSON.stringify( errRecord ) ) ;
      println( "failed to insert data to collection, rc = " + e ) ;
      throw e ;
   }
}

// create cs and put data in database
function cmdPutData( cl, dataNum, field )
{
   if( undefined == dataNum ){ dataNum = 3; }
   try
   {
      if( undefined == field )
      {
         var field = {"key_int":123, "key_long":3000000000, "key_float1":123.456,
                      "key_float2":123e+50, "key_string":"value",
                      "key_oid":{ "$oid":"123abcd00ef12358902300ef" },
                      "key_bool":true, "key_date":{ "$date":"2012-01-01" },
                      "key_timestamp":{ "$timestamp":"2012-01-01-13.14.26.124233" },
                      "key_binary":{"$binary":"aGVsbG8gd29ybGQ=", "$type":"1" },
                      "key_regex":{ "$regex":"^张", "$options":"i" },
                      "key_obj":{"subobj":"value"},"key_array":[ "abc", 0, "def" ],
                      "key_null":null} ;
      }
      for( var i = 0 ; i < dataNum ; ++i )
      {
         cl.insert( field ) ;
      }
      var cnt = 0 ;
      do
      {
         ++cnt ;
         sleep( 3 ) ;
      }while( dataNum != cl.find( field ).count() && 1000 >= cnt ) ;
      if( dataNum != cl.find( field ).count() )
      {
         println( "Field===" + field ) ;
         println( "the number of records " + cl.find(field).count() +
                  "not equla : " + dataNum ) ;
         throw "ErrNumRecords" ;
      }
   }
   catch( e )
   {
      println( "failed to insert data to collection, rc = " + e ) ;
      throw e ;
   }
}

// command for tools that include sdbimprt and sdbexprt
function cmdToolRun( cmdType, cmdOption )
{
   if( undefined == cmdType || undefined == cmdOption )
      throw "pass wrong command line" ;
   try
   {
      var cmd = new Cmd() ;
      var cmdLine = cmdType + " " + cmdOption ;
      var cmdContent = cmd.run( cmdLine ) ;
   }
   catch( e )
   {
      println( "====>CMD LINE : " + cmdLine ) ;
      println( "failed to execute command for import or" +
               " export tools, rc = " + e ) ;
      throw e ;
   }
}

// export data from collection
function cmdExportData( db, exportFile, expType  )
{
   try
   {
      var executeCmd = InstallPath + "/bin/sdbexprt" ;
      var cs = db.getCS( COMMCSNAME ) ;
      var cl = cs.getCL( COMMCLNAME ) ;
      if( 0 == cl.count() )
         throw "collection have no record" ;
      var cmd = new Cmd() ;
      if( "json" == expType )
      {
         var cmdLine = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+" -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+exportFile+" --type json" ;
         var cmdRun = executeCmd + " " + cmdLine ;
         println( cmdRun) ;
         cmd.run( cmdRun ) ;
      }
      else if( "csv" == expType )
      {
         var cmdLine = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+" -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+exportFile+" --type csv --fields 'key_int,key_long,key_float1,key_float2,key_string,key_oid,key_bool,key_date,key_timestamp,key_binary,key_regex,key_obj,key_array,key_null'" ;
         var cmdRun = executeCmd + " " + cmdLine ;
         println( cmdRun) ;
         cmd.run( cmdRun ) ;

      }
      else
      {
         throw "specify error export type" ;
      }
      println( "succes to export " + expType + " data"  ) ;
   }
   catch( e )
   {
      println( "failed to export data in comlib, rc = " + e ) ;
      throw e ;
   }
}


// Get Global Variable
/******************************************************************************
*@Description:when run these testcase in sequoiadb or trunk fold that not
*               installed, get home fold.   <?how to get sequoiadb home fold?>
******************************************************************************/
function toolGetInstallPath( localPath )
{
   try
   {
      var folder = cmd.run( 'ls ' + localPath ).split( '\n' ) ;
      var fcnt = 0 ;
      for( var i = 0 ; i < folder.length ; ++i )
      {
         if( "bin" == folder[i] || "SequoiaDB" == folder[i] ||
             "testcase" == folder[i] )
         {
            fcnt++ ;
         }
      }
      if( 2 <= fcnt )
         InstallPath = localPath ;
      return InstallPath ;
   }
   catch( e )
   {
      println( "failed to get install path in source install, rc = " + rc  ) ;
      throw e ;
   }
}

/******************************************************************************
*@Description:initalize the global variable in the begninning.
******************************************************************************/
function initGlobalVar()
{
   try
   {
      var local = cmd.run( "pwd" ).split( "\n" ) ;
      LocalPath = local[0] ;
      try
      {
         var install = cmd.run( "sed -n '3p'  /etc/default/sequoiadb" ).split( "=" ) ;
         var installPath = install[1].split( "\n" ) ;
         InstallPath = installPath[0] ;
      }
      catch( e )
      {
         if( 2 == e )
            InstallPath = toolGetInstallPath( LocalPath ) ;
         else
            throw "failed to excute:sed -n '3p'  /etc/default/sequoiadb" ;
      }
      cmd.run( "mkdir -p " + InstallPath +
               "/testcases/hlt/js_testcases/js/sdbMigrate" ) ;    // create folder
      LocalPath = InstallPath + "/testcases/hlt/js_testcases/js/sdbMigrate" ;
   }
   catch( e )
   {
      println( "failed to get global variable:cmd/LocalPath/InstallPath" + e ) ;
      throw e ;
   }
}

/******************************************************************************
*@Description: auto generate data.
******************************************************************************/
function autoGenData( fileName, recordNum, isErrRecord )
{
   if( undefined == fileName ){ fileName = CSPREFIX + "_autoGen_file.json" ; }
   if( undefined == recordNum ){ recordNum = 200 ; }
   if( undefined == isErrRecord ){ isErrRecord = false ; }
   try
   {
      var cmd = new Cmd(),
          autoFile = LocalPath + "/autoGenData.sh",
          cmdLine = autoFile + " " + fileName + " " + recordNum +
                    " " + isErrRecord ;

      println( "#AUTOGEN DATA CMD: " + cmdLine ) ;
      //cmd.run( "rm -rf " + autoFile ) ;
      // chmod file
      cmd.run( "chmod 755 " + autoFile ) ;
      cmd.run( cmdLine ) ;
      var retValue = new Array( fileName, recordNum, isErrRecord ) ;
      return retValue ;
   }
   catch( e )
   {
      println( "failed to auto generate data, rc = " + e ) ;
      throw e ;
   }
}

function cmdVerifyData( srcCL, dstCL, key, haveRet )
{
   if( undefined != key )
      var queryCond = { key: { "$exist":1 } } ;
   else
      var queryCond = null ;
   if( undefined != haveRet ){ haveRet = false }
   try
   {
      var srcRecords = srcCL.find( queryCond ).toArray(),
          recordArr = new Array() ;
          dstRecords = dstCL.find( queryCond ).toArray() ;
      // check the number of them
      if( srcCL.find( queryCond ).count() != dstCL.find( queryCond ).count() )
      {
         println( "source records: " + srcCL.find( queryCond ).count() +
                  " not equal" +
                  "destnation records: " + dstCL.find( queryCond ).count() ) ;
         throw "RecordsNotEqual" ;
      }
      for( var i = 0 ; i < srcCL.find(queryCond).count() ; ++i )
      {
         for( var j = 0 ; j < dstCL.find(queryCond).count(); ++j )
         {
            if( srcRecords[i] == dstRecords[j] )
            {
               srcRecords.splice( i, 1 ) ;
               dstRecords.splice( i, 1 ) ;
            }
         }
      }
      // if srcCL equal with dstCL, the length will be 0
      if( 0 != srcRecords.length || 0 != dstRecords.length || true != haveRet )
      {
         println( "source records : " + srcRecords ) ;
         println( "destnation records : " + srcRecords ) ;
         throw "ErrSrcDstRecords" ;
      }
      else if( 0 != srcRecords.length )
      {
         println( "source records : " + srcRecords ) ;
         println( "destnation records : " + srcRecords ) ;
         throw "ErrSrcRecords" ;
      }
      else
      {
         return dstRecords ;
      }
   }
   catch( e )
   {
      println( "failed to verify records, rc = " + e ) ;
      throw e ;
   }
}

/*******************************************************************************
* @Description : construct nest object and will line feed[newline]. such as :
*                {
*                  a:
*                    {
*                      b:"nestObject"
*                    }
*                }
*******************************************************************************/
function migNestObject( nestLayer, totalLayer )
{
   if( undefined == nestLayer )
   {
      nestLayer = 31 ;
      totalLayer = 31 ;
   }
   if( undefined == totalLayer )
      totalLayer = nestLayer ;
   var nestRecord = "" ;
   var nestFieldVal = "Oh, My God !" ;
   var space = "" ;
   var spaceVar = "  " ;
   if( 0 == nestLayer )
   {
      nestRecord = '"' + nestFieldVal + '"' ;
      return nestRecord ;
   }
   else
   {
      var nestFunc = migNestObject( nestLayer - 1, totalLayer ) ;
      var cnt = totalLayer - nestLayer ;
      var nestFieldName = "nestField_" + cnt ;
      spaceBrace = nestSpace( cnt ) ;
      spaceField = nestSpace( cnt+1 ) ;
      nestRecord = '{\n' + spaceField + '"' +
                   nestFieldName + '": ' + nestFunc + '\n' + spaceBrace + '}' ;
      return nestRecord ;
   }
}

/*******************************************************************************
* @Description : construct nest two space, such as "  "
*******************************************************************************/
function nestSpace( number )
{
   var space = "  " ;
   var nospace = "" ;
   if( 0 == number )
      return nospace ;
   else if( 1 == number )
      return space ;
   else
   {
      var totalSpace = space + nestSpace( number - 1 ) ;
      return totalSpace;
   }
}
