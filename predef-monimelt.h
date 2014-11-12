
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
MOM_PREDEFINED_NAMED(GET,_9dsak0qcy0v_1c5z9th7x3i,1573018885)

////!indicates the HTTP HEAD method
MOM_PREDEFINED_NAMED(HEAD,_47fatww79x6_vh8ap22c0ch,3922245622)

////!indicates the HTTP POST method
MOM_PREDEFINED_NAMED(POST,_5wmusj136kq_u5qpehp89aq,919851673)

////!The agenda is central to Monimelt.
////+It is the queue of taskets to be executed by worker threads.
MOM_PREDEFINED_NAMED(agenda,_02u53qxa7dm_sttmhffpchr,15591209)

////!arguments of procedure or primitive
MOM_PREDEFINED_NAMED(arguments,_2x2zpyhfqum_0utui69rzea,3803054010)

////!for assignment <var> := <expr>
MOM_PREDEFINED_NAMED(assign,_8vzddhpmhp8_z0005cqyqzf,861132230)

////!gives the attribute[s], at least in dumped JSON...
MOM_PREDEFINED_NAMED(attr,_6w3dvx83dfw_xzc6aif6isv,4127160085)

////!for block items
MOM_PREDEFINED_NAMED(block,_94tq2iauet8_jujpjhjrzsm,131104555)

////!for blocks in routines, etc...
MOM_PREDEFINED_NAMED(blocks,_1r880c1yk3z_i5e8mprcj90,1909866882)

////!for calls at end of  blocks
MOM_PREDEFINED_NAMED(call,_02av6173qvf_pehzhe755j2,838828699)

////!inside switches: case <constant-expr> <block>
MOM_PREDEFINED_NAMED(case,_38w4qwrmd6z_74x5z80v5k6,284764426)

////!used as node of primitive expansions, etc...
MOM_PREDEFINED_NAMED(chunk,_8x6fxcm4z2k_vdaqicfi4z0,886999649)

////!JSON for closure values
MOM_PREDEFINED_NAMED(closed_values,_9mxi9e605ay_ihpjyrwq250,776151069)

////!JSON for closure payload
MOM_PREDEFINED_NAMED(closure,_97zkxf62r11_6eedwwv3eu8,2411599826)

////!JSON for closure routine name
MOM_PREDEFINED_NAMED(closure_routine,_28941cvehx8_9rf4udyeq8v,4235061253)

////!notably for error code in JSONRPC
MOM_PREDEFINED_NAMED(code,_0yyp8vmw4si_wf49m4d4zwq,163984103)

////!gives a human-readable comment
MOM_PREDEFINED_NAMED(comment,_41u1utcxyek_22cftxt3xxm,2024771965)

////!for constant in routines, etc...
MOM_PREDEFINED_NAMED(constant,_4zywi5fh3ef_ys9sq93vsc1,3780502208)

////!gives the item content, at least in dumped JSON...
MOM_PREDEFINED_NAMED(content,_8s357rq2dzk_k8ze95tikjm,1630350867)

////!something for counting
MOM_PREDEFINED_NAMED(count,_6f4k9pqzryk_w25f8vxuyyc,841187505)

////!Attribute giving the C type
MOM_PREDEFINED_NAMED(ctype,_0ee6afx5850_ji17eq0wmfa,119327747)

////!for side-effecting instructions in blocks, etc...
MOM_PREDEFINED_NAMED(do,_5c789try94y_ssy6a22fpep,3287581817)

////!the double C type
MOM_PREDEFINED_NAMED(double,_7j7x11c25h3_wkchtuwpusx,3897333387)

////!Gives the double floating-point numbers of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(doubles,_17spwr8dkzv_tsf2s8diazu,1135570567)

////!error case, on for JSONRPC
MOM_PREDEFINED_NAMED(error,_4qcw2mwjswm_j9q0k9d04hm,797525452)

////!for exited processes
MOM_PREDEFINED_NAMED(exited,_3v4d7uzex6f_euek4pztiuh,1710495600)

