/* Copyright (c) 1990-2023, Jsoftware Inc.  All rights reserved.           */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Adverbs: Inverse & Identity Functions                                   */

#include "j.h"


static F1(jtinvamp);

static B ip(A w,C c,C d){A f,g;V*v;
 v=FAV(w); f=v->fgh[0]; g=v->fgh[1];
 R CSLASH==ID(f)&&c==ID(FAV(f)->fgh[0])&&d==ID(g);
}

static B jtconsf(J jt,A w){C c;
 c=ID(w);
 if(c==CFCONS||c==CQQ&&(NUMERIC&AT(FAV(w)->fgh[0])))R 1;  // 9: or m"n  (scaf should test infinite rank?)
 if(c==CFORK&&FAV(w)->fgh[2])R jtconsf(jt,unname(FAV(w)->fgh[0]))&&jtconsf(jt,unname(FAV(w)->fgh[2]));  //constant g constant
 R 0;
}    // 1 iff w is a constant function or (constant function) g (constant function)

static F2(jtfong){A f;C c;V*v;
 ARGCHK2(a,w);
 v=FAV(a); c=v->id; f=v->fgh[0]; A g=v->fgh[1];  A h=v->fgh[2];
 if(c==CFORK&&h==0){h=g; g=f; f=ds(CCAP);}   // reconsitute capped fork 
 R (c&~1)==CLEFT ? w : c==CFORK&&(NOUN&AT(f)||CCAP==ID(f)) ? folk(f,g,fong(h,w)) : folk(ds(CCAP),a,w);
}   // [: f g  with simplifications: [: ][ w -> w;  [: (N/[: x y) w -> N/[: x [: y w   and y omitted if ][

static F1(jtinvfork){A f,fi,g,gi,h,k;B b,c;V*v;
 ARGCHK1(w);
 v=FAV(w);
 // extract fgh; replace nameref with value in fh
 if(v->fgh[2]){RZ(f=unname(v->fgh[0])); g=v->fgh[1]; RZ(h=unname(v->fgh[2]));}else{f=ds(CCAP); g=v->fgh[0]; RZ(h=unname(v->fgh[1]));}  // reconsitute capped fork
 if(CCAP==ID(f))R fong(invrecur(h),invrecur(g));  //  [: g h => h^:_1 @: g^:-1, simplified
 c=1&&NOUN&AT(f); b=c||jtconsf(jt,f);  // c if NVV fork; b if NVV or f is 9: = 'f is constant'
 ASSERT(b!=jtconsf(jt,h),EVDOMAIN);  // must have a const, but not 2 - no verb to invert then
 RZ(k=c?f:df1(gi,num(0),b?f:h));  // evaluate the constant on atomic 0
 RZ(gi=invrecur(b?amp(k,g):amp(g,k)));  // if f const get const&g^_1; if h const g&const^:_1
 RZ(fi=invrecur(b?h:f));  // invert the non-const arg
 if(CAMP==ID(gi)){
  v=FAV(gi); 
  if     (NOUN&AT(v->fgh[0]))RZ(gi=folk(v->fgh[0],     v->fgh[1], ds(CRIGHT)))
  else if(NOUN&AT(v->fgh[1]))RZ(gi=folk(v->fgh[1],swap(v->fgh[0]),ds(CRIGHT)));
 }
 R fong(fi,gi);  // if f const, h^:_1 @: const&g^:_1   if h const, f^:_1 @: g&const^:_1
}

static DF1(jtexpandf){A f; ARGCHK2(w,self); f=FAV(self)->fgh[0]; R expand(VAV(f)->fgh[0],w);}

static DF1(jtexpandg){A f,g,z;V*v;
 ARGCHK2(w,self);
 f=FAV(self)->fgh[0]; v=FAV(f); g=v->fgh[1];
 jt->fill=FAV(g)->fgh[1]; z=expand(v->fgh[0],w); jt->fill=0;   // elements of FAV cannot be virtual
 R z;
}

