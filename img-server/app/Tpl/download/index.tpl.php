<?php
if(isset($_SERVER['HTTP_IF_MODIFIED_SINCE'])){
    header('Last-Modified: '.$_SERVER['HTTP_IF_MODIFIED_SINCE'],true,304);
    exit;
}

header("Cache-Control: private, max-age=10800, pre-check=10800");
header("Pragma: private");
header("Expires: " . date(DATE_RFC822, strtotime("2 day") - 28800));
header("Content-type: image/" . $img_type);
echo $img_data;
