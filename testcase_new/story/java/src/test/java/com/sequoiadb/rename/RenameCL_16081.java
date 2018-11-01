package com.sequoiadb.rename;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCL_16081.java 修改cs名和修改cl名并发 
 * @author luweikang
 * @date 2018年10月17日
 * @review  wuyan 2018.10.31
 */
public class RenameCL_16081 extends SdbTestBase{
	
	private String csName = "renameCS_16081";
	private String clNameA = "rename_CL_16081A";
	private String clNameB = "rename_CL_16081B";
	private String newCLName= "rename_CL_16081_new";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	private DBCollection clA = null;
	private DBCollection clB = null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		cs = sdb.createCollectionSpace(csName);//TODO:1.非特殊情况建议用公共CS，不需要再创建cs
		clA = cs.createCollection(clNameA);
		clB = cs.createCollection(clNameB);
		RenameUtil.insertData(clA, recordNum);
		RenameUtil.insertData(clB, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCLAThread reCLANameThread = new RenameCLAThread();
		RenameCLBThread reCLBNameThread = new RenameCLBThread();
		
		reCLANameThread.start();
		reCLBNameThread.start();
		
		boolean clARe = reCLANameThread.isSuccess();
		boolean clBRe = reCLBNameThread.isSuccess();
		
		Sequoiadb db = null; //TODO:2、可以用放在try里面，使用jkd1.7新增资源释放接口，不需要写finally
		try{//TODO:3、这里不需要再重新newdb了吧，既然sdb已作为全局变量，可以直接用sdb
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			if(clARe && !clBRe){//TODO:4、变量名尽量不要缩写，命名建议反应所代表的意义
				RenameUtil.checkRenameCLResult(db, csName, clNameA, newCLName);
				RenameUtil.checkRenameCLResult(db, csName, "16081NotExistCLB", clNameB);
			}else if(!clARe && clBRe){
				RenameUtil.checkRenameCLResult(db, csName, clNameB, newCLName);
				RenameUtil.checkRenameCLResult(db, csName, "16081NotExistCLA", clNameA);
			}else if(!clARe && !clBRe){
				Assert.fail("rename cl name to the same name, all failed");
			}else{
				Assert.fail("rename cl name to the same name, all success");
			}//TODO:5、上述代码存在同样问题，如果某个线程失败，没有捕获异常并判断是否符合预期
		} finally{
			db.close();
		}
	}
	
	@AfterClass
	public void tearDown(){
		CommLib.clearCS(sdb, csName);
		if(sdb!=null){//TODO：6、sdb建议在finally里面关闭，如果上述操作步骤失败就不会执行
			sdb.close();
		}
	}
	
	private class RenameCLAThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {//TODO:7、同上，建议new db使用自动释放资源方式，放在try条件中
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clNameA, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
	private class RenameCLBThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			try {
				CollectionSpace cs = db.getCollectionSpace(csName);
				cs.renameCollection(clNameB, newCLName);
			}finally {
				db.close();
			}
		}
	}
	
}
