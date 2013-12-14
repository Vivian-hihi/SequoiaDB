package com.sequoia.hadoop.test;

import java.util.ArrayList;
import java.util.List;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.junit.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Sequoiadb;

public class InitialData {
	public static final String conn = "192.168.20.113:40000";
	public static final String csInputName = "testHadoopCS"; // hadoopInputCS
	public static final String cInputName = "testHadoopCL"; // hadoopInputC
	public static final String csOutputName = "hadoopOutputCS";
	public static final String cOutputName = "hadoopOutputC";
	public static final String csPigOutputName = "pigOutCs";
	public static final String cPigOutputName = "pigOutC";
	public static final String USERNAME = "";
	public static final String PASSWORD = "";

	private Sequoiadb sdb;
	private CollectionSpace csInput;
	private CollectionSpace csOutput;
	private DBCollection dbcInput;
	private DBCollection dbcOutput;

	public InitialData() {
		sdb = new Sequoiadb(conn, "", "");
	}

	@Test
	public void initialDB() {
		int insertNumber = 4;
		if (!sdb.isCollectionSpaceExist(csInputName))
			csInput = sdb.createCollectionSpace(csInputName);
		else
			csInput = sdb.getCollectionSpace(csInputName);
		// if (sdb.isCollectionSpaceExist(csOutputName))
		// sdb.dropCollectionSpace(csOutputName);
		// if (sdb.isCollectionSpaceExist("pigOutCs"))
		// sdb.dropCollectionSpace("pigOutCs");
		if (csInput.isCollectionExist(cInputName))
			dbcInput = csInput.getCollection(cInputName);
		else
			dbcInput = csInput.createCollection(cInputName);

		List<BSONObject> list = new ArrayList<BSONObject>();
		while (insertNumber > 0) {
			int j = 0;
			while (j < 100) {
				list.clear();
				for (int i = 0; i < 1000; ++i) {
					BSONObject o = new BasicBSONObject();
					o.put("name", "name_" + i);
					o.put("age", i);
					o.put("street", "street_" + i);
					list.add(o);
				}
				dbcInput.bulkInsert(list, DBCollection.FLG_INSERT_CONTONDUP);
				++j;
			}
			--insertNumber;
		}
		System.out.println("Initial data in DB");
		sdb.disconnect();
	}

	@Test
	public void queryDBInput() {
		System.out.println("Query data in DBInput");
		csInput = sdb.getCollectionSpace(csInputName);
		dbcInput = csInput.getCollection(cInputName);
		DBCursor cursor = dbcInput.query();
		while (cursor.hasNext()) {
			BSONObject o = cursor.getNext();
			System.out.println(o.toString());
		}
		sdb.disconnect();
	}

	@Test
	public void queryDBOutput() {
		System.out.println("Query data in DBOutput");
		csOutput = sdb.getCollectionSpace(csOutputName);
		dbcOutput = csOutput.getCollection(cOutputName);
		DBQuery dbquery = new DBQuery();
		DBCursor cursor = dbcOutput.query(dbquery);
		while (cursor.hasNext()) {
			BSONObject o = cursor.getNext();
			System.out.println(o.toString());
		}
		sdb.disconnect();
	}

	@Test
	public void queryPigOutPut() {
		System.out.println("Query data in DBOutput");
		csOutput = sdb.getCollectionSpace(csPigOutputName);
		dbcOutput = csOutput.getCollection(cPigOutputName);
		DBQuery dbquery = new DBQuery();
		DBCursor cursor = dbcOutput.query(dbquery);
		if (cursor == null)
			System.out.println("no data in" + cPigOutputName);
		else
			while (cursor.hasNext()) {
				BSONObject o = cursor.getNext();
				System.out.println(o.toString());
			}
		sdb.disconnect();
	}

}
