<?php

$name = $_POST['name'];

$res = array();
if (strlen($name) > 0) {

	$i = 0;
	$data = array();

	for ($i=1; $i<5; $i++) {
		$item_idx = $i;
		$item_name = $name."_".$i;
		$gender = (($i % 2) == 0) ? "female" : "male";
		$item = array(
			'id'=> $item_idx, 
			'uid'=>$item_idx, 
			'name'=>$item_name, 
			'gender'=>$gender,
			'birthday'=>'1982-06-14'
			);
	
		$data[] = $item;
	}
	
	$res['status'] = 'succ';
	$res['data'] = $data;

} else {
	$res['status'] = 'fail';
	$res['des'] = 'parameters error';
}

echo json_encode($res);

?>

