// avp 2016 http://pastebin.com/TGdUA9Se
#ifndef _CYCLIST_H
#define _CYCLIST_H

/* 
   
   Circular doubly linked list with a single pointer list head:
   the attempt of answer to line 592 in 
   https://github.com/torvalds/linux/blob/master/include/linux/list.h
   
589  * Double linked lists with a single pointer list head.
590  * Mostly useful for hash tables where the two pointer list head is
591  * too wasteful.
592  * You lose the ability to access the tail in O(1).

   API

   Circular doubly linked list included in data struct.
   struct cyclist { 
     struct cyclist *next, *prev;
   };

   struct cyclist * list_header;

   get the struct for this entry macros:
   container_of (field_addr, type, field_name) 
   CONTAINER_OF (field_addr, type, field_name)
   list_entry (ptr, type, member)

   iterate over list forward macros:
     struct cyclist *pos, *head;
   cyclist_foreach (pos, head)
   cyclist_foreach_entry (pos, head, member)
   cyclist_foreach_from (pos, head)
   cyclist_foreach_cont (pos, head)

   iterate over list backward macros:
   cyclist_foreach_rev (pos, head)
   cyclist_foreach_rev_entry (pos, head, member)
   cyclist_foreach_rev_from (pos, head)
   cyclist_foreach_rev_cont (pos, head)

   iterate over a list safe against removal of list entry macros:
     struct cyclist_save s;
   cyclist_foreach_safe (pos, s, head)
   cyclist_foreach_from_safe (pos, s, head)
   cyclist_foreach_cont_safe (pos, s, head)
   cyclist_foreach_rev_safe (pos, s, head)
   cyclist_foreach_rev_from_safe (pos, s, head)
   cyclist_foreach_rev_cont_safe (pos, s, head)

   list manipulation functions:
     struct cyclist *cyclist, *new, *old, *head;
     struct cyclist_save save;
   cyclist_is_empty (cyclist) tests whether a list is empty
   cyclist_is_singular (cyclist) tests whether a list has just one entry
   cyclist_singular (cyclist)  convert any entry to singular list 
   cyclist_add_tail (new, &head) add a new entry to tail
   cyclist_add_tail_safe (new, &head, &save) add a new entry to tail
                                        in foreach_..._safe () { ... } loops
   cyclist_add (new, &head) add a new head entry
   cyclist_add_safe (new, &head, &save) add a new entry to tail
                                        in foreach_..._safe () { ... } loops
   cyclist_insert (new, old) add a new entry after old
   cyclist_insert_safe (new, old, &save) add a new entry after old 
                                        in foreach_..._safe () { ... } loops
   cyclist_remove (old) delete entry from any list, make it singular
   cyclist_del (old, &head) delete entry from list, correct head, 
                          make it singular
   cyclist_del_safe (old, &head, &save) delete entry from list, correct head, 
                          make it singular in foreach_..._safe () { ... } loops
   cyclist_replace (new, old) replace entry in list, make it singular
   cyclist_replace_safe (new, old, &head, &save) replace entry in list and
                      make it singular in foreach_..._safe () { ... } loops
   cyclist_move (old, &to__add_tail)
   cyclist_move_safe (old, &list1, &save1, &to, &save2)
   cyclist_swap (&elem1, &elem2)
   cyclist_cswap (elem1, elem2)
   cyclist_swap_safe (&elem1, &list1, &save1, &elem2, &list2, &save2)
   cyclist_split (cyclist, new) cut a list into two
   cyclist_join (cyclist, cyclist) join 2 lists 
   cyclist_cut (cyclist, struct cyclist *lis2head, how) how: 0 - simple, 1 - linus
   cyclist_roll (struct cyclist *lis2head) 

   TODO
+-   debug, 
-   doc?

   Is it good? I don't know. 
   May be the best practice in difficult cases will be to cut() such lists 
   to simple double linked lists, operate with them, and make roll() 
   after all.
 */

// circular list, embedded in your data
struct cyclist {
  struct cyclist *next, *prev;
};

