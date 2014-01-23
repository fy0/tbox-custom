/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2012, ruki All rights reserved.
 *
 * @author		ruki
 * @file		sort.c
 * @ingroup 	algorithm
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "sort.h"
#include "../libc/libc.h"

/* ///////////////////////////////////////////////////////////////////////
 * head
 */
static __tb_inline__ tb_bool_t tb_heap_check(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{
	// the comparer 
	if (!comp) comp = tb_iterator_comp;

	// walk
	if (head != tail)
	{
		tb_size_t root;
		for (root = head; ++head != tail; ++root)
		{
			// root < left?
			if (tb_iterator_comp(iterator, tb_iterator_item(iterator, root), tb_iterator_item(iterator, head)) < 0) return tb_false;
			// end?
			else if (++head == tail) break;
			// root < right?
			else if (tb_iterator_comp(iterator, tb_iterator_item(iterator, root), tb_iterator_item(iterator, head)) < 0) return tb_false;
		}
	}

	// ok
	return tb_true;
}
/*!push heap
 *
 * <pre>
 * hole: bottom => top
 * init:
 *                                          16(top)
 *                               -------------------------
 *                              |                         |
 *                              14                        10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       8(parent)      7           9             3
 *                   ---------      
 *                  |         |     
 *                  2      (hole) <= 11(val)
 * after:
 *                                          16(top)
 *                               -------------------------
 *                              |                         |
 *                              14(parent)                10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       11(hole)       7           9             3
 *                   ---------      
 *                  |         |    
 *                  2         8 
 * </pre>
 */
static __tb_inline__ tb_void_t tb_heap_push(tb_iterator_t* iterator, tb_size_t head, tb_size_t hole, tb_size_t top, tb_cpointer_t item, tb_iterator_comp_t comp)
{
	// check
	tb_assert_and_check_return(comp);

	// (hole - 1) / 2: the parent node of the hole
	// finds the final hole
	tb_size_t 		parent = 0;
	tb_cpointer_t 	parent_item = tb_null;
	for (parent = (hole - 1) >> 1; hole > top && (comp(iterator, (parent_item = tb_iterator_item(iterator, head + parent)), item) < 0); parent = (hole - 1) >> 1)
	{	
		// move item: parent => hole
//		tb_iterator_copy(iterator, head + parent, item);
		tb_iterator_copy(iterator, head + hole, parent_item);

		// move node: hole => parent
		hole = parent;
	}

	// copy item
	tb_iterator_copy(iterator, head + hole, item);
}
/*! adjust heap
 *
 * <pre>
 * init:
 *                                          16(head)
 *                               -------------------------
 *                              |                         |
 *                           (hole)                       10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       8(larger)      7           9             3
 *                   ---------       ----
 *                  |         |     |
 *                  2         4     1(tail - 1)
 *
 * after:
 *                                          16(head)
 *                               -------------------------
 *                              |                         |
 *                              8                        10
 *                        --------------             -------------
 *                       |              |           |             |
 *                      (hole)          7           9             3
 *                   ---------       ----
 *                  |         |     |
 *                  2 (larger)4     1(tail - 1)
 *
 * after:
 *                                          16(head)
 *                               -------------------------
 *                              |                         |
 *                              8                        10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       4              7           9             3
 *                   ---------       ----
 *                  |         |     |
 *                  2      (hole)   1(tail - 1)
 *
 * </pre>
 */
static __tb_inline__ tb_void_t tb_heap_adjust(tb_iterator_t* iterator, tb_size_t head, tb_size_t hole, tb_size_t tail, tb_cpointer_t item, tb_iterator_comp_t comp)
{
	// the comparer 
	if (!comp) comp = tb_iterator_comp;

#if 0
	// save top position
	tb_size_t top = hole;

	// 2 * hole + 2: the right child node of hole
	tb_size_t child = (hole << 1) + 2;
	for (; child < tail; child = (child << 1) + 2)
	{	
		// the larger child node
		if (comp(iterator, tb_iterator_item(iterator, head + child), tb_iterator_item(iterator, head + child - 1)) < 0) child--;

		// the larger child node => hole
		tb_iterator_copy(iterator, head + hole, tb_iterator_item(iterator, head + child));

		// move the hole down to it's larger child node 
		hole = child;
	}

	// no right child node? 
	if (child == tail)
	{	
		// the last child => hole
		tb_iterator_copy(iterator, head + hole, tb_iterator_item(iterator, head + tail - 1));

		// move hole down to tail
		hole = tail - 1;
	}

	// push item into the hole
	tb_heap_push(iterator, head, hole, top, item, comp);
#else

	// walk, 2 * hole + 1: the left child node of hole
	tb_size_t 		child = (hole << 1) + 1;
	tb_cpointer_t 	child_item = tb_null;
	tb_cpointer_t 	child_item_r = tb_null;
	for (; child < tail; child = (child << 1) + 1)
	{	
		// the larger child node
		child_item = tb_iterator_item(iterator, head + child);
		if (child + 1 < tail && comp(iterator, child_item, (child_item_r = tb_iterator_item(iterator, head + child + 1))) < 0) 
		{
			child++;
			child_item = child_item_r;
		}

		// end?
		if (comp(iterator, child_item, item) < 0) break;

		// the larger child node => hole
		tb_iterator_copy(iterator, head + hole, child_item);

		// move the hole down to it's larger child node 
		hole = child;
	}

	// copy item
	tb_iterator_copy(iterator, head + hole, item);

#endif
}
/*!make heap
 *
 * <pre>
 * heap:    16      14      10      8       7       9       3       2       4       1
 *
 *                                          16(head)
 *                               -------------------------
 *                              |                         |
 *                              14                        10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       8       (tail / 2 - 1)7    9             3
 *                   ---------       ----
 *                  |         |     |
 *                  2         4     1(tail - 1)
 * </pre>
 */
static __tb_inline__ tb_void_t tb_heap_make(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{
	// init
	tb_size_t 		step = tb_iterator_step(iterator);
	tb_pointer_t 	temp = step > sizeof(tb_pointer_t)? tb_malloc(step) : tb_null;
	tb_assert_and_check_return(step <= sizeof(tb_pointer_t) || temp);

	// make
	tb_size_t hole;
	tb_size_t bottom = tail - head;
	for (hole = (bottom >> 1); hole > 0; )
	{
		--hole;

		// save hole
		if (step <= sizeof(tb_pointer_t)) temp = tb_iterator_item(iterator, head + hole);
		else tb_memcpy(temp, tb_iterator_item(iterator, head + hole), step);

		// reheap top half, bottom to top
		tb_heap_adjust(iterator, head, hole, bottom, temp, comp);
	}

	// free
	if (temp && step > sizeof(tb_pointer_t)) tb_free(temp);

	// check
	tb_assert(tb_heap_check(iterator, head, tail, comp));
}
/*!pop the top of heap to last and reheap
 *
 * <pre>
 *                                          16(head) 
 *                               ----------------|--------
 *                              |                |        |
 *                              14               |        10
 *                        --------------         |   -------------
 *                       |              |        |  |             |
 *                       8              7        |  9             3
 *                   ---------       ----        |
 *                  |         |     |            |
 *                  2         4     1(last)<-----
 *                                (hole)
 * </pre>
 */   
static __tb_inline__ tb_void_t tb_heap_pop0(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_cpointer_t item, tb_iterator_comp_t comp)
{
	// top => last
	tb_iterator_copy(iterator, tail - 1, tb_iterator_item(iterator, head));

	// reheap it
	tb_heap_adjust(iterator, head, 0, tail - head - 1, item, comp);

	// check
//	tb_assert(tb_heap_check(iterator, head, tail - head - 1, comp));
}

/* ///////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_void_t tb_sort(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{
	// check
	tb_check_return(iterator && head != tail);

	// random access iterator? 
	if (iterator->mode & TB_ITERATOR_MODE_RACCESS) 
	{
		if (tail > head + 100000) tb_heap_sort(iterator, head, tail, comp);
		else tb_quick_sort(iterator, head, tail, comp); //!< @note the recursive stack size is limit
	}
	else tb_bubble_sort(iterator, head, tail, comp);
}
tb_void_t tb_sort_all(tb_iterator_t* iterator, tb_iterator_comp_t comp)
{
	tb_sort(iterator, tb_iterator_head(iterator), tb_iterator_tail(iterator), comp);
}
tb_void_t tb_bubble_sort(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{
	// check
	tb_assert_and_check_return(iterator && iterator->mode & TB_ITERATOR_MODE_FORWARD);
	tb_check_return(head != tail);

	// init
	tb_size_t 		step = tb_iterator_step(iterator);
	tb_pointer_t 	temp = step > sizeof(tb_pointer_t)? tb_malloc(step) : tb_null;
	tb_assert_and_check_return(step <= sizeof(tb_pointer_t) || temp);

	// the comparer
	if (!comp) comp = tb_iterator_comp;

	// sort
	tb_size_t 		itor1, itor2;
	for (itor1 = head; itor1 != tail; itor1 = tb_iterator_next(iterator, itor1))
	{
		for (itor2 = itor1, itor2 = tb_iterator_next(iterator, itor2); itor2 != tail; itor2 = tb_iterator_next(iterator, itor2))
		{
			if (comp(iterator, tb_iterator_item(iterator, itor2), tb_iterator_item(iterator, itor1)) < 0)
			{
				if (step <= sizeof(tb_pointer_t)) temp = tb_iterator_item(iterator, itor1);
				else tb_memcpy(temp, tb_iterator_item(iterator, itor1), step);
				tb_iterator_copy(iterator, itor1, tb_iterator_item(iterator, itor2));
				tb_iterator_copy(iterator, itor2, temp);
			}
		}
	}

	// free
	if (temp && step > sizeof(tb_pointer_t)) tb_free(temp);
}
tb_void_t tb_bubble_sort_all(tb_iterator_t* iterator, tb_iterator_comp_t comp)
{
	tb_bubble_sort(iterator, tb_iterator_head(iterator), tb_iterator_tail(iterator), comp);
}

/*!the insertion sort
 *
 * <pre>
 * old:     5       2       6       2       8       6       1
 *
 *        (hole)
 * step1: ((5))     2       6       2       8       6       1
 *        (next) <=
 *
 *        (hole)  
 * step2: ((2))    (5)      6       2       8       6       1
 *                (next) <=
 *
 *                        (hole)
 * step3:   2       5     ((6))     2       8       6       1
 *                        (next) <=
 *
 *                 (hole)       
 * step4:   2      ((2))   (5)     (6)      8       6       1
 *                                (next) <=
 *
 *                                        (hole)
 * step5:   2       2       5       6     ((8))     6       1
 *                                        (next) <=
 *
 *                                        (hole) 
 * step6:   2       2       5       6     ((6))    (8)       1
 *                                                (next) <=
 *
 *        (hole)                                         
 * step7: ((1))    (2)     (2)     (5)     (6)     (6)      (8)       
 *                                                        (next)
 * </pre>
 */
tb_void_t tb_insert_sort(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{	
	// check
	tb_assert_and_check_return(iterator && iterator->mode & TB_ITERATOR_MODE_BDIRECT);
	tb_check_return(head != tail);
	
	// init
	tb_size_t 		step = tb_iterator_step(iterator);
	tb_pointer_t 	temp = step > sizeof(tb_pointer_t)? tb_malloc(step) : tb_null;
	tb_assert_and_check_return(step <= sizeof(tb_pointer_t) || temp);

	// the comparer
	if (!comp) comp = tb_iterator_comp;

	// sort
	tb_size_t last, next;
	for (next = tb_iterator_next(iterator, head); next != tail; next = tb_iterator_next(iterator, next))
	{
		// save next
		if (step <= sizeof(tb_pointer_t)) temp = tb_iterator_item(iterator, next);
		else tb_memcpy(temp, tb_iterator_item(iterator, next), step);

		// look for hole and move elements[hole, next - 1] => [hole + 1, next]
		for (last = next; last != head && (last = tb_iterator_prev(iterator, last), comp(iterator, temp, tb_iterator_item(iterator, last)) < 0); next = last)
				tb_iterator_copy(iterator, next, tb_iterator_item(iterator, last));

		// item => hole
		tb_iterator_copy(iterator, next, temp);
	}

	// free
	if (temp && step > sizeof(tb_pointer_t)) tb_free(temp);
}
tb_void_t tb_insert_sort_all(tb_iterator_t* iterator, tb_iterator_comp_t comp)
{
	tb_insert_sort(iterator, tb_iterator_head(iterator), tb_iterator_tail(iterator), comp);
}
tb_void_t tb_quick_sort(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{	
	// check
	tb_assert_and_check_return(iterator && iterator->mode & TB_ITERATOR_MODE_RACCESS);
	tb_check_return(head != tail);

	// init
	tb_size_t 		step = tb_iterator_step(iterator);
	tb_pointer_t 	key = step > sizeof(tb_pointer_t)? tb_malloc(step) : tb_null;
	tb_assert_and_check_return(step <= sizeof(tb_pointer_t) || key);

	// the comparer
	if (!comp) comp = tb_iterator_comp;

	// hole => key
	if (step <= sizeof(tb_pointer_t)) key = tb_iterator_item(iterator, head);
	else tb_memcpy(key, tb_iterator_item(iterator, head), step);

	// sort
	tb_size_t l = head;
	tb_size_t r = tail - 1;
	while (r > l)
	{
		// find: <= 
		for (; r != l; r--)
			if (comp(iterator, tb_iterator_item(iterator, r), key) < 0) break;
		if (r != l) 
		{
			tb_iterator_copy(iterator, l, tb_iterator_item(iterator, r));
			l++;
		}

		// find: =>
		for (; l != r; l++)
			if (comp(iterator, tb_iterator_item(iterator, l), key) > 0) break;
		if (l != r) 
		{
			tb_iterator_copy(iterator, r, tb_iterator_item(iterator, l));
			r--;
		}
	}

	// key => hole
	tb_iterator_copy(iterator, l, key);

	// sort [head, hole - 1]
	tb_quick_sort(iterator, head, l, comp);

	// sort [hole + 1, tail]
	tb_quick_sort(iterator, ++l, tail, comp);

	// free
	if (key && step > sizeof(tb_pointer_t)) tb_free(key);
}
tb_void_t tb_quick_sort_all(tb_iterator_t* iterator, tb_iterator_comp_t comp)
{
	tb_quick_sort(iterator, tb_iterator_head(iterator), tb_iterator_tail(iterator), comp);
}
/*!the heap sort 
 * 
 * <pre>
 * init:
 *
 *                                           16(head)
 *                               -------------------------
 *                              |                         |
 *                              4                         10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       14             7           9             3
 *                   ---------       ----
 *                  |         |     |
 *                  2         8     1(last - 1)
 * 
 * make_heap:
 *
 *                                           16(head)
 *                               -------------------------
 *                              |                         |
 *                              14                        10
 *                        --------------             -------------
 *                       |              |           |             |
 *                       8              7           9             3
 *                   ---------       ----
 *                  |         |     |
 *                  2         4     1(last - 1)
 * pop_heap:
 *
 *                                          16(head)--------------------------
 *                               -------------------------                     |
 *                              |                         |                    |
 *                              4                         10                   |
 *                        --------------             -------------             |
 *                       |              |           |             |            | 
 *                       14             7           9             3            |
 *                   ---------       ----                                      |
 *                  |         |     |                                          |
 *                  2         8     1(last - 1) <------------------------------ 
 *
 *                                          (hole)(head)
 *                               -------------------------               
 *                              |                         |                  
 *                              4                         10                 
 *                        --------------             -------------           
 *                       |              |           |             |          (val = 1)
 *                       14             7           9             3         
 *                   ---------       ----                                    
 *                  |         |     |                                       
 *                  2         8     16(last - 1)
 *                           
 * adjust_heap:
 *                                          14(head)
 *                               -------------------------               
 *                              |                         |                  
 *                              8                        10                 
 *                        --------------             -------------           
 *                       |              |           |             |           (val = 1)         
 *                       4              7           9             3         
 *                   ---------                                         
 *                  |         |                                            
 *                  2      (hole)(last - 1)   16
 *
 *
 * push_heap:
 *                                          14(head)
 *                               -------------------------               
 *                              |                         |                  
 *                              8                        10                 
 *                        --------------             -------------           
 *                       |              |           |             |           (val = 1)         
 *                       4              7           9             3              |
 *                   ---------                                                   |
 *                  |         | /-----------------------------------------------
 *                  2      (hole)(last - 1)   16
 *
 *                                          14(head)
 *                               -------------------------               
 *                              |                         |                  
 *                              8                        10                 
 *                        --------------             -------------           
 *                       |              |           |             |           (val = 1)         
 *                       4              7           9             3            
 *                   ---------                                                   
 *                  |         |  
 *                  2       1(last - 1)   16
 *
 * pop_heap adjust_heap push_heap ...
 *
 * final_heap:
 *                                           1(head)
 *                            
 *                         
 *                              2                         3               
 *                               
 *                              
 *                       4              7           8             9           
 *                                                            
 *             
 *                  10       14      16
 *     
 * result: 1 2 3 4 7 8 9 10 14 16
 * </pre>
 */
tb_void_t tb_heap_sort(tb_iterator_t* iterator, tb_size_t head, tb_size_t tail, tb_iterator_comp_t comp)
{
	// check
	tb_assert_and_check_return(iterator && iterator->mode & TB_ITERATOR_MODE_RACCESS);
	tb_check_return(head != tail);

	// make
	tb_heap_make(iterator, head, tail, comp);

	// init
	tb_size_t 		step = tb_iterator_step(iterator);
	tb_pointer_t 	last = step > sizeof(tb_pointer_t)? tb_malloc(step) : tb_null;
	tb_assert_and_check_return(step <= sizeof(tb_pointer_t) || last);

	// pop0 ...
	for (; tail > head + 1; tail--)
	{
		// save last
		if (step <= sizeof(tb_pointer_t)) last = tb_iterator_item(iterator, tail - 1);
		else tb_memcpy(last, tb_iterator_item(iterator, tail - 1), step);

		// pop0
		tb_heap_pop0(iterator, head, tail, last, comp);
	}

	// free
	if (last && step > sizeof(tb_pointer_t)) tb_free(last);
}
tb_void_t tb_heap_sort_all(tb_iterator_t* iterator, tb_iterator_comp_t comp)
{
	tb_heap_sort(iterator, tb_iterator_head(iterator), tb_iterator_tail(iterator), comp);
}