static F2(jtdiag){I m,p,r,t,*v;
 ARGCHK2(a,w);
 r=AR(w); t=AT(w);
 v=AS(w);   m=0;      DO(r, m=MIN(m,v[i]););
 v=AS(w)+r;
 if(TYPESNE(t,AT(a)))RZ(a=cvt(t,a));
 if(AR(a)){
  ASSERT(m==AN(a),EVLENGTH);
  ASSERT(0,EVNONCE);
 }else{
  ASSERT(0,EVNONCE);
}}

static F1(jtbminv){A*wv,x,z=w;I i,j,m,r,*s,t=0,*u,**v,*y,wn,wr,*ws;
 ARGCHK1(w);
 ASSERT(0,EVNONCE);
 ASSERT(BOX&AT(w),EVDOMAIN);
 wn=AN(w); wr=AR(w); ws=AS(w); wv=AAV(w); 
 if(1>=wr)R raze(w);
 if(!wn)R iota(reshape(sc(wr),num(0)));
 GATV0(x,INT,wr,1); u=AV(x); mvc(wr*SZI,u,1,MEMSET00);
 GATV0(x,INT,wr,1); v=(I**)AV(x);
 DO(wr, m=ws[i]; GATV0(x,INT,m,1); mvc(m*SZI,v[i]=AV(x),1,MEMSETFF););
 for(i=0;i<wn;++i){
  x=C(wv[i]); r=AR(x); s=AS(x);
  if(AN(x)){if(!t)t=AT(x); ASSERT(HOMO(t,AT(x)),EVDOMAIN);}
  ASSERT(2>r||r==wr,EVRANK);
  if(2>r)z=0;
  else DO(wr, y=v[i]+u[i]; if(0>*y)*y=s[i]; else ASSERT(*y==s[i],EVLENGTH););
  j=wr; while(1){--j; ++u[j]; if(ws[j]>u[j])break; u[j]=0;}
 }
 if(!z){A f,h,*zv;I*hv;
  GATVR(z,BOX,wn,2,ws); zv=AAV(z);
  GATV0(h,INT,wr,1); hv=AV(h);
  GA10(f,t,1); RZ(f=filler(f)); mvc(wr*SZI,u,1,MEMSET00);
  for(i=0;i<wn;++i){
   zv[i]=x=C(wv[i]);
   if(2>AR(x)){DO(wr, hv[i]=v[i][u[i]];); RZ(zv[i]=diag(x,reshape(h,f)));}
   j=wr-1; while(1){--j; ++u[j]; if(ws[j]>u[j])break; u[j]=0;}
 }}
 DO(wr, A t=z; RZ(df1(z,t,slash(under(qq(ds(CCOMMA),sc(wr-i)),ds(COPE))))););
 RETF(jtopenforassembly(jt,z));
}    /* <;.1 or <;.2 inverse on matrix argument */

