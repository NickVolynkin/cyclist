// avp 2016 http://pastebin.com/d9Vymps2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

#include "cyclist.h" // http://pastebin.com/TGdUA9Se
#include "debug.h"   // http://pastebin.com/pd0EcWCZ

#define IF_DEBUG(e,t) \
  if (DEBUG(1000,e,t))

struct cltest {
  int v;
  struct cyclist list;
};

struct cltest *clget(int v) {
  struct cltest *p = (__typeof__(p))malloc(sizeof(*p));
  p->v = v;
  return p;
}

static int blabla = 0;

#define PRINT_LIST(p) print_list(__STRING(p) ": ", p, __LINE__)
#define IF1_PRILIST(p) if (blabla > 1) PRINT_LIST(p)

static void print_list (const char *msg, struct cyclist *l, int caller_lineno)
{
  struct cltest *p;
  struct cyclist *t;
  char emsg[100];
  sprintf(emsg, "links corrupted (caller line %d)\n", caller_lineno);

  puts(msg);
  cyclist_foreach(t, l) {
    p = list_entry(t, __typeof__(*p), list);
    printf("%d %p {%p %p} in %p\n", p->v, t, t->next, t->prev, l);
    DEBUG(0, (t->next->prev == t && t->prev->next == t), emsg);
  }
  puts("---------------");
}

static void vstore (struct cyclist *t, char *str, int size)
{
  int v = 0, l = strlen(str);
  struct cltest *p = CONTAINER_OF(t, struct cltest, list);
  if (p)
    v = p->v;
  DEBUG(0, (sprintf(str + l, "%d", v) + l + 1 < size), str);
}

#include <stdarg.h>
int
check_vlist (struct cyclist *l, ...)
{
  va_list args;
  va_start (args, l);
  int rc = 1, v = -1;
  struct cltest *t;
  
  cyclist_foreach_entry(t, l, list) {
    DEBUG(0, (t->list.next->prev == &t->list && t->list.prev->next == &t->list),
	  "cyclist corrupted");
    v = va_arg(args, int);
    if (v < 1 || t->v != v)
      break;
  }

  if (t || va_arg(args, int))
    rc = 0;
  
  va_end(args);
  return rc;
}

int
check_list (struct cyclist *l, const char *str)
{
  struct cltest *t;
  
  cyclist_foreach_entry(t, l, list) {
    DEBUG(0, (t->list.next->prev == &t->list && t->list.prev->next == &t->list),
	  "cyclist corrupted");
    if (!*str || t->v + '0' != *str++)
      return 0;
  }
  return *str == 0;
}

static void
loop_foreach_test (const char *msg, struct cyclist *l, const char *spat)
{
  int sz, i, j;
  char sbuf[sz = (strlen(spat) + 2)], rspat[sz];
  for (i = sz - 3, j = 0; i >= 0; i--, j++)
    rspat[j] = spat[i];
  rspat[j] = 0;
  
  if (msg && blabla)
    printf("%s  [%s]\n", msg, spat);
  struct cyclist *t = 0;
  struct cltest *p;
  struct cyclist_save sv;

  sbuf[0] = 0;
  cyclist_foreach(t, l) {
    vstore(t, sbuf, sz);
    DEBUG(0, (t->next->prev == t && t->prev->next == t),
	  "cyclist_foreach links corrupted");
  }
  DEBUG(0, strcmp(spat, sbuf) == 0, "cyclist_foreach");

  sbuf[0] = 0;
  cyclist_foreach_rev(t, l)
    vstore(t, sbuf, sz);
  DEBUG(0, strcmp(rspat, sbuf) == 0, "cyclist_foreach_rev");

  sbuf[0] = 0;
  cyclist_foreach_safe(t, sv, l)
    vstore(t, sbuf, sz);
  DEBUG(0, strcmp(spat, sbuf) == 0, "cyclist_foreach_safe");

  sbuf[0] = 0;
  cyclist_foreach_rev_safe(t, sv, l)
    vstore(t, sbuf, sz);
  DEBUG(0, strcmp(rspat, sbuf) == 0, "cyclist_foreach_rev_safe");

  i = 0;
  cyclist_foreach_entry(p, l, list)
    sbuf[i++] = p->v + '0';
  sbuf[i] = 0;
  DEBUG(0, strcmp(spat, sbuf) == 0, "cyclist_foreach_entry");

  i = 0;
  cyclist_foreach_rev_entry(p, l, list)
    sbuf[i++] = p->v + '0';
  sbuf[i] = 0;
  DEBUG(0, strcmp(rspat, sbuf) == 0, "cyclist_foreach_rev_entry");

  sbuf[0] = 0;    
  cyclist_foreach(t, l)
    break;
  DEBUG(0, (l) ? t->next == l->next : 1, "cyclist_foreach start");
  cyclist_foreach_from(t, l) {
    vstore(t, sbuf, sz);
    break;
  }
  DEBUG(0, sbuf[0] == spat[0], "cyclist_foreach_from");
  cyclist_foreach_cont(t, l) 
    vstore(t, sbuf, sz);
  DEBUG(0, strcmp(spat, sbuf) == 0, "cyclist_foreach_cont");

  sbuf[0] = 0;    
  cyclist_foreach_rev(t, l)
    break;
  DEBUG(0, l ? t->next == l : 1, "cyclist_foreach_rev start");
  cyclist_foreach_rev_from(t, l) {
    vstore(t, sbuf, sz);
    break;
  }
  DEBUG(0, sbuf[0] == rspat[0], "cyclist_foreach_rev_from");
  cyclist_foreach_rev_cont(t, l) 
    vstore(t, sbuf, sz);
  DEBUG(0, t == 0 && strcmp(rspat, sbuf) == 0, "cyclist_foreach_rev_cont");

  sbuf[0] = 0;    
  cyclist_foreach_safe(t, sv, l)
    break;
  DEBUG(0, l ? t->next == l->next : 1, "cyclist_foreach_safe start");
  cyclist_foreach_from_safe(t, sv, l) {
    vstore(t, sbuf, sz);
    break;
  }
  DEBUG(0, sbuf[0] == spat[0], "cyclist_foreach_from_safe");
  cyclist_foreach_cont_safe(t, sv, l) 
    vstore(t, sbuf, sz);
  DEBUG(0, strcmp(spat, sbuf) == 0, "cyclist_foreach_cont_safe");

  sbuf[0] = 0;    
  cyclist_foreach_rev_safe(t, sv, l)
    break;
  DEBUG(0, l ? t->next == l : 1, "cyclist_foreach_rev_safe start");
  cyclist_foreach_rev_from_safe(t, sv, l) {
    vstore(t, sbuf, sz);
    break;
  }
  DEBUG(0, sbuf[0] == rspat[0], "cyclist_foreach_rev_from_safe");
  cyclist_foreach_rev_cont_safe(t, sv, l) 
    vstore(t, sbuf, sz);
  DEBUG(0, t == 0 && strcmp(rspat, sbuf) == 0, "cyclist_foreach_rev_cont_safe");

  
  // check t = 0 not raise SIGSEGV for _from/_cont MACRO
  cyclist_foreach_from(t, l);
  cyclist_foreach_cont(t, l);
  cyclist_foreach_rev_from(t, l);
  cyclist_foreach_rev_cont(t, l);
  
  cyclist_foreach_from_safe(t, sv, l);
  cyclist_foreach_cont_safe(t, sv, l);
  cyclist_foreach_rev_from_safe(t, sv, l);
  cyclist_foreach_rev_cont_safe(t, sv, l);

}

