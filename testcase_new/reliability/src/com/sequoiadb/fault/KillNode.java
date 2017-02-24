package com.sequoiadb.fault ;

import com.sequoiadb.commlib.Ssh ;
import com.sequoiadb.exception.CommException ;
import com.sequoiadb.exception.ReliabilityException ;

public class KillNode extends Fault {

    private String hostName ;
    private String svcName ;
    private String user ;
    private String passwd ;
    private int pid = -1 ;

    public KillNode( String hostName, String svcName, String user, String passwd ) {
        super( "killNode" ) ;
        this.hostName = hostName ;
        this.svcName = svcName ;
        this.user = user ;
        this.passwd = passwd ;
    }

    @Override
    public void make() throws ReliabilityException{
        Ssh ssh = new Ssh( hostName, user, passwd ) ;

        ssh.exec( "lsof -i:" + svcName + " | sed '1d' | awk '{print $2}'" ) ;
        if ( ssh.getStdout().length() <= 0 ) {
            throw new CommException( "Get PID Faile,Check svcName Is Correct:"
                    + svcName ) ;
        }
        pid = Integer.parseInt( ssh.getStdout().substring( 0,
                ssh.getStdout().length() - 1 ) ) ;
        ssh.exec( "kill -9 " + pid ) ;
        ssh.close() ;

    }

    @Override
    public boolean checkMakeResult() throws ReliabilityException{
        if ( pid == -1 ) {
            System.out.println( "pid -1" ) ;
            return false ;
        }
        Ssh ssh = new Ssh( hostName, user, passwd ) ;
        ssh.exec( "lsof -i:" + svcName + " | sed '1d' | awk '{print $2}'" ) ;
        ssh.close() ;
        if ( ssh.getStdout().length() <= 0 ) {
            System.out.println( "pid not find" ) ;
            return false ;
        }
        int currentPid = Integer.parseInt( ssh.getStdout().substring( 0,
                ssh.getStdout().length() - 1 ) ) ;
        if ( pid != currentPid ) {
            pid = currentPid ;
            return true ;
        } else {
            System.out.println( "pid equal" ) ;
            return false ;
        }
    }

    @Override
    public void restore() throws ReliabilityException {

        // TODO:如果节点没有异常重启，需要去启动这个节点吗？
    }

    @Override
    public boolean checkRestoreResult() throws ReliabilityException {
        // TODO:
        return true ;
    }

}
