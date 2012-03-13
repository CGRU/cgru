import os

cgru_scripts_path = os.environ['NUKE_CGRU_PATH'] + '/scripts'

sys.path.append( cgru_scripts_path)
nuke.pluginAddPath( cgru_scripts_path)
