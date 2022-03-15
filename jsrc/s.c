/* Copyright 1990-2007, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Symbol Table                                                            */

#include "j.h"


/* a symbol table aka locale is a type LX vector                          */
/* the length is odd after deducting the leading info entry                               */
/* zero elements mean unused entry                                         */
/* non-zero elements are indices in the global symbol pool and             */
/*     are head pointers to a linked list                                  */
/* the first element is symbol pool index for locale info                  */

/* the global symbol pool is a type INT matrix                             */
/* the number of columns is symcol=ceiling(sizeof(L)/sizeof(I))            */
/* elements are interpreted per type L (see jtype.h)                       */
/* a linked list of free entries is kept using the next pointer                  */
// JT(jt,symp):     symbol pool array fixed rank 0, holding L entries (each 3 Is (4 in 32-bit))
// SYMORIGIN    symbol pool array base.  First ele is base of free chain
// AM(JT(jt,symp)) symbol table index (monotonically increasing) - not really needed


/* numbered locales:                                                       */
// JT(jt,stnum)   A block, data is hashtable holding symtab pointer or 0.  Fixed rank 0
// AN(JT(jt,stnum)) size of hashtable in entries (each 1 L*)
// AS(JT(jt,stnum))[1] next loc# to allocate
// AM(JT(jt,stnum)) number of entries in use in table

/* named locales:                                                          */
/* JT(jt,stloc):  locales symbol table                                        */

// within private locales, AK points to the global symbol table when the entity started, and AM is a chain field
// to the local table in the calling explicit definition
//
// within named/numbered locales, AK points to the path (a list of boxes, using AAV0, pointing to the SYMB blocks of the path)
// and AM is the Bloom filter for the locale.


#define symcol ((sizeof(L)+SZI-1)/SZI)

B jtsymext(J jt,B b){A x,y;I j,m,n,*v,xn,yn;L*u;
 if(b){y=(A)((I)SYMORIGIN-AKXR(0)); j=allosize(y)+NORMAH*SZI; yn=AN(y); n=yn/symcol;}  // .  Get header addr by backing off offset of LAV0; extract allo size from header (approx)  yn=#Is in old allo
 else {            j=((I)1)<<12;                  yn=0; n=1;   }  // n is # rows in chain base + old values
 m=j<<1;                     // new size in bytes - 2 * old size
 m-=AKXR(0);                // m is now amount to allo to keep total byte size indicated by j<<1
 m/=symcol*SZI;              // round m to # LX entries we can fit
 xn=m*symcol;             // xn=#Is to allo
 GATV0(x,INT,xn,0); v=(I*)LAV0(x);    // allo the array; v->new symbol 0
 if(b)ICPY(v,LAV0(y),yn);             // if extension, copy old data to new block
 mvc(SZI*(xn-yn),v+yn,1,MEMSET00);               /* 0 unused area for safety  kludge  */
 // dice the added area into symbols, chain them together, add to free chain
 u=n+(L*)v; j=1+n;    // u->start of new area  j=sym# of (1st new sym+1), will always chain each symbol to the next
 DQ(m-n-1, u++->next=(LX)(j++););    // for each new symbol except the last, install chain.  Leave last chain 0
 if(b)u->next=SYMGLOBALROOT;             // if there is an old chain, transfer it to the end of the new chain
 ACINITZAP(x); SYMORIGIN=LAV0(x);           // preserve new array and switch to it
// obsolete  ((L*)v)[0].next=(LX)n;                           /* new base of free chain               */
 SYMGLOBALROOT=(LX)n;  // start the new free chain with the first added ele
 if(b)fa(y);                                /* release old array           */
 R 1;
}    /* 0: initialize (no old array); 1: extend old array */

// Make sure there are n symbols available for allocation.  Extend the symbol table if not.
// This must be called outside of any lock and only when the local free-symbol queue has fewer than n values
I jtreservesym(J jt,I n){
 // loop till we get the number of symbols required
 I nsymadded=0;  // number of symbols we have added.  We ignore any symbols we actually have, since there can't be many of them
 while(1){
  // count off symbols from the global area, up to a fair number (we need to get enough to justify the lock overhead)
  I ninlock;
  WRITELOCK(JT(jt,symlock))
  L *sympv=SYMORIGIN; LX sprev;    // start of symbol block, sym# of a symbol in the chain, starts at the symbol holding SYMGLOBALROOT
  NOUNROLL for(ninlock=0, sprev=0;ninlock<100&&SYMNEXT(sympv[sprev].next);++ninlock,sprev=SYMNEXT(sympv[sprev].next));  // ninlock counts symbols; at end sprev points to a valid one (unless chain is empty)
  if(ninlock!=0){  // if the global chain is not empty...
   // transfer what we got to our local table
   LX localhead=SYMNEXT(SYMLOCALROOT);   // start of the local chain
   SYMLOCALROOT=SYMGLOBALROOT;   // make the new symbols the head of the local chain
   SYMGLOBALROOT=SYMNEXT(sympv[sprev].next);  // restore the rest of the global symbols to the global chain
   sympv[sprev].next=localhead;  // restore our preexisting local symbols to the local chain
  }
  WRITEUNLOCK(JT(jt,symlock))
  // if we didn't get enough, call a system lock and extend/relocate the table
  if((nsymadded+=ninlock)>=n)break;  // incr total symbols added; success if we got enough
  if(jtsystemlock(jt)){I extok=symext(1); jtsystemunlock(jt); RZ(extok)}  // extend symbol table under the big lock
 }
// obsolete  RZ(symext(1)); /* extend pool if req'd        */
// obsolete  SYMRESERVE(n)   // check to make sure we got enough 
 R 1;
}

// hv->hashtable slot; allocate new symbol, install as head/tail of hash chain, with previous chain appended
// if SYMNEXT(tailx)==0, append at head (immediately after *hv); if SYMNEXT(tailx)!=0, append after tailx.  If queue is empty, tailx is always 0
// The stored chain pointer to the new record is given the non-PERMANENT status from the sign of tailx
// result is new symbol
// Caller must ensure, by prior use of SYMRESERVE, that the symbol is available
L* jtsymnew(J jt,LX*hv, LX tailx){LX j;L*u,*v;
// obsolete  SYMRESERVE(1)  // scaf
 j=SYMNEXT(SYMLOCALROOT);
 SYMLOCALROOT=SYMORIGIN[j].next;       /* new top of stack            */
 u=j+SYMORIGIN;  // the new symbol.  u points to it, j is its index
 if(likely(SYMNEXT(tailx)!=0)) {L *t=SYMNEXT(tailx)+SYMORIGIN;
  // appending to tail, must be a symbol.  Queue is known to be nonempty
  u->next=t->next;t->next=j|(tailx&SYMNONPERM);  // it's always the end: point to next & prev, and chain from prev.  Everything added here is non-PERMANENT
 }else{
  // appending to head.
  u->next=*hv;  // chain old queue to u
  *hv=j|(tailx);   // set new head, flagged as NONPERM unless suppressed
 }
 R u;
}    /* allocate a new pool entry and insert into hash table entry hv */

