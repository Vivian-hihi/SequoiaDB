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
         return '11810' ;
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

};
?>
