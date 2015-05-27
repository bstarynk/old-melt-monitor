// file functions.c

/**   Copyright (C)  2015 Free Software Foundation, Inc.
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "monimelt.h"


////////////////////////////////////////////////////////////////
//// magic_attribute
bool
  momfunc_1itm_to_val_emitter_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  assert (clonode);
  assert (itm);
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (magic_attribute));
  momvalue_t vclos =
    mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_magic_attribute),
		   2,
		   mom_nodev ((momnode_t *) itm->itm_data1),
		   mom_nodev ((momnode_t *) itm->itm_data2));
  MOM_DEBUGPRINTF (dump, "emitter_of_magic_attribute vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_magic_attribute */




bool
  momfunc_1itm_to_void_scanner_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm)
{
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (magic_attribute));
  momvalue_t valgetclos = mom_nodev ((momnode_t *) itm->itm_data1);
  momvalue_t valputclos = mom_nodev ((momnode_t *) itm->itm_data2);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute itm=%s valgetclos=%s valputclos=%s",
		   mom_item_cstring (itm), mom_output_gcstring (valgetclos),
		   mom_output_gcstring (valputclos));
  mom_scan_dumped_value (valgetclos);
  mom_scan_dumped_value (valputclos);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_magic_attribute end itm=%s",
		   mom_item_cstring (itm));
  return true;
}				/* end scanner_of_magic_attribute */


bool
  momfunc_1itm_to_void_filler_of_magic_attribute
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_magic_attribute itm=%s",
		   mom_item_cstring (itm));
  if (!clonode || clonode->slen < 2)
    MOM_FATAPRINTF ("filler_of_magic_attribute %s has bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));
  momvalue_t vgetclos = clonode->arrsons[0];
  momvalue_t vputclos = clonode->arrsons[1];
  if (vgetclos.typnum != momty_node)
    MOM_FATAPRINTF ("filler_of_magic_attribute %s has bad getter %s",
		    mom_item_cstring (itm), mom_output_gcstring (vgetclos));
  if (vputclos.typnum != momty_node)
    MOM_FATAPRINTF ("filler_of_magic_attribute %s has bad getter %s",
		    mom_item_cstring (itm), mom_output_gcstring (vputclos));
  itm->itm_kind = MOM_PREDEFINED_NAMED (magic_attribute);
  itm->itm_data1 = (void *) vgetclos.vnode;
  itm->itm_data2 = (void *) vputclos.vnode;
  MOM_DEBUGPRINTF (dump,
		   "filler_of_magic_attribute itm=%s done vgetclos=%s vputclos=%s",
		   mom_item_cstring (itm),
		   mom_output_gcstring (vgetclos),
		   mom_output_gcstring (vputclos));
  return true;
}				/* end of filler_of_magic_attribute */


