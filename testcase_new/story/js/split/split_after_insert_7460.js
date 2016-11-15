
function main() {
   var NUM = 500 ;
   //var NUM = 50000 ;
   //@ clean before
   commDropCL( db, COMMCSNAME, COMMCLNAME, true, true,
               "drop CL in the beginning" );
   try{
      //createCS
      var varCS = commCreateCS( db, COMMCSNAME, true, "create CS" );
      //createCL , split in a
      var varCL = varCS.createCL(COMMCLNAME,{ShardingKey:{id:1}, ShardingType:"range",
                                 ReplSize:0,Compressed:true});
   }catch( e ){
      throw e ;
   }
   var arrGroupName = getGroupName2(db, true);
   try{
      if( !(1 < arrGroupName.length) ){
         println("least two groups");
         db.dropCS( COMMCSNAME );
         throw e;
      }
   }catch( e ){
      return 0;
   }
   var PGname = getPG( COMMCSNAME, COMMCLNAME );
   var _PGname = PGname;
   var t = 1;
   var catadb = new Sdb(COORDHOSTNAME,CATASVCNAME);
   //nodeAddrArr
   var nodeAddrArr = [];
   for( var i = 0 ; i != arrGroupName.length ; ++i ){
      if( PGname == arrGroupName[i][0] ){
         nodeAddrArr.push( arrGroupName[i][1] + ":" + arrGroupName[i][2] );
         break;
      }
   }
   //groups split in sepecific condition
   if( 2 == arrGroupName.length ) 
   {
      //insert data
      for(var i=0;i<NUM*2;i++) 
      {
         try 
         {
            varCL.insert({id:i});
         } 
         catch(e) 
         {
            throw e;
         }
      }
      for(var i=0; i !=arrGroupName.length ;++i){
         if( PGname != arrGroupName[i][0] ){
            try{
               varCL.split( PGname,arrGroupName[i][0],{id:NUM} );
               while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
               nodeAddrArr.push(arrGroupName[i][1] + ":" + arrGroupName[i][2]);
            }catch(e){
               throw e;
            }
            break;
         }
      }
   }else{
      //groups are more than two,we split three groups only
      //insert data
      var doc = [];
      for(var i=0;i<NUM*3;i++)
      {                   
         doc.push({id:i});
      }
      varCL.insert( doc ) ;  
      
      for(var i=0; i != 3 && t < 3;++i){
         if( PGname != arrGroupName[i][0] ){
            if( _PGname != arrGroupName[i][0] ){
               try{
                  // println(PGname+"~~~~"+arrGroupName[i][0]);
                  varCL.split( PGname,arrGroupName[i][0],{id:NUM*t} );
                  while( catadb.SYSCAT.SYSTASKS.find().count() != 0 );
                  PGname = arrGroupName[i][0] ;
                  nodeAddrArr.push(arrGroupName[i][1] + ":" + arrGroupName[i][2]);
                  ++t;
               }catch(e){
                  throw e;
               }
            }
         }
      }
   }
   for(var i = 0 ; i != nodeAddrArr.length ; ++i){
      var gdb = new Sdb(nodeAddrArr[i]+"");
      try{
         var len = eval( "gdb."+COMMCSNAME+"."+COMMCLNAME+".count()" );
         println(len);
      }catch(e){
         throw e;
      }
      if( len != NUM ){
         println( "expect is: " + NUM + ", actual is: " + len );
         println( "Debug begin: the records are as below: " ) ;
         try
         {
            var record ;
            var counter = 0 ;
         }
         catch( e )
         {
         }
         println( "Debug end" ) ;
         throw "NotExpectResult";
      }
   }
   //@ Clean End
   commDropCL( db, COMMCSNAME, COMMCLNAME, false, false,
               "drop CL in the end" );
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
