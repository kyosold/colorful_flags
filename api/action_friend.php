<?php

include_once('common.php');
include_once('utils.php');

$type = trim(avoid_sql($_POST['type']));
$myid = trim(avoid_sql($_POST['myid']));
$fid = trim(avoid_sql($_POST['fid']));

$res = array();

if (strlen($type) > 0 &&
	strlen($myid) > 0 &&
	strlen($fid) > 0) {

	if ($type == "add") {
		return add_friend($type, $myid, $fid);
	} else if ($type == "del") {
	}
}

$res['status'] = 'fail';
$res['des'] = 'parameters error';

echo json_encode($res);


// --------------------------------------------------------

function add_friend($type, $myid, $fid) {
	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	$sql = "SELECT id, status FROM liao_relationship WHERE myid = '{$myid}' AND fid = '{$fid}' LIMIT 1";
    $result = mysql_query($sql);
    if ($row = mysql_fetch_array($result)) {
		if ($row['status'] == 0) {
        	$res = show_info('fail', '请求已经发送');
        } else {
        	$res = show_info('fail', '对方已经是你的好友了');
        }
        echo json_encode($res);
        
        mysql_close($conn);

        return 0;
    }
    unset($result);
    
    $sql = "INSERT INTO liao_relationship (myid, fid, pid, status) ";
    $sql .= " VALUES ('{$myid}', '{$fid}', '{$nickname}', 0, 0) ";
    if (!mysql_query($sql, $conn)) {
        mysql_close($conn);

        $res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
        echo json_encode($res);

        return 0;
    }

    $affected_rows = mysql_affected_rows();

    mysql_close($conn);

    if ($affected_rows == 1) {
        $res = show_info('succ', '请求成功!');
    } else {
        $res = show_info('fail', '请求失败!');
    }

    echo json_encode($res);	
}

?>
