/* Copyright (c) 1990-2023, Jsoftware Inc.  All rights reserved.           */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Conversions Amongst Internal Types                                      */

#include "j.h"
#include "vcomp.h"

#define KF1(f)          B f(J jt,A w,void*yv)
#define KF1F(f)         B f(J jt,A w,void*yv,D fuzz)  // calls that use optional fuzz
#define KF2(f)          B f(J jt,A w,void*yv,I mode)
#define CVCASE(a,b)     (6*((0x28c>>(a))&7)+((0x28c>>(b))&7))   // Must distinguish 0 2 3 4 6 7->4 3 1 0 2 5  01010001100
#define CVCASECHAR(a,b) ((3*(0x40000>>(a))+(0x40000>>(b)))&07)  // distinguish character cases - note last case is impossible (equal types)

static KF1(jtC1fromC2){UC*x;US c,*v;
 v=USAV(w); x=(C*)yv;
 DQ(AN(w), c=*v++; if(!(256>c))R 0; *x++=(UC)c;);
 R 1;
}

static KF1(jtC2fromC1){UC*v;US*x;
 v=UAV(w); x=(US*)yv;
 DQ(AN(w), *x++=*v++;);
 R 1;
}

static KF1(jtC1fromC4){UC*x;C4 c,*v;
 v=C4AV(w); x=(C*)yv;
 DQ(AN(w), c=*v++; if(!(256>c))R 0; *x++=(UC)c;);
 R 1;
}

static KF1(jtC2fromC4){US*x;C4 c,*v;
 v=C4AV(w); x=(US*)yv;
 DQ(AN(w), c=*v++; if(!(65536>c))R 0; *x++=(US)c;);
 R 1;
}

static KF1(jtC4fromC1){UC*v;C4*x;
 v=UAV(w); x=(C4*)yv;
 DQ(AN(w), *x++=*v++;);
 R 1;
}

static KF1(jtC4fromC2){US*v;C4*x;
 v=USAV(w); x=(C4*)yv;
 DQ(AN(w), *x++=*v++;);
 R 1;
}

typedef struct {I a; I b; I c; I d;} int4;
static __attribute__((aligned(CACHELINESIZE))) int4 Ifours[16] = {
{0,0,0,0}, {1,0,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,1,0}, {1,0,1,0}, {0,1,1,0}, {1,1,1,0}, {0,0,0,1}, {1,0,0,1}, {0,1,0,1}, {1,1,0,1}, {0,0,1,1}, {1,0,1,1}, {0,1,1,1}, {1,1,1,1},
};

// conversion of bytes, with overfetch
static KF1(jtIfromB){
 int4 sink;  // we divert stores to here
 I n=AN(w); UI *wv=UIAV(w); int4 *zv=yv;
 UI b8;  // 8 bits
 while(1){
  b8=*wv++;  // fetch next 8 bits
  if((n-=SZI)<=0)break;  // exit loop with b8 set if not >8 slots left
  b8|=b8>>7; b8|=b8>>14;  // now bits 0-3 and 32-35 have 4 binary values
  b8&=(UI)0xf0000000f;  // remove garb.
  *zv++=Ifours[b8&0xf];  // move 4 Is
#if SY_64
  *zv++=Ifours[b8>>32];  // move 4 Is
#endif
 }
 // we have moved all the full 8-bit sections; handle the last one, 1-8 bytes.  n is -7 to 0
 b8&=VALIDBOOLEAN; b8|=b8>>7; b8|=b8>>14;  // last section may have garbage; clear it.  now bits 0-3 and 32-35 have 4 binary values
 b8&=(UI)0xf0000000f;  // remove garb.
 I *zvI=(I*)zv;  // take output address before we divert it
 n+=SZI-1;  // n is 0 to 7, # words to move-1
#if SY_64
 // if there is a full 4-bit section, copy it and advance to remnant.  n must be -7 to -4
 zv=n&4?zv:&sink;  // if moving only 1-4 words, divert this
 *zv=Ifours[b8&0xf];  // move the 4-bit section
 zvI+=n&4;  // advance word output pointer if we copied the 4-bit section
 b8>>=(n&4)<<LGBB;  // if we wrote the bits out, shift in the new ones
 n&=4-1;  // get length of remnant-1, 0-3
#endif
 // move the remnant, 1-4 bytes
 *zvI++=b8&1; zvI=n<1?&jt->shapesink[0]:zvI; *zvI++=(b8>>=1)&1;  zvI=n<2?&jt->shapesink[0]:zvI; *zvI++=(b8>>=1)&1; zvI=n<3?&jt->shapesink[0]:zvI; *zvI=(b8>>=1)&1;
 R 1;
}

typedef struct {D a; D b; D c; D d;} dbl4;
static __attribute__((aligned(CACHELINESIZE))) dbl4 Dfours[16] = {
{0,0,0,0}, {1,0,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,1,0}, {1,0,1,0}, {0,1,1,0}, {1,1,1,0}, {0,0,0,1}, {1,0,0,1}, {0,1,0,1}, {1,1,0,1}, {0,0,1,1}, {1,0,1,1}, {0,1,1,1}, {1,1,1,1},
};

