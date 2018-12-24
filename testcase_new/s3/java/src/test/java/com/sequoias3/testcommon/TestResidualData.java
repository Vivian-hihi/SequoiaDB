package com.sequoias3.testcommon;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;

public class TestResidualData extends S3TestBase{
	@BeforeClass
	private void setUp() throws Exception {}

	@Test
	private void printResidualData() throws Exception{
        int errorCount = 0;
        Sequoiadb db = null;
        DBCursor cursor = null;
        String printInfo = "";
        try{
            db =  new Sequoiadb(S3TestBase.coordUrl, "", "");
            CollectionSpace metaCs = db.getCollectionSpace("MetaCollectionSpace");
            List<DBCollection> clList = new ArrayList<DBCollection>();
            List<String> clNameList = metaCs.getCollectionNames();
            for(String csclName : clNameList){
                String clname = csclName.substring(metaCs.getName().length()+1);
                clList.add(metaCs.getCollection(clname));
            }
            
            for(DBCollection cl : clList) {
                cursor = cl.query();
                if(cursor.hasNext()) {
                	printInfo+="\n===============begin print " + metaCs.getName() +"."+ cl.getName() + " data============\n";
                    while(cursor.hasNext()){
                        if(cursor.getNext().containsField("Name")){
                            if(!cursor.getCurrent().get("Name").equals(S3TestBase.s3UserName)&&!cursor.getCurrent().get("Name").equals(S3TestBase.bucketName)&&!cursor.getCurrent().get("Name").equals(S3TestBase.enableVerBucketName)){
                            	printInfo+=cursor.getCurrent().toString()+"\n";
                                errorCount++;
                            }
                        }else{
                        	printInfo+=cursor.getCurrent().toString()+"\n";
                            errorCount++;
                        }
                    }
                    printInfo+="\n===============end print " + metaCs.getName() +"."+ cl.getName() + " data==============\n";
                }
            }
            cursor.close();
            
            CollectionSpace dataCs = db.getCollectionSpace("DataCollectionSpace");
            try{
                DBCollection objectDataList = dataCs.getCollection("ObjectDataList");
                cursor = objectDataList.listLobs();
                if(cursor.hasNext()){
                	printInfo+="\n===============begin print " + dataCs.getName() +"."+ objectDataList.getName() + " data============\n";
                    while(cursor.hasNext()){
                    	printInfo+=cursor.getNext().toString()+"\n";
                        errorCount++;
                    }
                    printInfo+="\n===============end print " + dataCs.getName() +"."+ objectDataList.getName() + " data============\n";
                }
            } catch(BaseException e){
                Assert.assertEquals(e.getErrorCode(), SDBError.SDB_DMS_NOTEXIST.getErrorCode(), "getCollection ObjectDataList failed");
            }
            
            File file = new File(S3TestBase.installPath+"/tools/sequoias3/log/residualdata.log");
            if(!file.exists()) {
                file.createNewFile();
            }
            
            FileWriter fw = new FileWriter(file.getAbsoluteFile(),false);
            BufferedWriter bw = new BufferedWriter(fw);
            bw.write(printInfo);
            bw.close();
            
            if(errorCount != 0){
                throw new Exception("There is data residue problem");
            }
        } finally{
            if(cursor != null){
                cursor.close();
            }
            if(db != null){
                db.close();
            }
        }
    }
	@AfterClass
	private void tearDown() throws Exception {}
}
