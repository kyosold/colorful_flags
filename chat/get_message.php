<?php

$uid = $_POST['uid'];
$fuid = $_POST['fuid'];
$qid = $_POST['qid'];
$number = $_POST['number'];
$type = $_POST['type'];

$data = array();
$data['uid'] = $uid;
$data['fuid'] = $fuid;
$data['qid'] = $qid;
$data['type'] = $type;
$data['number'] = $number;

$uri = 'http://sc.vmeila.com/api/get_messagesv2.php';

$ch = curl_init();

curl_setopt($ch, CURLOPT_URL, $uri);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_HEADER, 0);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, $data);

$return = curl_exec($ch);

curl_close($ch);

//print_r($return);

$jsonRes = json_decode($return, true);
//var_dump($jsonRes);
//var_dump($jsonRes['queue']);
foreach( $jsonRes['queue'] as &$item) {
	if ($item['queue_type'] == 'TXT' || $item['queue_type'] == 'SCMTXT') {
		$content = base64_decode( $item['queue_content'] );
		$item['queue_content'] = $content;
	}
}

echo json_encode($jsonRes);

/*$res = json_decode($return, TRUE);
$webRes = array();
foreach ($res => $item) {
	$echo $item."\n";
}*/


?>