// used as temporary storage in cyclist_foreach_..._safe macros
struct cyclist_save {
  struct cyclist *next, /* next item  for cyclist iterations
			 if p is current item, then .next = p->next for forward 
			 and .next = p->prev for backwark
		       */
    *savehead; /* when we iterate over list forward and delete current item,
	      the list head moves forward too and overtake current item,
	      so for breaking loop we need to compare (.next) and 
	      loop endpoint (head) with previous head position,
	      and that position saved in this field (updates every step)
	    */
  int direction; // 1 forward, -1 backward, 0 stop iterations
#define S_FORWARD  1
#define S_BACKWARD -1
};

/**
 * Get offset of a structure member
 */
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * Casts a member of a structure out to the containing structure
 * container_of, CONTAINER_OF, list_entry - get the struct for this entry
 * @ptr:	the &struct cyclist pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the l2item_struct within the struct.
 */
#ifndef  container_of
#define container_of(field_addr,type,field_name)			\
  ((type *)((char *)field_addr -					\
	    (char *)&((type *)0)->field_name))
#endif // container_of
#ifndef list_entry
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)
#endif // list_entry
#ifndef CONTAINER_OF
// more safe version of container_of 
#define CONTAINER_OF(field_addr,type,field_name)			\
  ((field_addr) ? container_of(field_addr,type,field_name) : 0)
#endif

// internal macros for cyclist forward/backward iterations
#define _CL_STEP(newpos, head, endpos)			\
  (((head) && (newpos) && (endpos)) ? ((newpos) == (endpos) ? 0 : (newpos)) : 0)
#define _CL_SAFESTEP_BACK(h, p, ss)				\
  ({ if ((h) && (ss).direction && (p = (ss).next)) {		\
      (ss).savehead = (h); (ss).next = (ss).next->prev;		\
      if ((ss).next == (h)->prev)				\
	(ss).next = 0; } else if (!(ss).direction) p = 0; p; })


/**
 * cyclist_foreach - iterate over a list
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @head:	the head for your list.
 */
#define cyclist_foreach(pos, head) \
  for (pos = head; pos; pos = _CL_STEP(pos->next, head, head))

/**
 * cyclist_foreach_entry - iterate over a list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list (cyclist *, not type *)!!!
 * @member:	the name of the cyclist within the struct.
 */
#define cyclist_foreach_entry(pos, head, member)				\
  for (pos = CONTAINER_OF(head, __typeof__(*pos), member);		\
       pos;								\
       pos = (head) ? (pos->member.next == (head) ? 0			\
		       : list_entry(pos->member.next,			\
				    __typeof__(*pos), member)) : 0)

/**
 * cyclist_foreach_from	- iterate over a list from the given point
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @head:	the head for your list.
 * 
 * Start to iterate over list  from @pos point
 */
#define cyclist_foreach_from(pos, head) \
  for (; pos; pos = _CL_STEP(pos->next, head, head))

/**
 * cyclist_foreach_cont - continue list iteration from the current point
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @head:	the head for your list.
 *
 * Start to iterate over list, continuing  after @pos point
 */
#define cyclist_foreach_cont(pos, head)				\
  for (pos = pos ? _CL_STEP(pos->next, head, head) : 0;		\
       pos; pos = _CL_STEP(pos->next, head, head))

/**
 * cyclist_foreach_rev - iterate over a list backwards 
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @head:	the head for your list.
 */
#define cyclist_foreach_rev(pos, head)			\
  for (pos = head ? (head)->prev : 0; pos;		\
       pos = _CL_STEP(pos->prev, head, head->prev))

/**
 * cyclist_foreach_rev_entry - iterate over a list of given type backwards 
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list (cyclist *, not type *)!!!
 * @member:	the name of the cyclist within the struct.
 */
#define cyclist_foreach_rev_entry(pos, head, member)			\
  for (pos = (head) ? list_entry((head)->prev,__typeof__(*pos), member) : 0; \
       pos;								\
       pos = (head) ? (pos->member.prev == (head)->prev ? 0		\
		       : list_entry(pos->member.prev,			\
				    __typeof__(*pos), member)) : 0)