//////////////// functions
bool
  momfunc_1itm_to_val_emitter_of_function
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  momitem_t *itmkind = itm->itm_kind;
  assert (itmkind);
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_function itm=%s kind %s (of kind %s)",
		   mom_item_cstring (itm), mom_item_cstring (itmkind),
		   mom_item_cstring (itmkind->itm_kind));
  assert (itmkind->itm_kind == MOM_PREDEFINED_NAMED (function_signature));
  momvalue_t vclos = mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_function),
				    1,
				    mom_itemv (itmkind));
  MOM_DEBUGPRINTF (dump, "emitter_of_function vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_function */



bool
  momfunc_1itm_to_void_filler_of_function
  (const momnode_t *clonode, momitem_t *itm)
{
  char bufnam[256];
  memset (bufnam, 0, sizeof (bufnam));
  MOM_DEBUGPRINTF (load, "filler_of_function itm=%s", mom_item_cstring (itm));
  if (!clonode || clonode->slen < 1)
    MOM_FATAPRINTF ("filler_of_function %s has bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));
  momitem_t *itmsig = mom_value_to_item (clonode->arrsons[0]);
  if (!itmsig
      || itmsig->itm_kind != MOM_PREDEFINED_NAMED (function_signature))
    MOM_FATAPRINTF ("filler_of_function %s has bad closed signature %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (clonode->arrsons[0]));
  MOM_DEBUGPRINTF (load, "filler_of_function itm=%s itmsig=%s",
		   mom_item_cstring (itm), mom_item_cstring (itmsig));
  momvalue_t cfunradv = MOM_NONEV;
  {
    mom_item_lock (itmsig);
    cfunradv =
      mom_item_unsync_get_attribute (itmsig,
				     MOM_PREDEFINED_NAMED (function_radix));
    mom_item_unlock (itmsig);
  }
  MOM_DEBUGPRINTF (load, "filler_of_function itm=%s itmsig=%s cfunradv %s",
		   mom_item_cstring (itm), mom_item_cstring (itmsig),
		   mom_output_gcstring (cfunradv));
  if (cfunradv.typnum != momty_string)
    MOM_FATAPRINTF
      ("filler_of_function %s with kind %s and bad `function_radix` %s",
       mom_item_cstring (itm), mom_item_cstring (itmsig),
       mom_output_gcstring (cfunradv));
  if (snprintf
      (bufnam, sizeof (bufnam), MOM_FUNCTION_PREFIX "%s_%s",
       mom_value_cstr (cfunradv),
       mom_item_cstring (itm)) >= (int) sizeof (bufnam))
    MOM_FATAPRINTF ("filler_of_function %s with kind %s too long name %s",
		    mom_item_cstring (itm), mom_item_cstring (itmsig),
		    bufnam);
  void *adfun = mom_dynload_symbol (bufnam);
  if (!adfun)
    MOM_FATAPRINTF
      ("filler_of_function %s with kind %s failed to find C function %s",
       mom_item_cstring (itm), mom_item_cstring (itmsig), bufnam);
  {
    mom_item_lock (itm);
    itm->itm_kind = itmsig;
    itm->itm_data1 = adfun;
    mom_item_unlock (itm);
  }
  MOM_DEBUGPRINTF (load, "filler_of_function %s done kind %s function %s @%p",
		   mom_item_cstring (itm), mom_item_cstring (itmsig), bufnam,
		   adfun);
  return true;
}				/* end of filler_of_function */

bool
  momfunc_1itm_to_void_scanner_of_function
  (const momnode_t *clonode, momitem_t *itm)
{
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_function itm=%s", mom_item_cstring (itm));
  momitem_t *kinditm = itm->itm_kind;
  if (kinditm
      && kinditm->itm_kind == MOM_PREDEFINED_NAMED (function_signature))
    {
      Dl_info dlinfo;
      memset (&dlinfo, 0, sizeof (dlinfo));
      if (dladdr (itm->itm_data1, &dlinfo))
	{
	  MOM_DEBUGPRINTF (dump,
			   "scanner_of_function itm=%s"
			   " dli_fname %s, dli_sname %s, dli_saddr %p, itmdata %p"
			   "; delta %#lx",
			   mom_item_cstring (itm),
			   dlinfo.dli_fname,
			   dlinfo.dli_sname,
			   dlinfo.dli_saddr,
			   itm->itm_data1,
			   (long) ((char *) dlinfo.dli_saddr -
				   (char *) itm->itm_data1));
	  if (!strncmp
	      (dlinfo.dli_sname, MOM_FUNCTION_PREFIX,
	       strlen (MOM_FUNCTION_PREFIX))
	      && !strncmp (dlinfo.dli_fname,
			   MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX,
			   strlen (MOM_MODULE_DIRECTORY
				   MOM_SHARED_MODULE_PREFIX)))
	    {
	      MOM_DEBUGPRINTF (dump,
			       "scanner_of_function itm=%s  good dli_fname %s",
			       mom_item_cstring (itm), dlinfo.dli_fname);
	      char *restnam = (char *) MOM_GC_STRDUP ("restnam",
						      dlinfo.dli_fname +
						      strlen
						      (MOM_MODULE_DIRECTORY
						       MOM_SHARED_MODULE_PREFIX));
	      MOM_DEBUGPRINTF (dump, "scanner_of_function itm=%s restnam %s",
			       mom_item_cstring (itm), restnam);
	      char *dotrest = strchr (restnam, '.');
	      if (dotrest)
		*dotrest = '\0';
	      momitem_t *moditm = mom_find_item (restnam);
	      MOM_DEBUGPRINTF (dump,
			       "scanner_of_function itm=%s truncated restnam %s moditm %s",
			       mom_item_cstring (itm), restnam,
			       mom_item_cstring (moditm));
	      if (moditm)
		mom_scan_dumped_module_item (moditm);
	      MOM_DEBUGPRINTF (dump,
			       "scanner_of_function itm=%s done moduleitem %s",
			       mom_item_cstring (itm),
			       mom_item_cstring (moditm));
	    }
	  else
	    MOM_DEBUGPRINTF (dump,
			     "scanner_of_function itm=%s strange dli_fname <%s> not starting with "
			     MOM_MODULE_DIRECTORY MOM_SHARED_MODULE_PREFIX,
			     mom_item_cstring (itm), dlinfo.dli_fname);


	}
      else
	MOM_WARNPRINTF ("scanner_of_function itm=%s ad@%p failed : %s",
			mom_item_cstring (itm), itm->itm_data1, dlerror ());
    }
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_function end itm=%s", mom_item_cstring (itm));
  return true;
}				/* end scanner_of_function */

////////////////////////////////////////////////////////////////
//// plain kind
bool
  momfunc_1itm_to_val_emitter_of_plain_kind
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  momitem_t *itmclokind = NULL;
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_plain_kind itm=%s", mom_item_cstring (itm));
  if (!clonode || clonode->slen == 0)
    itmclokind = itm->itm_kind;
  else if (!(itmclokind = mom_value_to_item (clonode->arrsons[0])))
    MOM_FATAPRINTF ("emitter_of_plain_kind itm=%s bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));

  momvalue_t vclos =
    mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_plain_kind),
		   1, mom_itemv (itmclokind));
  MOM_DEBUGPRINTF (dump, "emitter_of_plain_kind vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_plain_kind */

bool
  momfunc_1itm_to_void_filler_of_plain_kind
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_plain_kind itm=%s", mom_item_cstring (itm));
  if (!clonode || clonode->slen < 1)
    MOM_FATAPRINTF ("filler_of_plain_kind %s has bad closure %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (mom_nodev (clonode)));
  momitem_t *itmkind = mom_value_to_item (clonode->arrsons[0]);
  if (!itmkind)
    MOM_FATAPRINTF ("filler_of_plain_kind %s with bad kind %s",
		    mom_item_cstring (itm),
		    mom_output_gcstring (clonode->arrsons[0]));
  assert (!itm->itm_kind || itm->itm_kind == itmkind);
  {
    mom_item_lock (itm);
    itm->itm_kind = itmkind;
    itm->itm_data1 = NULL;
    itm->itm_data2 = NULL;
    mom_item_unlock (itm);
  }
  MOM_DEBUGPRINTF (load, "filler_of_plain_kind %s done kind %s",
		   mom_item_cstring (itm), mom_item_cstring (itmkind));
  return true;

}


////////////////////////////////////////////////////////////////
////////// association-s


bool
  momfunc_1itm_to_void_scanner_of_association
  (const momnode_t *clonode, momitem_t *itm)
{
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_association itm=%s", mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (association));
  struct momattributes_st *assoc = itm->itm_data1;
  if (assoc)
    mom_attributes_scan_dump (assoc);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_association end itm=%s",
		   mom_item_cstring (itm));
  return true;
}				/* end scanner_of_association */

bool
  momfunc_1itm_to_val_emitter_of_association
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_association itm=%s", mom_item_cstring (itm));
  assert (clonode);
  assert (itm);
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (association));
  struct momattributes_st *assoc = itm->itm_data1;
  unsigned nbattr = mom_attributes_count (assoc);
  const momseq_t *seqattr = mom_attributes_set (assoc, MOM_NONEV);
  assert (nbattr == 0 || (seqattr && seqattr->slen == nbattr));
  momvalue_t *assarr =
    MOM_GC_ALLOC ("associations", (nbattr + 1) * sizeof (momvalue_t));
  for (unsigned ix = 0; ix < nbattr; ix++)
    {
      const momitem_t *itm = seqattr->arritm[ix];
      assert (itm && itm != MOM_EMPTY);
      struct momentry_st *ent = mom_attributes_find_entry (assoc, itm);
      assert (ent != NULL);
      assarr[ix] =		//
	mom_nodev_new (MOM_PREDEFINED_NAMED (association),
		       2, mom_itemv (itm), ent->ent_val);
    };
  momvalue_t vclos =
    mom_nodev_sized (MOM_PREDEFINED_NAMED (filler_of_association),
		     nbattr,
		     assarr);
  MOM_DEBUGPRINTF (dump, "emitter_of_association vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_association */




bool
  momfunc_1itm_to_void_filler_of_association
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_association itm=%s", mom_item_cstring (itm));
  if (!clonode)
    MOM_FATAPRINTF ("filler_of_association %s without closure",
		    mom_item_cstring (itm));

  if (clonode->conn != MOM_PREDEFINED_NAMED (filler_of_association))
    MOM_FATAPRINTF ("filler_of_association %s has bad closure",
		    mom_item_cstring (itm));
  unsigned nbent = clonode->slen;
  unsigned siz = ((5 * nbent / 4 + 3) | 7) + 1;
  struct momattributes_st *assoc = mom_attributes_make (siz);
  for (unsigned ix = 0; ix < nbent; ix++)
    {
      momvalue_t vcomp = clonode->arrsons[ix];
      if (vcomp.typnum != momty_node
	  || vcomp.vnode->conn != MOM_PREDEFINED_NAMED (association)
	  || vcomp.vnode->slen != 2)
	MOM_FATAPRINTF ("filler_of_association %s has bad comp#%d %s",
			mom_item_cstring (itm), ix,
			mom_output_gcstring (vcomp));
      assoc =
	mom_attributes_put (assoc,
			    mom_value_to_item (vcomp.vnode->arrsons[0]),
			    &vcomp.vnode->arrsons[1]);
    }
  itm->itm_kind = MOM_PREDEFINED_NAMED (association);
  itm->itm_data1 = assoc;
  MOM_DEBUGPRINTF (dump,
		   "filler_of_association itm=%s done",
		   mom_item_cstring (itm));
  return true;
}				/* end of filler_of_association */




////////////////////////////////////////////////////////////////
////////// hashed_set-s


bool
  momfunc_1itm_to_void_scanner_of_hashed_set
  (const momnode_t *clonode, momitem_t *itm)
{
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_hashed_set itm=%s", mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (hashed_set));
  struct momhashset_st *hset = itm->itm_data1;
  if (hset)
    mom_hashset_scan_dump (hset);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_hashed_set end itm=%s",
		   mom_item_cstring (itm));
  return true;
}				/* end scanner_of_hashed_set */

