<?php

function getConfig( $path )
{
   if( file_exists( $path ) === false )
   {
      echo "No such file, $path\n" ;
      return false ;
   }
   $json = file_get_contents( $path ) ;
   if( $json === false )
   {
      echo "Failed to get file contents, $path\n" ;
      return false ;
   }
   $arr = json_decode( $json, true ) ;
   if( $arr == NULL )
   {
      echo "Failed to decode json, $path\n" ;
      return false ;
   }
   return $arr ;
}

function getVersion( $path )
{
   return getConfig( $path ) ;
}

function getCnPath( $id, $config, $path )
{
   if( array_key_exists( 'contents', $config ) )
   {
      if( array_key_exists( 'id', $config ) && $config['id'] == $id )
      {
         return $path ;
      }
      else
      {
         foreach( $config['contents'] as $index => $value )
         {
            $newPath = getCnPath( $id, $value, "$path/".$value['cn'] ) ;
            if( $newPath !== false )
            {
               return $newPath ;
            }
         }
      }
   }
   return false ;
}

function getDirPath( $id, $config, $path )
{
   if( array_key_exists( 'contents', $config ) )
   {
      if( array_key_exists( 'id', $config ) && $config['id'] == $id )
      {
         return $path ;
      }
      else
      {
         foreach( $config['contents'] as $index => $value )
         {
            $filename = array_key_exists( 'dir', $value ) ? $value['dir'] : $value['file'] ;
            $newPath = getDirPath( $id, $value, "$path/$filename" ) ;
            if( $newPath !== false )
            {
               return $newPath ;
            }
         }
      }
   }
   return false ;
}