/**
 * cyclist_foreach_rev_from - iterate backwards over a list from the given point
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @head:	the head for your list.
 * 
 * Start to iterate backwards over list  from @pos point
 */
#define cyclist_foreach_rev_from(pos, head)			\
  for (; pos; pos = _CL_STEP(pos->prev, head, head->prev))

/**
 * cyclist_foreach_rev_cont - continue list iteration from the current point
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @head:	the head for your list.
 *
 * Start to iterate backwards over list, continuing  after @pos point
 */
#define cyclist_foreach_rev_cont(pos, head)			\
  for (pos = pos ? _CL_STEP(pos->prev, head, head->prev) : 0;	\
       pos; pos = _CL_STEP(pos->prev, head, head->prev))

/**
 * cyclist_foreach_safe - iterate over a list, 
 *              safe against removal of list entry
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @s:		struct cyclist_save to use as temporary storage
 * @head:	the head for your list. 
 */
#define cyclist_foreach_safe(pos, s, head)				\
  for (s.direction = S_FORWARD, s.savehead = pos = (head),		\
	 s.next = pos ? pos->next : 0; head && pos;			\
       pos = ((!s.direction || s.next == s.savehead) ? 0 : s.next),	\
	 pos ? s.next = s.next->next, s.savehead = (head) : 0)

/**
 * cyclist_foreach_from_safe - iterate over list from given point, 
 *              safe against removal list entry
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @s:		struct cyclist_save to use as temporary storage
 * @head:	the head for your list.
 */
#define cyclist_foreach_from_safe(pos, s, head)				\
  for (s.direction = S_FORWARD, s.savehead = head,			\
	 s.next = pos ? pos->next : 0; head && pos;			\
       pos = ((!s.direction || s.next == s.savehead) ? 0 : s.next),	\
	 pos ? s.next = s.next->next, s.savehead = (head) : 0)

/**
 * cyclist_foreach_cont_safe - continue list iteration from the current point, 
 *              safe against removal list entry
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @s:		struct cyclist_save to use as temporary storage
 * @head:	the head for your list.
 */
#define cyclist_foreach_cont_safe(pos, s, head)				\
  for (s.direction = S_FORWARD, s.savehead = (head),			\
	 s.next = (pos = pos ? _CL_STEP(pos->next, head, head) : 0) ?	\
	 pos->next : 0;							\
       head && pos;							\
       pos = ((!s.direction || s.next == s.savehead) ? 0 : s.next),	\
	 pos ? s.next = s.next->next, s.savehead = (head) : 0)

/**
 * cyclist_foreach_rev_safe - iterate over a list backwards, 
 *              safe against removal list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @s:		struct cyclist_save to use as temporary storage
 * @head:	the head for your list.
 */
#define cyclist_foreach_rev_safe(pos, s, head)				\
  for (s.direction = S_BACKWARD, s.savehead = (head),			\
	 pos = (head) ? (head)->prev : 0,				\
	 s.next = pos ? pos == (head)? 0 : pos->prev : 0;		\
       (head) && pos;							\
       _CL_SAFESTEP_BACK(head, pos, s))

/**
 * cyclist_foreach_rev_from_safe - iterate over a list backwards from given 
 *              point, safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @s:		struct cyclist_save to use as temporary storage
 * @head:	the head for your list.
 */
#define cyclist_foreach_rev_from_safe(pos, s, head)			\
  for (s.direction = S_BACKWARD, s.savehead = (head),			\
	 s.next = pos ? pos == (head)? 0 : pos->prev : 0;		\
       (head) && pos;							\
       _CL_SAFESTEP_BACK(head, pos, s))

/**
 * cyclist_foreach_rev_cont_safe - continue list iteration backwards,
 *              safe against removal list entry
 * @pos:	the &struct cyclist to use as a loop cursor.
 * @s:		struct cyclist_save to use as temporary storage
 * @head:	the head for your list.
 */
