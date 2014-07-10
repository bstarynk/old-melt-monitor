-- state-monimelt dump 2014 Jul 10

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
INSERT INTO t_params VALUES('dump_reason','after predefined params');
INSERT INTO t_items VALUES('_02u53qxa7dm_sttmhffpchr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The agenda is central to Monimelt.\nIt is the queue of taskets to be executed by worker threads."}],
 "content": null, "kind": "queue", "payload": []}
');
INSERT INTO t_items VALUES('_06yp8ueq6yf_5ts408yww29','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the kind of the payload of an item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_085krqf192t_z1m3zs77ww5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to be used inside display items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0acmecj244a_6krws4rx7v1','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the body in a routine, or the routine elsewhere"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0afqepa7jkr_qky26hpv98d','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "translate a single procedure"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0h331ch957p_j6a8i7v4e6y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0hpzi8m7wym_1y4ypmm9y47','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handler for JSON requests"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0ihu411vkua_z4sh56hicdt','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "body of a routine"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0zmdkdxj7kp_491yqpcuaz8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "assignment operator"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_2d7i21ihwd8_xjcp4uhs11u",
     "_7wk9y7e7r0z_575esi8ys5x"]}}, {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype":
    "node", "node": "_2vxxtir316j_meap5sq6ykr", "sons": [{"item_ref": "_2d7i21ihwd8_xjcp4uhs11u",
      "jtype": "item_ref", "space": ".root"}, " = (", {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
      "jtype": "item_ref", "space": ".predef"}, ")"]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_11xee72y1d3_t3cqzi5dq3k','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "iterate on increasing integers"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_9wwqwxqcm4p_y7di7fs8tsk",
     "_79vm7uxit6c_53qt1qi2wuj", "_6qcw93kypcv_0iiepqtk73j", "_0ihu411vkua_z4sh56hicdt"]}},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": ["for(", {"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref",
      "space": ".root"}, "=", {"item_ref": "_79vm7uxit6c_53qt1qi2wuj", "jtype": "item_ref",
      "space": ".root"}, "; ", {"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref",
      "space": ".root"}, "<", {"item_ref": "_6qcw93kypcv_0iiepqtk73j", "jtype": "item_ref",
      "space": ".root"}, "; ", {"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref",
      "space": ".root"}, "++) {", {"item_ref": "_0ihu411vkua_z4sh56hicdt",
      "jtype": "item_ref", "space": ".predef"}, "}"]}}, {"attr": "_70ty9z1tm4p_eccsxmyfe25",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_17spwr8dkzv_tsf2s8diazu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the double floating-point numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1f94j87qumw_mhzkriesx7c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to update the value in displays after edition."}],
 "content": null, "kind": "routine", "payload": "update_display_value"}
');
INSERT INTO t_items VALUES('_240dwt57s08_a8uy366sev5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "useless noop routine"}],
 "content": {"jtype": "node", "node": "_240dwt57s08_a8uy366sev5", "sons": ["{spare1 noop}",
   "{spare2 noop}", null]}, "kind": "routine", "payload": "noop"}
');
INSERT INTO t_items VALUES('_28941cvehx8_9rf4udyeq8v','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure routine name"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2d7i21ihwd8_xjcp4uhs11u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some variable"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2mayc646pdu_w4d18fmx8u3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some sequence"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2v75mmyph64_4h4kys78740','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate sets of item, or give their array of elements"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2vmrrvq5kdk_9um63pstcu9','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The true of JSON.\nWe cannot use true because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2vxxtir316j_meap5sq6ykr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the expansion of a primitive"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_356014y9ueu_xv6j0eskszw','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for input display"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_35a1p2kdx9h_ap5pe704vtr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "ajax routine for edition"},
  {"attr": "_7sav6zery1v_24sa6jwwu6c", "val": {"jtype": "node", "node": "_35a1p2kdx9h_ap5pe704vtr",
    "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
      "space": ".root"}, {"jtype": "node", "node": "_73im2zryfij_a7zmkketcfc",
      "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
"space": ".root"}, "{spare1-edit_value}"]}, {"jtype": "node", "node": "_7pyjxst21ce_vhc0tk0em0u",
      "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
"space": ".root"}, "{spare1-display_value}"]}, {"jtype": "node", "node": "_1f94j87qumw_mhzkriesx7c",
      "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
"space": ".root"}, "{spare1-update_display_value}"]}, "{spare5-ajax_edit}",
     null]}}], "content": null, "kind": "routine", "payload": "ajax_edit"}
