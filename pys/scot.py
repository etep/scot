#!/usr/bin/python
import sys, os, re, time, shutil, jpsy
################################################################################
################################################################################
class Params:
   verbose = True
   numruns = 100
   envname = 'SCOT_HOME_DIR'
   def __init__( self ):
      
      if not self.envname in os.environ:
         print 'Before using ' + sys.argv[0] + ', you need to set the environment variable "' + self.envname + '"'
         print 'to point to the home directory for this project.'
         sys.exit( -1 )
      
      self.scotHome = os.environ[ self.envname ]
      self.pysHome  = os.path.join( self.scotHome, 'pys'  )
      self.tekHome  = os.path.join( self.scotHome, 'tech' )
      self.txtHome  = os.path.join( self.scotHome, 'txt'  )
      
      self.optpy    = os.path.join( self.pysHome, 'optim.py' )
      self.glbfile  = os.path.join( self.tekHome, 'glb90n_par.sp' )
      self.datfile  = os.path.join( self.tekHome, 'Tech90.dat'  )
      self.prmfile  = os.path.join( self.tekHome, 'scmos90.prm' )
      self.cmdfile  = os.path.join( self.txtHome, 'optcmd.txt'  )
   

################################################################################
################################################################################
def SwapFext( filename, fext ):
   toks = filename.split( '.' )
   toks.pop()
   toks.append( fext )
   return '.'.join( toks )

################################################################################
################################################################################
params = Params()

hspfile = sys.argv[1]
sspfile = SwapFext( hspfile, 'ssp' )
diofile = SwapFext( hspfile, 'dio' )
assert os.path.isfile( hspfile )
( path, filename ) = os.path.split( hspfile )
if path != '': os.chdir( path )

hspCmd = ' '.join( [ params.optpy, '-hsp', hspfile, params.glbfile, sspfile ] )
irsCmd = ' '.join( [ params.optpy, '-irs', hspfile, sspfile, params.prmfile, str( params.numruns ) ] )
sspCmd = ' '.join( [ params.optpy, '-ssp', sspfile, params.datfile, diofile ] )
solCmd = ' '.join( [ params.optpy, '-sol', diofile, sspfile, params.cmdfile ] )

jpsy.SystemWrapper( hspCmd, verbose = params.verbose, trial = False )
jpsy.SystemWrapper( irsCmd, verbose = params.verbose, trial = False )
jpsy.SystemWrapper( sspCmd, verbose = params.verbose, trial = False )
jpsy.SystemWrapper( solCmd, verbose = params.verbose, trial = False )






