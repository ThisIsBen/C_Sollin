<?php
	session_start();
	include_once "./config.inc.php";
	$error='';
//////////////

//////////////
	if (isset($_POST['submit'])) {
		if (empty($_POST['user']) || empty($_POST['password'])) {
			$error = "Username or Password is invalid";
		}
		else {
			$inputAccount = $_POST['user'];
			$inputPassword = $_POST['password'];

			

			//$link = mysql_connect($host, $user, $upwd) or die ("Unable to connect!");
			// To protect MySQL injection for Security purpose
			$inputAccount = stripslashes($inputAccount);
			$inputPassword = stripslashes($inputPassword);

			//$inputEmail = mysql_real_escape_string($inputEmail);
			$inputPassword = mysql_real_escape_string($inputPassword);

			//$db = mysql_select_db("hw3", $link) or die ("Unable to select database!");

			$query = mysql_query("SELECT * FROM user_info WHERE  Account='$inputAccount' AND Password = '$inputPassword'");

			$rows = mysql_num_rows($query);
			if ($rows == 1) {
				$_SESSION['login_username'] = $inputAccount; // Initializing Session
				$_SESSION['login_password'] = $_POST['password'];
			   
				header("location: show.php"); // Redirecting To Other Page
			} 
			else {
				$error = "Username or Password is invalid";
				echo "<script type='text/javascript'>";
				echo "alert(\"Username or Password is invalid\")";
				echo "</script>";
				header("location: index.html");
			}
			//mysql_close($link); // Closing Connection
		}
	}
?>