void test_swapsort (struct cyclist **pl)
{
  struct cyclist *t, *s, *l = *pl ;
  struct cyclist_save tsv, ssv;

  cyclist_foreach_safe(t, tsv, l) {
    int v, v1 = container_of(t, struct cltest, list)->v;
    //    printf(":: %d\n", v1);
    if ((s = t->next) != l) 
      cyclist_foreach_from_safe(s, ssv, l) {

	if ((v = container_of(s, struct cltest, list)->v) < v1) {
	  v1 = v;
	  cyclist_swap_safe(&t, &l, &tsv,  &s, 0, &ssv);
	}
	// printf(" -> %d %d\n", v1, container_of(s, struct cltest, list)->v);
      }
  }

  *pl = l;
}

void check_cutroll (struct cyclist *l1, struct cyclist *list2head, int how,
		    const char *cmps, const char *msg)
{
  cyclist_cut(l1, list2head, how);
  char str[10];
  struct cyclist *t;
  int i;
    
  for(i = 0, t = list2head->next; how ? t != list2head : t != 0; t = t->next)
    str[i++] = container_of(t, struct cltest, list)->v + '0';
  str[i] = 0;
  l1 = cyclist_roll(list2head);
  DEBUG(0, (strcmp(str, cmps) == 0 &&
	    check_list(l1, cmps)), msg);
}


