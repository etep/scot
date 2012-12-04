#!/usr/bin/python
from pyparsing import Literal, CaselessLiteral, Word, Combine, Group, LineEnd
from pyparsing import oneOf, restOfLine, alphas, nums, delimitedList, cppStyleComment
from pyparsing import Keyword, Optional, ZeroOrMore, OneOrMore, Suppress, FollowedBy, MatchFirst
from pyparsing import ParseException, ParseElementEnhance
import sys, os, pyparsing
################################################################################
################################################################################
def ReadFileLines( fn = None ):
    # opens a file
    # returns the contents as a list of lines in the file
    # trailing new-line chars are removed
    if fn is None:
        print 'Usage: ReadFileLines( fileName )'
        print 'returns the file contents as a list of lines with newline chars stripped'
        assert False
    FH = open( fn, "r" )
    lines = [ line.rstrip() for line in FH.readlines() ]
    FH.close()
    return lines

def JoinContinuedLines( ilines ):
    # scan for lines that end with the continuation character '\'
    # join them together and drop the backslash
    olines = []
    xline  = ''
    for line in ilines:
        if line == '': continue
        if line[ -1 ] == '\\':
            xline = xline + ' ' + line[ :-1 ]
        else:
            xline = xline + ' ' + line
            xline = xline.lstrip().rstrip()
            # toks  = xline.split()
            # # to make statements w/out parens unambiguous,
            # # reverse the tokens' order (grrr...)
            # toks.reverse()
            # xline = ' '.join( toks )
            olines.append( xline )
            xline = ''
    return olines

################################################################################
################################################################################
def PfxStr( n ):
    pfxtoks = n * [ '...' ]
    pfx = ' '.join( pfxtoks )
    return pfx

def GetINamePfx( mname ):
    if mname == 'vsource': return 'v'
    if mname == 'P_TRANSISTOR': return 'm'
    if mname == 'N_TRANSISTOR': return 'm'
    return 'x'

def MName2Hsp( mname ):
    if mname == 'vsource':      return ''
    if mname == 'P_TRANSISTOR': return 'pmos'
    if mname == 'N_TRANSISTOR': return 'nmos'
    return mname

def MosfetParamKeyConvert( k ):
    if k == 'width':  return 'w'
    if k == 'length': return 'l'
    return k

################################################################################
################################################################################
class ParameterList:
    def hspstring( self, inamePfx = 'x' ):
        if self.none: return ''
        toks = []
        for k, v in self.kvps.iteritems():
            vstr = ''.join( v )
            if inamePfx == 'm':
                k = MosfetParamKeyConvert( k )
            s = '='.join([ k, vstr ])
            toks.append( s )
        return ' '.join( toks )
    
    def pprint( self, npfx = 0 ):
        pfx = PfxStr( npfx )
        print pfx, 'parameter list:'
        pfx = PfxStr( npfx + 1 )
        for k, v in self.kvps.iteritems():
            print pfx, k, '=>', v
    
    def AddParameter( self, tok ):
        v = tok.pop()
        k = tok.pop()
        assert len( tok ) == 0
        self.kvps[ k ] = v
    
    def __init__( self, toks ):
        self.kvps = {}
        self.none = True
        for t in toks:
            self.AddParameter( t )
        if len( self.kvps ):
            self.none = False
    

class Comment:
    def pprint( self, npfx = 0 ):
        pfx = PfxStr( npfx )
        print pfx, 'comment =', self.text
    
    def hspstring( self ):
        return ' '.join([ '*', self.text ])
    
    def hspprint( self ):
        print self.hspstring()
    
    def __init__( self, text ):
        self.text = text
    

class InstantiationBase:
    def hspstring( self ):
        inamePfx = GetINamePfx( self.mname )
        mname = MName2Hsp( self.mname )
        iname = inamePfx + self.iname
        plist = self.params.hspstring( inamePfx )
        clist = ' '.join( self.connections )
        s = ' '.join([ iname, clist, mname, plist ])
        return s
    
    def hspprint( self ):
        print self.hspstring()
    
    def pprint( self, npfx = 0 ):
        pfx = PfxStr( npfx )
        print pfx, 'iname =', self.iname, ', mname =', self.mname, ', connections =', self.connections
        if not self.params.none:
            self.params.pprint( npfx + 1 )
    
    def __init__( self ):
        self.connections = []
        self.mname = None
        self.iname = None
        self.params = None
    

class Instantiation( InstantiationBase ):
    def __init__( self, toks ):
        # super( Instantiation, self ).__init__()
        # iname: instance name
        # mname: model name (name of subckt or device model)
        InstantiationBase.__init__( self )
        self.params = ParameterList( toks.pop() )
        self.mname  = toks.pop()
        cnames = toks.pop()
        for c in cnames:
            self.connections.append( c )
        self.iname = toks.pop()
        # print 'i, iname =', self.iname, ', connections =', self.connections, ', mname =', self.mname, ', toks =', toks
        
    

