class Session:

    def __init__(self, i_environ):

        self.USER_ID = None
        self.GROUPS = None
        self.REMOTE_ADDR = None

        if 'REMOTE_ADDR' in i_environ:
            self.REMOTE_ADDR = i_environ['REMOTE_ADDR']
        if 'REMOTE_USER' in i_environ:
            self.USER_ID = i_environ['REMOTE_USER']

