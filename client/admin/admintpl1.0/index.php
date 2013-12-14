<?php
if (!session_id()) session_start();
if ( empty($_SESSION['address']) || empty($_SESSION['width']) || empty($_SESSION['height']) )
{
	header ("Location:login.php") ;
	exit() ;
}
$db = new SequoiaDB() ;
$db->connect ( $_SESSION['address'], $_SESSION['user'], $_SESSION['password'] ) ;
$arr = $db->getError () ;
if ( $arr['errno'] )
{
	header ("Location:login.php") ;
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>后台管理</title>
</head>
<frameset name="index" cols="200px,80%">
  <frame src="php/left_1.php" name="left" scroll="no" noresize="noresize"/>
  <frame src="php/right_1.php" name="right" />
</frameset><noframes></noframes>

</html>