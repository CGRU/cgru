function forum_InitTopic()
{
	$('asset_label').textContent = 'Ask New Question'
	var data = n_Request({"getfile":'rules/forum.html'});
	$('asset').innerHTML = data;

	u_GuestAttrsDraw( $('forum_guest_form'));

	if( g_auth_user == null ) $('forum_guest_form').style.display = 'block';
}

function forum_NewQuestionOnClick()
{
	var question = {};
	question.id = c_Strip( $('forum_question_id').textContent);
	question.body = c_Strip( $('forum_question_body').textContent);
	if( question.id.length == 0 )
	{
		c_Error('Required question ID attribute is empty.');
		return;
	}
	if( question.body.length == 0 )
	{
		c_Error('Required question body attribute is empty.');
		return;
	}

	question.guest = u_GuestAttrsGet( $('forum_guest_form'));
console.log( JSON.stringify( question));
}

if( ASSETS.topic ) forum_InitTopic();

