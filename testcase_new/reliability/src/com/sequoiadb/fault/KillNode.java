package com.sequoiadb.fault ;

import org.testng.annotations.Test ;

import com.sequoiadb.commlib.SdbTestBase ;
import com.sequoiadb.commlib.Ssh ;
import com.sequoiadb.exception.FaultException ;
import com.sequoiadb.exception.ReliabilityException ;

public class KillNode extends Fault {
    private String hostName ;
    private String svcName ;
    private String user ;
    private String passwd ;
    private String pid = "-1" ;
    private Ssh ssh ;
    private String remotePath ;
    private int port ;
    private final String localScriptPath = "./script" ;
    private final String scriptName = "killNode.sh" ;

    @Test
    public static void test() throws ReliabilityException {
        KillNode kn = new KillNode( "192.168.31.31", "11830" ) ;
        kn.setUser( "root" ) ;
        kn.setPwd( "sequoiadb" ) ;
        kn.setRemotePath( "/tmp" ) ;
        kn.init() ;
        kn.make() ;
        for ( int i = 0; i < 10; i++ ) {
            try {
                Thread.sleep( 1000 ) ;
            } catch ( InterruptedException e ) {
                // TODO Auto-generated catch block
                e.printStackTrace() ;
            }
            System.out.println( "check make:" + kn.checkMakeResult() ) ;

        }
        kn.restore() ;
        System.out.println( kn.checkRestoreResult() ) ;
        kn.fini() ;
    }

    // for debug
    public void setPort( int port ) {
        this.port = port ;
    }

    // for debug
    public void setUser( String user ) {
        this.user = user ;
    }

    // for debug
    public void setPwd( String passwd ) {
        this.passwd = passwd ;
    }

    // for debug
    public void setRemotePath( String remotePath ) {
        this.remotePath = remotePath ;
    }

    public KillNode( String hostName, String svcName ) {
        super( "killNode" ) ;
        this.hostName = hostName ;
        this.svcName = svcName ;
        this.user = SdbTestBase.remoteUser ;
        this.passwd = SdbTestBase.remotePwd ;
        this.remotePath = SdbTestBase.workDir ;
        this.port = 22 ;
    }

    @Override
    public void make() throws FaultException {
        try {
            ssh.exec( remotePath + "/" + scriptName + " " + svcName ) ;
            pid = ssh.getStdout().substring( 0, ssh.getStdout().length() - 1 ) ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    @Override
    public boolean checkMakeResult() throws FaultException {
        if ( pid.equals( "-1" ) ) {
            return false ;
        }
        try {
            ssh.exec( "lsof -i:" + svcName + " | sed '1d' | awk '{print $2}'" ) ;
            if ( ssh.getStdout().length() <= 0 ) {
                return false ;
            }
            String currentPid = ssh.getStdout().substring( 0,
                    ssh.getStdout().length() - 1 ) ;
            if ( !pid.equals( currentPid ) ) {
                pid = currentPid ;
                return true ;
            } else {
                return false ;
            }
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    @Override
    public void restore() throws FaultException {

        // TODO:如果节点没有异常重启，需要去启动这个节点吗？
    }

    @Override
    public boolean checkRestoreResult() throws FaultException {
        try {
            ssh.exec( "lsof -i:" + svcName + " | sed '1d' | awk '{print $2}'" ) ;
            if ( ssh.getStdout().length() <= 0 ) {
                return false ;
            }
            return true ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
    }

    @Override
    public boolean init() throws FaultException {
        try {
            if ( ssh == null ) {
                ssh = new Ssh( hostName, user, passwd, port ) ;
            }
            ssh.scpTo( localScriptPath + "/" + scriptName, remotePath ) ;
            ssh.exec( "chmod 777 " + remotePath + "/" + scriptName ) ;
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
        return true ;
    }

    @Override
    public boolean fini() throws FaultException {
        try {
            if ( ssh != null ) {
                ssh.exec( "rm -rf " + remotePath + "/" + scriptName ) ;
                ssh.close() ;
            }
        } catch ( ReliabilityException e ) {
            throw new FaultException( e ) ;
        }
        return true ;
    }

}
