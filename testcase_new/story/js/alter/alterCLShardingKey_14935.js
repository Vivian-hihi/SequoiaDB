/* *****************************************************************************
@discretion: cl add shardingKey
@author£º2018-4-16 wuyan  Init
***************************************************************************** */
var clName1 = CHANGEDPREFIX + "_alterclShardingKey_14935a"; 
var clName2 = CHANGEDPREFIX + "_alterclShardingKey_14935b";  

main(db);
function main(db)
{	  
	try
	{  
	   if( true == commIsStandalone( db ) )
      {
         println( "run mode is standalone" );
         return;
      }  
	   //clean environment before test
      commDropCL( db, COMMCSNAME, clName1, true, true,"drop CL in the beginning" ) ; 
      commDropCL( db, COMMCSNAME, clName2, true, true,"drop CL in the beginning" ) ;  
         
      //create cl           
      var dbcl1 = commCreateCL( db, COMMCSNAME, clName1);    
      var dbcl2 = commCreateCL( db, COMMCSNAME, clName2);     
      
      //test a :alter cl1,one shardingKey field
      var shardingKeyField1 = {a:1};
      dbcl1.setAttributes({ShardingKey:shardingKeyField1}); 
      checkAlterResult( clName1, "ShardingKey", shardingKeyField1 ); 
      
      //test b: alter cl2,many shardingKey field
      var shardingKeyField2 = {a:1,b:1,c:-1};
      dbcl2.setAttributes({ShardingKey:shardingKeyField2}); 
      checkAlterResult( clName1, "ShardingKey", shardingKeyField1 );    
      
      //clean
      commDropCL( db, COMMCSNAME, clName1, true, true,"clear collection in the beginning" ) ;   
      commDropCL( db, COMMCSNAME, clName2, true, true,"clear collection in the beginning" ) ;         
   }
   catch( e )
   {
      throw buildException( "alter shardingKey by setAttributes fail:", e); 
   }
   finally
   {
      if( db != null )
      {
         db.close()
      }
   }   
}


