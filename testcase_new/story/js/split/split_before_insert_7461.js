/******************************************************************************
@Description : 1. split range-collection by keys ，then insert data
@Modify list :
               2014-07-04  pusheng Ding  Init
               2014-07-08  xiaojun Hu    Changed
               2016-02-16  wuyan changed（add the testcase describe，and add the sleep time）
******************************************************************************/
function main() {
   //@ clean before
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop CL in the beginning" );
   try{
      var claSize = new RSize( COMMCSNAME );
      //createCS
      var varCS = commCreateCS( db, COMMCSNAME, true, "create CS" );
      //createCL , split in a
      var varCL = varCS.createCL(COMMCLNAME,{ShardingKey:{id:1}, ShardingType:"range",
                                 ReplSize:claSize.ReplSize(),Compressed:true});
   }catch( e ){
      throw e ;
   }
   var arrGroupName = getGroupName2(db, true);
   try{
      if( !(1 < arrGroupName.length) ){
         println("least two groups");
         throw e;
      }
   }catch( e ){
      return 0;
   }
   var PGname = getPG( COMMCSNAME, COMMCLNAME );
   var _PGname = PGname;
   var t = 1;
   var catadb = new Sdb(COORDHOSTNAME,CATASVCNAME);
   //SLgroupID are groupsID
   var SLgroupID = [];
   for( var i = 0 ; i != arrGroupName.length ; ++i ){
      if( PGname == arrGroupName[i][0] ){
         SLgroupID.push( arrGroupName[i][1] + ":" + arrGroupName[i][2] );
         break;
      }
   }
   //groups split in sepecific condition
   if( 2 == arrGroupName.length ) {
      for(var i=0; i !=arrGroupName.length ;++i){
         if( PGname != arrGroupName[i][0] ){
            try{
               varCL.split( PGname,arrGroupName[i][0],{id:500} );
               while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
               SLgroupID.push(arrGroupName[i][1] + ":" + arrGroupName[i][2]);
            }catch(e){
               throw e;
            }
            break;
         }
      }
   }else{
      //groups are more than two,we split three groups only
      for(var i=0; i != 3 && t < 3;++i){
         if( PGname != arrGroupName[i][0] ){
            if( _PGname != arrGroupName[i][0] ){
               try{
                  // println(PGname+"~~~~"+arrGroupName[i][0]);
                  varCL.split( PGname,arrGroupName[i][0],{id:500*t} );
                  while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
                  PGname = arrGroupName[i][0] ;
                  SLgroupID.push( arrGroupName[i][1] + ":" + arrGroupName[i][2] );
                  ++t;
               }catch(e){
                  throw e;
               }
            }
         }
      }
   }
   //insert data
   try
   {
      var docs = [];
      for( var i = 0 ; i != 500*SLgroupID.length ; ++i )
      {
         docs.push({id:i});
      }
      varCL.insert(docs);
   }
   catch( e )
   {
      throw e;
   }
   
   //check split result
   for(var i = 0 ; i != SLgroupID.length ; ++i)
   {
      var gdb = new Sdb(SLgroupID[i]+"");
      try
      {
         var len = eval( "gdb."+COMMCSNAME+"."+COMMCLNAME+".find().count()" );
         println(len)
      }
      catch(e)
      {
         throw e;
      }
      if(len != 500 ){
         println( "expect is: 500, actual is: " + len );
         throw "NotExpect";
      }
   }
   //@ clean end
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop CL in the beginning" );
}

try {
   if( true == commIsStandalone( db ) ) {
      println( "run mode is standalone" );
   } else {
      main();
      db.close();
   }
} catch( e ) {
   throw e ;
}
