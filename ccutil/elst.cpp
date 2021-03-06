/**********************************************************************
 * File:        elst.c  (Formerly elist.c)
 * Description: Embedded list handling code which is not in the include file.
 * Author:      Phil Cheatle
 * Created:     Fri Jan 04 13:55:49 GMT 1991
 *
 * (C) Copyright 1991, Hewlett-Packard Ltd.
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 ** http://www.apache.org/licenses/LICENSE-2.0
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 *
 **********************************************************************/

#include          "mfcpch.h"     //precompiled headers
#include <stdlib.h>
#include "elst.h"

/***********************************************************************
 *  MEMBER FUNCTIONS OF CLASS: ELIST_LINK
 *  =====================================
 **********************************************************************/

/***********************************************************************
 *							ELIST_LINK::serialise_asc
 *
 *  Generates an error as it should never be called.
 **********************************************************************/

void ELIST_LINK::serialise_asc(  //default serialise
                               FILE *f) {
  SERIALISE_LINKS.error ("ELIST_LINK::serialise_asc", ABORT,
    "Don't call this, override!");
}


void ELIST_LINK::de_serialise_asc(  //default de_serialise
                                  FILE *f) {
  SERIALISE_LINKS.error ("ELIST_LINK::de_serialise_asc", ABORT,
    "Don't call this, override!");
}


/***********************************************************************
 *  MEMBER FUNCTIONS OF CLASS: ELIST
 *  ================================
 **********************************************************************/

/***********************************************************************
 *							ELIST::internal_clear
 *
 *  Used by the destructor and the "clear" member function of derived list
 *  classes to destroy all the elements on the list.
 *  The calling function passes a "zapper" function which can be called to
 *  delete each element of the list, regardless of its derived type.  This
 *  technique permits a generic clear function to destroy elements of
 *  different derived types correctly, without requiring virtual functions and
 *  the consequential memory overhead.
 **********************************************************************/

void
ELIST::internal_clear (          //destroy all links
void (*zapper) (ELIST_LINK *)) {
                                 //ptr to zapper functn
  ELIST_LINK *ptr;
  ELIST_LINK *next;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::internal_clear", ABORT, NULL);
  #endif

  if (!empty ()) {
    ptr = last->next;            //set to first
    last->next = NULL;           //break circle
    last = NULL;                 //set list empty
    while (ptr) {
      next = ptr->next;
      zapper(ptr);
      ptr = next;
    }
  }
}


/***********************************************************************
 *							ELIST::internal_deep_copy
 *
 *  Used during explict deep copy of a list.  The "copier" function passed
 *  allows each element to be correctly deep copied (assuming that each class
 *  in the inheritance hierarchy does properly deep copies its members).  The
 *  function passing technique is as for "internal_clear".
 **********************************************************************/

void
                                 //ptr to copier functn
ELIST::internal_deep_copy (ELIST_LINK * (*copier) (ELIST_LINK *),
const ELIST * list) {            //list being copied
  ELIST_ITERATOR from_it ((ELIST *) list);
  ELIST_ITERATOR to_it(this);

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::internal_deep_copy", ABORT, NULL);
  if (!list)
    BAD_PARAMETER.error ("ELIST::internal_deep_copy", ABORT,
      "source list is NULL");
  #endif

  for (from_it.mark_cycle_pt (); !from_it.cycled_list (); from_it.forward ())
    to_it.add_after_then_move (copier (from_it.data ()));
}


/***********************************************************************
 *							ELIST::assign_to_sublist
 *
 *  The list is set to a sublist of another list.  "This" list must be empty
 *  before this function is invoked.  The two iterators passed must refer to
 *  the same list, different from "this" one.  The sublist removed is the
 *  inclusive list from start_it's current position to end_it's current
 *  position.  If this range passes over the end of the source list then the
 *  source list has its end set to the previous element of start_it.  The
 *  extracted sublist is unaffected by the end point of the source list, its
 *  end point is always the end_it position.
 **********************************************************************/

void ELIST::assign_to_sublist(                           //to this list
                              ELIST_ITERATOR *start_it,  //from list start
                              ELIST_ITERATOR *end_it) {  //from list end
  const ERRCODE LIST_NOT_EMPTY =
    "Destination list must be empty before extracting a sublist";

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::assign_to_sublist", ABORT, NULL);
  #endif

  if (!empty ())
    LIST_NOT_EMPTY.error ("ELIST.assign_to_sublist", ABORT, NULL);

  last = start_it->extract_sublist (end_it);
}


