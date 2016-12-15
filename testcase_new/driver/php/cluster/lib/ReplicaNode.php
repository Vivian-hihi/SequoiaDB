/****************************************************
@description:      ReplicaNode operate, warp class
@testlink cases:   seqDB-7636-7644
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
class ReplicaNode
{
    protected $hostName ;
    protected $serviceName ;
    protected $dbpath ;
    protected $cfg ;
    protected $group ;
    protected $node ;
    protected $db ;
    protected $err ; 
    
 
    public function __construct($db,
                                $group, 
                                $hostName, 
                                $srvName,
                                $dbpath = '',
                                $cfg = NULL,
                                $node = NULL)
    {
       $this->db = $db ;
       $this->group = $group ;
       $this->hostName =  $hostName ;
       $this->serviceName =  $srvName ; 
       $this->dbpath =  $dbpath ;
       var_dump( $this->group );
       if ( empty( $node ) && !empty( $this->group ))
       {
          $this->node = $this->group->getNode( $this->hostName.':'.$this->serviceName );
          $this->err = $this->db->getError() ;
       }
       else
       {
          $this->node = $node;
       }
       //var_dump($this->node);
       $this->cfg = $cfg;
    }
    
    public function getError()
    {
       return $this->err;
    }
    
    public function getName()
    {
       if ( empty( $this->node ) ) return $this->hostName.":".$this->serviceName ;
       $name = $this->node->getName() ;
       if ( 0 == strcmp( $name, $this->hostName.":".$this->serviceName ))
       {
          return $name;
       }
       else
       {
          return array($name, $this->hostName.":".$this->serviceName); 
       }
    }
    
    public function getHostName()
    {
       $name = $this->node->getHostName();
       if (0 == strcmp($name, $this->hostName))
       {
          return $name;
       }
       else
       {
          return array($name, $this->hostName); 
       }
    }
    
    public function getServiceName()
    {
       $name = $this->node->getServiceName();
       if (0 == strcmp( $name, $this->serviceName ) )
       {
          return $name;
       }
       else 
       {
          return array( $name, $this->serviceName ); 
       }
    }
    
    public function getStatus()
    {
       $status = $this->node->getStatus() ;
       return $status;
    }
    
    public function __destruct()
    {
    }
    
    public function create()
    {
        $err = $this->group->createNode( $this->hostName, $this->serviceName,
                                        $this->dbpath, $this->cfg );
                                        
        if ($err['errno'] == 0)
        {
           $this->node = $this->group->getNode( $this->hostName.":".$this->serviceName );
           if ( empty( $this->node ) )
           {
              //$err = $this->
              echo "empty node\n";
           }                                        
        }
        return $err['errno'];
    }
    
    public function drop()
    {
        //var_dump($this->hostName);
        //var_dump($this->serviceName);
        //var_dump($this->cfg);
        if ( empty( $this->cfg ) ){
           $err = $this->group->removeNode( $this->hostName, $this->serviceName, $this->cfg );
        }
        else{
           $err = $this->group->removeNode( $this->hostName, $this->serviceName, $this->cfg );
        }
        return $err['errno'];
    }
    
    public function start()
    {
       $err = $this->node->start();
       return $err['errno'];
    }
    
    public function stop()
    {
       $err = $this->node->stop();
       return $err['errno'];
    }
    
    public function connect()
    {
       $nodedb = $this->node->connect();
       return $nodedb;
    }
}
?>
