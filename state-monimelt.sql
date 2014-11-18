-- state-monimelt dump 2014 Nov 18

 --   Copyright (C) 2014 Free Software Foundation, Inc.
 --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
 --  This sqlite3 dump file state-monimelt.sql is part of GCC.
 --
 --  GCC is free software; you can redistribute it and/or modify
 --  it under the terms of the GNU General Public License as published by
 --  the Free Software Foundation; either version 3, or (at your option)
 --  any later version.
 --
 --  GCC is distributed in the hope that it will be useful,
 --  but WITHOUT ANY WARRANTY; without even the implied warranty of
 --  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 --  GNU General Public License for more details.
 --  You should have received a copy of the GNU General Public License
 --  along with GCC; see the file COPYING3.   If not see
 --  <http://www.gnu.org/licenses/>.

BEGIN TRANSACTION;
CREATE TABLE t_params (parname VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE, parvalue TEXT NOT NULL);
CREATE TABLE t_items (itm_idstr VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE, itm_jdata TEXT NOT NULL);
CREATE TABLE t_names (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, n_idstr VARCHAR(30) UNIQUE NOT NULL, n_spacename VARCHAR(20) NOT NULL);
CREATE TABLE t_modules (modname VARCHAR(100) PRIMARY KEY ASC NOT NULL UNIQUE);
-- state-monimelt tables contents
INSERT INTO t_params VALUES('dump_format_version','MoniMelt2014B');
INSERT INTO t_params VALUES('dump_reason','command dump');
INSERT INTO t_items VALUES('_02av6173qvf_pehzhe755j2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for calls at end of  blocks"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_02q6zk9f5st_im0z75re15f','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the void C type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_02u53qxa7dm_sttmhffpchr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The agenda is central to Monimelt.\nIt is the queue of taskets to be executed by worker threads."}],
 "content": null, "kind": "queue", "payload": []}