// find inverse of f&g when one arg is a noun and the other a verb
static F1(jtinvamp){A f,ff,g,h,x,y;B nf,ng;C c,d,*yv;I n;V*u,*v;
 ARGCHK1(w);
 v=FAV(w);
 f=v->fgh[0]; nf=!!(NOUN&AT(f));
 g=v->fgh[1]; ng=!!(NOUN&AT(g));
 h=nf?g:f; x=nf?f:g; c=ID(h); u=VAV(h);   // h=verb arg, x=noun arg
 switch(c){
  case CPLUS:    R amp(negate(x),h);
  case CSTAR:    R amp(recip(x), h);
  case CMINUS:   R nf?w:amp(x,ds(CPLUS));
  case CDIV:     R nf?w:amp(x,ds(CSTAR));
  case CROOT:    R amp(ds(nf?CEXP:CLOG),x);
  case CEXP:     R ng&&equ(x,num(2))?ds(CROOT):amp(x,ds(nf?CLOG:CROOT));
  case CLOG:     R nf?amp(x,ds(CEXP)):amp(ds(CROOT),x);
  case CJDOT:    R nf?atop(invrecur(ds(CJDOT)),amp(ds(CMINUS),x)):amp(ds(CMINUS),jdot1(x));
  case CRDOT:    R nf?atop(invrecur(ds(CRDOT)),amp(ds(CDIV  ),x)):amp(ds(CDIV  ),rdot1(x));
  case CLBRACE:  R nf?amp(pinv(x),h):amp(x,ds(CIOTA));
  case COBVERSE: ff=FAV(h)->fgh[1]; R amp(nf?x:ff,nf?ff:x);
  case CPDERIV:  if(nf&&!AR(x))R ds(CPDERIV); break;  // only atom&p.. is invertible
xco:
  case CXCO:     RE(n=i0(x)); ASSERT(n&&BETWEENC(n,-2,2),EVDOMAIN);  // fall through to create (-x)&u
  case CROT:          // fall through to create (-x)&u
  case CCIRCLE:       // fall through to create (-x)&u
  case CSPARSE:  if(nf)R amp(negate(x),h);   break;
  case CABASE:   if(nf)R amp(x,ds(CBASE));   break;
  case CIOTA:    if(nf)R amp(ds(CLBRACE),x); break;
  case CTHORN:   if(nf)R ds(CEXEC);          break;
  case CTILDE:   
   if(ff=FAV(h)->fgh[0],VERB&AT(ff))R invamp(amp(nf?ff:x,nf?x:ff));
   else{ff=unname(h); R invamp(amp(nf?x:ff,nf?ff:x));}
  case CSCO:     
   ASSERT(nf!=0,EVDOMAIN); 
   RE(n=i0(x)); ASSERT(n&&BETWEENC(n,-6,6),EVDOMAIN);
   R amp(sc(-n),h);
  case CUCO:
   ASSERT(nf!=0,EVDOMAIN); 
   RE(n=i0(x)); ASSERT(BETWEENC(n,1,8)&!BETWEENC(n,5,6),EVDOMAIN);
   R amp(sc(-(1^(-n))),h);
  case CCANT:    
   ASSERT(nf!=0,EVDOMAIN); 
   R obverse(eva(x,"] |:~ u C.^:_1 i.@#@$"),w);
  case CPCO:
   if(nf){
    RE(n=i0(x));
    switch(n){
     case -4: case 4: R amp(negate(x),h);
     case -1:         R ds(CPCO);
     case  2:         R obverse(eval("*/@(^/)\"2"),w);
     case  3:         R eval("*/");
   }}
   break;
  case CQCO:     
   if(nf){
    ASSERT(!AR(x),EVRANK);
    R obverse(eval(all1(lt(x,zeroionei(0)))?"*/@(^/)\"2":"(p:@i.@# */ .^ ])\"1"),w);
   }
   break;
  case CFIT:
   if(nf&&CXCO==ID(FAV(g)->fgh[0]))goto xco; // m&(x:!.n)^:_1 is (-m)&(x:!.n)
   ASSERT(nf&&CPOUND==ID(FAV(g)->fgh[0]),EVDOMAIN);
   ASSERT(1==AR(x),EVRANK);
   R fdef(0,CPOWOP,VERB, jtexpandg,jtvalenceerr, w,num(-1),0L, VFLAGNONE, RMAX,0L,0L);
  case CPOUND:
   ASSERT(nf!=0,EVDOMAIN);
   ASSERT(1==AR(x),EVRANK);
   R fdef(0,CPOWOP,VERB, jtexpandf,jtvalenceerr, w,num(-1),0L, VFLAGNONE, RMAX,0L,0L);
   break;
  case CPOWOP:
   if(VGERL&u->flag){ff=AAV(u->fgh[2])[1]; R amp(nf?x:ff,nf?ff:x);} 
   break;
  case CCOMMA:  
   SETIC(x,n); 
   R obverse(1==n?ds(nf?CDROP:CCTAIL):amp(sc(nf?n:-n),ds(CDROP)),w);
  case CBASE:   
   if(!nf)break;
   R AR(x) ? amp(x,ds(CABASE)) : 
    obverse(evc(x,mag(x),"$&u@>:@(v&(<.@^.))@(1&>.)@(>./)@:|@, #: ]"),w);
  case CATOMIC:
   if(ng){ASSERT(equ(x,nub(x)),EVDOMAIN); R obverse(atop(f,amp(x,ds(CIOTA))),w);}  // fall through to common obverse (?)
  case CCYCLE:
   if(nf&&AR(x)<=(c==CCYCLE))R obverse(eva(w,"/:@u@(i.@#) { ]"),w); break;
  case CDROP:
   if(!(nf&&1>=AR(x)))break;
   RZ(x=cvt(INT,x));
   RZ(y=eps(v2(-1L,1L),signum(x))); yv=CAV(y);
   f=amp(mag(x),ds(CPLUS));
   g=1==AN(x)?ds(CPOUND):atop(amp(tally(x),ds(CTAKE)),ds(CDOLLAR));
   h=!yv[1]?f:atop(!yv[0]?ds(CMINUS):amp(negate(signum(x)),ds(CSTAR)),f);
   R obverse(hook(swap(ds(CTAKE)),atop(h,g),mark),w);
  case CDOMINO:
   if(!(2==AR(x)&&AS(x)[0]==AS(x)[1]))break;
   ff=eval("+/ .*");
   R nf?atop(h,amp(ff,minv(x))):amp(x,ff);
  case CDOT:
   if(ip(h,CPLUS,CSTAR)){
    ASSERT(2==AR(x),EVRANK);
    ASSERT(AS(x)[0]==AS(x)[1],EVLENGTH);
    R nf?amp(ds(CDOMINO),x):amp(h,minv(x));
   }
   break;
  case CQQ:
   if(ng&&equ(x,num(1))&&equ(f,eval("i.\"1")))R hook(ds(CFROM),ds(CEQ),mark);
   break;
  case CBSLASH:
   if(nf&&(n=i0(x),0>n)&&(d=ID(u->fgh[0]),(d&-2)==CLEFT))R slash(ds(CCOMMA));  // LEFT || RIGHT
   break;
  case CIBEAM:  // h is x!:y
   if(likely(nf!=0)){   // n&(x!:y)
    x=FAV(h)->fgh[0]; y=FAV(h)->fgh[1];
    if(NOUN&AT(x)&&equ(x,num(3))&&NOUN&AT(y)){  // 3!:y
     RE(n=i0(f));
     if(all1(eps(y,v2(4L,5L)))){ASSERT(n&&BETWEENC(n,-2,2),EVDOMAIN); R amp(sc(-n),g);}  // inverse of _2..2&(3!:(4..5)) is (-n)&(3!:(4..5))
     if(all1(eps(y,v2(1L,3L)))){ASSERT(0==n||1==n||10==n||11==n,EVDOMAIN); R foreign(x,num(2));}  // inverse of [0/1/10/11]&(3!:[1/3]) is 3!:2
    }
   }
   break;
  case CBDOT:
   RE(n=i0(x));
   switch(i0(FAV(h)->fgh[1])){
    case 22: case 25:          R w;
    case 19: case 28:          if(ng)R w; break;
    case 21: case 26:          if(nf)R w; break;
    case 32: case 33: case 34: ASSERT(nf!=0,EVDOMAIN); R amp(negate(x),h);
   }
   break;
  case CPOLY:
   if(nf&&1==AR(x)&&2==AN(x)&&NUMERIC&AT(x)&&!equ(zeroionei(0),tail(x))){  // linear polynomial only
    RZ(y=recip(tail(x)));
    R amp(apip(tymes(y,negate(head(x))),y),h);
 }}
 ASSERT(0,EVDOMAIN);
}

