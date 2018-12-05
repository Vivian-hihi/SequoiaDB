
METACSNAME = "MetaCollectionSpace";

USERNAME   = "User";

BUCKETNAME = "BucketList";

if(typeof(COORDSVCNAME)=="undefined"){COORDSVCNAME="50000"}

function main()
{
   var db = new Sdb("localhost",COORDSVCNAME);
   try{   
//           db.dropCS(METACSNAME);
        	   
	   var varCS = db.createCS(METACSNAME);
	   
	   var varCLUser = varCS.createCL(USERNAME);
	   
	   varCLUser.createIndex("idIndex",{ID:1},true,true)
	   
	   varCLUser.createIndex("nameIndex",{Name:1},true,true)
	   
	   varCLUser.createIndex("accessIndex",{AccessKeyID:1},true,true)
	   
	   var varCLBucket = varCS.createCL(BUCKETNAME)
	   
	   varCLBucket.createIndex("idIndex",{ID:1},true,true)
	   
	   varCLBucket.createIndex("nameIndex",{Name:1},true,true)

   }catch ( e )
   {
      println( "failed to create meta cs. error code: " + e + ", description: " + getLastErrMsg()) ;
   }
   
}

main();