class InstantiationNoParens( InstantiationBase ):
    def ConsumeParams( self, toks ):
        while True:
            tok = toks.pop()
            if type( tok ) != type( 'string' ):
                self.params.AddParameter( tok )
            else:
                return tok
                assert False
        assert False
    
    def ConsumeConnxns( self, toks ):
        while len( toks ) > 1:
            c = toks.pop()
            self.connections.append( c )
        self.connections.reverse()
        return toks.pop()
    
    def __init__( self, toks ):
        InstantiationBase.__init__( self )
        self.params = ParameterList( [] )
        self.mname = self.ConsumeParams( toks )
        self.iname = self.ConsumeConnxns( toks )
    

class SubCkt:
    def hspdecl( self ):
        toks = []
        pstr = self.parameters.hspstring()
        toks.append( '.subckt' )
        toks.append( self.name )
        for p in self.portnames:
            toks.append( p )
        toks.append( pstr )
        return ' '.join( toks )
    
    def hspstring( self ):
        lines = []
        lines.append( self.hspdecl() )
        for item in self.items:
            s = item.hspstring()
            lines.append( s )
        lines.append( '.ends $ %s' % self.name )
        lines.append( '' )
        return '\n'.join( lines )
    
    def hspprint( self ):
        print self.hspstring()
    
    def pprint( self, npfx = 0 ):
        pfx = PfxStr( npfx )
        print pfx, 'subckt:', self.name
        npfx += 1
        pfx = PfxStr( npfx )
        print pfx, 'portnames =', self.portnames
        for i in self.instantiations:
            i.pprint( npfx + 1 )
    
    def AddInstantiation( self, toks ):
        i = Instantiation( toks )
        self.instantiations.append( i )
        self.items.append( i )
    
    def AddComment( self, text ):
        c = Comment( text )
        self.comments.append( c )
        self.items.append( c )
    
    def SetParameterList( self, plist ):
        self.parameters = plist
    
    def __init__( self, toks ):
        pnames = toks.pop()
        self.parameters = ParameterList( [] )
        self.portnames  = []
        self.comments   = []
        self.items      = []
        self.instantiations = []
        for p in pnames:
            self.portnames.append( p )
        self.name = toks.pop()
    

