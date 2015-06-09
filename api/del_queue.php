<?php

include_once('common.php');
include_once('utils.php');

$qid = trim(avoid_sql($_POST['qid']));
$fdel = trim(avoid_sql($_POST['fdel']));
$tdel = trim(avoid_sql($_POST['tdel']));

$res = array();

if (strlen($qid) > 0) {

	// 更新用户信息
	if ($fdel == 1) {
		$sql = "UPDATE liao_queue SET fdel = {$fdel} WHERE id = {$qid} LIMIT 1";
	} else if ($tdel == 1) {
		$sql = "UPDATE liao_queue SET tdel = {$tdel} WHERE id = {$qid} LIMIT 1";
	} else {
		$res = show_info('succ', 'fdel and tdel both is null, do not need update');
		echo json_encode($res);
		return 0;
	}

	// 更新信息
	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
	if (!$conn) {
		$res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
		return 0;
	}

	mysql_query("set names utf8");
	mysql_select_db(DB_NAME, $conn);
	
	if (!mysql_query($sql, $conn)) {
		mysql_close($conn);

		$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
		$res['sql'] = $sql;
		echo json_encode($res);

		return 0;
	}

	$affected_rows = mysql_affected_rows();

	mysql_close($conn);


	if ($affected_rows > 0 || $update_avatar == 1) {
    	$res = show_info('succ', '更新成功!');
		$res['sql'] = $sql;
	} else {
    	$res = show_info('succ', '更新成功!');
		$res['sql'] = $sql;
	}
    echo json_encode($res);
	
} else {

	$res['status'] = 'fail';
	$res['des'] = 'parameters error';

	echo json_encode($res);
}





?>
