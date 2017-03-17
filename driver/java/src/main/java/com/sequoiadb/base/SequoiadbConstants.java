/*
 * Copyright 2017 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

package com.sequoiadb.base;

public class SequoiadbConstants {
    public final static String FIELD_NAME_NAME = "Name";
    public final static String FIELD_NAME_OLDNAME = "OldName";
    public final static String FIELD_NAME_NEWNAME = "NewName";
    public final static String FIELD_NAME_HOST = "HostName";
    public final static String FIELD_NAME_GROUPNAME = "GroupName";
    public final static String FIELD_NAME_GROUPSERVICE = "Service";
    public final static String FIELD_NAME_GROUP = "Group";
    public final static String FIELD_NAME_NODEID = "NodeID";
    public final static String FIELD_NAME_GROUPID = "GroupID";
    public final static String FIELD_NAME_PRIMARY = "PrimaryNode";
    public final static String FIELD_NAME_SERVICENAME = "Name";
    public final static String FIELD_NAME_SERVICETYPE = "Type";
    public final static String FIELD_NAME_SOURCE = "Source";
    public final static String FIELD_NAME_TARGET = "Target";
    public final static String FIELD_NAME_SPLITQUERY = "SplitQuery";
    public final static String FIELD_NAME_SPLITENDQUERY = "SplitEndQuery";
    public final static String FIELD_NAME_SPLITPERCENT = "SplitPercent";
    public final static String FIELD_NAME_PATH = "Path";
    public final static String FIELD_NAME_FUNC = "func";
    public final static String FIELD_NAME_SUBCLNAME = "SubCLName";
    public final static String FIELD_NAME_HINT = "Hint";
    public final static String FIELD_NAME_ASYNC = "Async";
    public final static String FIELD_NAME_TASKID = "TaskID";
    public final static String FIELD_NAME_OPTIONS = "Options";
    public final static String FIELD_NAME_DOMAIN = "Domain";

    public final static String FIELD_NAME_ARGS = "Args";
    public final static String FIELD_NAME_ALTER = "Alter";
    public final static String FIELD_NAME_ALTER_TYPE = "AlterType";
    public final static String FIELD_NAME_VERSION = "Version";

    public final static int SDB_ALTER_VERSION = 1;
    public final static String SDB_ALTER_CL = "collection";
    public final static String SDB_ALTER_CRT_ID_INDEX = "create id index";
    public final static String SDB_ALTER_DROP_ID_INDEX = "drop id index";

    final static String FIELD_NAME_MODIFY = "$Modify";
    final static String FIELD_NAME_OP = "OP";
    final static String FIELD_NAME_OP_UPDATE = "Update";
    final static String FIELD_NAME_OP_REMOVE = "Remove";
    final static String FIELD_NAME_RETURNNEW = "ReturnNew";
    final static String FIELD_OP_VALUE_UPDATE = "update";
    final static String FIELD_OP_VALUE_REMOVE = "remove";

    final static String FIELD_NAME_SET_ON_INSERT = "$SetOnInsert";

    final static String FMP_FUNC_TYPE = "funcType";
    final static String FIELD_COLLECTION = "Collection";
    final static String FIELD_TOTAL = "Total";
    final static String FIELD_INDEX = "Index";
    final static String FIELD_NAME_PREFERED_INSTANCE = "PreferedInstance";
    final static String FIELD_NAME_RETYE = "ReturnType";

    final static String FIELD_NAME_ONLY_DETACH = "OnlyDetach";
    final static String FIELD_NAME_ONLY_ATTACH = "OnlyAttach";

    public final static String IXM_NAME = "name";
    public final static String IXM_KEY = "key";
    public final static String IXM_UNIQUE = "unique";
    public final static String IXM_ENFORCED = "enforced";
    public final static String IXM_INDEXDEF = "IndexDef";
    public final static String IXM_FIELD_NAME_SORT_BUFFER_SIZE = "SortBufferSize";
    public final static int IXM_SORT_BUFFER_DEFAULT_SIZE = 64;

    public final static String PMD_OPTION_SVCNAME = "svcname";
    public final static String PMD_OPTION_DBPATH = "dbpath";

    public final static String OID = "_id";

    public final static int FLG_UPDATE_UPSERT = 0x00000001;

    public enum PreferInstanceType {
        INS_TYPE_MIN(0),
        INS_NODE_1(1),
        INS_NODE_2(2),
        INS_NODE_3(3),
        INS_NODE_4(4),
        INS_NODE_5(5),
        INS_NODE_6(6),
        INS_NODE_7(7),
        INS_MASTER(8),
        INS_SLAVE(9),
        INS_ANYONE(10),
        INS_TYPE_MAX(11);

        private int typeCode;

        PreferInstanceType(int typeCode) {
            this.typeCode = typeCode;
        }

        public int getCode() {
            return typeCode;
        }
    }
}