bool
  momfunc_1itm_to_val_emitter_of_hashed_set
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_hashed_set itm=%s", mom_item_cstring (itm));
  assert (clonode);
  assert (itm);
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (hashed_set));
  struct momhashset_st *hset = itm->itm_data1;
  const momseq_t *seqset = mom_hashset_elements_set (hset);
  momvalue_t vclos =
    mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_hashed_set),
		   1,
		   mom_unsafe_setv (seqset));
  MOM_DEBUGPRINTF (dump, "emitter_of_hashed_set vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_hashed_set */




bool
  momfunc_1itm_to_void_filler_of_hashed_set
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_hashed_set itm=%s", mom_item_cstring (itm));
  if (!clonode)
    MOM_FATAPRINTF ("filler_of_hashed_set %s without closure",
		    mom_item_cstring (itm));

  if (clonode->conn != MOM_PREDEFINED_NAMED (filler_of_hashed_set)
      || clonode->slen != 1 || (clonode->arrsons[0].typnum != momty_null
				&& clonode->arrsons[0].typnum != momty_set))
    MOM_FATAPRINTF ("filler_of_hashed_set %s has bad closure",
		    mom_item_cstring (itm));
  itm->itm_kind = MOM_PREDEFINED_NAMED (hashed_set);
  if (clonode->arrsons[0].typnum == momty_set)
    {
      struct momhashset_st *hset =	//
	mom_hashset_add_sized_items (NULL,	//
				     clonode->arrsons[0].vset->slen,
				     clonode->arrsons[0].vset->arritm);
      itm->itm_data1 = hset;
    }
  else
    itm->itm_data1 = NULL;
  return true;
}				/* end of filler_of_hashed_set */




