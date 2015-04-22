<?php

include_once('common.php');
include_once('utils.php');

$account = trim(avoid_sql($_POST['account']));
$password = trim(avoid_sql($_POST['password']));
$devToken = trim(avoid_sql($_POST['ios_token']));
$get_friends_list = trim(avoid_sql($_POST['get_friend_list']));

$res = array();

if ((strlen($account) > 0) &&
	(strlen($password) > 0) &&
	(strlen($devToken) > 0)) {

	$password = md5($password); 

	$mydata = array();
    
    $conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	$sql = "SELECT id, account, nickname, birthday, gender, ios_token, status FROM liao_user WHERE account = '{$account}' and password = '{$password}' LIMIT 1";
    $result = mysql_query($sql);
    if ($row = mysql_fetch_array($result)) {
    	if ($row['ios_token'] != $devToken) {
    		// 更新ios_token
    		$sql = "UPDATE liao_user SET ios_token = '{$devToken}' WHERE id = ". $row['id'] . " LIMIT 1";
    		
    		if (!mysql_query($sql, $conn)) {
    			mysql_close($conn);
    			
    			$res = show_info('fail', '系统出错(4000), 请稍候重试 :)');
    			echo json_encode($res);
    			
    			return 0;
    		}
    	}
    
        $mydata['id'] = $row['id'];
        $mydata['account'] = $row['account'];
        $mydata['nickname'] = $row['nickname'];
        $mydata['birthday'] = $row['birthday'];
        $mydata['gender'] = $row['gender'];
        $mydata['status'] = $row['status'];
        $mydata['icon'] = get_avatar_url($row['id']);

    } else {
    	mysql_close($conn);

        $res = show_info('fail', '登录失败, 帐号或密码错误');
        echo json_encode($res);

        return 0;
    }
	unset($result);

	$friends = array();
	$sql = "SELECT u.id as uid, u.nickname as nickname, u.birthday as birthday, u.gender as gender, u.ios_token as ios_token, r.status as status FROM liao_user AS u, liao_relationship AS r WHERE u.id = r.fid AND r.myid = {$mydata['id']}";
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
	unset($result);
	mysql_close($conn);
	
    $res = show_info('succ', '登录成功');
    $res['myself'] = $mydata;
	$res['friends'] = $friends;
    echo json_encode($res);

    return 0;
	
} else {

	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}



echo json_encode($res);


?>