// conversion of bytes, with overfetch
static KF1(jtDfromB){
 dbl4 sink;  // we divert stores to here
 I n=AN(w); UI *wv=UIAV(w); dbl4 *zv=yv;
 UI b8;  // 8 bits
 while(1){
  b8=*wv++;  // fetch next 8 bits
  if((n-=SZI)<=0)break;  // exit loop with b8 set if not 4 slots left
  b8|=b8>>7; b8|=b8>>14;  // now bits 0-3 and 32-35 have 4 binary values
  b8&=(UI)0xf0000000f;  // remove garb.
  *zv++=Dfours[b8&0xf];  // move 4 Is
#if SY_64
  *zv++=Dfours[b8>>32];  // move 4 Is
#endif
 }
 // we have moved all the full 8-bit sections; handle the last one, 1-8 bytes.  n is -7 to 0
 b8&=VALIDBOOLEAN; b8|=b8>>7; b8|=b8>>14;  // last section may have garbage; clear it.  now bits 0-3 and 32-35 have 4 binary values
 b8&=(UI)0xf0000000f;  // remove garb.
 D *zvD=(D*)zv;  // take output address before we divert it
 n+=SZI-1;  // n is 0 to 7, # words to move-1
#if SY_64
 // if there is a full 4-bit section, copy it and advance to remnant.  n must be -7 to -4
 zv=n&4?zv:&sink;  // if moving only 1-4 words, divert this
 *zv=Dfours[b8&0xf];  // move the 4-bit section
 zvD+=n&4;  // advance word output pointer if we copied the 4-bit section
 b8>>=(n&4)<<LGBB;  // if we wrote the bits out, shift in the new ones
 n&=4-1;  // get length of remnant-1, 0-3
#endif
 // move the remnant, 1-4 bytes
 *zvD++=b8&1; zvD=n<1?(D*)&jt->shapesink[0]:zvD; *zvD++=(b8>>=1)&1;  zvD=n<2?(D*)&jt->shapesink[0]:zvD; *zvD++=(b8>>=1)&1; zvD=n<3?(D*)&jt->shapesink[0]:zvD; *zvD=(b8>>=1)&1;
 R 1;
}

#if SY_64 && (C_AVX2 || EMU_AVX2)
static KF1(jtDfromI){I n=AN(w); D *x=DAV(w); D *z=yv;
/* Optimized full range int64_t to double conversion           */
/* Emulate _mm256_cvtepi64_pd()                                */
// Tip o'hat to wim and Peter Cordes on stackoverflow
 AVXATOMLOOP(0,
  CVTEPI64DECLS
 ,
  CVTEPI64(u,u)
 ,
  R 1;
 )
}
#else
static KF1(jtDfromI){I n=AN(w); I *x=IAV(w); D *z=yv;
DQ(n, *z++=(D)*x++;)
R 1;
}
#endif



static KF1(jtBfromI){B*x;I n,p,*v;
 n=AN(w); v=AV(w); x=(B*)yv;
 DQ(n, p=*v++; *x++=(B)p; if(p&-2)R 0;);
 R 1;
}

static KF1F(jtBfromD){B*x;D p,*v;I n;
 n=AN(w); v=DAV(w); x=(B*)yv;
 DQ(n, p=*v++; if(p<-2||2<p)R 0;   // handle infinities
  I val=2; val=(p==0)?0:val; val=FIEQ(p,1.0,fuzz)?1:val; if(val==2)R 0; *x++=(B)val; )
 R 1;
}

static KF1F(jtIfromD){D p,q,*v;I i,k=0,n,*x;
 n=AN(w); v=DAV(w); x=(I*)yv;
#if SY_64
 for(i=0;i<n;++i){
  p=v[i]; q=jround(p);
  if(!ISFTOIOKFZ(p,q,fuzz))R 0;  // error if value not tolerably integral
  *x++=(I)q;  // if tolerantly integral, store it
 }
#else
 q=IMIN*(1+fuzz); D r=IMAX*(1+fuzz);
 DO(n, p=v[i]; if(p<q||r<p)R 0;);
 for(i=0;i<n;++i){
  p=v[i]; q=jfloor(p);
  if(FIEQ(p,q,fuzz))*x++=(I)q; else if(FIEQ(p,1+q,fuzz))*x++=(I)(1+q); else R 0;
 }
#endif
 R 1;
}

static KF1F(jtDfromZ){D d,*x;I n;Z*v;
 n=AN(w); v=ZAV(w); x=(D*)yv;
 if(fuzz)DQ(n, d=ABS(v->im); if(d!=inf&&d<=fuzz*ABS(v->re)){*x++=v->re; v++;} else R 0;)
 else        DQ(n, d=    v->im ; if(!d                            ){*x++=v->re; v++;} else R 0;);
 R 1;
}

static KF1(jtXfromB){ // array content yv as X from array w of B
 GMP; X*y= (X*)yv; I wn= AN(w); B*wv= BAV(w);
 DO(wn, y[i]= wv[i] ?X1 :X0;)
 R !jt->jerr;
}

