# Cyclist

Circular doubly linked list with a single pointer list head.

This is an attempt to answer these lines from the
[Linux implementation of doubly linked list][linux-dll]:

[linux-dll]: https://github.com/torvalds/linux/blob/master/include/linux/list.h

    * Double linked lists with a single pointer list head.
    * Mostly useful for hash tables where the two pointer list head is
    * too wasteful.
    * You lose the ability to access the tail in O(1).

Is it good? I don't know.
Maybe the best practice in difficult cases would be to `cut()` such lists
to simple doubly linked lists, operate them, and finally make `roll()`.

## Author

The authorship of the [initial version][commit-1] of this library belongs to [avp], who granted the right to publish and maintain it on GitHub.

[avp]: https://github.com/avp210159
[commit-1]: https://github.com/NickVolynkin/cyclist/commit/9ebdf246ea0e3675025669bbfc5d67a140d5eab9


# API

## Circular doubly linked list included in data struct.

```c
struct cyclist {
 struct cyclist *next, *prev;
};

struct cyclist * list_header;
```

## Get the struct for this entry

```c
container_of (field_addr, type, field_name);

// synonym of container_of
list_entry (ptr, type, member);

// same as container_of, but checks that field_addr != NULL
CONTAINER_OF (field_addr, type, field_name);
```

## Iteration

### Iterate forward

```c
struct cyclist *pos, *head;
cyclist_foreach (pos, head);
cyclist_foreach_entry (pos, head, member);
cyclist_foreach_from (pos, head);
cyclist_foreach_cont (pos, head);
```

### Iterate backwards

```c
cyclist_foreach_rev (pos, head);
cyclist_foreach_rev_entry (pos, head, member);
cyclist_foreach_rev_from (pos, head);
cyclist_foreach_rev_cont (pos, head);
```

### Iterate safely from removal of a list entry

```c
struct cyclist_save s;
cyclist_foreach_safe (pos, s, head);
cyclist_foreach_from_safe (pos, s, head);
cyclist_foreach_cont_safe (pos, s, head);
cyclist_foreach_rev_safe (pos, s, head);
cyclist_foreach_rev_from_safe (pos, s, head);
cyclist_foreach_rev_cont_safe (pos, s, head);
```

## List manipulations

### General

```c
struct cyclist *cyclist, *new, *old, *head;
struct cyclist_save save;
// test whether a list is empty
cyclist_is_empty (cyclist);
// test whether a list has just one entry
cyclist_is_singular (cyclist);
// convert any entry to singular list
cyclist_singular (cyclist);
```

### Adding entries

```c
// add a new entry to tail
cyclist_add_tail (new, &head);
// add a new entry to tail in foreach_..._safe () { ... } loops
cyclist_add_tail_safe (new, &head, &save);
// add a new head entry
cyclist_add (new, &head);
// add a new entry to tail in foreach_..._safe () { ... } loops
cyclist_add_safe (new, &head, &save);
// add a new entry after old
cyclist_insert (new, old);
// add a new entry after old in foreach_..._safe () { ... } loops
cyclist_insert_safe (new, old, &save);
```

### Removing entries

```c
// delete entry from any list, make it singular
cyclist_remove (old);
// delete entry from list, correct head, make it singular
cyclist_del (old, &head);
// delete entry from list, correct head, make it singular in foreach_..._safe () { ... } loops
cyclist_del_safe (old, &head, &save);
// replace entry in list, make it singular
```

### Replacing entries

```c
cyclist_replace (new, old);
// replace entry in list and make it singular in foreach_..._safe () { ... } loops
cyclist_replace_safe (new, old, &head, &save);
```

### Moving entries

```c
cyclist_move (old, &to__add_tail);
cyclist_move_safe (old, &list1, &save1, &to, &save2);
cyclist_swap (&elem1, &elem2);
cyclist_cswap (elem1, elem2);
cyclist_swap_safe (&elem1, &list1, &save1, &elem2, &list2, &save2);
```

### Split, join, cut and roll

```c
// cut a list into two
cyclist_split (cyclist, new);
// join 2 lists
cyclist_join (cyclist, cyclist);
// cut the list to make it non-circular
// how: 0 - simple, 1 - linus
cyclist_cut (cyclist, struct cyclist *lis2head, how);
// un-cut the list to make it circular
cyclist_roll (struct cyclist *lis2head);
```
