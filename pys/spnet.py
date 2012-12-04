#!/usr/bin/python
import os, sys, re, argparse, hspy, jpsy
################################################################################
################################################################################
class Params:
   verbose = True
   lowercase = True
   localbind = False
   envname = 'SCOT_HOME_DIR'
   joinch  = '__'
   def __init__( self ):
      
      if not self.envname in os.environ:
         print 'Before using ' + sys.argv[0] + ', you need to set the environment variable "' + self.envname + '"'
         print 'to point to the home directory for this project.'
         sys.exit( -1 )
      
      self.homeDir = os.environ[ self.envname ]
   

class Capacitor:
   def ToStr( self, connxns, extparams, netpfx = '' ):
      # assume the connected nodes are local nets:
      name  = self.gparams.joinch.join( [ netpfx, self.name  ] )
      node0 = self.gparams.joinch.join( [ netpfx, self.node0 ] )
      node1 = self.gparams.joinch.join( [ netpfx, self.node1 ] )
      #
      # go ahead and use a connected net if it exists:
      if self.node0 in connxns: node0 = connxns[ self.node0 ]
      if self.node1 in connxns: node1 = connxns[ self.node1 ]
      #
      # put an 'c' on the uniquified name -- make it a capacitor :)
      name = 'c' + name
      #
      #
      # determine if capacitor value is passed in as a parameter
      farads = self.farads
      if farads in extparams:
         farads = extparams[ farads ]
      #
      # instantiate the new capacitor:
      toks = [ name, node0, node1, farads ]
      toks = [ '%-40s' % x for x in toks ]
      if self.gparams.lowercase:
         toks = [ x.lower() for x in toks ]
      line = ' '.join( toks )
      # -- TODO -- remove -- print '------------------------------------------------------------------------------------------------------------------------------'
      # -- TODO -- remove -- print 'connxns =', connxns
      # -- TODO -- remove -- print 'extparams =', extparams
      # -- TODO -- remove -- print 'netpfx =', netpfx
      # -- TODO -- remove -- print 'name =', name
      # -- TODO -- remove -- print 'line =', line
      # -- TODO -- remove -- print '------------------------------------------------------------------------------------------------------------------------------'
      # -- TODO -- remove -- assert False
      return line
   
   def Print( self, connxns, extparams, netpfx ):
      line = self.ToStr( connxns, extparams, netpfx )
      print line
   
   def __init__( self, cdef, gparams ):
      self.gparams = params
      
      # get the parameters (width, length, mfactor, etc...)
      ( cdef, self.params ) = hspy.ExtractParameters( cdef )
            
      # now get nmos/pmos, name, and the s,g,d, and b ports
      toks = cdef.split()
      self.farads = toks.pop()
      self.node0  = toks.pop()
      self.node1  = toks.pop()
      self.name   = toks.pop()
      assert len( toks ) == 0
      # -- TODO -- any sanity checks for Capacitor in spnet.py ??
   

class DotCom:
   # spice deck line that
   # ... begins with a dot
   # ... or is a comment
   def ToStr( self, connxns, extparams, netpfx = None ):
      return self.line
   
   def Print( self, connxns, extparams, netpfx = None ):
      line = self.ToStr( connxns, extparams, netpfx )
      print line
    
   def __init__( self, line, gparams ):
      self.gparams = params
      self.type = 'dotcom'
      self.line = line
   

class Mosfet:
   def ToStr( self, connxns, extparams, netpfx = '' ):
      # assume s,g,d, and b are local nets:
      name = self.gparams.joinch.join( [ netpfx, self.name ] )
      b    = self.gparams.joinch.join( [ netpfx, self.b    ] )
      d    = self.gparams.joinch.join( [ netpfx, self.d    ] )
      g    = self.gparams.joinch.join( [ netpfx, self.g    ] )
      s    = self.gparams.joinch.join( [ netpfx, self.s    ] )
      #
      # go ahead and use a connected net if it exists:
      if self.b in connxns: b = connxns[ self.b ]
      if self.d in connxns: d = connxns[ self.d ]
      if self.g in connxns: g = connxns[ self.g ]
      if self.s in connxns: s = connxns[ self.s ]
      #
      # put an 'm' on the uniquified name -- make it a mosfet :)
      name = 'm' + name
      #
      # put the parameters in:
      boundparams = []
      for key, val in self.params.iteritems():
         if val in extparams:
            val = extparams[ val ]
         bp = '='.join( [ key, val ] )
         boundparams.append( bp )
      boundparams = ' '.join( boundparams )
      #
      # instantiate the new mosfet:
      toks = [ name, s, g, d, b, self.type, boundparams ]
      toks = [ '%-20s' % x for x in toks ]
      if self.gparams.lowercase:
         toks = [ x.lower() for x in toks ]
      line = ' '.join( toks )
      return line
   
   def Print( self, connxns, extparams, netpfx = '' ):
      line = self.ToStr( connxns, extparams, netpfx )
      print line
   
   def __init__( self, mdef, gparams ):
      self.gparams = params
      
      # get the parameters (width, length, mfactor, etc...)
      ( mdef, self.params ) = hspy.ExtractParameters( mdef )
      
      # now get nmos/pmos, name, and the s,g,d, and b ports
      toks = mdef.split()
      self.type = toks.pop()
      self.b    = toks.pop()
      self.d    = toks.pop()
      self.g    = toks.pop()
      self.s    = toks.pop()
      self.name = toks.pop()
      assert len( toks ) == 0
      # -- TODO -- can do more sanity checks on the back connection
      assert self.type.lower() in [ 'nmos', 'pmos' ]
      assert self.b.lower()    in [ 'vdd',  'gnd'  ]
   

