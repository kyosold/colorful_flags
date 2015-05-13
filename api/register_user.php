<?php

include_once('common.php');
include_once('utils.php');

$account = trim(avoid_sql($_POST['account']));
$password = trim(avoid_sql($_POST['password']));
$nickname = trim(avoid_sql($_POST['nickname']));
$gender = trim(avoid_sql($_POST['gender']));
$birthday = trim(avoid_sql($_POST['birthday']));
$devToken = trim(avoid_sql($_POST['ios_token']));
$avatarData = $_POST["avatar"];


$res = array();

if ((strlen($account) > 0) &&
	(strlen($password) > 0) &&
	(strlen($gender) > 0) &&
	(strlen($nickname) > 0) &&
	(strlen($birthday) > 0) &&
	(strlen($devToken) > 0)) {


	$password = md5($password); 
    
    // 提交注册
    $conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);
	
	$sql = "SELECT id FROM liao_pwds WHERE account = '{$account}' LIMIT 1";
    $result = mysql_query($sql);
    if ($row = mysql_fetch_array($result)) {
        mysql_close($conn);

        $res = show_info('fail', '该帐号已经存在，换个帐号使用吧 :)');
        echo json_encode($res);

        return 0;
    }
    unset($result);
    
	// 注册用户信息
    $sql = "INSERT INTO liao_user (nickname, birthday, gender, ios_token, status, icon) ";
    $sql .= " VALUES ('{$nickname}', '{$birthday}', $gender, '{$devToken}', 0, '') ";
    if (!mysql_query($sql, $conn)) {
        mysql_close($conn);

        $res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
        echo json_encode($res);

        return 0;
    }

    $affected_rows = mysql_affected_rows();

	$uid = mysql_insert_id();

	// 注册用户帐号
	$sql = "INSERT INTO liao_pwds (account, passwd, uid) VALUES ('{$account}', '{$password}', {$uid}) ";
	if (!mysql_query($sql, $conn)) {
		// 回滚

		$sql = "DELETE FROM liao_user WHTERE id = {$uid} LIMIT 1";
		mysql_query($sql, $conn);

		mysql_close($conn);

		$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
		echo json_encode($res);

		return 0;
	}	

    mysql_close($conn);

    if ($affected_rows == 1) {

		$avatar_path = AVATAR_PATH ."/". $uid ."/avatar/";
		$avatar_img = $avatar_path ."avatar.jpg";
		$avatar_thumb = $avatar_path ."s_avatar.jpg";
	
		// 创建用户目录
		if (mk_dir($avatar_path) != TRUE) {
			$res = show_info('fail', '系统出错(4004)，请稍候重试 :)');
			$res['data'] = $avatar_path;
			echo json_encode($res);

			return 1;
		}

		// 检查头像是否有数据,有的话写文件
	/* avatar =     {
        error = 0;
        name = "avatar.jpg";
        size = 1149069;
        "tmp_name" = "/tmp/phpEOvfiK";
        type = "image/jpeg";
    	};
	*/
		$avatar_name = strtolower($_FILES['avatar']['name']);
		$avatar_size = $_FILES['avatar']['size'];

		if (strlen($avatar_name) > 0 && $avatar_size > 0) {
			if (move_uploaded_file($_FILES['avatar']['tmp_name'], $avatar_img)) {
				$img = new Imagick($avatar_img);
				$img->thumbnailImage(100, 100);	
				file_put_contents($avatar_thumb, $img);
			}	
		}

        $res = show_info('succ', '注册成功!');
    } else {
        $res = show_info('fail', '注册失败!');
    }

    echo json_encode($res);
	
} else {

	$res['status'] = 'fail';
	$res['des'] = 'parameters error';

	echo json_encode($res);
}





?>
