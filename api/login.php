<?php

include_once('common.php');
include_once('utils.php');

$account = trim(avoid_sql($_POST['account']));
$password = trim(avoid_sql($_POST['password']));
$devToken = trim(avoid_sql($_POST['ios_token']));
$get_friends_list = trim(avoid_sql($_POST['get_friends_list']));

$res = array();

if ((strlen($account) > 0) &&
	(strlen($password) > 0) &&
	(strlen($devToken) > 0)) {

	$password = md5($password); 
    
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
    
        mysql_close($conn);
        
        $data = array();
        $data['id'] = $row['id'];
        $data['account'] = $row['account'];
        $data['nickname'] = $row['nickname'];
        $data['birthday'] = $row['birthday'];
        $data['gender'] = $row['gender'];
        $data['status'] = $row['status'];
        $data['icon'] = get_avatar_url($row['id']);

        $res = show_info('succ', '登录成功');
        $res['myself'] = $data;
        echo json_encode($res);

        return 0;
    } else {
    	mysql_close($conn);

        $res = show_info('fail', '登录失败, 帐号或密码错误');
        echo json_encode($res);

        return 0;
    }
	
} else {

	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}



echo json_encode($res);


?>
