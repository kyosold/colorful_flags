<?php

include_once('common.php');
include_once('utils.php');

$page = 0;

$name = trim(avoid_sql($_POST['name']));
$page = trim(avoid_sql($_POST['page']));

$res = array();

if (strlen($name) > 0) {

	$members = array();

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	$sql = "SELECT id, account, nickname, birthday, gender, ios_token, status, icon FROM liao_user WHERE account like '%{$account}%' LIMIT ". ($page * ROWS_OF_PAGE) ." ". ROWS_OF_PAGE;
    $result = mysql_query($sql);
    while ($row = mysql_fetch_array($result)) {
        mysql_close($conn);
        
        $data = array();
        $data['id'] = $row['id'];
        $data['account'] = $row['account'];
        $data['nickname'] = $row['nickname'];
        $data['birthday'] = $row['birthday'];
        $data['gender'] = $row['gender'];
        $data['ios_token'] = $row['ios_token'];
        $data['status'] = $row['status'];
        $data['icon'] = $row['icon'];
        
        $members[] = $data;
        
    }

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