static KF1(jtXfromI){ // array content yv as X from array w of I
 GMP; X*y= (X*)yv; I wn= AN(w), *wv= AV(w);
 DO(wn, I wi= wv[i];
	 if (unlikely(!wi)) y[i]= X0;
  else if (unlikely(1==wi)) y[i]= X1;
  else if (unlikely(-1==wi)) y[i]= X_1;
  else y[i]= XgetI(wi);
 );
 R !jt->jerr;
}

static X jtxd1(J jt, D p, I mode) {PROLOG(0052);A t;D d,e=tfloor(p),q,r;I m,*u;
 switch(mode){
  case XMFLR:   p=e;                            break;
  case XMCEIL:  p=jceil(p);                     break;
  case XMEXACT: ASSERT(TEQ(p,e),EVDOMAIN); p=e; break;
  case XMEXMT:  if(!TEQ(p,e))R vec(INT,0L,&iotavec[-IOTAVECBEGIN]);
 }
 ASSERT(p!= inf, EWRAT);
 ASSERT(p!=-inf, EWRAT);
 GEMP0; mpz_t mpz; jmpz_init_set_d(mpz, p); X z= Xmp(z);
 EPILOG(z); 
}

static KF2(jtXfromD){ // array content yv as X from array w of D
 GMP; X*y= (X*)yv; I wn= AN(w); D*wv= DAV(w);
 DO(wn, D wvi= wv[i]; ASSERT(!(inf==wvi||infm==wvi),EWIRR); y[i]= xd1(wvi, mode););
 R !jt->jerr;
}

static KF1(jtBfromX){ // array content yv as B from array w of X
 B*y= yv; I wn= AN(w); X*wv= XAV(w); 
 DO(wn, X W= wv[i];
  if (0>XSGN(W)) R0;
  if (1<XSGN(W)) R0;
  if (0==XSGN(W)) *y++= 0;
  else if (1==XLIMB0(W)) *y++= 1;
  else R0;
 );
 R 1;
}

static KF1(jtIfromX){
 I *y=yv; X*wv=XAV(w);
 DO(AN(w), X W= wv[i];
  if (1==XSGN(W)) // w[i] is positive
   if (IMAX<XLIMB0(W)) R 0; else *y++= XLIMB0(W);
  else if (-1==XSGN(W)) // w[i] is negative
   if ((UI)IMIN<XLIMB0(W)) R 0; else *y++= -XLIMB0(W);
// -IMIN = IMIN for 2's compliment
// if ((UI)-IMIN<XLIMB0(W)) R 0; else *y++= -XLIMB0(W);
  else if (0==XSGN(W)) *y++= 0; // w[i] is 0
  else R0; // w[i] is too big
 )
 R 1;
}

static KF1(jtDfromX){
 D*y=yv; X*wv=XAV(w);
 DQ(AN(w), X W=*wv++; mpX(W); *y++= jmpz_get_d(mpW););
 R 1;
}

static KF1(jtQfromX){X*v=XAV(w),*x=(X*)yv; DQ(AN(w), *x++=*v++; *x++=X1;); R 1;}

static KF2(jtQfromD){
 GMP; if(!(w))R 0; /* FIXME: why do we need this here, but not other conversion routines? */
 I n=AN(w), i; D*wv=DAV(w), t; Q*x=(Q*)yv, q; S*tv=3*C_LE+(S*)&t;
 for(i=0;i<n;++i){
  t=wv[i]; q.d= X1;
  ASSERT(!_isnan(t),EVNAN);
  B neg; if(neg=0>t)t=-t;
  if     (t==inf) q= Q_;
  else if(t==0.0) q= Q0;
  else if(1.0==jt->cct && XMEXACT==mode){
   // x:!.0 - cut no corners
   I e=(I)(0xfff0&*tv); e>>=4; e-=1023;   // exponent, with bias removed
   unsigned long long num=((*(unsigned long long *)&t)&0xfffffffffffffLL)+0x10000000000000;  // numerator with hidden bit restored
#if SY_64
   q.n=XgetI(num);
#else /* SY_32 */
   I qnlo= num&0xffffffff;
   I qnhi= (num>>32)+(qnlo<0);
   q.n= XaddXX(XgetI(qnlo), XmulXX(XgetD(4294967296.0), XgetI(qnhi)));
#endif
   if(e<=52)q.d=xpow(xc(2L),xc(52-e));  // for 1.0, num is 1 in bit 52, e is 0, thus denom=2^52
   else{q.d=X1; q.n=xtymes(q.n,xpow(xc(2L),xc(e-52)));}  // large exponents need big numerators
   q=qstd(q);
  }else if(1.1102230246251565e-16<t&&t<9.007199254740992e15){
   D d=jround(1/dgcd(1.0,t)), c=jround(d*t); 
   q.n=xd1(c,mode); q.d=xd1(d,mode); q=qstd(q);
  }else{
   B recip; if(recip=1>t)t=1.0/t;
   I e=(I)(0xfff0&*tv); e>>=4; e-=1023;
   if(recip){q.d=xtymes(xd1(t/pow(2.0,e-53.0),mode),xpow(xc(2L),xc(e-53))); q.n=X1;}
   else     {q.n=xtymes(xd1(t/pow(2.0,e-53.0),mode),xpow(xc(2L),xc(e-53))); q.d=X1;}
  }
  q.n=rifvsdebug(q.n); q.d=rifvsdebug(q.d);
  if(neg) {
   if (ACISPERM(AC(q.n))) {
    if (ISX1(q.n)) q.n= X_1;
    else if (ISX_1(q.n)) q.n= X1;
    else if (ISX0(q.n)) q.n= X0;
    else SEGFAULT; // this should never happen
   } else QSGN(q)= -QSGN(q);
  }
  *x++=q;
 }
 R !jt->jerr;
}