// free all the symbols in symbol table w.  As long as the symbols are PERMANENT, delete the values but not the name.
// For non-PERMANENT, delete name and value.
// Reset the fields in the deleted blocks.
// This is used only for freeing local symbol tables, thus does not need to clear the name/path or worry about CACHED values
extern void jtsymfreeha(J jt, A w){I j,wn=AN(w); LX k,* RESTRICT wv=LXAV0(w);
 LX freeroot=0; LX *freetailchn=(LX *)jt->shapesink;  // sym index of first freed ele; addr of chain field in last freed ele
 L *jtsympv=SYMORIGIN;  // Move base of symbol block to a register.  Block 0 is the base of the free chain.  MUST NOT move the base of the free queue to a register,
  // because when we free an explicit locale it frees its symbols here, and one of them might be a verb that contains a nested SYMB, giving recursion.  It is safe to move sympv to a register because
  // we know there will be no allocations during the free process.
 // loop through each hash chain, clearing the blocks in the chain.  Do not clear chain 0, which holds x/y bucket numbers
 // There cannot be any CACHED blocks here,  since they exist only in named locales
 for(j=SYMLINFOSIZE;j<wn;++j){
  LX *aprev=&wv[j];  // this points to the predecessor of the last block we processed
  // process the chain
  if(k=*aprev){
   // first, free the PERMANENT values (if any), but not the names
   NOUNROLL do{
    if(!SYMNEXTISPERM(k))break;  // we are about to free k.  exit if it is not permanent
    I nextk=jtsympv[k].next;  // unroll loop 1 time
    aprev=&jtsympv[k].next;  // save last item we processed here
    if(jtsympv[k].val){
     // if the value was abandoned to an explicit definition, we took usecount 8..1  -> 1 ; revert that.  Can't change an ACPERMANENT!
     // otherwise decrement the usecount
     SYMVALFA(jtsympv[k]);
     jtsympv[k].val=0;jtsympv[k].valtype=0;  // clear value - don't clear name
    }
    k=nextk;
   }while(k);
   // clear chain in last PERMANENT block
   *aprev=0;  // only the PERMANENT survive
   // We are now pointing at the first non-permanent, if any.  Erase them all, deleting the name and value
   if(k){
    LX k1=SYMNEXT(k);  // remember first non-PERMANENT
    freeroot=freeroot?freeroot:k1;  // remember overall first value
    NOUNROLL do{
     k=SYMNEXT(k);  // remove address flagging
     I nextk=jtsympv[k].next;  // unroll loop once
     aprev=&jtsympv[k].next;  // save last item we processed here
     fa(jtsympv[k].name);fa(jtsympv[k].val);jtsympv[k].name=0;jtsympv[k].valtype=0;jtsympv[k].val=0;jtsympv[k].sn=0;jtsympv[k].flag=0;
     k=nextk;
    }while(k);
    // make the non-PERMANENTs the base of the free pool & chain previous pool from them
    *freetailchn=k1; freetailchn=aprev;  // free chain may have permanent flags
   }
  }
 }
 if(likely(freeroot!=0)){*freetailchn=SYMLOCALROOT;SYMLOCALROOT=freeroot;}  // put all blocks freed here onto the free chain
}

static SYMWALK(jtsympoola, I,INT,100,1, 1, *zv++=j;)

F1(jtsympool){A aa,q,x,y,*yv,z,zz=0,*zv;I i,n,*u,*xv;L*pv;LX j,*v;
 ARGCHK1(w); 
 ASSERT(1==AR(w),EVRANK); 
 ASSERT(!AN(w),EVLENGTH);
 READLOCK(JT(jt,stlock)) READLOCK(JT(jt,stloc)->lock) READLOCK(JT(jt,symlock))
 GAT0E(z,BOX,3,1,goto exit;); zv=AAV(z);
 n=AN((A)((I)SYMORIGIN-AKXR(0)))/symcol; pv=SYMORIGIN;
 GATV0E(x,INT,n*5,2,goto exit;); AS(x)[0]=n; AS(x)[1]=5; xv= AV(x); zv[0]=incorp(x);  // box 0: sym info
 GATV0E(y,BOX,n,  1,goto exit;);                         yv=AAV(y); zv[1]=incorp(y);  // box 1: 
 for(i=0;i<n;++i,++pv){         /* per pool entry       */
  *xv++=i;   // sym number
  *xv++=(q=pv->val)?LOWESTBIT(AT(pv->val)):0;  // type: only the lowest bit.  Must allow SYMB through
  *xv++=pv->flag+(pv->name?LHASNAME:0)+(pv->val?LHASVALUE:0);  // flag
  *xv++=pv->sn;    
  *xv++=SYMNEXT(pv->next);
  RZGOTO(*yv++=(q=pv->name)?incorp(sfn(SFNSIMPLEONLY,q)):mtv,exit);  // simple name
 }
 // Allocate box 3: locale name
 GATV0E(y,BOX,n,1,goto exit;); yv=AAV(y); zv[2]=incorp(y);
 DO(n, yv[i]=mtv;);
 n=AN(JT(jt,stloc)); v=LXAV0(JT(jt,stloc)); 
 for(i=0;i<n;++i){  // for each chain-base in locales pool
  for(j=v[i];j=SYMNEXT(j),j;j=SYMORIGIN[j].next){      // j is index to named local entry; process the chain
   x=SYMORIGIN[j].val;  // x->symbol table for locale
   RZGOTO(yv[j]=yv[LXAV0(x)[0]]=aa=incorp(sfn(SFNSIMPLEONLY,LOCNAME(x))),exit);  // install name in the entry for the locale
   RZGOTO(q=sympoola(x),exit); u=AV(q); DO(AN(q), yv[u[i]]=aa;);
  }
 }
 n=jtcountnl(jt);
 for(i=0;i<n;++i)if(x=jtindexnl(jt,i)){   /* per numbered locales */
  RZGOTO(      yv[LXAV0(x)[0]]=aa=incorp(sfn(SFNSIMPLEONLY,LOCNAME(x))),exit);
  RZGOTO(q=sympoola(x),exit); u=AV(q); DO(AN(q), yv[u[i]]=aa;);
 }
 if(EXPLICITRUNNING){               /* per local table      */
  RZGOTO(aa=incorp(cstr("**local**")),exit);
  RZGOTO(q=sympoola(jt->locsyms),exit); u=AV(q); DO(AN(q), yv[u[i]]=aa;);
 }
 zz=z;
exit: ;
 READUNLOCK(JT(jt,stlock)) READUNLOCK(JT(jt,stloc)->lock) READUNLOCK(JT(jt,symlock))
 RETF(zz);
}    /* 18!:31 symbol pool */

