<?php

include_once('common.php');
include_once('utils.php');

$uid = trim(avoid_sql($_POST['uid']));
$nickname = trim(avoid_sql($_POST['nickname']));
$gender = trim(avoid_sql($_POST['gender']));
$birthday = trim(avoid_sql($_POST['birthday']));


$res = array();

if (strlen($uid) > 0) {
	$mydata = array();

/*$res = show_info('succ', '更新成功!')
$res['des'] = $_FILES['avatar'];
echo json_encode($res);
return;*/

	$update_avatar = 0;
	if (strlen($_FILES['avatar']['name']) > 0) {
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

		// 检查是否已经有头像,有的就删除
		if (file_exists($avatar_img)) {
			unlink($avatar_img);
		}
		if (file_exists($avatar_thumb)) {
			unlink($avatar_thumb);
		}

		if (strlen($avatar_name) > 0 && $avatar_size > 0) {
			if (move_uploaded_file($_FILES['avatar']['tmp_name'], $avatar_img)) {
				$img = new Imagick($avatar_img);
				$img->thumbnailImage(100, 100);	
				file_put_contents($avatar_thumb, $img);

				$mydata['icon'] = get_avatar_url($uid);
				$update_avatar = 1;
			}	
		}
	}


	// 更新数据库
	$set_str = "";
	if (strlen($nickname) > 0) {
		$set_str .= " nickname = '{$nickname} '";
		$mydata['nickname'] = $nickname;
	}
	if (strlen($gender) > 0) {
		if (strlen($set_str) > 0) {
			$set_str .= ", ";
		}
		$set_str .= " gender = {$gender} ";
		$mydata['gender'] = $gender;
	}
	if (strlen($birthday) > 0) {
		if (strlen($set_str) > 0) {
			 $set_str .= ", ";
		}	
		$set_str .= " birthday = '{$birthday}' ";
		$mydata['birthday'] = $birthday;
	}

	if (strlen($set_str) > 0) {
		// 更新信息
		$conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
		if (!$conn) {
			$res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
			echo json_encode($res);
			
			return 0;
		}

		mysql_query("set names utf8");
		mysql_select_db(DB_NAME, $conn);
		
		// 更新用户信息
		$sql = "UPDATE liao_user SET {$set_str} WHERE id = {$uid} LIMIT 1";
		if (!mysql_query($sql, $conn)) {
			mysql_close($conn);

			$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
			echo json_encode($res);

			return 0;
		}

		$affected_rows = mysql_affected_rows();

		mysql_close($conn);

	}

    


	if ($affected_rows > 0 || $update_avatar == 1) {
		$mydata['icon'] = get_avatar_url($uid);

    	$res = show_info('succ', '更新成功!');
		$res['myself'] = $mydata;
	} else {
    	$res = show_info('succ', '更新成功!');
		$res['myself'] = $mydata;
$res['update_avatar'] = $update_avatar;
	}
    echo json_encode($res);
	
} else {

	$res['status'] = 'fail';
	$res['des'] = 'parameters error';

	echo json_encode($res);
}





?>