////!for exited processes with exit code >0
MOM_PREDEFINED_NAMED(failed,_9sd1mh9q1zf_3duewi6fsaq,2529541293)

////!formal arguments...
MOM_PREDEFINED_NAMED(formals,_2ummst105ck_xracfy8v87y,2305693407)

////!frames in tasklet
MOM_PREDEFINED_NAMED(frames,_4cw8jv45vsk_4mh9ex64904,407235052)

////!for JSONRPC and elsewhere
MOM_PREDEFINED_NAMED(id,_7a9sxskjhcp_kpf30ka97ex,2687612409)

////!for conditionals, etc..
MOM_PREDEFINED_NAMED(if,_8ejwdt1a5yx_2meizztvte0,2841552917)

////!The C type for word integers
MOM_PREDEFINED_NAMED(intptr_t,_51u3st4u9mc_zdvms6jti0a,2777173293)

////!gives the item reference, at least in dumped JSON...
MOM_PREDEFINED_NAMED(item_ref,_6hf2vzmrsee_t35suhjvtj4,3823165917)

////!for JIT code of JIT-ed routines
MOM_PREDEFINED_NAMED(jit,_24yt56xf3d5_4w80i326kjz,3601262178)

////!in JSON dump, jtype of JSON array values
MOM_PREDEFINED_NAMED(json_array,_35vp60aw7em_d436vfie4ud,1502443136)

////!The false of JSON.
////+We cannot use false because it is a #define-ed macro.
MOM_PREDEFINED_NAMED(json_false,_4mha85xcfwi_9zqcvkiy3dk,2247010143)

////!in JSON dump, jtype of JSON object values
MOM_PREDEFINED_NAMED(json_object,_3xpyd539p4m_23h7wi59xi9,4271722182)

////!The true of JSON.
////+We cannot use true because it is a #define-ed macro.
MOM_PREDEFINED_NAMED(json_true,_2vmrrvq5kdk_9um63pstcu9,1958647740)

////!for JSONRPC
MOM_PREDEFINED_NAMED(jsonrpc,_0h331ch957p_j6a8i7v4e6y,453860511)

////!handler for JSONRPC requests
MOM_PREDEFINED_NAMED(jsonrpc_handler,_0hpzi8m7wym_1y4ypmm9y47,984209833)

////!in JSON dumps, give the type of a value
MOM_PREDEFINED_NAMED(jtype,_7urjeiw3evy_m7k72uv6790,450999481)

////!for jumps at end of  blocks
MOM_PREDEFINED_NAMED(jump,_2kxisdsque9_u9awek5wup1,759819674)

////!in JSON dumps, give the kind of the payload of an item
MOM_PREDEFINED_NAMED(kind,_06yp8ueq6yf_5ts408yww29,3458907209)

////!locals of function
MOM_PREDEFINED_NAMED(locals,_4p33dhxywm0_id6tti2kyw6,45612953)

////!for locked blocks
MOM_PREDEFINED_NAMED(lock,_232t5qs2v8e_zu2wy53cqe3,668574763)

////!notably for error message in JSONRPC
MOM_PREDEFINED_NAMED(message,_4jp2meuzru2_a58afyxwxa2,1089261779)

////!for JSONRPC and elsewhere
MOM_PREDEFINED_NAMED(method,_3hv5ymapjed_y8q6hsvhw8u,56524514)

////!module to be compiled...
MOM_PREDEFINED_NAMED(module,_7sqk8vh89xr_6tj8dq7vqju,741175951)

////!the routines in a module
MOM_PREDEFINED_NAMED(module_routines,_9dcxaqk8tqe_fam9mcxme9w,3332802288)

////!The C type for constant literal C strings
MOM_PREDEFINED_NAMED(momcstr_t,_80e7dsukuq3_6p7jffmz1yi,2233584871)

////!the value C type
MOM_PREDEFINED_NAMED(momval_t,_3uwzqwvj6zj_s63am4qivpt,858334073)

////!in JSON dumps, indicate nodes, or give their connective item
MOM_PREDEFINED_NAMED(node,_4m7x6811f6j_t480zu575mz,1812992144)

