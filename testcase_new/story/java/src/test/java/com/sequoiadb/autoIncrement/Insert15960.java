package com.sequoiadb.autoIncrement;

import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

public class Insert15960 extends SdbTestBase{
	private Sequoiadb sdb = null;
	private CollectionSpace scs = null;
	private DBCollection scl = null;
    private String clName = "cl_15960";
    private String indexName = "idIndex";
    private int threadNum = 5;
    private int threadInsertNum = 10000;
    private int currentValue = 1;
    private List<String> coordNodes = null;
    
	@BeforeClass
    public void setUp() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if (CommLib.isStandAlone(sdb)) {
			throw new SkipException("skip StandAlone or one coordNode");
		}
		coordNodes = CommLib.getNodeAddress(sdb, "SYSCoord");
		if (coordNodes.size() < 2) {
			throw new SkipException("skip one coordNode");
		}
		
		scs = sdb.getCollectionSpace(csName);
        scl = scs.createCollection(clName, (BSONObject)JSON.parse("{AutoIncrement:{Field:\"id\", AcquireSize:1}}"));
        scl.createIndex(indexName, "{id:1}", true, true);
	}
	
	@AfterClass
    public void tearDown(){
		scs.dropCollection(clName);
		sdb.close();
	}
	
	@Test
    public void test() {
		InsertThread insertThread1 = new InsertThread(coordNodes.get(0));
		InsertThread insertThread2 = new InsertThread(coordNodes.get(1));
		insertThread1.start(threadNum);
		insertThread2.start(threadNum);
		if(!(insertThread1.isSuccess() && insertThread2.isSuccess())){
            Assert.fail(insertThread1.getErrorMsg() + insertThread2.getErrorMsg());
        }
		
		AutoIncrementUtils.checkResult(scl, threadInsertNum*threadNum*2, currentValue);
		
	}
	
	private class InsertThread extends SdbThreadBase {
		private String coordNode;
		public InsertThread(String coordNode) {
			super();
			this.coordNode = coordNode;
		}
		@Override
		public void exec() throws BaseException{
			try(Sequoiadb db = new Sequoiadb(coordNode, "", "")){
				DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
				for(int i=0; i< threadInsertNum; i++){
					BSONObject obj = (BSONObject) JSON.parse("{a:" + i + "}");
					cl.insert(obj);
				}
			}
			
		}
	}

}
