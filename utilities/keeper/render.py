import af

Renders = []

def refresh():
   global Render
   Renders = af.Cmd().renderGetLocal()
   print( Renders)

