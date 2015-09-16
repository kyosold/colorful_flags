var myData;
var friendsData; 
var talkingFriend;
var _maxQID;
var _minQID;

function get_login_cmd(user, pwd) {
	var login_cmd = "LOGIN account:"+ user +" password:"+ pwd +" ios_token:000 get_friend_list:1 \r\n\r\n";
	return login_cmd;
}

function get_sendtxt_cmd(qtype, fuid, fnick, tuid, tios_token, message) {
	var time = Date.parse(new Date())/1000;
	$.base64.utf8encode = true;
	var b64_fnick = $.base64.btoa(fnick);
	var b64_message = $.base64.btoa(message);
	
	var txt_cmd = "SENDTXT mid:0_"+ time +" qtype:"+ qtype +" fuid:"+ fuid +" fios_token:000 fnick:"+ b64_fnick +" tuid:"+ tuid +" tios_token:"+ tios_token +" message:"+ b64_message +"\r\n\r\n";
	return txt_cmd;
}


function outFriends(friendData) {
	var i = 0;
	//var outHTML = '';
	while (i < friendData.length) {
		var friend = friendData[i];
		
		//var html = "<li id='friendItem_"+i+"' onclick='clickFriendToTalk("+i+");'>";
		var html = "<i id='friendStatus_"+ friend.uid +"'></i>";
		html += '<div class="friend-nav-list">';
		html += '<figure class="friend_avatar">';
		html += '<img src="'+ friend.icon +'"/>';
		html += '</figure>';
		html += '<div class="meta">';
		html += '<div class="meta__name">'+ friend.nickname +'</div>';
		html += '<div class="meta__sub--dark">';

		if (friend.gender == 0) {
			html += '女';
		} else {
			html += '男';
		}

		html += '</div>';
		html += '</div>';
		html += '</div>';
		html += '</li>';

		//outHTML += html;

		var li = document.createElement("li");
		li.setAttribute("id", "friendItem_"+ friend.uid);
		li.setAttribute("onclick", "clickFriendToTalk("+ friend.uid +");");
		li.setAttribute("uid", friend.uid);
		li.innerHTML = html;

		$('#firendDiv')[0].appendChild(li);	
	
		i++;
	}

	//$('#firendDiv')[0].innerHTML = outHTML;
}

function moveFriendNavToTop(fuid) {
	var topFriendItem = "friendItem_"+ fuid;
	var topItem = $('#'+topFriendItem)[0];
	topItem.setAttribute('class', 'selected');

	$('#firendDiv')[0].removeChild(topItem);

	var flist = $('#firendDiv')[0].childNodes;
	var topNode = flist[0];
	if (topNode) {
		topNode.setAttribute('class', '');
	}
	
	$('#firendDiv')[0].insertBefore(topItem, topNode);
}

function unreadStatusFriendNavWithUID(fuid) {
	var topFriendStatus = "friendStatus_"+ fuid;
	var topStatus = $('#'+topFriendStatus)[0];
	topStatus.setAttribute('class', 'status_indicator_unread');
		
	moveFriendNavToTop(fuid);
}
function nullStatusFriendNavWithUID(fuid) {
	var topFriendStatus = "friendStatus_"+ fuid;
	var topStatus = $('#'+topFriendStatus)[0];
	topStatus.setAttribute('class', '');
}

function getFriendNavIndexWithUID(uid) {
	var flist = $('#firendDiv')[0].childNodes;
	for (i=0; i<flist.length; i++) {
		var fuid = flist[i].getAttribute('uid');
		if (uid == fuid) {
			return i;
		}
	}

	return -1;
}
function getFriendDataIndexWithUID(uid) {
	if ( friendsData === undefined ) {
		return -1;
	}	
	for (i=0; i<friendsData.length; i++) {
		if (uid == friendsData[i].uid) {
			return i;
		}
	}

	return -1;
}


function parse_response_data(data) {
	var resData = data;
	var resArray = resData.split(" ");
        
    var resType = resArray[0];
    var resStatus = resArray[1];

    if (resType == "LOGIN") {
        if (resStatus == "OK") {
            var resJSONString;

            if (resArray.length > 3) {
                var resJSON = new Array();
                var i = 2;
                while (i < resArray.length) {
                    resJSON.push(resArray[i]);
                    i++;
                }
                resJSONString = resJSON.join(" ");
            } else {
                resJSONString = resArray[2];
            }

            var jsonData = eval('('+ resJSONString +')');
			myData = jsonData.myself;
            friendsData = jsonData.friends;

			LoginedWithName(myData.nickname);

            outFriends(friendsData, document.getElementById('friendsDIV'));

        } else {
            alert("登录失败");

            showLoginDialog();
        }

		return;

    } else if (resType =="SENDTXT") {
		

		return;

	} else if (resType =="MESSAGENOTICE") {
		if (resArray.length != 7) {
			log("get buffer error");
			return;
		}
		$.base64.utf8decode = true;

		var fuid = resArray[2];
		var fnick = resArray[3];
		var tuid = resArray[4];
		var fmess = resArray[5];

		// --- 查询发件人是否被隐藏
		// 暂不实现
		//
		
		var fnickB64 = $.base64.atob(fnick);
		var fmessB64 = $.base64.atob(fmess);

		// 有新消息到达
		if (resStatus == "SYS") {

		} else if (resStatus == "DELETESCM") {

		} else if (resStatus == "TXT"
				|| resStatus == "SCMTXT"
				|| resStatus == "IMG"
				|| resStatus == "SCMIMG"
				|| resStatus == "AUDIO"
				|| resStatus == "SCMAUDIO"
		) {
			var isTalking = isChattingStayFUID(fuid);
			if (isTalking == 1) {
				// 正在聊天
				_getOnlineDataWithProcessType(fuid, 3);
			} else {
				// 没在聊天
				unreadStatusFriendNavWithUID(fuid);					
			}
		}
		

		return;

	}

}