/*
// not accurate enough for J's test suite:
static KF1(jtDfromQ){
 D*x= yv; Q*wv=QAV(w);
 DQ(AN(w), Q W= *wv++;
  if (ISQinf(W)) *x++= 0<QSGN(W) ?inf :infm;
  else *x++= ({mpQ(W); jmpq_get_d(mpW);});
 ); 
 R 1;
}
*/

static KF1(jtDfromQ){
 Q*wv=QAV(w); D*x=(D*)yv; I wn=AN(w);
 // mp_limb_t is too large for us to represent it's "base" directly
 // so: compromise: we represent the square root of that value, and take half steps.
 const I hb= 4*sizeof (mp_limb_t), hba= 1LL<<hb, L= ~-hba;
 for(I i=0;i<wn;++i){
  Q r= wv[i];
  if (ISQinf(r))x[i]= QSGN(r)>0 ?inf :infm;
  else {
 // J double (64 bit binary IEEE 754 floating point) can represent
 // 2^1023 and 2^-1022, but 2^1024 is infinity and 2^-1023 is 0.
 // So... if we convert the numerator and denominator to type double
 // find their ratio, but our intermediate values are _ or 0 we fail.
 // To work around this issue, if either are "too large" we'll first
 // scale down to fit within range (then scaling the result).
   I szn= IbitsX(r.n), szd= IbitsX(r.d), lim= 1023LL-(szd>szn);
   I e= 0, pn= szn-lim, pd= szd-lim;
   if (pn>0) {e+= pn; r.n= Xfdiv_qXX(r.n, XpowUU(2,pn));}
   if (pd>0) {e-= pd; r.d= Xfdiv_qXX(r.d, XpowUU(2,pd));}
   D n=0.0, f=1.0; UI*v=UIAV1(r.n); DO(XLIMBLEN(r.n), UI u= v[i]; n+=f*(L&u); f*=hba; n+=f*(u>>hb); f*=hba;)
   D d=0.0; f=1.0;    v=UIAV1(r.d); DO(XLIMBLEN(r.d), UI u= v[i]; d+=f*(L&u); f*=hba; d+=f*(u>>hb); f*=hba;)
   D xi= n/d;
   if (e) {
    D ex= 1.0; DQ(llabs(e), ex*=2;)
    xi= e>0 ?xi*ex :xi/ex;
   }
   x[i]= (QSGN(r)>0 ?1 :-1)*xi;
 }}
 R 1;
}

static KF1(jtXfromQ){
 GMP; X*x=yv; Q*v= QAV(w);
 DQ(AN(w), if (!ISQINT(*v)) R0;; *x++= v++->n;);
 R 1;
}

static KF1(jtZfromD){
 D *wv=DAV(w); Z *zv=yv; DQ(AN(w), zv->im=0.0; zv++->re=*wv++;) R 1;
}

