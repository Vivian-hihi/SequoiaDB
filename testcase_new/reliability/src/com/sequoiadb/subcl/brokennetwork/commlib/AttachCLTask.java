package com.sequoiadb.subcl.brokennetwork.commlib;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.task.FaultMakeTask;
import com.sequoiadb.task.OperateTask;

public class AttachCLTask extends OperateTask {
    private int attachedSclCnt = 0;
    private String safeUrl = null;
    private String mclName = null;
    private int[] expErrCodes = null;
    
    public AttachCLTask(String name, String mclName, String safeUrl, int[] expErrCodes) {
        super(name);
        this.mclName = mclName;
        this.safeUrl = safeUrl;
        this.expErrCodes = expErrCodes;
    }

    @Override
    public void exec() throws Exception {
        Sequoiadb db = null;
        try {
            db = new Sequoiadb(safeUrl, "", "");
            CollectionSpace cs = db.getCollectionSpace(SdbTestBase.csName);
            DBCollection mcl = cs.getCollection(mclName);
            int rangeStart = 0;
            for (int i = 0; i < Utils.SCLNUM; i++) {
                int rangeEnd = rangeStart + Utils.RANGE_WIDTH;
                String sclFullName = SdbTestBase.csName + "." + mclName + "_" + i;
                mcl.attachCollection(sclFullName, (BSONObject) JSON.parse("{ LowBound: { a: " + rangeStart
                        + " }, " + "UpBound: { a: " + rangeEnd + " } }"));
                rangeStart += Utils.RANGE_WIDTH;
                attachedSclCnt++;
            }
        } catch (BaseException e) {
            int actErrCode = e.getErrorCode();
            boolean isExpected = false;
            for (int i = 0; i < expErrCodes.length; i++) {
                if (actErrCode == expErrCodes[i]) {
                    isExpected = true;
                    break;
                }
            }
            if (!isExpected) { throw e; }
        } finally {
            if (db != null) {
                db.disconnect();
            }
        }
    }
    
    public int getAttachedSclCnt() {
        return attachedSclCnt;
    }

    // 这是几乎通用的，建议抽出。
    @Override
    public void faultNotify(BSONObject status) {
        if (status.get(FaultMakeTask.MAKE_RESULT) == OperateTask.faultStatus.MAKEFAILURE) {
            Assert.fail("fail to make fault");
        }
        if (status.get(FaultMakeTask.RESTORE_RESULT) == OperateTask.faultStatus.RESTOREFAILURE) {
            Assert.fail("fail to restore fault");
        }
    }
}  