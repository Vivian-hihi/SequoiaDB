package com.sequoiadb.metaopr.comm;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Domain;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.commlib.GroupMgr;
import com.sequoiadb.commlib.GroupWrapper;
import com.sequoiadb.commlib.NodeWrapper;
import com.sequoiadb.commlib.SdbTestBase;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.ReliabilityException;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.SkipException;

import java.text.SimpleDateFormat;
import java.util.*;

/**
 * @FileName
 * @Author laojingtang
 * @Date 17-4-20
 * @Version 1.00
 */
public class MyUtil {

    /**
     * 批量产生名字
     *
     * @param preName
     * @param num
     * @return
     */
    public static List<String> createNames(String preName, int num) {
        List<String> names = new ArrayList<>(1000);
        for (int i = 0; i < num; i++) {
            names.add(preName + i);
        }
        return names;
    }

    /**
     * 打印开始时间
     *
     * @param obj
     */
    public static void printBeginTime(Object obj) {
        System.out.println(obj.getClass().getName() + " begin at:"
                + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss").format(new Date()));
    }

    /**
     * 打印结束时间
     *
     * @param obj
     */
    public static void printEndTime(Object obj) {
        System.out.println(obj.getClass().getName() + " end at:"
                + new SimpleDateFormat("YYYY-MM-dd HH:mm:ss").format(new Date()));
    }

    /**
     * 获取连到编目节点的sdb连接
     *
     * @return
     */
    public static Sequoiadb getSdb() {
        return new Sequoiadb(SdbTestBase.coordUrl, "", "");
    }

    /**
     * 关闭连接
     *
     * @param db
     */
    public static void closeDb(Sequoiadb db) {
        if (db.isClosed() == false) {
            db.closeAllCursors();
            db.close();
        }
    }

    /**
     * 检查主备节点是否同步
     */
    public static boolean isCatalogGroupSync() throws ReliabilityException {
        GroupMgr mgr = new GroupMgr();
        GroupWrapper catalogGroup = mgr.getGroupByName("SYSCatalogGroup");
        boolean result = catalogGroup.checkInspect(60);
        mgr.close();
        return result;
    }

    /**
     * 插入给定数量的简单数据到cl
     *
     * @param csName
     * @param clName
     * @param number
     */
    public static void insertSimpleDataIntoCl(String csName, String clName, int number) {
        try (Sequoiadb db = getSdb()) {
            List<BSONObject> list = new ArrayList<>(number);
            for (int i = 0; i < number; i++) {
                list.add(new BasicBSONObject("a", i));
            }
            db.getCollectionSpace(csName)
                    .getCollection(clName)
                    .insert(list);
        }
    }

    /**
     * 获取编目主节点
     *
     * @return
     * @throws ReliabilityException
     */
    public static NodeWrapper getMasterNodeOfCatalog() throws ReliabilityException {
        GroupMgr mgr = new GroupMgr();
        NodeWrapper master = mgr.getGroupByName("SYSCatalogGroup").getMaster();
//        mgr.close();//这里不能close
        return master;
    }

    public static GroupWrapper getCataGroup() throws ReliabilityException {
        GroupMgr mgr = new GroupMgr();
        return mgr.getGroupByName("SYSCatalogGroup");
    }

    public static List<String> getDataGroupNames() {
        List<String> names = null;
        GroupMgr mgr = null;
        try {
            mgr = new GroupMgr();
            names = mgr.getAllDataGroupName();
            mgr.close();
        } catch (ReliabilityException e) {
            throw new BaseException(e.getMessage());
        } finally {
            if (mgr != null)
                mgr.close();
            return names;
        }
    }

    /**
     * 获取编目备节点
     *
     * @return
     * @throws ReliabilityException
     */
    public static NodeWrapper getSlaveNodeOfCatalog() throws ReliabilityException {
        GroupMgr mgr = new GroupMgr();
        NodeWrapper slave = mgr.getGroupByName("SYSCatalogGroup").getSlave();
//        mgr.close();这里不能close
        return slave;
    }

    /**
     * 在多个cs上都创建一个cl
     *
     * @param csnames
     * @param clname
     * @return
     */
    public static int createClInManyCs(List<String> csnames, String clname) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            for (String name : csnames) {
                try {
                    db.getCollectionSpace(name).createCollection(clname);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }

    /**
     * 在一个cs上批量创建cl
     *
     * @param csname
     * @param clNames
     * @return
     */
    public static int createClInSingleCs(String csname, List<String> clNames) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            CollectionSpace cs = db.getCollectionSpace(csname);
            for (String name : clNames) {
                try {
                    cs.createCollection(name);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }

    /**
     * 创建单个cl
     *
     * @param csName
     * @param clName
     * @return
     */
    public static int createCl(String csName, String clName) {
        try (Sequoiadb db = getSdb()) {
            db.getCollectionSpace(csName)
                    .createCollection(clName);
            return 1;
        } catch (BaseException e) {
            return 0;
        }
    }

    /**
     * 批量创建cs
     *
     * @param names
     * @return
     */
    public static int createCS(List<String> names) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            for (String name : names) {
                try {
                    db.createCollectionSpace(name);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }


    /**
     * 创建cs，并指定domain
     *
     * @param csName
     * @param domainName
     * @return
     */
    public static int createCS(String csName, String domainName) {
        try (Sequoiadb db = getSdb()) {
            BSONObject options = (BSONObject) JSON.parse("{'Domain':'" + domainName + "'}");
            db.createCollectionSpace(csName, options);
            return 1;
        } catch (BaseException e) {
            return 0;
        }
    }


    public static boolean createCS(String name) {
        try (Sequoiadb db = getSdb()) {
            db.createCollectionSpace(name);
            return true;
        } catch (BaseException e) {
            return false;
        }
    }

    /**
     * 创建domain，包含groupName1,groupName2
     *
     * @param domainName
     * @param groupName1
     * @param groupName2
     * @return
     */
    public static int createDomain(String domainName, String groupName1, String groupName2) {
        try (Sequoiadb db = getSdb()) {
            BSONObject options = (BSONObject) JSON.parse("{'Groups':['" + groupName1 + "','" + groupName2 + "']}");
            db.createDomain(domainName, options);
            return 1;
        } catch (BaseException e) {
            return 0;
        }
    }

    /**
     * 创建domain，包含groupName1,groupName2,并指定autosplit为true
     *
     * @param domainName
     * @param groupName1
     * @param groupName2
     * @return
     */
    public static int createDomainAutoSplit(String domainName, String groupName1, String groupName2) {
        try (Sequoiadb db = getSdb()) {
            BSONObject options = (BSONObject) JSON.parse("{'Groups':['" + groupName1 + "','" + groupName2 + "'],'AutoSplit':true})");
            db.createDomain(domainName, options);
            return 1;
        } catch (BaseException e) {
            return 0;
        }
    }

    /**
     * 修改domain属性
     *
     * @param domain
     * @param groupName1
     * @param groupName2
     */
    public static void alterDomain(Domain domain, String groupName1, String groupName2) {
        BSONObject options = (BSONObject) JSON.parse("{'Groups':['" + groupName1 + "','" + groupName2 + "'],'AutoSplit':true})");
        domain.alterDomain(options);
    }

    /**
     * 修改domain属性
     *
     * @param domainName
     * @param groupName1
     * @param groupName2
     */
    public static boolean alterDomain(String domainName, String groupName1, String groupName2) {
        try (Sequoiadb db = getSdb()) {
            Domain domain = db.getDomain(domainName);
            BSONObject options = (BSONObject) JSON.parse("{'Groups':['" + groupName1 + "','" + groupName2 + "'],'AutoSplit':true})");
            domain.alterDomain(options);
            return true;
        } catch (BaseException e) {
            return false;
        }
    }


    /**
     * 批量创建domain
     *
     * @param domainNames
     * @return
     */
    public static int createDomains(List<String> domainNames) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            List<String> groupNames = getDataGroupNames();
            String groupName1 = groupNames.get(0);
            String groupName2 = groupNames.get(1);
            BSONObject options = (BSONObject) JSON.parse("{'Groups':['" + groupName1 + "','" + groupName2 + "']}");
            for (String name : domainNames) {
                try {
                    db.createDomain(name, options);
                    count++;
                } catch (BaseException e) {
                }
            }
        } finally {
            return count;
        }
    }

    /**
     * 该方法适用于删除多个cs有相同cl name的情况
     *
     * @param csNames
     * @param clName
     * @return
     */
    public static int dropSingleClManyCs(List<String> csNames, String clName) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            for (String name :
                    csNames) {
                try {
                    db.getCollectionSpace(name).dropCollection(clName);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }

    public static int dropCS(String csName) {
        List<String> list = new ArrayList<>(1);
        list.add(csName);
        return dropCS(list);
    }

    public static int dropDomain(String domainName) {
        try (Sequoiadb db = getSdb()) {
            db.dropDomain(domainName);
            return 1;
        } catch (BaseException e) {
            return 0;
        }
    }

    public static int dropDomain(List<String> domains) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            for (String name : domains) {
                try {
                    db.dropDomain(name);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }

    /**
     * 删除cs
     *
     * @param csNames
     * @return
     */
    public static int dropCS(List<String> csNames) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            for (String name :
                    csNames) {
                try {
                    db.dropCollectionSpace(name);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }

    /**
     * 给定的domain是否已经全部删除
     *
     * @param domains
     * @return
     */
    public static boolean isDomainsDeleted(List<String> domains) {
        try (Sequoiadb db = getSdb()) {
            for (String domain : domains) {
                if (db.isDomainExist(domain))
                    return false;
            }
        }
        return true;
    }

    public static boolean isDomainAllCreated(List<String> domains) {
        try (Sequoiadb db = getSdb()) {
            for (String domain : domains) {
                if (db.isDomainExist(domain) == false)
                    return false;
            }
        }
        return true;
    }


    /**
     * 在指定cs上删除一个或多个cl，返回删除的个数
     *
     * @param csName
     * @param clNames
     * @return
     */
    public static int dropCls(String csName, List<String> clNames) {
        int count = 0;
        try (Sequoiadb db = getSdb()) {
            CollectionSpace cs = db.getCollectionSpace(csName);
            for (String clName : clNames) {
                try {
                    cs.dropCollection(clName);
                    count++;
                } catch (BaseException e) {
                }
            }
        }
        return count;
    }

    /**
     * 检查数组里的cs是否全部被删除
     *
     * @param csNames
     * @return
     */
    public static boolean isCsAllDeleted(List<String> csNames) {
        try (Sequoiadb db = getSdb()) {
            for (String name : csNames) {
                boolean isExist = db.isCollectionSpaceExist(name);
                if (isExist == true)
                    return false;
            }
        }
        return true;
    }

    /**
     * 检查数组的cs是否全部被创建
     *
     * @param csNames
     * @return
     */
    public static boolean isCsAllCreated(List<String> csNames) {
        try (Sequoiadb db = getSdb()) {
            for (String csName : csNames) {
                if (db.isCollectionSpaceExist(csName) == false)
                    return false;
            }
            return true;
        }
    }

    public static boolean isCsExisted(String csName) {
        boolean flag = false;
        try (Sequoiadb db = getSdb()) {
            DBCursor cursor = db.listCollectionSpaces();
            while (cursor.hasNext())
                if (cursor.getNext().get("Name").equals(csName))
                    flag = true;
        } finally {
            return flag;
        }
    }


    /**
     * 检查当前集群环境是否可用
     *
     * @return
     */
    public static boolean checkBusiness() {
        GroupMgr groupMgr = null;
        try {
            groupMgr = new GroupMgr();
            if (groupMgr.checkBusiness() == true)
                return true;
            else
                throw new SkipException("当前环境异常，GroupMgr.checkBusiness()==false，跳过该用例。");
        } catch (ReliabilityException e) {
            throw new SkipException("当前环境异常，GroupMgr.checkBusiness()==false，跳过该用例。");
        } finally {
            if (groupMgr != null)
                groupMgr.close();
        }
    }

    /**
     * 把集合里记录清空
     *
     * @param csName
     * @param clName
     * @return
     */
    public static boolean deleteAllInCl(String csName, String clName) {
        try (Sequoiadb db = getSdb()) {
            db.getCollectionSpace(csName)
                    .getCollection(clName).delete((BSONObject) null);
            return true;
        } catch (BaseException e) {
            return false;
        }
    }

    /**
     * 直连指定数据组的主节点，获取指定cl的记录数量。
     * 当有异常抛出时，返回0。调用时需要注意，这样的处理是否符合你程序的处理逻辑。
     *
     * @param groupName
     * @param csName
     * @param clName
     * @return
     * @throws ReliabilityException
     */
    public static long getClCountFromGroupMaster(String groupName, String csName, String clName) {
        GroupMgr mgr = null;
        Sequoiadb db = null;
        try {
            mgr = new GroupMgr();
            GroupWrapper groupWrapper = mgr.getGroupByName(groupName);
            NodeWrapper node = groupWrapper.getMaster();
            if(node==null)
                return 0;
            db = node.connect();
            return db.getCollectionSpace(csName).getCollection(clName).getCount();
        } catch (BaseException e) {
            return 0;
        } catch (ReliabilityException e) {
            return 0;
        } finally {
            if (mgr != null)
                mgr.close();
            if (db != null)
                db.close();
        }
    }

    /**
     * 判断数组里的cl是否全部删除
     *
     * @param csName
     * @param clNames
     * @return
     */
    public static boolean isClAllDeleted(String csName, List<String> clNames) {
        try (Sequoiadb db = getSdb()) {
            CollectionSpace cs = db.getCollectionSpace(csName);
            for (String clName : clNames) {
                if (cs.isCollectionExist(clName))
                    return false;
            }
            return true;
        }
    }

    /**
     * 判断数组的cl是否全部创建
     *
     * @return
     */
    public static boolean isClAllCreated(String csName, List<String> clNames) {
        try (Sequoiadb db = getSdb()) {
            CollectionSpace cs = db.getCollectionSpace(csName);
            for (String name : clNames) {
                if (cs.isCollectionExist(name) == false)
                    return false;
            }
        } catch (BaseException e) {
            return false;
        }
        return true;
    }
}
