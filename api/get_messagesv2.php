<?php

include_once('common.php');
include_once('utils.php');

$qid = 0;
$start = 0;
$number = 20;

$uid = trim(avoid_sql($_POST['uid']));
$fuid = trim(avoid_sql($_POST['fuid']));
$qid = trim(avoid_sql($_POST['qid']));
$number = trim(avoid_sql($_POST['number']));
$type = trim(avoid_sql($_POST['type']));

$res = array();

if ((strlen($uid) > 0) && (strlen($fuid) > 0)) {

	$msg_queue = array();

	$sql = "";
	$select = "SELECT id, fuid, fnick, tuid, cdate, queue_file, queue_type, queue_size, expire, fdel, tdel FROM liao_queue ";
	$where = "";
	$order = "";
	if ($type == 'LASTEST') {
		$where = "WHERE (tuid = {$uid} OR fuid = {$uid}) AND (tuid = {$fuid} OR fuid = {$fuid}) ";

		// number + 1: 用于给客户端显示是否还有可用数据
		$order = "ORDER BY cdate DESC LIMIT {$number}";

	} else if ($type == 'PARTNEW') {
		if (strlen($qid) <= 0) {
			$res['status'] = 'fail';
			$res['des'] = 'parameters error';
			
			echo json_encode($res);
			return 0;
		}

		$where = "WHERE (tuid = {$uid} OR fuid = {$uid}) AND (tuid = {$fuid} OR fuid = {$fuid}) AND (id > {$qid}) ";

		// number + 1: 用于给客户端显示是否还有可用数据
		$number++;
		$order = "ORDER BY id ASC LIMIT {$number}";

	} else if ($type == 'PARTOLD') {
		if (strlen($qid) <= 0) {
			$res['status'] = 'fail';
			$res['des'] = 'parameters error';
			
			echo json_encode($res);
			return 0;
		}

		$where = "WHERE (tuid = {$uid} OR fuid = {$uid}) AND (tuid = {$fuid} OR fuid = {$fuid}) AND (id < {$qid}) ";

		$order = "ORDER BY id DESC LIMIT {$number}";
		
	} else if ($type == 'UNREAD') {
		if (strlen($qid) <= 0) {
			$res['status'] = 'fail';
			$res['des'] = 'parameters error';
			
			echo json_encode($res);
			return 0;
		}

		$where = "WHERE tuid = {$uid} AND fuid = {$fuid} AND expire = 0 AND id > {$qid} ";	

		// number + 1: 用于给客户端显示是否还有可用数据
		$number++;
		$order = "ORDER BY id ASC LIMIT {$number}";

	} else if ($type == 'ALL') {
		$where = "WHERE (tuid = {$uid} OR fuid = {$uid}) AND (tuid = {$fuid} OR fuid = {$fuid}) AND (id > {$qid}) ";
		// number + 1: 用于给客户端显示是否还有可用数据
		$number++;
		$order = "ORDER BY id ASC LIMIT {$number}";
	
	} else if ($type == 'QID') {
		$where = "WHERE id = {$qid} ";
		$order = "ORDER BY id ASC LIMIT {$number}";

	} else if ($type == 'Previous') {
		$where = "WHERE (tuid = {$uid} OR fuid = {$uid}) AND (tuid = {$fuid} OR fuid = {$fuid}) AND (id < {$qid}) ";
		$order = "ORDER BY id ASC LIMIT {$number}";
	}

	$sql = $select . $where . $order;

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		$res['sql'] = $sql;
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);

    $result = mysql_query($sql);
    while ($row = mysql_fetch_array($result)) {

		// 过滤掉已经被自己删除的
		if ($row['fdel'] == 1) {
			if ($row['fuid'] == $uid)
				continue;
		} 
		if ($row['tdel'] == 1) {
			if ($row['tuid'] == $uid)
				continue;
		}

        $idx_data = array();
        $idx_data['id'] = $row['id'];
        $idx_data['fuid'] = $row['fuid'];
        $idx_data['fnick'] = $row['fnick'];
        $idx_data['tuid'] = $row['tuid'];
        $idx_data['cdate'] = $row['cdate'];
        $idx_data['queue_file'] = $row['queue_file'];
        $idx_data['queue_type'] = $row['queue_type'];
        $idx_data['queue_size'] = $row['queue_size'];
        $idx_data['expire'] = $row['expire'];

		if ($idx_data['queue_type'] == 'TXT') {
			$idx_data['queue_content'] = file_get_contents($idx_data['queue_file']);
		}

		$msg_queue[] = $idx_data;
    }
	mysql_close($conn);

	$msg_queue2 = array();
	$mi = count($msg_queue)-1;
	for ($mi; $mi >= 0; $mi--) {
		$idx_data = array();
		$idx_data = $msg_queue[$mi];
		$msg_queue2[] = $idx_data;
	}

	$res = show_info('succ', '获取成功');
	$res['queue'] = $msg_queue2;
	$res['sql'] = $sql;
    
	echo json_encode($res);

    return 0;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}




echo json_encode($res);

?>

