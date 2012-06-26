#!/usr/bin/python
import sys, os, re, time, shutil, jpsy, multiprocessing
################################################################################
################################################################################
class Params:
   verbose = True
   parjobs = True
   envname = 'SCOT_HOME_DIR'
   def __init__( self ):
      
      if not self.envname in os.environ:
         print 'Before using ' + sys.argv[0] + ', you need to set the environment variable "' + self.envname + '"'
         print 'to point to the home directory for this project.'
         sys.exit( -1 )
      
      self.scotHome   = os.environ[ self.envname ]
      self.ciroptSrc  = os.path.join( self.scotHome, 'ciropt', 'src' )
      self.diogenSrc  = os.path.join( self.scotHome, 'diogen', 'src' )
      self.ggpsolSrc  = os.path.join( self.scotHome, 'ggpsolexp', 'src' )
      self.irsimSrc   = os.path.join( self.scotHome, 'irsim', 'src', 'irsim' )
      
      self.buildHomes = []
      self.buildHomes.append( self.diogenSrc )
      self.buildHomes.append( self.ciroptSrc )
      self.buildHomes.append( self.ggpsolSrc )
      self.buildHomes.append( self.irsimSrc )
      
      self.numCPUs = multiprocessing.cpu_count()
   

################################################################################
################################################################################
params = Params()
args = ' '.join( sys.argv[ 1: ] )
make = ' '.join( [ 'make', '-j', str( params.numCPUs), args ] )

for buildHome in params.buildHomes:
   os.chdir( buildHome )
   jpsy.SystemWrapper( make, verbose = params.verbose, trial = False )

