<?php

include_once('common.php');
include_once('utils.php');

$page = 0;

$uid = trim(avoid_sql($_POST['uid']));
$expire = trim(avoid_sql($_POST['expire']));

$res = array();

if (strlen($uid) > 0) {

	$members = array();

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);

	$sql = "";
	if ($expire == 'unread') {		
		$sql = "SELECT id, tag_type, fuid, fnick, fios_token, tuid, tios_token, queue_type, queue_file FROM liao_queue WHERE tuid = {$uid} AND expire = 0 AND queue_type = 'SYS' ORDER BY id";
	} else {
		$sql = "SELECT id, tag_type, fuid, fnick, fios_token, tuid, tios_token, queue_type, queue_file FROM liao_queue WHERE tuid = {$uid} AND queue_type = 'SYS' ORDER BY id";
	}
	
//echo $sql;
    $result = mysql_query($sql);
    while ($row = mysql_fetch_array($result)) {
        
        $data = array();
        $data['id'] = $row['id'];
		$data['tag_type'] = $row['tag_type'];
        $data['fuid'] = $row['fuid'];
        $data['fnick'] = $row['fnick'];
        $data['fios_token'] = $row['fios_token'];
        $data['ficon'] = get_avatar_url($row['fuid']);
        $data['tuid'] = $row['tuid'];
        $data['tios_token'] = $row['tios_token'];
        $data['queue_type'] = $row['queue_type'];
        $data['queue_file'] = $row['queue_file'];
        
        $members[] = $data;
        
    }
	mysql_close($conn);


	$res = show_info('succ', '查询成功');
    $res['data'] = $members;
    
	echo json_encode($res);

    return 0;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}




echo json_encode($res);

?>

