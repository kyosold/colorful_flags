<?php

include_once('common.php');
include_once('utils.php');

$qid = trim(avoid_sql($_POST['qid']));
$action = trim(avoid_sql($_POST['action']));

$res = array();

if (strlen($qid) > 0) {
	$myid = 0;
	$fid = 0;

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);

	$sql = "SELECT id, tag_type, fuid, fnick, fios_token, tuid, tios_token, queue_type, queue_file FROM liao_queue WHERE id = {$qid} AND expire = 0 LIMIT 1";
//echo $sql;
    $result = mysql_query($sql);
    if ($row = mysql_fetch_array($result)) {
       	$myid = $row['tuid']; 
       	$fid = $row['fuid']; 
    }
	unset($result);

	if ($myid == 0 || $fid == 0) {
		mysql_close($conn);
		$res = show_info('fail', '统出错(4005)，请稍候重试 :)');
		echo json_encode($res);
		return 0;
	}

	$sql = "UPDATE liao_queue SET expire = 1 WHERE id = {$qid} LIMIT 1";
	if (!mysql_query($sql, $conn)) {
		mysql_close($conn);

		$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
		echo json_encode($res);
		return 0;
	}	


	if ($action == "add") {
		$sql = "INSERT INTO liao_relationship (myid, fid, status) VALUES ";
		$sql .= "({$myid}, {$fid}, 0), ({$fid}, {$myid}, 0);";	

		if (!mysql_query($sql, $conn)) {
			mysql_close($conn);
			$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
			echo json_encode($res);
			return 0;
		}
		$affected_rows = mysql_affected_rows();
		if ($affected_rows != 2) {
			mysql_query("UPDATE liao_queue SET expire = 0 WHERE id = {$qid} LIMIT 1", $conn);

			mysql_close($conn);
			$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
			echo json_encode($res);
			return 0;
		}
	} else if ($action == "cancel" || $action == "reject") {

	}

	mysql_close($conn);

	$res = show_info('succ', '添加成功');
    
	echo json_encode($res);

    return 0;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}




echo json_encode($res);

?>