#define cyclist_foreach_rev_cont_safe(pos, s, head)			\
  for (s.direction = S_BACKWARD, s.savehead = (head),			\
	 pos = pos ? _CL_STEP(pos->prev, head, head->prev) : 0,		\
	 s.next = pos ? pos == (head)? 0 : pos->prev : 0;		\
       (head) && pos;							\
       _CL_SAFESTEP_BACK(head, pos, s))

/**
 * cyclist_is_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int cyclist_is_empty (struct cyclist *head)
{
  return !head;
}

/**
 * cyclist_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int cyclist_is_singular (struct cyclist *head)
{
  return head && head->next == head;
}

/**
 * cyclist_singular - convert any entry to singular list 
 * @entry: the item 
 */
static inline struct cyclist * cyclist_singular (struct cyclist *entry) {
  return entry ? ((entry)->next = (entry)->prev = (entry)) : 0;
}

/**
 * cyclist_add_tail - add a new entry to tail
 * @newe: new entry to be added
 * @head: list head to add item before it
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 * Returns: @newe
 */
static inline struct cyclist *
cyclist_add_tail (struct cyclist *newe, struct cyclist **head)
{
  if (!newe || !head)
    return 0;
  if (cyclist_is_empty(*head)) {
    newe->prev = newe->next = newe;
    *head = newe;
  } else {
    newe->next = *head;
    newe->prev = (*head)->prev;
    (*head)->prev->next = newe;
    (*head)->prev = newe;
  }
  return newe;
}

/**
 * cyclist_add_tail_safe - add a new entry to tail safty for iteration forward
 * @newe: new entry to be added
 * @head: list head to add item before it
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 * Returns: @newe
 */
static inline struct cyclist *
cyclist_add_tail_safe (struct cyclist *newe, struct cyclist **head, 
		     struct cyclist_save *s)
{
  if (!newe || !head)
    return 0;
  if (cyclist_is_empty(*head)) {
    newe->prev = newe->next = newe;
    *head = newe;
  } else {
    newe->next = *head;
    newe->prev = (*head)->prev;
    (*head)->prev->next = newe;
    (*head)->prev = newe;
    if (s && s->direction == S_FORWARD && s->next == *head)
      s->next = newe;
  }
  return newe;
}


/**
 * cyclist_add - add a new head entry
 * @newe: new entry to be added
 * @head: list head to add item after it
 *
 * Insert a new entry head before the specified head.
 * This is good for implementing stacks.
 * Returns: @newe
 */
static inline struct cyclist *
cyclist_add (struct cyclist *newe, struct cyclist **head)
{
  return cyclist_add_tail(newe, head) ? *head = newe : 0;
}

/**
 * cyclist_add_safe - add a new head entry, safety for iteration backward
 * @newe: new entry to be added
 * @head: list head to add it after
 * @s:    pointer to struct cyclist_save which used as the @head list iterator 
 *
 * Insert a new entry head before the specified head.
 * Returns: @newe
 */
static inline struct cyclist *
cyclist_add_safe (struct cyclist *newe, 
		  struct cyclist **head, struct cyclist_save *s)
{
  if (newe && head) {
    cyclist_add_tail(newe, head);
    if (s && s->direction == S_BACKWARD && s->next == 0)
      s->next = newe;
    return *head = newe;
  }
  return 0;
}

/**
 * cyclist_insert - add a new entry after old
 * @newe: new entry to be added
 * @old: existed list entry
 *
 * Insert a new entry after the specified one.
 * Returns: @newe
 */
static inline struct cyclist *
cyclist_insert (struct cyclist *newe, struct cyclist *old)
{
  if (old && newe) {
    newe->next = old->next;
    old->next = newe;
    newe->prev = old;
    newe->next->prev = newe;
    return newe;
  } else
    return 0;
}