int
main (int ac, char *av[])
{
#ifdef PRINT
  blabla = 1;
#endif
  if (av[1]) {
    if (strncmp(av[1], "-v", 2))
      exit(puts("Usage: ./a.out [-v]\n  -v -- verbose"));
    if ((blabla = atoi(av[1] + 2) + 1) < 1)
      blabla = 1;
  }  

  printf("Go test it, blabla=%d\n", blabla);
  
  struct cltest *pcl, *p;
  struct cyclist  x, *l1 = 0, *l2 = 0, *ldel = 0, *s, *t = cyclist_singular(&x),
    *dl = 0, *cl = 0, *it;
  struct cyclist_save sv, sv2;
  int n[3], tv, y, m = 0, i, count = 0, err = 0;

  loop_foreach_test("foreach... null list", l1, "");

  IF_DEBUG(t == &x, "CYCLIST_SINGULAR")
    printf("CYCLIST_SINGULAR(&x %p {%p %p})\n", &x, x.next, x.prev);

  cyclist_add(&(pcl = clget(++count))->list, &l1);
  if (DEBUG(0, (l1 == &pcl->list && l1->next == l1->prev && l1->next == l1),
	    "ERR: cyclist_add first") || blabla)
    printf("cyclist_add {%d %p %p} list: %p pcl: %p\n",
	   (container_of(l1, struct cltest, list))->v, l1->next, l1->prev, 
	   l1, pcl);

  loop_foreach_test("foreach... singular list", l1, "1");

  if (blabla)
    puts("foreach_safe 1 item list: del l1 -> add to l2");
  cyclist_foreach_safe(t, sv, l1) {
    cyclist_del(t, &l1);
    cyclist_add(t, &l2);
  }
  loop_foreach_test("  foreach... null list l1", l1, "");
  loop_foreach_test("  foreach... singular list l2", l2, "1");
  if (blabla)
    puts("foreach_rev_safe 1 item list: del l2 -> add to l1");
  cyclist_foreach_rev_safe(t, sv, l2) {
    cyclist_del(t, &l2);
    cyclist_add(t, &l1);
  }
  loop_foreach_test("  foreach... null list l2", l2, "");
  loop_foreach_test("  foreach... singular list l1", l1, "1");

  cyclist_add(&(pcl = clget(++count))->list, &l1);
  loop_foreach_test("foreach... 2 items list", l1, "21");

  if (blabla)
    puts("foreach_(rev_)safe 2 items del l1 -> add to l2 and l2 -> l1");
  cyclist_foreach_safe(t, sv, l1) {
    cyclist_del(t, &l1);
    cyclist_add(t, &l2);
  }
  cyclist_foreach_rev_safe(t, sv, l2) { // ERR ???
    cyclist_del(t, &l2);
    cyclist_add(t, &l1);
  }
  loop_foreach_test("  foreach... null list l2", l2, "");
  loop_foreach_test("  foreach... 2 items list l1", l1, "12");
  cyclist_add(&(pcl = clget(++count))->list, &l1);
  loop_foreach_test("foreach... 3 items list l1", l1, "312");
  IF1_PRILIST(l1);

  if (blabla) puts("while/for technique & cyclist_del()");
  i = 0;
  if ((t = l1))
    do {
      tv = n[i++] = (container_of(t, struct cltest, list))->v;
      IF_DEBUG(i < 4, "do ... while")
	printf("loop4 %d %p: {%d %p %p} \n", i, t, tv, t->next, t->prev),
	err++;
      t = t->next;
    } while (t != l1);
  DEBUG(0, (!err && i == 3 && n[0] == 3 && n[2] == 2),
	"do { ... } while technique");


  if (blabla)
    printf("delete all %d items from l1, add them to l2 tail\n", i);
  for (i = 0; (t = l1); i++) {
    cyclist_del(t, &l1);
    cyclist_add_tail(t, &l2);
    IF_DEBUG(i < 4, "cyclist_del, cyclist_add_tail")
      printf("move to l2 %p: {%d %p %p} \n", t,
	     (container_of(t, struct cltest, list))->v, t->next, t->prev),
      err++;
  }
  DEBUG(0, (!err && l1 == 0 && i == 3 &&
	    check_list(l2, "312")),
	"cyclist_del, cyclist_add_tail");

  // count = 3
  IF1_PRILIST(l1);
  IF1_PRILIST(l2);

  if (blabla) puts("move all odd values from l2 to head l1");
  i = 0;
  cyclist_foreach_safe(t, sv, l2) {
    ++i;
    if (blabla > 2)
      printf("l2 item %p: {%d %p %p}\n", t,
	     (container_of(t, struct cltest, list))->v, t->next, t->prev);
    if ((container_of(t, struct cltest, list))->v & 1) {
      cyclist_del(t, &l2);
      cyclist_add(t, &l1);
      if (blabla > 2)
	printf("move to head %d %p: {%d %p %p}\n", i, t,
	       (container_of(t, struct cltest, list))->v, t->next, t->prev);
    }
  }
  DEBUG(0, (check_list(l1, "13") && check_list(l2, "2") && i == 3),
	"cyclist_foreach_safe, cyclist_del(t), cyclist_add");
  if (blabla)
    puts("insert singular l2 after head l1");
  l2 = cyclist_insert(l2, l1); 
  IF1_PRILIST(l1);
  IF1_PRILIST(l2);
  DEBUG(0, (check_list(l1, "123") && check_list(l2, "231")), "insert 1");

  if (blabla)
    puts("swap, cswap, split l2 to again l1 = '13' & l2 = '2'");
  s = l2->next; // 3
  t = l2->prev; // 1
  cyclist_swap(&s, &t);
  DEBUG(0, (check_list(l1, "132") && check_list(l2, "213")
	    && check_list(s, "132") && check_list(t, "321")), "swap 1");
  cyclist_cswap(l2->next, l2->prev);
  DEBUG(0, (check_list(l1, "123") && check_list(l2, "231")
	    && check_list(s, "123") && check_list(t, "312")), "cswap 1");
  cyclist_cswap(l2->next, l2->prev);
  DEBUG(0, (check_list(l1, "132") && check_list(l2, "213")), "cswap 2");
  cyclist_split(l1, l2);
  DEBUG(0, (check_list(l1, "13") && check_list(l2, "2")),
	"cyclist_split to again l1, l2");

  
  if (blabla)
    puts("cyclist_foreach_rev_from, cyclist_foreach_rev_cont for 3 l1 items");
  cyclist_add_tail(&(clget(++count))->list, &l1); i = 0;
  IF1_PRILIST(l2);
  IF1_PRILIST(l1);

  cyclist_foreach_rev(t, l1) {
    ++i;
    if (blabla > 2)
      printf("  foreach::%d %p: {%d %p %p}\n", i, t,
	     (container_of(t, struct cltest, list))->v, t->next, t->prev);
    break;
  }
  if (blabla > 1) puts("  cont...");
  s = t;
  cyclist_foreach_rev_cont(t, l1) {
    ++i;
    if (blabla > 2)
      printf("  foreach::%d %p: {%d %p %p}\n", i, t,
	     (container_of(t, struct cltest, list))->v, t->next, t->prev);
  }
  if (blabla > 1) puts("  again from begin");
  t = s;
  cyclist_foreach_rev_from(t, l1) {
    ++i; s = t;
    if (blabla > 2)
      printf("  foreach::%d %p: {%d %p %p}\n", i, s,
	     (container_of(t, struct cltest, list))->v, t->next, t->prev);
  }
  if (blabla > 1) puts("  again cyclist_foreach_rev_from() from last");
  t = s;
  cyclist_foreach_rev_from(t, l1) {
    ++i;
    if (blabla > 2)
      printf("  foreach::%d %p: {%d %p %p}\n", i, t,
	     (container_of(t, struct cltest, list))->v, t->next, t->prev);
  }
  DEBUG(0, (i == 7 && check_list(l1, "134")),
	"cyclist_foreach_rev_...(t, l1)");

  if (blabla) 
    puts("swaps (sort), move, join...");
  test_swapsort(&l2);
  DEBUG(0, check_list(l2, "2"), "swapsort '2'");
  IF1_PRILIST(l2);
  l1 = l1->next;
  IF1_PRILIST(l1);
  test_swapsort(&l1);
  DEBUG(0, check_list(l1, "134"), "swapsort '341'");
  cyclist_join(l1, l2);
  cyclist_cswap(l1, l1->next); l1 = l1->prev;
  IF1_PRILIST(l1);
  test_swapsort(&l1);
  DEBUG(0, check_list(l1, "1234"), "swapsort '3142'");
  cyclist_cswap(l1->next, l1->prev);
  cyclist_split(l1, l2 = l1->next->next);
  IF1_PRILIST(l2);
  IF1_PRILIST(l1);
  test_swapsort(&l1);
  test_swapsort(&l2);
  DEBUG(0, (check_list(l1, "14") && check_list(l2, "23")), 
	  "swapsort '14', '32'");
  t = l1->prev;
  cyclist_swap(&l1, &t);
  DEBUG(0, check_list(l1, "41"), "swap '14'");
    cyclist_join(l1, l2);
  DEBUG(0, check_list(l1, "4123"), "join '41', '23'");
  test_swapsort(&l1);
  DEBUG(0, check_list(l1, "1234"), "swapsort '4123'");
  // reverse list
  l2 = 0;
  cyclist_foreach_rev_safe(t, sv, l1)
    cyclist_move_safe(t, &l1, &sv, &l2, 0);
  DEBUG(0, (check_list(l1, "") && check_list(l2, "4321")), 
	"l2 = reverse '1234'");
  test_swapsort(&l2);
  DEBUG(0, check_list(l2, "1234"), "swapsort '4321'");
  IF1_PRILIST(l2);
  IF1_PRILIST(l1);
  l2 = cyclist_move(l2, &l1);
  t = 0;
  cyclist_move(l2->next, &t);
  DEBUG(0, (check_list(l1, "1") 
	    && check_list(l2, "24") && check_list(t, "3")), 
	"move '1234' to '1','24',t='3'");
  cyclist_swap(&l1, &t);
  DEBUG(0, (check_list(l1, "3") 
	    && check_list(l2, "24") && check_list(t, "1")), 
	"swap l1 = '1', t='3'");
  IF1_PRILIST(l2);
  IF1_PRILIST(l1);
  IF1_PRILIST(t);
  cyclist_cswap(l1, t);
  DEBUG(0, (check_list(l1, "3") 
	    && check_list(l2, "24") && check_list(t, "1")), 
	"cswap both singular l1 = '1', t='3'");
  cyclist_cswap(s = l2->next, t);
  DEBUG(0, (check_list(l1, "3") && check_list(s, "4")
	    && check_list(l2, "21") && check_list(t, "12")), 
	"cyclist_cswap(s = l2->next, t)");
  cyclist_swap(&t, &s);
  DEBUG(0, (check_list(l1, "3") && check_list(s, "1")
	    && check_list(l2, "24") && check_list(t, "42")), 
	"cyclist_cswap(&t, &s)");
  IF1_PRILIST(l2);
  IF1_PRILIST(l1);
  IF1_PRILIST(t);
  IF1_PRILIST(s);
  cyclist_join(l1, cyclist_join(l2, s));
  IF1_PRILIST(l1);
  test_swapsort(&l1);
  DEBUG(0, check_list(l1, "1234"), 
	"cyclist_join(l1, cyclist_join(l2, s)); swapsort");
  cyclist_del(l2 = l1->next, &l1);
  DEBUG(0, (check_list(l1, "134") && check_list(l2, "2")), 
	    "cyclist_del(l2 = l1->next, &l1)");
  IF1_PRILIST(l2);
  IF1_PRILIST(l1);
  
  if (blabla) puts("check delete in foreach 1, 2, 3 of 4 ...");

  cyclist_add((s = &clget(9)->list), &dl);
  cyclist_foreach_safe(t, sv, dl) 
    cyclist_del(t, &dl);
  DEBUG(0, dl == 0 && cyclist_singular(s), "cyclist_foreach_save, cyclist_del 1");
  cyclist_add(s, &dl);
  cyclist_foreach_rev_safe(t, sv, dl) {
    DEBUG(0, (container_of(t, struct cltest, list))->v == 9, 
	  "cyclist_foreach_rev_save t->v == 9");
    cyclist_del(t, &dl);
  }
  DEBUG(0, dl == 0 && cyclist_singular(s), "cyclist_foreach_rev_save, cyclist_del 1");
  // todo check del ...

  t = l1->next; // print l1 = 1 9 3 4
  cyclist_add(s, &t); i = 0;
  cyclist_foreach_safe(t, sv, l1) {
    ++i;
    y = (container_of(t, struct cltest, list))->v;
    if (blabla > 1) printf("l1-> %d %p: {%d %p %p}\n", i, t,
			   y, t->next, t->prev);
    if (i == 2 && y == 9)
      m++;
  }
  DEBUG(0, m == 1 && i == 4, "cyclist_foreach_save, l1 4 items");

  // here dl == 0
  y = 4; i = 0; t = l1->next->next; // t -> 3
  cyclist_foreach_from_safe(t, sv, l1) {
    IF_DEBUG( ++i < 3 && t->next == sv.next, "cyclist_foreach_from_safe l1")
      printf("from l1->next->next %d %p: {%d %p %p}\n", i, t,
	     y = (container_of(t, struct cltest, list))->v, t->next, t->prev);
  }
  DEBUG(0, i == 2 && y == 4, "cyclist_foreach_from_safe l1->next->next");

  i = 0; t = l1->next->next; // t -> 3
  cyclist_foreach_cont_safe(t, sv, l1)
    IF_DEBUG( ++i < 2 && t->next == sv.next, "cyclist_foreach_cont_safe l1")
      printf("cont l1->next->next %d %p: {%d %p %p}\n", i, t,
	   y = (container_of(t, struct cltest, list))->v, t->next, t->prev);
  DEBUG(0, i == 1 && y == 4, "cyclist_foreach_cont_safe l1->next->next");

  // l2 is singular, test it
  i = 0; t = l2;
  cyclist_foreach_from_safe(t, sv, l2)
    IF_DEBUG( ++i < 2 && t->next == sv.next, "cyclist_foreach_from_safe sing")
      printf("l2: %d %p: {%d %p %p}\n", i, t,
	     y = (container_of(t, struct cltest, list))->v, t->next, t->prev);
  DEBUG(0, i == 1 && y == 4, "cyclist_foreach_from_safe singular l2");

  i = 0; t = l2;
  cyclist_foreach_cont_safe(t, sv, l2)
    IF_DEBUG( ++i, "cyclist_foreach_cont_safe sing")
      printf("l2: %d %p: {%d %p %p}\n", i, t,
	   y = (container_of(t, struct cltest, list))->v, t->next, t->prev);
  DEBUG(0, i == 0 && y == 4, "cyclist_foreach_cont_safe singular l2");


  if (blabla)
    puts("cyclist_foreach_rev_safe ... some tests 4 items l1='1234', dl=''");

  IF_DEBUG(dl == 0 &&
	   (container_of(l1, struct cltest, list))->v == 1, "dl empty") {
    print_list("dl empty, test 1, 2, 3, 4 cyclist_foreach_rev_safe with delete",
	       dl, __LINE__);
    PRINT_LIST(l1);
    abort();
  }
  cyclist_foreach_rev_safe(t, sv, l1) {
    cyclist_del(t, &l1);
    cyclist_add(t, &dl);
  }
  IF_DEBUG(l1 == 0 &&
	   (container_of(dl, struct cltest, list))->v == 1 &&
	   (container_of(dl->next, struct cltest, list))->v == 9 &&
	   (container_of(dl->next->next, struct cltest, list))->v == 3 &&
	   (container_of(dl->prev, struct cltest, list))->v == 4 , "l1 empty") {
    print_list("after move  (all 4)\nl1: ", l1, __LINE__);
    PRINT_LIST(dl);
    abort();
  }
  //  puts("move 1 item from dl to cl");
  cyclist_foreach_rev_safe(t, sv, dl) {
    cyclist_del(t, &dl);
    cyclist_add(t, &cl);
    break;
  }
  IF_DEBUG(cl->next == cl->prev, "move 1 item from dl to cl") {
    PRINT_LIST(dl);
    PRINT_LIST(cl);
    abort();
  }
  //  puts("continue move (all 3) from dl to l1");
  cyclist_foreach_rev_safe(t, sv, dl) {
    cyclist_del(t, &dl);
    cyclist_add(t, &l1);
  }
  //    print_list("after delete all\nl1: ", l1);
  //    PRINT_LIST(dl);
  //  puts("again move all (realy 1) from cl to dl");
  cyclist_foreach_rev_safe(t, sv, cl) {
    cyclist_del(t, &cl);
    cyclist_add(t, &dl);
  }
  IF_DEBUG(cl == 0 && l1->next->next == l1->prev && dl == dl->next,
	   "OK, (tests 4, 3, and 1 items COMPETED) dl: ") {
    print_list("OK, (tests 4, 3, and 1 items COMPETED) dl: ", dl, __LINE__);
    PRINT_LIST(cl);
    print_list("now move 1 item from l1 to dl (for 2 items in dl) l1: ",
	       l1, __LINE__);
    abort();
  }
  t = l1;
  cyclist_del(t, &l1);
  cyclist_add(t, &dl);
  //  print_list("test move 2 items (all) to emty cl from dl:", dl);
  cyclist_foreach_rev_safe(t, sv, dl) {
    cyclist_del(t, &dl);
    cyclist_add(t, &cl);
  }
  IF_DEBUG(dl == 0 && l1->next == l1->prev && cl->next == cl->prev,
	   "test move 2 items (all) to emty cl from dl:") {
    print_list("end foreach_rev_safe l1:", l1, __LINE__);
    PRINT_LIST(cl);
    print_list("and restore l1, moving all from cl dl: ", dl, __LINE__);
    abort();
  }
  i = 0;
  cyclist_foreach_rev_safe(t, sv, cl) {
    cyclist_del(t, &cl);
    i++ ?  cyclist_add(t, &l1) :  cyclist_add_tail(t, &l1);
  }

  //  puts("foreach_entry in l1 (4 items)");
  IF_DEBUG(cl == 0, "foreach_entry in l1 (4 items)") {
    cyclist_foreach_entry(p, l1, list)
      printf("%d %p %p {%p %p}\n",
	     p->v, p, &p->list, p->list.next, p->list.prev);
    puts("---\nforeach_rev_entry in l1 (4 items)");
    cyclist_foreach_rev_entry(p, l1, list)
      printf("%d %p %p {%p %p}\n",
	     p->v, p, &p->list, p->list.next, p->list.prev);
  
    print_list("repeate tests for cyclist_foreach_rev_..._safe and l1: ",
	       l1, __LINE__);
    abort();
  }
  i =0;
  cyclist_foreach_rev_safe(t, sv, l1) {
    if (i++)
      break;
    cyclist_del(t, &l1);
    cyclist_add(t, &dl);
  }
  cyclist_foreach_rev_cont_safe(t, sv, l1) {
    cyclist_del(t, &l1);
    cyclist_add(t, &cl);
  }
  t = l1;
  cyclist_foreach_rev_from_safe(t, sv, l1) {
    cyclist_del(t, &l1);
    cyclist_add(t, &dl);
  }
  IF_DEBUG((check_list(l1, "") &&
	    check_list(cl, "19") && check_list(dl, "34")),
	   "check l1 cl dl") {
    PRINT_LIST(l1);
    PRINT_LIST(cl);
    PRINT_LIST(dl);
  }

  t = dl->next;
  IF_DEBUG((cyclist_remove(t) &&
	    cyclist_remove(t) == 0 &&
	    cyclist_remove(t) == 0 &&
	    check_list(dl, "3") &&
	    check_list(t, "4")), "check remove") {
    PRINT_LIST(dl);
    PRINT_LIST(t);
    abort();
  }

  if (blabla) puts("foreach_entry ...");
  m = 0;
  cyclist_foreach_entry(p, dl, list) {
    if (blabla > 1) printf("%d %p %p {%p %p}\n",
			   p->v, p, &p->list, p->list.next, p->list.prev);
    m = m * 10 + p->v;
  }
  DEBUG(0, m == 3, "foreach_entry in dl (one entry)");
  
  cyclist_foreach_rev_entry(p, dl, list) {
    if (blabla > 1) printf("%d %p %p {%p %p}\n",
			   p->v, p, &p->list, p->list.next, p->list.prev);
    m = m * 10 + p->v;
  }
  DEBUG(0, m == 33, "foreach_rev_entry in dl (one entry)");

  IF1_PRILIST(t);
  cyclist_add_tail(t, &dl);
  IF1_PRILIST(dl);

  m = 0;
  cyclist_foreach_entry(p, l1, list) {
    if (blabla > 1)
      printf("%d %p %p {%p %p}\n",
	   p->v, p, &p->list, p->list.next, p->list.prev);
    m = m * 10 + p->v;
  }
  DEBUG(0, m == 0, "foreach_entry in l1 (empty)");
  
  cyclist_foreach_rev_entry(p, l1, list) {
    if (blabla > 1)
      printf("%d %p %p {%p %p}\n",
	     p->v, p, &p->list, p->list.next, p->list.prev);
    m = m * 10 + p->v;
  }
  DEBUG(0, m == 0, "foreach_rev_entry in l1 (empty)");

  cyclist_foreach_entry(p, cl, list) {
    if (blabla > 1) printf("%d %p %p {%p %p}\n",
			   p->v, p, &p->list, p->list.next, p->list.prev);
    m = m * 10 + p->v;
  }
  DEBUG(0, m == 19, "foreach_entry in cl (two entries)");

  m = 0;
  cyclist_foreach_rev_entry(p, cl, list) {
    if (blabla > 1) printf("%d %p %p {%p %p}\n",
			   p->v, p, &p->list, p->list.next, p->list.prev);
    m = m * 10 + p->v;
  }
  DEBUG(0, m == 91, "foreach_rev_entry in cl (two entries)");

  if (blabla) puts("split/join");
  IF_DEBUG(check_list(cyclist_join(cl, dl), "1934"),
	   "cyclist_join(cl, dl)") 
    print_list("join cl dl: ", cl, __LINE__);

  IF_DEBUG(check_list(t = cyclist_split(cl, dl), "34"),
	   "cyclist_split(cl, dl)") {
    print_list("result  split cl dl: ", t, __LINE__);
    print_list("cl: split cl dl: ", cl, __LINE__);
    print_list("dl: split cl dl: ", dl, __LINE__);
  }
  
  IF_DEBUG(check_list(l1 = cyclist_split(cyclist_join(cl, dl), dl->next), "4"),
	   "split(join cl,dl) dl->next") {
    print_list("l1 =  split(join cl,dl) dl->next: ", l1, __LINE__);
    print_list("cl: split cl dl: ", cl, __LINE__);
    print_list("dl: split cl dl: ", dl, __LINE__);
  }

  dl = cyclist_split(cl, cl->next);
  DEBUG(0, (check_list(cl, "1") &&
	    check_list(dl, "93")), "cl: split cl cl->next: / dl ");

  cyclist_insert(cl, l1);
  DEBUG(0, (check_list(l1, "41")), "l1 =  insert(cl,l1): ");

  cyclist_split(cl, l1);
  DEBUG(0, (check_list(l1, "4") &&
	    check_list(cl, "1")), "l1 =  split(cl,l1):  cl =  split(cl,l1): ");

  cyclist_insert(cl, l1->prev);
  DEBUG(0, (check_list(l1, "41")), "l1 =  insert(cl,l1->prev): ");
  
  cyclist_insert(l2, cl);
  cyclist_join(cl, cyclist_split(t, t->next));
  cyclist_insert(t, cl->next);

  IF_DEBUG((check_list(cl, "12349") &&
	    check_list(t, "34912")), "insert/join/insert 5 lists") {
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(dl);
    PRINT_LIST(cl);
    PRINT_LIST(t);
  }

  IF_DEBUG((check_list(cyclist_split(cl, t), "349") &&
	    check_list(cl, "12")), "split all again") {
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(dl);
    PRINT_LIST(cl);
    PRINT_LIST(t);
  }

  cyclist_join(cl, t);
  // =====
  IF_DEBUG((check_list(dl, "91234") &&
	    check_vlist(cyclist_replace(s = &clget(++count)->list, dl),
		       1, 2, 3, 4, 5, 0) &&
	    check_list(dl, "9")), "replace dl->9 to new 5") {
    PRINT_LIST(cl);
    PRINT_LIST(dl);
    PRINT_LIST(s);
  }

  IF_DEBUG((check_list(dl, "9") &&
	    check_list(cyclist_replace(s = &clget(++count)->list, dl), "6") &&
	    check_list(s, "6") &&
	    check_list(cl, "12345")), "replace singular dl") {
    PRINT_LIST(cl);
    PRINT_LIST(dl);
    PRINT_LIST(s);
  }

  cyclist_join(dl, s);

  if (blabla) {
    puts("foreach add_safe/add_tail_safe");
    IF1_PRILIST(l1);
    IF1_PRILIST(l2);
    IF1_PRILIST(dl);
    IF1_PRILIST(cl);
    IF1_PRILIST(t);
  }

  s = &clget(++count /*7*/)->list;
  i = 0;
  cyclist_foreach_safe(t, sv, dl) {
    if (i == 0)
      cyclist_add_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 2 && check_list(dl, "796")),
	"cyclist_foreach_safe, cyclist_add_safe(0)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_safe(t, sv, dl) {
    if (i == 1)
      cyclist_add_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 2 && check_list(dl, "796")),
	"cyclist_foreach_safe, cyclist_add_safe(1)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_safe(t, sv, dl) {
    if (i == 0)
      cyclist_add_tail_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 3 && check_list(dl, "967")),
	"cyclist_foreach_safe, cyclist_add_tail_safe(0)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_safe(t, sv, dl) {
    if (i == 1)
      cyclist_add_tail_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 3 && check_list(dl, "967")),
	"cyclist_foreach_safe, cyclist_add_tail_safe(1)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_rev_safe(t, sv, dl) {
    if (i == 0)
      cyclist_add_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 3 && check_list(dl, "796")),
	"cyclist_foreach_rev_safe, cyclist_add_safe(0)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_rev_safe(t, sv, dl) {
    if (i == 1)
      cyclist_add_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 3 && check_list(dl, "796")),
	"cyclist_foreach_rev_safe, cyclist_add_safe(1)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_rev_safe(t, sv, dl) {
    if (i == 0)
      cyclist_add_tail_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 2 && check_list(dl, "967")),
	"cyclist_foreach_rev_safe, cyclist_add_tail_safe(0)");
  cyclist_del(s, &dl);

  i = 0;
  cyclist_foreach_rev_safe(t, sv, dl) {
    if (i == 1)
      cyclist_add_tail_safe(s, &dl, &sv);
    ++i;
  }
  DEBUG(0, (i == 2 && check_list(dl, "967")),
	"cyclist_foreach_rev_safe, cyclist_add_tail_safe(1)");
  cyclist_del(s, &dl);
  
  if (blabla) {
    puts("foreach _del_safe() works (for current pos)");
    IF1_PRILIST(dl);
    IF1_PRILIST(cl);
    IF1_PRILIST(s);
    IF1_PRILIST(ldel);
  }

  cyclist_foreach_safe(it, sv, cl) {
    cyclist_del_safe(it, &cl, &sv);
    cyclist_add_tail(it, &ldel);
  }
  DEBUG(0, (check_list(cl, "") && check_list(ldel, "12345")),
	"cyclist_foreach_safe(it, sv, cl) cyclist_del_safe  cyclist_add_tail(it, &ldel)");
  cyclist_foreach_safe(it, sv, ldel) {
    t = it->next;
    cyclist_del_safe(t, &ldel, &sv);
    cyclist_add_tail(t, &cl);
  }
  DEBUG(0, (check_list(cl, "241") && check_list(ldel, "35")),
	"cyclist_foreach_safe(it, sv, cl) cyclist_del_safe  cyclist_add_tail(it, &ldel)");
  cyclist_foreach_rev_safe(it, sv, cl) {
    cyclist_del_safe(it, &cl, &sv);
    cyclist_add_tail(it, &ldel);
  }
  DEBUG(0, (check_list(cl, "") && check_list(ldel, "35142")),
	"cyclist_foreach_safe(it, sv, cl) cyclist_del_safe  cyclist_add_tail(it, &ldel)");  
  cyclist_foreach_rev_safe(it, sv, ldel) {
    t = it->next; // delete back of direction !!!
    cyclist_del_safe(t, &ldel, &sv);
    cyclist_add_tail(t, &cl);
  }
  DEBUG(0, (check_list(cl, "3241") && check_list(ldel, "5")),
	"cyclist_foreach_safe(it, sv, cl) cyclist_del_safe  cyclist_add_tail(it, &ldel)");
  cyclist_foreach_rev_safe(it, sv, cl) {
    t = it->prev; // delete forward
    cyclist_del_safe(t, &cl, &sv);
    cyclist_add_tail(t, &ldel);
  }
  DEBUG(0, (check_list(cl, "21") && check_list(ldel, "543")),
	"cyclist_foreach_safe(it, sv, cl) cyclist_del_safe  cyclist_add_tail(it, &ldel)");

  cyclist_join(ldel, cl);
  cl = 0;
  IF_DEBUG((check_list(s, "7") && check_list(cl, "") &&
	    check_list(ldel, "54321") && check_list(t, "32154") &&
	    check_list(l1, "43215") && check_list(l2, "21543") &&
	    check_list(dl, "96") && check_list(it, "")),
	   "mid test lists")
    err = 1;
  if (err || blabla > 1) {
    if (!err)
      puts("mid test lists");
    PRINT_LIST(s);
    PRINT_LIST(cl);
    PRINT_LIST(dl);
    PRINT_LIST(ldel);
    PRINT_LIST(t);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(it);
    if (err)
      abort();
  }
  
  // _del_safe() works, but ... not obviously in loops // IN WORK NOW
  if (blabla)
    puts("foreach _del_safe() works, but ... not obviously in jump over loops");

  i = 0;
  cyclist_foreach_safe(it, sv, ldel) {
    t = it->next;
    p = container_of(t, struct cltest, list);
    if (blabla > 1)
      printf("%d: %d   ", i, p->v);
    i++;    
    cyclist_del_safe(t, &ldel, &sv);
    cyclist_add_tail(t, &cl);
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(ldel);
  }
  cyclist_join(ldel, cl);
  cl = 0;
  IF1_PRILIST(cl);
  IF1_PRILIST(ldel);
  DEBUG(0, (check_list(ldel, "31425") && i == 3),
	"cyclist_foreach_safe(ldel)/del_safe(t->next, cl)/join");
  
  i = 0;
  cyclist_foreach_rev_safe(it, sv, ldel) {
    t = it->prev;
    p = container_of(t, struct cltest, list);
    if (blabla > 1)
      printf("%d: %d   ", i, p->v);
    i++;    
    cyclist_del_safe(t, &ldel, &sv);
    cyclist_add_tail(t, &cl);
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(ldel);
    PRINT_LIST(it);
  }
  DEBUG(0, (check_list(ldel, "34") && check_list(cl, "215") && i == 3),
	"cyclist_foreach_rev_safe(ldel)/del_safe(t->next, cl)");

  l1 = l2 = 0;
  i = 0;
  cyclist_foreach_safe(it, sv, ldel) {
    //    t = it->prev;
    t = it->next;
    p = container_of(t, struct cltest, list);
    if (blabla > 1)
      printf("%d: %d   ", i, p->v);
    i++;    
    cyclist_del_safe(t, &ldel, &sv);
    cyclist_add_tail(t, &cl);
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(ldel);
  }
  cyclist_move(cl->prev, &ldel);
  if (blabla > 1) {
    puts("cyclist_move(cl->prev, &ldel);");
    PRINT_LIST(cl);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (check_list(ldel, "34") && check_list(cl, "215") && i == 1),
	"test2: cyclist_foreach_safe(ldel)/del_safe(t->next, cl)");

  cyclist_foreach_rev_safe(it, sv, ldel) {
    t = it->prev;
    p = container_of(t, struct cltest, list);
    if (blabla > 1)
      printf("%d: %d   ", i, p->v);
    i++;    
    cyclist_del_safe(t, &ldel, &sv);
    cyclist_add_tail(t, &cl);
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(ldel);
    PRINT_LIST(it);
  }
  DEBUG(0, (check_list(ldel, "4") && check_list(cl, "2153") && i == 2),
	"test2: cyclist_foreach_safe(ldel)/del_safe(t->next, cl)");

  if (blabla)
    puts("foreach _insert_safe()");
  i = 0;
  cyclist_foreach_safe(t, sv, cl) {
    p = container_of(t, struct cltest, list);
    if (blabla > 1) printf("%d: %d   ", i, p->v);
    i++;    
    if (i == 2) {
      if (blabla > 1) puts("");
      cyclist_foreach_safe(it, sv2, dl) {
	cyclist_del(it, &dl);
	p = container_of(it, struct cltest, list);
	if (blabla > 1) printf(">> %d: %d\n", i, p->v);
	cyclist_insert_safe(it, t->next, &sv);
      }
    }
  }
  cyclist_move(ldel, &cl);
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(dl);
    PRINT_LIST(ldel);
    PRINT_LIST(t);
    PRINT_LIST(it);
  }
  DEBUG(0, (i == 6 && check_list(dl, "") && check_list(cl, "2156934")),
	"cyclist_foreach_safe(t, cl) { if _insert_safe(t) }");
  dl = cyclist_split(cl, cl->prev->prev);
  i = 0;
  cyclist_foreach_rev_safe(t, sv, cl) {
    p = container_of(t, struct cltest, list);
    if (blabla > 1) printf("%d: %d   ", i, p->v);
    i++;    
    if (i == 2) {
      if (blabla > 1) puts("");
      cyclist_foreach_safe(it, sv2, dl) {
	cyclist_del(it, &dl);
	p = container_of(it, struct cltest, list);
	if (blabla > 1) printf(">> %d: %d\n", i, p->v);
	cyclist_insert_safe(it, t->prev, &sv);
      }
    }
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(dl);
  }
  DEBUG(0, (i == 7 && check_list(dl, "") && check_list(cl, "2153469")),
	"cyclist_foreach_rev_safe(t, cl) { if _insert_safe(t) }");
  
  if (blabla)
    puts("foreach _move_safe()");
  dl = cyclist_split(cl, cl->prev->prev);
  i = 0;
  cyclist_foreach_rev_safe(t, sv, cl) {
    i++;    
    p = container_of(t, struct cltest, list);
    if (blabla > 1) printf("%d: %d   ", i, p->v);
    if (i & 1) {
      struct cyclist *x = t->prev;
      if (blabla > 1) puts("");
      cyclist_foreach_safe(it, sv2, dl) {
	p = container_of(it, struct cltest, list);
	if (blabla > 1) printf(">> %d: %d\n", i, p->v);
	cyclist_move_safe(it, &dl, &sv2, &x, &sv);
	break;
      }
    }
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (i == 7 && check_list(dl, "") && check_list(cl, "2195634")),
	"cyclist_foreach_rev_safe(t, cl) { if _move_safe(t) }");

  if (blabla)
    puts("foreach _replace_safe()");
  l2 = cl;
  l1 = cyclist_replace_safe(&clget(++count /*8*/)->list, l2, &cl, 0);
  if (blabla > 1) {
    PRINT_LIST(cl);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (check_list(l2, "2") && check_list(cl, "8195634")
	    && check_list(l1, "1956348")),
	"cyclist_replace_safe");

  i = 0;
  cyclist_foreach_safe(t, sv, cl) {
    i++;    
    p = container_of(t, struct cltest, list);
    if (blabla > 1) printf("%d: %d   ", i, p->v);
    if (i == 3) {
      ldel = t;
      l1 = cyclist_replace_safe(l2, t, &cl, &sv);
    }
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (check_list(ldel, "9") && check_list(cl, "8125634")),
	"test1: foreach_safe / if i == 3 cyclist_replace_safe");
  i = 0;
  cyclist_foreach_safe(t, sv, cl) {
    i++;    
    p = container_of(t, struct cltest, list);
    if (blabla > 1) printf("%d: %d   ", i, p->v);
    if (i == 3) {
      l2 = t->next;
      l1 = cyclist_replace_safe(s, l2, &cl, &sv);
    }
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (i == 7 && check_list(l2, "5") && check_list(cl, "8127634")),
	"test2: foreach_safe / if i == 3 cyclist_replace_safe");

  i = 0;
  cyclist_join(ldel, l2);
  l2 = 0;
  cyclist_foreach_safe(it, sv2, ldel)
    break;

  
  if (blabla)
    puts("again _move_safe()");
  cyclist_foreach_safe(t, sv, cl) {
    i++;    
    p = container_of(t, struct cltest, list);
    if (blabla > 1) printf("%d: %d   ", i, p->v);
    if (i & 1) { // each odd step move 1 item from ldel to cl
      int j = 0;
      cyclist_foreach_from_safe(it, sv2, ldel) {
	  if (!j++)
	    cyclist_move_safe(it, &ldel, &sv2, &cl, &sv);
	  else
	    break;
	}
    }
    if (!(p->v & 1)) // move even values to l2
      cyclist_move_safe(t, &cl, &sv, &l2, 0);
  }
  if (blabla > 1) {
    puts("");
    PRINT_LIST(cl);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (i == 9 && check_list(l2, "8264")
	    && check_list(ldel, "")
	    && check_list(cl, "17395")), 
	"complex: foreach_safe / if ... cyclist_move_safe");

  i = 0;
  cyclist_foreach_rev_safe(t, sv, cl) {
    cyclist_move_safe(l2, &l2, 0, &t, &sv);
    i++;
  }
  if (blabla > 1) {
    PRINT_LIST(cl);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l1);
    PRINT_LIST(l2);
    PRINT_LIST(ldel);
  }
  DEBUG(0, (i == 5 && check_list(l2, "")
	    && check_list(cl, "147632985")),
	"foreach_rev_safe(cl) / cyclist_move_safe(l2)");

  if (blabla)
    puts("cut/roll");
  struct cyclist list2 = {0};
  DEBUG(0, check_list(l1 = cyclist_roll(&list2), ""), "roll empty usual");
  IF1_PRILIST(l1);
  list2 = (struct cyclist){&list2, &list2};
  DEBUG(0, check_list(l2 = cyclist_roll(&list2), ""), "roll empty linus");
  IF1_PRILIST(cyclist_roll(l2));
  cyclist_cut(l1, &list2, 1);
  DEBUG(0, list2.next == &list2 && list2.next == list2.prev, "cut empty linus");
  cyclist_cut(l1, &list2, 0);
  DEBUG(0, list2.next == 0 && list2.next == list2.prev, "cut empty usual");

  l1 = cl;
  l2 = cyclist_split(l1, l1->next);
  cl = cyclist_split(l2, l2->next->next);

  IF1_PRILIST(l1);
  IF1_PRILIST(l2);
  IF1_PRILIST(cl);


  check_cutroll(l1, &list2, 0, "1", "cyclist_cut/roll 1 item usual");
  check_cutroll(l1, &list2, 1, "1", "cyclist_cut/roll 1 item linus");
  check_cutroll(l2, &list2, 0, "47", "cyclist_cut/roll 2 items usual");
  check_cutroll(l2, &list2, 1, "47", "cyclist_cut/roll 2 items linus");
  check_cutroll(cl, &list2, 0, "632985", "cyclist_cut/roll 6 items usual");
  check_cutroll(cl, &list2, 1, "632985", "cyclist_cut/roll 6 items linus");
  
  if (blabla > 1) {
    PRINT_LIST(cl);
    PRINT_LIST(l1);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l2);
  }

  dl = cl->next;
  cyclist_swap(&dl, &s);
  DEBUG(0, (check_list(dl, "729856") && check_list(s, "34")),
	"again cyclist_swap in two lists");
  if (blabla > 1) {
    PRINT_LIST(cl);
    PRINT_LIST(l1);
    PRINT_LIST(s);
    PRINT_LIST(dl);
    PRINT_LIST(l2);
  }
  
  return puts("End") == EOF;
}