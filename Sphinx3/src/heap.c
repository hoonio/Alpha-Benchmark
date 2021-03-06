/*
 * 
 * This file is part of the ALPBench Benchmark Suite Version 1.0
 * 
 * Copyright (c) 2005 The Board of Trustees of the University of Illinois
 * 
 * All rights reserved.
 * 
 * ALPBench is a derivative of several codes, and restricted by licenses
 * for those codes, as indicated in the source files and the ALPBench
 * license at http://www.cs.uiuc.edu/alp/alpbench/alpbench-license.html
 * 
 * The multithreading and SSE2 modifications for SpeechRec, FaceRec,
 * MPEGenc, and MPEGdec were done by Man-Lap (Alex) Li and Ruchira
 * Sasanka as part of the ALP research project at the University of
 * Illinois at Urbana-Champaign (http://www.cs.uiuc.edu/alp/), directed
 * by Prof. Sarita V. Adve, Dr. Yen-Kuang Chen, and Dr. Eric Debes.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal with the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimers.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimers in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the names of Professor Sarita Adve's research group, the
 *       University of Illinois at Urbana-Champaign, nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this Software without specific prior written permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
 * SOFTWARE.
 * 
 */


/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * heap.c -- Generic heap structure for inserting in any and popping in sorted
 * 		order.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1999 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 05-Mar-99	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Fixed bug in heap_destroy() (in while loop exit condition).
 * 
 * 23-Dec-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Started.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "heap.h"
#include "ckd_alloc.h"


typedef struct heap_s {
    void *data;		/* Application data at this node */
    int32 val;		/* Associated with above application data; according to which
			   heap is sorted (in ascending order) */
    int32 nl, nr;	/* #left/right descendants of this node (for balancing heap) */
    struct heap_s *l;	/* Root of left descendant heap */
    struct heap_s *r;	/* Root of right descendant heap */
} heapnode_t;


#if 0
static void heap_dump (heapnode_t *top, int32 level)
{
    int32 i;
    
    if (! top)
	return;
    
    for (i = 0; i < level; i++)
	printf ("  ");
    /* print top info */
    heap_dump (top->l, level+1);
    heap_dump (top->r, level+1);
}
#endif


heap_t heap_new ( void )
{
    heapnode_t **h;
    
    h = (heapnode_t **) ckd_calloc (1, sizeof(heapnode_t *));
    *h = NULL;

    return ((heap_t) h);
}


static heapnode_t *subheap_insert (heapnode_t *root, void *data, int32 val)
{
    heapnode_t *h;
    void *tmpdata;
    int32 tmpval;
    
    if (! root) {
	h = (heapnode_t *) mymalloc (sizeof(heapnode_t));
	h->data = data;
	h->val = val;
	h->l = h->r = NULL;
	h->nl = h->nr = 0;
	return h;
    }

    /* Root already exists; if new value is less, replace root node */
    if (root->val > val) {
	tmpdata = root->data;
	tmpval = root->val;
	root->data = data;
	root->val = val;
	data = tmpdata;
	val = tmpval;
    }

    /* Insert new or old (replaced) node in right or left subtree; keep them balanced */
    if (root->nl > root->nr) {
	root->r = subheap_insert (root->r, data, val);
	root->nr++;
    } else {
	root->l = subheap_insert (root->l, data, val);
	root->nl++;
    }

    return root;
}


int32 heap_insert (heap_t heap, void *data, int32 val)
{
    heapnode_t **hp;

    hp = (heapnode_t **) heap;
    
    *hp = subheap_insert (*hp, data, val);

    return 0;
}


static heapnode_t *subheap_pop (heapnode_t *root)
{
    heapnode_t *l, *r;

    /* Propagate best value from below into root, if any */
    l = root->l;
    r = root->r;

    if (! l) {
	if (! r) {
	    myfree ((char *) root, sizeof(heapnode_t));
	    return NULL;
	} else {
	    root->data = r->data;
	    root->val = r->val;
	    root->r = subheap_pop (r);
	    root->nr--;
	}
    } else {
	if ((! r) || (l->val < r->val)) {
	    root->data = l->data;
	    root->val = l->val;
	    root->l = subheap_pop (l);
	    root->nl--;
	} else {
	    root->data = r->data;
	    root->val = r->val;
	    root->r = subheap_pop (r);
	    root->nr--;
	}
    }

    return root;
}


int32 heap_pop (heap_t heap, void **data, int32 *val)
{
    heapnode_t **hp, *h;

    hp = (heapnode_t **) heap;
    h = *hp;

    if (! h)
	return 0;
    
    *data = h->data;
    *val = h->val;
    
    *hp = subheap_pop (h);
    
    return 1;
}


int32 heap_top (heap_t heap, void **data, int32 *val)
{
    heapnode_t **hp, *h;

    hp = (heapnode_t **) heap;
    h = *hp;

    if (! h)
	return 0;
    
    *data = h->data;
    *val = h->val;
    
    return 1;
}


int32 heap_destroy (heap_t heap)
{
    void *data;
    int32 val;
    
    /* Empty the heap and free it */
    while (heap_pop (heap, &data, &val) > 0);
    ckd_free ((char *)heap);

    return 0;
}
