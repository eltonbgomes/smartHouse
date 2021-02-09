<?php include_once("status.php"); ?>
<!DOCTYPE html>
<html lang="pt">

<head>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta1/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-giJF6kkoqNQ00vy+HMDP7azOuL0xtbfIcaT9wjKHr8RbDVddVHyTfAAsrekwKmP1" crossorigin="anonymous">
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta1/dist/js/bootstrap.bundle.min.js" integrity="sha384-ygbV9kiqUc6oa4msXn9868pTtWMgiQaeYH7/t7LECLbyPA2x65Kgf80OJFdroafW" crossorigin="anonymous"></script>
</head>

<body>
    <?php
        for ($i=$_SESSION["nCIs"]; $i > 0; $i--) { 
            $auxDecimal = $_SESSION["status"][$i-1];
            for ($j=0; $j < $_SESSION["byte"] ; $j++) {//loop para colorir os botoes conforme o status
                $binario[$j] = $auxDecimal % 2;
                $auxDecimal = intdiv($auxDecimal, 2);
            }
            echo "<center>";
            for ($j=$_SESSION["byte"]; $j > 0 ; $j--) {
                if ($binario[$j-1]) {
                    $corBotao = "btn btn-warning";
                }else{
                    $corBotao = "btn btn-primary";
                }
                echo '<a type="button" href="enviaStatus.php?c='.$i.'&l='.$j.'" class="'.$corBotao.'">C'.$i.' L'.$j.'</a>';
            }
            echo '</center><hr>';
        }        
    ?>
    
    <center><a type="button" href="enviaStatus.php?reset=1" class="btn btn-danger">RESET</a></center>
</body>