function clickFriendToTalk(uid) {
	_maxQID = 0;
	_minQID = 0;
	_getOnlineDataWithProcessType(uid, 0);

}


function _getOnlineDataWithProcessType(uid, processType) {
	var ifd = getFriendDataIndexWithUID(uid);
	if (ifd < 0) {
		return;
	}

	var friend = friendsData[ifd];
	talkingFriend = friendsData[ifd];

	var data;
	if (processType == 0) {
		data = {
			'uid': myData.id,
			'fuid': friend.uid,
			'type': 'LASTEST',
			'number': '8'
		};
			
	} else if (processType == 1) {
		data = {
			'uid': myData.id,
			'fuid': friend.uid,
			'type': 'PARTNEW',
			'qid': _maxQID,
			'number': '8'
		};

	} else if (processType == 2) {
		data = {
			'uid': myData.id,
			'fuid': friend.uid,
			'type': 'PARTOLD',
			'qid': _minQID,
			'number': '8'
		};

	} else if (processType == 3) {
		data = {
			'uid': myData.id,
			'fuid': friend.uid,
			'type': 'UNREAD',
			'qid': _maxQID,
			'number': '8'
		};
	}


	$.ajax({
		type: 'post',
		url:'./get_message.php',
		data: data,
		beforeSend: function(XMLHttpRequest) { 
			if (processType == 3) {
				log("Loading unread message");
			} else {
				showLoadDialog();
			}
		},
		error: function(XMLHttpRequest, textStatus, errorThrown) {
			if (processType == 3) {
				log("");
			} else {
				hideLoadDialog();
			}
			alert("error:"+XMLHttpRequest.status+":"+XMLHttpRequest.readyState);
		},
		success: function(data, textStatus) { 
			if (processType == 3) {
				log("get data finish");
			} else {
				hideLoadDialog();
			}

			moveFriendNavToTop(uid);
			nullStatusFriendNavWithUID(uid);
			if (processType == 0) {
				clearMessageToDialog();
			}
			
			var res_obj = eval("("+ data +")");
			if (res_obj.status != "succ") {
				alert(res_obj.des);
				return;
			}

			var msg_queue = res_obj.queue;
			var i = 0;
			while (i < msg_queue.length) {
				var msg = msg_queue[i];
				var sender = "me";
				var avatar = myData.icon;
				var msgType = "TXT";

				if (msg.fuid != myData.id) {
					sender = "other";
					avatar = friend.icon;
				}
				msgType = msg.queue_type; 
			
				//$.base64.utf8decode = false;
				//var message = $.base64.atob(msg.queue_content);
				if (msg.id > _maxQID) 
					_maxQID = msg.id;
				if (msg.id < _minQID)
					_minQID = msg.id;

				addMessageToDialog(sender, avatar, msg.queue_content);
				chatScrollLast();

				i++;
			}
		},
		complete: function(XMLHttpRequest, textStatus) { 
			hideLoadDialog();
		}

	});
}


function addMessageToDialog(sender, avatar, message) {
	var chatDialog = $('#chatDialog')[0];

	var html = '';
	if (sender == "me") {
		html += '<div class="chat-msg">';
      	html += '<img class="chat-msg__pic-right" src="'+ avatar +'">';
      	html += '<p class="chat-msg__text-right">'+ message +'</p>';
    	html += '</div>';

	} else {
		html += '<div class="chat-msg">';
		html += '<img class="chat-msg__pic" src="'+ avatar +'">';
		html += '<p class="chat-msg__text">'+ message +'</p>';
		html += '</div>';

	}

	chatDialog.innerHTML += html;
}

function clearMessageToDialog() {
	var chatDialog = $('#chatDialog')[0];
	chatDialog.innerHTML = '';
}


function sendMessage()
{
	var inputT = $('#inputTextarea')[0];
	if (inputT.value.length > 0) {
		addMessageToDialog("me", myData.icon, inputT.value);
		chatScrollLast();

		var qtype = "TXT";	// "SCMTXT"
		var msg = get_sendtxt_cmd(qtype, myData.id, myData.nickname, talkingFriend.uid, talkingFriend.ios_token, inputT.value);
		wsSend(msg);
	}
	inputT.value = '';
}

function isChattingStayFUID(fuid) {
	if ((talkingFriend !== undefined) && talkingFriend.uid == fuid) {
		return 1;
	}
	return 0;
}


function chatScrollLast()
{
	var chatDialog = $('#chatDialog')[0];
	chatDialog.scrollTop = chatDialog.scrollHeight;
}