// l/string are length/addr of name, hash is hash of the name, g is symbol table  l is encoded in low bits of jt
// the symbol is deleted if found.  Return address of deleted symbol if it was cached - caller must then take responsibility for the name
// if the symbol is PERMANENT, it is not deleted but its value is removed
// if the symbol is CACHED, it is removed from the chain but otherwise untouched, leaving the symbol abandoned.  It is the caller's responsibility to handle the name
// We writelock the symbol table we are deleting from, and release the lock on exit.  It may be a symbol table or the global table stloc
L* jtprobedel(J jt,C*string,UI4 hash,A g){L *ret;
 F1PREFIP;
 RZ(g);
 L *sympv=SYMORIGIN;  // base of symbol pool
 LX *asymx=LXAV0(g)+SYMHASH(hash,AN(g)-SYMLINFOSIZE);  // get pointer to index of start of chain; address of previous symbol in chain
 WRITELOCK(g->lock)
 LX delblockx=*asymx;
 while(1){
  delblockx=SYMNEXT(delblockx);
  if(!delblockx){ret=0; break;}  // if chain empty or ended, not found
  L *sym=sympv+delblockx;  // address of next in chain, before we delete it
  LX nextdelblockx=sym->next;  // unroll loop once
  IFCMPNAME(NAV(sym->name),string,(I)jtinplace&0xff,hash,     // (1) exact match - if there is a value, use this slot, else say not found
    {
     if(unlikely(sym->flag&LCACHED)){
      *asymx=sym->next;   // cached: just unhook from chain.  can't be permanent
      ret=sym;   // cached block - return its address
     }else{
      SYMVALFA(*sym); sym->val=0; sym->valtype=0;  // decr usecount in value; remove value from symbol
      if(!(sym->flag&LPERMANENT)){*asymx=sym->next; fa(sym->name); sym->name=0; sym->flag=0; sym->sn=0; sym->next=SYMLOCALROOT; SYMLOCALROOT=delblockx;}  // add to symbol free list
      ret=0;  // normal return
     }
     break;  // name match - return
    }
   // if match, bend predecessor around deleted block, return address of match (now deleted but still points to value)
  )
  asymx=&sym->next;   // mismatch - step to next
  delblockx=nextdelblockx;
 }
 WRITEUNLOCK(g->lock)
 R ret;
}

// l/string are length/addr of name, hash is hash of the name, g is symbol table.  l is encoded in low bits of jt
// result is L* address of the symbol-table entry for the name, or 0 if not found or value missing
// locking is the responsibility of the caller
L*jtprobe(J jt,C*string,UI4 hash,A g){
 RZ(g);
 F2PREFIP;
 LX symx=LXAV0(g)[SYMHASH(hash,AN(g)-SYMLINFOSIZE)];  // get index of start of chain
 L *sympv=SYMORIGIN;  // base of symbol table
 L *symnext, *sym=sympv+SYMNEXT(symx);  // first symbol address - might be the free root if symx is 0
 NOUNROLL while(symx){  // loop is unrolled 1 time
  // sym is the symbol to process, symx is its index.  Start by reading next in chain.  One overread is OK, will be symbol 0 (the root of the freequeue)
  symnext=sympv+SYMNEXT(symx=sym->next);
  IFCMPNAME(NAV(sym->name),string,(I)jtinplace&0xff,hash,R sym->val?sym:0;)     // (1) exact match - if there is a value, use this slot, else say not found
  sym=symnext;  // advance to value we read
 }
 R 0;
}

// a is A for name; result is L* address of the symbol-table entry in the local symbol table, if there is one
// If the value is empty, return 0 for not found
// We know that there are buckets and that we should search them
// Take no locks
L *probelocalbuckets(L *sympv,A a,LX lx,I bx){NM*u;   // lx is LXAV0(locsyms)[bucket#], bx is index within bucket
 // There is always a local symbol table, but it may be empty
 RZ(a);u=NAV(a);  // u->NM block
 if(0 > (bx = ~bx)){
  // positive bucketx (now negative); that means skip that many items and then do name search.  This is set for words that were recognized as names but were not detected as assigned-to in the definition
  // we know we have tested for NAMEADDED; the normal path here is probably a failed search for a global.  recursive tables also come here
  I m=u->m; C* s=u->s; UI4 hsh=u->hash; // length/addr of name from name block, and hash
  if(unlikely(++bx!=0)){NOUNROLL do{lx = sympv[lx].next;}while(++bx);}  // rattle off the permanents, usually 1
  // Now lx is the index of the first name that might match.  Do the compares
  NOUNROLL while(lx=SYMNEXT(lx)) {L* l = lx+sympv;  // symbol entry
   IFCMPNAME(NAV(l->name),s,m,hsh,R l->val?l : 0;)
   lx = l->next;
  }
  R 0;  // no match.
 } else {
  L* l = lx+sympv;  // fetch hashchain headptr, point to L for first symbol
  // negative bucketx (now positive); skip that many items, and then you're at the right place
  if(unlikely(bx>0)){NOUNROLL do{l = l->next+sympv;}while(--bx);}  // skip the prescribed number, which is usually 1
  R l->val?l:0;
 }
}

