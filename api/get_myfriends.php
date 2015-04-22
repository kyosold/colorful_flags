<?php

include_once('common.php');
include_once('utils.php');

$page = 0;

$uid = trim(avoid_sql($_POST['uid']));

$res = array();

if (strlen($uid) > 0) {

	$friends = array();

	$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	//$sql = "SELECT u.id as uid, u.nickname as nickname, u.birthday as birthday, u.gender as gender, u.ios_token as ios_token, r.status as status FROM liao_user AS u, liao_relationship AS r WHERE u.id IN (SELECT fid FROM liao_relationship WHERE myid = {$uid})";
	$sql = "SELECT u.id as uid, u.nickname as nickname, u.birthday as birthday, u.gender as gender, u.ios_token as ios_token, r.status as status FROM liao_user AS u, liao_relationship AS r WHERE u.id = r.fid AND r.myid = {$uid}";
//echo $sql;
    $result = mysql_query($sql);
    while ($row = mysql_fetch_array($result)) {
        $friend_data = array();
        $friend_data['uid'] = $row['uid'];
        $friend_data['nickname'] = $row['nickname'];
        $friend_data['birthday'] = $row['birthday'];
        $friend_data['gender'] = $row['gender'];
        $friend_data['ios_token'] = $row['ios_token'];
        $friend_data['status'] = $row['status'];
        $friend_data['icon'] = get_avatar_url($row['uid']);

        $friends[] = $friend_data; 
    }
	mysql_close($conn);


	$res = show_info('succ', '查询成功');
	$res['friends'] = $friends;
    
	echo json_encode($res);

    return 0;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}




echo json_encode($res);

?>