');
INSERT INTO t_items VALUES('_35vp60aw7em_d436vfie4ud','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dump, jtype of JSON array values"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_36tp2s8s5s2_jzjm0cxdpjz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "attribute giving parent of a display, or of some graph, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_38s7ihasu0m_xzipyerxm3j','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the editor inside a display"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3dqr46p2xf4_29kf5vdtw4z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Call with results in res, in given state, the closure clos with given arguments"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_6djzuwz5pav_cri386ywjhj",
     "_6f9870y6v8t_kp8fcmq2ezv", "_97zkxf62r11_6eedwwv3eu8", "_8um1q4shitk_tpcmedvsfzu"]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3eu0rdq4upj_dp5ptr6hj04','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the expansion of a routine"}],
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
INSERT INTO t_items VALUES('_3v4d7uzex6f_euek4pztiuh','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3xpyd539p4m_23h7wi59xi9','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dump, jtype of JSON object values"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3xz3qrc6mfy_4r51up6u3pa','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handle ''ajax_system'' webrequests"},
  {"attr": "_7sav6zery1v_24sa6jwwu6c", "val": {"item_ref": "_3xz3qrc6mfy_4r51up6u3pa",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": "closure",
 "payload": {"closed_values": ["{spare closed-value ajax-system-0}", "{spare closed-value ajax-system-1}"],
  "closure_routine": "ajax_system"}}
');
INSERT INTO t_items VALUES('_3zqd7ai3rtu_md9athkx17u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the buffer payload kind, and also the clipboard buffer in editors"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41u1utcxyek_22cftxt3xxm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives a human-readable comment"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41v0erax6my_m6pytj0793u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the payload of items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41xwu6cpvq9_ezp5wzq7t4x','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the length of some sequence"},
  {"attr": "_70ty9z1tm4p_eccsxmyfe25", "val": {"item_ref": "_9jeymqk2732_wiq5kyczi9c",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_8um1q4shitk_tpcmedvsfzu",
   "val": {"jtype": "tuple", "tuple": ["_2mayc646pdu_w4d18fmx8u3"]}}, {"attr": "_967fch1xu4h_i87qjq1zt1h",
   "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr", "sons": ["mom_seqitem_length(",
     {"item_ref": "_2mayc646pdu_w4d18fmx8u3", "jtype": "item_ref", "space": ".root"},
     ")"]}}], "content": null, "kind": null, "payload": null}
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
INSERT INTO t_items VALUES('_4m7x6811f6j_t480zu575mz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate nodes, or give their connective item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4mha85xcfwi_9zqcvkiy3dk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The false of JSON.\nWe cannot use false because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4v93t3jzrtz_srt9ear8fm8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handle ''ajax_complete_name'' webrequests"},
  {"attr": "_7sav6zery1v_24sa6jwwu6c", "val": {"jtype": "node", "node": "_4v93t3jzrtz_srt9ear8fm8",
    "sons": ["{spare1 ajax-complete_name}", "{spare2 ajax-complete_name}",
     "{spare3 ajax-complete_name}", null]}}], "content": null, "kind": "routine",
 "payload": "ajax_complete_name"}