// a is A for name; result is L* address of the symbol-table entry in the local symbol table, if there is one
// If the value is empty, return 0 for not found
// Take no locks
L *jtprobelocal(J jt,A a,A locsyms){NM*u;I b,bx;
 // There is always a local symbol table, but it may be empty
 ARGCHK1(a);u=NAV(a);  // u->NM block
 if(likely((b = u->bucket)!=0)){
  L *sympv=SYMORIGIN;  // base of symbol array
  // we don't check for primary symbol index because that is normally picked up in parsea
  if(0 > (bx = ~u->bucketx)){
   // positive bucketx (now negative); that means skip that many items and then do name search.  This is set for words that were recognized as names but were not detected as assigned-to in the definition
   // If no new names have been assigned since the table was created, we can skip this search, since it must fail (this is the path for words in z eg)
   if(likely(!(AR(locsyms)&ARNAMEADDED)))R 0;
   LX lx = LXAV0(locsyms)[b];  // index of first block if any
   I m=u->m; C* s=u->s; UI4 hsh=u->hash; // length/addr of name from name block, and hash
   if(unlikely(++bx!=0)){NOUNROLL do{lx = sympv[lx].next;}while(++bx);}  // rattle off the permanents, usually 1
   // Now lx is the index of the first name that might match.  Do the compares
   NOUNROLL while(lx=SYMNEXT(lx)) {L* l = lx+sympv;  // symbol entry
    IFCMPNAME(NAV(l->name),s,m,hsh,R l->val?l : 0;)
    lx = l->next;
   }
   R 0;  // no match.
  } else {
   LX lx = LXAV0(locsyms)[b];  // index of first block if any
   L* l = lx+sympv;  // fetch hashchain headptr, point to L for first symbol
   // negative bucketx (now positive); skip that many items, and then you're at the right place
   if(unlikely(bx>0)){NOUNROLL do{l = l->next+sympv;}while(--bx);}  // skip the prescribed number, which is usually 1
   R l->val?l:0;
  }
 } else {
  // No bucket information, do full search.  This includes names that don't come straight from words in an explicit definition
  R jtprobe((J)((I)jt+NAV(a)->m),NAV(a)->s,NAV(a)->hash,locsyms);
 }
}

// a is A for name; result is L* address of the symbol-table entry in the local symbol table (which must exist)
// If not found, one is created
// Take no locks
L *jtprobeislocal(J jt,A a){NM*u;I bx;L *sympv=SYMORIGIN;
 // If there is bucket information, there must be a local symbol table, so search it
 ARGCHK1(a);u=NAV(a);  // u->NM block
 // if this is a looked-up assignment in a primary symbol table, use the stored symbol#
 I4 symx, b;
#if 0 // obsolete 
 I sb=u->sb.symxbucket; symx=sb; b=sb>>32;  // fetch 2 values together if possible
#else
 symx=u->symx; b=u->bucket;
#endif
 if(likely((SGNIF(AR(jt->locsyms),ARLCLONEDX)|(symx-1))>=0)){R sympv+(I)symx;
 }else if((likely(b!=0))){
  LX lx = LXAV0(jt->locsyms)[b];  // index of first block if any
  if(unlikely(0 > (bx = ~u->bucketx))){
   // positive bucketx (now negative); that means skip that many items and then do name search
   // Even if we know there have been no names assigned we have to spin to the end of the chain
   // We don't unroll these loops because there is usually one symbol ber bucket
   I m=u->m; C* s=u->s; UI4 hsh=u->hash;  // length/addr of name from name block, and hash
   LX tx = lx;  // tx will hold the address of the last item in the chain, in case we have to add a new symbol
   L* l;
   NOUNROLL while(0>++bx){tx = lx; lx = sympv[lx].next;}  // all permanent
   // Now lx is the index of the first name that might match.  Do the compares
   NOUNROLL while(lx=SYMNEXT(lx)) {
    l = lx+sympv;  // symbol entry
    IFCMPNAME(NAV(l->name),s,m,hsh,R l;)
    tx = lx; lx = l->next;
   }
   // not found, create new symbol.  If tx is 0, the queue is empty, so adding at the head is OK; otherwise add after tx.  Make it non-PERMANENT
   SYMRESERVE(1) l=symnew(&LXAV0(jt->locsyms)[b],tx|SYMNONPERM); 
   ra(a); l->name=a;  // point symbol table to the name block, and increment its use count accordingly
   AR(jt->locsyms)|=ARNAMEADDED;  // Mark that a name has been added beyond what was known at preprocessing time
   R l;
  } else {L* l = lx+sympv;  // fetch hashchain headptr, point to L for first symbol
   // negative bucketx (now positive); skip that many items, and then you're at the right place
   NOUNROLL while(bx--){l = l->next+sympv;}  // all permanent
   R l;  // return 
  }
 } else {
  // No bucket information, do full search.  No lock needed on local table
  L *l=probeis(a,jt->locsyms);
  RZ(l);
  AR(jt->locsyms)|=(~l->flag)&LPERMANENT;  // Mark that a name has been added beyond what was known at preprocessing time, if the added name is not PERMANENT
  R l;
 }
}

// Acquire a symbol and then xctl to probeis.  Suitable when the caller needs a symbol AND has no locks
// May fail if symbol cannot be allocated
L* jtprobeisres(J jt,A a,A g){SYMRESERVE(1) R probeis(a,g);}

// a is A for name
// g is symbol table to use
// result is L* symbol-table entry to use; cannot fail, because symbol has been reserved
// if not found, one is created.  Caller must ensure that a symbol is available
// locking is the responsibilty of the caller
L*jtprobeis(J jt,A a,A g){C*s;LX tx;I m;L*v;NM*u;L *sympv=SYMORIGIN;
 u=NAV(a); m=u->m; s=u->s; UI4 hsh=u->hash;  // m=length of name  s->name  hsh=hash of name
// obsolete  SYMRESERVE(1)   // make sure there will be a symbol if we need one
 LX *hv=LXAV0(g)+SYMHASH(hsh,AN(g)-SYMLINFOSIZE);  // get hashchain base among the hash tables
 if(tx=SYMNEXT(*hv)){                                 /* !*hv means (0) empty slot    */
  v=tx+sympv;
  NOUNROLL while(1){
   LX lxnext=v->next;  // unroll loop once
   u=NAV(v->name);  // name may be 0 but only in unmoored cached references, which are never on any chain
   IFCMPNAME(u,s,m,hsh,R v;)    // (1) exact match - may or may not have value
   if(!lxnext)break;                                /* (2) link list end */
   v=(tx=SYMNEXT(lxnext))+sympv;
  }
 }
 // not found, create new symbol.  If tx is 0, the queue is empty, so adding at the head is OK; otherwise add after tx
 v=symnew(hv,tx|SYMNONPERM);   // symbol is non-PERMANENT and known to be available
 ra(a); v->name=a;  // point symbol table to the name block, and increment its use count accordingly
 R v;
}    /* probe for assignment */

