1:@:(dbr bind Debug)@:(9!:19)2^_44[(prolog [ echo^:ECHOFILENAME) './gpcoinv.ijs'
NB. p:^:_1 --------------------------------------------------------------

f =: p:^:_1

([ -: f@:p:) x=:i.200
([ -: f@:p:) x=:?500$400
([ -: f@:p:) x=:1e6*>:i.105

v =: p: i.200
(f -: +/@(v&<)"0) x=:i.500
(f -: +/@(v&<)"0) x=:?500${:v

(i.@2: -: [ <: p:@(_1 0&+)@f)"0 x=:(v{~?20$#v)-.2
(i.@2: -: [ <: p:@(_1 0&+)@f)"0 x=:?5$2e9

105097564 = f <._1+2^31

NB. LeVeque, Fundamentals of Number Theory, Addison-Wesley, 1977, p. 5.

0        = f 1e0
4        = f 1e1
25       = f 1e2
168      = f 1e3
1229     = f 1e4
9592     = f 1e5
78498    = f 1e6
664579   = f 1e7
5761455  = f 1e8
50847534 = f 1e9


4!:55 ;:'f v x'



echo^:ECHOFILENAME 'memory used: ',":7!:1''

