package com.sequoiadb.faultmodel.fault.impl;

import com.sequoiadb.faultmodel.fault.FaultBase;
import com.sequoiadb.faultmodel.fault.FaultName;

public class MemoryLimit extends FaultBase {

    public MemoryLimit(String hostName, String svcName) {
        super(hostName, svcName, FaultName.MEMORYLIMIT);
    }
}