/**
 * cyclist_insert_safe - add a new entry after old 
 *                     in foreach_..._safe () { ... }
 * @newe: new entry to be added
 * @old:  existed list entry
 * @s:    pointer to struct cyclist_save which used as the @old list iterator 
 *
 * Insert a new entry after the specified one,
 * correct iterator to not miss new entry.
 * Returns: @newe
 */
static inline struct cyclist *
cyclist_insert_safe (struct cyclist *newe, 
		     struct cyclist *old, struct cyclist_save *s)
{
  if (old && newe) {
    newe->next = old->next;
    old->next = newe;
    newe->prev = old;
    newe->next->prev = newe;
    if (s && 
	((s->direction == S_FORWARD && s->next == newe->next) ||
	 (s->direction == S_BACKWARD && s->next == old)))
      s->next = newe;
    return newe;
  } else
    return 0;
}

/**
 * cyclist_remove - delete entry from any list and make it singular.
 * @entry: entry to be deleted
 *
 * If entry is the only head of the list, then other list items will be lost
 * Returns: next entry or NULL if deleted entry was singular list
 */
static inline struct cyclist * cyclist_remove (struct cyclist *entry)
{
  struct cyclist *ret = 0;
  if (entry) {
    ret = entry->next;

    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    cyclist_singular(entry);
  }
  return ret == entry ? 0 : ret;
}

/**
 * cyclist_del - delete entry from list and make it singular.
 * @entry: entry to be deleted
 * @head:  list head to delete it 
 *
 * If entry is the head of the list, then next entry became new head
 * Returns: @entry or NULL if cyclist_is_empty(@head)
 */
static inline struct cyclist *
cyclist_del (struct cyclist *entry, struct cyclist **head)
{
  if (!head || !(entry && *head))
    return 0;
  if (*head) {
    struct cyclist *next_entry = cyclist_remove(entry);
    if (*head == entry)
      *head = next_entry;
  }
  return entry;
}

/**
 * cyclist_del_safe - delete entry from list and make it singular
 *                  in foreach_..._safe () { ... }
 * @entry: entry to be deleted
 * @head:  list head to delete it 
 * @s:     pointer to struct cyclist_save which used as the @head list iterator 
 *
 * If entry is the head of the list, then next entry became new head
 * correct iterator to not take deleted entry.
 * Returns: @entry or NULL if cyclist_is_empty(@head)
 */
static inline struct cyclist *
cyclist_del_safe (struct cyclist *entry, 
		  struct cyclist **head, struct cyclist_save *s)
{
  if (entry) {
    if (head && *head) {
      if (s && s->direction && entry && s->next == entry) {
	if (*head == entry)
	  s->direction = 0;
	else
	  s->next = (s->direction == S_FORWARD) ? entry->next : entry->prev;
      }
      struct cyclist *next_entry = cyclist_remove(entry);
      if (*head == entry)
	//	*head = next_entry;
#if 1	
	if ((*head = next_entry) == 0 && s)
	  //	  if (s)
	    s->direction = 0;
#endif      
    } else {
      cyclist_remove(entry);
      return 0;
    }
  }

  return entry;
}

/**
 * cyclist_replace - replace entry in list and make it singular.
 * @newe:  new entry 
 * @entry: entry to be deleted
 * @head:  head of list, containing @entry
 *
 * If @entry is the head of the list, then new entry became new head
 * Returns: @newe->next after replace
 */
static inline struct cyclist *
cyclist_replace (struct cyclist *newe, struct cyclist *entry)
{
  if (newe && entry) {
    if (cyclist_is_singular(entry))
      cyclist_singular(newe);
    else {
      *newe = *entry;
      newe->next->prev = newe;
      newe->prev->next = newe;
      cyclist_singular(entry);
    }
    
    return newe->next;
  }
  return 0;
}

/**
 * cyclist_replace_safe - replace entry in list and make it singular
 *                  in foreach_..._safe () { ... }
 * @newe:  new entry
 * @entry: entry to be deleted
 * @head:  list head to replace it 
 * @s:     pointer to struct cyclist_save which used as the @head list iterator 
 *
 * If @entry is the head of the list, then new entry became new head
 * correct iterator to take replaced entry.
 * Returns: @newe->next after replace
 */
