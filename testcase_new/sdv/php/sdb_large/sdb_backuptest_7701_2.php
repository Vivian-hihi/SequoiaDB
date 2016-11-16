/****************************************************
@description:      backup operate,warp class
@testlink cases:   
@modify list:
        2016-4-27 wenjing wang init
****************************************************/
<?php
   define('Cur_Path', dirname(__FILE__));
   include_once Cur_Path.'/lib/backuptask.php';
class backuptest extends PHPUnit_Framework_TestCase
{
   private static $db;
   private static $backupTask;
   private static $options;
   public static function setUpBeforeClass()
   {
      self::$db = new SequoiaDB();
      self::$db->connect('localhost:11810');
      self::$backupTask = new BackupTask(self::$db);
   }

   public function testbackup()
   {
      $options = array('Name' => 'backupName');
      $ret = self::$backupTask->backupOffline($options);
      $this->assertEquals(0, $ret);
      $ret = self::$backupTask->listBackup($options);
      $this->assertEquals(true, $ret);

      self::$backupTask->removeBackup($options);
      self::$options = json_encode($options);
      $ret = self::$backupTask->backupOffline(self::$options);
      $this->assertEquals(0, $ret);

      $ret=self::$backupTask->listBackup(self::$options);
      $this->assertEquals(true, $ret);
      
   }

   /*
    * @depends testbackup 
    *
    */
   public function testremove()
   {
      $ret = self::$backupTask->removeBackup(self::$options);
      $this->assertEquals(0, $ret);
     
      $ret = self::$backupTask->listBackup(self::$options);
      $this->assertEquals(false, $ret);
      
      $ret = self::$backupTask->listBackup(null);
      $this->assertEquals(false, $ret);
   }
   
   public static function tearDownAfterClass()
   {
      self::$db->close();
   }
}
?>
