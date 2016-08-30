NB. ,./ ------------------------------------------------------------------

randuni''

(,./ -: |:) x=: a.{~?1001 123$#a.

f=: 3 : 0
 n=. #y
 r=. #$y
 if. 0=n do. +'domain error' end.
 if. 1=n do. {.y return. end.
 if. 1>:r do. 
  if. 2=n do. y else. (_2}.y) ,"1 0 (_2{.y) end.
  return.
 end.
 if. 2=r do. |:y return. end.
 s=. $ y
 t=. ((- 2&<)#$y) $ (1{s) , (*/0 2{s,1 1), 3}.s
 t ($,) (<"0 (<0 1) C. i.#$y) |: y
)

g=: 4 : 'x,.y'

((,./ -: g/) , ,./ -: f) x=: ?  1e6
((,./ -: g/) , ,./ -: f) x=: (1,?0$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1,?1$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1,?2$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1,?3$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1,?4$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1,?5$8) ?@$ 1e6

((,./ -: g/) , ,./ -: f) x=: ?1$1e6
((,./ -: g/) , ,./ -: f) x=: ?2$1e6
((,./ -: g/) , ,./ -: f) x=: ?3$1e6
((,./ -: g/) , ,./ -: f) x=: ?4$1e6
((,./ -: g/) , ,./ -: f) x=: ?(1+?123)$1e6

((,./ -: g/) , ,./ -: f) x=: (1+?2$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1+?3$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1+?4$8) ?@$ 1e6
((,./ -: g/) , ,./ -: f) x=: (1+?5$8) ?@$ 1e6


(,./ etx -: f etx) i.0

(,./"1 etx -: f"1  etx) i.1 2 3 0
(,./"2 etx -: f"2  etx) i.1 2 0 4
(,./"3 etx -: f"3  etx) i.1 0 3 4

(,./"1 etx -: g/"1 etx) i.1 2 3 0
(,./"2 etx -: g/"2 etx) i.1 2 0 4
(,./"3 etx -: g/"3 etx) i.1 0 3 4

((,./"1 -: g/"1) , ,./"1 -: f"1) x=: (1+?2$8) ?@$ 1e6
((,./"1 -: g/"1) , ,./"1 -: f"1) x=: (1+?3$8) ?@$ 1e6
((,./"1 -: g/"1) , ,./"1 -: f"1) x=: (1+?4$8) ?@$ 1e6
((,./"1 -: g/"1) , ,./"1 -: f"1) x=: (1+?5$8) ?@$ 1e6

((,./"2 -: g/"2) , ,./"2 -: f"2) x=: (1+?2$8) ?@$ 1e6
((,./"2 -: g/"2) , ,./"2 -: f"2) x=: (1+?3$8) ?@$ 1e6
((,./"2 -: g/"2) , ,./"2 -: f"2) x=: (1+?4$8) ?@$ 1e6
((,./"2 -: g/"2) , ,./"2 -: f"2) x=: (1+?5$8) ?@$ 1e6

((,./"3 -: g/"3) , ,./"3 -: f"3) x=: (1+?2$8) ?@$ 1e6
((,./"3 -: g/"3) , ,./"3 -: f"3) x=: (1+?3$8) ?@$ 1e6
((,./"3 -: g/"3) , ,./"3 -: f"3) x=: (1+?4$8) ?@$ 1e6
((,./"3 -: g/"3) , ,./"3 -: f"3) x=: (1+?5$8) ?@$ 1e6

'domain error' -: ,./ etx i.0
'domain error' -: ,./ etx i.0,?1$10
'domain error' -: ,./ etx i.0,?2$10
'domain error' -: ,./ etx i.0,?3$10

'limit error'  -: ,./ etx i. >IF64{1e7 1 1e5 0; 1e17 1 1e5 0
'limit error'  -: ,./ etx i. >IF64{3   1 1e9 0; 3 1 5e18 0


NB. ,./ on sparse arguments ---------------------------------------------

sp=: 4 : 0
 yy=: y
 t=: ,./"x yy
 c=: ; (i.1+r) <"1@comb&.> r=. #$y
 for_d. c do.
  assert. (t -: z) *. scheck z=: ,./"x s=: (2;d)$.yy
 end.
 1
)

_ sp x=: (* $ ?@$ 2:) (1+?2$8) ?@$ 1e6
_ sp x=: (* $ ?@$ 2:) (1+?3$8) ?@$ 1e6
_ sp x=: (* $ ?@$ 2:) (1+?4$8) ?@$ 1e6

1 sp x=: (* $ ?@$ 2:) (1+?2$8) ?@$ 1e6
1 sp x=: (* $ ?@$ 2:) (1+?3$8) ?@$ 1e6
1 sp x=: (* $ ?@$ 2:) (1+?4$8) ?@$ 1e6

2 sp x=: (* $ ?@$ 2:) (1+?2$8) ?@$ 1e6
2 sp x=: (* $ ?@$ 2:) (1+?3$8) ?@$ 1e6
2 sp x=: (* $ ?@$ 2:) (1+?4$8) ?@$ 1e6

3 sp x=: (* $ ?@$ 2:) (1+?2$8) ?@$ 1e6
3 sp x=: (* $ ?@$ 2:) (1+?3$8) ?@$ 1e6
3 sp x=: (* $ ?@$ 2:) (1+?4$8) ?@$ 1e6


NB. ,.&.>/ --------------------------------------------------------------

f    =: 4 : 'x,.&.>y'
test =: f/ -: ,.&.>/

test 5$&.>'abcde'
test 5$&.>u:'abcde'
test 5$&.>10&u:'abcde'
test 5$&.>s:@<"0 'abcde'

test ?@($&2)          &.> (1+?123),&.>1+?k$20 [ k=:10
test {&a.@?@($&(#a.)) &.> (1+?123),&.>1+?k$20
test {&adot1@?@($&(#adot1)) &.> (1+?123),&.>1+?k$20
test {&adot2@?@($&(#adot2)) &.> (1+?123),&.>1+?k$20
test {&sdot@?@($&(#sdot)) &.> (1+?123),&.>1+?k$20
test ?@($&1000)       &.> (1+?123),&.>1+?k$20
test o.@?@($&1000)    &.> (1+?123),&.>1+?k$20
test r.@?@($&1000)    &.> (1+?123),&.>1+?k$20
test {&x@?@($&(#x))   &.> (1+?123),&.>1+?k$20 [ x=.;:'bou stro phe don ic 1'
test {&x@?@($&(#x))   &.> (1+?123),&.>1+?k$20 [ x=.;:u:'bou stro phe don ic 1'
test {&x@?@($&(#x))   &.> (1+?123),&.>1+?k$20 [ x=.;:10&u:'bou stro phe don ic 1'
test {&x@?@($&(#x))   &.> (1+?123),&.>1+?k$20 [ x=.s:@<"0&.> ;:'bou stro phe don ic 1'
test {&x@?@($&(#x))   &.> (1+?123),&.>1+?k$20 [ x=.<"0@s: ;:'bou stro phe don ic 1'


test ((1+?123),&.>2 3 4) $&.> 2;3;4.5
test ((1+?123),&.>2 3 4) $&.> 2;3;4j5
test ((1+?123),&.>2 3 4) $&.> 2;3;0

'domain error' -: ,.&.>/ etx i.0

'length error' -: ,.&.>/ etx 1 2;3 4 5


4!:55 ;:'adot1 adot2 sdot c f g k s sp t test x yy z'