/***********************************************************************
 *							ELIST::length
 *
 *  Return count of elements on list
 **********************************************************************/

inT32 ELIST::length() {  //count elements
  ELIST_ITERATOR it(this);
  inT32 count = 0;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::length", ABORT, NULL);
  #endif

  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ())
    count++;
  return count;
}


/***********************************************************************
 *							ELIST::sort
 *
 *  Sort elements on list
 *  NB If you dont like the const declarations in the comparator, coerce yours:
 *   ( int (*)(const void *, const void *)
 **********************************************************************/

void
ELIST::sort (                    //sort elements
int comparator (                 //comparison routine
const void *, const void *)) {
  ELIST_ITERATOR it(this);
  inT32 count;
  ELIST_LINK **base;             //ptr array to sort
  ELIST_LINK **current;
  inT32 i;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::sort", ABORT, NULL);
  #endif

  /* Allocate an array of pointers, one per list element */
  count = length ();
  base = (ELIST_LINK **) malloc (count * sizeof (ELIST_LINK *));

  /* Extract all elements, putting the pointers in the array */
  current = base;
  for (it.mark_cycle_pt (); !it.cycled_list (); it.forward ()) {
    *current = it.extract ();
    current++;
  }

  /* Sort the pointer array */
  qsort ((char *) base, count, sizeof (*base), comparator);

  /* Rebuild the list from the sorted pointers */
  current = base;
  for (i = 0; i < count; i++) {
    it.add_to_end (*current);
    current++;
  }
  free(base);
}


/***********************************************************************
 *							ELIST::prep_serialise
 *
 *  Replace the last member with a count of elements for serialisation.
 *  This is used on list objects which are members of objects being
 *  serialised.  The containing object has been shallow copied and this member
 *  function is invoked on the COPY.
 **********************************************************************/

void ELIST::prep_serialise() {
  ELIST_ITERATOR this_it(this);
  inT32 count = 0;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::prep_serialise", ABORT, NULL);
  #endif

  count = 0;
  if (!empty ())
    for (this_it.mark_cycle_pt ();
    !this_it.cycled_list (); this_it.forward ())
  count++;
  last = (ELIST_LINK *) count;
}


/***********************************************************************
 *							ELIST::internal_dump
 *
 *  Cause each element on the list to be serialised by walking the list and
 *  calling the element_serialiser function for each element.  The
 *  element_serialiser simply does the appropriate coercion of the element to
 *  its real type and then invokes the elements serialise function
 **********************************************************************/

void
ELIST::internal_dump (FILE * f,
void element_serialiser (FILE *, ELIST_LINK *)) {
  ELIST_ITERATOR this_it(this);

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::internal_dump", ABORT, NULL);
  #endif

  if (!empty ())
    for (this_it.mark_cycle_pt ();
    !this_it.cycled_list (); this_it.forward ())
  element_serialiser (f, this_it.data ());
}


/***********************************************************************
 *							ELIST::internal_de_dump
 *
 *  Cause each element on the list to be de_serialised by extracting the count
 *  of elements on the list, (held in the last member of the dumped version of
 *  the list object), and then de-serialising that number of list elements,
 *  adding each to the end of the reconstructed list.
 **********************************************************************/

void
ELIST::internal_de_dump (FILE * f,
ELIST_LINK * element_de_serialiser (FILE *)) {
  inT32 count = (ptrdiff_t) last;
  ELIST_ITERATOR this_it;
  ELIST_LINK *de_serialised_element;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST::internal_de_dump", ABORT, NULL);
  #endif

  last = NULL;
  this_it.set_to_list (this);
  for (; count > 0; count--) {
    de_serialised_element = element_de_serialiser (f);
                                 //ignore old ptr
    de_serialised_element->next = NULL;
    this_it.add_to_end (de_serialised_element);
  }
}


/***********************************************************************
 *  MEMBER FUNCTIONS OF CLASS: ELIST_ITERATOR
 *  =========================================
 **********************************************************************/

/***********************************************************************
 *							ELIST_ITERATOR::forward
 *
 *  Move the iterator to the next element of the list.
 *  REMEMBER: ALL LISTS ARE CIRCULAR.
 **********************************************************************/

