# -*- coding: utf-8 -*-
import services
import parsers

service = []
for srv in services.__all__:
   __import__("services", globals(), locals(), [srv])
   service.append( eval(('services.%(srv)s.%(srv)s()') % vars()))

parser = []
for prs in parsers.__all__:
   __import__("parsers", globals(), locals(), [prs])
   parser.append( eval(('parsers.%(prs)s.%(prs)s()') % vars()))