class SCInst:
   # an instantiated subckt
   def Bind(  self, connxns, extparams, netpfx = None ):
      #
      # lnets: local net names, add prefix if not defined as an external connection
      lnets = []
      for p in self.ports:
         if p in connxns:
            lnets.append( connxns[ p ] )
            continue
         if netpfx is not None:
            p = self.gparams.joinch.join( [ netpfx, p ] )
         lnets.append( p )
      #
      # continue (or bootstrap) the net prefix (to uniquify net names)
      if netpfx is None: netpfx = self.iname
      else:              netpfx = self.gparams.joinch.join( [ netpfx, self.iname ] )
      #
      # grab the subcircuit of which we are an instance
      subckt  = self.gparams.subckts[ self.sname ]
      #
      # ptuples: packs the local subckt port names with the nets given on the line where this instance was declared
      # newconn: the same as ptuples, but as a dictionary mapping { port name => net }
      # connxns: merge of the local and external port to net map (deep copy) (local connections take precedence)
      # lparams: contains the default parameters from the line where this instance was declared
      ptuples = zip( subckt.ports, lnets )
      newconn = dict( ptuples )
      connxns = dict( connxns.items() + newconn.items() )
      # connxns = dict( newconn.items() + connxns.items() )
      lparams = dict( subckt.params.items() )
      #
      # parameters specified on the line where this subckt was instantiated:
      for k, v in self.params.iteritems():
         try:
            assert k in lparams
         except:
            print ''
            print 'k =', k
            print 'v =', v
            print 'lparams =', lparams
            print 'self.sname =', self.sname
            print 'self.iname =', self.iname
            print ''
            raise
         lparams[ k ] = v
      #
      # parameters passed in from a parent
      # (optionally, take the local parameters only)
      if not params.localbind:
         for k, v in lparams.iteritems():
            if v in extparams:
               lparams[ k ] = extparams[ v ]
      #
      return ( subckt, connxns, lparams, netpfx )
   
   def Print( self, connxns, extparams, netpfx = None ):
      ( subckt, connxns, lparams, netpfx ) = self.Bind( connxns, extparams, netpfx )
      #
      # tell the subcircuit to print its elements
      subckt.Print( connxns, lparams, netpfx )
   
   def ToStr( self, connxns, extparams, netpfx = None ):
      ( subckt, connxns, lparams, netpfx ) = self.Bind( connxns, extparams, netpfx )
      #
      # tell the subcircuit to print its elements
      return subckt.ToStr( connxns, lparams, netpfx )
   
   def __init__( self, idef, gparams ):
      self.gparams = gparams
      
      # get the parameters
      ( idef, self.params ) = hspy.ExtractParameters( idef )
      
      # now get the instance name, ports, and subckt name
      toks = idef.split()
      self.type  = 'scinst'
      self.sname = toks.pop()
      self.iname = toks.pop(0)
      self.ports = toks
   