################################################################################
################################################################################
class SpectreNetlistParser:
    def SubCktNames( self ):
        scnames = []
        for s in self.subckts:
            scnames.append( s.name )
        return scnames
    
    def BeginSubckt( self, s, l, t ):
        sc = SubCkt( t[0] )
        self.subckts.append( sc )
    
    def RecordSubckt( self, s, l, t ):
        self.items.append( self.subckts[ -1 ] )
    
    def RecordSubcktParams( self, s, l, t ):
        plist = ParameterList( t[0] )
        self.subckts[ -1 ].SetParameterList( plist )
    
    def RecordSubcktInstantiation( self, s, l, t ):
        self.subckts[ -1 ].AddInstantiation( t[0] )
    
    def RecordSubcktComment( self, s, l, t ):
        self.subckts[ -1 ].AddComment( t[0] )
    
    def RecordNetlistInstantiation( self, s, l, t ):
        i = Instantiation( t[0] )
        self.netlist.append( i )
        self.items.append( i )
    
    def RecordNetlistInstantiationNoParens( self, s, l, t ):
        i = InstantiationNoParens( t[0] )
        self.netlist.append( i )
        self.items.append( i )
    
    def RecordNetlistComment( self, s, l, t ):
        c = Comment( t[0] )
        self.comments.append( c )
        self.items.append( c )
    
    def SetWhitespaceChars( self ):
        whitespacechars = ''.join([ x for x in ParseElementEnhance.DEFAULT_WHITE_CHARS if x != '\n' ])
        ParseElementEnhance.setDefaultWhitespaceChars( whitespacechars )
    
    def Integer( self ):
        # numerical values
        # see http://pyparsing.wikispaces.com/file/view/SimpleCalc.py
        plus  = Literal( '+' )
        minus = Literal( '-' )
        plusorminus = plus | minus
        number  = Word( nums )
        integer = Combine( Optional( plusorminus ) + number )
        return integer
    
    def FloatNum( self, integer ):
        number = Word( nums ) 
        point  = Literal('.')
        e = CaselessLiteral('E')
        floatnumbera = Combine( integer + Optional( point + Optional( number ) ) + Optional( e + integer ) )
        floatnumberb = Combine( integer + Optional( point + Optional( number ) ) + oneOf( 'm u n p f' ) )
        floatnumber  = floatnumberb | floatnumbera
        return floatnumber
    
    def Parameter( self, identifier, floatnumber ):
        mult  = Literal( '*' )
        div   = Literal( '/' )
        paramname    = identifier
        paramvexpr   = floatnumber | identifier
        paramvalue   = Group( paramvexpr + ( ZeroOrMore( mult + paramvexpr ) ^ ZeroOrMore( div + paramvexpr ) ) )
        parameter    = Group( paramname + Suppress( '=' ) + paramvalue )
        return parameter
    
    def Instance( self, newline, identifier, parameter ):
        # instantiating a subckt, or device model
        lpar  = Literal( '(' )
        rpar  = Literal( ')' )
        nodename      = identifier
        iname         = identifier
        subcktname    = identifier
        connections   = Suppress( lpar ) + Group( nodename + nodename + ZeroOrMore( nodename ) ) + Suppress( rpar )
        instance      = Group( iname + connections + subcktname + Group( ZeroOrMore( parameter ) ) ) + newline
        return instance
    
    def InstanceNoParens( self, newline, identifier, parameter ):
        instanceNoPar = Group( OneOrMore( identifier ^ parameter ) ) + newline
        return instanceNoPar
    
    def SubCktParams( self, newline, parameter ):
        # parameter list for subckt
        paramdecltok = Literal( 'parameters' ).suppress()
        scparams     = Group( paramdecltok + OneOrMore( parameter ) ).setName( 'scparams' ) + newline
        return scparams
    
    def SubCkt( self, newline, comment, identifier, parameter, instance ):
        subcktname = identifier
        portname   = identifier
        scparams   = self.SubCktParams( newline, parameter ).setParseAction( self.RecordSubcktParams )
        portList   = Group( OneOrMore( portname ) ).setName( 'portList' )
        scdecltok  = Literal( 'subckt' ).suppress()
        subcktdecl = Group( scdecltok + subcktname + portList ).setParseAction( self.BeginSubckt ) + newline
        endsc      = Literal( 'ends' ) + restOfLine + newline
        scinstance = instance.copy().setParseAction( self.RecordSubcktInstantiation )
        sccomment  = comment.copy().setParseAction( self.RecordSubcktComment )
        scitem     = newline | sccomment | scinstance # | instanceNoPar
        # the full subckt
        subckt = subcktdecl + Optional( scparams ) + OneOrMore( scitem ) + Suppress( endsc )
        subckt.setParseAction( self.RecordSubckt )
        return subckt
    
    def Netlist( self, newline, comment, simcmd, subckt, inopar, instance ):
        # local copy of instance & instance, no parens, thus unique parse action:
        nlip      = instance.copy().setParseAction( self.RecordNetlistInstantiation )
        nlin      =   inopar.copy().setParseAction( self.RecordNetlistInstantiationNoParens )
        nlcomment =  comment.copy().setParseAction( self.RecordNetlistComment )
        #
        # note the order of nlin & nlip matters:
        return OneOrMore( newline | nlcomment | simcmd | subckt | nlin | nlip )
    
    def Parse( self, s ):
        return self.parser.parseString( s )
    
    def __init__( self ):
        
        self.netlist  = []
        self.subckts  = []
        self.comments = []
        self.items    = []
        
        # white space is apparently significant, therefore parse it:
        self.SetWhitespaceChars()
        newline    = Suppress( Literal( '\n' ) )
        #
        # cpp comments and simulator commands (stuff that we will ignore):
        comment    = cppStyleComment
        simcmd    = Combine( oneOf( 'simulator myOption ic' ) + restOfLine ).setName( 'simcmd' )
        #
        # a generic identifier:
        identifier = Word( alphas + nums + '_' )
        #
        # numbers:
        integer   = self.Integer()
        floatnum  = self.FloatNum( integer )
        #
        # parameters:
        parameter = self.Parameter( identifier, floatnum )
        #
        # subckt (or model) instances:
        instance  = self.Instance( newline, identifier, parameter )
        inopar    = self.InstanceNoParens( newline, identifier, parameter )
        #
        # subckt and the entire netlist:
        subckt    = self.SubCkt(  newline, comment, identifier, parameter, instance )
        parser    = self.Netlist( newline, comment, simcmd, subckt, inopar, instance )
        self.parser = parser
    

################################################################################
################################################################################

if __name__ == "__main__":
    parser = SpectreNetlistParser()
    
    lines = ReadFileLines( sys.argv[ 1 ] )
    lines = JoinContinuedLines( lines )
    s = '\n'.join( lines )
        
    x = parser.Parse( s )
    
    #for s in subckts: s.pprint()
    #print '--------------------------------------------------------------------------------------------------------------------------------------------'
    #for i in netlist: i.pprint()
        
    print '* first line in hspice'
    for s in parser.subckts: s.hspprint()
    for i in parser.netlist: i.hspprint()
    
    print ''
    print '.end'
    
    

