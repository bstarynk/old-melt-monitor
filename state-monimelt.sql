-- state-monimelt dump 2014 Nov 26

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
CREATE TABLE t_items (itm_idstr VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE, itm_jdata TEXT NOT NULL, itm_kind VARCHAR(24) NOT NULL);
CREATE TABLE t_names (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, n_idstr VARCHAR(30) UNIQUE NOT NULL, n_spacename VARCHAR(20) NOT NULL);
CREATE TABLE t_modules (modname VARCHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE);
-- state-monimelt tables contents
INSERT INTO t_params VALUES('dump_format_version','MoniMelt2014B');
INSERT INTO t_params VALUES('dump_reason','command dump');
INSERT INTO t_modules VALUES('_65961crktpj_vtt30qeqv21');
INSERT INTO t_items VALUES('_02av6173qvf_pehzhe755j2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for calls at end of  blocks"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_02q6zk9f5st_im0z75re15f','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the void C type"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_02u53qxa7dm_sttmhffpchr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The agenda is central to Monimelt.\nIt is the queue of taskets to be executed by worker threads."}],
 "content": null, "kind": "queue", "payload": []}
','queue');
INSERT INTO t_items VALUES('_02yd241wh4z_tca7i6iamf3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "start block of show_html_for_item_proc"},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": []}}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_03f56tp7q6w_f1t3fiyivph','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_INDENT_LESS"]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": []}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective to indent less."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_06uk4pppvx9_huv0v11v18j','{"attr": [{"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype": "set", "set": ["_16cd0fvmdrh_r77ajpy26za"]}},
  {"attr": "_3wh3e88sk28_d27qi2737zi", "val": {"item_ref": "_8y756mef2ca_w8cj58726vj",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "AJAX for appl menu"}, {"attr": "_4p33dhxywm0_id6tti2kyw6", "val": {"jtype":
    "set", "set": ["_350hj5kfymd_145tfc1sevi"]}}, {"attr": "_7sav6zery1v_24sa6jwwu6c",
   "val": {"item_ref": "_0ce0mmy7myq_1t5iw2mfrvm", "jtype": "item_ref", "space": ".root"}},
  {"attr": "_7yxp9xhih4z_9uzrqhkamxa", "val": {"jtype": "set", "set": ["_7yyaydvyhpr_teuchcqzs7k",
     "_8t137w1z1s9_2tea9xp64s6", "_8y756mef2ca_w8cj58726vj", "_91471ta1047_pra9zfqc2y1",
     "_9u6a6xy2e1p_qeapfc73cm4"]}}], "content": null, "kind": "tfunrout",
 "payload": {"constants": [], "tasklet_function": "_06uk4pppvx9_huv0v11v18j"}}
','tfunrout');
INSERT INTO t_items VALUES('_06yp8ueq6yf_5ts408yww29','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the kind of the payload of an item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_07zti91e4kd_952zqsd03fz','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_3wh3e88sk28_d27qi2737zi",
   "val": {"item_ref": "_1kj1j3878fe_duw3ts10hev", "jtype": "item_ref", "space": ".root"}},
  {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "todo procedure on exit"}, {"attr":
   "_4v74chqs1eh_chqd9cqw85t", "val": {"jtype": "set", "set": ["_1kj1j3878fe_duw3ts10hev"]}}],
 "content": null, "kind": "procedure", "payload": []}
','procedure');
INSERT INTO t_items VALUES('_0afqepa7jkr_qky26hpv98d','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "translate a single procedure"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0ce0mmy7myq_1t5iw2mfrvm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "closure for ajax_appl"}],
 "content": null, "kind": "closure", "payload": {"closed_values": [], "closure_function":
  "_06uk4pppvx9_huv0v11v18j"}}
','closure');
INSERT INTO t_items VALUES('_0ee6afx5850_ji17eq0wmfa','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Attribute giving the C type"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0h331ch957p_j6a8i7v4e6y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC"}], "content":
 null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0hi99dr3qqs_yuuciyy7xfz','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "some value variable"}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0hpzi8m7wym_1y4ypmm9y47','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handler for JSONRPC requests"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0ju7898wddc_1296qwc3a30','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_7j7x11c25h3_wkchtuwpusx",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" mom_double_val(",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get the value in a double or else NaN."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0pqeesw57dy_1q3yyp3a7um','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (intptr_t) mom_jsonob_size (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get the size of a JSONobject value or else 0."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0te6f7f9pz7_m91yy9iv5pd','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for output related variadic primitives."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0wm3xd5kpcc_y1e349eizfj','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_SMALL_SPACE,", {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for small spacing."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_0x2k07ik4tm_ed7vqphf5ak','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives verbatim code"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0yyp8vmw4si_wf49m4d4zwq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error code in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_0z0rsvwfkcj_dcpkx68i074','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_16cd0fvmdrh_r77ajpy26za", "_1a2aavj5vir_2hz681zdfqd"]}},
  {"attr": "_3wh3e88sk28_d27qi2737zi", "val": {"item_ref": "_02yd241wh4z_tca7i6iamf3",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "procedure to show on webx the HTML code for some item"}, {"attr": "_4v74chqs1eh_chqd9cqw85t",
   "val": {"jtype": "set", "set": ["_02yd241wh4z_tca7i6iamf3"]}}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_10ji3aajfx3_983iz4013ec','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_80e7dsukuq3_6p7jffmz1yi",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" mom_string_cstr (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get the string literal inside a string value"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_113idxzj0u9_h5c6i4t8hdy','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (momval_t) mom_make_string (",
     {"item_ref": "_389t77v85ej_fwpy6exy62x", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to make a string from a C string literal"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_11xee72y1d3_t3cqzi5dq3k','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "iterate on increasing integers"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_133zjf1f9zp_jq8kti38sd7','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for predefined items."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_15f5zxxsky9_3vv7edvmk75','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_5ryj031qxp3_8rx001d7y2x", "_96qx16z900r_dw5ppp22a0s"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": [" (intptr_t) mom_json_cmp(", {"item_ref": "_5ryj031qxp3_8rx001d7y2x",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_96qx16z900r_dw5ppp22a0s",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to compare in a JSON friendy way"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_16cd0fvmdrh_r77ajpy26za','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "variable with the web exchange"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_16wqrr2mwae_fmcdtax6p6s','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_5ryj031qxp3_8rx001d7y2x", "_96qx16z900r_dw5ppp22a0s"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["(intptr_t) mom_value_cmp (", {"item_ref": "_5ryj031qxp3_8rx001d7y2x",
      "jtype": "item_ref", "space": ".root"}, ", ", {"item_ref": "_96qx16z900r_dw5ppp22a0s",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to compare values"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_17spwr8dkzv_tsf2s8diazu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the double floating-point numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_18zm20vu86d_jey4zt03k8z','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (intptr_t) mom_is_json_array (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to test if value is a JSONarray"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_19ufza9zf05_muxxhkmm7ww','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives expansion for output related connectives"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_1a2aavj5vir_2hz681zdfqd','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "value variable for some item."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_1d21u7ivquk_qm06hv72eu8','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_INDENT_MORE"]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": []}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective to indent more"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_1dfsr53udxw_k9h81rfpdx8','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_STRING_VALUE, ", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for string values."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_1eqee5utt20_pyw3mat0zw4','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_HEX_INTPTR_T,", {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for integers, in hexadecimal."}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_1f94j87qumw_mhzkriesx7c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to update the value in displays after edition."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_1iyd2es3u59_x6uq7vhecjj','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["mom_full_dump(\"todo dump-at-exit\", ",
     {"item_ref": "_389t77v85ej_fwpy6exy62x", "jtype": "item_ref", "space": ".root"},
     ", NULL);", " mom_stop_event_loop();"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive inside todo at exit"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_1kj1j3878fe_duw3ts10hev','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "start block of todo proc on exit"},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": [{"jtype": "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype":
"node", "node": "_1iyd2es3u59_x6uq7vhecjj", "sons": [{"item_ref": "_389t77v85ej_fwpy6exy62x",
  "jtype": "item_ref", "space": ".root"}]}]}]}}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_1r880c1yk3z_i5e8mprcj90','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for blocks in routines, etc..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_1tzf3q2dix5_jqxphp9ivcw','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "hold the number of dumped items, e.g. in dump_data"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_1uarhtr96qc_28hwh1ueaq4','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(intptr_t) mom_json_array_size (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get size of JSONarray or else 0"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_1x4wmv7yiym_w3zfpdv8q0m','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_C_STRING,", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for C-encoded strings."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_1yz5fpjm6yt_319wacq9346','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_7j7x11c25h3_wkchtuwpusx",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["mom_clock_time(",
     {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to query a time, as a double ..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_20ds5w0c9z1_tvf5h12wrqp','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz", "_389t77v85ej_fwpy6exy62x"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["(intptr_t) mom_string_same((", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, "), (", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}, "))"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive, to compare a value against a literal string."}], "content":
 null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_21a2k0s62d3_t9r0i5860kd','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(intptr_t) mom_integer_val_or0 (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get integer value or else 0"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_232t5qs2v8e_zu2wy53cqe3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for locked blocks"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_240dwt57s08_a8uy366sev5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "useless noop routine"}],
 "content": {"jtype": "node", "node": "_240dwt57s08_a8uy366sev5", "sons": ["{spare1 noop}",
   "{spare2 noop}", null]}, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_24w2ce2eq1z_pddi9j2czci','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "related to dump of full state"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_24yt56xf3d5_4w80i326kjz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JIT code of JIT-ed routines"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2apx2jm2jjk_m470v1fcei0','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz", "_389t77v85ej_fwpy6exy62x"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": [" mom_jsonob_getstr (", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to retrieve in a JSONobject value a literal string name."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2d7i21ihwd8_xjcp4uhs11u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some variable"}], "content":
 null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2kxisdsque9_u9awek5wup1','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for jumps at end of  blocks"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2ky10qvckv2_kqa0pr8z29z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "starting block for test_fun1."},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": []}}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2mayc646pdu_w4d18fmx8u3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some sequence"}], "content":
 null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2qq1dh2ucpr_qtv6staqhti','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "formal for todo procedure"}], "content": null, "kind": null, "payload":
 null}
