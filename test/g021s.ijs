1:@:(dbr bind Debug)@:(9!:19)2^_44[(prolog [ echo^:ECHOFILENAME) './g021s.ijs'
randuni''

NB. >./\. B -------------------------------------------------------------

(0 1 1 1,:0 1 0 1) -: >./\. 0 0 1 1 ,: 0 1 0 1

max=: 4 : 'x>.y'

f=: 3 : '(>./\. -: max/\.) y ?@$ 2'
,f"1 x=:7 8 9,."0 1 [ _1 0 1+  255
,f"1 |."1 x
,f"1 x=:7 8 9,."0 1 [ _1 0 1+4*255
,f"1 |."1 x


NB. >./\. I -------------------------------------------------------------

max=: 4 : 'x>.y'

(>./\. -: max/\.) x=:1 2 3 1e9 2e9
(>./\. -: max/\.) |.x

(>./\.   -: max/\.  ) x=:_1e4+?    23$2e4
(>./\.   -: max/\.  ) x=:_1e4+?4   23$2e4
(>./\."1 -: max/\."1) x
(>./\.   -: max/\.  ) x=:_1e4+?7 5 23$2e4
(>./\."1 -: max/\."1) x
(>./\."2 -: max/\."2) x

(>./\.   -: max/\.  ) x=:_1e9+?    23$2e9
(>./\.   -: max/\.  ) x=:_1e9+?4   23$2e9
(>./\."1 -: max/\."1) x
(>./\.   -: max/\.  ) x=:_1e9+?7 5 23$2e9
(>./\."1 -: max/\."1) x
(>./\."2 -: max/\."2) x


NB. >./\. D -------------------------------------------------------------

max=: 4 : 'x>.y'

(>./\.   -: max/\.  ) x=:0.01*_1e4+?    23$2e4
(>./\.   -: max/\.  ) x=:0.01*_1e4+?4   23$2e4
(>./\."1 -: max/\."1) x
(>./\.   -: max/\.  ) x=:0.01*_1e4+?7 5 23$2e4
(>./\."1 -: max/\."1) x
(>./\."2 -: max/\."2) x


NB. >./\. Z -------------------------------------------------------------

max=: 4 : 'x>.y'

(>./\.   -: max/\.  ) x=:[&.j. 0.1*_1e2+?2     23$2e2
(>./\.   -: max/\.  ) x=:[&.j. 0.1*_1e2+?2 4   23$2e2
(>./\."1 -: max/\."1) x
(>./\.   -: max/\.  ) x=:[&.j. 0.1*_1e2+?2 7 5 23$2e2
(>./\."1 -: max/\."1) x
(>./\."2 -: max/\."2) x

NB. >./\. SB -------------------------------------------------------------

max=: 4 : 'x>.y'

(>./\. -: max/\.) x=:sdot0
(>./\. -: max/\.) |.x

(>./\.   -: max/\.  ) x=:sdot0{~ ?    23$#sdot0
(>./\.   -: max/\.  ) x=:sdot0{~ ?4   23$#sdot0
(>./\."1 -: max/\."1) x
(>./\.   -: max/\.  ) x=:sdot0{~ ?7 5 23$#sdot0
(>./\."1 -: max/\."1) x
(>./\."2 -: max/\."2) x

'domain error' -: >./\. etx 'triskaidekaphobia'
'domain error' -: >./\. etx ;:'professors in New England'
'domain error' -: >./\. etx u:'triskaidekaphobia'
'domain error' -: >./\. etx u:&.> ;:'professors in New England'
'domain error' -: >./\. etx 10&u:'triskaidekaphobia'
'domain error' -: >./\. etx 10&u:&.> ;:'professors in New England'

4!:55 ;:'adot1 adot2 sdot0 f max x'
randfini''


echo^:ECHOFILENAME 'memory used: ',":7!:1''

