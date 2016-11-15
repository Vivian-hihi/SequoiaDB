/****************************************************
@description:      ReplicaGroupMgr operate, warp class
@testlink cases:   seqDB-7636-7644
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
include 'ReplicaGroup.php';
class ReplicaGroupMgr
{
    private $db;
    private $groups;
    public function __construct($sdb)
    {
       $this->db = $sdb;
       $this->groups= array();
    }
    
    public function __destruct()
    {
    }
    
    public function getGroups()
    {
        $cursor = $this->db->listGroup();
        if (empty($cursor))
        {
            $err = $this->db->getError();
            return $err['errno'];
        }
        
        while($record = $cursor->next())
        {
           $group = new ReplicaGroup($this->db, $record['GroupName']);
           $group->getNodes();
           array_push($this->groups, $group);
        }
        
        return $this->groups;
    }
    
    public function getDataGroups()
    {
        $groups = array();
        $cursor = $this->db->listGroup();
        if (empty($cursor))
        {
            $err = $this->db->getError();
            return $err['errno'];
        }
        
        while($record = $cursor->next())
        {
           if ($record['GroupName'] != "SYSCoord" &&
               $record['GroupName'] != "SYSCatalogGroup" &&
               $record['GroupName'] != "SYSSpare")
           {
              $group = new ReplicaGroup($this->db, $record['GroupName']);
              $group->getNodes();
              array_push($groups, $group);
           }
        }
        
        return $groups;
    }
}
?>