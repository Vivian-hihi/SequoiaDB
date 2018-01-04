package com.sequoiadb.split;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;

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

/**
 * @FileName:SEQDB-10527 切分过程中删除CS :1、向cl中循环插入数据记录 2、执行split，设置范围切分条件
 *                       3、切分过程中执行删除cs操作，分别在如下阶段删除CS:
 *                       a、任务已下发还未开始执行（如执行split后，通过listTasks查看无任务，在此过程中删除cs）
 *                       b、迁移数据过程中（如直连目标组节点查看数据持续插入，可count查询数据量在增加） * 
 *                       c、目标组更新编目信息后删除cs（如直连目标组查看数据已迁移完成，或者直连编目节点查看cl信息中存在目标组）
 *                       4、查看切分和删除cs操作结果 备注：验证C场景
 * @author huangqiaohui
 * @update [wuyan 2018/01/03 增加insert记录数到5W]
 * @version 1.00
 *
 */

public class Split10527C extends SdbTestBase {
	private String clName = "testcaseCL_10527C";
	private String customCSName = "testcaseCS_10527C";
	private String srcGroupName;
	private String destGroupName;
	private Sequoiadb commSdb = null;
	private AtomicBoolean flag = new AtomicBoolean(false);

	@BeforeClass()
	public void setUp() {
		commSdb = new Sequoiadb(coordUrl, "", "");

		// 跳过 standAlone 和数据组不足的环境
		CommLib commlib = new CommLib();
		if (CommLib.isStandAlone(commSdb)) {
			throw new SkipException("skip StandAlone");
		}
		List<String> groupsName = commlib.getDataGroupNames(commSdb);
		if (groupsName.size() < 2) {
			throw new SkipException("current environment less than tow groups ");
		}
		srcGroupName = groupsName.get(0);
		destGroupName = groupsName.get(1);

		DBCollection dbCollection = createCSAndCL();
		insertData(dbCollection);// 写入待切分的记录（50000）
	}
	
	@Test(timeOut = 30 * 60 * 1000)
	public void splitAnddropCS() {		
		Sequoiadb dataNode = null;
		Split splitThread = null;
		try {
			// 启动切分
			splitThread = new Split();
			splitThread.start();

			// 等待目标组数据迁移完成			
			dataNode = commSdb.getReplicaGroup(destGroupName).getMaster().connect();
			// 获得目标组主节点链接
			while (dataNode.isCollectionSpaceExist(customCSName) != true && flag.get() == false) {
			}
			
			CollectionSpace cs = dataNode.getCollectionSpace(customCSName);
			while (cs.isCollectionExist(clName) != true && flag.get() == false) {
			}
			
			DBCollection cl = dataNode.getCollectionSpace(customCSName).getCollection(clName);
			//the count is 10916
			while (cl.getCount() < 10800 && flag.get() == false) {				
			}

			// 删除CS,sleeptime是为了随机覆盖：1、数据迁移完成，编目未更新；2、数据迁移完成，编目已更新			
			try{
				//随机删除CS
				Random random = new Random();
				int sleeptime = random.nextInt(1000);				
				Thread.sleep(sleeptime);				
				commSdb.dropCollectionSpace(customCSName);				
				Assert.assertEquals(commSdb.isCollectionSpaceExist(customCSName), false);				
			} catch (BaseException e) {				
				if ( e.getErrorCode() != -147){
					Assert.fail(e.getMessage() );
				}	
			}			

			// 检测切分线程
			Assert.assertEquals(splitThread.isSuccess(), true, splitThread.getErrorMsg());		
		} catch (InterruptedException e) {
			Assert.fail(e.getMessage() + "\r\n" + SplitUtils.getKeyStack(e, this));
		} finally {
			if (dataNode != null) {
				dataNode.disconnect();
			}
			if (splitThread != null) {
				splitThread.join();
			}
		}
	}

	@AfterClass
	public void tearDown() {
		try {
			//如果split过程中删除cs失败（-147），split完成后删除cs成功
			if (commSdb.isCollectionSpaceExist(customCSName)) {
				commSdb.dropCollectionSpace(customCSName);
			}
		} catch (BaseException e) {
			Assert.fail(e.getMessage() + "\r\n" + SplitUtils.getKeyStack(e, this));
		} finally {
			if (commSdb != null) {
				commSdb.disconnect();
			}
		}
	}

	class Split extends SdbThreadBase {

		@Override
		public void exec() throws Exception {
			Sequoiadb sdb = null;
			try {
				sdb = new Sequoiadb(coordUrl, "", "");
				DBCollection cl = sdb.getCollectionSpace(customCSName).getCollection(clName);
				cl.split(srcGroupName, destGroupName, (BSONObject) JSON.parse("{sk:100}"), 
						(BSONObject) JSON.parse("{sk:1000}"));				
			} catch (BaseException e) {
				if (e.getErrorCode() != -147) {
					throw e;
				}
			} finally {
				if (sdb != null) {
					sdb.disconnect();
				}
				flag.set(true);
			}
		}

	}
	
	private DBCollection createCSAndCL(){
		if( commSdb.isCollectionSpaceExist(customCSName)){
			commSdb.dropCollectionSpace(customCSName);			
		}
		CollectionSpace customCS = commSdb.createCollectionSpace(customCSName);
		DBCollection cl = customCS.createCollection(clName, (BSONObject) JSON
				.parse("{ShardingKey:{'sk':1},Partition:4096,ShardingType:'hash',Group:'" + srcGroupName + "'}"));
		return cl;		
	}
	
	//insert 5W records
	private void insertData(DBCollection cl) {
		for ( int i = 0; i < 50000; i+=10000){
			List<BSONObject>list = new ArrayList<BSONObject>();	
			for (int j = i + 0; j < i + 10000; j++) {				
				BSONObject obj = (BSONObject) JSON.parse("{sk:" + j +", test:"+"'testasetatatatatat'" + "}");				
				list.add(obj);	
			}
			cl.insert(list);
		}		
	}


}