ELIST_LINK *ELIST_ITERATOR::forward() {
  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::forward", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::forward", ABORT, NULL);
  #endif
  if (list->empty ())
    return NULL;

  if (current) {                 //not removed so
                                 //set previous
    prev = current;
    started_cycling = TRUE;
  }
  else {
    if (ex_current_was_cycle_pt)
      cycle_pt = next;
  }
  current = next;
  next = current->next;

  #ifdef _DEBUG
  if (!current)
    NULL_DATA.error ("ELIST_ITERATOR::forward", ABORT, NULL);
  if (!next)
    NULL_NEXT.error ("ELIST_ITERATOR::forward", ABORT,
      "This is: %i  Current is: %i",
      (int) this, (int) current);
  #endif
  return current;
}


/***********************************************************************
 *							ELIST_ITERATOR::data_relative
 *
 *  Return the data pointer to the element "offset" elements from current.
 *  "offset" must not be less than -1.
 *  (This function can't be INLINEd because it contains a loop)
 **********************************************************************/

ELIST_LINK *ELIST_ITERATOR::data_relative(                //get data + or - ...
                                          inT8 offset) {  //offset from current
  ELIST_LINK *ptr;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::data_relative", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::data_relative", ABORT, NULL);
  if (list->empty ())
    EMPTY_LIST.error ("ELIST_ITERATOR::data_relative", ABORT, NULL);
  if (offset < -1)
    BAD_PARAMETER.error ("ELIST_ITERATOR::data_relative", ABORT,
      "offset < -l");
  #endif

  if (offset == -1)
    ptr = prev;
  else
    for (ptr = current ? current : prev; offset-- > 0; ptr = ptr->next);

  #ifdef _DEBUG
  if (!ptr)
    NULL_DATA.error ("ELIST_ITERATOR::data_relative", ABORT, NULL);
  #endif

  return ptr;
}


/***********************************************************************
 *							ELIST_ITERATOR::move_to_last()
 *
 *  Move current so that it is set to the end of the list.
 *  Return data just in case anyone wants it.
 *  (This function can't be INLINEd because it contains a loop)
 **********************************************************************/

ELIST_LINK *ELIST_ITERATOR::move_to_last() {
  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::move_to_last", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::move_to_last", ABORT, NULL);
  #endif

  while (current != list->last)
    forward();

  return current;
}


/***********************************************************************
 *							ELIST_ITERATOR::exchange()
 *
 *  Given another iterator, whose current element is a different element on
 *  the same list list OR an element of another list, exchange the two current
 *  elements.  On return, each iterator points to the element which was the
 *  other iterators current on entry.
 *  (This function hasn't been in-lined because its a bit big!)
 **********************************************************************/

void ELIST_ITERATOR::exchange(                             //positions of 2 links
                              ELIST_ITERATOR *other_it) {  //other iterator
  const ERRCODE DONT_EXCHANGE_DELETED =
    "Can't exchange deleted elements of lists";

  ELIST_LINK *old_current;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::exchange", ABORT, NULL);
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::exchange", ABORT, NULL);
  if (!other_it)
    BAD_PARAMETER.error ("ELIST_ITERATOR::exchange", ABORT, "other_it NULL");
  if (!(other_it->list))
    NO_LIST.error ("ELIST_ITERATOR::exchange", ABORT, "other_it");
  #endif

  /* Do nothing if either list is empty or if both iterators reference the same
  link */

  if ((list->empty ()) ||
    (other_it->list->empty ()) || (current == other_it->current))
    return;

  /* Error if either current element is deleted */

  if (!current || !other_it->current)
    DONT_EXCHANGE_DELETED.error ("ELIST_ITERATOR.exchange", ABORT, NULL);

  /* Now handle the 4 cases: doubleton list; non-doubleton adjacent elements
  (other before this); non-doubleton adjacent elements (this before other);
  non-adjacent elements. */

                                 //adjacent links
  if ((next == other_it->current) ||
  (other_it->next == current)) {
                                 //doubleton list
    if ((next == other_it->current) &&
    (other_it->next == current)) {
      prev = next = current;
      other_it->prev = other_it->next = other_it->current;
    }
    else {                       //non-doubleton with
                                 //adjacent links
                                 //other before this
      if (other_it->next == current) {
        other_it->prev->next = current;
        other_it->current->next = next;
        current->next = other_it->current;
        other_it->next = other_it->current;
        prev = current;
      }
      else {                     //this before other
        prev->next = other_it->current;
        current->next = other_it->next;
        other_it->current->next = current;
        next = current;
        other_it->prev = other_it->current;
      }
    }
  }
  else {                         //no overlap
    prev->next = other_it->current;
    current->next = other_it->next;
    other_it->prev->next = current;
    other_it->current->next = next;
  }

  /* update end of list pointer when necessary (remember that the 2 iterators
    may iterate over different lists!) */

  if (list->last == current)
    list->last = other_it->current;
  if (other_it->list->last == other_it->current)
    other_it->list->last = current;

  if (current == cycle_pt)
    cycle_pt = other_it->cycle_pt;
  if (other_it->current == other_it->cycle_pt)
    other_it->cycle_pt = cycle_pt;

  /* The actual exchange - in all cases*/

  old_current = current;
  current = other_it->current;
  other_it->current = old_current;
}


