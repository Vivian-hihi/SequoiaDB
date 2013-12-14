<?php

$path = "./user/userlist.ult" ;
$csv = "" ;
$conn_arr_list  = array() ;
$link_result  = "" ;
$link_errno = 0 ;

$model     = empty ( $_POST['connectmodel']   ) ? "" : $_POST['connectmodel'] ;
$address   = empty ( $_POST['connectaddress'] ) ? "" : $_POST['connectaddress'] ;
$user      = empty ( $_POST['connectuser']    ) ? "" : $_POST['connectuser'] ;
$pwd       = empty ( $_POST['connectpwd']     ) ? "" : $_POST['connectpwd'] ;
$represent = empty ( $_POST['represent']      ) ? "" : $_POST['represent'] ;

if ( !file_exists ( $path ) )
{
	$file = fopen( $path, "w" ) ;
	fclose( $file ) ;
}


if ( $model == "list" )
{
	if ( file_exists ( $path ) )
	{
		$file = fopen( $path, "r+" ) ;
		while( !feof( $file ) )
		{
			$csv = fgetcsv( $file ) ;
			if ( $csv != false && $csv[0] != NULL )
			{
				array_push ( $conn_arr_list, $csv ) ;
			}
		}
		fclose( $file ) ;
	}
}
else if ( $model == "delete" )
{
	$temp1 = 0 ;
	$temp2 = 0 ;
	$inputdata = array('','','') ;
	if ( file_exists ( $path ) )
	{
		$file = fopen( $path, "r+" ) ;
		while( !feof( $file ) )
		{
			$csv = fgetcsv( $file ) ;
			if ( $csv[0] == $address )
			{
				$temp2 = ftell($file) ;
				fseek ( $file, $temp1 ) ;
				fputs( $file,  "," ) ;
				fseek ( $file, $temp2 ) ;
				break ;
			}
			$temp1 = ftell($file) ;
		}
		fclose( $file ) ;
	}
}
else if ( $model == "update" )
{
	$temp1 = 0 ;
	$temp2 = 0 ;
	$inputdata = array('','','') ;
	if ( file_exists ( $path ) )
	{
		$file = fopen( $path, "r+" ) ;
		while( !feof( $file ) )
		{
			$csv = fgetcsv( $file ) ;
			if ( $csv[0] == $address )
			{
				$temp2 = ftell($file) ;
				fseek( $file, $temp1 ) ;
				fputs( $file,  "," ) ;
				fseek( $file, $temp2 ) ;
				break ;
			}
			$temp1 = ftell($file) ;
		}
		fseek( $file, 0, SEEK_END  ) ;
		fputcsv( $file,  array ( $address, $user, $pwd, $represent ) ) ;
		fclose( $file ) ;
	}
}
else if ( $model == "testlink" )
{
	$db_t = new SequoiaDB() ;
	$arr = $db_t -> connect ( $address, $user, $pwd ) ;
	$link_errno = $arr['errno'] ;
	if ( $arr['errno'] == 0 )
	{
		$link_result = "连接成功" ;
	}
	else
	{
		$rc = $arr['errno'] ;
		$link_result = "连接失败：".(array_key_exists( $rc, $errno_cn ) ? $errno_cn[$rc] : $rc) ;
	}
}
else if ( $model == "connect" )
{
	$_SESSION['sdb_monitor_address'] = $address ;
	$_SESSION['sdb_monitor_user'] = $user ;
	$_SESSION['sdb_monitor_password'] = $pwd ;
	$db_t = new SequoiaDB() ;
	$arr = $db_t -> connect ( $address, $user, $pwd ) ;
	$link_errno = $arr['errno'] ;
	if ( $arr['errno'] == 0 )
	{
		$link_result = "连接成功" ;
	}
	else
	{
		$link_result = "连接失败，错误码：".$arr['errno'] ;
	}
}

$array_temp = array() ;
$error_record = 0 ;
if ( file_exists ( $path ) )
{
	$file = fopen( $path, "r+" ) ;
	while( !feof( $file ) )
	{
		$str = fgets( $file, 163840 ) ;
		if ( $str[0] != "," )
		{
			array_push ( $array_temp, $str ) ;
		}
		else
		{
			++$error_record ;
		}
	}
	fclose( $file ) ;
	if ( $error_record > 0 )
	{
		$file = fopen( $path, "w" ) ;
		foreach ( $array_temp as $child )
		{
			fputs( $file,  $child ) ;
		}
		fclose( $file ) ;
	}
}

$smarty -> assign( "connectlist_model", $model ) ;
$smarty -> assign( "connectlist_return", $conn_arr_list ) ;
$smarty -> assign( "link_result", $link_result ) ;
$smarty -> assign( "link_errno", $link_errno ) ;

?>