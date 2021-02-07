<?php
//header('Content-Type: text/html; charset=utf-8');
//mysql_set_charset('utf8');
//ini_set('default_charset','UTF-8');

$hostname_conexao1 = "localhost";
$database_conexao1 = "smarthouse";
$username_conexao1 = "root";
$password_conexao1 = "";

// $hostname_conexao1 = "54.39.75.7";
// $database_conexao1 = "bdqyngbnbsudmj189t37";
// $username_conexao1 = "usrxplt5d3lv7sfi";
// $password_conexao1 = "BhkiXAfdcVRh3q55Paou";

$con1 = mysqli_connect($hostname_conexao1, $username_conexao1, $password_conexao1);
if (!$con1)
{
// Handle error - notify administrator, log to a file, show an error screen, etc.
	die("Connection error: " . mysqli_connect_error());
}

mysqli_query($con1, "SET NAMES 'utf8'");
mysqli_query($con1, 'SET character_set_connection=utf8');
mysqli_query($con1, 'SET character_set_client=utf8');
mysqli_query($con1, 'SET character_set_results=utf8');
?>
