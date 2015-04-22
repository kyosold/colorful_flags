<?php

include_once('common.php');
include_once('utils.php');

$page = 0;
$number = 20;

$uid = trim(avoid_sql($_POST['uid']));
$fuid = trim(avoid_sql($_POST['fuid']));
$page = trim(avoid_sql($_POST['page']));
$number = trim(avoid_sql($_POST['number']));
$type = trim(avoid_sql($_POST['type']));

$res = array();

if (strlen($uid) > 0) {

	$msg_queue = array();

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	$sql = "";
	if ($type == 'unread') {
		$sql = "SELECT id, fuid, tuid, cdate, queue_file, queue_type FROM liao_queue WHERE tuid = {$uid} AND fuid = {$fuid} AND expire = 0 ORDER BY id ASC LIMIT {$page}, {$number}";
	} else {
		$sql = "SELECT id, fuid, tuid, cdate, queue_file, queue_type FROM liao_queue WHERE (tuid = {$uid} OR fuid = {$uid}) AND (tuid = {$fuid} OR fuid = {$fuid}) ORDER BY id ASC LIMIT {$page}, {$number}";
	}
//die();
    $result = mysql_query($sql);
    while ($row = mysql_fetch_array($result)) {

        $idx_data = array();
        $idx_data['id'] = $row['id'];
        $idx_data['fuid'] = $row['fuid'];
        $idx_data['tuid'] = $row['tuid'];
        $idx_data['cdate'] = $row['cdate'];
        $idx_data['queue_file'] = $row['queue_file'];
        $idx_data['queue_type'] = $row['queue_type'];

		if ($idx_data['queue_type'] == 'TXT') {
			$idx_data['queue_content'] = file_get_contents($idx_data['queue_file']);
		}

		$msg_queue[] = $idx_data;
    }
	mysql_close($conn);


	$res = show_info('succ', '获取成功');
	$res['queue'] = $msg_queue;
    
	echo json_encode($res);

    return 0;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}




echo json_encode($res);

?>

