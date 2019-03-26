/************************************
*@Description: 内置SQL支持timestamp带条件查询 
*@author:      wangkexin
*@createdate:  2019.3.2
*@testlinkCase:seqDB-13705
**************************************/

main();

function main()
{
    var csName = COMMCSNAME;
    var clName = "cl13705";

    var cl = commCreateCL( db, csName, clName, null, null, true, false, "create cl in the begin" );
    
    //正常timestamp类型数据、边界值、非法值
    //Timestamp类型能表示的时间范围为[1901-12-13T20:45:52.000000Z, 2038-01-19T03:14:07.999999Z] 
    insertSQL(db, cl, csName, clName, 'Timestamp("2019-03-02T10:48:50.000000Z")', {$timestamp:"2019-03-02T10:48:50.000000Z"}, true);
    insertSQL(db, cl, csName, clName, 'Timestamp("1901-12-13T20:45:52.000000Z")', {$timestamp:"1901-12-13T20:45:52.000000Z"}, true);
    insertSQL(db, cl, csName, clName, 'Timestamp("2038-01-19T03:14:07.999999Z")', {$timestamp:"2038-01-19T03:14:07.999999Z"}, true);
    insertSQL(db, cl, csName, clName, 'Timestamp("1901-12-13T20:45:51.999999Z")', false);
    insertSQL(db, cl, csName, clName, 'Timestamp("2038-01-19T03:14:08.000000Z")', false);
    insertSQL(db, cl, csName, clName, 'Timestamp("978192000000")', false);
    
    selectSQL(db, csName, clName, 'Timestamp("2019-03-02T10:48:50.000000Z")', true);
    selectSQL(db, csName, clName, 'Timestamp("1901-12-13T20:45:52.000000Z")', true);
    selectSQL(db, csName, clName, 'Timestamp("2038-01-19T03:14:07.999999Z")', true);
    selectSQL(db, csName, clName, 'Timestamp("1901-12-13T20:45:51.999999Z")', false);
    selectSQL(db, csName, clName, 'Timestamp("2038-01-19T03:14:08.000000Z")', false);
    selectSQL(db, csName, clName, 'Timestamp("978192000000")', false);
    
    updateSQL(db, cl, csName, clName, 'Timestamp("2019-03-02T10:48:50.000000Z")', 'Timestamp("2019-03-01T00:00:00.000000Z")', {$timestamp:"2019-03-01T00:00:00.000000Z"}, true);
    updateSQL(db, cl, csName, clName, 'Timestamp("1901-12-13T20:45:52.000000Z")', 'Timestamp("1901-12-14T20:45:52.000000Z")', {$timestamp:"1901-12-14T20:45:52.000000Z"}, true);
    updateSQL(db, cl, csName, clName, 'Timestamp("1901-12-14T20:45:52.000000Z")', 'Timestamp("1901-12-13T20:45:51.999999Z")',false);
    updateSQL(db, cl, csName, clName, 'Timestamp("2038-01-19T03:14:07.999999Z")', 'Timestamp("2037-01-19T03:14:07.999999Z")', {$timestamp:"2037-01-19T03:14:07.999999Z"}, true);
    updateSQL(db, cl, csName, clName, 'Timestamp("2037-01-19T03:14:07.999999Z")', 'Timestamp("2038-01-19T03:14:08.000000Z")',false);
    updateSQL(db, cl, csName, clName, 'Timestamp("2037-01-19T03:14:07.999999Z")', 'Timestamp("978192000000")',false);
    
    deleteSQL(db, cl, csName, clName, 'Timestamp("2019-03-01T00:00:00.000000Z")', {$timestamp:"2019-03-01T00:00:00.000000Z"}, true);
    deleteSQL(db, cl, csName, clName, 'Timestamp("1901-12-14T20:45:52.000000Z")', {$timestamp:"1901-12-14T20:45:52.000000Z"}, true);
    deleteSQL(db, cl, csName, clName, 'Timestamp("2037-01-19T03:14:07.999999Z")', {$timestamp:"2037-01-19T03:14:07.999999Z"}, true);
    deleteSQL(db, cl, csName, clName, 'Timestamp("1901-12-13T20:45:51.999999Z")', false);
    deleteSQL(db, cl, csName, clName, 'Timestamp("2038-01-19T03:14:08.000000Z")', false);
    deleteSQL(db, cl, csName, clName, 'Timestamp("978192000000")', false);
    
    commDropCL( db, csName, clName, true, true, "drop CL in the end" );
}

