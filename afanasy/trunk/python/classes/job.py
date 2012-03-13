class job:
   def __init__( self, name, username):
      self.name = name
      print 'Job "%s" class instanced.' % self.name

   def addblock( self, name, service, tasksnum):
      print 'Job addblock: "%s"' % self.name
      print 'Name = "%(name)s", service = "%(service)s", tasks number = %(tasksnum)d' % vars()

   def construct( self):
      print 'Job "%s" class constructed.' % self.name
      return True

   def destroy( self):
      print 'Job "%s" class destoyed.' % self.name