// look up a non-locative name using the locale path
// g is the current locale, l/string=length/name, hash is the hash for it (l is carried in the low 8 bits of jt)
// result is L* symbol-table slot for the name, with bit 0 set with the AR flags of the locale found in (this indicates named/numbered), or 0 if none
// Bit 0 (ARNAMED) of the result is set iff the name was found in a named locale
// We must have no locks coming in; we take a read lock on each symbol table we have to search
// if we find a name, we ra() it under lock.  All we have to do is increment the name since it is known to be recursive if possible
L*jtsyrd1(J jt,C *string,UI4 hash,A g){A*v,x,y;L*e;
 RZ(g);  // make sure there is a locale...
 // we store an extra 0 at the end of the path to allow us to unroll this loop once
 I bloom=BLOOMMASK(hash); v=AAV0(LOCPATH(g));
 NOUNROLL while(g){A gn=*v++; if((bloom&~LOCBLOOM(g))==0){READLOCK(g->lock) e=jtprobe(jt,string,hash,g); if(e){ACINCRPOSSP(e->val); READUNLOCK(g->lock) R (L*)((I)e+AR(g));} READUNLOCK(g->lock)} g=gn;}  // return when name found.
 R 0;  // fall through: not found
}    /* find name a where the current locale is g */ 
// same, but return the locale in which the name is found, and no ra().  We know the name will be found somewhere
A jtsyrd1forlocale(J jt,C *string,UI4 hash,A g){A*v,x,y;
// if(b&&jt->local&&(e=probe(NAV(a)->m,NAV(a)->s,NAV(a)->hash,jt->local))){av=NAV(a); R e;}  // return if found local
 RZ(g);  // make sure there is a locale...
 I bloom=BLOOMMASK(hash); v=AAV0(LOCPATH(g)); NOUNROLL while(g){A gn=*v++; L *e; if((bloom&~LOCBLOOM(g))==0){READLOCK(g->lock) e=jtprobe(jt,string,hash,g); READUNLOCK(g->lock) if(e){break;}} g=gn;}  // return when name found.
 R g;
}


// u is address of indirect locative: in a__b__c, it points to the b
// n is the length of the entire locative (4 in this example)
// result is address of symbol table to use for name lookup (if not found, it is created)
static A jtlocindirect(J jt,I n,C*u,UI4 hash){A x,y;C*s,*v,*xv;I k,xn;
 A g;  // current locale.
 L *e=0;  // value looked up
 s=n+u;   // s->end+1 of name
 while(u<s){
  v=s; NOUNROLL while('_'!=*--v); ++v;  // v->start of last indirect locative
  k=s-v; s=v-2;    // k=length of indirect locative; s->end+1 of next name if any
  ASSERT(k<256,EVLIMIT);
  if(likely(!e)){  // first time through
   e=jtprobe((J)((I)jt+k),v,hash,jt->locsyms);  // look up local first.  All these lookups ra() the value if found
   if(!e)e=(L*)((I)jtsyrd1((J)((I)jt+k),v,hash,jt->global)&~ARNAMED);else{ACINCRLOCALPOS(e->val);}  // if not local, try global, and remove cachable flag
  }else e=(L*)((I)jtsyrd1((J)((I)jt+k),v,(UI4)nmhash(k,v),g)&~ARNAMED);   // look up later indirect locatives, yielding an A block for a locative; remove cachable flag
  ASSERTN(e,EVVALUE,nfs(k,v));  // verify found
  y=e->val;    // y->A block for locale
  ASSERTNGOTO(!AR(y),EVRANK,nfs(k,v),exitfa);   // verify atomic
  if(AT(y)&(INT|B01)){g=findnl(BIV0(y)); ASSERTGOTO(g!=0,EVLOCALE,exitfa);  // if atomic integer, look it up
  }else{
   ASSERTNGOTO(BOX&AT(y),EVDOMAIN,nfs(k,v),exitfa);  // verify box
   x=C(AAV(y)[0]); if((((I)AR(x)-1)&-(AT(x)&(INT|B01)))<0) {
    // Boxed integer - use that as bucketx, the locale number
    g=findnl(BIV0(x)); ASSERTGOTO(g!=0,EVLOCALE,exitfa);  // boxed integer, look it up
   }else{
    xn=AN(x); xv=CAV(x);   // x->boxed contents, xn=length, xv->string
    ASSERTNGOTO(1>=AR(x),EVRANK,nfs(k,v),exitfa);   // verify list (or atom)
    ASSERTNGOTO(xn,EVLENGTH,nfs(k,v),exitfa);   // verify not empty
    ASSERTNGOTO(LIT&AT(x),EVDOMAIN,nfs(k,v),exitfa);  // verify string
    ASSERTNGOTO(vlocnm(xn,xv),EVILNAME,nfs(k,v),exitfa);  // verify legal name
    I bucketx=BUCKETXLOC(xn,xv);
    RZGOTO(g=stfindcre(xn,xv,bucketx),exitfa);  // find st for the name
   }
  }
 }
 R g;
exitfa: ;
 fa(e->val);
 R 0;
}

// a is a locative NAME block; result is the starting locale, or 0 if error
// if the locative is direct we just look up/create the locale with that name; if indirect we find the value, then look up that locale
A jtsybaseloc(J jt,A a) {I m,n;NM*v;
 n=AN(a); v=NAV(a); m=v->m;
 // Locative: find the indirect locale to start on, or the named locale, creating the locale if not found
 if(likely(!(NMILOC&v->flag)))R stfindcre(n-m-2,1+m+v->s,v->bucketx);
 R locindirect(n-m-2,2+m+v->s,(UI4)v->bucketx);
}