// Convert the data in w to the type t.  w and t must be noun types.  A new buffer is always created (with a
// copy of the data if w is already of the right type), and returned in *y.  Result is
// 0 if error, 1 if success.  If the conversion loses precision, error is returned
// Calls through bcvt are tagged with a flag in jt, indicating to set fuzz=0
// For sparse arguments, this function calls other J verbs and must clear the ranks in that case only
// kludge this interface sucks - why not return the A block of the result?  We could expunge jtcvt() 
B jtccvt(J jt,I tflagged,A w,A*y){F1PREFIP;A d;I n,r,*s,wt; void *wv,*yv;I t=tflagged&(NOUN|SPARSE);
 ARGCHK1(w);
 r=AR(w); s=AS(w); wt=AT(w); n=AN(w);
 if(unlikely(((t|wt)&SPARSE+BOX+SBT+FUNC)!=0)){
  // Unusual cases: box, ticker, sparse, func (should not occur).
  ASSERTSYS(!((t|wt)&FUNC),"non-noun in cvt");
  // Handle sparse
  if(likely(ISSPARSE(t|wt))){
   RANK2T oqr=jt->ranks; RESETRANK; 
   switch((ISSPARSE(t)?2:0)+(ISSPARSE(AT(w))?1:0)){I t1;P*wp,*yp;
   case 1: RZ(w=denseit(w)); break;  // sparse to dense
   case 2: RZ(*y=sparseit(cvt(DTYPE(t),w),IX(r),cvt(DTYPE(t),num(0)))); jt->ranks=oqr; R 1;  // dense to sparse; convert type first (even if same dtype)
   case 3: // sparse to sparse
    t1=DTYPE(t);
    GASPARSE(*y,t,1,r,s); yp=PAV(*y); wp=PAV(w);
    SPB(yp,a,ca(SPA(wp,a)));
    SPB(yp,i,ca(SPA(wp,i)));
    SPB(yp,e,cvt(t1,SPA(wp,e)));
    SPB(yp,x,cvt(t1,SPA(wp,x)));
    jt->ranks=oqr; R 1;
   }
   // must be sparse to dense.  Carry on now that w is dense
   jt->ranks=oqr;
  }else{
   // conversion of BOXED or SBT.  Types better be equal
   ASSERT(n==0||TYPESEQ(t,wt),EVDOMAIN)
   // fall through to make clone
  }
 }
 // Now known to be non-sparse, and numeric or literal except when empty or BOX or SBT not being changed
 // If type is already correct, return a clone - used to force a copy.  Should get rid of this kludge
 if(unlikely(TYPESEQ(t,AT(w)))){RZ(*y=ca(w)); R 1;}
 // else if(n&&t&JCHAR){ASSERT(HOMO(t,wt),EVDOMAIN); RZ(*y=uco1(w)); R 1;}
 // Kludge on behalf of result assembly: we want to be able to stop converting after the valid cells.  If NOUNCVTVALIDCT is set in the type,
 // we use the input *y as as override on the # cells to convert.  We use it to replace n (for use here) and yv, and AK(w) and AN(w) for the subroutines.
 // If NOUNCVTVALIDCT is set, w is modified: the caller must restore AN(w) and AK(w) if it needs it
 // TODO: same-length conversion could be done in place
 GA(d,t,n,r,s); yv=voidAV(d);  // allocate the same # atoms, even if we will convert fewer
 if(unlikely(tflagged&NOUNCVTVALIDCT)){
  I inputn=*(I*)y;  // fetch input, in case it is called for
  if(inputn>0){  // if converting the leading values, just update the counts
   n=inputn;  // set the counts for local use, and in the block to be converted
  }else{  // if converting trailing values...
   I offset=(n+inputn)<<bplg(t);  // byte offset to start of data
   AK(w)+=(n+inputn)<<bplg(wt); yv=(I*)((C*)yv+((n+inputn)<<bplg(t)));  // advance input and output pointers to new area
   n=-inputn;  // get positive # atoms to convert
  }
  AN(w)=n;  // change atomct of w to # atoms to convert
 }
 // If n and AN have been modified, it doesn't matter for rank-1 arguments whether the shape of the result is listed as n or s[0] since only n atoms will
 // be used.  For higher ranks, we need the shape from s.  So it's just as well that we take the shape from s now
 *y=d;  wv=voidAV(w); // return the address of the new block
 if(unlikely(!n))R 1;
 // Perform the conversion based on data types
 // For branch-table efficiency, we split the literal conversions into one block, and
 // the rest in another
 if(unlikely((t|wt)&(LIT+C2T+C4T+SBT+XD+XZ))) {   // there are no SBT+XD+XZ conversions, but we have to show domain error
   // one of the types is literal.
   // we must account for all NOUN types.  If there is a non-char, that's an error
  ASSERT(!((t|wt)&(SBT+XD+XZ+NUMERIC+BOX)),EVDOMAIN);  // No conversions for these types
  switch (CVCASECHAR(CTTZ(t),CTTZ(wt))){
   case CVCASECHAR(LITX, C2TX): R C1fromC2(w, yv);
   case CVCASECHAR(LITX, C4TX): R C1fromC4(w, yv);
   case CVCASECHAR(C2TX, LITX): R C2fromC1(w, yv);
   case CVCASECHAR(C2TX, C4TX): R C2fromC4(w, yv);
   case CVCASECHAR(C4TX, LITX): R C4fromC1(w, yv);
   case CVCASECHAR(C4TX, C2TX): R C4fromC2(w, yv);
   default:                ASSERT(0, EVDOMAIN);
  }
 }
 // types here must both be among B01 INT FL CMPX XNUM RAT
 switch (CVCASE(CTTZ(t),CTTZ(wt))){
  case CVCASE(INTX, B01X): R jtIfromB(jt, w, yv);
  case CVCASE(XNUMX, B01X): R XfromB(w, yv);
  case CVCASE(RATX, B01X): GATV(d, XNUM, n, r, s); R XfromB(w, AV(d)) && QfromX(d, yv);
  case CVCASE(FLX, B01X): R jtDfromB(jt, w, yv);
  case CVCASE(CMPXX, B01X): {Z*x = (Z*)yv; B*v = (B*)wv; DQ(n, x->im=0.0; x++->re = *v++;); } R 1;
  case CVCASE(B01X, INTX): R BfromI(w, yv);
  case CVCASE(XNUMX, INTX): R XfromI(w, yv);
  case CVCASE(RATX, INTX): GATV(d, XNUM, n, r, s); R XfromI(w, AV(d)) && QfromX(d, yv);
  case CVCASE(FLX, INTX): R jtDfromI(jt, w, yv);
  case CVCASE(CMPXX, INTX): {Z*x = (Z*)yv; I*v = wv; DQ(n, x->im=0.0; x++->re = (D)*v++;); } R 1;
  case CVCASE(B01X, FLX): R BfromD(w, yv, (I)jtinplace&JTNOFUZZ?0.0:FUZZ);
  case CVCASE(INTX, FLX): R IfromD(w, yv, (I)jtinplace&JTNOFUZZ?0.0:FUZZ);
  case CVCASE(XNUMX, FLX): R XfromD(w, yv, (jt->xmode&REPSGN(SGNIFNOT(tflagged,XCVTXNUMORIDEX)))|CVTTOXMODE(tflagged));
  case CVCASE(RATX, FLX): R QfromD(w, yv, (jt->xmode&REPSGN(SGNIFNOT(tflagged,XCVTXNUMORIDEX)))|CVTTOXMODE(tflagged));
  case CVCASE(CMPXX, FLX): R ZfromD(w, yv);
  case CVCASE(B01X, CMPXX): GATV(d, FL, n, r, s); if(!(DfromZ(w, AV(d), (I)jtinplace&JTNOFUZZ?0.0:FUZZ)))R 0; R BfromD(d, yv, (I)jtinplace&JTNOFUZZ?0.0:FUZZ);
  case CVCASE(INTX, CMPXX): GATV(d, FL, n, r, s); if(!(DfromZ(w, AV(d), (I)jtinplace&JTNOFUZZ?0.0:FUZZ)))R 0; R IfromD(d, yv, (I)jtinplace&JTNOFUZZ?0.0:FUZZ);
  case CVCASE(XNUMX, CMPXX): GATV(d, FL, n, r, s); if(!(DfromZ(w, AV(d), (I)jtinplace&JTNOFUZZ?0.0:FUZZ)))R 0; R XfromD(d, yv, (jt->xmode&REPSGN(SGNIFNOT(tflagged,XCVTXNUMORIDEX)))|CVTTOXMODE(tflagged));
  case CVCASE(RATX, CMPXX): GATV(d, FL, n, r, s); if(!(DfromZ(w, AV(d), (I)jtinplace&JTNOFUZZ?0.0:FUZZ)))R 0; R QfromD(d, yv, (jt->xmode&REPSGN(SGNIFNOT(tflagged,XCVTXNUMORIDEX)))|CVTTOXMODE(tflagged));
  case CVCASE(FLX, CMPXX): R DfromZ(w, yv, (I)jtinplace&JTNOFUZZ?0.0:FUZZ);
  case CVCASE(B01X, XNUMX): R BfromX(w, yv);
  case CVCASE(INTX, XNUMX): R IfromX(w, yv);
  case CVCASE(RATX, XNUMX): R QfromX(w, yv);
  case CVCASE(FLX, XNUMX): R DfromX(w, yv);
  case CVCASE(CMPXX, XNUMX): GATV(d, FL, n, r, s); if(!(DfromX(w, AV(d))))R 0; R ZfromD(d, yv);
  case CVCASE(B01X, RATX): GATV(d, XNUM, n, r, s); if(!(XfromQ(w, AV(d))))R 0; R BfromX(d, yv);
  case CVCASE(INTX, RATX): GATV(d, XNUM, n, r, s); if(!(XfromQ(w, AV(d))))R 0; R IfromX(d, yv);
  case CVCASE(XNUMX, RATX): R XfromQ(w, yv);
  case CVCASE(FLX, RATX): R DfromQ(w, yv);
  case CVCASE(CMPXX, RATX): GATV(d, FL, n, r, s); if(!(DfromQ(w, AV(d))))R 0; R ZfromD(d, yv);
  default:                ASSERT(0, EVDOMAIN);
 }
}

