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
    private $db ;
    private $groups ;
    private $err ;
    public function __construct($sdb)
    {
       $this->db = $sdb;
       $this->groups= array();
    }
    
    public function __destruct()
    {
    }
    
    public function getError()
    {
       return $this->err;
    }
    
    public function getGroups()
    {
        $cursor = $this->db->listGroup();
        if ( empty( $cursor ) )
        {
            $this->err = $this->db->getError();
            return $this->groups;
        }
        
        while( $record = $cursor->next() )
        {
           $group = new ReplicaGroup( $this->db, $record['GroupName'] );
           if ( $group->getError()['errno'] != 0 )
           {
              $this->err = $group->getError();
              unset( $this->groups );
              return;
           }
           
           $group->getNodes();
           if ( $group->getError()['errno'] != 0 )
           {
              $this->err = $group->getError();
              unset( $this->groups );
              return;
           }
           array_push( $this->groups, $group );
        }
        
        return $this->groups;
    }
    
    public function getDataGroups()
    {
        $groups = array();
        $cursor = $this->db->listGroup();
        if ( empty( $cursor ) )
        {
            $this->err = $this->db->getError();
            return $groups;
        }
        
        while( $record = $cursor->next() )
        {
           if ( $record['GroupName'] != "SYSCoord" &&
                $record['GroupName'] != "SYSCatalogGroup" &&
                $record['GroupName'] != "SYSSpare" )
           {
              $group = new ReplicaGroup( $this->db, $record['GroupName'] );
              if ( $group->getError()['errno'] != 0 )
              {
                 $this->err = $group->getError();
                 unset( $this->groups );
                 return;
              }
              $group->getNodes();
              if ( $group->getError()['errno'] != 0 )
              {
                 $this->err = $group->getError();
                 unset( $this->groups );
                 return;
              }
              array_push( $groups, $group );
           }
        }
        
        return $groups;
    }
}
?>