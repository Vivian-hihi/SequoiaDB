/****************************************************
@description:      CI input parameters
@modify list:
        2016-11-22 wenjing wang init
****************************************************/
<?php
class globalParameter
{
   static public function getHostName()
   {
      if ( !isset( $_POST['HOSTNAME'] ) && empty( $_POST['HOSTNAME'] ) )
      {
         return 'localhost' ;
      }
      else
      {
         return $_POST['HOSTNAME'] ;
      }
   }

   static public function getCoordPort()
   {
      if ( !isset( $_POST['SVCNAME'] ) && empty( $_POST['SVCNAME'] ) )
      {
         return 50000 ;
      }
      else
      {
         return $_POST['SVCNAME'] ;
      }
   }

   static public function getChangedPrefix()
   {
      if ( !isset( $_POST['CHANGEDPREFIX'] ) && empty( $_POST['CHANGEDPREFIX'] ) )
      {
         return "php_test" ;
      }
      else
      {
         return $_POST['CHANGEDPREFIX'] ;
      }
   }
   static public function getSpareportStart()
   {
      if ( !isset( $_POST['SPAREPORTSTART'] ) && empty( $_POST['SPAREPORTSTART'] ) )
      {
         return 26000 ;
      }
      else
      {
         return $_POST['SPAREPORTSTART'] ;
      }
   }
   
   static public function getSpareportStop()
   {
      if ( !isset( $_POST['SPAREPORTSTOP'] ) && empty( $_POST['SPAREPORTSTOP'] ) )
      {
         return 27000 ;
      }
      else
      {
         return $_POST['SPAREPORTSTOP'] ;
      }
   }
   
   static public function getDbPathPrefix()
   {
      if ( !isset( $_POST['DBPATHPREFIX'] ) && empty( $_POST['DBPATHPREFIX'] ) )
      {
         return "/data/disk5/sequoiadb/database/data" ;
      }
      else
      {
         return $_POST['DBPATHPREFIX'] ;
      }
   }

};
?>
