/************************************
*@Description: 创建普通集合，并在该集合上使用pop操作
*@author:      luweikang
*@createdate:  2017.7.10
*@testlinkCase:seqDB-11838
**************************************/

main();

function main()
{
   var csName = CHANGEDPREFIX + "_11838_CS";
   var clName = CHANGEDPREFIX + "_11838_CL";
   
   //clean CS befor test
   println("---begin test---");
   commDropCS( db, csName, true, "drop CS" );
   
   //create normal CS and CL
   dbcl = commCreateCL( db, csName, clName, null, null, true, false, "create normal CS" );
   
   //insert data
   normalCLinsertData( dbcl );
   
   //normalCL pop data
   println("---pop data---");
   try
   {
      dbcl.pop( { LogicalID:0, Direction:-1 } );
   }
   catch( e )
   {
      if( e !== -32 )
      {
         throw buildException( "normalCL pop data", e, "normalCL pop data", "pop fail", "pop success");
      }
   }
   
   //clean environment after test
   println( "---end the test---" );
   commDropCS( db, csName, true, "drop CS in the end" );
}

function normalCLinsertData( dbcl )
{
   var doc = [{No:1,a:10},{No:2,a:50},{No:3,a:-1001},
           {No:4,a:{$decimal: "123.456"}},{No:5,a:101.02}, 
           {No:6,a:{$numberLong:"9223372036854775807"}},{No:7,a:{$numberLong:"-9223372036854775808"}},
           {No:8,a:{$date: "2017-05-01"}},{No:9,a:{$timestamp: "2017-05-01-15.32.18.000000"}},
           {No:10,a:{$binary:"aGVsbG8gd29ybGQ=",$type:"1"}},
           {No:11,a:{$regex:"^z",$options:"i"}},
           {No:12,a:null},	           
           {No:13,a:{$oid:"123abcd00ef12358902300ef"}}, 
           {No:14,a:"abc"},
           {No:15,a:{MinKey:1}},
           {No:16,a:{MaxKey:1}},
           {No:17,a:true},{No:18,a:false},
           {No:19,a:{name:"Jack"}},
           {No:20,a:[1]},
           {No:21,a:[3]},
           {No:22,a:22}];
   try
   {
      dbcl.insert(doc);
      //println( "--insert data success" ) ;
   }
   catch(e)
   {
      throw buildException("normalCLinsertData()",e,"insert", "insert success","insert fail");
   }
}