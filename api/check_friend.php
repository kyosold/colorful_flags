<?php

include_once('common.php');
include_once('utils.php');

$myid = trim(avoid_sql($_POST['myid']));
$fid = trim(avoid_sql($_POST['fid']));

$res = array();

if (strlen($myid) > 0 &&
	strlen($fid) > 0) {

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	$sql = "SELECT id FROM liao_queue WHERE fuid = '{$myid}' AND tuid = '{$fid}' AND tag_type = 'RECVADDFRDREQ' AND expire = '0' LIMIT 1";
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
    
    mysql_close($conn);

    $res = show_info('succ', '正常,可以继续发送请求!');
	$res['sql'] = $sql;

    echo json_encode($res);	
	return;
}

$res['status'] = 'fail';
$res['des'] = 'parameters error';

echo json_encode($res);



?>