static inline struct cyclist *
cyclist_replace_safe (struct cyclist *newe, struct cyclist *entry,
		    struct cyclist **head, struct cyclist_save *s)

{
  if (newe && entry && head) {
    if (cyclist_is_empty(*head))
      return cyclist_add(newe, head);
    cyclist_replace(newe, entry);
    if (*head == entry)
      *head = newe;
    if (s && s->next == entry)
      s->next = newe;
    return newe->next;
  }
  return 0;
}

/**
 * cyclist_move - move entry 
 * @entry:  list entry to move
 * @before: list (may be another list) entry to move @entry before it
 *
 * If *@before == NULL move @entry to this new list
 * Returns: initial @entry->next or NULL if moved entry was singular list
 */
static inline struct cyclist *
cyclist_move (struct cyclist *entry, struct cyclist **before)
{
  if (!entry || !before)
    return 0;
  struct cyclist *ret = cyclist_is_singular(entry) ? 0 : entry->next;

  if (entry->next != *before) {
    cyclist_remove(entry);
    cyclist_add_tail(entry, before);
  }
  return ret;
}

/**
 * cyclist_move_safe - move entry while iterate over it
 * @entry:  list entry to move
 * @head:   iterated list head with @entry
 * @f:      pointer to struct cyclist_save for the @head list iterator 
 * @before: list (may be another list) entry to move @entry before it
 * @t:      pointer to struct cyclist_save for the @before list iterator 
 *
 * If *@before == NULL move @entry to this new list
 * Returns: initial @entry->next or NULL if moved entry was singular list
 */
static inline struct cyclist *
cyclist_move_safe (struct cyclist *entry, 
		   struct cyclist **head, struct cyclist_save *f,
		   struct cyclist **before, struct cyclist_save *t)
{
  if (!entry || !before)
    return 0;
  struct cyclist *ret = cyclist_is_singular(entry) ? 0 : entry->next;

  if (entry->next != *before) {
    cyclist_del_safe(entry, head, f);
    cyclist_add_tail_safe(entry, before, t);
  }
  return ret;
}

#define _CL_SWAP(a, b) ({ __typeof__(a) t = a; a = b; b = t; })
/**
 * cyclist_swap - swap 2 elements and its pointers in one or different lists
 * @px:     addr of pointer to first cyclist element
 * @py:     addr of pointer to other cyclist element
 *
 * Content of pointers swaps too, so if @px pointed to `E1` in `list1` 
 * and @py pointed to `E2` in `list2`, then after swap 
 * @px pointed to `E2` in list1 and @py pointed to `E1` in list2,
 * or in other words, pointers here acts as indices in arrays.
 */
static inline void cyclist_swap (struct cyclist **px, struct cyclist **py)
{
  if (!(*px && *py))
    return;
  struct cyclist *x = *px, *y = *py;
  int sx = cyclist_is_singular(x);

  _CL_SWAP(*px, *py); // swap pointers

  if ((sx && cyclist_is_singular(y)) // both are singular
      || (x->next == y && y->next == x)) // 2-items list: x -> y
    return;

  if (sx || cyclist_is_singular(y)) {
    if (sx) // x singular, y not
      cyclist_replace(x, y);
    else    // y singular
      cyclist_replace(y, x);
    return;
  } 

  if (x->next == y || y->next == x) { // neighbors
    /*    ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccqwwwwwwvvvvvvaaaaaaaaaaaaaaaaaazzzcccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccczzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzmmmmm,+

     */ // Desi's comment !!!
    if (y->next == x) 
      _CL_SWAP(x, y); // reoder names    
    // ... -> x -> y -> ... x before y
    y->next->prev = x;
    x->prev->next = y;
    x->next = y->next;
    y->prev = x->prev;
    x->prev = y;
    y->next = x;
  } else {
    _CL_SWAP(*x, *y); // SWAP content
    // correct neighbors links
    x->prev->next = x;
    x->next->prev = x;
    y->prev->next = y;
    y->next->prev = y;
  }
}


