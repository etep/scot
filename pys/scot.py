#!/usr/bin/python
import sys, os, re, time, shutil, jpsy
################################################################################
################################################################################
class Params:
   verbose = True
   numruns = 100
   envname = 'SCOT_HOME_DIR'
   envtemp = 'SCOT_TEMP_DIR'
   def SetupTempPath( self ):
      theTime = time.localtime()
      yr   = '%04d' % ( theTime.tm_year )
      mon  = '%02d' % ( theTime.tm_mon  )
      day  = '%02d' % ( theTime.tm_mday )
      hr   = '%02d' % ( theTime.tm_hour )
      mt   = '%02d' % ( theTime.tm_min  )
      sec  = '%02d' % ( theTime.tm_sec  )
      date = '_'.join( [yr, mon, day] )
      tyme = '.'.join( [hr, mt,  sec] )
      path = '_'.join( [ 'scot', date, tyme ] )
      self.runpath = os.path.join( self.tempHome, path )
      os.makedirs( self.runpath )
      os.environ[ 'SCOT_TEMP_DIR_OPTIM_DOT_PY' ] = self.runpath
   
   def __init__( self ):
      
      if not ( self.envname in os.environ and self.envtemp in os.environ ):
         ( progpath, progname ) = os.path.split( sys.argv[0] )
         print 'Before using ' + progname + ', you need to set the environment variables "' + self.envname + '" and "' + self.envtemp + '"'
         print 'to point to the home directory for this project and to a suitable place for temporary files.'
         sys.exit( -1 )
      
      self.scotHome = os.environ[ self.envname ]
      self.tempHome = os.environ[ self.envtemp ]
      self.pysHome  = os.path.join( self.scotHome, 'pys'  )
      self.tekHome  = os.path.join( self.scotHome, 'tech' )
      self.txtHome  = os.path.join( self.scotHome, 'txt'  )
      
      self.optpy    = os.path.join( self.pysHome, 'optim.py' )
      self.glbfile  = os.path.join( self.tekHome, 'glb90n_par.sp' )
      self.datfile  = os.path.join( self.tekHome, 'Tech90.dat'  )
      self.prmfile  = os.path.join( self.tekHome, 'scmos90.prm' )
      self.cmdfile  = os.path.join( self.txtHome, 'optcmd.txt'  )
      
      self.SetupTempPath()
   

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
sspfile = os.path.join( params.runpath, sspfile )
diofile = os.path.join( params.runpath, diofile )

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
#jpsy.SystemWrapper( solCmd, verbose = params.verbose, trial = False )






