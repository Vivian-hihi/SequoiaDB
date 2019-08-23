/************************************
*@Description: seqDB-19052 rename子表集合名
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
    var mainCLName = "mainCL_19052";
    var subCSName = "subCS_19052";
    var oldSubCLName = "subCL_19052old";
    var newSubCLName = "subCL_19052new";
    var filePath = WORKDIR + "/lob19052/";
    var fileName = "file19052"
    var fileFullPath = filePath + fileName;
    var fileMD5 = makeTmpFile( filePath, fileName );
    
    commDropCL(db, mainCSName, mainCLName);
    commDropCS(db, subCSName);
    
    var mainCL = createMainCLAndAttachCL(db, mainCSName, mainCLName, oldSubCLName, "YYYYMMDD", 1, 20190801, 5);
    commCreateCL(db, subCSName, oldSubCLName);
    mainCL.attachCL( subCSName + "." + oldSubCLName, {"LowBound": {"date": "20190806"}, "UpBound": {"date": "20190811"}});
    var lobOids1 = insertLob(mainCL, fileFullPath, "YYYYMMDD", 5, 10, 2, "20190801");
     
    db.getCS(subCSName).renameCL(oldSubCLName, newSubCLName);
    db.getCS(subCSName).renameCL(newSubCLName, oldSubCLName);
    db.getCS(subCSName).renameCL(oldSubCLName, newSubCLName);
    
    var lobOids2 = insertLob(mainCL, fileFullPath, "YYYYMMDD", 5, 10, 2, "20190801"); 
    checkLobMD5(mainCL, lobOids1, fileMD5);
    checkLobMD5(mainCL, lobOids2, fileMD5);
    
    for(i in lobOids1)
    {
        mainCL.deleteLob(lobOids1[i]);
        try
        {
            mainCL.getLob(lobOids1[i], filePath + "/checkLob19052_" + i );
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
    commDropCS(db, subCSName);
}

main();