1:@:(dbr bind Debug)@:(9!:19)2^_44[(prolog [ echo^:ECHOFILENAME) './g110i.ijs'
NB. */ B ----------------------------------------------------------------

times=: 4 : 'x*y'

(*/   -: times/  ) x=:?3 5 23$2
(*/"1 -: times/"1) x
(*/"2 -: times/"2) x

(*/   -: times/  ) x=:?3 5 32$2
(*/"1 -: times/"1) x
(*/"2 -: times/"2) x

f=: 3 : '(*/ -: times/) y ?@$ 2'
,f"1 x=:7 8 9,."0 1 [ _1 0 1+  255
,f"1 |."1 x
,f"1 x=:7 8 9,."0 1 [ _1 0 1+4*255
,f"1 |."1 x


NB. */ I ----------------------------------------------------------------

times=: 4 : 'x*y'

(*/ -: times/) x=:10 20 30 1e6 2e6
(*/ -: times/) |.x

(*/   -: times/  ) x=:_1e2+?    23$2e2
(*/   -: times/  ) x=:_1e2+?4   23$2e2
(*/"1 -: times/"1) x
(*/   -: times/  ) x=:_1e2+?7 5 23$2e2
(*/"1 -: times/"1) x
(*/"2 -: times/"2) x


NB. */ D ----------------------------------------------------------------

times=: 4 : 'x*y'

(*/   -: times/  ) x=:0.1*_1e2+?    23$2e2
(*/   -: times/  ) x=:0.1*_1e2+?4   23$2e2
(*/"1 -: times/"1) x
(*/   -: times/  ) x=:0.1*_1e2+?7 5 23$2e2
(*/"1 -: times/"1) x
(*/"2 -: times/"2) x


NB. */ Z ----------------------------------------------------------------

times=: 4 : 'x*y'

(*/   -: times/  ) x=:j./0.1*_1e2+?2     23$2e2
(*/   -: times/  ) x=:j./0.1*_1e2+?2 4   23$2e2
(*/"1 -: times/"1) x
(*/   -: times/  ) x=:j./0.1*_1e2+?2 7 5 23$2e2
(*/"1 -: times/"1) x
(*/"2 -: times/"2) x

4!:55 ;:'f times x'



echo^:ECHOFILENAME 'memory used: ',":7!:1''

