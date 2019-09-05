package com.sequoiadb.faultmodel.fault.impl;

import com.sequoiadb.faultmodel.fault.FaultBase;
import com.sequoiadb.faultmodel.fault.FaultName;

public class DiskLimit extends FaultBase {

    public DiskLimit(String hostName, String svcName) {
        super(hostName, svcName, FaultName.DISKLIMIT);
    }
}
