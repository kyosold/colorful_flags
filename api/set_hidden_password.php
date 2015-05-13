<?php

include_once('common.php');
include_once('utils.php');

$ptype = trim(avoid_sql($_POST['ptype']));
$htype = trim(avoid_sql($_POST['htype']));
$myuid = trim(avoid_sql($_POST['myuid']));
$fuid = trim(avoid_sql($_POST['fuid']));
$password = trim(avoid_sql($_POST['password']));

$res = array();

$pwd_num = 0;

if ((strlen($ptype) > 0) &&
	(strlen($htype) >= 0) &&
	(strlen($password) > 0) &&
	(strlen($myuid) > 0) &&
	(strlen($fuid) > 0)) {

	$password = md5($password); 
	$pid = 0;
	$account = '';
    
    // 提交注册
    $conn = mysql_connect(DB_HOST, DB_USER, DB_PWD);
    if (!$conn) {
        $res = show_info('fail', '系统出错(2000)，请稍候重试 :)');
		echo json_encode($res);
		
        return 0;
    }

    mysql_query("set names utf8");
    mysql_select_db(DB_NAME, $conn);

	if ($ptype == 'new') {
		// 查询密码是否已经存在,并且获取帐号
		$sql = "SELECT id, account, passwd FROM liao_pwds WHERE uid = {$myuid}";
		$result = mysql_query($sql);
		while ($row = mysql_fetch_array($result)) {
			if ($pwd_num > MAX_PWD_NUM_NOR) {
				mysql_close($conn);

				$res = show_info('fail', '密码数量超限,请升级该帐号');
				echo json_encode($res);

				return 0;
			}

			if ($password == $row['passwd']) {
				mysql_close($conn);

				$res = show_info('fail', '该密码存在');
				echo json_encode($res);

				return 0;
			}

			$account = $row['account'];

			$pwd_num++;
		}
		unset($result);

		if ($account == "") {
			mysql_close($conn);
			$res = show_info('fail', '系统出错(4006)，请稍候重试 :)');
			echo json_encode($res);
			return 0;
		}

		
		// 添加用户新密码
		$sql = "INSERT INTO liao_pwds (account, passwd, uid) VALUES ('{$account}', '{$password}', {$myuid}) ";
		if (!mysql_query($sql, $conn)) {
			mysql_close($conn);

			$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
			echo json_encode($res);

			return 0;
		}	
		
		$affected_rows = mysql_affected_rows();
		$pid = mysql_insert_id();

	} else if ($ptype == "update") {
		// 查询用户密码的id
		$sql = "SELECT id FROM liao_pwds WHERE uid = {$myuid} AND passwd = '{$password}' LIMIT 1";
		$result = mysql_query($sql); 
		if ($row = mysql_fetch_array($result)) {
			$pid = $row['id'];
		}
	}

	if ($pid > 0) {
		// 更新好友隐藏状态
		$sql = "UPDATE liao_relationship SET pid = {$pid}, status = {$htype} WHERE myid = {$myuid} AND fid = {$fuid} LIMIT 1";
		if (!mysql_query($sql, $conn)) {
			// 回滚
			$sql = "DELETE FROM liao_pwds WHERE id = {$pid} LIMIT 1";
			mysql_query($sql, $conn);
		
			mysql_close($conn);

			$res = show_info('fail', '系统出错(2001)，请稍候重试 :)');
			echo json_encode($res);

			return 0;
		}
		$affected_rows = mysql_affected_rows();

		mysql_close($conn);

		//if ($affected_rows == 1) {
			$res = show_info('succ', '操作成功!');
		//} else {
		//	$res = show_info('fail', '操作失!');
		//}
	} else {
		mysql_close($conn);
		$res = show_info('fail', '操作失败!');
	}

    echo json_encode($res);
	
} else {

	$res['status'] = 'fail';
	$res['des'] = 'parameters error';

	echo json_encode($res);
}





?>
