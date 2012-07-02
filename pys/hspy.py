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
         v = subtoks[1].lstrip( "'" ).rstrip( "'" ).lstrip( '(' ).rstrip( ')' )
         params[ k ] = v
      else:
         toks.append( tok )
         break
   assert len( toks )
   line = ' '.join( toks )
   return ( line, params )
