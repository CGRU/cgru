import os

af_scripts_path = os.environ['NUKE_AF_PATH'] + '/scripts'
af_gizmos_path  = os.environ['NUKE_AF_PATH'] + '/gizmos'

sys.path.append( af_scripts_path)
#sys.path.append( af_gizmos_path)
nuke.pluginAddPath( af_scripts_path)
nuke.pluginAddPath( af_gizmos_path)
