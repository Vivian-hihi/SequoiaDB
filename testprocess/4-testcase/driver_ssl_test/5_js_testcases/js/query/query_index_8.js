
CSPREFIX_CS = CSPREFIX+"foo" ;
CSPREFIX_CL = CSPREFIX+"bar" ;
var db = new SecureSdb( COORDHOSTNAME, COORDSVCNAME ) ;

// clear
commDropCS( db, CSPREFIX_CS, true, "clear in begin" ) ;

try
{
   var claSize = new RSize( CSPREFIX_CS );
   var varCS = db.createCS( CSPREFIX_CS );

   for( var i=1;i<10;i++ )
   {
      varCS.createCL( CSPREFIX_CL+i,{ReplSize:claSize.ReplSize(), Compressed:true} );
   }
}
catch( e )
{
   println("failed to create CL" + i );
   throw e ;
}

for( var i=1;i<10;i++ )
{
   try
   {
      var varCL = varCS.getCL(CSPREFIX_CL+i);
   }
   catch(e)
   {
      println("failed to get CL,i="+i);
      throw e ;
   }

   for(var j =0 ;j <1000 ;j++ )
   {
      try
      {
         if ( j<500 )
         {
            varCL.insert({name:"foo"+j});
         }
         else
         {
            varCL.insert({no:j,name:"foo"+j});
         }
      }
      catch( e )
      {
         println("failed to insert records,j="+j);
         throw e ;
      }
   }

   try
   {
      varCL.createIndex("testIndex",{name:-1},true);
   }
   catch( e )
   {
      println("failed to create index,i="+i);
      throw e ;
   }

   try
   {
      rc = varCL.find({$or:[{no:{$gt:250,$lt:500}},{no:{$gt:500,$lt:900}}],name:{$regex:"foo*"},no:{$mod:[5,3]},$and:[{no:{$gt:800}},{no:{$lt:850}}],name:{$nin:["foo898","foo893","foo888"]}});   
   }
   catch ( e )
   {
      println( "failed to find record , rc= " + e ) ;
      throw e ;
   }
   
   var count = 0 ;
   try
   {
      count = rc.count() ;
      println( "count1: " + count ) ;
   }
   catch( e )
   {
      println( "get rc count after create index failed: " + e + " in times " + j ) ;
      throw e ;
   }
   if( count != 10 )
   {
      println("return wrong number of records ");
      println(varCL.find({$or:[{no:{$gt:250,$lt:500}},{no:{$gt:500,$lt:900}}],name:{$regex:"foo*"},no:{$mod:[5,3]},$and:[{no:{$gt:800}},{no:{$lt:850}}],name:{$nin:["foo898","foo893","foo888"]}}));   
      throw -1 ;
   }
}

// clear
commDropCS( db, CSPREFIX_CS, false, "clear in end" ) ;

