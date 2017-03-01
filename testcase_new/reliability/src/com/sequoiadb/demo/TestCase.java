package com.sequoiadb.demo ;

import java.util.List ;
import java.util.concurrent.atomic.AtomicBoolean ;

import org.bson.BSONObject ;
import org.bson.util.JSON ;
import org.testng.Assert ;
import org.testng.SkipException ;
import org.testng.annotations.AfterClass ;
import org.testng.annotations.BeforeClass ;
import org.testng.annotations.Test ;

import com.sequoiadb.base.DBCollection ;
import com.sequoiadb.base.Sequoiadb ;
import com.sequoiadb.commlib.GroupMgr ;
import com.sequoiadb.commlib.NodeWrapper ;
import com.sequoiadb.commlib.SdbTestBase ;
import com.sequoiadb.exception.BaseException ;
import com.sequoiadb.exception.ReliabilityException ;
import com.sequoiadb.fault.KillNode ;
import com.sequoiadb.task.FaultMakeTask ;
import com.sequoiadb.task.OperateTask ;
import com.sequoiadb.task.TaskMgr ;

public class TestCase extends SdbTestBase {
    private TaskMgr manager = null ;
    private String srcGroupName ;
    private String destGroupName ;

    @BeforeClass
    public void setUp() {
        try {
            if ( !GroupMgr.getInstance().checkBusiness() ) {
                throw new SkipException( "cluster check failed" ) ;
            }

            List< String > groupNames = GroupMgr.getInstance()
                    .getAllDataGroupName() ;
            if ( groupNames.size() < 2 ) {
                throw new SkipException( "group number less than 2" ) ;
            }
            srcGroupName = groupNames.get( 0 ) ;
            destGroupName = groupNames.get( 1 ) ;
            prepare() ;
        } catch ( ReliabilityException e ) {
            // TODO Auto-generated catch block
            e.printStackTrace() ;
        }
        System.out.println( "setup complete" ) ;
    }

    // 切分时catalog主节点异常重启
    @Test
    public void test() {
        // taskManager
        manager.start() ;
        manager.join() ;
        Assert.assertTrue( manager.getExceptions().isEmpty() , manager.getErrorMsg());
        try {
            manager.fini() ;
        } catch ( ReliabilityException e ) {
            // TODO Auto-generated catch block
            e.printStackTrace() ;
            Assert.fail( e.getMessage() ) ;
        }
        System.out.println( "child thread over:" + manager.getErrorMsg() ) ;

        check() ;
    }

    private void check() {

    }

    private void prepare() throws ReliabilityException {
        NodeWrapper node = GroupMgr.getInstance()
                .getGroupByName( "SYSCatalogGroup" ).getMaster() ;
        System.out.println( "destNode " + node.hostName() + ":"
                + node.svcName() ) ;

        KillNode killnode = new KillNode( node.hostName(), node.hostName() ) ;
        FaultMakeTask faultMaker = new FaultMakeTask( killnode, 6, 30 ) ;

        // OperateTask(insert,split)
        TaskMgr manager = new TaskMgr( faultMaker ) ;
        manager.addTask( "com.sequoiadb.demo.TestCase.Insert" ) ;
        manager.addTask( "com.sequoiadb.demo.TestCase.Split" ) ;
        manager.init() ;
    }

    @AfterClass
    public void tearDown() {
        System.out.println( "tearDown" ) ;
    }

    public class Insert extends OperateTask {
        private AtomicBoolean makeFalg = new AtomicBoolean( false ) ;
        private AtomicBoolean restoreFalg = new AtomicBoolean( false ) ;
        private OperateTask.faultStatus status ;
        private Sequoiadb db = null ;
        private DBCollection cl = null ;
        boolean isExist = false ;
        private int sk = 0 ;
        private String coordUrl ;

        public Insert() {
            super( "insert" ) ;
            this.coordUrl = SdbTestBase.coordUrl ;
            // TODO Auto-generated constructor stub
        }

