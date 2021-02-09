<?php
    session_start();
    include_once("conecta.php");
    $_SESSION["byte"] = 8;

    $sql="Select id_output, status from output";

    mysqli_select_db($con1,$database_conexao1);
	$Recordset1 = mysqli_query($con1,$sql) or die(mysqli_error($con1));
	$row1 = mysqli_fetch_assoc($Recordset1);
	$totalRows_Recordset1 = mysqli_num_rows($Recordset1);

    $i = 0;
    do{
        $_SESSION["id_output"][$i] = $row1["id_output"];
        $_SESSION["status"][$i] = $row1["status"];
        $i++;
    } while($row1 = mysqli_fetch_assoc($Recordset1));

    $_SESSION["nCIs"] = $totalRows_Recordset1;
?>