#!/usr/bin/python
import sys, os, re, time, shutil, jpsy
################################################################################
################################################################################
class Params:
   verbose = True
   cleanup = True
   trial   = False
   def __init__( self ):
      
      envscot = 'SCOT_HOME_DIR'
      
      if not envscot in os.environ:
         ( progpath, progname ) = os.path.split( sys.argv[0] )
         print 'Before using ' + progname + ', you need to set the environment variable "' + self.envscot + '"'
         print 'to point to the home directory for this project and to a suitable place for temporary files.'
         sys.exit( -1 )
      
      self.homeDir = os.environ[ envscot ]
      
      self.tmpHome = os.path.join( self.homeDir, 'tmp'  )
      self.tarHome = os.path.join( self.tmpHome, 'scot' )
   

################################################################################
################################################################################
def CopyRepoToTmpPath( params ):
   cmd   = 'hg status --all'
   lines = jpsy.SystemWrapperPipelined( cmd, params.verbose, trial = params.trial )
   
   for line in lines:
      toks = line.split()
      if toks[0] in [ 'A', 'M', '?' ]:
         print line
         print '-----------------------------'
         print 'can only tarball a clean repo'
         print '-----------------------------'
         assert False
      assert toks[0] in [ 'C', 'I' ]
      if toks[0] == 'C':
         src = os.path.join( params.homeDir, toks[1] )
         dst = os.path.join( params.tarHome, toks[1] )
         ( dstp, dstf ) = os.path.split( dst )
         if not os.path.isdir( dstp ):
            os.makedirs( dstp )
         assert os.path.isdir( dstp )
         shutil.copy( src, dst )

def MakeTarBall( params, version ):
   os.chdir( params.tmpHome )
   cmd = 'tar cvfj scot-%s.tar.bz2 dagmtx' % version
   SystemWrapper( cmd, params.verbose, trial = params.trial )

def Cleanup( params ):
   if params.cleanup:
      shutil.rmtree( params.tarHome )

################################################################################
################################################################################
if len( sys.argv ) < 2:
   ( progPath, progName ) = os.path.split( sys.argv[0] )
   print 'Usage:', progName, '<versionNumber>'
   sys.exit( -1 )

version = sys.argv[1]
params  = Params()

CopyRepoToTmpPath( params )
MakeTarBall( params, version )
Cleanup( params )