/***********************************************************************
 *							ELIST_ITERATOR::extract_sublist()
 *
 *  This is a private member, used only by ELIST::assign_to_sublist.
 *  Given another iterator for the same list, extract the links from THIS to
 *  OTHER inclusive, link them into a new circular list, and return a
 *  pointer to the last element.
 *  (Can't inline this function because it contains a loop)
 **********************************************************************/

ELIST_LINK *ELIST_ITERATOR::extract_sublist(                             //from this current
                                            ELIST_ITERATOR *other_it) {  //to other current
  #ifdef _DEBUG
  const ERRCODE BAD_EXTRACTION_PTS =
    "Can't extract sublist from points on different lists";
  const ERRCODE DONT_EXTRACT_DELETED =
    "Can't extract a sublist marked by deleted points";
  #endif
  const ERRCODE BAD_SUBLIST = "Can't find sublist end point in original list";

  ELIST_ITERATOR temp_it = *this;
  ELIST_LINK *end_of_new_list;

  #ifdef _DEBUG
  if (!this)
    NULL_OBJECT.error ("ELIST_ITERATOR::extract_sublist", ABORT, NULL);
  if (!other_it)
    BAD_PARAMETER.error ("ELIST_ITERATOR::extract_sublist", ABORT,
      "other_it NULL");
  if (!list)
    NO_LIST.error ("ELIST_ITERATOR::extract_sublist", ABORT, NULL);
  if (list != other_it->list)
    BAD_EXTRACTION_PTS.error ("ELIST_ITERATOR.extract_sublist", ABORT, NULL);
  if (list->empty ())
    EMPTY_LIST.error ("ELIST_ITERATOR::extract_sublist", ABORT, NULL);

  if (!current || !other_it->current)
    DONT_EXTRACT_DELETED.error ("ELIST_ITERATOR.extract_sublist", ABORT,
      NULL);
  #endif

  ex_current_was_last = other_it->ex_current_was_last = FALSE;
  ex_current_was_cycle_pt = FALSE;
  other_it->ex_current_was_cycle_pt = FALSE;

  temp_it.mark_cycle_pt ();
  do {                           //walk sublist
    if (temp_it.cycled_list ())  //cant find end pt
      BAD_SUBLIST.error ("ELIST_ITERATOR.extract_sublist", ABORT, NULL);

    if (temp_it.at_last ()) {
      list->last = prev;
      ex_current_was_last = other_it->ex_current_was_last = TRUE;
    }

    if (temp_it.current == cycle_pt)
      ex_current_was_cycle_pt = TRUE;

    if (temp_it.current == other_it->cycle_pt)
      other_it->ex_current_was_cycle_pt = TRUE;

    temp_it.forward ();
  }
  while (temp_it.prev != other_it->current);

                                 //circularise sublist
  other_it->current->next = current;
  end_of_new_list = other_it->current;

                                 //sublist = whole list
  if (prev == other_it->current) {
    list->last = NULL;
    prev = current = next = NULL;
    other_it->prev = other_it->current = other_it->next = NULL;
  }
  else {
    prev->next = other_it->next;
    current = other_it->current = NULL;
    next = other_it->next;
    other_it->prev = prev;
  }
  return end_of_new_list;
}
