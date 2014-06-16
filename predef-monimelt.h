
////////////////////////////////////////////////////////////////++

/// generated file predef-monimelt.h ** DO NOT EDIT

/// Copyright (C) 2014 Free Software Foundation, Inc.
// MONIMELT is a monitor for MELT - see http://gcc-melt.org/
// This file is part of GCC.

// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.

// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.   If not see
//  <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////--


#ifndef MOM_PREDEFINED_NAMED
#error missing MOM_PREDEFINED_NAMED
#endif /*MOM_PREDEFINED_NAMED*/
#ifndef MOM_PREDEFINED_ANONYMOUS
#error missing MOM_PREDEFINED_ANONYMOUS
#endif /*MOM_PREDEFINED_ANONYMOUS*/


////!indicates the HTTP GET method
MOM_PREDEFINED_NAMED(GET,_9dsak0qcy0v_1c5z9th7x3i)

////!indicates the HTTP HEAD method
MOM_PREDEFINED_NAMED(HEAD,_47fatww79x6_vh8ap22c0ch)

////!indicates the HTTP POST method
MOM_PREDEFINED_NAMED(POST,_5wmusj136kq_u5qpehp89aq)

////!The agenda is central to Monimelt.
////+It is the queue of taskets to be executed by worker threads.
MOM_PREDEFINED_NAMED(agenda,_02u53qxa7dm_sttmhffpchr)

////!gives the attribute[s], at least in dumped JSON...
MOM_PREDEFINED_NAMED(attr,_6w3dvx83dfw_xzc6aif6isv)

////!the buffer payload kind, and also the clipboard buffer in editors
MOM_PREDEFINED_NAMED(buffer,_3zqd7ai3rtu_md9athkx17u)

////!Gives the closure of Json for frames of tasklets
MOM_PREDEFINED_NAMED(closure,_97zkxf62r11_6eedwwv3eu8)

////!gives a human-readable comment
MOM_PREDEFINED_NAMED(comment,_41u1utcxyek_22cftxt3xxm)

////!gives the item content, at least in dumped JSON...
MOM_PREDEFINED_NAMED(content,_8s357rq2dzk_k8ze95tikjm)

////!to be used inside display items
MOM_PREDEFINED_NAMED(display,_085krqf192t_z1m3zs77ww5)

////!for double values and displays
MOM_PREDEFINED_NAMED(double,_89ejvxupprm_f219pqwz13s)

////!Gives the double floating-point numbers of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(doubles,_17spwr8dkzv_tsf2s8diazu)

////!gives the editor inside a display
MOM_PREDEFINED_NAMED(editor,_38s7ihasu0m_xzipyerxm3j)

////!to be used inside display items for null
MOM_PREDEFINED_NAMED(empty,_8s4wcve2u49_252vwyzyrxd)

////!for exited processes
MOM_PREDEFINED_NAMED(exited,_3v4d7uzex6f_euek4pztiuh)

////!for exited processes with exit code >0
MOM_PREDEFINED_NAMED(failed,_9sd1mh9q1zf_3duewi6fsaq)

////!frames in tasklet
MOM_PREDEFINED_NAMED(frames,_4cw8jv45vsk_4mh9ex64904)

////!for integer values and displays
MOM_PREDEFINED_NAMED(integer,_9jeymqk2732_wiq5kyczi9c)

////!used in editor, etc. to reference some item
MOM_PREDEFINED_NAMED(item,_53748kde7s1_pkz810exr27)

////!gives the item reference, at least in dumped JSON...
MOM_PREDEFINED_NAMED(item_ref,_6hf2vzmrsee_t35suhjvtj4)

////!in JSON dump, jtype of JSON array values
MOM_PREDEFINED_NAMED(json_array,_35vp60aw7em_d436vfie4ud)

////!The false of JSON.
////+We cannot use false because it is a #define-ed macro.
MOM_PREDEFINED_NAMED(json_false,_4mha85xcfwi_9zqcvkiy3dk)

////!in JSON dump, jtype of JSON object values
MOM_PREDEFINED_NAMED(json_object,_3xpyd539p4m_23h7wi59xi9)

////!The true of JSON.
////+We cannot use true because it is a #define-ed macro.
MOM_PREDEFINED_NAMED(json_true,_2vmrrvq5kdk_9um63pstcu9)

////!in JSON dumps, give the type of a value
MOM_PREDEFINED_NAMED(jtype,_7urjeiw3evy_m7k72uv6790)

////!in JSON dumps, give the kind of the payload of an item
MOM_PREDEFINED_NAMED(kind,_06yp8ueq6yf_5ts408yww29)

////!in JSON dumps, indicate nodes, or give their connective item
MOM_PREDEFINED_NAMED(node,_4m7x6811f6j_t480zu575mz)

////!Group together all noticed values in dump outcome.
MOM_PREDEFINED_NAMED(notice,_7diyc1cwj8z_x630afccr8e)

////!Gives the integer numbers of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(numbers,_3fw5acswe59_9016fqe4d41)

////!to be used inside display items to give the origin
MOM_PREDEFINED_NAMED(origin,_5sw59dauckp_8eustjwf58u)

////!in JSON dumps, give the payload of items
MOM_PREDEFINED_NAMED(payload,_41v0erax6my_m6pytj0793u)

////!gives the rank
MOM_PREDEFINED_NAMED(rank,_7kkh6qiq1vc_e69zp2feuhe)

////!first result in tasklet
MOM_PREDEFINED_NAMED(res1,_3j3s2e0510a_096chqpijq7)

////!second result in tasklet
MOM_PREDEFINED_NAMED(res2,_8u5ar84utwm_99k5mq2d589)

////!third result in tasklet
MOM_PREDEFINED_NAMED(res3,_60ist2ad22c_cfpjp5ay6uj)

////!in JSON dumps, indicate sets of item, or give their array of elements
MOM_PREDEFINED_NAMED(set,_2v75mmyph64_4h4kys78740)

////!gives the size, e.g. in editors
MOM_PREDEFINED_NAMED(size,_5s59qeamxta_70k0mt77r9i)

////!in JSON dumps, give the sons of nodes
MOM_PREDEFINED_NAMED(sons,_4ezpkss1akd_94f4h25sqe4)

////!in JSON dumps, give the space of items
MOM_PREDEFINED_NAMED(space,_7rf7axuc9h4_2aw6utwmsas)

////!Gives the state of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(state,_6f9870y6v8t_kp8fcmq2ezv)

////!in JSON dumps, used for long chunked strings
MOM_PREDEFINED_NAMED(string,_8j516kuv89j_4hc4w6ykmr6)

////!for terminated processes
MOM_PREDEFINED_NAMED(terminated,_3jpt8yuzuyw_ti1pyz3me1c)

////!in JSON dumps, used for tuples of items
MOM_PREDEFINED_NAMED(tuple,_7vw56h18sw0_hv77m6q8uxu)

////!gives the update time when applicable
MOM_PREDEFINED_NAMED(updated,_5x41iah0kis_x8rrv3ww44t)

////!in JSON dumps, used for values in attribute lists of items
MOM_PREDEFINED_NAMED(val,_7wk9y7e7r0z_575esi8ys5x)

////!Gives the values of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(values,_91pketvc5pz_wq0v0wpauw8)

////!attribute giving the web handler inside items
MOM_PREDEFINED_NAMED(web_handler,_7sav6zery1v_24sa6jwwu6c)

#undef MOM_PREDEFINED_NAMED
#undef MOM_PREDEFINED_ANONYMOUS
// eof predef-monimelt.h
