<?php

function addNewEdition( $id, $version )
{
   $sql = "insert into tp_edition(`id`,`cat_id`,`title`,`describe`,`add_time`,`is_open`) values('$id','','$version','$version','".time()."','1')" ;
   
   $db = mysqli_connect( "192.168.20.248:3306", "root", "sequoiadb", "cn_comm" ) ;
   if( mysqli_connect_errno() )
   {
      echo "Connect failed: ".mysqli_connect_error()."\n" ;
      return false ;
   }
   
   mysqli_query( $db, "set names utf8" ) ;
   
   if( $result = mysqli_query( $db, "select * from tp_edition where `id`='$id'" ) )
   {
      $editions = mysqli_fetch_all( $result, MYSQLI_NUM ) ;
      if( count( $editions ) > 0 )
      {
         mysqli_free_result( $result ) ;
         mysqli_close( $db ) ;
         return true ;
      }
      mysqli_free_result( $result ) ;
   }
   else
   {
      echo mysqli_error( $db )."\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   
   if( mysqli_query( $db, $sql ) == FALSE )
   {
      echo "Falied to insert new edition\n" ;
      echo mysqli_error( $db )."\n" ;
      mysqli_close( $db ) ;
      return false ;
   }

   mysqli_close( $db ) ;
   return true ;
}

function addNewDir( $id )
{
   $db = mysqli_connect( "192.168.20.248:3306", "root", "sequoiadb", "cn_comm" ) ;
   if( mysqli_connect_errno() )
   {
      echo "Connect failed: ".mysqli_connect_error()."\n" ;
      return false ;
   }

   mysqli_query( $db, "set names utf8" ) ;

   $sql = "CREATE TABLE IF NOT EXISTS `tp_dir$id` (`cat_id` int(11) NOT NULL AUTO_INCREMENT,`cat_img` varchar(200) DEFAULT NULL,`cat_name` varchar(255) NOT NULL DEFAULT '',`cat_en_name` varchar(60) DEFAULT NULL,`cat_type` tinyint(1) unsigned NOT NULL DEFAULT '1',`keywords` varchar(255) NOT NULL DEFAULT '',`cat_desc` varchar(255) NOT NULL DEFAULT '',`sort_order` tinyint(3) unsigned NOT NULL DEFAULT '50',`parent_id` int(11) unsigned NOT NULL DEFAULT '0',`content` text,`is_open` int(11) NOT NULL DEFAULT '1',PRIMARY KEY (`cat_id`),KEY `cat_type` (`cat_type`),KEY `sort_order` (`sort_order`),KEY `parent_id` (`parent_id`)) DEFAULT CHARSET=utf8" ;
   if( mysqli_query( $db, $sql ) == FALSE )
   {
      echo "Falied to create dir table\n" ;
      echo mysqli_error( $db )."\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   if( mysqli_query( $db, "truncate table `tp_dir$id`" ) == FALSE )
   {
      echo "Falied to truncate table tp_dir$id\n" ;
      echo mysqli_error( $db )."\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   
   mysqli_close( $db ) ;
   return true ;
}

function _insertDirRecord( $db, $id, $dir, $order, $parentId )
{
   $sql = "INSERT INTO `tp_dir$id` (`cat_id`, `cat_img`, `cat_name`, `cat_en_name`, `cat_type`, `keywords`, `cat_desc`, `sort_order`, `parent_id`, `content`, `is_open`) VALUES(".$dir['id'].", NULL, '".$dir['cn']."', NULL, 1, '', '', $order, $parentId, NULL, 1)" ;
   if( mysqli_query( $db, $sql ) == FALSE )
   {
      echo "Falied to insert tp_dir$id record\n" ;
      echo mysqli_error( $db )."\n" ;
      return false ;
   }
}

function _insertDirAll( $db, $id, $config, $order, $parentId )
{
   if( _insertDirRecord( $db, $id, $config, $order, $parentId ) )
   {
      echo "Failed to insert dir record, id = $config.id\n" ;
      return false ;
   }
   if( array_key_exists( 'contents', $config ) )
   {
      foreach( $config['contents'] as $index => $value )
      {
         if( _insertDirAll( $db, $id, $value, $index + 1, $config['id'] ) == FALSE )
         {
            echo "Failed to insert dir record, id = $value.id\n" ;
            return false ;
         }
      }
   }
   return true ;
}

function insertDir( $id, $config )
{
   $config = array(
      "id" => 1, 
      "cn" => "文档", 
      "en" => "document", 
      "contents" => array(
         array(
            "id" => 3, 
            "cn" => "文档", 
            "en" => "document", 
            "contents" => $config['contents']
         ),
         array(
            "id" => 4, 
            "cn" => "帮助FAQ", 
            "en" => "FAQ", 
            "contents" => array(
               array(
                  "id" => 1432191003,
                  "cn" => "SequoiaDB基础",
                  "en" => ""
               ),
               array(
                  "id" => 1432191004,
                  "cn" => "SequoiaDB操作",
                  "en" => ""
               ),
               array(
                  "id" => 1432191005,
                  "cn" => "SequoiaDB分片",
                  "en" => ""
               ),
               array(
                  "id" => 1432191006,
                  "cn" => "SequoiaDB集合分区",
                  "en" => ""
               ),
               array(
                  "id" => 1432191007,
                  "cn" => "SequoiaDB运维",
                  "en" => ""
               ),
               array(
                  "id" => 1432191008,
                  "cn" => "SequoiaDB问题诊断",
                  "en" => ""
               )
            )
         )
      )
   ) ;
   $db = mysqli_connect( "192.168.20.248:3306", "root", "sequoiadb", "cn_comm" ) ;
   if( mysqli_connect_errno() )
   {
      echo "Connect failed: ".mysqli_connect_error()."\n" ;
      return false ;
   }
   
   mysqli_query( $db, "set names utf8" ) ;
   
   if( _insertDirAll( $db, $id, $config, 1, 0 ) == FALSE )
   {
      echo "Failed to insert dir all\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   
   mysqli_close( $db ) ;
   return true ;
}

function addNewDoc( $id )
{
   $db = mysqli_connect( "192.168.20.248:3306", "root", "sequoiadb", "cn_comm" ) ;
   if( mysqli_connect_errno() )
   {
      echo "Connect failed: ".mysqli_connect_error()."\n" ;
      return false ;
   }

   mysqli_query( $db, "set names utf8" ) ;

   $sql = "CREATE TABLE IF NOT EXISTS `tp_doc$id` (`article_id` int(11) unsigned NOT NULL AUTO_INCREMENT,`cat_id` int(11) NOT NULL DEFAULT '0',`title` varchar(150) DEFAULT NULL,`filetitle` varchar(100) DEFAULT NULL,`fileshort` varchar(200) NOT NULL,`content` longtext NOT NULL,`keywords` varchar(255) NOT NULL DEFAULT '',`is_open` tinyint(1) unsigned NOT NULL DEFAULT '1',`is_recommend` tinyint(1) NOT NULL DEFAULT '0',`add_time` int(10) unsigned NOT NULL DEFAULT '0',`file_url` varchar(255) NOT NULL DEFAULT '',`link` varchar(255) NOT NULL DEFAULT '',`description` varchar(255) DEFAULT NULL,`sort_order` int(8) NOT NULL DEFAULT '50',`short` text,`original_img` varchar(50) DEFAULT NULL,`thumb_img` varchar(50) DEFAULT NULL,`label` varchar(100) DEFAULT NULL,`is_link` tinyint(1) NOT NULL DEFAULT '1',`downocunt` int(11) NOT NULL DEFAULT '0',`article_url` text,`edition` int(11) NOT NULL DEFAULT '0',`subEdition` int(11) NOT NULL DEFAULT '1',PRIMARY KEY (`article_id`),KEY `cat_id` (`cat_id`)) DEFAULT CHARSET=utf8" ;
   if( mysqli_query( $db, $sql ) == FALSE )
   {
      echo "Falied to create doc table\n" ;
      echo mysqli_error( $db )."\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   if( mysqli_query( $db, "truncate table `tp_doc$id`" ) == FALSE )
   {
      echo "Falied to truncate table tp_doc$id\n" ;
      echo mysqli_error( $db )."\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   
   mysqli_close( $db ) ;
   return true ;
}

function _insertDocAll( $db, $id, $config, $path )
{
   if( array_key_exists( 'contents', $config ) )
   {
      $newPath = $path ;
      if( array_key_exists( 'dir', $config ) )
      {
         if( getOSInfo() == 'windows' )
         {
            $newPath = $path."\\".$config['dir'] ;
         }
         else
         {
            $newPath = $path."/".$config['dir'] ;
         }
      }
      foreach( $config['contents'] as $index => $value )
      {
         if( _insertDocAll( $db, $id, $value, $newPath ) == FALSE )
         {
            echo "Failed to insert dir record, id = ".$value['id']."\n" ;
            return false ;
         }
      }
   }
   else
   {
      if( getOSInfo() == 'windows' )
      {
         $path = $path."\\".$config['file'].".html" ;
      }
      else
      {
         $path = $path."/".$config['file'].".html" ;
      }
      if( file_exists( $path ) == FALSE )
      {
         echo "No such file $path\n" ;
         return false ;
      }
      $catid = $config['id'] ;
      $title = $config['cn'] ;
      $contents = file_get_contents( $path ) ;
      $title = mysqli_real_escape_string( $db, $title ) ;
      $contents = mysqli_real_escape_string( $db, $contents ) ;
      $sql = "INSERT INTO `tp_doc$id` (`cat_id`, `title`, `fileshort`, `content`, `keywords`, `add_time`, `file_url`, `link`, `edition`) VALUES($catid, '$title', '', '$contents', '', '".time()."', '', '', $id)" ;
      if( mysqli_query( $db, $sql ) == FALSE )
      {
         echo "Falied to insert tp_doc$id record\n" ;
         echo mysqli_error( $db )."\n" ;
         return false ;
      }
   }
   return true ;
}

function insertDoc( $id, $config, $root )
{
   $db = mysqli_connect( "192.168.20.248:3306", "root", "sequoiadb", "cn_comm" ) ;
   if( mysqli_connect_errno() )
   {
      echo "Connect failed: ".mysqli_connect_error()."\n" ;
      return false ;
   }

   mysqli_query( $db, "set names utf8" ) ;
   
   $rootPath = "" ;
   if( getOSInfo() == 'windows' )
   {
      $rootPath = "$root\\build\\mid" ;
   }
   else
   {
      $rootPath = "$root/build/mid" ;
   }
   if( _insertDocAll( $db, $id, $config, $rootPath ) == FALSE )
   {
      echo "Failed to insert dir all\n" ;
      mysqli_close( $db ) ;
      return false ;
   }
   
   mysqli_close( $db ) ;
   return true ;
}