// clear rank before calling ccvt - needed for sparse arrays only but returns the block as the result
A jtcvt(J jt,I t,A w){A y;B b; 
 b=ccvt(t,w,&y);
 if(0==b) { // used to be x:_ could be NUMX, now must be RAT
  if(jt->jerr==EWIRR) {RESETERR; b=ccvt(RAT,w,&y);}
  ASSERT(b!=0,EVDOMAIN);
 }
 R y;
}

// Convert numeric type to lowest precision that fits.  Push fuzz/rank onto a stack,
// and use 'exact' and 'no rank' for them.  If mode=0, do not promote XNUM/RAT to fixed-length types.
// If mode bit 1 is set, minimum precision is INT; if mode bit 2 is set, minimum precision is FL; if mode bit 3 is set, minimum precision is CMPX 
// Result is a new buffer, always
A jtbcvt(J jt,C mode,A w){FPREFIP(J); A y,z=w;
 ARGCHK1(w);
#ifdef NANFLAG
 // there may be values (especially b types) that were nominally CMPX but might actually be integers.  Those were
 // stored with the real part being the actual integer value and the imaginary part as the special 'flag' value.  We
 // handle those here.  If all the imaginary parts were flags, we accept all the integer parts and change the type
 // to integer.  If none of the imaginary parts were flags, we leave the input unchanged.  If some were flags, we
 // convert the flagged values to float and keep the result as complex
 if((((AN(w)-1)|(AT(w)&CMPX)-1))>=0){  // not empty AND complex
  I allflag=1, anyflag=0; Z *wv = ZAV(w); DO(AN(w), I isflag=*(I*)&wv[i].im==NANFLAG; allflag&=isflag; anyflag|=isflag;)
  if(anyflag){
   I ipok=SGNIF(jtinplace,JTINPLACEWX) & AC(w);  // both sign bits set (<0) if inplaceable
   if(allflag){
    if(ipok>=0)GATV(z,INT,AN(w),AR(w),AS(w));
    I *zv=IAV(z);  // output area
    DO(AN(w), zv[i]=*(I*)&wv[i].re;)  // copy the results as integers
   }else{
    if(ipok>=0)GATV(z,CMPX,AN(w),AR(w),AS(w));
    Z *zv=ZAV(z);  // output area
    DO(AN(w), if(*(I*)&wv[i].im==NANFLAG){zv[i].re=(D)*(I*)&wv[i].re; zv[i].im=0.0;}else{zv[i]=wv[i];})  // copy floats, and converts any integers back to float
   }
   w=z;  // this result is now eligible for further demotion
  }
 }
#endif
 // for all numerics, try Boolean/int/float in order, stopping when we find one that holds the data
 if(mode&1||!(AT(w)&XNUM+RAT)){  // if we are not stopping at XNUM/RAT
  // To avoid a needless copy, suppress conversion to B01 if type is B01, to INT if type is INT, etc
  // set the NOFUZZ flag in jt to insist on an exact match so we won't lose precision
  jtinplace=(J)((I)jt+JTNOFUZZ);  // demand exact match
  z=!(mode&14)&&jtccvt(jtinplace,B01,w,&y)?y:
    (y=w,ISDENSETYPE(AT(w),INT)||(!(mode&12)&&jtccvt(jtinplace,INT,w,&y)))?y:
    (y=w,ISDENSETYPE(AT(w),FL)||(!(mode&8)&&jtccvt(jtinplace,FL,w,&y)))?y:w;  // convert to enabled modes one by one, stopping when one works
 }
 RNE(z);
}    /* convert to lowest type. 0=mode: don't convert XNUM/RAT to other types */

