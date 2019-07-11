/******************************************************************************
*@Description : seqDB-18281:options参数校验
*@Author      : 2019-5-6  XiaoNi Zhao
******************************************************************************/

main();
function main()
{
   var mainClName = "mainCl_18281_2";
   var subClName = "subCl_18281_2";
   var indexName = "idx";
   
   commDropCL( db, COMMCSNAME, mainClName );
   commDropCL( db, COMMCSNAME, subClName );   
   
   var mainCl = commCreateCLByOption( db, COMMCSNAME, mainClName, { IsMainCL: true, ShardingKey: { a: 1 } } );
   var subCl = commCreateCLByOption( db, COMMCSNAME, subClName, { ShardingKey: { a: 1 } } );
   var fullName = COMMCSNAME +"."+ subClName;
   println("COMMCSNAME===="+COMMCSNAME);
   println("fullName====="+fullName);
   mainCl.attachCL( fullName, { LowBound: { a: 0 }, UpBound: { a: 1000 } } );
                           
   /**************************** test1, field name lowercase ***************************/
   println("\n---Test1, create index, field name lowercase.");
   mainCl.createIndex( indexName, {a:1}, {unique:true, enforced:true} );          
   
   println("---Check results."); 
   checkIndex( mainCl, indexName, true, true, false ); 
      
   mainCl.dropIndex( indexName );
    
                           
   /**************************** test2, field name invalid ***************************/
   println("\n---Test2, create index, field name invalid.");
   var keyArr = [{isUnique:true}, {enforced:true}, {sortBufferSize:true}, {notNull:true}, {aa:true}];
   for ( i = 0; i < keyArr.length; i++) 
   {
      try
      {
         mainCl.createIndex( indexName, {a:1}, keyArr[i] );
      }
      catch ( e ) 
      {
         if( e !== -6 )
         {
            throw buildException( "checkResult", null, "", -6, "  " + e );
         }  
      }
   } 
   
   try 
   {
      mainCl.getIndex( indexName );
   } 
   catch ( e ) 
   {
      if( e !== -47 )
      {
         throw buildException( "checkResult", null, "", -47, "  " + e );
      } 
   }  
   
                           
   /**************************** test3, default value ***************************/
   println("\n---Test3, create index, default value.");
   mainCl.createIndex( indexName, {a:1},{unique:true} );          
   
   println("---Check results."); 
   checkIndex( mainCl, indexName, true, false, false ); 
      
   // mainClean index
   mainCl.dropIndex( indexName ); 
   
                           
   /**************************** test4, 2 diff name for same field ***************************/
   println("\n---Test3, create index, 2 diff name for same field.");
   var keyArr = [{enforced:true, Enforced:false}, {unique:false, Unique:false}, {NotNull:true, aa:false}];
   try
   {
      mainCl.createIndex( indexName, {a:1}, keyArr[i] );
   }
   catch ( e ) 
   {
      if( e !== -6 )
      {
         throw buildException( "checkResult", null, "", -6, "  " + e );
      }  
   }
   
                           
   /**************************** test5, boolean:0 ***************************/
   println("\n---Test5, create index, boolean:0.");
   mainCl.createIndex( indexName, {a:1}, {unique:1, enforced:0,NotNull:0} );
   println("---Check results."); 
   checkIndex( mainCl, indexName, true, false, false );
    
   // mainClean index
   mainCl.dropIndex( indexName ); 
   mainCl.remove();
   
                           
   /**************************** test6, unique:1, enforced:1,NotNull:1 ***************************/
   println("\n---Test6, create index, unique:1, enforced:1,NotNull:1.");
   mainCl.createIndex( indexName, {a:1}, {unique:1, enforced:1,NotNull:1} );
   println("---Check results."); 
   checkIndex( mainCl, indexName, true, true, true );
  
   try
   {
      mainCl.insert({a:1,b:4});
   } 
   catch ( e ) 
   {
      if( e !== -38 )
      {
         throw e;
      }  
   }
   mainCl.dropIndex( indexName ); 
   mainCl.remove();
   
   /**************************** test7, unique:1, enforced:1 ***************************/
   println("\n---Test7, create index, unique:1, enforced:1.");
   mainCl.createIndex( indexName, { a: 1 }, { unique: 1, enforced: 1 } );
   println("---Check results."); 
   checkIndex( mainCl, indexName, true, true );   
   mainCl.dropIndex( indexName ); 
   mainCl.remove();
             
   /**************************** test8, unique:0, enforced:0,NotNull:0 ***************************/
   println("\n---Test8, create index, unique:0, enforced:0,NotNull:0.");
   mainCl.createIndex( indexName, {a:1}, {unique:1, enforced:0, NotNull:0} );
   println("---Check results."); 
   checkIndex( mainCl, indexName, true, false, false );
   mainCl.dropIndex( indexName ); 
   mainCl.remove();
   
   /**************************** test9, NotNull:string/otherNum ***************************/
   println("\n---Test9, create index, NotNull:string/otherNum.");
   var keyArr = [{NotNull:"true"}, {NotNull:"false"}, {NotNull:2}];
   try
   {
      mainCl.createIndex( indexName, {a:1}, keyArr[i] );
   }
   catch ( e ) 
   {
      if( e !== -6 )
      {
         throw buildException( "checkResult", null, "", -6, "  " + e );
      }  
   }
                          
   // mainClean env
   commDropCL( db, COMMCSNAME, mainClName, false, false, "Failed to drop mainCl in the end-condition" );
}

function checkIndex( mainCl, indexName, expUni, expEnf, expNot ) 
{
   if( expUni == undefined ){ expUni = false };
   if( expEnf == undefined ){ expEnf = false };
   if( expNot == undefined ){ expNot = false };
   
   var indexDef = mainCl.getIndex( indexName ).toObj().IndexDef;
   var actUni = indexDef.unique;
   var actEnf = indexDef.enforced;
   var actNot = indexDef.NotNull;   
   if( actUni !== expUni || actEnf !== expEnf || actNot !== expNot )
   {
      var expResults = JSON.stringify( {unique: expUni, enforced: expEnf, NotNull: expNot} );
      var actResults = JSON.stringify( {unique: actUni, enforced: actEnf, NotNull: actNot} );
      throw buildException( "checkResult", null, "", expResults, "  " + actResults );
   }    
}

function checkRecords( mainCl, expRecs ) 
{
   println("---Check records.");
   var rc = mainCl.find( {}, {_id:{$include:0}} ).sort({b:1} );
   var actRecs = [];
   while( rc.next() )
   {
      actRecs.push( rc.current().toObj() );
   }
   if( JSON.stringify( expRecs ) !== JSON.stringify( actRecs ) )
   {
      throw buildException( "checkResult", null, "", JSON.stringify( expRecs ), "  " + JSON.stringify( actRecs ) );
   }
}