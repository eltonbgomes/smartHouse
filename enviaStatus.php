<?php
    include_once("status.php");
    if (isset($_GET['c'])){
        $c = $_GET['c'];
    }else{
        $c = -1;
    }
    if (isset($_GET['l'])){
        $l = $_GET['l'];
    }else{
        $l = -1;
    }
    if (isset($_GET['reset'])){
        $reset = $_GET['reset'];
    }else{
        $reset = 0;
    }

    if ($reset){
        for ($i=0; $i < $_SESSION["nCIs"]; $i++) { 
            $status = 0;
            $j=$i+1;
            $sql = "update output set status='$status' where id_output='$j'";

            mysqli_select_db($con1, $database_conexao1);
            $Recordset1 = mysqli_query($con1,$sql) or die(mysqli_error($con1));
        }
    }else{
        $l = 2 ** ($l-1);
        $status = $_SESSION["status"][$c-1] ^ $l;

        $sql = "update output set status='$status' where id_output='$c'";

        mysqli_select_db($con1, $database_conexao1);
        $Recordset1 = mysqli_query($con1,$sql) or die(mysqli_error($con1));
    }

    header("Location: index.php");
?>