// simpleinv[i] is the inverse function for function i
static const C simpleinv[128] = {
[CDIV&127]=CDIV, [CPLUS&127]=CPLUS, [CMINUS&127]=CMINUS, [CLEFT&127]=CLEFT, [CRIGHT&127] =CRIGHT , [CREV&127] =CREV , [CCANT&127]=CCANT, [CPOLY&127]=CPOLY,
[CNOT&127]=CNOT, [CGRADE&127]=CGRADE, [CCYCLE&127] =CCYCLE , [CDOMINO&127]=CDOMINO, [COPE&127] =CBOX , [CBOX&127]=COPE, [CLOG&127] =CEXP , [CEXP&127]=CLOG,
[CGE&127]=CLE, [CLE&127]=CGE, [CHALVE&127]=CPLUSCO, [CPLUSCO&127]=CHALVE, [CSQRT&127]=CSTARCO, [CSTARCO&127]=CSQRT, [CHEAD&127]=CLAMIN, [CLAMIN&127] =CHEAD ,
[CABASE&127]=CBASE, [CBASE&127]=CABASE, [CTHORN&127]=CEXEC, [CEXEC&127]=CTHORN,
};

// Return inverse of monad w.  recur is a recursion indicator, always forced to 0 for the initial call, and
// set to 1 here for recursive calls
A jtinv(J jt, A w, I recur){A f,ff,g;B b,nf,ng,vf,vg;C id;I p,q;V*v;
 ARGCHK1(w); STACKCHKOFL  // make sure we don't have a recursion loop through inv
 ASSERT(VERB&AT(w),EVDOMAIN); 
 id=ID(w); v=FAV(w);  // id=pseudochar for w, v->verb info
#define simpleinvvalues(w) CCM(w,CDIV)+CCM(w,CPLUS)+CCM(w,CMINUS)+CCM(w,CLEFT)+CCM(w,CRIGHT)+CCM(w,CREV)+CCM(w,CCANT)+CCM(w,CPOLY)+ \
 CCM(w,CNOT)+CCM(w,CGRADE)+CCM(w,CCYCLE)+CCM(w,CDOMINO)+CCM(w,COPE)+CCM(w,CLOG)+CCM(w,CBOX)+CCM(w,CEXP)+ \
 CCM(w,CGE)+CCM(w,CLE)+CCM(w,CHALVE)+CCM(w,CPLUSCO)+CCM(w,CSQRT)+CCM(w,CSTARCO)+CCM(w,CHEAD)+CCM(w,CLAMIN)+ \
 CCM(w,CABASE)+CCM(w,CBASE)+CCM(w,CTHORN)+CCM(w,CEXEC)
 CCMWDS(simpleinv) CCMCAND(simpleinv,cand,id) if(CCMTST(cand,id))R ds(simpleinv[id&127]);
 // in case id indicates a modifier, set (f|g) to the operand, n? if it is a noun or name, v? if it is a verb
 f=v->fgh[0]; nf=f&&AT(f)&NOUN+NAME; vf=f&&!nf;
 g=v->fgh[1]; ng=g&&AT(g)&NOUN+NAME; vg=g&&!ng;
 switch(id){
  case CCIRCLE:  R eval("1p_1&*");
  case CJDOT:    R eval("0j_1&*");
  case CRDOT:    R eval("%&0j1@^.");
  case CPLUSDOT: R eval("j./\"1\"_ :. +.");
  case CSTARDOT: R eval("r./\"1\"_ :. *.");
  case CDGRADE:  R eval("/:@|.");
  case CWORDS:   R eval("}:@;@(,&' '&.>\"1) :. ;:");
  case CBANG:    R eval("3 : '(-(y -~ !)%0.001&* (0.001%~[:-/[:! 0.001 0 +/ ]) ])^:_<.&170^:(-:+)^.y' :. !");
xco:
  case CXCO:     R amp(num(-1),w);
  case CSPARSE:  R fdef(0,CPOWOP,VERB,jtdenseit,jtvalenceerr, w,num(-1),0L, VFLAGNONE, RMAX,RMAX,RMAX);
  case CPCO:     R fdef(0,CPOWOP,VERB,jtplt,    jtvalenceerr, w,num(-1),0L, 0L, 0L,  0L,  0L  );
  case CQCO:     R eval("*/");
  case CUCO:     R amp(num(3),w);
  case CUNDER:   R under(invrecur(f),g);
  case CFORK:    R invfork(w);
  case CAMP:     if(nf!=ng){R invamp(w);}  // fall through if not m&v or u&n
  case CAT:      if(vf&&vg)R atop(invrecur(g),invrecur(f));   break;
  case CAMPCO:
  case CATCO:    if(vf&&vg)R atco(invrecur(g),invrecur(f));   break;
  case CSLASH:   if(CSTAR==ID(f))R ds(CQCO);                  break;
  case CQQ:      if(vf)R qq(invrecur(f),g);                   break;
  case COBVERSE: if(vf&&vg)R obverse(g,f);                    break;  // if defined obverse, return it
  case CSCO:     R amp(num(5),w);
  case CFIT:     if(CXCO==ID(f))goto xco; //_1 x:!.n y is the same as _1 x: y, and n was already verified to be valid, so treat this the same as plain x:
  case CPOWOP:   
   if(vf&&ng){RE(p=i0(g)); R -1==p?f:1==p?invrecur(f):powop(0>p?f:invrecur(f),sc(ABS(p)),0);}
   if(VGERL&v->flag)R AAV(v->fgh[2])[1];
   break;
  case CTILDE:
   if(nf)R invrecur(symbrd(f));  // name~ - resolve name & try again
   switch(ID(f)){   // inverses for reflexive monads
    case CPLUS:  R ds(CHALVE);
    case CSTAR:  R ds(CSQRT);
    case CJDOT:  R eval("0.5j_0.5&*");
    case CLAMIN: R eval("{. :. (,:~)");
    case CSEMICO:R eval(">@{. :. (;~)");
    case CCOMMA: R eval("<.@-:@# {. ] :. (,~)");
    case CEXP:   R eval("3 : '(- -&b@(*^.) % >:@^.)^:_ ]1>.b=.^.y' \" 0 :. (^~)");
   }
   break;
  case CBSLASH:
  case CBSDOT:
   if(CSLASH==ID(f)&&(ff=FAV(f)->fgh[0],ff&&VERB&AT(ff))){  //  ff/\  or ff/\.
    b=id==CBSDOT;
    switch(ID(ff)){
     case CPLUS: R obverse(eval(b?"- 1&(|.!.0)":" - |.!.0"),w);
     case CSTAR: R obverse(eval(b?"% 1&(|.!.1)":" % |.!.1"),w);
     case CEQ:   R obverse(eval(b?"= 1&(|.!.1)":" = |.!.1"),w);
     case CNE:   R obverse(eval(b?"~:1&(|.!.0)":" ~:|.!.0"),w);
     case CMINUS:R obverse(eval(b?"+ 1&(|.!.0)":"(- |.!.0) *\"_1 $&1 _1@#"),w);
     case CDIV:  R obverse(eval(b?"* 1&(|.!.1)":"(% |.!.1) ^\"_1 $&1 _1@#"),w);
   }}
   break;
  case CCUT:
   if(CBOX==ID(f)&&ng&&(p=i0(g),1==p||2==p))R fdef(0,CPOWOP,VERB, jtbminv,jtvalenceerr, w,num(-1), 0L,VFLAGNONE, RMAX,RMAX,RMAX);
   break;
  case CIBEAM:
   p=i0(f); q=i0(g);
   if(3==p&&1==q)R foreign(f,num(2));
   if(3==p&&2==q)R foreign(f,num(1)   );
   if(3==p&&3==q)R foreign(f,num(2));
   break;
  case CHOOK:
    if(BOTHEQ8(ID(f),ID(g),CFROM,CEQ))R eval("i.\"1&1");
   break;
 }
 // Failure - no inverse found.  If there are names in w, try fixing w and try on that.
 // But only fix once, at the top recursion level, (1) to avoid an infinite loop if
 // there is a circular reference that leaves names in the fixed form of w; (2) to avoid
 // repeated fixing of lower branches, which will only be tried again when higher levels are fixed
 if(!recur&&!nameless(w))R invrecur(fix(w,zeroionei(0)));
 ASSERT(0,EVDOMAIN);
}