function insertSQL(db, cl, csName, clName, insertValue, checkValue, result)
{
    var sql = 'insert into '+csName+'.'+clName+'(num, textFields ) values (3, ' + insertValue + ')';
    if(result)
    {
        try
        {
            db.execUpdate( sql );
            var cursor = cl.find({textFields:checkValue});
            if(cursor.next() === undefined)
            {
                throw buildException("insertSQL()",null,"check record " + insertValue, "have data", "no data");
            }
        }
        catch( e )
        {
            throw buildException("insertSQL()",e ,"insert record " + insertValue, "insert success", "insert failed: "+e);
        }
        finally
        {
            cursor.close()
        }
    }
    else
    {
        try
        {
            db.execUpdate( sql );
            throw buildException("insertSQL()",null,"insert error record " + insertValue, "insert failed", "insert success");
        }
        catch( e )
        {
            if( e!=-6 && e!=-195 )
            {
                throw buildException("insertSQL()",e ,"insert record " + insertValue, '-6', e );
            }
        }
    }
}

function updateSQL(db, cl, csName, clName, oldValue, newValue, checkValue, result)
{
    var sql = 'update '+csName+'.'+clName+' set textFields=' + newValue + ' where textFields=' + oldValue;
    if(result)
    {
        try
        {
            db.execUpdate(sql);
            var cursor = cl.find({textFields:checkValue});
            if(cursor.next() === undefined)
            {
                throw buildException("updateSQL()",null,"check record " + newValue, "have data","no data");
            }
        }
        catch( e )
        {
            throw buildException("updateSQL()", e, "update record " + oldValue + " to " + newValue, "update success","update failed: " + e);
        }
        finally
        {
            cursor.close()
        }
    }
    else
    {
        try
        {
            db.execUpdate( sql );
            throw buildException("updateSQL()", null, "update record " + oldValue + " to " + newValue + " should failed", "update failed", "update success");
        }
        catch( e )
        {
            if( e!=-6 && e!=-195 )
            {
                throw buildException("updateSQL()", e,"update record " + oldValue + " to " + newValue, '-6', e);
            }
        }
    }
}

function selectSQL(db, csName, clName, value, result)
{
    var sql = 'select * from '+csName+"."+clName+' where textFields=' + value;
    if(result)
    {
        try
        {
            var cursor = db.exec( sql );
            if(cursor.next() === undefined)
            {
                throw buildException("selectSQL()",null,"check record " + value, "have data","no data");
            }
        }
        catch( e )
        {
            throw buildException("selectSQL()",e ,"select record " + value, "select success","select failed: "+e);
        }
        finally
        {
            cursor.close()
        }
    }
    else
    {
        try
        {
            db.execUpdate( sql );
            throw buildException("selectSQL()",null,"select error record " + value, "select failed", "select success");
        }
        catch( e )
        {
            if( e!=-6 && e!=-195 )
            {
                throw buildException("selectSQL()",e ,"select record " + value, '-6', e );
            }
        }
    }
}

function deleteSQL(db, cl, csName, clName, deleteValue, checkValue, result)
{
    var sql = 'delete from '+csName+'.'+clName+' where textFields='+deleteValue;
    if(result)
    {
        try
        {
            db.execUpdate( sql );
            var cursor = cl.find({textFields:checkValue});
            if(cursor.next() !== undefined)
            {
                throw buildException("deleteSQL()",null,"check record " + deleteValue, "no data","have data");
            }
        }
        catch( e )
        {
            throw buildException("deleteSQL()",e ,"delete record " + deleteValue, "delete success","delete failed: "+e);
        }
        finally
        {
            cursor.close()
        }
    }
    else
    {
        try
        {
            db.execUpdate( sql );
            throw buildException("deleteSQL()",null,"delete error record " + deleteValue, "delete failed", "delete success");
        }
        catch( e )
        {
            if( e!=-6 && e!=-195 )
            {
                throw buildException("deleteSQL()",e ,"delete record " + deleteValue, '-6', e );
            }
        }
    }
}