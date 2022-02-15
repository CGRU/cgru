import json

def sendJob(i_args, o_out):

    obj = dict()
    obj['job'] = i_args['job']

    try:
        afnetwork = __import__('afnetwork', globals(), locals(), [])
    except:
        o_out['error'] = 'Can`t import afnetwork module.'
        o_out['info'] = '%s' % traceback.format_exc()
        return

    status, data = afnetwork.sendServer(json.dumps(obj))

    o_out['response'] = data
    o_out['status'] = status

