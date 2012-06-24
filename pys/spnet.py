#!/usr/bin/python
import os, sys, re, argparse, jpsy
################################################################################
################################################################################
class Params:
   verbose = True
   lowcase = True
   envname = 'SCOT_HOME_DIR'
   joinch  = '_'
   def __init__( self ):
      
      if not self.envname in os.environ:
         print 'Before using ' + sys.argv[0] + ', you need to set the environment variable "' + self.envname + '"'
         print 'to point to the home directory for this project.'
         sys.exit( -1 )
      
      self.homeDir = os.environ[ self.envname ]
   

class DotCom:
   # spice deck line that
   # ... begins with a dot
   # ... or is a comment
   def Print( self, connxns, extparams, netpfx = None ):
      print self.line
      pass
   
   def __init__( self, line, gparams ):
      self.gparams = params
      self.type = 'dotcom'
      self.line = line
   

class Mosfet:
   def Print( self, connxns, extparams, netpfx = '' ):
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
      if self.gparams.lowcase:
         toks = [ x.lower() for x in toks ]
      line = ' '.join( toks )
      print line
   
   def __init__( self, mdef, gparams ):
      self.gparams = params
      
      # get the parameters (width, length, mfactor, etc...)
      ( mdef, self.params ) = ExtractParameters( mdef )
      
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
   def Print( self, connxns, extparams, netpfx = None ):
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
         assert k in lparams
         lparams[ k ] = v
      #
      # parameters passed in from a parent
      for k, v in lparams.iteritems():
         if v in extparams:
            lparams[ k ] = extparams[ v ]
      #
      # tell the subcircuit to print its elements
      subckt.Print( connxns, lparams, netpfx )
   
   def __init__( self, idef, gparams ):
      self.gparams = gparams
      
      # get the parameters
      ( idef, self.params ) = ExtractParameters( idef )
      
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
   
   def ParseSubCktDef( self, line0 ):
      
      # pull the parameters out
      ( scdef, self.params ) = ExtractParameters( line0 )
      
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
         elif line[0] == '*':
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
         line[0] = ' '
         outLines[ -1 ] += line
         continue
      
      # base case: just put the line into the sanitized list
      outLines.append( line )
   return outLines

################################################################################
################################################################################
def PackEqualsSigns( line ):
   # removes whitespace around equals signs
   # ... transforms: 'foo bar baz = zam'
   # ...       into: 'foo bar baz=zam'
   toks = line.split( '=' )
   toks = [ x.lstrip().rstrip() for x in toks ]
   return '='.join( toks )

def ExtractParameters( line ):
   params = {}
   line = PackEqualsSigns( line )
   toks = line.split()
   while len( toks ):
      tok = toks.pop()
      subtoks = tok.split( '=' )
      if len( subtoks ) == 2:
         k = subtoks[0]
         v = subtoks[1]
         params[ k ] = v
      else:
         toks.append( tok )
         break
   assert len( toks )
   line = ' '.join( toks )
   return ( line, params )

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

lines = jpsy.ReadFileLines( sys.argv[1] )

lines = Sanitize( lines )

( subckts, lines ) = ReadInSubckts( lines, params )
( mainckt, lines ) = ReadInMainCkt( lines, params )

# put the subckt dictionary into the global parameters object
# all the other elements already hold a reference to the global parameters
params.subckts = subckts

connxns = {}
connxns[ 'vdd' ] = 'vdd'
connxns[ 'gnd' ] = 'gnd'

for elem in mainckt:
   elem.Print( connxns, {} )

# print '\n'.join( lines )
