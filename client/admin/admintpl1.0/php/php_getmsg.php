<?php
if (!session_id()) session_start();
$order = empty( $_POST['order'] ) ? "" : $_POST['order'] ;
if ( $order == "resolution" )
{
	$_SESSION['width'] = empty( $_POST['width'] ) ? 1300 : $_POST['width'] ;
	$_SESSION['height'] = empty( $_POST['height'] ) ? 800 : $_POST['height'] ;
	echo $_SESSION['width'].",".$_SESSION['height'] ;
}
else
{
	echo "unknow";
}
?>