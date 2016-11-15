/****************************************************
@description:      ReplicaGroup operate, warp class
@testlink cases:   seqDB-7636-7644
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
include 'ReplicaNode.php';
class ReplicaGroup
{
    protected $db;
    protected $name;
    protected $group;
    protected $nodes;
    protected $PrimaryNode;
   
    
    public function __construct($sdb, $groupName="")
    {
       $this->db = $sdb;
       $this->name = $groupName;
       $this->nodes= array();
       if (strlen($groupName)){
          var_dump($groupName);
          $this->group = $this->db->getGroup($groupName);
       }
    }
    
    public function __destruct()
    {
    }
    
    public function isCataLog()
    {
       $ret = $this->group->isCatalog();
       return $ret;
    }
    
    public function getName()
    {
        $name = $this->group->getName();
        if ($name == $this->name)
        {
           return $name;
        }
        else
        {
           return "";
        }
    }
    
    public function reelect( $options = NULL )
    {
        $err = $this->group->reelect(options);
        return $err['errno'];
    }
    
    public function getNodeNum()
    {
       return count($this->nodes);
       // return $this->group->getNodeNum(0);
    }
    
    public function getNodes()
    {
        $detail = $this->group->getDetail();
        $groupName = $detail['GroupName'];
        if ($groupName != $this->name)
        {
           return NULL;
        }
        
        $nodesOfGroup = $detail['Group'];
        $this->PrimaryNode = $detail['PrimaryNode'];
        for ($i=0; $i<count($nodesOfGroup); ++$i)
        {
           $tmp = $nodesOfGroup[$i];
           $repliNode = new ReplicaNode($this->group, 
                                        $tmp['HostName'], 
                                        $tmp['Service'][0]['Name'],
                                        $tmp['dbpath']);
           array_push($this->nodes, $repliNode);
        }
        
        return $this->nodes;
    }
    
    public function getMaster()
    {
        $node = $this->group->getMaster();
        for ($i=0; $i<count($this->nodes); ++$i)
        {
            $tmpnode = $this->nodes[$i];
            if ($node->getHostName() == $tmpnode->getHostName() && 
                $node->getServiceName() == $tmpnode->getServiceName())
            {
               break;
            }
        }
        
        if ($i != count($this->nodes))
        {
           return $node;
        }
        else 
        {
           return NULL;
        }
    }
    
    public function getSlave()
    {
        $node = $this->group->getSlave();
        for ($i=0; $i<count($this->nodes); ++$i)
        {
            $tmpnode = $this->nodes[$i];
            if ($node->getHostName() == $tmpnode->getHostName() && 
                $node->getServiceName() == $tmpnode->getServiceName())
            {
               break;
            }
        }
        
        if ($i != count($this->nodes))
        {
           return $node;
        }
        else 
        {
           return NULL;
        }
    }
    
    public function create($hostName='' , $serviceName='', $dbpath='', $cfg = NULL )
    {
        if (!empty($this->group)) return;
        if (0 === strcmp("SYSCatalogGroup", $this->name))
        {
           $err = $this->db->createCataGroup($hostName, $serviceName, $dbpath, $cfg);
        }
        else 
        {
           $err = $this->db->createGroup($this->name);
        }
        
        echo "create.................\n";
        var_dump($err); 
        if (0 == $err['errno'])
        {
           $this->group = $this->db->getGroup($this->name) ;
           if (empty($this->group))
           {
              $err = $this->db->getError();
              return $err['errno'];
           }
        }
        return $err['errno'];
    }
    
    public function drop()
    {
       $err = $this->db->removeGroup($this->name);
       return $err['errno'];
    }
    
    public function start()
    {
       $err = $this->group->start();
       return $err['errno'];
    }
    
    public function stop()
    {
       $err = $this->group->stop();
       return $err['errno'];
    }
    
    public function getNode($name)
    {
        for ($i=0; $i<count($this->nodes); $i++)
        {
           $repliNode = $this->nodes[$i];
           if ($repliNode->getName() == $name){
              return $repliNode;
           }
        }

        $token = strtok($name, ":");
        $hostName = $token;
        $port = '11810';
        if ($token !== false)
        {
            $token = strtok($name, ":");
            if ($token !== false)
            {
                $port = $token;
            }
        }

        $node = $this->group->getNode($name);
        if(empty($node))
        {
           return NULL;
        }
        $repliNode = new ReplicaNode($this->group, $node->getHostName(), 
                                     $node->getServiceName(),NULL, $node);
        
        return $repliNode;
    }
    
    public function addNode( $hostName, $serviceName, $dbpath, $cfg = NULL )
    {
        var_dump($this->group);
        $repliNode = new ReplicaNode($this->group, $hostName, $serviceName,
                                     $dbpath, $cfg );
        $err = $repliNode->create();
        if ($err == 0)
        {
           array_push($this->nodes, $repliNode);
        }
        return $err;
    }
    
    public function removeNode($hostName, $serviceName)
    {
        for ($i=0; $i <count($this->nodes); ++$i)
        {
           $node = $this->nodes[$i];
           if ($node->getHostName() == $hostName &&
               $node->getServiceName() == $serviceName)
           {
              break;
           }
        }
        $err = $node->drop();
        if ($err == 0)
        {
           for(; $i<count($this->nodes) - 1;++$i)
           {
              $this->nodes[$i] = $this->nodes[$i+1];
           }
        }
        return $err;
    }
    
    public function attachNode($node, $options=NULL)
    {
        $err = $this->group->attachNode($node->getHostName(), $node->getServiceName(), $options);
        return $err['errno'];
    }
    
    public function detachNode($node, $options=NULL)
    {
        $err = $this->group->detachNode($node->getHostName(), $node->getServiceName(), $options);
        return $err['errno'];
    }
}
?>
