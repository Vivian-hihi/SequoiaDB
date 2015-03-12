
function main( db )
{
   if ( commIsStandalone(db) )
   {
      println( "In standalone, not run test" ) ;
      return ;
   }

   // clean
   fmpCleanProcedures( db, CSPREFIX ) ;

   // create procedures
   var nameArray = new Array( CSPREFIX+"_createCSAndCL", CSPREFIX+"_insertRecord", CSPREFIX+"_readData", CSPREFIX+"_dropCS" ) ;
   var index = 0 ;

   try
   {
      eval( "db.createProcedure( function " + nameArray[index] + "(csName,clName) {"+
               "try {"+
                  "db.dropCS( csName ) ;"+
               "}"+
               "catch(e) {"+
                  "if( e!=-34 ) {"+
                     "println( 'failed to drop CS ,e:' + e ) ;"+
                     "throw e ;"+
                  "}"+
               "}"+
               "try {"+
                  "var cs = db.createCS( csName ); "+
                  "var cl = cs.createCL( clName, {ReplSize:0} ); "+
               "}"+
               "catch(e) {"+
                  "println('failed to create collection ,e=' + e ) ;"+
                  "throw e ;"+
               "}"+
               "return cl ;"+
            "} )"
         ) ;
      ++index ;

      eval( "db.createProcedure( function " + nameArray[index] + "(num, cl) {"+
               "cl.remove();"+
               "for( var i = 0; i < num ; i++ ) "+
               "{"+
                  "try"+
                  "{"+
                     "cl.insert({_id:{$oid:Math.random()},group:i%5,price:i,name:'test'+i } ) ;"+
                  "}"+
                  "catch(e)"+
                  "{"+
                     "println('at the time of '+i+' fail to insert record,e='+e ) ;"+
                     "throw e ;"+
                  "}"+
               "}"+
            "} )"
          );
      ++index ;

      eval( "db.createProcedure( function " + nameArray[index] + "(num, csName, clName) {"+
                "var cl = " + nameArray[0] + "( csName, clName ) ; "+
                nameArray[1] + "( num, cl ) ;"+
                "return cl.aggregate({$group:{_id:'$group'}});"+
            "} ) "
          );
      ++index ;

      eval( "db.createProcedure( function " + nameArray[index] + "( csName ) {" +
               "try"+
               "{"+
                  "db.dropCS( csName ) ;"+
               "}"+
               "catch(e)"+
               "{"+
                  "if( e != -34 )"+
                  "{"+
                     "println('failed to drop CS ,e='+e); "+
                     "throw e ;"+
                  "}"+
               "}"+
            "} )"
          ) ;
   }
   catch( e )
   {
      println( "Create procedure " + nameArray[index] + " failed: " + e ) ;
      throw e ;
   }

   // check procedures
   try
   {
      var rc = db.listProcedures({$or:[{name:nameArray[0]},{name:nameArray[1]},{name:nameArray[2]},{name:nameArray[3]}]});
   }
   catch(e)
   {
      println("Failed to execute listProcedures function,e=" + e ) ;
      throw e ;
   }
   if ( rc.size() != 4 )
   {
      println("Return wrong number of records ,rc.size()=" + rc.size() ) ;
      throw "wrong number of procedures" ;
   }

   // run procedures
   try
   {
      db.eval( nameArray[2]+"( 100, \"" + COMMCSNAME + "\", \"" + COMMCLNAME + "\" )" ) ;
   }
   catch(e)
   {
      println("Failed to execute eval function[" + nameArray[2] + "], e=" + e ) ;
      throw e ;
   }
   try
   {
      db.eval( nameArray[3]+"(\"" + COMMCSNAME + "\")" );
   }
   catch(e)
   {
      println("failed to execute eval function[" + nameArray[3] + "],e=" + e ) ;
      throw e ;
   }

   // remove procedures
   fmpRemoveProcedures( nameArray, false ) ;

   // check procedures
   try
   {
      var rc = db.listProcedures({$or:[{name:nameArray[0]},{name:nameArray[1]},{name:nameArray[2]},{name:nameArray[3]}]});
   }
   catch(e)
   {
      println("Failed to execute listProcedures function,e=" + e ) ;
      throw e ;
   }
   if ( rc.size() != 0 )
   {
      println("Return not zero number of records ,rc.size()=" + rc.size() ) ;
      throw "not zero number of procedures" ;
   }
}

try
{
   main( db ) ;
   db.close() ;
}
catch( e )
{
   throw e ;
}
