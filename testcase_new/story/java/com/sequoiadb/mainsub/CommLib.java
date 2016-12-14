package com.sequoiadb.mainsub;

import java.util.ArrayList;

import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;

public class CommLib {
	/**
	 * get dataGroupNames
	 * @param sdb
	 * @return dataGroupNames
	 */
	public static ArrayList<String> getDataGroupNames(Sequoiadb sdb){
		ArrayList<String> dataGroupNames = new ArrayList<String>();
		try{
			dataGroupNames = sdb.getReplicaGroupNames();
			dataGroupNames.remove("SYSCatalogGroup");
			dataGroupNames.remove("SYSCoord");
			}catch(BaseException e){
				Assert.fail("Failed to get dataGroupsName. ErrorMsg:\n" + e.getMessage());
			}
		return dataGroupNames;
	}
}
