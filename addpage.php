<?php
include_once "./config.inc.php";
//mysql_query("SET NAMES utf8",$link);
$url_name=$_POST['url_name'];
$url=$_POST['url'];
//$user=$_COOKIE['user'];
//$result=mysql_query("SELECT * FROM $user",$link)or die(mysql_error());
//

session_start();

    $username = $_SESSION['login_username'];
	$userpassword=$_SESSION['login_password'];
    $User_No_result=mysql_query("SELECT `No` FROM `user_info` WHERE `Account`= '$username' AND  `Password`= '$userpassword'");
	
	if (!$User_No_result) {
    echo 'Could not run query: ' . mysql_error();
    exit;
}
	//if (mysql_num_rows($User_No_result)>=1){
    $row = mysql_fetch_array($User_No_result);
    //echo "user Id: " . $row['No'];
	
	$No=$row['No'];
	echo $No;
//}
//else{
    //echo "not found!";
//}
  //$row = mysql_fetch_row($User_No_result);

   //echo $row[0];
   mysql_free_result($User_No_result);


//



//$num=mysql_num_rows($result);
//if($num){
    
	$add=mysql_query("INSERT INTO `bookmark`(`No`, `Bookmark`, `URL`) VALUES ('$No','$url_name','$url')");
//	}
//else{
//	$add=mysql_query("INSERT INTO $user(url,url_name) VALUES('$url','$url_name')",$link);
//	}

if($add){
    echo ('<script language="JavaScript"> alert("新增成功");</script>');
	header( "location:show.php");
	}
else{
	echo ('<script language="JavaScript"> alert("新增失敗");</script>');
	header("Refresh:0.5; url=\"show.php");
}












?>