////////////////////////////////////////////////////////////////
////////// item_queue-s


bool
  momfunc_1itm_to_void_scanner_of_item_queue
  (const momnode_t *clonode, momitem_t *itm)
{
  assert (clonode);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_item_queue itm=%s", mom_item_cstring (itm));
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (item_queue));
  struct momqueueitems_st *iqu = itm->itm_data1;
  if (iqu)
    mom_queueitem_scan_dump (iqu);
  MOM_DEBUGPRINTF (dump,
		   "scanner_of_item_queue end itm=%s",
		   mom_item_cstring (itm));
  return true;
}				/* end scanner_of_item_queue */

bool
  momfunc_1itm_to_val_emitter_of_item_queue
  (const momnode_t *clonode, momitem_t *itm, momvalue_t *res)
{
  MOM_DEBUGPRINTF (dump,
		   "emitter_of_item_queue itm=%s", mom_item_cstring (itm));
  assert (clonode);
  assert (itm);
  assert (itm->itm_kind == MOM_PREDEFINED_NAMED (item_queue));
  struct momqueueitems_st *iqu = itm->itm_data1;
  const momseq_t *tupque = iqu ? mom_queueitem_tuple (iqu, MOM_NONEV) : NULL;
  momvalue_t vclos =
    mom_nodev_new (MOM_PREDEFINED_NAMED (filler_of_item_queue),
		   1,
		   mom_tuplev (tupque));
  MOM_DEBUGPRINTF (dump, "emitter_of_item_queue vclos=%s",
		   mom_output_gcstring (vclos));
  *res = vclos;
  return true;
}				/* end emitter_of_item_queue */




