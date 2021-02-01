# -*- coding: utf-8 -*-

from parsers import maya, redshift


class maya_redshift(maya.maya, redshift.redshift):
    """Maya Redshift parser
    """

    def __init__(self):
        maya.maya.__init__(self)
        redshift.redshift.__init__(self)
