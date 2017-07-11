/************************************
*@Description: get actual result and check it 
*@author:      zhaoyu
*@createDate:  2017.7.11
**************************************/
function findBySortLimitSkipAndCheck( dbcl, findConf, selectConf, sortConf, limitConf, skipConf, expRecs )
{
	if( typeof(findConf) == "undefined" ) { findConf = null; }
	if( typeof(selectConf) == "undefined" ) { selectConf = null; }
	if( typeof(sortConf) == "undefined" ) { sortConf = null; }
	if( typeof(limitConf) == "undefined" ) { limitConf = null; }
	if( typeof(skipConf) == "undefined" ) { skipConf = null; }
   var rc = dbcl.find( findConf, selectConf ).sort( sortConf ).limit( limitConf ).skip( skipConf );
   println("--begin to check the data");
   checkRec( rc, expRecs );
   println("--end check the data");
}

/************************************
*@Description: compare actual and expect result,
               they is not the same ,return error ,
               else return ok
*@author:      zhaoyu
*@createDate:  2015.5.20
**************************************/
function checkRec( rc, expRecs )
{				
	//get actual records to array
	var actRecs = [];
   while( rc.next() )
   {
		actRecs.push( rc.current().toObj() );
   }
   //check count
	if( actRecs.length !== expRecs.length )
   {
   	println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));
   	throw buildException("check count", null, "",
									expRecs.length, actRecs.length);
   }
   
   //check every records every fields,expRecs as compare source
   for( var i in expRecs )
   {
   	var actRec = actRecs[i];
   	var expRec = expRecs[i];
   	
   	for ( var f in expRec )
   	{
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(i)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
   //check every records every fields,actRecs as compare source
   for( var j in actRecs )
   {
   	var actRec = actRecs[j];
   	var expRec = expRecs[j];
   	
   	for ( var f in actRec )
   	{
   	   if(f == "_id")
   	   {
   	      continue;
   	   }
   		if( JSON.stringify(actRec[f]) !== JSON.stringify(expRec[f]) )
	   	{
	   		println("\nerror occurs in "+(parseInt(j)+1)+"th record, in field '"+f+"'");
	   		println("\nactual record= "+JSON.stringify(actRec)+"\n\nexpect record= "+JSON.stringify(expRec));
	   		println("\nactual recs in cl= "+JSON.stringify(actRecs)+"\n\nexpect recs= "+JSON.stringify(expRecs));   		
	   		throw buildException("checkRec()", "rec ERROR");
	   	}
   	}
   }
}
