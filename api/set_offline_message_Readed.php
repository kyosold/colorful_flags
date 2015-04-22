<?php

include_once('common.php');
include_once('utils.php');

$ids = trim(avoid_sql($_POST['ids']));

$res = array();

if (strlen($ids) > 0) {

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);

	$sql = "UPDATE liao_queue SET expire = 1 WHERE id IN ({$ids});";
	if (!mysql_query($sql, $conn)) {
		mysql_close($conn);

		$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
		echo json_encode($res);
		return 0;
	}	

	$affected_rows = mysql_affected_rows();

	mysql_close($conn);

	$res = show_info('succ', '更新成功');
	$res['des'] = $affected_rows;
    
	echo json_encode($res);

    return 0;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}




echo json_encode($res);

?>