');
INSERT INTO t_items VALUES('_4yxdswc8qwf_vxzy95hd399','{"attr": [{"attr": "_3eu0rdq4upj_dp5ptr6hj04", "val": {"jtype": "tuple", "tuple":
    [{"item_ref": "_8y1sw8z084j_4ts0y0jydha", "jtype": "item_ref", "space": ".root"},
     "_5yfdp53cpi1_0i5k33wms7c"]}}, {"attr": "_3fw5acswe59_9016fqe4d41", "val": {"jtype":
    "tuple", "tuple": [{"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref",
      "space": ".root"}, {"item_ref": "_456hz6qd6x2_jyy24w6q84z", "jtype": "item_ref",
      "space": ".root"}]}}, {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype":
    "tuple", "tuple": ["_7sqk8vh89xr_6tj8dq7vqju"]}}, {"attr": "_0acmecj244a_6krws4rx7v1",
   "val": {"jtype": "node", "node": "_0ihu411vkua_z4sh56hicdt", "sons": [{"jtype":
      "node", "node": {"item_ref": "_590trid9ycw_f6kaajwca63", "jtype": "item_ref",
       "space": ".root"}, "sons": [{"item_ref": "_8y1sw8z084j_4ts0y0jydha",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": {"item_ref": "_86ft82euar7_cm50jcthhwe",
       "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_8y1sw8z084j_4ts0y0jydha",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": {"item_ref": "_547q7emtfsk_ect0yratp6e",
       "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_8y1sw8z084j_4ts0y0jydha",
"jtype": "item_ref", "space": ".root"}, {"jtype": "node", "node": "_53cuy70z4tf_86tzz364trd",
"sons": [{"item_ref": "_7sqk8vh89xr_6tj8dq7vqju", "jtype": "item_ref", "space": ".predef"}]},
       {"item_ref": "_7sqk8vh89xr_6tj8dq7vqju", "jtype": "item_ref", "space": ".predef"}]},
     {"jtype": "node", "node": {"item_ref": "_0zmdkdxj7kp_491yqpcuaz8", "jtype": "item_ref",
       "space": ".root"}, "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c",
"jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": {"item_ref": "_5vi29c2i54k_i2ufkty9kmp",
 "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_7sqk8vh89xr_6tj8dq7vqju",
  "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_53cuy70z4tf_86tzz364trd",
  "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c", "jtype": "item_ref", "space":
    ".predef"}]}]}]}, {"jtype": "node", "node": {"item_ref": "_0zmdkdxj7kp_491yqpcuaz8",
       "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_456hz6qd6x2_jyy24w6q84z",
"jtype": "item_ref", "space": ".root"}, {"jtype": "node", "node": {"item_ref": "_41xwu6cpvq9_ezp5wzq7t4x",
 "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c",
  "jtype": "item_ref", "space": ".predef"}]}]}, {"jtype": "node", "node": {"item_ref":
       "_11xee72y1d3_t3cqzi5dq3k", "jtype": "item_ref", "space": ".root"},
      "sons": [{"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref",
"space": ".root"}, 0, {"item_ref": "_456hz6qd6x2_jyy24w6q84z", "jtype": "item_ref",
"space": ".root"}, {"jtype": "node", "node": "_0ihu411vkua_z4sh56hicdt", "sons": [{"jtype":
  "node", "node": {"item_ref": "_0zmdkdxj7kp_491yqpcuaz8", "jtype": "item_ref",
   "space": ".root"}, "sons": [{"item_ref": "_70aer7teeui_kvzkiqq2rd2", "jtype": "item_ref",
    "space": ".root"}, {"jtype": "node", "node": {"item_ref": "_80wxf4c8q92_qq8k6xc0xxj",
     "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk",
      "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": {"item_ref":
   "_3dqr46p2xf4_29kf5vdtw4z", "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref":
    "_3j3s2e0510a_096chqpijq7", "jtype": "item_ref", "space": ".predef"},
   {"item_ref": "_6qp266amrz7_izi5rx6ukuk", "jtype": "item_ref", "space": ".root"},
   {"item_ref": "_0afqepa7jkr_qky26hpv98d", "jtype": "item_ref", "space": ".root"},
   {"item_ref": "_70aer7teeui_kvzkiqq2rd2", "jtype": "item_ref", "space": ".root"},
   {"item_ref": "_8y1sw8z084j_4ts0y0jydha", "jtype": "item_ref", "space": ".root"}]}]}]}]}}],
 "content": null, "kind": "routine", "payload": "translate_module"}