bool
  momfunc_1itm_to_void_filler_of_item_queue
  (const momnode_t *clonode, momitem_t *itm)
{
  MOM_DEBUGPRINTF (dump,
		   "filler_of_item_queue itm=%s", mom_item_cstring (itm));
  if (!clonode)
    MOM_FATAPRINTF ("filler_of_item_queue %s without closure",
		    mom_item_cstring (itm));

  if (clonode->conn != MOM_PREDEFINED_NAMED (filler_of_item_queue)
      || clonode->slen != 1 || (clonode->arrsons[0].typnum != momty_null
				&& clonode->arrsons[0].typnum != momty_tuple))
    MOM_FATAPRINTF ("filler_of_item_queue %s has bad closure",
		    mom_item_cstring (itm));
  itm->itm_kind = MOM_PREDEFINED_NAMED (item_queue);
  if (clonode->arrsons[0].typnum == momty_tuple)
    {
      struct momqueueitems_st *iqu =
	MOM_GC_ALLOC ("new queue items", sizeof (struct momqueueitems_st));
      const momseq_t *tup = mom_value_to_tuple (clonode->arrsons[0]);
      assert (tup != NULL);
      unsigned nbitems = mom_seq_length (tup);
      for (unsigned ix = 0; ix < nbitems; ix++)
	{
	  const momitem_t *curitm = mom_seq_nth (tup, ix);
	  if (curitm)
	    mom_queueitem_push_back (iqu, curitm);
	}
      itm->itm_data1 = iqu;
    }
  else
    itm->itm_data1 = NULL;
  return true;
}				/* end of filler_of_item_queue */


