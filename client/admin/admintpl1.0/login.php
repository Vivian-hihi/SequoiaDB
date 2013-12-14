<?php
if (!session_id()) session_start();
$address = empty($_POST['ipaddress'])?"": $_POST['ipaddress'] ;
$user = empty($_POST['user'])?"": $_POST['user'] ;
$password = empty($_POST['pwd'])?"": $_POST['pwd'] ;
$remember = empty($_POST['remember'])?"": $_POST['remember'] ;
$error = "" ;
if ( $address != "" )
{
	$db = new SequoiaDB() ;
	$db->connect ( $address, $user, $password ) ;
	$arr = $db->getError () ;
	if ( $arr['errno'] == 0 )
	{
		//$_SESSION['sdb'] = $db ;
		$_SESSION['address'] = $address ;
		$_SESSION['user'] = $user ;
		$_SESSION['password'] = $password ;
		if ( $remember == 'on' )
		{
			setcookie( "sequoiadb[address]", $address );
			setcookie( "sequoiadb[user]", $user );
		}
		else
		{
			setcookie( "sequoiadb" );
		}
		header ("Location:index.php") ;
	}
	else
	{
		$error = "登录错误 error: ".$arr['errno'] ;
	}
}
else
{
	$address = "localhost:50000" ;
	if ( !empty ( $_COOKIE["sequoiadb"] ) )
	{
		$temp = $_COOKIE["sequoiadb"] ;
		if ( array_key_exists ( 'address', $temp ) )
		{
			$address = $temp['address'] ;
		}
		if ( array_key_exists ( 'user', $temp ) )
		{
			$user = $temp['user'] ;
		}
	}
}
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>后台管理</title>
<!--                       CSS                       -->
<!-- Main Stylesheet -->
<link rel="stylesheet" href="css/login_style.css" type="text/css" media="screen" />
<script type="text/javascript" language="javascript" src="js/common.js" ></script>
<script type="text/javascript" language="javascript" src="js/flot/jquery.js"></script>

</head>
<body id="login">
<div id="login-wrapper" class="png_bg">
  <div id="login-top">
    <h1>后台管理</h1>
    <!-- Logo (221px width) -->
    <a href="#"><img id="logo" src="images/orange_logo2.png" border="0" alt="logo" width="400px;" /></a> </div>
  <!-- End #logn-top -->
  <div id="login-content">
  <span style="color:#F00; font-size:14px;"><?php echo $error;?></span>
    <form action="login.php" method="post">
      <p>
        <label>连接地址</label>
        <input class="text-input" type="text" name="ipaddress" value="<?php echo $address;?>" />
      </p>
      <div class="clear"></div>
      <p>
        <label>用户名</label>
        <input class="text-input" type="text" name="user" value="<?php echo $user;?>" />
      </p>
      <div class="clear"></div>
      <p>
        <label>密码</label>
        <input class="text-input" type="password" name="pwd" />
      </p>
      <div class="clear"></div>
      <p id="remember-password">
        <input name="remember" type="checkbox" <?php if ( !empty ( $_COOKIE["sequoiadb"] ) ) echo 'checked="checked"';?> />
        记住我 </p>
      <div class="clear"></div>
      <p>
        <input class="button" type="submit" value="登  录" />
      </p>
    </form>
  </div>
  <!-- End #login-content -->
</div>
<!-- End #login-wrapper -->
</body>
</html>
