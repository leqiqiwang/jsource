1:@:(dbr bind Debug)@:(9!:19)2^_44[(prolog [ echo^:ECHOFILENAME) './gsp221.ijs'
NB. ~. ------------------------------------------------------------------

f=: 3 : '(scheck t) *. (~.p) -: t=: ~.s=: (2;y)$.p'

c=: ; (i.1+r) <"1@comb&.>r=: 4
f&> c [ p=: (?200$100) { (?100 4 5$2) *    ?  100 4 5 3$2
f&> c [ p=: (?200$100) { (?100 4 5$2) *    ?  100 4 5 3$100
f&> c [ p=: (?100$ 50) { (? 50 4 5$2) * o. ?   50 4 5 3$100
f&> c [ p=: (?100$ 50) { (? 50 4 5$2) * j./?2  50 4 5 3$100

c=: ; (i.1+r) <"1@comb&.>r=: 3
f&> c [ p=:                          ?100 4 5$1000
f&> c [ p=:                   0 (0)} ?100 4 5$1000
f&> c [ p=:                   0 (9)} ?100 4 5$1000
f&> c [ p=:                   0    * ?100 4 5$1000
f&> c [ p=: (?200$100) {             ?100 4 5$1000
f&> c [ p=: (?200$100) {(?100 4$2) * ?100 4 5$1000

(~.$.p) -: ~.p=: i.0
(~.$.p) -: ~.p=: i.0 5
(~.$.p) -: ~.p=: i.5 0

p=: ?100$1e9
q=: (?100$20){?20 5$10
s=: q p}1 $. 1e9 5 ; 0 ; 2-2
t=: ~.s
scheck t
($t)   -: $~.0,q
(2$.t) -: ,0
(3$.t) -: 0
(4$.t) -: ,. (i.#t)-.{.(i.1+#p) -. p
(5$.t) -: ~.(q/:p) -. (0 * {. q)


4!:55 ;:'c f p q r s t'

echo^:ECHOFILENAME 'memory used: ',":7!:1''