// look up a name (either simple or locative) using the full name resolution
// result is symbol-table slot for the name if found, or 0 if not found
// If the block is found, the value has been ra()d
L*jtsyrd(J jt,A a,A locsyms){A g;
 ARGCHK1(a);
 if(likely(!(NAV(a)->flag&(NMLOC|NMILOC)))){L *e;
  // If there is a local symbol table, search it first
  if(e = probelocal(a,locsyms)){ACINCRLOCALPOSSP(e->val); R e;}  // return flagging the result if local.
  g=jt->global;  // Continue with the current locale
 } else RZ(g=sybaseloc(a));
 R (L*)((I)jtsyrd1((J)((I)jt+NAV(a)->m),NAV(a)->s,NAV(a)->hash,g)&~ARNAMED);  // Not local: look up the name starting in locale g
}
// same, but return locale in which found
A jtsyrdforlocale(J jt,A a){A g;
 ARGCHK1(a);
 if(likely(!(NAV(a)->flag&(NMLOC|NMILOC)))){L *e;
  // If there is a local symbol table, search it first
  if(e = probelocal(a,jt->locsyms)){R jt->locsyms;}  // return flagging the result if local
  g=jt->global;  // Start with the current locale
 } else RZ(g=sybaseloc(a));
 R jtsyrd1forlocale((J)((I)jt+NAV(a)->m),NAV(a)->s,NAV(a)->hash,g);  // Not local: look up the name starting in locale g
}
// same as syrd, but we have already checked for buckets
// look up a name (either simple or locative) using the full name resolution
// result is symbol-table slot for the name if found, or 0 if not found
// If the name/value are found, ra() the value
L*jtsyrdnobuckets(J jt,A a){A g;
 ARGCHK1(a);
 if(likely(!(NAV(a)->flag&(NMLOC|NMILOC)))){L *e;
  // If there is a local symbol table, search it first - but only if there is no bucket info.  If there is bucket info we have checked already
  if(unlikely(!NAV(a)->bucket))if(e = jtprobe((J)((I)jt+NAV(a)->m),NAV(a)->s,NAV(a)->hash,jt->locsyms)){ACINCRLOCALPOSSP(e->val); R e;}  // return if found locally from name
  g=jt->global;  // Start with the current locale
 } else RZ(g=sybaseloc(a));  // if locative, start in locative locale
 R (L*)((I)jtsyrd1((J)((I)jt+NAV(a)->m),NAV(a)->s,NAV(a)->hash,g)&~ARNAMED);  // Not local: look up the name starting in locale g
}


static A jtdllsymaddr(J jt,A w,C flag){A*wv,x,y,z;I i,n,*zv;L*v;
 ARGCHK1(w);
 n=AN(w); wv=AAV(w); 
 ASSERT(!n||BOX&AT(w),EVDOMAIN);
 GATV(z,INT,n,AR(w),AS(w)); zv=AV(z); 
 NOUNROLL for(i=0;i<n;++i){
  x=C(wv[i]); v=syrd(nfs(AN(x),CAV(x)),jt->locsyms); 
  ASSERT(v!=0,EVVALUE);
  y=v->val;
  ASSERTGOTO(NOUN&AT(y),EVDOMAIN,exitfa);
  zv[i]=flag?(I)AV(y):(I)v;
  fa(y);  // undo the ra() in syrd
 }
 RETF(z);
exitfa:;
 fa(y); R 0;
}    /* 15!:6 (0=flag) or 15!:14 (1=flag) */

F1(jtdllsymget){R dllsymaddr(w,0);}
F1(jtdllsymdat){R dllsymaddr(w,1);}

// look up the name w using full name resolution.  Return the value if found, abort if not found or invalid name
F1(jtsymbrd){L*v; ARGCHK1(w); ASSERTN(v=syrd(w,jt->locsyms),EVVALUE,w); tpush(v->val); R v->val;}   // undo the ra() in syrd

// look up name w, return value unless locked or undefined; then return just the name
F1(jtsymbrdlocknovalerr){A y;L *v;
 if(!(v=syrd(w,jt->locsyms))){
  // no value.  Could be undefined name (no error) or some other error including value error, which means error looking up an indirect locative
  // If error, abort with it; if undefined, return a reference to the undefined name
  RE(0);   // if not simple undefined, error
  R nameref(w,jt->locsyms);  // return reference to undefined name
 }
 // no error.  Return the value unless locked function
 y=v->val; tpush(y);  // undo the ra() in syrd
 R FUNC&AT(y)&&(jt->glock||VLOCK&FAV(y)->flag)?nameref(w,jt->locsyms):y;
}

// Same, but value error if name not defined
F1(jtsymbrdlock){A y;
 RZ(y=symbrd(w));
 R FUNC&AT(y)&&(jt->glock||VLOCK&FAV(y)->flag)?nameref(w,jt->locsyms):y;
}


// w is a value, v is the symbol-table entry about to be assigned
// Called only in debug mode.  If we redefine an executing name, it would invalidate
// the debug stack.  Fail if any redefinition would change part of speech or the id of the executing function.
// If the currently-executing definition is reloaded, mark the stack entry: xdefn will pick it up when debug is on
// as modified - xdefn will try to hot-swap to the new definition between lines
// If the modified name is executing higher on the stack, fail
// returns nonzero for OK to allow the assignment to proceed
B jtredef(J jt,A w,L*v){A f;DC c,d;
 // find the most recent DCCALL, exit if none
 d=jt->sitop; NOUNROLL while(d&&!(DCCALL==d->dctype&&d->dcj))d=d->dclnk; if(!(d&&DCCALL==d->dctype&&d->dcj))R 1;
 if(v==(L*)d->dcn){  // if the saved stabent (from the unquote lookup) matches the name being assigned...
  // attempted reassignment of the executing name
  // insist that the redefinition have the same type, and the same explicit character
  f=d->dcf;
  ASSERTN(TYPESEQ(AT(f),AT(w))&&(CCOLON==FAV(f)->id)==(CCOLON==FAV(w)->id),EVSTACK,d->dca);
  d->dcf=w;
  // If we are redefining the executing explicit definition during debug, remember that.
  // debug will switch over to the new definition before the next line is executed.
  // Reassignment outside of debug continues executing the old definition
  if(CCOLON==FAV(w)->id){d->dcredef=1;}
  // Erase any stack entries after the redefined call
  c=jt->sitop; NOUNROLL while(c&&DCCALL!=c->dctype){c->dctype=DCJUNK; c=c->dclnk;}
 }
 // Don't allow redefinition of a name that is suspended higher up on the stack
 c=d; NOUNROLL while(c=c->dclnk){ ASSERTN(!(DCCALL==c->dctype&&v==(L*)c->dcn),EVSTACK,c->dca);}
 R 1;
}    /* check for changes to stack */

// find symbol entry for name a in global symbol table g; this is known to be global assignment
// the name a may require lookup through the path; once we find the locale, we search only in it
// Result is &symbol-table entry for the name, or a new one
// We are called for purposes of setting assignsym for inplace assignments.  The symbol we create will eventually be assigned
// except when the locale is changed by the verb being called.  That is exceedingly rare.  When it happens, the symbol entry created
// here will persist until the locale is deleted (it has a name and no value).  Normally the symbol created here will be the target
// of its assignment - possibly only eventually after execution of a verb - and it will be used then.  We go ahead and create the
// symbol rather than failing the lookup because it does save a lookup in the eventual call to symbis.
L* jtprobeisquiet(J jt,A a){A g;
 I n=AN(a); NM* v=NAV(a); I m=v->m;  // n is length of name, v points to string value of name, m is length of non-locale part of name
 if(likely(n==m)){g=jt->global;}   // if not locative, define in default locale
 else{C* s=1+m+v->s; if(!(g=NMILOC&v->flag?locindirect(n-m-2,1+s,(UI4)v->bucketx):stfindcre(n-m-2,s,v->bucketx))){RESETERR; R 0;}}  // if locative, find the locale for the assignment; error is not fatal
 SYMRESERVE(1) WRITELOCK(g->lock) L *res=probeis(a, g); WRITEUNLOCK(g->lock)   // return pointer to slot, creating one if not found
 R res;
}

