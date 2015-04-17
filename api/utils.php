<?php
include_once('common.php');

// avoid sql
function avoid_sql($value)
{
    // 去除斜杠
    if (get_magic_quotes_gpc()) {
        $value = stripslashes($value);
    }   

    if (!is_numeric($value)) {
        $value = mysql_real_escape_string($value);
    }    

    return $value;  
}


function show_info($status, $desc)
{
	$res = array();
	$res['status'] = $status;
	$res['des'] = $desc;
	
	return $res;
}


function mk_dir($dir, $mode = 0755)
{
	if (is_dir($dir) || @mkdir($dir, $mode))
		return TRUE;

	if (!mk_dir(dirname($dir), $mode))
		return FALSE;

	return @mkdir($dir, $mode); 
}


function get_avatar_url($uid)
{
	return AVATAR_HOST ."/". $uid ."/avatar/s_avatar.jpg";
}

?>