static F1(jtneutral){A x,y;B b;V*v;
 ARGCHK1(w);
 v=FAV(w);
 ASSERT(!v->lrr,EVDOMAIN);
 RZ(y=v2(0L,1L));
 RZ(x=scf(infm)); b=equ(y,CALL2(v->valencefns[1],x,y,w)); RESETERR; if(b)R x;
 x=ainf;          b=equ(y,CALL2(v->valencefns[1],x,y,w)); RESETERR; if(b)R x;
 x=zeroionei(0);          b=equ(y,CALL2(v->valencefns[1],x,y,w)); RESETERR; if(b)R num(0); 
 x=zeroionei(1);           b=equ(y,CALL2(v->valencefns[1],x,y,w)); RESETERR; if(b)R num(1);
 RZ(x=scf(infm)); b=equ(y,CALL2(v->valencefns[1],y,x,w)); RESETERR; if(b)R x;
 x=ainf;          b=equ(y,CALL2(v->valencefns[1],y,x,w)); RESETERR; if(b)R x;
 x=zeroionei(0);          b=equ(y,CALL2(v->valencefns[1],y,x,w)); RESETERR; if(b)R num(0);
 x=zeroionei(1);           b=equ(y,CALL2(v->valencefns[1],y,x,w)); RESETERR; if(b)R num(1);
 ASSERT(0,EVDOMAIN);
}    /* neutral of arbitrary rank-0 function */

