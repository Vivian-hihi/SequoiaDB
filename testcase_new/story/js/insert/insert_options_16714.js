/******************************************************************************
*@Description: test insert data with options
*@author:      wangkexin
*@createdate:  2018.11.26
*@testlinkCase: seqDB-16714:options取值验证
******************************************************************************/

main(db);
function main( db )
{
	var clName = CHANGEDPREFIX + "_insert16714";
	var oid = "5bf7575bdc4e88fa3dd16714";
	
	commDropCL( db, COMMCSNAME, clName, true, true, "clear collection in the beginning" ) ; 
	
	var claSize = new RSize( COMMCSNAME );
	var cs = db.getCS(COMMCSNAME);
	var cl = cs.createCL(clName,{ReplSize:claSize.ReplSize(), Compressed:true});
   
	println("begin to insert  data")
	// insert data
	try
	{
		cl.insert({_id:1,a:1,b:1});
		var returnOidString = cl.insert({_id:oid,test:"test16714"},{ReturnOID:true});
		cl.insert([{"_id":oid,test:"test167145_1"},{_id:123}],{ContOnDup:true});
		println( "success to insert data to the collection" ) ;
	}
	catch( e )
	{
			throw buildException("check insert data with options", e);
	}
	
	try
	{
		if(cl.find().count() != 3)
		{	
			throw buildException("check data count result", null, "check the number  of records in the collection",
									3, cl.find().count());
		}	
		
		var obj1 = {_id:1,a:1,b:1};
		var obj2 = {"_id": oid,"test": "test16714"};
		var obj3 = {"_id": 123};
		
		var cur = cl.find({_id:1});
		if (!compareObj(obj1, cur.next().toObj()))
		{
			throw "obj1 verify failed";
		}
		
		var cur = cl.find({_id:returnOidString.toObj()._id.toString()})
		if (!compareObj(obj2, cur.next().toObj()))
		throw "obj2 verify failed";

		var cur = cl.find({_id:123});
		if (!compareObj(obj3, cur.next().toObj()))
		throw "obj3 verify failed";
		println("successful insertion of data with options");
		
		cur.close();
	}
	catch( e )
	{
			throw buildException("check insert data with options results ", e);
	}	
	finally
	{
		commDropCL( db, COMMCSNAME, clName, true, true, "drop collection in the end , error" ) ;
	}
}