');
INSERT INTO t_items VALUES('_06yp8ueq6yf_5ts408yww29','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the kind of the payload of an item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0afqepa7jkr_qky26hpv98d','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "translate a single procedure"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0ee6afx5850_ji17eq0wmfa','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Attribute giving the C type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0h331ch957p_j6a8i7v4e6y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0hpzi8m7wym_1y4ypmm9y47','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handler for JSONRPC requests"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0x2k07ik4tm_ed7vqphf5ak','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives verbatim code"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0yyp8vmw4si_wf49m4d4zwq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error code in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_11xee72y1d3_t3cqzi5dq3k','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "iterate on increasing integers"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_17spwr8dkzv_tsf2s8diazu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the double floating-point numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1f94j87qumw_mhzkriesx7c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to update the value in displays after edition."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1r880c1yk3z_i5e8mprcj90','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for blocks in routines, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_232t5qs2v8e_zu2wy53cqe3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for locked blocks"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_240dwt57s08_a8uy366sev5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "useless noop routine"}],
 "content": {"jtype": "node", "node": "_240dwt57s08_a8uy366sev5", "sons": ["{spare1 noop}",
   "{spare2 noop}", null]}, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_24w2ce2eq1z_pddi9j2czci','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "related to dump of full state"},
  {"attr": "_0hpzi8m7wym_1y4ypmm9y47", "val": {"item_ref": "_6zm92afs4yc_60a8ujmi1ef",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_24yt56xf3d5_4w80i326kjz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JIT code of JIT-ed routines"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_28941cvehx8_9rf4udyeq8v','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure routine name"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2d7i21ihwd8_xjcp4uhs11u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some variable"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2kxisdsque9_u9awek5wup1','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for jumps at end of  blocks"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2ky10qvckv2_kqa0pr8z29z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "starting block for test_fun1."},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": []}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2mayc646pdu_w4d18fmx8u3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some sequence"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2u8svx94yq4_34icz9j1fyx','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for constants in routines..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2ummst105ck_xracfy8v87y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "formal arguments..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2v75mmyph64_4h4kys78740','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate sets of item, or give their array of elements"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2vmrrvq5kdk_9um63pstcu9','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The true of JSON.\nWe cannot use true because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2x2zpyhfqum_0utui69rzea','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "arguments of procedure or primitive"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_35vp60aw7em_d436vfie4ud','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dump, jtype of JSON array values"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_37x98fyestf_ttup2cu68r6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the expansion of a primitive"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_38w4qwrmd6z_74x5z80v5k6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "inside switches: case <constant-expr> <block>"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3dqr46p2xf4_29kf5vdtw4z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Call with results in res, in given state, the closure clos with given arguments"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3fw5acswe59_9016fqe4d41','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the integer numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3hv5ymapjed_y8q6hsvhw8u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3j3s2e0510a_096chqpijq7','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "first result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3jpt8yuzuyw_ti1pyz3me1c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for terminated processes"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3uwzqwvj6zj_s63am4qivpt','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the value C type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3v4d7uzex6f_euek4pztiuh','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3wh3e88sk28_d27qi2737zi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for starting block in routines, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3xpyd539p4m_23h7wi59xi9','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dump, jtype of JSON object values"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41u1utcxyek_22cftxt3xxm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives a human-readable comment"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41v0erax6my_m6pytj0793u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the payload of items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41xwu6cpvq9_ezp5wzq7t4x','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the length of some sequence"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4215uc2u6qk_52kqyra86y5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_456hz6qd6x2_jyy24w6q84z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "common length integer"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_47fatww79x6_vh8ap22c0ch','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP HEAD method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4cw8jv45vsk_4mh9ex64904','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "frames in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4ezpkss1akd_94f4h25sqe4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the sons of nodes"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4jp2meuzru2_a58afyxwxa2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error message in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4m7x6811f6j_t480zu575mz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate nodes, or give their connective item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4mha85xcfwi_9zqcvkiy3dk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The false of JSON.\nWe cannot use false because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4p33dhxywm0_id6tti2kyw6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "locals of function"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4q1v3ax0ffi_hv7fwpsv6uf','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "our test module, to test the C code generation."},
  {"attr": "_9dcxaqk8tqe_fam9mcxme9w", "val": {"jtype": "set", "set": ["_682ush7cppa_s7vzfd2rsxp",
     "_8qmqy249w63_fxdr6rdz48m"]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4qcw2mwjswm_j9q0k9d04hm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "error case, on for JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4v74chqs1eh_chqd9cqw85t','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for procedure related things"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_51u3st4u9mc_zdvms6jti0a','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The C type for word integers"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_547q7emtfsk_ect0yratp6e','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to put in a given item, some attribute with some value"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_590trid9ycw_f6kaajwca63','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to make an item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5c5jh9185sv_qru5amf9v18','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "put an item inside an assoc"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5c789try94y_ssy6a22fpep','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for side-effecting instructions in blocks, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5vi29c2i54k_i2ufkty9kmp','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get the attribute"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5wmusj136kq_u5qpehp89aq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP POST method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5xa08a3ittw_imt86y9q33c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "starting block for test_proc1."},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": []}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_60ist2ad22c_cfpjp5ay6uj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "third result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6443sk5q0zt_8xdi02c6tzu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for ending some blocks"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_65961crktpj_vtt30qeqv21','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "first module, should become able to translate itself."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_682ush7cppa_s7vzfd2rsxp','{"attr": [{"attr": "_2u8svx94yq4_34icz9j1fyx", "val": {"jtype": "set", "set": ["_0yyp8vmw4si_wf49m4d4zwq"]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6djzuwz5pav_cri386ywjhj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error message in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6f4k9pqzryk_w25f8vxuyyc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "something for counting"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6f9870y6v8t_kp8fcmq2ezv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the state of Json for frames of tasklets."},
  {"attr": "_0h331ch957p_j6a8i7v4e6y", "val": {"item_ref": "_6p6v25323aq_97d9ude6j12",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_0hpzi8m7wym_1y4ypmm9y47",
   "val": {"item_ref": "_6p6v25323aq_97d9ude6j12", "jtype": "item_ref", "space": ".root"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6hf2vzmrsee_t35suhjvtj4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item reference, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6p6v25323aq_97d9ude6j12','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives thru JSONRPC the state of monimelt"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6qcw93kypcv_0iiepqtk73j','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "high bound"}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6w3dvx83dfw_xzc6aif6isv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the attribute[s], at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6zm92afs4yc_60a8ujmi1ef','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine for JSONRPC dump & exit"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70aer7teeui_kvzkiqq2rd2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "current procedure, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70mt4fvrva2_pk76eevwada','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "get value associated in some assoc item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_73im2zryfij_a7zmkketcfc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to edit a value during edition in ajax_objects"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_79vm7uxit6c_53qt1qi2wuj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "low bound"}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7a9sxskjhcp_kpf30ka97ex','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7diyc1cwj8z_x630afccr8e','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Group together all noticed values in dump outcome."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7j7x11c25h3_wkchtuwpusx','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the double C type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7pyjxst21ce_vhc0tk0em0u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to display a value during edition in ajax_objects"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7qk90k9vx0u_31ivff77td7','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "keep association between edited items and their editor"}],
 "content": null, "kind": "assoc", "payload": []}
');
INSERT INTO t_items VALUES('_7rf7axuc9h4_2aw6utwmsas','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the space of items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7sav6zery1v_24sa6jwwu6c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "attribute giving the web handler inside items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7sqk8vh89xr_6tj8dq7vqju','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "module to be compiled..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7urjeiw3evy_m7k72uv6790','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the type of a value"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7vw56h18sw0_hv77m6q8uxu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for tuples of items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7wk9y7e7r0z_575esi8ys5x','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for values in attribute lists of items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7yxp9xhih4z_9uzrqhkamxa','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for routines for tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_80e7dsukuq3_6p7jffmz1yi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The C type for constant literal C strings"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_80wxf4c8q92_qq8k6xc0xxj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives in sequence seq the element of given rank"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_86ft82euar7_cm50jcthhwe','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "start an associative item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8ejwdt1a5yx_2meizztvte0','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for conditionals, etc.."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8fk0de81s9r_5d4v4x7qmxr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for dispatching statements on items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8j516kuv89j_4hc4w6ykmr6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for long chunked strings"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8qmqy249w63_fxdr6rdz48m','{"attr": [{"attr": "_2u8svx94yq4_34icz9j1fyx", "val": {"jtype": "set", "set": ["_41u1utcxyek_22cftxt3xxm",
     "_7wk9y7e7r0z_575esi8ys5x"]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8qpa7j0chkh_k630ujw6jiw','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for switch statements on integers"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8s357rq2dzk_k8ze95tikjm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item content, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8u5ar84utwm_99k5mq2d589','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "second result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8vzddhpmhp8_z0005cqyqzf','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for assignment <var> := <expr>"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8x6fxcm4z2k_vdaqicfi4z0','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "used as node of primitive expansions, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8y1sw8z084j_4ts0y0jydha','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "value containing the translation"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_91pketvc5pz_wq0v0wpauw8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the values of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_94tq2iauet8_jujpjhjrzsm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for block items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_97zkxf62r11_6eedwwv3eu8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure payload"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9dcxaqk8tqe_fam9mcxme9w','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the routines in a module"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9dsak0qcy0v_1c5z9th7x3i','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP GET method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9mxi9e605ay_ihpjyrwq250','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure values"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9sd1mh9q1zf_3duewi6fsaq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes with exit code >0"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9wwqwxqcm4p_y7di7fs8tsk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "common index number"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_names VALUES('GET','_9dsak0qcy0v_1c5z9th7x3i','.predef');
INSERT INTO t_names VALUES('HEAD','_47fatww79x6_vh8ap22c0ch','.predef');
INSERT INTO t_names VALUES('POST','_5wmusj136kq_u5qpehp89aq','.predef');
INSERT INTO t_names VALUES('agenda','_02u53qxa7dm_sttmhffpchr','.predef');
INSERT INTO t_names VALUES('arguments','_2x2zpyhfqum_0utui69rzea','.predef');
INSERT INTO t_names VALUES('assign','_8vzddhpmhp8_z0005cqyqzf','.predef');
INSERT INTO t_names VALUES('assoc_get','_70mt4fvrva2_pk76eevwada','.root');
INSERT INTO t_names VALUES('assoc_put','_5c5jh9185sv_qru5amf9v18','.root');
INSERT INTO t_names VALUES('attr','_6w3dvx83dfw_xzc6aif6isv','.predef');
INSERT INTO t_names VALUES('block','_94tq2iauet8_jujpjhjrzsm','.predef');
INSERT INTO t_names VALUES('blocks','_1r880c1yk3z_i5e8mprcj90','.predef');
INSERT INTO t_names VALUES('call','_02av6173qvf_pehzhe755j2','.predef');
INSERT INTO t_names VALUES('call_at_state','_3dqr46p2xf4_29kf5vdtw4z','.root');
INSERT INTO t_names VALUES('case','_38w4qwrmd6z_74x5z80v5k6','.predef');
INSERT INTO t_names VALUES('chunk','_8x6fxcm4z2k_vdaqicfi4z0','.predef');
INSERT INTO t_names VALUES('closed_values','_9mxi9e605ay_ihpjyrwq250','.predef');
INSERT INTO t_names VALUES('closure','_97zkxf62r11_6eedwwv3eu8','.predef');
INSERT INTO t_names VALUES('closure_routine','_28941cvehx8_9rf4udyeq8v','.predef');
INSERT INTO t_names VALUES('code','_0yyp8vmw4si_wf49m4d4zwq','.predef');
INSERT INTO t_names VALUES('comment','_41u1utcxyek_22cftxt3xxm','.predef');
INSERT INTO t_names VALUES('constants','_2u8svx94yq4_34icz9j1fyx','.predef');
INSERT INTO t_names VALUES('content','_8s357rq2dzk_k8ze95tikjm','.predef');
INSERT INTO t_names VALUES('count','_6f4k9pqzryk_w25f8vxuyyc','.predef');
INSERT INTO t_names VALUES('ctype','_0ee6afx5850_ji17eq0wmfa','.predef');
INSERT INTO t_names VALUES('dispatch','_8fk0de81s9r_5d4v4x7qmxr','.predef');
INSERT INTO t_names VALUES('display_value','_7pyjxst21ce_vhc0tk0em0u','.root');
INSERT INTO t_names VALUES('do','_5c789try94y_ssy6a22fpep','.predef');
INSERT INTO t_names VALUES('double','_7j7x11c25h3_wkchtuwpusx','.predef');
INSERT INTO t_names VALUES('doubles','_17spwr8dkzv_tsf2s8diazu','.predef');
INSERT INTO t_names VALUES('dump','_24w2ce2eq1z_pddi9j2czci','.root');
INSERT INTO t_names VALUES('edit_value','_73im2zryfij_a7zmkketcfc','.root');
INSERT INTO t_names VALUES('editors','_7qk90k9vx0u_31ivff77td7','.root');
INSERT INTO t_names VALUES('error','_4qcw2mwjswm_j9q0k9d04hm','.predef');
INSERT INTO t_names VALUES('exited','_3v4d7uzex6f_euek4pztiuh','.predef');
INSERT INTO t_names VALUES('failed','_9sd1mh9q1zf_3duewi6fsaq','.predef');
INSERT INTO t_names VALUES('first_module','_65961crktpj_vtt30qeqv21','.root');
INSERT INTO t_names VALUES('for_each_up_to','_11xee72y1d3_t3cqzi5dq3k','.root');
INSERT INTO t_names VALUES('formals','_2ummst105ck_xracfy8v87y','.predef');
INSERT INTO t_names VALUES('frames','_4cw8jv45vsk_4mh9ex64904','.predef');
INSERT INTO t_names VALUES('get_attribute','_5vi29c2i54k_i2ufkty9kmp','.root');
INSERT INTO t_names VALUES('high','_6qcw93kypcv_0iiepqtk73j','.root');
INSERT INTO t_names VALUES('id','_7a9sxskjhcp_kpf30ka97ex','.predef');
INSERT INTO t_names VALUES('if','_8ejwdt1a5yx_2meizztvte0','.predef');
INSERT INTO t_names VALUES('intptr_t','_51u3st4u9mc_zdvms6jti0a','.predef');
INSERT INTO t_names VALUES('item_ref','_6hf2vzmrsee_t35suhjvtj4','.predef');
INSERT INTO t_names VALUES('ix','_9wwqwxqcm4p_y7di7fs8tsk','.root');
INSERT INTO t_names VALUES('jit','_24yt56xf3d5_4w80i326kjz','.predef');
INSERT INTO t_names VALUES('json_array','_35vp60aw7em_d436vfie4ud','.predef');
INSERT INTO t_names VALUES('json_false','_4mha85xcfwi_9zqcvkiy3dk','.predef');
INSERT INTO t_names VALUES('json_object','_3xpyd539p4m_23h7wi59xi9','.predef');
INSERT INTO t_names VALUES('json_true','_2vmrrvq5kdk_9um63pstcu9','.predef');
INSERT INTO t_names VALUES('jsonrpc','_0h331ch957p_j6a8i7v4e6y','.predef');
INSERT INTO t_names VALUES('jsonrpc_handler','_0hpzi8m7wym_1y4ypmm9y47','.predef');
INSERT INTO t_names VALUES('jtype','_7urjeiw3evy_m7k72uv6790','.predef');
INSERT INTO t_names VALUES('jump','_2kxisdsque9_u9awek5wup1','.predef');
INSERT INTO t_names VALUES('kind','_06yp8ueq6yf_5ts408yww29','.predef');
INSERT INTO t_names VALUES('len','_456hz6qd6x2_jyy24w6q84z','.root');
INSERT INTO t_names VALUES('locals','_4p33dhxywm0_id6tti2kyw6','.predef');
INSERT INTO t_names VALUES('lock','_232t5qs2v8e_zu2wy53cqe3','.predef');
INSERT INTO t_names VALUES('low','_79vm7uxit6c_53qt1qi2wuj','.root');
INSERT INTO t_names VALUES('make_item','_590trid9ycw_f6kaajwca63','.root');
INSERT INTO t_names VALUES('message','_4jp2meuzru2_a58afyxwxa2','.predef');
INSERT INTO t_names VALUES('method','_3hv5ymapjed_y8q6hsvhw8u','.predef');
INSERT INTO t_names VALUES('module','_7sqk8vh89xr_6tj8dq7vqju','.predef');
INSERT INTO t_names VALUES('module_routines','_9dcxaqk8tqe_fam9mcxme9w','.predef');
INSERT INTO t_names VALUES('momcstr_t','_80e7dsukuq3_6p7jffmz1yi','.predef');
INSERT INTO t_names VALUES('momval_t','_3uwzqwvj6zj_s63am4qivpt','.predef');
INSERT INTO t_names VALUES('node','_4m7x6811f6j_t480zu575mz','.predef');
INSERT INTO t_names VALUES('noop','_240dwt57s08_a8uy366sev5','.root');
INSERT INTO t_names VALUES('notice','_7diyc1cwj8z_x630afccr8e','.predef');
INSERT INTO t_names VALUES('numbers','_3fw5acswe59_9016fqe4d41','.predef');
INSERT INTO t_names VALUES('params','_4215uc2u6qk_52kqyra86y5','.predef');
INSERT INTO t_names VALUES('payload','_41v0erax6my_m6pytj0793u','.predef');
INSERT INTO t_names VALUES('primitive_expansion','_37x98fyestf_ttup2cu68r6','.predef');
INSERT INTO t_names VALUES('proc','_70aer7teeui_kvzkiqq2rd2','.root');
INSERT INTO t_names VALUES('procedure','_4v74chqs1eh_chqd9cqw85t','.predef');
INSERT INTO t_names VALUES('put_attribute','_547q7emtfsk_ect0yratp6e','.root');
INSERT INTO t_names VALUES('res1','_3j3s2e0510a_096chqpijq7','.predef');
INSERT INTO t_names VALUES('res2','_8u5ar84utwm_99k5mq2d589','.predef');
INSERT INTO t_names VALUES('res3','_60ist2ad22c_cfpjp5ay6uj','.predef');
INSERT INTO t_names VALUES('result','_6djzuwz5pav_cri386ywjhj','.predef');
INSERT INTO t_names VALUES('return','_6443sk5q0zt_8xdi02c6tzu','.predef');
INSERT INTO t_names VALUES('seq','_2mayc646pdu_w4d18fmx8u3','.root');
INSERT INTO t_names VALUES('sequence_length','_41xwu6cpvq9_ezp5wzq7t4x','.root');
INSERT INTO t_names VALUES('sequence_nth','_80wxf4c8q92_qq8k6xc0xxj','.root');
INSERT INTO t_names VALUES('set','_2v75mmyph64_4h4kys78740','.predef');
INSERT INTO t_names VALUES('sons','_4ezpkss1akd_94f4h25sqe4','.predef');
INSERT INTO t_names VALUES('space','_7rf7axuc9h4_2aw6utwmsas','.predef');
INSERT INTO t_names VALUES('start','_3wh3e88sk28_d27qi2737zi','.predef');
INSERT INTO t_names VALUES('start_assoc','_86ft82euar7_cm50jcthhwe','.root');
INSERT INTO t_names VALUES('state','_6f9870y6v8t_kp8fcmq2ezv','.predef');
INSERT INTO t_names VALUES('string','_8j516kuv89j_4hc4w6ykmr6','.predef');
INSERT INTO t_names VALUES('switch','_8qpa7j0chkh_k630ujw6jiw','.predef');
INSERT INTO t_names VALUES('tasklet_function','_7yxp9xhih4z_9uzrqhkamxa','.predef');
INSERT INTO t_names VALUES('terminated','_3jpt8yuzuyw_ti1pyz3me1c','.predef');
INSERT INTO t_names VALUES('test_fun1','_8qmqy249w63_fxdr6rdz48m','.root');
INSERT INTO t_names VALUES('test_module','_4q1v3ax0ffi_hv7fwpsv6uf','.root');
INSERT INTO t_names VALUES('test_proc1','_682ush7cppa_s7vzfd2rsxp','.root');
INSERT INTO t_names VALUES('translate_procedure','_0afqepa7jkr_qky26hpv98d','.root');
INSERT INTO t_names VALUES('translation','_8y1sw8z084j_4ts0y0jydha','.root');
INSERT INTO t_names VALUES('tuple','_7vw56h18sw0_hv77m6q8uxu','.predef');
INSERT INTO t_names VALUES('update_display_value','_1f94j87qumw_mhzkriesx7c','.root');
INSERT INTO t_names VALUES('val','_7wk9y7e7r0z_575esi8ys5x','.predef');
INSERT INTO t_names VALUES('values','_91pketvc5pz_wq0v0wpauw8','.predef');
INSERT INTO t_names VALUES('var','_2d7i21ihwd8_xjcp4uhs11u','.root');
INSERT INTO t_names VALUES('verbatim','_0x2k07ik4tm_ed7vqphf5ak','.predef');
INSERT INTO t_names VALUES('void','_02q6zk9f5st_im0z75re15f','.predef');
INSERT INTO t_names VALUES('web_handler','_7sav6zery1v_24sa6jwwu6c','.predef');
COMMIT;
-- state-monimelt end dump 