static I abandflag=LWASABANDONED;  // use this flag if there is no incumbent value
// assign symbol: assign name a in symbol table g to the value w (but g is ignored if a is a locative)
// Result points to the symbol-table block for the assignment
// flags set if jt: bit 0=this is a final assignment;
//  we tried using bit 1=jt->asginfo.assignsym is nonzero, use it; it saves a few cycles testing e, but it seemed risky if ASGNSAFE failed
// if g is marked as having local symbols, we assume that it is equal to jt->locsyms (especially in subroutines)
L* jtsymbis(J jt,A a,A w,A g){F2PREFIP;A x;I wn,wr;L*e;
 ARGCHK2(a,w);
 I anmf=NAV(a)->flag; RZ(g)  // fetch flags for the name
// obsolete  A jtlocal=jt->locsyms, 
// obsolete  A jtglobal=jt->global;  // current public symbol table
 e = jt->asginfo.assignsym; // set e if assignsym
 if(unlikely((anmf&(NMLOC|NMILOC))!=0)){I n=AN(a); I m=NAV(a)->m;
  // locative: n is length of name, v points to string value of name, m is length of non-locale part of name
  // Find the symbol table to use, creating one if none found.  Unfortunately assignsym doesn't give us the symbol table
  C*s=1+m+NAV(a)->s; if(unlikely(anmf&NMILOC))g=locindirect(n-m-2,1+s,(UI4)NAV(a)->bucketx);else g=stfindcre(n-m-2,s,NAV(a)->bucketx); RZ(g);
 }else{  // no locative: if g is a flag for assignsym, set it to the correct symbol table
  // not locative assignment, check for global assignment to a locally-defined name
  if(unlikely(g==jt->global))ASSERT(!probelocal(a,jt->locsyms),EVDOMAIN)  // this will usually have a positive bucketx and will fail quickly.  Unlikely that symx is present
 }
 // g has the locale we are writing to
 anmf=AR(g);  // get rank-flags for the locale g
// obsolete if ASGNSAFE perfect  if(unlikely(!((I)jtinplace&JTASSIGNSYMNON0))){++scafnoe;
 if(unlikely(e==0)){
  // we don't have e, look it up.  NOTE: this temporarily undefines the name, which will have a null value pointer.  We accept this, because any reference to
  // the name was invalid anyway and is subject to having the value removed
// obsolete   if(g==jt->locsyms)e=probeislocal(a); else{SYMRESERVE(1) WRITELOCK(g->lock) e=probeis(a, g); WRITEUNLOCK(g->lock)}
  if((anmf&ARLOCALTABLE)!=0)e=probeislocal(a); else{SYMRESERVE(1) WRITELOCK(g->lock) e=probeis(a, g); WRITEUNLOCK(g->lock)}
  RZ(e)
// obsolete   RZ(e=g==jtlocal?probeislocal(a) : probeis(a,g));   // set e to symbol-table slot to use
  if(unlikely(jt->glock!=0))if(unlikely(AT(w)&FUNC))if(likely(FAV(w)->fgh[0]!=0)){FAV(w)->flag|=VLOCK;}  // fn created in locked function is also locked
 }

 if(unlikely(jt->uflags.us.cx.cx_c.db))RZ(redef(w,e));  // if debug, check for changes to stack
 if(unlikely(e->flag&(LCACHED|LREADONLY))){  // exception cases
  ASSERT(!(e->flag&LREADONLY),EVRO)  // if writing read-only value (xxx_index), fail
  // LCACHED: We are reassigning a value that is cached somewhere.  We must protect the old value.  We will create a new symbol after e, transfer ownership of
  // the name to the new symbol, and then delete e, which will actually just make it a value-only unmoored symbol
  SYMRESERVE(1) L *newe=symnew(0,(e-SYMORIGIN)|SYMNONPERM); jtprobedel((J)((I)jt+NAV(e->name)->m),NAV(e->name)->s,NAV(e->name)->hash,g); newe->name=e->name; e->name=0; e=newe;
 }
 // if we are writing to a non-local table, update the table's Bloom filter.
 if((anmf&ARLOCALTABLE)==0){BLOOMOR(g,BLOOMMASK(NAV(a)->hash));}
 x=e->val;   // if x is 0, this name has not been assigned yet; if nonzero, x points to the incumbent value
 I xaf;  // holder for nvr/free flags
 {I *aaf=&AFLAG(x); aaf=x?aaf:&abandflag; xaf=*aaf;}  // flags from x, of LWASABANDONED if there is no x

 if(likely(!(AFNJA&xaf))){
  I wt=AT(w);
  // Normal case of non-memory-mapped assignment.
  // If we are assigning the same data block that's already there, don't bother with changing use counts or anything else (assignment-in-place)
  if(likely(x!=w)){
   // Increment the use count of the value being assigned, to reflect the fact that the assigned name will refer to it.
   // This realizes any virtual value, and makes the usecount recursive if the type is recursible
   // If the value is abandoned inplaceable, we can just zap it, set its usecount to 1, and make it recursive if not already
   // We do this only for final assignment, because we are creating a name that would then need to be put onto the NVR stack for protection if the sentence continued
   // If w does not contain NVR information, initialize it to do so.  LSB indicates NVR; till then it is a zap pointer
   // SPARSE nouns must never be inplaceable, because their components are not 
   rifv(w); // must realize any virtual
   if(likely((SGNIF((I)jtinplace,JTFINALASGNX)&AC(w)&(-(wt&NOUN)))<0)){  // if final assignment of abandoned noun
    *AZAPLOC(w)=0; ACRESET(w,ACUC1) if(unlikely(((wt^AFLAG(w))&RECURSIBLE)!=0)){AFLAGORLOCAL(w,wt&RECURSIBLE) jtra(w,wt);}  // zap it, make it non-abandoned, make it recursive (incr children if was nonrecursive).  This is like raczap(1)
      // NOTE: NJA can't zap either, but it never has AC<0
    AMNVRSET(w,AMNV);  // going from abandoned to name semantics: set NVR info in value
   }else{
    ra(w);  // if zap not allowed, just ra() the whole thing
    if(likely(!(AFLAG(w)&AFNJA)))AMNVRCINI(w);  // if not using name semantics now, and not NJA, initialize to name semantics
   }
   e->val=w; e->valtype=ATYPETOVALTYPE(wt);  // install the new value
   // if the value we are assigning is an ADV, and it contains no name references, and the name is not a locative, mark the value as NAMELESS to speed up later parsing.
   // NOTE that the value may be in use elsewhere; may even be a primitive.  Perhaps we should flag the name rather than the value.  But namelessness is a characteristic of the value.
   if(unlikely((wt&ADV)!=0))if(!(NAV(a)->flag&NMLOC+NMILOC+NMIMPLOC+NMDOT)&&(I)nameless(w))e->valtype=VALTYPENAMELESSADV;

   // We have raised the usecount of w.  Now dispose of x
   // If this is a reassignment, we need to decrement the use count in the old name, since that value is no longer used.
   // But if the value of the name is 'out there' in the sentence (coming from an earlier reference), we'd better not delete
   // that value until its last use.
   // For simplicity, we defer ALL deletions till the end of the sentence.  We put the to-be-deleted value onto the NVR stack if it isn't there already,
   // and free it.  If the value is already on the NVR stack and has been deferred-freed, we decrement the usecount here to mark the current free, knowing that the whole block
   // won't be freed till later.  By deferring all deletions we don't have to worry about whether local values are on the stack; and that allows us to avoid putting local values
   // on the NVR stack at all.
   // ABANDONED values can never be NVR (which are never inplaceable), so they will be flagged as !NVR,!UNFREED,ABANDONED
   if(((xaf|e->flag)&LWASABANDONED)!=0){
    // here for the cases where we don't change the usecount in x: nonexistent or abandoned x
    if(unlikely((e->flag&LWASABANDONED)!=0)){
     // Reassigning an x/y that was abandoned into this execution.  We did not increment the value when we started, so we'd better not decrement now.
     // However, we did change 8..1 to 1, and if the 1 is still there, we set it back to 8..1 so that the caller can see that the value is unincorporated.
     // The case where x==w is of interest (it comes up in x =. x , 5).  In that case we will not change the usecount of x/w below, so we have to keep the ABANDONED
     // status in sync with the usecount.  The best thing is to keep both unchanged, so that we can continue to inplace x
     ACOR(x,ACINPLACE&(AC(x)-2))  // apply ABANDONED: 1 -> 8..1 but only if we are going to replace x; we don't want 8..1 in an active name
     e->flag&=~(1LL<<LWASABANDONEDX);  // turn off abandoned flag after it has been applied, but only if we replace x
    }
   }else{
    // x must be decremented, one way or another.  If the value is virtual, it will eventually be freed by tpop, so we can just nonrecursively decrement the usecount now
    if(unlikely((xaf&AFVIRTUAL)!=0)){fadecr(x)  // virtual value, AM is still the backer
    }else{I am,nam;
     // Normal reassignment of a name.  AM must have NVR semantics.  What we do depends on NVR status
     AMNVRFREEACT(x,(I)jtinplace&JTFINALASGN,am,nam)  // analyze AM.  This sets am/nam as used by following code
     if(likely(am==nam)){fa(x);  // look at AM field, loaded into AM.  If value is on NVR and already deferred-free, OR if not on NVR and this is final assignment, we can free.  The block may still be in use elsewhere
     }else if(unlikely((am&-AMNVRCT)==0)){
      // (1) the value in x is not on the NVR stack.  But it may still be at large in the sentence, because we don't push local names
      // onto the NVR stack.  So, we defer the deletion until the end of the sentence, by adding the name to the NVR stack.  If we are already at the end of the sentence
      // we avoid this, and just fa(), since the local name cannot be in use higher up
      // (2) the value is not VIRTUAL.  The only way for an assigned value to be VIRTUAL is for it to be an initial assignment to x/y.  And to get here
      // the value must not have been abandoned.  So the usecount was raised on assignment (otherwise we would have gone through the no-fa special case).  So it is safe to fa() immediately then
      A nvra=jt->nvra;
      if(unlikely((I)(jt->parserstackframe.nvrtop+1U) > AN(nvra))){RZ(extnvr((L*)1)); nvra=jt->nvra;}  // Extend nvr stack if necessary.  copied from parser
      AAV1(nvra)[jt->parserstackframe.nvrtop++] = x;   // record the place where the value was protected (i. e. this sentence); it will be freed when this sentence finishes
     }
     // if the block was on the NVR stack and not freed, we have marked it freed and we will just wait for the eventual deletion
    }
   }
  }
 } else {  // x exists, and is either read-only or memory-mapped
  ASSERT(!(AFRO&xaf),EVRO);   // error if read-only value
  if(x!=w){  // replacing name with different mapped data.  If data is the same, just leave it alone
   realizeifvirtual(w);  // realize if virtual.  The copy stored in the mapped array must be real
   I wt=AT(w); wn=AN(w); wr=AR(w); I m=wn<<bplg(wt);
   ASSERT((wt&DIRECT)>0,EVDOMAIN);  // boxed, extended, etc can't be assigned to memory-mapped array
   ASSERT(allosize(x)>=m,EVALLOC);  // ensure the file area can hold the data
   AT(x)=wt; AN(x)=wn; AR(x)=(RANKT)wr; MCISH(AS(x),AS(w),wr); MC(AV(x),AV(w),m);  // copy in the data
  }
 }
 e->sn=jt->currslistx;  // Save the script in which this name was defined
 if(unlikely(JT(jt,stch)!=0))e->flag|=LCH;  // update 'changed' flag if enabled - no harm in marking locals too
 R e;   // return the block for the assignment
}    /* a: name; w: value; g: symbol table */

// assign symbol and free values immediately
// assignment within a sentence requires that values linger on a bit: till the end of the sentence or sometimes till printing is complete
// Values awaiting deletion are accumulated within the NVR stack till the sentence ends.  If there is an assignment not in a sentence, such as for for_x. or from sockets or DLLs,
// we have to finish the deletion immediately so that the NVR stack doesn't overflow
L* jtsymbisdel(J jt,A a,A w,A g){
 // All we have to do is mark the assignment as final.  That prevents any additions to the NVR stack.
 R jtsymbis((J)((I)jt|JTFINALASGN),a,w,g);
}
