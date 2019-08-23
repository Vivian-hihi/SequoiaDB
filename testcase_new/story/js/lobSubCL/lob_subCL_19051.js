/************************************
*@Description: seqDB-19051 rename子表集合空间名 
*@author:      luweikang
*@createDate:  2019.8.12
**************************************/
function main()
{
    if(commIsStandalone( db ))
    {
        println("skip standalone mode");
        return;
    }
    
    var mainCSName = COMMCSNAME;
    var mainCLName = "mainCL_19051";
    var oldSubCSName = "subCS_19051old";
    var newSubCSName = "subCS_19051new";
    var subCLName = "subCL_19051";
    var filePath = WORKDIR + "/lob19051/";
    var fileName = "file19051"
    var fileFullPath = filePath + fileName;
    var fileMD5 = makeTmpFile( filePath, fileName );
    
    commDropCL(db, mainCSName, mainCLName);
    commDropCS(db, oldSubCSName);
    commDropCS(db, newSubCSName); 
    
    var mainCL = createMainCLAndAttachCL(db, mainCSName, mainCLName, subCLName, "YYYYMMDD", 1, 20190801, 5);
    commCreateCL(db, oldSubCSName, subCLName);
    mainCL.attachCL( oldSubCSName + "." + subCLName, {"LowBound": {"date": "20190806"}, "UpBound": {"date": "20190811"}});
    var lobOids1 = insertLob(mainCL, fileFullPath, "YYYYMMDD", 5, 10, 2, "20190801");
    
    db.renameCS(oldSubCSName, newSubCSName);
    db.renameCS(newSubCSName, oldSubCSName);
    db.renameCS(oldSubCSName, newSubCSName);
    
    var lobOids2 = insertLob(mainCL, fileFullPath, "YYYYMMDD", 5, 10, 2, "20190801"); 
    checkLobMD5(mainCL, lobOids1, fileMD5);
    checkLobMD5(mainCL, lobOids2, fileMD5);
    
    for(i in lobOids1)
    {
        mainCL.deleteLob(lobOids1[i]);
        try
        {
            mainCL.getLob(lobOids1[i], filePath + "/checkLob19051_" + i );
            throw 0;
        }
        catch( e )
        {
            if( e !== -4 )
            {
                throw buildException( "check delete lob", e, "gets the deleted lob: " + lobOids1[i], -4, e ); 
            }
        }
    }
    
    deleteTmpFile( filePath );
    cleanMainCL(db, mainCSName, mainCLName);
    commDropCS(db, newSubCSName);
}

main();