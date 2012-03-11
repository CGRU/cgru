import hou
import soho

import afanasy

current_afnode = hou.pwd()

submit_afnode_parm = current_afnode.parm('submit_afnode')
if submit_afnode_parm is None: soho.error('Can\'t find "submit_afnode" parameter on "%s"' % current_afnode.path())

submit_afnode_path = submit_afnode_parm.eval()
if submit_afnode_path is None: soho.error('Can\'t eval "submit_afnode" parameter on "%s"' % current_afnode.path())
if submit_afnode_path == '': soho.error('Empty "submit_afnode" parameter on "%s"' % current_afnode.path())

submit_afnode = hou.node( submit_afnode_path)
if submit_afnode is None: soho.error('Can\'t find "%s" node specified in "%s"' % (submit_afnode_path, current_afnode.path()))
if submit_afnode.type().name() != 'afanasy': soho.error('Node "%s" specified in "%s" is not of "afanasy" type' % (submit_afnode_path, current_afnode.path()))

afanasy.render( submit_afnode)
