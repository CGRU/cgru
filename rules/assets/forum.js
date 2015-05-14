forum_qid_max_len = 48;

function forum_Init()
{
	a_SetLabel('Ask New Question');
	n_Request({"send":{"getfile":'rules/assets/forum.html'},"func":forum_InitHTML,"info":'get forum.html',"parse":false});
}
function forum_InitHTML( i_data)
{
	$('asset').innerHTML = i_data;

	u_GuestAttrsDraw( $('forum_guest_form'));

	if( g_auth_user == null ) $('forum_guest_form').style.display = 'block';

	$('qid_max_len').textContent = forum_qid_max_len;
}

function forum_QuestionIDOnBlur( i_e )
{
	forum_QuestionIDGet();
}

function forum_QuestionIDGet()
{
	var id = $('forum_question_id').textContent;
	id = c_Strip( id).replace(/\W/g,'_').substr(0,forum_qid_max_len);
	$('forum_question_id').textContent = id;
	n_WalkDir({"paths":[g_CurPath()],"wfunc":forum_QuestionIDReceived,"question_id":id});
}
function forum_QuestionIDReceived( i_data, i_args)
{
	var id = i_args.question_id;
	var walks = i_data;

	if( walks && walks.length && walks[0].folders && walks[0].folders.length )
		for( var i = 0; i < walks[0].folders.length; i++)
			if( walks[0].folders[i].name == id )
			{
				c_Error('Question "' + id + '" already exists. Try other.');
				$('forum_question_id').classList.add('error');
				return null;
			}
	$('forum_question_id').classList.remove('error');
	c_Info('Question ID: ' + id);
}

function forum_NewQuestionOnClick()
{
	var question = {};

	if($('forum_question_id').classList.contains('error')) return;

	question.id = $('forum_question_id').textContent;
	if( question.id == null ) return;
	if( question.id.length == 0 )
	{
		c_Error('Required question ID attribute is empty.');
		$('forum_question_id').classList.add('error');
		return;
	}

	question.body = c_Strip( $('forum_question_body').innerHTML);
	if( question.body.length == 0 )
	{
		c_Error('Question body is empty.');
		return;
	}

	var user_id = null;
	if( g_auth_user == null )
	{
		question.guest = u_GuestAttrsGet( $('forum_guest_form'));
		if( question.guest == null )
			return;
		user_id = question.guest.id;
	}
	else
		user_id = g_auth_user.id;

//console.log( JSON.stringify( question));

	var folder = ASSETS.topic.path + '/' + question.id;
	var path = RULES.root + '/' + folder  + '/' + RULES.rufolder + '/body.html';

	question.user_id = user_id;	
	question.folder = folder;

	n_Request({"send":{"save":{"file":path,"data":btoa(question.body),"type":"base64"}},"func":forum_BodySaved,"question":question});
}
function forum_BodySaved( i_data, i_args)
{
	if( i_data.error )
	{
		c_Error( i_data.error);
		return;
	}

	var question = i_args.question;

	var status = {};
	status.body = {};
	status.body.cuser = question.user_id;
	status.body.ctime = c_DT_CurSeconds();
	if( question.guest ) status.body.guest = question.guest;
	st_Save( status, question.folder, forum_StatusSaved, question);
}
function forum_StatusSaved( i_data, i_args)
{
	if( i_data.error )
	{
		c_Error( i_data.error);
		return;
	}

	var question = i_args.args;

	nw_MakeNews({"title":'question',"path":question.folder,"user":question.user_id,"guest":question.guest});
	g_GO( question.folder);
}

if( ASSETS.topic )
{
	if( ASSETS.topic.path == g_CurPath() ) forum_Init();
	else $('asset_div').style.display = 'none';
}