F1(jticvt){A z;D*v,x;I i,n,*u;
 ARGCHK1(w);
 n=AN(w); v=DAV(w);
 GATV(z,INT,n,AR(w),AS(w)); u=AV(z);
 for(i=0;i<n;++i){
  x=*v++; if(x<IMIN||FLIMAX<=x)R w;  // if conversion will fail, skip it
  *u++=(I)x;
 }
 R z;
}

A jtpcvt(J jt,I t,A w){A y;B b;RANK2T oqr=jt->ranks;
 RESETRANK; b=ccvt(t,w,&y); jt->ranks=oqr;
 R b?y:w;
}    /* convert w to type t, if possible, otherwise just return w */

#if !C_CRC32C
F1(jtcvt0){I n,t;D *u;
 ARGCHK1(w);
 t=AT(w); n=AN(w); 
 if(n&&t&FL+CMPX){
  if(t&CMPX)n+=n; u=DAV(w);
  DQ(n, if(*u==0.0)*u=0.0;        ++u; ); 
 }
 R w;
}    /* convert -0 to 0 in place */
#endif

F1(jtxco1){ARGCHK1(w); ASSERT(!ISSPARSE(AT(w)),EVNONCE); R cvt(AT(w)&B01+INT+XNUM?XNUM:RAT,w);}

F2(jtxco2){A z;B b;I j,n,r,*s,t,*wv,*zu,*zv;
 ARGCHK2(a,w);
 n=AN(w); r=AR(w); t=AT(w);
 ASSERT(!ISSPARSE(t),EVNONCE);
 RE(j=i0(a));
 switch(j){
  case -2: R aslash1(CDIV,w);
  case -1: R bcvt(1,w);
  case  1: R xco1(w);
  case  2: 
   if(!(t&RAT))RZ(w=cvt(RAT,w));
   GATV0(z,XNUM,2*n,r+1); MCISH(AS(z),AS(w),r) AS(z)[r]=2;  // don't overfetch from AS(w)
   MC(AV(z),AV(w),2*n*SZI);
   R z;
  case  3:
   ASSERT(t&XD+XZ,EVDOMAIN);
   b=(~t>>XDX)&1;   // b=NOT XD
   GATV0(z,INT,n<<b,r+b); s=AS(z); if(b)*s++ =2; MCISH(s,AS(w),r);
   zv=AV(z); zu=n+zv; wv=AV(w);
   if(t&XD){DX*v=(DX*)wv;   DQ(n,         *zv++=v->p;);}
   else    {ZX*v=(ZX*)wv,y; DQ(n, y=*v++; *zv++=y.re.p; *zu++=y.im.p;);}
   R z;
  default:
   ASSERT(20<=j,EVDOMAIN);
   GA(z,t&CMPX?XZ:XD,n,r,AS(w));
   if(t&INT){fprintf(stderr,"jtco2 would call DxfI\n"); R 0;}
   ASSERT(0,EVNONCE);
}}