');
INSERT INTO t_items VALUES('_53748kde7s1_pkz810exr27','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "used in editor, etc. to reference some item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_53cuy70z4tf_86tzz364trd','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to mention some item verbatim or quote-d \u00e0 la Lisp"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_547q7emtfsk_ect0yratp6e','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to put in a given item, some attribute with some value"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_53748kde7s1_pkz810exr27",
     "_6w3dvx83dfw_xzc6aif6isv", "_7wk9y7e7r0z_575esi8ys5x"]}}, {"attr": "_967fch1xu4h_i87qjq1zt1h",
   "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr", "sons": ["mom_item_put_attribute(mom_value_to_item(",
     {"item_ref": "_53748kde7s1_pkz810exr27", "jtype": "item_ref", "space": ".predef"},
     "), (", {"item_ref": "_6w3dvx83dfw_xzc6aif6isv", "jtype": "item_ref",
      "space": ".predef"}, "), (\"", {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
      "jtype": "item_ref", "space": ".predef"}, "))"]}}, {"attr": "_70ty9z1tm4p_eccsxmyfe25",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_590trid9ycw_f6kaajwca63','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to make an item"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_3j3s2e0510a_096chqpijq7"]}},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": [{"item_ref": "_3j3s2e0510a_096chqpijq7", "jtype": "item_ref",
      "space": ".predef"}, " = (momval_t) mom_make_item()"]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5c5jh9185sv_qru5amf9v18','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "put an item inside an assoc"},
  {"attr": "_70ty9z1tm4p_eccsxmyfe25", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_967fch1xu4h_i87qjq1zt1h",
   "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr", "sons": ["mom_item_assoc_put((momitem_t*)",
     {"item_ref": "_53748kde7s1_pkz810exr27", "jtype": "item_ref", "space": ".predef"},
     ", mom_value_to_item(", {"item_ref": "_6w3dvx83dfw_xzc6aif6isv", "jtype": "item_ref",
      "space": ".predef"}, "), ", {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
      "jtype": "item_ref", "space": ".predef"}, ")"]}}, {"attr": "_8um1q4shitk_tpcmedvsfzu",
   "val": {"jtype": "tuple", "tuple": ["_53748kde7s1_pkz810exr27", "_6w3dvx83dfw_xzc6aif6isv",
     "_7wk9y7e7r0z_575esi8ys5x"]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5s59qeamxta_70k0mt77r9i','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the size, e.g. in editors"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5sw59dauckp_8eustjwf58u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to be used inside display items to give the origin"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5vi29c2i54k_i2ufkty9kmp','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get the attribute"},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": ["mom_item_get_attribute(mom_value_to_item(", {"item_ref": "_53748kde7s1_pkz810exr27",
      "jtype": "item_ref", "space": ".predef"}, "), mom_value_to_item(", {"item_ref":
      "_6w3dvx83dfw_xzc6aif6isv", "jtype": "item_ref", "space": ".predef"},
     "))"]}}, {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple",
    "tuple": ["_53748kde7s1_pkz810exr27", "_6w3dvx83dfw_xzc6aif6isv"]}}, {"attr":
   "_70ty9z1tm4p_eccsxmyfe25", "val": {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_5wmusj136kq_u5qpehp89aq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP POST method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5x41iah0kis_x8rrv3ww44t','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the update time when applicable"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5yfdp53cpi1_0i5k33wms7c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "give the procedures inside a module"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_60ist2ad22c_cfpjp5ay6uj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "third result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_65961crktpj_vtt30qeqv21','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "first module, should become able to translate itself."},
  {"attr": "_5yfdp53cpi1_0i5k33wms7c", "val": {"jtype": "set", "set": ["_0afqepa7jkr_qky26hpv98d",
     "_4yxdswc8qwf_vxzy95hd399"]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6djzuwz5pav_cri386ywjhj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for the result name or tuple"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6f9870y6v8t_kp8fcmq2ezv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the state of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6hf2vzmrsee_t35suhjvtj4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item reference, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6mwwr0i4y9p_5aupdxjxdk1','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handle ''ajax_objects'' webrequests"},
  {"attr": "_7sav6zery1v_24sa6jwwu6c", "val": {"jtype": "node", "node": "_6mwwr0i4y9p_5aupdxjxdk1",
    "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
      "space": ".root"}, {"jtype": "node", "node": "_73im2zryfij_a7zmkketcfc",
      "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
"space": ".root"}, "{spare1-edit_value}"]}, {"jtype": "node", "node": "_7pyjxst21ce_vhc0tk0em0u",
      "sons": [{"item_ref": "_7qk90k9vx0u_31ivff77td7", "jtype": "item_ref",
"space": ".root"}, "{spare1-display_value}"]}, "{spare4-ajax_objects}", null]}}],
 "content": null, "kind": "routine", "payload": "ajax_objects"}
