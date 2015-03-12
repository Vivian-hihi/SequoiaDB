/*******************************************************************************
*@Description : import record that the size like : [(32M-2048B)+(>2048B)].
*                                                    ---------   ------
*                                                   can import   cannot import
*               [SEQUOIADBMAINSTREAM-489]
*@Modify list :
*               2014-11-10  xiaojun Hu  Change
*******************************************************************************/

function GenJsonRecord( filename )
{
   if( undefined == filename )
   {
      return "ERROR" ;
   }
   // 5461 * 6144 = 33552384[32M-2048]
   var file = new File( filename ) ;
   var lineNum = 5461 ;
   var recordSize = 6144 ;   // size more than 2048
   var fixChar = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890#_" ;
   var fixRecord = '{"fieldLarge":""}' ;
   var rcrdValLen = recordSize - fixRecord.length -1 ; // record value length
   var recordValue = "" ;
   for( var i = 0 ; i < parseInt(rcrdValLen/fixChar.length) ; ++i )
   {
      recordValue += fixChar ;
   }
   for( var i = 0 ; i < parseInt(rcrdValLen%fixChar.length) ; ++i )
   {
      recordValue += fixChar.charAt(i) ;
   }
   var conRecord = '{"fieldLarge":"' + recordValue + '"}' ;  // complete record
   for( var i = 0 ; i < lineNum ; ++i )
   {
      file.write( conRecord + "\n" ) ;
   }
   // add the last record
   recordValue = "" ;
   conRecord = "" ;
   // when the size of last record equal 4047[Import]
   for( var i = 0 ; i < parseInt(4046/fixChar.length) ; ++i )
   {
      recordValue += fixChar ;
   }
   for( var i = 0 ; i < parseInt(4046%fixChar.length) ; ++i )
   {
      recordValue += fixChar.charAt(i) ;
   }
   conRecord = '{"fieldLarge":"' + recordValue + '"}' ;  // complete record
   file.write( conRecord ) ;
}

// MAIN
function main( db )
{
   try
   {
      var FILENAME = LocalPath + "/" + CSPREFIX + "_import_bucket_jira489.file";
          impCmd = InstallPath + "/bin/sdbimprt",
          nestNum = 10 ,
          recordNum = 100,
          cmd = new Cmd() ;
      // create cs
      var cl = commCreateCL( db, COMMCSNAME, COMMCLNAME, 0, true, true, true,
                             "create collection in the beginning" ) ;
      // auto generate record
      cmd.run( "rm -rf " + FILENAME ) ;
      GenJsonRecord( FILENAME ) ;   // 5461 + 1 line
      var impCmdOption = "--hostname "+COORDHOSTNAME+" --svcname "+COORDSVCNAME+
                         " -c "+COMMCSNAME+" -l "+COMMCLNAME+" --file "+FILENAME+
                         " --type json " ;
      try
      {
         cmdToolRun( impCmd, impCmdOption ) ;
         println( "success to import records" ) ;
         //verify
         var cnt = 0 ;
         while( 5462 != cl.count() && 1000 > cnt )
         {
            cnt++ ;
            sleep( 3 ) ;
         }
         if( 5462 != cl.count() )
         {
            println( "expect record number: 5462, " +
                     "actual record number: " + cl.count() ) ;
            throw "ErrorNumberRecord" ;
         }
      }
      catch( e )
      {
         println( "failed to import records" + e ) ;
         throw e ;
      }
      cmd.run( "rm -rf " + FILENAME ) ;
   }
   catch( e )
   {
      cmd.run( "rm -rf " + FILENAME ) ;
      throw e ;
   }
}

// Run Main
try
{
   initGlobalVar() ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop collection in the beginning" ) ;
   main( db ) ;
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop collection in the beginning" ) ;
}
catch( e )
{
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop collection in the beginning" ) ;
   throw e ;
}