        @Override
        public void faultMakeNotify( faultStatus status ) {
            System.out.println( super.getName() + " MakeNotify:" + status ) ;
            this.status = status ;
            makeFalg.set( true ) ;
            if ( status != faultStatus.MAKESUCCESS ) {
                isExist = true ;
            }
        }

        @Override
        public void faultRestoreNotify( faultStatus status ) {
            System.out.println( super.getName() + " Restore:" + status ) ;
            this.status = status ;
            restoreFalg.set( true ) ;
            isExist = true ;

        }

        @Override
        public void Do() throws Exception {
            try {
                System.out.println( "isnert start" ) ;
                while ( !isExist ) {
                    if ( restoreFalg.get() ) {
                        break ;
                    }
                    cl.insert( ( BSONObject ) JSON.parse( "{sk:" + sk + "}" ) ) ;
                    sk++ ;
                }
                System.out.println( "isnert complete" ) ;
            } catch ( Exception e ) {
                throw e ;
            }
        }

        @Override
        public boolean init() {
            // TODO Auto-generated method stub
            System.out.println( "init" ) ;
            try {
                db = new Sequoiadb( this.coordUrl, "", "" ) ;
                db.setSessionAttr( ( BSONObject ) JSON
                        .parse( "{PreferedInstance:'M'}" ) ) ;
                cl = db.createCollectionSpace( "testcs" )
                        .createCollection(
                                "testcl",
                                ( BSONObject ) JSON
                                        .parse( "{ShardingKey:{sk:1},Group:'group1'}" ) ) ;

                while ( true ) {
                    cl.insert( ( BSONObject ) JSON.parse( "{sk:" + sk + "}" ) ) ;
                    sk++ ;
                    if ( sk == 1000 ) {
                        break ;
                    }
                }
            } catch ( BaseException e ) {
                System.out.println( e.getMessage() ) ;
                return false ;
            }
            return true ;
        }

        @Override
        public boolean fini() {
            // TODO Auto-generated method stub
            System.out.println( "fini" ) ;
            try {
                System.out.println( "check expected:" + sk + ",actual:"
                        + cl.getCount() ) ;
                db.dropCollectionSpace( "testcs" ) ;
            } catch ( BaseException e ) {
                return false ;
            } finally {
                db.disconnect() ;
            }
            return true ;
        }

    }

    public class Split extends OperateTask {
        private AtomicBoolean makeFalg = new AtomicBoolean( false ) ;
        private AtomicBoolean restoreFalg = new AtomicBoolean( false ) ;
        private OperateTask.faultStatus status ;
        private Sequoiadb db = null ;
        private String coordUrl ;

        public Split() {
            super( "split" ) ;
            this.coordUrl = SdbTestBase.coordUrl ;
            // TODO Auto-generated constructor stub
        }

        @Override
        public void faultMakeNotify( faultStatus status ) {
            System.out.println( super.getName() + " MakeNotify:" + status ) ;
            this.status = status ;
            makeFalg.set( true ) ;

        }

        @Override
        public void faultRestoreNotify( faultStatus status ) {
            System.out.println( super.getName() + " Restore:" + status ) ;
            this.status = status ;
            restoreFalg.set( true ) ;
        }

        @Override
        public void Do() {
            try {
                System.out.println( "split start" ) ;
                DBCollection cl = db.getCollectionSpace( "testcs" )
                        .getCollection( "testcl" ) ;
                cl.split( srcGroupName, destGroupName, 90 ) ;
                System.out.println( "split complete" ) ;
            } catch ( Exception e ) {
                e.printStackTrace() ;
            }
        }

        @Override
        public boolean init() {
            // TODO Auto-generated method stub
            System.out.println( "init" ) ;
            try {
                db = new Sequoiadb( this.coordUrl, "", "" ) ;
            } catch ( BaseException e ) {
                return false ;
            }
            return true ;
        }

        @Override
        public boolean fini() {
            // TODO Auto-generated method stub
            System.out.println( "fini" ) ;
            try {
                if ( db != null ) {
                    db.disconnect() ;
                }
            } catch ( BaseException e ) {
                System.out.println( e.getMessage() ) ;
                return false ;
            }
            return true ;
        }

    }

}