class Subckt:
   def Print( self, connxns, extparams, netpfx = None ):
      #
      # print each element in this subcircuit
      # ... SCInst::Print()
      # ... Mosfet::Print()
      # ... DotCom::Print()
      for e in self.elems:
         e.Print( connxns, extparams, netpfx )
   
   def ToStr( self, connxns, extparams, netpfx = None ):
      #
      # make a string including each element in this subcircuit
      # ... SCInst::ToStr()
      # ... Mosfet::ToStr()
      # ... DotCom::ToStr()
      lines = []
      for e in self.elems:
         lines.append( e.ToStr( connxns, extparams, netpfx ) )
      return '\n'.join( lines )
   
   def ParseSubCktDef( self, line0 ):
      
      # pull the parameters out
      ( scdef, self.params ) = hspy.ExtractParameters( line0 )
      
      # tokenize
      toks = scdef.split()
      
      # sanity, and remove the first token
      # then get the subckt name
      assert toks.pop(0).lower() == '.subckt'
      self.name  = toks.pop(0)
      self.ports = toks
   
   def __init__( self, lines, gparams ):
      self.gparams = gparams
      
      # setup
      self.ports  = []
      self.elems  = []
      self.params = {}
      
      self.ParseSubCktDef( lines.pop(0) )
      
      for line in lines:
         if re.match( '\.ends', line, re.I ):
            break
         if   line[0].lower() == 'm':
            m = Mosfet( line, gparams )
            self.elems.append( m )
         elif line[0].lower() == 'x':
            x = SCInst( line, gparams )
            self.elems.append( x )
         elif line[0].lower() == 'c':
            x = Capacitor( line, gparams )
            self.elems.append( x )
         elif line[0] == '*':
            continue
         elif line[0] == '.':
            continue
         else:
            print ''
            print 'unanticipated line:'
            print line
            print ''
            print 'need to add a new circuit element to spnet.py'
            print 'should be easy'
            print ''
            assert False
   

################################################################################
################################################################################
def Sanitize( inpLines ):
   outLines = []
   for line in inpLines:
      
      # remove whitespace
      line = line.rstrip().lstrip()
      
      # skip empty lines
      if line == '': continue
      
      # join continuation lines
      if line[0] == '+':
         # -- TODO -- verify replace semantics: pattern, repl, max-num-to-replace
         line = line.replace( '+', ' ', 1 )
         outLines[ -1 ] += line
         continue
      
      # base case: just put the line into the sanitized list
      outLines.append( line )
   return outLines

################################################################################
################################################################################
def ReadInSubckts( inpLines, params ):
   subckts  = {}
   outLines = []
   insubckt = False
   for line in inpLines:
      if not insubckt:
         # not in a subckt:
         # ... detect the beginning of a subckt
         # ... or just add the line to the main ckt
         if re.match( '\.subckt', line, re.I ):
            insubckt = True
            subLines = []
            subLines.append( line )
         else:
            outLines.append( line )
      else:
         # in a subckt:
         # ... normally (else condition) keep collecting subckt lines
         # ... detect the end of the subckt, parse it in, add it to the dictionary
         if re.match( '\.ends', line, re.I ):
            insubckt = False
            subLines.append( line )
            subckt = Subckt( subLines, params )
            subckts[ subckt.name ] = subckt
         else:
            subLines.append( line )
   return ( subckts, outLines )

def ReadInMainCkt( inpLines, params ):
   mainckt  = []
   outLines = []
   for line in lines:
      if line[0] in [ '.', '*' ]:
         c = DotCom( line, params )
         mainckt.append( c )
         outLines.append( line )
      elif line[0].lower() == 'm':
         m = Mosfet( line, params )
         mainckt.append( m )
      elif line[0].lower() == 'x':
         x = SCInst( line, params )
         mainckt.append( x )
      else:
         print ''
         print 'unanticipated input:'
         print line
         print ''
         assert False
   return ( mainckt, outLines )

################################################################################
################################################################################
params = Params()
parser = argparse.ArgumentParser( description = 'The Stanford Circuit Optimization Tool (SCOT): fasths2flat.py' )

parser.add_argument( '-i', '--inpfilename', nargs = 1 ) # -- TODO -- make this a required argument
parser.add_argument( '-o', '--outfilename', nargs = 1 ) # -- TODO -- make this a required argument

args = parser.parse_args()

assert args.inpfilename is not None
assert args.outfilename is not None

ifn = args.inpfilename[0]
ofn = args.outfilename[0]

lines = jpsy.ReadFileLines( ifn )

lines = Sanitize( lines )

( subckts, lines ) = ReadInSubckts( lines, params )
( mainckt, lines ) = ReadInMainCkt( lines, params )

# put the subckt dictionary into the global parameters object
# all the other elements already hold a reference to the global parameters
params.subckts = subckts

connxns = {}
connxns[ 'vdd' ] = 'vdd'
connxns[ 'gnd' ] = 'gnd'

lines = []
for elem in mainckt:
   lines.append( elem.ToStr( connxns, {} ) )

jpsy.WriteLinesToFile( lines, ofn )

