# -*- coding: utf-8 -*-
import services

service = []
for srv in services.__all__:
   __import__("services", globals(), locals(), [srv])
   service.append( eval(('services.%(srv)s.%(srv)s()') % vars()))