////!Group together all noticed values in dump outcome.
MOM_PREDEFINED_NAMED(notice,_7diyc1cwj8z_x630afccr8e,3456383299)

////!Gives the integer numbers of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(numbers,_3fw5acswe59_9016fqe4d41,1824167294)

////!for JSONRPC and elsewhere
MOM_PREDEFINED_NAMED(params,_4215uc2u6qk_52kqyra86y5,709750062)

////!in JSON dumps, give the payload of items
MOM_PREDEFINED_NAMED(payload,_41v0erax6my_m6pytj0793u,3962456714)

////!the expansion of a primitive
MOM_PREDEFINED_NAMED(primitive_expansion,_37x98fyestf_ttup2cu68r6,1061786883)

////!for procedure related things
MOM_PREDEFINED_NAMED(procedure,_4v74chqs1eh_chqd9cqw85t,2329607430)

////!first result in tasklet
MOM_PREDEFINED_NAMED(res1,_3j3s2e0510a_096chqpijq7,3304263536)

////!second result in tasklet
MOM_PREDEFINED_NAMED(res2,_8u5ar84utwm_99k5mq2d589,201664136)

////!third result in tasklet
MOM_PREDEFINED_NAMED(res3,_60ist2ad22c_cfpjp5ay6uj,1346622887)

////!notably for error message in JSONRPC
MOM_PREDEFINED_NAMED(result,_6djzuwz5pav_cri386ywjhj,2755350724)

////!for ending some blocks
MOM_PREDEFINED_NAMED(return,_6443sk5q0zt_8xdi02c6tzu,305481998)

////!in JSON dumps, indicate sets of item, or give their array of elements
MOM_PREDEFINED_NAMED(set,_2v75mmyph64_4h4kys78740,2780130992)

////!in JSON dumps, give the sons of nodes
MOM_PREDEFINED_NAMED(sons,_4ezpkss1akd_94f4h25sqe4,3753917517)

////!in JSON dumps, give the space of items
MOM_PREDEFINED_NAMED(space,_7rf7axuc9h4_2aw6utwmsas,3553961129)

////!for starting block in routines, etc...
MOM_PREDEFINED_NAMED(start,_3wh3e88sk28_d27qi2737zi,388835065)

////!Gives the state of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(state,_6f9870y6v8t_kp8fcmq2ezv,1856647655)

////!in JSON dumps, used for long chunked strings
MOM_PREDEFINED_NAMED(string,_8j516kuv89j_4hc4w6ykmr6,4254767725)

////!for switches <expr> <case>...
MOM_PREDEFINED_NAMED(switch,_638zh0145dj_rytv9kuj8v3,3894804359)

////!for routines for tasklet
MOM_PREDEFINED_NAMED(tasklet_function,_7yxp9xhih4z_9uzrqhkamxa,1188253756)

////!for terminated processes
MOM_PREDEFINED_NAMED(terminated,_3jpt8yuzuyw_ti1pyz3me1c,2596985071)

////!in JSON dumps, used for tuples of items
MOM_PREDEFINED_NAMED(tuple,_7vw56h18sw0_hv77m6q8uxu,3766188650)

////!in JSON dumps, used for values in attribute lists of items
MOM_PREDEFINED_NAMED(val,_7wk9y7e7r0z_575esi8ys5x,746430217)

////!Gives the values of Json for frames of tasklets.
MOM_PREDEFINED_NAMED(values,_91pketvc5pz_wq0v0wpauw8,1652042255)

////!gives verbatim code
MOM_PREDEFINED_NAMED(verbatim,_0x2k07ik4tm_ed7vqphf5ak,661264363)

////!the void C type
MOM_PREDEFINED_NAMED(void,_02q6zk9f5st_im0z75re15f,1828712906)

////!attribute giving the web handler inside items
MOM_PREDEFINED_NAMED(web_handler,_7sav6zery1v_24sa6jwwu6c,2339220870)

#undef MOM_PREDEFINED_NAMED
#undef MOM_PREDEFINED_ANONYMOUS
// eof predef-monimelt.h