','');
INSERT INTO t_items VALUES('_2rpd6wy50xt_etjfuj6s8jr','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_ITEM, mom_value_to_item(", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for items."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_2u8svx94yq4_34icz9j1fyx','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for constants in routines..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2ummst105ck_xracfy8v87y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "formal arguments..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2v75mmyph64_4h4kys78740','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_set"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate sets of item, or give their array of elements"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2vmrrvq5kdk_9um63pstcu9','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The true of JSON.\nWe cannot use true because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2vy0ah3jrd1_mm66ja7rfj7','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "integer variable."}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_2x2zpyhfqum_0utui69rzea','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "arguments of procedure or primitive"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3230kudxwis_qs5ss05i85w','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON to give the function in a closure."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_32r2krfzqaa_ep4usqkhayd','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" mom_make_integer (",
     {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to make a boxed integer"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_350hj5kfymd_145tfc1sevi','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "value variable for what"}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_35vp60aw7em_d436vfie4ud','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_jsonarray"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dump, jtype of JSON array values"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_36esaxiqmd5_ciet8ws1f5d','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_item"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for the item type"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_37kswkaa035_qjpxwd7e67f','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for making entries in json_object"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_37x98fyestf_ttup2cu68r6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the expansion of a primitive"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_389t77v85ej_fwpy6exy62x','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_80e7dsukuq3_6p7jffmz1yi",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "variable, for literal C strings"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_38w4qwrmd6z_74x5z80v5k6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "inside switches: case <constant-expr> <block>"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_39hpqv0jqj6_9sa2v0vhfm6','{"attr": [{"attr": "_232t5qs2v8e_zu2wy53cqe3", "val": {"item_ref": "_5tihf27p4rj_t80tzx4fxrf",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "start block of todo on dump"}, {"attr": "_94tq2iauet8_jujpjhjrzsm",
   "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq", "sons": [{"jtype":
      "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node",
"node": "_87r5zd69i6m_zr0hupaer90", "sons": [{"item_ref": "_389t77v85ej_fwpy6exy62x",
  "jtype": "item_ref", "space": ".root"}]}]}]}}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_3dqr46p2xf4_29kf5vdtw4z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Call with results in res, in given state, the closure clos with given arguments"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3ff1zzfsxzc_xk423qutvqr','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_NEWLINE"]}}, {"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype":
    "tuple", "tuple": []}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "output connective for newlines."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3fw5acswe59_9016fqe4d41','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the integer numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3h56h532u19_1ssc4pje2x6','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["mom_type(",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get the type of a value"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3hv5ymapjed_y8q6hsvhw8u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3i8mqyfreeh_2w73i1khumx','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_80e7dsukuq3_6p7jffmz1yi",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "C string giving a name."}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3j3s2e0510a_096chqpijq7','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "first result in tasklet"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3jkcrmcyzep_4ddf6qriy44','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz", "_389t77v85ej_fwpy6exy62x"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["(intptr_t) mom_json_cmp (", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to compare a JSON value -boxed string or item- to a literal string"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3jpt8yuzuyw_ti1pyz3me1c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for terminated processes"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3scu6d0kfes_vc7f1ryt673','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_FLUSH"]}}, {"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype":
    "tuple", "tuple": []}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "output connective for flushing"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3uwzqwvj6zj_s63am4qivpt','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the value C type"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3v4d7uzex6f_euek4pztiuh','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3wh3e88sk28_d27qi2737zi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for starting block in routines, etc..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3x55htdct2h_upumrci06w1','{"attr": [{"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype": "tuple", "tuple":
    ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype":
    "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(intptr_t) mom_is_integer (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to test if value v is an int"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_3xpyd539p4m_23h7wi59xi9','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_jsonobject"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dump, jtype of JSON object values"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_41u1utcxyek_22cftxt3xxm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives a human-readable comment"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_41v0erax6my_m6pytj0793u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the payload of items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_41xwu6cpvq9_ezp5wzq7t4x','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the length of some sequence"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4215uc2u6qk_52kqyra86y5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_456hz6qd6x2_jyy24w6q84z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "common length integer"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_47fatww79x6_vh8ap22c0ch','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP HEAD method"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4cw8jv45vsk_4mh9ex64904','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "frames in tasklet"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4ew7uvid6ep_wzq6f8hruz9','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0te6f7f9pz7_m91yy9iv5pd",
   "val": {"item_ref": "_5ic5uk22icm_7ws16feu699", "jtype": "item_ref", "space": ".root"}},
  {"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype": "tuple", "tuple": ["_16cd0fvmdrh_r77ajpy26za"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOM_WEBX_OUT (mom_value_to_item(", {"item_ref": "_16cd0fvmdrh_r77ajpy26za",
      "jtype": "item_ref", "space": ".root"}, "),", {"item_ref": "_5ic5uk22icm_7ws16feu699",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output primitive to fill a web exchange."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_4ezpkss1akd_94f4h25sqe4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the sons of nodes"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4ie5pvk8m1x_rzsut0mdza0','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "HTTP_OK"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "literal for HTTP protocol return code, success..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4jp2meuzru2_a58afyxwxa2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error message in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4m7x6811f6j_t480zu575mz','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_node"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate nodes, or give their connective item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4mha85xcfwi_9zqcvkiy3dk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The false of JSON.\nWe cannot use false because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4p33dhxywm0_id6tti2kyw6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "locals of function"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4qcw2mwjswm_j9q0k9d04hm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "error case, on for JSONRPC"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4rrcat1av7m_55z67m8hurq','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_7j7x11c25h3_wkchtuwpusx",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" mom_double_val_or0 (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get a double value or else 0.0"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4v74chqs1eh_chqd9cqw85t','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for procedure related things"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4v7s8xjeh8e_q8h93d2f43y','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (intptr_t) mom_is_jsonob(",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to test if value is a JSONobject"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4wx5f4704sp_v9kfazsqe9h','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "variable with the web session"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_4x8e2mwmacp_ekxdw3vqqpd','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_JS_HTML, ", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for HTML + Javascript encoded strings."}], "content":
 null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_4xicv8w07x7_3hzzmpw8iwt','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_DOUBLE_TIME,", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}, ", ", {"item_ref": "_837uvkhhyar_tdhv0jy5r72",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x", "_837uvkhhyar_tdhv0jy5r72"]}},
  {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "output connective for a double representing a time, cstr is an strftime_centi format & dbl_x is the time."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_50623j9vemk_1hp2q2czrhi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "hold the cpu time, e.g. in dump_data "}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_51u3st4u9mc_zdvms6jti0a','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_int"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The C type for word integers"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_547q7emtfsk_ect0yratp6e','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_1a2aavj5vir_2hz681zdfqd", "_8yepyw577hd_yr28eiysfss",
     "_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype":
    "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" mom_item_put_attribute(",
     {"item_ref": "_1a2aavj5vir_2hz681zdfqd", "jtype": "item_ref", "space": ".root"},
     ".ptr, ", {"item_ref": "_8yepyw577hd_yr28eiysfss", "jtype": "item_ref",
      "space": ".root"}, ".ptr, ", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "to put in a given item, some attribute with some value"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_590trid9ycw_f6kaajwca63','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to make an item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_59wxs8qi3jd_vctt2cyuva9','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (intptr_t) mom_string_slen (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to retrieve the length of a string value or else 0"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5a9mai6ar7t_87kveq8ic29','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_7j7x11c25h3_wkchtuwpusx",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": []}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["mom_elapsed_real_time()"]}},
  {"attr": "_41u1utcxyek_22cftxt3xxm", "val": ",primitive to get the elapsed real time"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5ar352xfzeu_2hrq5jjs95p','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(intptr_t) mom_value_hash(",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to compute the hash code of a value"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5c5jh9185sv_qru5amf9v18','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "put an item inside an assoc"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5c789try94y_ssy6a22fpep','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for side-effecting instructions in blocks, etc..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5ic5uk22icm_7ws16feu699','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "pseudo-formal for output arguments"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5k1ph0qwrky_v6a6ap5uxzp','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_DOUBLE_F,", {"item_ref": "_837uvkhhyar_tdhv0jy5r72",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_837uvkhhyar_tdhv0jy5r72"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for doubles in %f fixed-point format"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5m370797c14_32w6fyj9749','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_DOUBLE_G,", {"item_ref": "_837uvkhhyar_tdhv0jy5r72",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_837uvkhhyar_tdhv0jy5r72"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for doubles in %g scientific format"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5ryj031qxp3_8rx001d7y2x','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "some value variable"}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5s26uzwhveh_fv3twe3s2je','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(momval_t) mom_value_to_item (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to converting a value to an item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5tihf27p4rj_t80tzx4fxrf','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "item to hold transient dump data"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5vi29c2i54k_i2ufkty9kmp','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_1a2aavj5vir_2hz681zdfqd", "_8yepyw577hd_yr28eiysfss"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": [" mom_item_get_attribute(", {"item_ref": "_1a2aavj5vir_2hz681zdfqd",
      "jtype": "item_ref", "space": ".root"}, ".ptr, ", {"item_ref": "_8yepyw577hd_yr28eiysfss",
      "jtype": "item_ref", "space": ".root"}, ".ptr)"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to get the attribute"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_5w0cahd5yri_e7f3m67xf4f','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["mom_value_json(",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to dynamically cast to a JSON value"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5w5idj1j9ah_u00krzevvj7','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_LITERAL, ", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for literal or computed strings."}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5wmusj136kq_u5qpehp89aq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP POST method"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5x0p1k3qswi_wwe62qt0zea','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_5ryj031qxp3_8rx001d7y2x", "_96qx16z900r_dw5ppp22a0s"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": [" mom_jsonob_get(", {"item_ref": "_5ryj031qxp3_8rx001d7y2x",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_96qx16z900r_dw5ppp22a0s",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to retrieve in a JSONobject value a name string or item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5xa08a3ittw_imt86y9q33c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "starting block for test_proc1."},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": []}}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5xw5qm751tv_jvm099ita0w','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "hold the named items, e.g. in dump_data"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_5yms6ak2u0p_ri0wwutvhc1','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_SPACE,", {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7", "jtype":
      "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for spacing."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_60ist2ad22c_cfpjp5ay6uj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "third result in tasklet"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_62ryyaxj112_wyw4upc7deh','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_16cd0fvmdrh_r77ajpy26za"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [{"item_ref":
      "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"},
     ".pitem)", " (momval_t) mom_webx_session ("]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to get the session from a web exchange"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6443sk5q0zt_8xdi02c6tzu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for ending some blocks"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_65961crktpj_vtt30qeqv21','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "first module, with core AJAX functions etc..."},
  {"attr": "_9dcxaqk8tqe_fam9mcxme9w", "val": {"jtype": "set", "set": ["_06uk4pppvx9_huv0v11v18j",
     "_07zti91e4kd_952zqsd03fz", "_0z0rsvwfkcj_dcpkx68i074", "_7x6as13park_w64mrkx2xtm"]}}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_682ush7cppa_s7vzfd2rsxp','{"attr": [{"attr": "_2u8svx94yq4_34icz9j1fyx", "val": {"jtype": "set", "set": ["_0yyp8vmw4si_wf49m4d4zwq"]}},
  {"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype": "tuple", "tuple": []}},
  {"attr": "_3wh3e88sk28_d27qi2737zi", "val": {"item_ref": "_5xa08a3ittw_imt86y9q33c",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "procedure 1 inside our test_module."}, {"attr": "_4v74chqs1eh_chqd9cqw85t",
   "val": {"jtype": "set", "set": ["_5xa08a3ittw_imt86y9q33c"]}}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6djzuwz5pav_cri386ywjhj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error message in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6ek4zitda2z_dfd1sddm812','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz", "_2vy0ah3jrd1_mm66ja7rfj7"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": [" (intptr_t) mom_integer_val_def(", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to get the integer from an integer value"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6f4k9pqzryk_w25f8vxuyyc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "something for counting"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6f9870y6v8t_kp8fcmq2ezv','{"attr": [{"attr": "_0h331ch957p_j6a8i7v4e6y", "val": {"item_ref": "_6p6v25323aq_97d9ude6j12",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_0hpzi8m7wym_1y4ypmm9y47",
   "val": {"item_ref": "_6p6v25323aq_97d9ude6j12", "jtype": "item_ref", "space": ".root"}},
  {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the state of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6hf2vzmrsee_t35suhjvtj4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item reference, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6mutx3sq2eu_23r4mvpac6m','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(intptr_t) mom_is_double (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to test if value is double"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6p6v25323aq_97d9ude6j12','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives thru JSONRPC the state of monimelt"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6qcw93kypcv_0iiepqtk73j','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "high bound"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6rs26jmh9ya_jv0aiqf4kvx','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the dump state hold transient data about last dump."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6u6cp2a2tsz_ses4qchc3y3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "hold the real elapsed time, e.g. in dump_data"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6vrzjdj7ij8_dupds6c9895','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "hold the names of items, e.g. in dump_data"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6w3dvx83dfw_xzc6aif6isv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the attribute[s], at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_6xjwjjh686j_hczfxhwjp1f','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_GPLV3P_NOTICE,", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective to emit a GPL version 3+ copyright notice for the given file name."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_70aer7teeui_kvzkiqq2rd2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "current procedure, etc..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_70mt4fvrva2_pk76eevwada','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "get value associated in some assoc item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_71a9mq07fvv_448p3r7vikd','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_JSON_VALUE,", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for JSON values"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_73im2zryfij_a7zmkketcfc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to edit a value during edition in ajax_objects"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_79vm7uxit6c_53qt1qi2wuj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "low bound"}], "content": null,
 "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7a9sxskjhcp_kpf30ka97ex','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7diyc1cwj8z_x630afccr8e','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Group together all noticed values in dump outcome."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7dritvdcmhf_yw3u1jquevr','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_7j7x11c25h3_wkchtuwpusx",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz", "_837uvkhhyar_tdhv0jy5r72"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": [" mom_double_val_def (", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_837uvkhhyar_tdhv0jy5r72",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "pritmive to get the double in a double value, with a default double"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7j5uq75spm6_wuhuc4pe5th','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_16cd0fvmdrh_r77ajpy26za", "_389t77v85ej_fwpy6exy62x",
     "_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype":
    "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["mom_webx_reply (mom_value_to_item (",
     {"item_ref": "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"},
     "), ", {"item_ref": "_389t77v85ej_fwpy6exy62x", "jtype": "item_ref",
      "space": ".root"}, ",", {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7", "jtype": "item_ref",
      "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to reply to a web exchange."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7j7x11c25h3_wkchtuwpusx','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the double C type"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7mj6c8az5d3_vzmwd2uuiu7','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_null"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes the null type of nil."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7pyjxst21ce_vhc0tk0em0u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to display a value during edition in ajax_objects"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7qk90k9vx0u_31ivff77td7','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "keep association between edited items and their editor"}],
 "content": null, "kind": "assoc", "payload": []}
','assoc');
INSERT INTO t_items VALUES('_7rf7axuc9h4_2aw6utwmsas','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the space of items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7sav6zery1v_24sa6jwwu6c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "attribute giving the web handler inside items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7sqk8vh89xr_6tj8dq7vqju','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "module to be compiled..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7urjeiw3evy_m7k72uv6790','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the type of a value"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7vw56h18sw0_hv77m6q8uxu','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_tuple"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for tuples of items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7wk9y7e7r0z_575esi8ys5x','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for values in attribute lists of items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7x6as13park_w64mrkx2xtm','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2u8svx94yq4_34icz9j1fyx",
   "val": {"jtype": "set", "set": ["_133zjf1f9zp_jq8kti38sd7", "_1tzf3q2dix5_jqxphp9ivcw",
     "_50623j9vemk_1hp2q2czrhi", "_5tihf27p4rj_t80tzx4fxrf", "_5xw5qm751tv_jvm099ita0w",
     "_6rs26jmh9ya_jv0aiqf4kvx", "_6u6cp2a2tsz_ses4qchc3y3", "_6vrzjdj7ij8_dupds6c9895",
     "_7diyc1cwj8z_x630afccr8e", "_7sqk8vh89xr_6tj8dq7vqju"]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_3wh3e88sk28_d27qi2737zi",
   "val": {"item_ref": "_39hpqv0jqj6_9sa2v0vhfm6", "jtype": "item_ref", "space": ".root"}},
  {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "todo procedure on dump"}, {"attr":
   "_4v74chqs1eh_chqd9cqw85t", "val": {"jtype": "set", "set": ["_39hpqv0jqj6_9sa2v0vhfm6"]}}],
 "content": null, "kind": "procedure", "payload": [null, null, null, null,
  null, null, null, null, null, null]}
','procedure');
INSERT INTO t_items VALUES('_7x7vwep1fw4_i158put1x07','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2qq1dh2ucpr_qtv6staqhti", "_389t77v85ej_fwpy6exy62x"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["mom_stop_work_with_todo (", {"item_ref": "_2qq1dh2ucpr_qtv6staqhti",
      "jtype": "item_ref", "space": ".root"}, ",", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to stop agenda."}], "content": null, "kind": null, "payload":
 null}
','');
INSERT INTO t_items VALUES('_7yxp9xhih4z_9uzrqhkamxa','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for routines for tasklet"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_7yyaydvyhpr_teuchcqzs7k','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "block to handle ajax_appl when whatv=dump"},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": [{"jtype": "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype":
"node", "node": "_7x7vwep1fw4_i158put1x07", "sons": [{"item_ref": "_7x6as13park_w64mrkx2xtm",
  "jtype": "item_ref", "space": ".root"}, "."]}]}, {"jtype": "node", "node": "_2kxisdsque9_u9awek5wup1",
      "sons": [{"item_ref": "_91471ta1047_pra9zfqc2y1", "jtype": "item_ref",
"space": ".root"}]}]}}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_80e7dsukuq3_6p7jffmz1yi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The C type for constant literal C strings"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_80wxf4c8q92_qq8k6xc0xxj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives in sequence seq the element of given rank"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_822d1dkak1h_cw0598pe64d','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_HTML, ", {"item_ref": "_389t77v85ej_fwpy6exy62x", "jtype":
      "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for HTML string"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_837uvkhhyar_tdhv0jy5r72','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_7j7x11c25h3_wkchtuwpusx",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "double variable"}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_84ih8qi7vw2_k31sih7ewjj','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["(intptr_t) mom_is_jsonable (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to test if a value is JSONable"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_86ft82euar7_cm50jcthhwe','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "start an associative item"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_87r5zd69i6m_zr0hupaer90','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": ["/* primitive _87r5zd... in start-block of todo-on-dump */\n",
     " struct mom_dumpoutcome_st doutc;\n memset(&doutc, 0, sizeof(doutc));",
     " mom_full_dump(\"todo-dump-with-outcome\", ", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}, ", &doutc);\n MOM_INFORMPRINTF(\" dumped with outcome %d items into %s\", doutc.odmp_nbdumpeditems, ",
     {"item_ref": "_389t77v85ej_fwpy6exy62x", "jtype": "item_ref", "space": ".root"},
     ");\n", " mom_item_put_attribute (", {"item_ref": "_6rs26jmh9ya_jv0aiqf4kvx",
      "jtype": "item_ref", "space": ".root"}, ".pitem, ", {"item_ref": "_6u6cp2a2tsz_ses4qchc3y3",
      "jtype": "item_ref", "space": ".root"}, ".pitem, mom_let_transient(mom_make_double(doutc.odmp_elapsedtime)));\n",
     " mom_item_put_attribute (", {"item_ref": "_6rs26jmh9ya_jv0aiqf4kvx",
      "jtype": "item_ref", "space": ".root"}, ".pitem, ", {"item_ref": "_50623j9vemk_1hp2q2czrhi",
      "jtype": "item_ref", "space": ".root"}, ".pitem, mom_let_transient(mom_make_double(doutc.odmp_cputime)));\n",
     " mom_item_put_attribute (", {"item_ref": "_6rs26jmh9ya_jv0aiqf4kvx",
      "jtype": "item_ref", "space": ".root"}, ".pitem, ", {"item_ref": "_1tzf3q2dix5_jqxphp9ivcw",
      "jtype": "item_ref", "space": ".root"}, ".pitem, mom_let_transient(mom_make_integer(doutc.odmp_nbdumpeditems)));\n",
     " mom_item_put_attribute (", {"item_ref": "_6rs26jmh9ya_jv0aiqf4kvx",
      "jtype": "item_ref", "space": ".root"}, ".pitem, ", {"item_ref": "_7diyc1cwj8z_x630afccr8e",
      "jtype": "item_ref", "space": ".predef"}, ".pitem, mom_let_transient(doutc.odmp_nodenotice));\n",
     " mom_item_put_attribute (", {"item_ref": "_6rs26jmh9ya_jv0aiqf4kvx",
      "jtype": "item_ref", "space": ".root"}, ".pitem, ", {"item_ref": "_133zjf1f9zp_jq8kti38sd7",
      "jtype": "item_ref", "space": ".root"}, ".pitem, mom_let_transient(doutc.odmp_setpredef));\n",
     " mom_item_put_attribute (", {"item_ref": "_6rs26jmh9ya_jv0aiqf4kvx",
      "jtype": "item_ref", "space": ".root"}, ".pitem, ", {"item_ref": "_7sqk8vh89xr_6tj8dq7vqju",
      "jtype": "item_ref", "space": ".predef"}, ".pitem, mom_let_transient(doutc.odmp_nodemodules));\n",
     " mom_continue_working();\n"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive in start block of todo on dump"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_89uzitiq84u_50h4cqpkay9','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_DEC_INTPTR_T,", {"item_ref": "_2vy0ah3jrd1_mm66ja7rfj7",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_2vy0ah3jrd1_mm66ja7rfj7"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for integers."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_8ejwdt1a5yx_2meizztvte0','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for conditionals, etc.."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8fk0de81s9r_5d4v4x7qmxr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for dispatching statements on items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8j516kuv89j_4hc4w6ykmr6','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "momty_string"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for long chunked strings"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8qmqy249w63_fxdr6rdz48m','{"attr": [{"attr": "_2u8svx94yq4_34icz9j1fyx", "val": {"jtype": "set", "set": ["_41u1utcxyek_22cftxt3xxm",
     "_7wk9y7e7r0z_575esi8ys5x"]}}, {"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype":
    "tuple", "tuple": []}}, {"attr": "_3wh3e88sk28_d27qi2737zi", "val": {"item_ref":
    "_2ky10qvckv2_kqa0pr8z29z", "jtype": "item_ref", "space": ".root"}}, {"attr":
   "_41u1utcxyek_22cftxt3xxm", "val": "function 1 inside our test_module."},
  {"attr": "_7yxp9xhih4z_9uzrqhkamxa", "val": {"jtype": "set", "set": ["_2ky10qvckv2_kqa0pr8z29z"]}}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8qpa7j0chkh_k630ujw6jiw','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for switch statements on integers"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8qqv70zdmmk_w4qs625q0s7','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_SLASHCOMMENT_STRING,", {"item_ref": "_389t77v85ej_fwpy6exy62x",
      "jtype": "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_389t77v85ej_fwpy6exy62x"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for C-encoded comment strings starting with //!"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8s357rq2dzk_k8ze95tikjm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item content, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8t137w1z1s9_2tea9xp64s6','{"attr": [{"attr": "_232t5qs2v8e_zu2wy53cqe3", "val": {"item_ref": "_16cd0fvmdrh_r77ajpy26za",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "block to handle appl with what=exit"}, {"attr": "_94tq2iauet8_jujpjhjrzsm",
   "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq", "sons": [{"jtype":
      "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node",
"node": "_7x7vwep1fw4_i158put1x07", "sons": [{"item_ref": "_07zti91e4kd_952zqsd03fz",
  "jtype": "item_ref", "space": ".root"}, "."]}]}, {"jtype": "node", "node": "_5c789try94y_ssy6a22fpep",
      "sons": [{"jtype": "node", "node": "_4ew7uvid6ep_wzq6f8hruz9", "sons": [{"item_ref":
  "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"}, "<em>Monimelt</em> <b>save then exit</b> at <i>",
 {"jtype": "node", "node": "_4xicv8w07x7_3hzzmpw8iwt", "sons": ["%c", {"jtype": "node",
    "node": "_1yz5fpjm6yt_319wacq9346", "sons": [{"item_ref": "_9jdufs9sew7_u6x3k3wfseq",
      "jtype": "item_ref", "space": ".root"}]}]}, "</i>"]}]}, {"jtype": "node",
      "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node", "node": "_7j5uq75spm6_wuhuc4pe5th",
"sons": [{"item_ref": "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"},
 "text/html", {"item_ref": "_4ie5pvk8m1x_rzsut0mdza0", "jtype": "item_ref",
  "space": ".root"}]}]}]}}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8u5ar84utwm_99k5mq2d589','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "second result in tasklet"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8vzddhpmhp8_z0005cqyqzf','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for assignment <var> := <expr>"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8x6fxcm4z2k_vdaqicfi4z0','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "used as node of primitive expansions, etc..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8y1sw8z084j_4ts0y0jydha','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "value containing the translation"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8y756mef2ca_w8cj58726vj','{"attr": [{"attr": "_232t5qs2v8e_zu2wy53cqe3", "val": {"item_ref": "_16cd0fvmdrh_r77ajpy26za",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "starting block for ajax_appl."}, {"attr": "_94tq2iauet8_jujpjhjrzsm",
   "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq", "sons": [{"jtype":
      "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node",
"node": "_9r25vmrrk8a_vus02yyfp8w", "sons": ["ajax_appl webx:", {"item_ref": "_16cd0fvmdrh_r77ajpy26za",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_8vzddhpmhp8_z0005cqyqzf",
      "sons": [{"item_ref": "_350hj5kfymd_145tfc1sevi", "jtype": "item_ref",
"space": ".root"}, {"jtype": "node", "node": "_8zwifiifaf0_pyayp6hpha4", "sons": [{"item_ref":
  "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"}, "what_mom"]}]},
     {"jtype": "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node",
"node": "_9r25vmrrk8a_vus02yyfp8w", "sons": ["ajax_appl whatv:", {"item_ref": "_350hj5kfymd_145tfc1sevi",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_8ejwdt1a5yx_2meizztvte0",
      "sons": [{"jtype": "node", "node": "_20ds5w0c9z1_tvf5h12wrqp", "sons": [{"item_ref":
  "_350hj5kfymd_145tfc1sevi", "jtype": "item_ref", "space": ".root"}, "exit"]},
       {"item_ref": "_8t137w1z1s9_2tea9xp64s6", "jtype": "item_ref", "space": ".root"}]},
     {"jtype": "node", "node": "_8ejwdt1a5yx_2meizztvte0", "sons": [{"jtype": "node",
"node": "_20ds5w0c9z1_tvf5h12wrqp", "sons": [{"item_ref": "_350hj5kfymd_145tfc1sevi",
  "jtype": "item_ref", "space": ".root"}, "quit"]}, {"item_ref": "_9u6a6xy2e1p_qeapfc73cm4",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_8ejwdt1a5yx_2meizztvte0",
      "sons": [{"jtype": "node", "node": "_20ds5w0c9z1_tvf5h12wrqp", "sons": [{"item_ref":
  "_350hj5kfymd_145tfc1sevi", "jtype": "item_ref", "space": ".root"}, "dump"]},
       {"item_ref": "_7yyaydvyhpr_teuchcqzs7k", "jtype": "item_ref", "space": ".root"}]}]}}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_8yepyw577hd_yr28eiysfss','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "value variable for some attribute item"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_8zwifiifaf0_pyayp6hpha4','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_16cd0fvmdrh_r77ajpy26za", "_3i8mqyfreeh_2w73i1khumx"]}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["mom_webx_post_arg (mom_value_to_item (", {"item_ref": "_16cd0fvmdrh_r77ajpy26za",
      "jtype": "item_ref", "space": ".root"}, "), ", {"item_ref": "_3i8mqyfreeh_2w73i1khumx",
      "jtype": "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "primitive to get a POST argument."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_91471ta1047_pra9zfqc2y1','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "block for ajax_appl continue after dump"},
  {"attr": "_94tq2iauet8_jujpjhjrzsm", "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq",
    "sons": [{"jtype": "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype":
"node", "node": "_4ew7uvid6ep_wzq6f8hruz9", "sons": [{"item_ref": "_16cd0fvmdrh_r77ajpy26za",
  "jtype": "item_ref", "space": ".root"}, "<em>Monimelt</em> <b>dump then continue</b> at<i>",
 {"jtype": "node", "node": "_4xicv8w07x7_3hzzmpw8iwt", "sons": ["%c", {"jtype": "node",
    "node": "_1yz5fpjm6yt_319wacq9346", "sons": [{"item_ref": "_9jdufs9sew7_u6x3k3wfseq",
      "jtype": "item_ref", "space": ".root"}]}]}, "</i>"]}]}, {"jtype": "node",
      "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node", "node": "_7j5uq75spm6_wuhuc4pe5th",
"sons": [{"item_ref": "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"},
 "text/html", {"item_ref": "_4ie5pvk8m1x_rzsut0mdza0", "jtype": "item_ref",
  "space": ".root"}]}]}]}}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_91pketvc5pz_wq0v0wpauw8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the values of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_94tq2iauet8_jujpjhjrzsm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for block items"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_96qx16z900r_dw5ppp22a0s','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_3uwzqwvj6zj_s63am4qivpt",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "some value variable"}], "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_97zkxf62r11_6eedwwv3eu8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure payload"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_98z00ctcfaa_v3vsrczdt0e','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (intptr_t) mom_integer_val_orm1(",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to get integer value or else -1"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9dcxaqk8tqe_fam9mcxme9w','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the routines in a module"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9dsak0qcy0v_1c5z9th7x3i','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP GET method"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9hjrq1h1s0p_h1qpmj1ucuy','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_VALUE,", {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype":
      "item_ref", "space": ".root"}]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for Monimelt values."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_9j2kuwuk892_ajmt7d309a5','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": ["_0hi99dr3qqs_yuuciyy7xfz"]}}, {"attr": "_37x98fyestf_ttup2cu68r6",
   "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0", "sons": [" (intptr_t) mom_is_string (",
     {"item_ref": "_0hi99dr3qqs_yuuciyy7xfz", "jtype": "item_ref", "space": ".root"},
     ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to test if value is a string"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9jdufs9sew7_u6x3k3wfseq','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "CLOCK_REALTIME"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "literal for identifying the literal clock"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9ju0dqm82h4_hqi2kjqxa83','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_51u3st4u9mc_zdvms6jti0a",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0x2k07ik4tm_ed7vqphf5ak",
   "val": "HTTP_NOT_FOUND"}, {"attr": "_41u1utcxyek_22cftxt3xxm", "val": "literal for HTTP protocol return code, not found..."}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9mxi9e605ay_ihpjyrwq250','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure values"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9r25vmrrk8a_vus02yyfp8w','{"attr": [{"attr": "_0ee6afx5850_ji17eq0wmfa", "val": {"item_ref": "_02q6zk9f5st_im0z75re15f",
    "jtype": "item_ref", "space": ".predef"}}, {"attr": "_0te6f7f9pz7_m91yy9iv5pd",
   "val": {"item_ref": "_5ic5uk22icm_7ws16feu699", "jtype": "item_ref", "space": ".root"}},
  {"attr": "_2ummst105ck_xracfy8v87y", "val": {"jtype": "tuple", "tuple": []}},
  {"attr": "_37x98fyestf_ttup2cu68r6", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOM_DEBUG(run, ", {"item_ref": "_5ic5uk22icm_7ws16feu699", "jtype":
      "item_ref", "space": ".root"}, ")"]}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output primitive for MOM_DEBUG(run, ...)"}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_9sd1mh9q1zf_3duewi6fsaq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes with exit code >0"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9u6a6xy2e1p_qeapfc73cm4','{"attr": [{"attr": "_232t5qs2v8e_zu2wy53cqe3", "val": {"item_ref": "_16cd0fvmdrh_r77ajpy26za",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "block to handle ajax_appl when what=quit"}, {"attr": "_94tq2iauet8_jujpjhjrzsm",
   "val": {"jtype": "node", "node": "_0yyp8vmw4si_wf49m4d4zwq", "sons": [{"jtype":
      "node", "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node",
"node": "_4ew7uvid6ep_wzq6f8hruz9", "sons": [{"item_ref": "_16cd0fvmdrh_r77ajpy26za",
  "jtype": "item_ref", "space": ".root"}, "<em>Monimelt</em> <b>quitting</b> without saving at <i>",
 {"jtype": "node", "node": "_4xicv8w07x7_3hzzmpw8iwt", "sons": ["%c", {"jtype": "node",
    "node": "_1yz5fpjm6yt_319wacq9346", "sons": [{"item_ref": "_9jdufs9sew7_u6x3k3wfseq",
      "jtype": "item_ref", "space": ".root"}]}]}, "</i>"]}]}, {"jtype": "node",
      "node": "_5c789try94y_ssy6a22fpep", "sons": [{"jtype": "node", "node": "_7j5uq75spm6_wuhuc4pe5th",
"sons": [{"item_ref": "_16cd0fvmdrh_r77ajpy26za", "jtype": "item_ref", "space": ".root"},
 "text/html", {"item_ref": "_4ie5pvk8m1x_rzsut0mdza0", "jtype": "item_ref",
  "space": ".root"}]}]}, {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
      "sons": [" MOM_INFORMPRINTF(\"quitting Monimelt per web request\");\n exit(EXIT_SUCCESS); // per web request\n"]}]}}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_items VALUES('_9w7y4v2xpyr_f48demccutx','{"attr": [{"attr": "_19ufza9zf05_muxxhkmm7ww", "val": {"jtype": "node", "node": "_8x6fxcm4z2k_vdaqicfi4z0",
    "sons": ["MOMOUTDO_SMALL_NEWLINE"]}}, {"attr": "_2ummst105ck_xracfy8v87y",
   "val": {"jtype": "tuple", "tuple": []}}, {"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "output connective for small newlines."}], "content": null, "kind": null,
 "payload": null}
