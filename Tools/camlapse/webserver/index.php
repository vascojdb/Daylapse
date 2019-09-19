<?php
ini_set('display_startup_errors', 1);
ini_set('display_errors', 1);
error_reporting(-1);
?>

<!DOCTYPE html>
<html>
<head>
<title>Daylapse</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.10.2/css/all.min.css">
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body {
  font-family: Arial;
  color: white;
}

.split {
  height: 100%;
  width: 50%;
  position: fixed;
  z-index: 1;
  top: 0;
  overflow-x: hidden;
  padding-top: 20px;
}

.left {
  left: 0;
  background-color: black;
  overflow: hidden;
}

.right {
  right: 0;
  background-color: #333333;
}

.centered {
  position: absolute;
  top: 0%;
  left: 0%;
  width: 100%;
  text-align: center;
}

span.blocks {
  display: inline-block;
  padding-left: 10px;
  padding-right: 10px;
  padding-top: 10px;
}

img {
  border-style: solid;
  border-color: white;
}
</style>
</head>
<body>

<div class="split left">
    <div class="centered">
        <?php
        $files = glob('img/*.jpg');
        $index = str_replace(".jpg", "", $files[count($files)-1]);
        $epoch = (int)str_replace("img/", "", $index);

        echo "<h1>" . date('l, d F Y\<\b\r\>H:i', $epoch) . "</h1>";
        echo "<a href=\"img/timelapse.mp4\" target=\"_blank\">";
        echo "<img src=\"" . $index . ".jpg\" alt=\"Picture\" style=\"width: 90%;\">";
        echo "</a>";
        
        $logfile = fopen($index . ".log","r");
        while(!feof($logfile)) {
            $log_line = explode("=", fgets($logfile));
            $param = $log_line[0];
            if (count($log_line) > 1) $value = $log_line[1];
            else $value = 0;
            
            if (strcmp($param, 'CPU_TEMP') == 0) {
                echo "<span class=\"blocks\" style=\"font-size: 20px;\">";
                echo "<i class=\"fas fa-thermometer-half a-fw\"></i> <i class=\"fas fa-microchip a-fw\"></i> <b>CPU temperature</b><br>" . $value . "&deg;C";
                echo "</span>";
            }
            /*
            if (strcmp($param, 'IN_TEMP') == 0) {
                echo "<span class=\"blocks\" style=\"font-size: 20px;\">";
                echo "<i class=\"fas fa-thermometer-half a-fw\"></i> <i class=\"fas fa-box a-fw\"></i> <b>Box temperature</b><br>" . $value . "&deg;C";
                echo "</span>";
            }
            */
            if (strcmp($param, 'OUT_TEMP') == 0) {
                echo "<span class=\"blocks\" style=\"font-size: 20px;\">";
                echo "<i class=\"fas fa-thermometer-half a-fw\"></i> <i class=\"fas fa-tree a-fw\"></i> <b>Outside temperature</b><br>" . $value . "&deg;C";
                echo "</span>";
            }
            //if (strcmp($param, 'UPTIME') == 0) {
            //    echo "<p>System uptime: " . $value . "s</p>";
            //}
        }
        fclose($logfile);
        ?> 
    </div>
</div>
<div class="split right">
    <div class="centered">
        <?php
        $files = glob('img/*.jpg');
        $files = array_reverse($files);
        foreach ($files as $file) {
            $index = str_replace(".jpg", "", $file);
            $epoch = (int)str_replace("img/", "", $index);

            echo "<span class=\"blocks\" style=\"font-size: 12px;\">";
            echo date('Y/m/d - \<\b\>H:i\<\b\>', $epoch) . "<br>";
            
            $tooltip = "";
            $logfile = fopen($index . ".log","r");
            while(!feof($logfile)) {
                $log_line = explode("=", fgets($logfile));
                $param = $log_line[0];
                if (count($log_line) > 1) $value = str_replace("\n", "", $log_line[1]);
                else $value = 0;
                
                if (strcmp($param, 'CPU_TEMP') == 0) $tooltip = $tooltip . "CPU: " . $value . "&deg;C\r";
                /*else if (strcmp($param, 'IN_TEMP') == 0) $tooltip = $tooltip . "Box: " . $value . "&deg;C\r";*/
                else if (strcmp($param, 'OUT_TEMP') == 0) $tooltip = $tooltip . "Outside: " . $value . "&deg;C\r";
            }
            fclose($logfile);
            
            $image = exif_thumbnail($file, $width, $height, $type);
            echo "<a href=\"" . $index . ".jpg\" target=\"_blank\">";
            echo "<img width='100%' src='data:image/gif;base64,".base64_encode($image)."' title=\"" . $tooltip . "\">";
            echo "</a></span>";
        }
        ?>
    </div>
</div>
</body>
</html>
