import rulib

def getStatusFilePath(i_path = None):
    return rulib.functions.getRuFilePath('status.json', i_path)

def getStatus(i_path = None):
    return rulib.functions.readObj(getStatusFilePath(i_path))

def setStatuses(i_args, i_paths):
    pass