');
INSERT INTO t_items VALUES('_6qcw93kypcv_0iiepqtk73j','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "high bound"}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6qp266amrz7_izi5rx6ukuk','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6w3dvx83dfw_xzc6aif6isv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the attribute[s], at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70aer7teeui_kvzkiqq2rd2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "current procedure, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70mt4fvrva2_pk76eevwada','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "get value associated in some assoc item"},
  {"attr": "_70ty9z1tm4p_eccsxmyfe25", "val": {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_8um1q4shitk_tpcmedvsfzu",
   "val": {"jtype": "tuple", "tuple": ["_53748kde7s1_pkz810exr27", "_6w3dvx83dfw_xzc6aif6isv"]}},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": ["mom_item_assoc_get((momitem_t*)", {"item_ref": "_53748kde7s1_pkz810exr27",
      "jtype": "item_ref", "space": ".predef"}, ",", {"item_ref": "_6w3dvx83dfw_xzc6aif6isv",
      "jtype": "item_ref", "space": ".predef"}, ")"]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70ty9z1tm4p_eccsxmyfe25','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "within a primitive gives the type of result"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_73im2zryfij_a7zmkketcfc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to edit a value during edition in ajax_objects"}],
 "content": null, "kind": "routine", "payload": "edit_value"}
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
INSERT INTO t_items VALUES('_7jzvaihqxfw_0c2y7t976tu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "type for no data"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7kkh6qiq1vc_e69zp2feuhe','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the rank"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7pyjxst21ce_vhc0tk0em0u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to display a value during edition in ajax_objects"}],
 "content": null, "kind": "routine", "payload": "display_value"}