// 9!:23 audit indices
// x is indexes (may be sparse)
// y is type;parms
// type is 0 (normal), 1 (treat complex as 2 separate reals), 2 allow _ as imin/imax
// parms is list of 0-2 elements min,max allowed
// result is rc;index: rc: 0=OK, 1=nonnumeric, 2=nonintegral value 3=out of bounds
// index=index list of failing entry, or empty if rc=0 or 1
F2(jtindaudit){PROLOG(365);
 ARGCHK2(a,w)
 ASSERT(AT(w)&BOX,EVDOMAIN) ASSERT(AR(w)==1,EVRANK) ASSERT(AN(w)==2,EVLENGTH)  // need 2 boxes
 I type; RE(type=i0(C(AAV(w)[0]))); ASSERT(BETWEENC(type,0,1),EVDOMAIN)  // first box must hold integer type 0 or 1
 I min=IMIN, max=IMAX; A mma=C(AAV(w)[1]); ASSERT(AR(mma)<=1,EVRANK) ASSERT(AN(mma)<=2,EVLENGTH) if(!(AT(mma)&INT))RZ(mma=cvt(INT,mma))  // second box must hold 0-2 int values
 if(AN(mma)>=1)min=IAV(mma)[0]; if(AN(mma)>=2)max=IAV(mma)[1];  // extract min/max
 A ind=a, origind=ind; if(ISSPARSE(AT(ind))){ind=SPA(PAV(ind),x);}  // point to the indexes; if sparse, to the unique indexes
 I indt=AT(ind); I indn=AN(ind); if(indn==0)R jlink(zeroionei(0),mtv);  if(!(indt&NUMERIC))R jlink(zeroionei(1),mtv);  // get type & number of indexes; fast return if empty; failure if nonintegral
 if((indt&CMPX)&&type==1){indt=FL|C2T; indn<<=1;}  // for x # y, allow treating a complex as two reals.  C2T is a flag that we did this
 A vind; RZ(vind=jtvirtual(jt,ind,0,0)) AN(vind)=1; // in case conversions are needed, we will step through with a virtual block
 I vstride=bp(indt);  // size of an atom
 I findex;  // failing index if any, total # atoms to check
 A zwk, xwk; if(indt&CMPX)GAT0(zwk,FL,1,0) if(indt&RAT)GAT0(xwk,XNUM,1,0)  // intermediates for the 2-stwp conversions
 I errtype=2;  // the type of audit failure: 2=noninteger, 3=out of bounds
 for(findex=0;findex<indn;AK(vind)+=vstride,++findex){   // loop trying each index
  I ival;   // the integer value of the index
  // fetch to index and convert to integer if needed
  if(indt&INT)ival=IAV(vind)[0];
  else if(indt&B01)ival=BAV(vind)[0];
  else if(indt&FL){I err; if(err=!jtIfromD(jt,vind,&ival,FUZZ)){if(type==2&&DAV(vind)[0]==inf)err=0,ival=IMAX; if(type==2&&DAV(vind)[0]==infm)err=0,ival=IMIN;}; if(err)break;}
  else if(indt&CMPX){if(!jtDfromZ(jt,vind,&DAV(zwk)[0],FUZZ))break; if(!jtIfromD(jt,zwk,&ival,FUZZ))break;}
  else if(indt&XNUM){if(!jtIfromX(jt,vind,&ival))break;}
  else if(indt&RAT){if(!jtXfromQ(jt,vind,&XAV(xwk)[0]))break; if(!jtIfromX(jt,xwk,&ival))break;}
  // value fetched, see if in range
  if(!BETWEENC(ival,min,max)){errtype=3; break;}  // exit loop if index not in range, with appropriate error type
 }
 A z;  // result
 if(findex==indn)z=jlink(zeroionei(0),mtv);  // normal return with no error
 else{
  // the search failed at findex.  Create the index list
  if(indt&C2T)findex>>=1;  // if 2 reals per complex, convert failing index back to complex
  if(!ISSPARSE(AT(origind))){
   // non-sparse
   RZ(z=abase2(shape(ind),sc(findex)))  // result is ($ind) #: findex
  }else{
   // sparse
   RZ(z=shape(origind)); // z=$ind
   P *p=PAV(origind); A sparseaxes=SPA(p,a); A denseaxes=box(box(box(sparseaxes)));  // the list of sparse axes
   A id; RZ(id=abase2(over(zeroionei(0),from(denseaxes,z)),sc(findex)))     // id =.(0 , (<<<2$.ind) { $ind) #: findex  nonsparse indexes are in blocks of fixed size.  convert to block# , index list in block
   A r; RZ(df2(r,from(head(id),SPA(p,i)),z,amend(sparseaxes)))  // r =. (({.id) { 4$.ind) (2$.ind)} z   convert block# to index list of sparse axes, save those axes in result
   RZ(df2(z,behead(id),r,amend(denseaxes))) // z =. (}. id)  (<<<2$.ind)} r   insert index list in block into result
  }
  z=jlink(sc(errtype),z);  // return error type and location
 }
 EPILOG(z);
}