/**
 * cyclist_cswap - swap only elements in one or different lists
 * @x:     first cyclist element
 * @y:     other cyclist element
 */
static inline void cyclist_cswap (struct cyclist *x, struct cyclist *y)
{
  cyclist_swap(&x, &y);
}

/**
 * cyclist_swap_safe - swap 2 elements in one or different lists
 * @px:     addr of pointer to first cyclist element
 * @xhead:  iterated list head with @px
 * @sx:     pointer to struct cyclist_save for the @xhead list iterator 
 * @py:     addr of pointer to other cyclist element
 * @yhead:  iterated list head with @py
 * @sy:     pointer to struct cyclist_save for the @yhead list iterator 
 *
 * lists headers and iterators may be NULL
 */
static inline void 
cyclist_swap_safe (struct cyclist **px, 
		   struct cyclist **xhead, struct cyclist_save *sx,
		   struct cyclist **py, 
		   struct cyclist **yhead, struct cyclist_save *sy)
{
#define CORRECT(ptr, pval, x, y) if (ptr && pval == x) pval = y;
  CORRECT(xhead, *xhead, *px, *py);
  CORRECT(yhead, *yhead, *py, *px);
  CORRECT(sx, sx->next, *px, *py);
  CORRECT(sy, sy->next, *py, *px);
#undef CORRECT
  cyclist_swap(px, py);
}

/**
 * cyclist_split - cut a list into two
 * @head: source list
 * @newl: entry in source list, cut point
 *
 * New list contains entries from @newl to @head->prev (inclusive)
 * Old list contains entries from @head to @newl->prev (inclusive)
 * Returns: new list (@newl) or NULL, if @head == @newl
 */
static inline struct cyclist *
cyclist_split (struct cyclist *head, struct cyclist *newl)
{
  struct cyclist *ret = 0, *tail;
  if (head && newl && head != newl) {
    ret = newl;
    tail = newl->prev;
    newl->prev = head->prev;
    newl->prev->next = newl;
    head->prev = tail;
    tail->next = head;
  }
  return ret;
}

/**
 * cyclist_join - join 2 lists 
 * @head: old list
 * @newl: list append to the tail of old
 *
 * Note: @newl must be not in old list
 * Returns: joined list (@head)
 */
static inline struct cyclist *
cyclist_join (struct cyclist *head, struct cyclist *newl)
{
  struct cyclist *tail;
  if (head && newl && head != newl) {
    tail = head->prev;
    tail->next = newl;
    head->prev = newl->prev;
    newl->prev = tail;
    head->prev->next = head;
  }
  return head;
}

/**
 * cyclist_cut - convert circular list to linear 2-links list
 * @head:    cyclist element
 * @l2head:  pointer to usual (or linux) double linked list head {head, tail} 
 * @how:     0 -- make usual 2-linked list, other -- make linux/list.h list
 *
 * Returns:  pointer to first item in new 2-linked list
 */
static inline struct cyclist *cyclist_cut (struct cyclist *head, 
					   struct cyclist *l2head, int how)
{
  struct cyclist *ptr = how ? l2head : 0;

  if (head) {
    l2head->next = head;
    l2head->prev = head->prev;
    head->prev = ptr;
    l2head->prev->next = ptr;
  } else if (ptr)
    l2head->next = l2head->prev = ptr;
  else
    *l2head = (__typeof__ (*l2head)){0};

  return head;
}

/**
 * cyclist_roll make circular list from linear 2-links list
 * @l2head:  pointer to usual (or linux) double linked list head {head, tail} 
 *
 * Returns:  pointer to first item in new cyclist
 */
static inline struct cyclist *cyclist_roll (struct cyclist *l2head)
{
  if (l2head && l2head->next && l2head->next != l2head) {
    l2head->next->prev = l2head->prev;
    return l2head->prev->next = l2head->next;
  } else
    return 0;
}

#endif