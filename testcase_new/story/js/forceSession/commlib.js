/**
 * 通过nodename获取 GroupID、GroupName、NodeID
 * 备注：协调节点除外
 * @param nodename
 * @constructor
 */
function InfoByNodeName(nodename) {

    this.HostName = nodename.split(":")[0];
    this.svcname = nodename.split(":")[1];
    this.dataGroup = commGetGroups(db, true, "", true, true, true).concat(commGetGroups(db, true, "SYSCatalogGroup", false, true, true));
    /**
     * 获取GroupID
     * @returns GroupID
     */
    this.getGroupIDByNodeName = function () {
        for (var i = 0; i < this.dataGroup.length; i++) {
            for (var j = 1; j < this.dataGroup[i].length; j++) {
                // println(this.dataGroup[i][j].HostName +"=="+ this.HostName);
                // println(this.dataGroup[i][j].svcname +"=="+ this.svcname);
                if (this.dataGroup[i][j].HostName == this.HostName && this.dataGroup[i][j].svcname == this.svcname) {
                    return this.dataGroup[i][0].GroupID;
                }
            }
        }
        return null;
    }

    /**
     * 获取GroupName
     * @returns GroupName
     */
    this.getGroupNameByNodeName = function () {
        for (var i = 0; i < this.dataGroup.length; i++) {
            for (var j = 1; j < this.dataGroup[i].length; j++) {
                if (this.dataGroup[i][j].HostName == this.HostName && this.dataGroup[i][j].svcname == this.svcname) {
                    return this.dataGroup[i][0].GroupName;
                }
            }
        }
        return null;
    }

    /**
     * 获取NodeId
     * @returns NodeId
     */
    this.getNodeIdByNodeName = function () {
        for (var i = 0; i < this.dataGroup.length; i++) {
            for (var j = 1; j < this.dataGroup[i].length; j++) {
                if (this.dataGroup[i][j].HostName == this.HostName && this.dataGroup[i][j].svcname == this.svcname) {
                    return this.dataGroup[i][j].NodeID;
                }
            }
        }
        return null;
    }
}