bool
  momfunc_1val_to_val_transform_block_statement
  (const momnode_t *clonode, momvalue_t valblockstmt, momvalue_t *pvalres)
{
  MOM_DEBUGPRINTF (gencod,
		   "transform_block_statement start valblockstmt=%s",
		   mom_output_gcstring (valblockstmt));
  assert (clonode != NULL);
  momitem_t *funcitm = mom_node_conn (clonode);
  momvalue_t vclos = mom_raw_item_get_indexed_component (funcitm, 0);
  MOM_DEBUGPRINTF (gencod,
		   "transform_block_statement vclos=%s",
		   mom_output_gcstring (vclos));
  if (vclos.typnum != momty_node)
    MOM_FATAPRINTF
      ("corrupted transform_block_statement funcitm %s starts with bad vclos %s for valblockstmt %s",
       mom_item_cstring (funcitm), mom_output_gcstring (vclos),
       mom_output_gcstring (valblockstmt));
  if (valblockstmt.typnum != momty_item)
    {
      MOM_WARNPRINTF ("transform_block_statement bad valblockstmt %s",
		      mom_output_gcstring (valblockstmt));
      return false;
    }
  momitem_t *resitm = NULL;
  if (!mom_applval_1itm_to_item (vclos, valblockstmt.vitem, &resitm))
    return false;
  MOM_DEBUGPRINTF (gencod,
		   "transform_block_statement valblockstmt=%s resitm=%s",
		   mom_output_gcstring (valblockstmt),
		   mom_item_cstring (resitm));
  if (!resitm)
    return false;
  if (pvalres)
    *pvalres = mom_itemv (resitm);
  return true;
}				/* end of transform_block_statement */


// see http://en.wikipedia.org/wiki/Basic_block
bool
  momfunc_1itm_to_item_transform_block_statement_item
  (const momnode_t *clonode, momitem_t *blockstmtitm, momitem_t **presitm)
{
  enum transfblockstmtitemclo_en
  {
    tbsi_decorator,
    tbsi__end
  };
  MOM_DEBUGPRINTF (gencod,
		   "transform_block_statement_item start blockstmtitm=%s",
		   mom_item_cstring (blockstmtitm));
  if (!blockstmtitm)
    return false;
  if (blockstmtitm->itm_kind == MOM_PREDEFINED_NAMED (block))
    {
      *presitm = blockstmtitm;
      return true;
    };
  if (blockstmtitm->itm_kind != MOM_PREDEFINED_NAMED (block_statement))
    return false;
  momitem_t *funcitm = mom_node_conn (clonode);
  momvalue_t vdecoclos =
    mom_raw_item_get_indexed_component (funcitm, tbsi_decorator);
  MOM_DEBUGPRINTF (gencod, "transform_block_statement_item vdecoclos=%s",
		   mom_output_gcstring (vdecoclos));
  if (!mom_applval_1itm_to_void (vdecoclos, blockstmtitm))
    return false;
  MOM_FATAPRINTF
    ("unimplemented transform_block_statement_item blockstmtitm=%s",
     mom_item_cstring (blockstmtitm));
}				/* end fo transform_block_statement_item */



// see http://en.wikipedia.org/wiki/Basic_block
bool
  momfunc_1itm_to_void_decorate_block_statement_item
  (const momnode_t *clonode, momitem_t *blockstmtitm)
{
  MOM_DEBUGPRINTF (gencod,
		   "decorate_block_statement_item start blockstmtitm=%s",
		   mom_item_cstring (blockstmtitm));
  if (!blockstmtitm
      || blockstmtitm->itm_kind != MOM_PREDEFINED_NAMED (block_statement))
    return false;
  mom_item_lock (blockstmtitm);
  {
    momitem_t *opitm =
      mom_value_to_item (mom_raw_item_get_indexed_component
			 (blockstmtitm, 0));
    MOM_DEBUGPRINTF (gencod,
		     "decorate_block_statement_item blockstmtitm=%s opitm=%s",
		     mom_item_cstring (blockstmtitm),
		     mom_item_cstring (opitm));
    if (!opitm)
      goto endlock;
    switch (mom_item_hash (opitm))
      {
      case MOM_PREDEFINED_NAMED_CASE (int_switch, opitm, otherwiseoplab):
	{
#warning unimplemented decorate_block_statement_item
	}
	break;
      otherwiseoplab:
      default:
	break;
      };			/* end switch hash opitm */
    goto endlock;
  }
endlock:
  mom_item_unlock (blockstmtitm);
  MOM_DEBUGPRINTF (gencod,
		   "decorate_block_statement_item end blockstmtitm=%s",
		   mom_item_cstring (blockstmtitm));

  /// put a leader_block attribute into each leader statement
#warning decorate_block_statement_item unimplemented
  MOM_FATAPRINTF
    ("unimplemented decorate_block_statement_item blockstmtitm=%s",
     mom_item_cstring (blockstmtitm));
}