F1(jtiden){A f,g,x=0;V*u,*v;
 RZ(w=fix(w,zeroionei(0))); ASSERT(VERB&AT(w),EVDOMAIN);
 v=FAV(w); f=v->fgh[0]; g=v->fgh[1];
 switch(v->id){
  default:      RZ(x=neutral(w)); break;
  case CCOMMA:  R eval("i.@(0&,)@(2&}.)@$");
  case CDOT:    if(!(ip(w,CPLUS,CSTAR)||ip(w,CPLUSDOT,CSTARDOT)||ip(w,CNE,CSTARDOT)))break;  // if matrix multiply, fall through to...
  case CDOMINO: R atop(atop(ds(CEQ),ds(CGRADE)),ds(CHEAD));
  case CCYCLE:
  case CLBRACE: R atop(ds(CGRADE),ds(CHEAD));
  case CSLASH:  if(VERB&AT(f))R atop(iden(f),ds(CPOUND)); break;
  case CPLUS: case CMINUS: case CSTILE:   case CNE:
  case CGT:   case CLT:    case CPLUSDOT: case CJDOT:   case CRDOT:
                x=num(0); break;
  case CSTAR: case CDIV:   case CEXP:     case CROOT:   case CBANG:
  case CEQ:   case CGE:    case CLE:      case CSTARDOT:
                x=num(1); break;
  case CMAX:    x=scf(infm); break;
  case CMIN:    x=ainf; break;
  case CUNDER:  ; A t; df1(x,df1(t,mtv,iden(f)),inv(g)); break;
  case CAT:
   if(CAMP==ID(f)&&(u=FAV(f),NOUN&AT(u->fgh[0])&&!AR(u->fgh[0])&&CSTILE==ID(u->fgh[1])))switch(ID(g)){
    case CSTAR: case CEXP: x=num(1);  break;
    case CPLUS:            x=num(0);
   }
   break;
  case CBDOT: ;  // canned inverses for (bt b.)
   I bt=i0(g);
#define INVM1 BMK(25)
#define INV0 (BMK(2)+BMK(4)+BMK(5)+BMK(6)+BMK(7)+BMK(18)+BMK(20)+BMK(21)+BMK(22)+BMK(23))
#define INV1 (BMK(1)+BMK(9)+BMK(11)+BMK(13)+BMK(17)+BMK(27)+BMK(29))
   if(bt<32&&((INVM1|INV0|INV1)&(1LL<<bt))){I bi;
    bi=-1; bi=INV0&(1LL<<bt)?0:bi; bi=INV1&(1LL<<bt)?1:bi;
    x=num(bi);
   }
   break;
  }
 ASSERT(x!=0,EVDOMAIN);
 R folk(x,swap(ds(CDOLLAR)),atop(ds(CBEHEAD),ds(CDOLLAR)));
}

F1(jtidensb){A x=0,w0=w;V*v;
 RZ(w=fix(w,zeroionei(0))); ASSERT(VERB&AT(w),EVDOMAIN);
 v=FAV(w);
 switch(v->id){
  default:      R iden(w0);
  case CMAX:    GATV0(x,SBT,1,0);SBAV(x)[0]=0; break;
// no max symbol  case CMIN:    GATV0(x,SBT,1,0);SBAV(x)[0]=SBUV4(JT(jt,sbu))[0].down; break;
 }
 ASSERT(x!=0,EVDOMAIN);
 R folk(x,swap(ds(CDOLLAR)),atop(ds(CBEHEAD),ds(CDOLLAR)));
}