','');
INSERT INTO t_items VALUES('_9wwqwxqcm4p_y7di7fs8tsk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "common index number"}],
 "content": null, "kind": null, "payload": null}
','');
INSERT INTO t_names VALUES('CLOCK_REALTIME','_9jdufs9sew7_u6x3k3wfseq','.root');
INSERT INTO t_names VALUES('GET','_9dsak0qcy0v_1c5z9th7x3i','.predef');
INSERT INTO t_names VALUES('HEAD','_47fatww79x6_vh8ap22c0ch','.predef');
INSERT INTO t_names VALUES('HTTP_NOT_FOUND','_9ju0dqm82h4_hqi2kjqxa83','.root');
INSERT INTO t_names VALUES('HTTP_OK','_4ie5pvk8m1x_rzsut0mdza0','.root');
INSERT INTO t_names VALUES('MOMOUT_C_STRING','_1x4wmv7yiym_w3zfpdv8q0m','.root');
INSERT INTO t_names VALUES('MOMOUT_DEC_INTPTR_T','_89uzitiq84u_50h4cqpkay9','.root');
INSERT INTO t_names VALUES('MOMOUT_DOUBLE_F','_5k1ph0qwrky_v6a6ap5uxzp','.root');
INSERT INTO t_names VALUES('MOMOUT_DOUBLE_G','_5m370797c14_32w6fyj9749','.root');
INSERT INTO t_names VALUES('MOMOUT_DOUBLE_TIME','_4xicv8w07x7_3hzzmpw8iwt','.root');
INSERT INTO t_names VALUES('MOMOUT_FLUSH','_3scu6d0kfes_vc7f1ryt673','.root');
INSERT INTO t_names VALUES('MOMOUT_GPLV3P_NOTICE','_6xjwjjh686j_hczfxhwjp1f','.root');
INSERT INTO t_names VALUES('MOMOUT_HEX_INTPTR_T','_1eqee5utt20_pyw3mat0zw4','.root');
INSERT INTO t_names VALUES('MOMOUT_HTML','_822d1dkak1h_cw0598pe64d','.root');
INSERT INTO t_names VALUES('MOMOUT_INDENT_LESS','_03f56tp7q6w_f1t3fiyivph','.root');
INSERT INTO t_names VALUES('MOMOUT_INDENT_MORE','_1d21u7ivquk_qm06hv72eu8','.root');
INSERT INTO t_names VALUES('MOMOUT_ITEM','_2rpd6wy50xt_etjfuj6s8jr','.root');
INSERT INTO t_names VALUES('MOMOUT_JSON_VALUE','_71a9mq07fvv_448p3r7vikd','.root');
INSERT INTO t_names VALUES('MOMOUT_JS_HTML','_4x8e2mwmacp_ekxdw3vqqpd','.root');
INSERT INTO t_names VALUES('MOMOUT_LITERAL','_5w5idj1j9ah_u00krzevvj7','.root');
INSERT INTO t_names VALUES('MOMOUT_NEWLINE','_3ff1zzfsxzc_xk423qutvqr','.root');
INSERT INTO t_names VALUES('MOMOUT_SLASHCOMMENT_STRING','_8qqv70zdmmk_w4qs625q0s7','.root');
INSERT INTO t_names VALUES('MOMOUT_SMALL_NEWLINE','_9w7y4v2xpyr_f48demccutx','.root');
INSERT INTO t_names VALUES('MOMOUT_SMALL_SPACE','_0wm3xd5kpcc_y1e349eizfj','.root');
INSERT INTO t_names VALUES('MOMOUT_SPACE','_5yms6ak2u0p_ri0wwutvhc1','.root');
INSERT INTO t_names VALUES('MOMOUT_STRING_VALUE','_1dfsr53udxw_k9h81rfpdx8','.root');
INSERT INTO t_names VALUES('MOMOUT_VALUE','_9hjrq1h1s0p_h1qpmj1ucuy','.root');
INSERT INTO t_names VALUES('MOM_WEBX_OUT','_4ew7uvid6ep_wzq6f8hruz9','.root');
INSERT INTO t_names VALUES('POST','_5wmusj136kq_u5qpehp89aq','.predef');
INSERT INTO t_names VALUES('agenda','_02u53qxa7dm_sttmhffpchr','.predef');
INSERT INTO t_names VALUES('ajax_appl','_06uk4pppvx9_huv0v11v18j','.root');
INSERT INTO t_names VALUES('ajax_appl_closure','_0ce0mmy7myq_1t5iw2mfrvm','.root');
INSERT INTO t_names VALUES('arguments','_2x2zpyhfqum_0utui69rzea','.predef');
INSERT INTO t_names VALUES('assign','_8vzddhpmhp8_z0005cqyqzf','.predef');
INSERT INTO t_names VALUES('assoc_get','_70mt4fvrva2_pk76eevwada','.root');
INSERT INTO t_names VALUES('assoc_put','_5c5jh9185sv_qru5amf9v18','.root');
INSERT INTO t_names VALUES('atitmv','_8yepyw577hd_yr28eiysfss','.root');
INSERT INTO t_names VALUES('attr','_6w3dvx83dfw_xzc6aif6isv','.predef');
INSERT INTO t_names VALUES('block','_94tq2iauet8_jujpjhjrzsm','.predef');
INSERT INTO t_names VALUES('blocks','_1r880c1yk3z_i5e8mprcj90','.predef');
INSERT INTO t_names VALUES('call','_02av6173qvf_pehzhe755j2','.predef');
INSERT INTO t_names VALUES('call_at_state','_3dqr46p2xf4_29kf5vdtw4z','.root');
INSERT INTO t_names VALUES('case','_38w4qwrmd6z_74x5z80v5k6','.predef');
INSERT INTO t_names VALUES('chunk','_8x6fxcm4z2k_vdaqicfi4z0','.predef');
INSERT INTO t_names VALUES('closed_values','_9mxi9e605ay_ihpjyrwq250','.predef');
INSERT INTO t_names VALUES('closure','_97zkxf62r11_6eedwwv3eu8','.predef');
INSERT INTO t_names VALUES('closure_function','_3230kudxwis_qs5ss05i85w','.predef');
INSERT INTO t_names VALUES('code','_0yyp8vmw4si_wf49m4d4zwq','.predef');
INSERT INTO t_names VALUES('comment','_41u1utcxyek_22cftxt3xxm','.predef');
INSERT INTO t_names VALUES('constants','_2u8svx94yq4_34icz9j1fyx','.predef');
INSERT INTO t_names VALUES('content','_8s357rq2dzk_k8ze95tikjm','.predef');
INSERT INTO t_names VALUES('count','_6f4k9pqzryk_w25f8vxuyyc','.predef');
INSERT INTO t_names VALUES('cpu_time','_50623j9vemk_1hp2q2czrhi','.root');
INSERT INTO t_names VALUES('cstr','_389t77v85ej_fwpy6exy62x','.root');
INSERT INTO t_names VALUES('ctype','_0ee6afx5850_ji17eq0wmfa','.predef');
INSERT INTO t_names VALUES('dbl_x','_837uvkhhyar_tdhv0jy5r72','.root');
INSERT INTO t_names VALUES('debug_run','_9r25vmrrk8a_vus02yyfp8w','.root');
INSERT INTO t_names VALUES('dispatch','_8fk0de81s9r_5d4v4x7qmxr','.predef');
INSERT INTO t_names VALUES('display_value','_7pyjxst21ce_vhc0tk0em0u','.root');
INSERT INTO t_names VALUES('do','_5c789try94y_ssy6a22fpep','.predef');
INSERT INTO t_names VALUES('double','_7j7x11c25h3_wkchtuwpusx','.predef');
INSERT INTO t_names VALUES('doubles','_17spwr8dkzv_tsf2s8diazu','.predef');
INSERT INTO t_names VALUES('dump','_24w2ce2eq1z_pddi9j2czci','.root');
INSERT INTO t_names VALUES('dump_data','_5tihf27p4rj_t80tzx4fxrf','.root');
INSERT INTO t_names VALUES('dump_state','_6rs26jmh9ya_jv0aiqf4kvx','.root');
INSERT INTO t_names VALUES('edit_value','_73im2zryfij_a7zmkketcfc','.root');
INSERT INTO t_names VALUES('editors','_7qk90k9vx0u_31ivff77td7','.root');
INSERT INTO t_names VALUES('elapsed_time','_6u6cp2a2tsz_ses4qchc3y3','.root');
INSERT INTO t_names VALUES('error','_4qcw2mwjswm_j9q0k9d04hm','.predef');
INSERT INTO t_names VALUES('exited','_3v4d7uzex6f_euek4pztiuh','.predef');
INSERT INTO t_names VALUES('failed','_9sd1mh9q1zf_3duewi6fsaq','.predef');
INSERT INTO t_names VALUES('first_module','_65961crktpj_vtt30qeqv21','.root');
INSERT INTO t_names VALUES('for_each_up_to','_11xee72y1d3_t3cqzi5dq3k','.root');
INSERT INTO t_names VALUES('formals','_2ummst105ck_xracfy8v87y','.predef');
INSERT INTO t_names VALUES('frames','_4cw8jv45vsk_4mh9ex64904','.predef');
INSERT INTO t_names VALUES('get_attribute','_5vi29c2i54k_i2ufkty9kmp','.root');
INSERT INTO t_names VALUES('high','_6qcw93kypcv_0iiepqtk73j','.root');
INSERT INTO t_names VALUES('i','_2vy0ah3jrd1_mm66ja7rfj7','.root');
INSERT INTO t_names VALUES('id','_7a9sxskjhcp_kpf30ka97ex','.predef');
INSERT INTO t_names VALUES('if','_8ejwdt1a5yx_2meizztvte0','.predef');
INSERT INTO t_names VALUES('intptr_t','_51u3st4u9mc_zdvms6jti0a','.predef');
INSERT INTO t_names VALUES('item','_36esaxiqmd5_ciet8ws1f5d','.root');
INSERT INTO t_names VALUES('item_ref','_6hf2vzmrsee_t35suhjvtj4','.predef');
INSERT INTO t_names VALUES('itmv','_1a2aavj5vir_2hz681zdfqd','.root');
INSERT INTO t_names VALUES('ix','_9wwqwxqcm4p_y7di7fs8tsk','.root');
INSERT INTO t_names VALUES('jit','_24yt56xf3d5_4w80i326kjz','.predef');
INSERT INTO t_names VALUES('json_array','_35vp60aw7em_d436vfie4ud','.predef');
INSERT INTO t_names VALUES('json_entry','_37kswkaa035_qjpxwd7e67f','.predef');
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
INSERT INTO t_names VALUES('mom_clock_time','_1yz5fpjm6yt_319wacq9346','.root');
INSERT INTO t_names VALUES('mom_double_val','_0ju7898wddc_1296qwc3a30','.root');
INSERT INTO t_names VALUES('mom_double_val_def','_7dritvdcmhf_yw3u1jquevr','.root');
INSERT INTO t_names VALUES('mom_double_val_or0','_4rrcat1av7m_55z67m8hurq','.root');
INSERT INTO t_names VALUES('mom_elapsed_real_time','_5a9mai6ar7t_87kveq8ic29','.root');
INSERT INTO t_names VALUES('mom_integer_val_def','_6ek4zitda2z_dfd1sddm812','.root');
INSERT INTO t_names VALUES('mom_integer_val_or0','_21a2k0s62d3_t9r0i5860kd','.root');
INSERT INTO t_names VALUES('mom_integer_val_orm1','_98z00ctcfaa_v3vsrczdt0e','.root');
INSERT INTO t_names VALUES('mom_is_double','_6mutx3sq2eu_23r4mvpac6m','.root');
INSERT INTO t_names VALUES('mom_is_integer','_3x55htdct2h_upumrci06w1','.root');
INSERT INTO t_names VALUES('mom_is_json_array','_18zm20vu86d_jey4zt03k8z','.root');
INSERT INTO t_names VALUES('mom_is_jsonable','_84ih8qi7vw2_k31sih7ewjj','.root');
INSERT INTO t_names VALUES('mom_is_jsonob','_4v7s8xjeh8e_q8h93d2f43y','.root');
INSERT INTO t_names VALUES('mom_is_string','_9j2kuwuk892_ajmt7d309a5','.root');
INSERT INTO t_names VALUES('mom_json_array_size','_1uarhtr96qc_28hwh1ueaq4','.root');
INSERT INTO t_names VALUES('mom_json_cmp','_15f5zxxsky9_3vv7edvmk75','.root');
INSERT INTO t_names VALUES('mom_json_cstr_cmp','_3jkcrmcyzep_4ddf6qriy44','.root');
INSERT INTO t_names VALUES('mom_jsonob_get','_5x0p1k3qswi_wwe62qt0zea','.root');
INSERT INTO t_names VALUES('mom_jsonob_getstr','_2apx2jm2jjk_m470v1fcei0','.root');
INSERT INTO t_names VALUES('mom_jsonob_size','_0pqeesw57dy_1q3yyp3a7um','.root');
INSERT INTO t_names VALUES('mom_make_integer','_32r2krfzqaa_ep4usqkhayd','.root');
INSERT INTO t_names VALUES('mom_make_string','_113idxzj0u9_h5c6i4t8hdy','.root');
INSERT INTO t_names VALUES('mom_stop_work_with_todo','_7x7vwep1fw4_i158put1x07','.root');
INSERT INTO t_names VALUES('mom_string_cstr','_10ji3aajfx3_983iz4013ec','.root');
INSERT INTO t_names VALUES('mom_string_same','_20ds5w0c9z1_tvf5h12wrqp','.root');
INSERT INTO t_names VALUES('mom_string_slen','_59wxs8qi3jd_vctt2cyuva9','.root');
INSERT INTO t_names VALUES('mom_type','_3h56h532u19_1ssc4pje2x6','.root');
INSERT INTO t_names VALUES('mom_value_cmp','_16wqrr2mwae_fmcdtax6p6s','.root');
INSERT INTO t_names VALUES('mom_value_hash','_5ar352xfzeu_2hrq5jjs95p','.root');
INSERT INTO t_names VALUES('mom_value_json','_5w0cahd5yri_e7f3m67xf4f','.root');
INSERT INTO t_names VALUES('mom_value_to_item','_5s26uzwhveh_fv3twe3s2je','.root');
INSERT INTO t_names VALUES('mom_webx_post_arg','_8zwifiifaf0_pyayp6hpha4','.root');
INSERT INTO t_names VALUES('mom_webx_reply','_7j5uq75spm6_wuhuc4pe5th','.root');
INSERT INTO t_names VALUES('mom_webx_session','_62ryyaxj112_wyw4upc7deh','.root');
INSERT INTO t_names VALUES('momcstr_t','_80e7dsukuq3_6p7jffmz1yi','.predef');
INSERT INTO t_names VALUES('momval_t','_3uwzqwvj6zj_s63am4qivpt','.predef');
INSERT INTO t_names VALUES('named_items','_5xw5qm751tv_jvm099ita0w','.root');
INSERT INTO t_names VALUES('names','_6vrzjdj7ij8_dupds6c9895','.root');
INSERT INTO t_names VALUES('namestr','_3i8mqyfreeh_2w73i1khumx','.root');
INSERT INTO t_names VALUES('nb_dumped_items','_1tzf3q2dix5_jqxphp9ivcw','.root');
INSERT INTO t_names VALUES('node','_4m7x6811f6j_t480zu575mz','.predef');
INSERT INTO t_names VALUES('noop','_240dwt57s08_a8uy366sev5','.root');
INSERT INTO t_names VALUES('notice','_7diyc1cwj8z_x630afccr8e','.predef');
INSERT INTO t_names VALUES('null','_7mj6c8az5d3_vzmwd2uuiu7','.root');
INSERT INTO t_names VALUES('numbers','_3fw5acswe59_9016fqe4d41','.predef');
INSERT INTO t_names VALUES('outargs','_5ic5uk22icm_7ws16feu699','.root');
INSERT INTO t_names VALUES('output','_0te6f7f9pz7_m91yy9iv5pd','.predef');
INSERT INTO t_names VALUES('output_expansion','_19ufza9zf05_muxxhkmm7ww','.predef');
INSERT INTO t_names VALUES('params','_4215uc2u6qk_52kqyra86y5','.predef');
INSERT INTO t_names VALUES('payload','_41v0erax6my_m6pytj0793u','.predef');
INSERT INTO t_names VALUES('predefined','_133zjf1f9zp_jq8kti38sd7','.root');
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
INSERT INTO t_names VALUES('show_html_for_item_proc','_0z0rsvwfkcj_dcpkx68i074','.root');
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
INSERT INTO t_names VALUES('test_proc1','_682ush7cppa_s7vzfd2rsxp','.root');
INSERT INTO t_names VALUES('todoproc','_2qq1dh2ucpr_qtv6staqhti','.root');
INSERT INTO t_names VALUES('translate_procedure','_0afqepa7jkr_qky26hpv98d','.root');
INSERT INTO t_names VALUES('translation','_8y1sw8z084j_4ts0y0jydha','.root');
INSERT INTO t_names VALUES('tuple','_7vw56h18sw0_hv77m6q8uxu','.predef');
INSERT INTO t_names VALUES('update_display_value','_1f94j87qumw_mhzkriesx7c','.root');
INSERT INTO t_names VALUES('v','_0hi99dr3qqs_yuuciyy7xfz','.root');
INSERT INTO t_names VALUES('v1','_5ryj031qxp3_8rx001d7y2x','.root');
INSERT INTO t_names VALUES('v2','_96qx16z900r_dw5ppp22a0s','.root');
INSERT INTO t_names VALUES('val','_7wk9y7e7r0z_575esi8ys5x','.predef');
INSERT INTO t_names VALUES('values','_91pketvc5pz_wq0v0wpauw8','.predef');
INSERT INTO t_names VALUES('var','_2d7i21ihwd8_xjcp4uhs11u','.root');
INSERT INTO t_names VALUES('verbatim','_0x2k07ik4tm_ed7vqphf5ak','.predef');
INSERT INTO t_names VALUES('void','_02q6zk9f5st_im0z75re15f','.predef');
INSERT INTO t_names VALUES('web_handler','_7sav6zery1v_24sa6jwwu6c','.predef');
INSERT INTO t_names VALUES('webses','_4wx5f4704sp_v9kfazsqe9h','.root');
INSERT INTO t_names VALUES('webx','_16cd0fvmdrh_r77ajpy26za','.root');
INSERT INTO t_names VALUES('whatv','_350hj5kfymd_145tfc1sevi','.root');
COMMIT;
-- state-monimelt end dump 
