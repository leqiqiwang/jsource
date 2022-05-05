1:@:(dbr bind Debug)@:(9!:19)2^_44[(prolog [ echo^:ECHOFILENAME) './gsp530n.ijs'
NB. } scattered amendment ------------------------------------------------

d=: (?23 5$2)*?23 5 7 3 2$4

f=: 3 : '(0 i}d) -: 0 i}s=:(2;y)$.d'
c=: ; (i.1+r) <"1@comb&.>r=:#$d

] b=: f&>c [ i=: <"1 ?   12 1$1{.$d
] b=: f&>c [ i=: <"1 ?  2 6 2$2{.$d
] b=: f&>c [ i=: <"1 ?2 2 3 3$3{.$d
] b=: f&>c [ i=: <"1 ?  4 3 4$4{.$d
] b=: f&>c [ i=: <"1 ?1 3 3 5$5{.$d

d=: (?23 5$2)*?23 5 7 3 2$4

f=: 3 : '(999 i}d) -: 999 i}s=:(2;y)$.d'
c=: ; (i.1+r) <"1@comb&.>r=:#$d

] b=: f&>c [ i=: <"1 ?   12 1$1{.$d
] b=: f&>c [ i=: <"1 ?  2 6 2$2{.$d
] b=: f&>c [ i=: <"1 ?2 2 3 3$3{.$d
] b=: f&>c [ i=: <"1 ?  4 3 4$4{.$d
] b=: f&>c [ i=: <"1 ?1 3 3 5$5{.$d

d=: (?23 5$2) * ?23 5 3 2 4$5
c=: ; (i.1+r) <"1@comb&.>r=:#$d
g=: 3 : '($y{d) ?@$ 7'

f=: 3 : '(a i}d) -: a i}s=:(2;y)$.d [ a=: g i'
] b=: f&>c [ i=: <"1 ?   12 1$1{.$d
] b=: f&>c [ i=: <"1 ?  2 6 2$2{.$d
] b=: f&>c [ i=: <"1 ?2 2 3 3$3{.$d
] b=: f&>c [ i=: <"1 ?  4 3 4$4{.$d
] b=: f&>c [ i=: <"1 ?1 3 3 5$5{.$d

f=: 4 : '(a i}d) -: a i}s=: (2;x)$.d [ a=: g i=: x h y'

h=: 4 : '<"1 v#:(>:?2*n)?@$n=. */v=. y{.$d'
] b=: |: c f&>/ >:i.#$d

h=: 4 : '<"1 v#:5 7 4 ?@$n=. */v=. y{.$d'
] b=: |: c f&>/ >:i.#$d

f=: 3 : '(a i}d) -: a i}s=:(2;y)$.d'
i=: <?#d                      NB. single index
] b=: f&>c [ a=:   ?(}.$d)$5  NB. dense  replacement data
] b=: f&>c [ a=: $.?(}.$d)$5  NB. sparse replacement data


NB. amend in place -------------------------------------------------------

d=: (0=?7 5$3)*?7 5 1000$1000
s=: (2;0 1)$.d

i=: <"1 ?15 2$7 5
(IF64{9000 18000) > t=: 7!:2 's=: 0 i}s'
s -: d=: 0 i}d
scheck s

i=: <"1 ?75 3$7 5 1000
(IF64{9000 18000) > t=: 7!:2 's=: 0 i}s'
s -: d=: 0 i}d
scheck s

i=: <"1 (?4$1000),.~(?4$#i){i=:4$.s
x=: ?(#i)$1e6
(IF64{9000 18000) > t=: 7!:2 's=: x i}s'
s -: d=: x i}d
scheck s

i=: <"1 (?4$1000),.~(?4$#i){i=: (7 5#:i.35)-.4$.s
x=: ?(#i)$1e6
s=: x i}s  NB. not in place
s -: d=: x i}d
scheck s


NB. amend in place for sparse replacement data ---------------------------

d=: (0=?7 5$3)*?7 5 1000$1000
s=: (2;0 1)$.d

i=: <"1 (?375$#x){x=: 4$.s
a=: $. ($i{d)$3
(IF64{30000 60000) > t=: 7!:2 's=: a i}s'
s -: d=: a i}d
scheck s

i=: <"1 (?4$1000),.~(?4$#x){x=:4$.s
a=: $.?(#i)$1e6
(IF64{30000 60000) > t=: 7!:2 's=: a i}s'
s -: d=: a i}d
scheck s

i=: <"1 (?4$1000),.~(?4$#i){i=: (7 5#:i.35)-.4$.s
a=: $. ?(#i)$1e6
s=: a i}s  NB. not in place
s -: d=: a i}d
scheck s


NB. sparse replacement data ----------------------------------------------

d=: (?23 5$2) * ?23 5 3 2 4$5
c=: ; (i.1+r) <"1@comb&.>r=:#$d
g=: 3 : '?($y{d)$7'

f=: 3 : '(a i}d) -: ($.a) i}s=:(2;y)$.d [ a=: g i'

] b=: f&>c [ i=: <"1 ?   12 1$1{.$d
] b=: f&>c [ i=: <"1 ?  2 6 2$2{.$d
] b=: f&>c [ i=: <"1 ?2 2 3 3$3{.$d
] b=: f&>c [ i=: <"1 ?  4 3 4$4{.$d
] b=: f&>c [ i=: <"1 ?1 3 3 5$5{.$d

f=: 4 : '(a i}d) -: ($.a) i}s=: (2;x)$.d [ a=: g i=: x h y'

h=: 4 : '<"1 v#:(>:?2*n)?@$n=. */v=. y{.$d'
] b=: |: c f&>/ >:i.#$d

h=: 4 : '<"1 v#:3 4 5?@$n=. */v=. y{.$d'
] b=: |: c f&>/ >:i.#$d

s=: 1$.5 6;'';123
d=: 5 6$123
i=: <"1 ?3 2$5 6
x=: ?($i{d)$1000
s -: d
(x i}d) -: x i}s

case=: 3 : 0
 select. y
 case. 0 do.
  d=: i.4 3 2
  i=: 3,&.>0 1 2
 case. 1 do. 
  d=: 4{. i.3 3 4 2
  i=: 3 ,&.>0 1 2
 case. 2 do.
  d=: 4{. i. 3 3 4 2
  i=: 3 ,&.>0 1 2
 case. 3 do.
  d=: 4{. ?. 3 3 4 2$4
  i=: <"1 ?.5 2 3 4 3$4 3 4
 case. 4 do.
  d=: ? 11 7 3 4 2$4
  i=: <"1 ?500 5$$d
 end.
 s=. $i{d
 e=: (?.s$2)*100+?.s$50
 ca=: ; (i.1+r) <"1@comb&.> r=: #$e
 cw=: ; (i.1+r) <"1@comb&.> r=: #$d
 1
)

g=: 4 : '((2;x)$.e) i} (2;y)$.d'
f=: 4 : '(e i}d) -: x g y'
h=: 4 : '((2;y)$.e i}d) ;  x g y'

case 0
*./, b=: ca f&>/cw
case 1
*./, b=: ca f&>/cw
case 2
*./, b=: ca f&>/cw
case 3
*./, b=: ca f&>/cw
case 4
*./, b=: ca f&>/cw

'domain error' -: 'abc'   (0 0;1 1;2 2)} etx $.i.4 5

'index error'  -: 999     (0 1;2 4)    } etx $.i.3 4

'length error' -: (i.3 4) (2;3)        } etx (2;0 1)$. i.4 5


4!:55 ;:'a b c ca case cw d e f g h i m r s t x y'



echo^:ECHOFILENAME 'memory used: ',":7!:1''