');
INSERT INTO t_items VALUES('_7qk90k9vx0u_31ivff77td7','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "keep association between edited items and their editor"},
  {"attr": "_3zqd7ai3rtu_md9athkx17u", "val": "to mention some item verbatim or quote-d \u00e0a Lisp"}],
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
INSERT INTO t_items VALUES('_80wxf4c8q92_qq8k6xc0xxj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives in sequence seq the element of given rank"},
  {"attr": "_70ty9z1tm4p_eccsxmyfe25", "val": {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_967fch1xu4h_i87qjq1zt1h",
   "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr", "sons": ["(momval_t) mom_seqitem_nth_item(",
     {"item_ref": "_2mayc646pdu_w4d18fmx8u3", "jtype": "item_ref", "space": ".root"},
     ", ", {"item_ref": "_7kkh6qiq1vc_e69zp2feuhe", "jtype": "item_ref", "space":
      ".predef"}, ")"]}}, {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple",
    "tuple": ["_2mayc646pdu_w4d18fmx8u3", "_7kkh6qiq1vc_e69zp2feuhe"]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_85rz4j0q982_67im8sstj9s','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "miscellanous attribute, e.g. to test the editor"},
  {"attr": "_2v75mmyph64_4h4kys78740", "val": {"jtype": "set", "set": ["_91pketvc5pz_wq0v0wpauw8"]}},
  {"attr": "_7vw56h18sw0_hv77m6q8uxu", "val": {"jtype": "tuple", "tuple": ["_7wk9y7e7r0z_575esi8ys5x",
     "_7vw56h18sw0_hv77m6q8uxu"]}}, {"attr": "_4m7x6811f6j_t480zu575mz", "val": {"jtype":
    "node", "node": "_4m7x6811f6j_t480zu575mz", "sons": [{"jtype": "node",
      "node": "_2v75mmyph64_4h4kys78740", "sons": [{"jtype": "set", "set": ["_2v75mmyph64_4h4kys78740"]}]},
     {"jtype": "node", "node": "_7vw56h18sw0_hv77m6q8uxu", "sons": [{"jtype": "tuple",
"tuple": ["_7vw56h18sw0_hv77m6q8uxu"]}]}, {"jtype": "node", "node": "_4m7x6811f6j_t480zu575mz",
      "sons": [123, "some long string", null]}, null]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_86ft82euar7_cm50jcthhwe','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "start an associative item"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_53748kde7s1_pkz810exr27"]}},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": ["mom_item_start_assoc((momitem_t*)", {"item_ref": "_53748kde7s1_pkz810exr27",
      "jtype": "item_ref", "space": ".predef"}, ")"]}}, {"attr": "_70ty9z1tm4p_eccsxmyfe25",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_89ejvxupprm_f219pqwz13s','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for double values and displays"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8j516kuv89j_4hc4w6ykmr6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for long chunked strings"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8s357rq2dzk_k8ze95tikjm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item content, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8s4wcve2u49_252vwyzyrxd','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to be used inside display items for null"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8u5ar84utwm_99k5mq2d589','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "second result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8um1q4shitk_tpcmedvsfzu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "arguments of a routine"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8y1sw8z084j_4ts0y0jydha','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "value containing the translation"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_91pketvc5pz_wq0v0wpauw8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the values of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_967fch1xu4h_i87qjq1zt1h','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for the expansion of a primitive"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_97zkxf62r11_6eedwwv3eu8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure payload"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9dsak0qcy0v_1c5z9th7x3i','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP GET method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9jeymqk2732_wiq5kyczi9c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for integer values and displays"}],
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
INSERT INTO t_names VALUES('ajax_complete_name','_4v93t3jzrtz_srt9ear8fm8','.root');
INSERT INTO t_names VALUES('ajax_edit','_35a1p2kdx9h_ap5pe704vtr','.root');
INSERT INTO t_names VALUES('ajax_objects','_6mwwr0i4y9p_5aupdxjxdk1','.root');
INSERT INTO t_names VALUES('ajax_system','_3xz3qrc6mfy_4r51up6u3pa','.root');
INSERT INTO t_names VALUES('arguments','_8um1q4shitk_tpcmedvsfzu','.predef');
INSERT INTO t_names VALUES('assign','_0zmdkdxj7kp_491yqpcuaz8','.root');
INSERT INTO t_names VALUES('assoc_get','_70mt4fvrva2_pk76eevwada','.root');
INSERT INTO t_names VALUES('assoc_put','_5c5jh9185sv_qru5amf9v18','.root');
INSERT INTO t_names VALUES('attr','_6w3dvx83dfw_xzc6aif6isv','.predef');
INSERT INTO t_names VALUES('body','_0ihu411vkua_z4sh56hicdt','.predef');
INSERT INTO t_names VALUES('buffer','_3zqd7ai3rtu_md9athkx17u','.predef');
INSERT INTO t_names VALUES('call_at_state','_3dqr46p2xf4_29kf5vdtw4z','.root');
INSERT INTO t_names VALUES('closed_values','_9mxi9e605ay_ihpjyrwq250','.predef');
INSERT INTO t_names VALUES('closure','_97zkxf62r11_6eedwwv3eu8','.predef');
INSERT INTO t_names VALUES('closure_routine','_28941cvehx8_9rf4udyeq8v','.predef');
INSERT INTO t_names VALUES('comment','_41u1utcxyek_22cftxt3xxm','.predef');
INSERT INTO t_names VALUES('content','_8s357rq2dzk_k8ze95tikjm','.predef');
INSERT INTO t_names VALUES('display','_085krqf192t_z1m3zs77ww5','.predef');
INSERT INTO t_names VALUES('display_value','_7pyjxst21ce_vhc0tk0em0u','.root');
INSERT INTO t_names VALUES('double','_89ejvxupprm_f219pqwz13s','.predef');
INSERT INTO t_names VALUES('doubles','_17spwr8dkzv_tsf2s8diazu','.predef');
INSERT INTO t_names VALUES('edit_value','_73im2zryfij_a7zmkketcfc','.root');
INSERT INTO t_names VALUES('editor','_38s7ihasu0m_xzipyerxm3j','.predef');
INSERT INTO t_names VALUES('editors','_7qk90k9vx0u_31ivff77td7','.root');
INSERT INTO t_names VALUES('empty','_8s4wcve2u49_252vwyzyrxd','.predef');
INSERT INTO t_names VALUES('exited','_3v4d7uzex6f_euek4pztiuh','.predef');
INSERT INTO t_names VALUES('expansion','_2vxxtir316j_meap5sq6ykr','.predef');
INSERT INTO t_names VALUES('failed','_9sd1mh9q1zf_3duewi6fsaq','.predef');
INSERT INTO t_names VALUES('first_module','_65961crktpj_vtt30qeqv21','.root');
INSERT INTO t_names VALUES('for_each_up_to','_11xee72y1d3_t3cqzi5dq3k','.root');
INSERT INTO t_names VALUES('frames','_4cw8jv45vsk_4mh9ex64904','.predef');
INSERT INTO t_names VALUES('get_attribute','_5vi29c2i54k_i2ufkty9kmp','.root');
INSERT INTO t_names VALUES('gives','_70ty9z1tm4p_eccsxmyfe25','.predef');
INSERT INTO t_names VALUES('high','_6qcw93kypcv_0iiepqtk73j','.root');
INSERT INTO t_names VALUES('id','_7a9sxskjhcp_kpf30ka97ex','.predef');
INSERT INTO t_names VALUES('input','_356014y9ueu_xv6j0eskszw','.predef');
INSERT INTO t_names VALUES('integer','_9jeymqk2732_wiq5kyczi9c','.predef');
INSERT INTO t_names VALUES('item','_53748kde7s1_pkz810exr27','.predef');
INSERT INTO t_names VALUES('item_ref','_6hf2vzmrsee_t35suhjvtj4','.predef');
INSERT INTO t_names VALUES('ix','_9wwqwxqcm4p_y7di7fs8tsk','.root');
INSERT INTO t_names VALUES('json_array','_35vp60aw7em_d436vfie4ud','.predef');
INSERT INTO t_names VALUES('json_false','_4mha85xcfwi_9zqcvkiy3dk','.predef');
INSERT INTO t_names VALUES('json_handler','_0hpzi8m7wym_1y4ypmm9y47','.predef');
INSERT INTO t_names VALUES('json_object','_3xpyd539p4m_23h7wi59xi9','.predef');
INSERT INTO t_names VALUES('json_true','_2vmrrvq5kdk_9um63pstcu9','.predef');
INSERT INTO t_names VALUES('jsonrpc','_0h331ch957p_j6a8i7v4e6y','.predef');
INSERT INTO t_names VALUES('jtype','_7urjeiw3evy_m7k72uv6790','.predef');
INSERT INTO t_names VALUES('kind','_06yp8ueq6yf_5ts408yww29','.predef');
INSERT INTO t_names VALUES('len','_456hz6qd6x2_jyy24w6q84z','.root');
INSERT INTO t_names VALUES('locals','_3eu0rdq4upj_dp5ptr6hj04','.predef');
INSERT INTO t_names VALUES('low','_79vm7uxit6c_53qt1qi2wuj','.root');
INSERT INTO t_names VALUES('make_item','_590trid9ycw_f6kaajwca63','.root');
INSERT INTO t_names VALUES('method','_3hv5ymapjed_y8q6hsvhw8u','.predef');
INSERT INTO t_names VALUES('misc','_85rz4j0q982_67im8sstj9s','.root');
INSERT INTO t_names VALUES('module','_7sqk8vh89xr_6tj8dq7vqju','.predef');
INSERT INTO t_names VALUES('node','_4m7x6811f6j_t480zu575mz','.predef');
INSERT INTO t_names VALUES('noop','_240dwt57s08_a8uy366sev5','.root');
INSERT INTO t_names VALUES('notice','_7diyc1cwj8z_x630afccr8e','.predef');
INSERT INTO t_names VALUES('numbers','_3fw5acswe59_9016fqe4d41','.predef');
INSERT INTO t_names VALUES('origin','_5sw59dauckp_8eustjwf58u','.predef');
INSERT INTO t_names VALUES('params','_4215uc2u6qk_52kqyra86y5','.predef');
INSERT INTO t_names VALUES('parent','_36tp2s8s5s2_jzjm0cxdpjz','.predef');
INSERT INTO t_names VALUES('payload','_41v0erax6my_m6pytj0793u','.predef');
INSERT INTO t_names VALUES('primitive','_967fch1xu4h_i87qjq1zt1h','.predef');
INSERT INTO t_names VALUES('proc','_70aer7teeui_kvzkiqq2rd2','.root');
INSERT INTO t_names VALUES('procedures','_5yfdp53cpi1_0i5k33wms7c','.predef');
INSERT INTO t_names VALUES('put_attribute','_547q7emtfsk_ect0yratp6e','.root');
INSERT INTO t_names VALUES('rank','_7kkh6qiq1vc_e69zp2feuhe','.predef');
INSERT INTO t_names VALUES('res1','_3j3s2e0510a_096chqpijq7','.predef');
INSERT INTO t_names VALUES('res2','_8u5ar84utwm_99k5mq2d589','.predef');
INSERT INTO t_names VALUES('res3','_60ist2ad22c_cfpjp5ay6uj','.predef');
INSERT INTO t_names VALUES('result','_6djzuwz5pav_cri386ywjhj','.root');
INSERT INTO t_names VALUES('routine','_0acmecj244a_6krws4rx7v1','.predef');
INSERT INTO t_names VALUES('seq','_2mayc646pdu_w4d18fmx8u3','.root');
INSERT INTO t_names VALUES('sequence_length','_41xwu6cpvq9_ezp5wzq7t4x','.root');
INSERT INTO t_names VALUES('sequence_nth','_80wxf4c8q92_qq8k6xc0xxj','.root');
INSERT INTO t_names VALUES('set','_2v75mmyph64_4h4kys78740','.predef');
INSERT INTO t_names VALUES('size','_5s59qeamxta_70k0mt77r9i','.predef');
INSERT INTO t_names VALUES('sons','_4ezpkss1akd_94f4h25sqe4','.predef');
INSERT INTO t_names VALUES('space','_7rf7axuc9h4_2aw6utwmsas','.predef');
INSERT INTO t_names VALUES('start_assoc','_86ft82euar7_cm50jcthhwe','.root');
INSERT INTO t_names VALUES('state','_6f9870y6v8t_kp8fcmq2ezv','.predef');
INSERT INTO t_names VALUES('string','_8j516kuv89j_4hc4w6ykmr6','.predef');
INSERT INTO t_names VALUES('terminated','_3jpt8yuzuyw_ti1pyz3me1c','.predef');
INSERT INTO t_names VALUES('translate_module','_4yxdswc8qwf_vxzy95hd399','.predef');
INSERT INTO t_names VALUES('translate_procedure','_0afqepa7jkr_qky26hpv98d','.root');
INSERT INTO t_names VALUES('translation','_8y1sw8z084j_4ts0y0jydha','.root');
INSERT INTO t_names VALUES('tuple','_7vw56h18sw0_hv77m6q8uxu','.predef');
INSERT INTO t_names VALUES('update_display_value','_1f94j87qumw_mhzkriesx7c','.root');
INSERT INTO t_names VALUES('updated','_5x41iah0kis_x8rrv3ww44t','.predef');
INSERT INTO t_names VALUES('val','_7wk9y7e7r0z_575esi8ys5x','.predef');
INSERT INTO t_names VALUES('values','_91pketvc5pz_wq0v0wpauw8','.predef');
INSERT INTO t_names VALUES('var','_2d7i21ihwd8_xjcp4uhs11u','.root');
INSERT INTO t_names VALUES('verbatim','_53cuy70z4tf_86tzz364trd','.predef');
INSERT INTO t_names VALUES('void','_7jzvaihqxfw_0c2y7t976tu','.predef');
INSERT INTO t_names VALUES('web_handler','_7sav6zery1v_24sa6jwwu6c','.predef');
COMMIT;
-- state-monimelt end dump 
