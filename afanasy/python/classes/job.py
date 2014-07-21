# -*- coding: utf-8 -*-


class job:  # TODO: class names should use CamelCase
    """Missing DocString

    :param name:
    :param username:
    """

    def __init__(self, name, username):
        self.name = name
        print('Job "%s" class instanced.' % self.name)

    def addblock(self, name, service, tasksnum):
        """Missing DocString

        :param name:
        :param service:
        :param tasksnum:
        :return:
        """
        print('Job addblock: "%s"' % self.name)
        print('Name = "%(name)s", service = "%(service)s", tasks number = '
              '%(tasksnum)d' % vars())

    def construct(self):
        """Missing DocString
        """
        print('Job "%s" class constructed.' % self.name)
        return True

    def destroy(self):
        """Missing DocString
        """
        print('Job "%s" class destoyed.' % self.name)
