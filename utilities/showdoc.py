#!/usr/bin/env python

import os
import sys

cgru_location_Var = 'CGRU_LOCATION'

cgru_location = os.getenv( cgru_location_Var)
if cgru_location is None or cgru_location == '':
   cgru_location = os.path.abspath( sys.argv[0])
   cgru_location = os.path.dirname( cgru_location)
   cgru_location = os.path.dirname( cgru_location)
   os.putenv( cgru_location_Var, cgru_location)
   os.environ[ cgru_location_Var] = cgru_location

cgru_python = os.getenv('CGRU_PYTHON')
if cgru_python is None or cgru_python == '':
   sys.path.append( os.path.join( cgru_location, 'lib/python'))

import cgrudocs

path = None
if len(sys.argv) > 1: path = sys.argv[1]

cgrudocs.show( path)
