import os

cgru_scripts_path = os.environ['NUKE_CGRU_PATH'] + '/scripts'
cgru_gizmos_path  = os.environ['NUKE_CGRU_PATH'] + '/gizmos'

sys.path.append( cgru_scripts_path)

nuke.pluginAddPath( cgru_scripts_path)
nuke.pluginAddPath( cgru_gizmos_path )
