-- state-monimelt dump 2014 Nov 06

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
INSERT INTO t_params VALUES('dump_reason','exit dump');
INSERT INTO t_items VALUES('_02mtzeca0pf_kc9d1i34ap4','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1853}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_02u53qxa7dm_sttmhffpchr','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The agenda is central to Monimelt.\nIt is the queue of taskets to be executed by worker threads."}],
 "content": null, "kind": "queue", "payload": []}
');
INSERT INTO t_items VALUES('_031ar4875we_s1xarptwct2','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1845}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_05utk0hrpcw_usi08r18z82','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 508}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_06yp8ueq6yf_5ts408yww29','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the kind of the payload of an item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_085krqf192t_z1m3zs77ww5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to be used inside display items"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_09c2urv85vr_cj9f93j15z1','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1306}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
      "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref",
"space": ".predef"}, -128, 127]}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0acmecj244a_6krws4rx7v1','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the body in a routine, or the routine elsewhere"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0afqepa7jkr_qky26hpv98d','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "translate a single procedure"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0cte3uqc744_m1674pufa83','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 989}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0f6fadaa09t_cwsaijsevaq','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1906}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_87axj4q44z5_xddqmrkw875", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0frfrj16j0j_tpz51c5ffrj','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 280}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_984m70p3jfc_2385qzu6x15", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_0h331ch957p_j6a8i7v4e6y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0hpzi8m7wym_1y4ypmm9y47','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handler for JSONRPC requests"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0i2zv78m8mm_zrzwkmuv9fy','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "type for 32 bits signed"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0ihu411vkua_z4sh56hicdt','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "body of a routine"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0mvqzz7radr_zk1errj4eus','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1253}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_57vx8i37ar4_1346w0cep34", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0mwp3hwhqt3_48vf8kavu1q','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0pcyvpuhi7w_3965ke5dwyf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1857}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_0ph6457k18e_jp473ckp3zs','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1368}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0pijqmm7krh_sxxavschmpi','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1322}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_0pppmw2yfdj_vpumuxt09y3','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2331}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0rmhd2s0u3f_z773fur038v','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1842}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0v6se8r3uf4_eqye8sq0r3q','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1372}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0vz73hu7aph_cyhxumztwzy','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1881}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_2x2u9vdare0_5dj5y8zt6ww", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0xv5kv0peuq_iepz5x197q1','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 278}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0yavvxi653k_e1ui813cih8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "type for 64 bits unsigned"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0yeu3tyzw9z_jqhcu2z2x34','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes unsigned_short type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0yyp8vmw4si_wf49m4d4zwq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error code in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0z10v92cc0m_ydhtvzrujjz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for enum types"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_0zmdkdxj7kp_491yqpcuaz8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for assignment in programs"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": [{"item_ref":
      "_2d7i21ihwd8_xjcp4uhs11u", "jtype": "item_ref", "space": ".root"},
     "_7wk9y7e7r0z_575esi8ys5x"]}}, {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype":
    "node", "node": "_2vxxtir316j_meap5sq6ykr", "sons": [{"item_ref": "_2d7i21ihwd8_xjcp4uhs11u",
      "jtype": "item_ref", "space": ".root"}, " = (", {"item_ref": "_7wk9y7e7r0z_575esi8ys5x",
      "jtype": "item_ref", "space": ".predef"}, ")"]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_107fvfyjvty_fm8a4q6eika','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1127}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_11hm4aw0exy_hfhij5sayy3','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1283}], "content": null,
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
INSERT INTO t_items VALUES('_12jd44c02kt_aqmepaxvrqc','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1251}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_16z19uecsa0_9ppuyipkrdx','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 633}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
"sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
    "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"},
   -128, 127]}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_57vx8i37ar4_1346w0cep34",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_17spwr8dkzv_tsf2s8diazu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the double floating-point numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_18jez9x90m0_zawc40wpwvz','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1533}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_0z10v92cc0m_ydhtvzrujjz",
      "sons": [{"item_ref": "_7tfqkjxktup_1t1a8csf66a", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1989w3c3m0k_2mw7tizf5h1','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1828}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_19fei8jwqtj_0mqfvi8s172','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1akf45cmq0w_pxm7mz4vz4x','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1544}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1dmki2tps3h_fzcs0sj24m9','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2386}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1ep6tdcx90e_cexqjti73wx','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1548}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1f94j87qumw_mhzkriesx7c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to update the value in displays after edition."}],
 "content": null, "kind": "routine", "payload": "update_display_value"}
');
INSERT INTO t_items VALUES('_1hc511xzwc4_3uyk52zy4v5','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1128}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1he6qfkkr8e_8x22eu10cus','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1185}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1ij3y9cmvk5_8tkiqiy17ws','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC routine to define some field in union or record"}],
 "content": null, "kind": "closure", "payload": {"closed_values": ["json_rpc_meltmom_define_field spare 0",
   "json_rpc_meltmom_define_field spare 1", "json_rpc_meltmom_define_field spare 2"],
  "closure_routine": "json_rpc_meltmom_define_field"}}
');
INSERT INTO t_items VALUES('_1jriw29kezf_4wx1rtck86x','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 184}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1k0aik92ucm_zrq3tuk876s','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1kqu73x9qhf_h0hkrfedk8z','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 611}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"item_ref":
      "_89ejvxupprm_f219pqwz13s", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
    "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1ky66221jjr_atfxmj1zi16','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2603}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}]}]}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1mdwwz1zc1k_uui6fjawdxw','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2563}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
    "sons": [{"item_ref": "_0yavvxi653k_e1ui813cih8", "jtype": "item_ref",
      "space": ".predef"}, 0, 9223372036854775807]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1mv28mq6p9s_aairsjxey8i','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 161}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_89ejvxupprm_f219pqwz13s",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1t5e93sh0uj_fz3qakh4d1i','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1325}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1t7vvf9p1qm_qy53em9d5p8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for performing side-effects in programs"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1tkmhtxtu7f_y3h9kp9jvje','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2599}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_0mwp3hwhqt3_48vf8kavu1q",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1ttzd8kyed8_15ww8d9c24p','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 627}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1uc4z4204z5_tjmdyvsmc78','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 629}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1up5sjxcj8i_cdkus86zxhc','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1837}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1upqvx9m5si_pe93kw913pf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1130}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_6jyxveszh5w_28ruvjudjiw",
"jtype": "item_ref", "space": ".root"}, 0, 4294967295]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1vd2t9p4krp_yjsuuc88k61','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for pointers"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1wevxkfudp9_cpeu1adxcrp','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 185}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1wjk5me9u9c_y0ksm1sj8j7','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1545}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_1xvf96xdsje_ttcp13u7rw6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1694}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_1y91p3us9s9_p88di58xuek','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC method to declare some name"},
  {"attr": "_0hpzi8m7wym_1y4ypmm9y47", "val": {"item_ref": "_6d4dwqa6m09_c5vtjswfpfi",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_21xtrp170v1_17c7pk1rh4r','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for variadic functions"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_23ey21s44px_iarreqeqhr5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes unsigned_long type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_23kika6z9qv_7uj6hwy5qdf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1862}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"item_ref":
      "_57vx8i37ar4_1346w0cep34", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_23ucm00wpiw_hhyz7px0c73','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1696}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_240dwt57s08_a8uy366sev5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "useless noop routine"}],
 "content": {"jtype": "node", "node": "_240dwt57s08_a8uy366sev5", "sons": ["{spare1 noop}",
   "{spare2 noop}", null]}, "kind": "routine", "payload": "noop"}
');
INSERT INTO t_items VALUES('_24i91kxrvwm_10cmfd12dck','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "type for pointer-sized integers"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_24w2ce2eq1z_pddi9j2czci','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "related to dump of full state"},
  {"attr": "_0hpzi8m7wym_1y4ypmm9y47", "val": {"item_ref": "_6zm92afs4yc_60a8ujmi1ef",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_24yt56xf3d5_4w80i326kjz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JIT code of JIT-ed routines"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_266cwehdrjc_144jy18dwh1','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 503}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_27k36a8racw_as8j0p646ia','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2619}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_28941cvehx8_9rf4udyeq8v','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure routine name"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_28jrsus3ti3_da1ztz7ex3x','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 515}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_28qsshh7hwr_k2jdce4atqt','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1331}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2a1fq7ks2ak_wvxk68qe4mv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for block of statements"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2aeczcr3u9f_cz273p64kkj','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1741}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_2d28hsmu8zm_fziq009d8q8','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 782}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_2d7i21ihwd8_xjcp4uhs11u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some variable"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2eq3e457rtx_zu76dzrdk98','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 984}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2fjizx76kia_567e7h9s69z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for minimum"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2fvh1ti34mc_sqjvu53d3i5','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1907}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"},
     {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_87axj4q44z5_xddqmrkw875", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2ir1y95wiuk_j879f5utux9','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1832}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2m5kpyivq0q_dp274hd64ur','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2611}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2mayc646pdu_w4d18fmx8u3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "some sequence"}], "content":
 null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2qmau9u6ie1_7ry5vh1vhe0','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for monimelt union prefixed with mom"}],
 "content": null, "kind": null, "payload": null}
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
INSERT INTO t_items VALUES('_2w8v4htahij_su1zqvfuyd6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1856}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2x2u9vdare0_5dj5y8zt6ww','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 512}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_2xv0dpvp5md_6u53m54kw0i','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1006}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9y82yz8z01a_x0dx4kx0x18",
      "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0", "jtype": "item_ref",
"space": ".root"}]}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_32dzf3hj8yw_t4yj6mayh8p','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 285}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_3vvtdeqxssw_dthyjy2dz5t", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_34jvm2m8mam_imy67jedrhs','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_350vk8y5jau_wscjm4tmxfr','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2607}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_0mwp3hwhqt3_48vf8kavu1q",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
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
INSERT INTO t_items VALUES('_39xwz6y59rr_muu52mx672m','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2560}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
    "sons": [{"item_ref": "_6fiizwecy7v_pxd0wxx8c1m", "jtype": "item_ref",
      "space": ".predef"}, 0, 4294967295]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_3dqr46p2xf4_29kf5vdtw4z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Call with results in res, in given state, the closure clos with given arguments"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_6djzuwz5pav_cri386ywjhj",
     "_6f9870y6v8t_kp8fcmq2ezv", "_97zkxf62r11_6eedwwv3eu8", "_8um1q4shitk_tpcmedvsfzu"]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3ei0fm012ue_2aryjhv3v0t','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 283}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_984m70p3jfc_2385qzu6x15", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_95jrd5tjcsu_41chfihe6za", "jtype": "item_ref",
"space": ".predef"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref":
    "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3eu0rdq4upj_dp5ptr6hj04','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the expansion of a routine"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3fw5acswe59_9016fqe4d41','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the integer numbers of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3h0cu9jtev8_fi1tqhqtpk4','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2596}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_0mwp3hwhqt3_48vf8kavu1q",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
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
INSERT INTO t_items VALUES('_3jrcmvrfmaw_ypdwhej35cx','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2032}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
  "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}]}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_9arrtc64f96_z86ukm7u5sf", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3k72ktm822u_8exaz46p3y9','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 308}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_626yhtpekvc_eyqv9eyzmkf",
  "jtype": "item_ref", "space": ".root"}]}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3k7s4156a7a_1p1rar740pk','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 780}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_3qf9kmsmfyx_9w5fsqa2tzm','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2312}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_3r17a3jsysq_8vspi2t8cph','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1903}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_87axj4q44z5_xddqmrkw875", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3ru7qrk03jv_v76f8ity5i6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 808}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"item_ref":
      "_7e1ak9qwf9h_wp5tptm4uyi", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_835qk04icz0_e6srx044eqq", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3szfdhp0656_2tar36yeia5','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2638}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3u5f8v43w3k_acw3tuers2u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes short type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3v4d7uzex6f_euek4pztiuh','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3vvtdeqxssw_dthyjy2dz5t','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 540}], "content": null,
 "kind": null, "payload": null}
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
INSERT INTO t_items VALUES('_3z5f1d6uqxq_29m1cfv36rf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2623}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_3zqd7ai3rtu_md9athkx17u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the buffer payload kind, and also the clipboard buffer in editors"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_3ztr5wx3aws_fkquc120ajj','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1709}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_413ff3d1ajz_0f0vvj7m1e3','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC routine to define some type"}],
 "content": null, "kind": "closure", "payload": {"closed_values": ["json_rpc_meltmom_define_type spare 0",
   "json_rpc_meltmom_define_type spare 1", "json_rpc_meltmom_define_type spare 2"],
  "closure_routine": "json_rpc_meltmom_define_type"}}
');
INSERT INTO t_items VALUES('_41ciqzka19m_wjq6xe7u28h','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 764}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_34jvm2m8mam_imy67jedrhs",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_862u2w97uth_dvy6mmywjyq", "jtype": "item_ref",
"space": ".predef"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref",
"space": ".predef"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref":
    "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_41r244ahp3q_j09a33vry98','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1901}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_87axj4q44z5_xddqmrkw875", "jtype": "item_ref", "space": ".root"}]}}],
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
INSERT INTO t_items VALUES('_42cx8xskxe3_vu23eezcksq','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4363z4ey9pa_zvc1w07qctv','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1683}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_456hz6qd6x2_jyy24w6q84z','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "common length integer"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_46uqew6jxj7_m9te8h258md','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1889}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"},
     {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_2x2u9vdare0_5dj5y8zt6ww", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_47fatww79x6_vh8ap22c0ch','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP HEAD method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_47sm0wm5q8f_7kexsx1q63k','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1909}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_87axj4q44z5_xddqmrkw875", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4986vckxzau_i0zcf008zy0','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1319}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref":
  "_19fei8jwqtj_0mqfvi8s172", "jtype": "item_ref", "space": ".root"}]}]},
     {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_4a62sfh041d_ews4tzh4apy','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2590}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_0mwp3hwhqt3_48vf8kavu1q",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_4cw8jv45vsk_4mh9ex64904','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "frames in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4dexizaxjta_1vvp4hchyu6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2295}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4ezpkss1akd_94f4h25sqe4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the sons of nodes"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4fkajpe0f90_vzy4efaffse','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1834}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
  "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}]}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_4hvu6yexjt2_w338z9c3dk0','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1700}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4jp2meuzru2_a58afyxwxa2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error message in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4kkhcwtk7e8_e26xhjapp7j','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 979}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4m7x6811f6j_t480zu575mz','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, indicate nodes, or give their connective item"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4mha85xcfwi_9zqcvkiy3dk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "The false of JSON.\nWe cannot use false because it is a #define-ed macro."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4pjyeaerj5v_j1me6wz2qfp','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1257}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4qcw2mwjswm_j9q0k9d04hm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "error case, on for JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4qd5iv2jk0x_t09911fyth3','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1121}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_2x2u9vdare0_5dj5y8zt6ww", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4qyqy53wv00_vky0cm98ryd','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 947}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_4qywz13t3m7_jkxp06jp1u8','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1552}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_89ejvxupprm_f219pqwz13s", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4rjaerz2211_6h0xzvp4v4v','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 835}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_705c10s4fte_27kjcrw2rxr", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4uqfx2kqr91_ha9rqdwhcwe','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1846}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4v3d25zvxed_mycxxj2mcjr','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2349}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_4v93t3jzrtz_srt9ear8fm8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "handle ''ajax_complete_name'' webrequests"},
  {"attr": "_7sav6zery1v_24sa6jwwu6c", "val": {"jtype": "node", "node": "_4v93t3jzrtz_srt9ear8fm8",
    "sons": ["{spare1 ajax-complete_name}", "{spare2 ajax-complete_name}",
     "{spare3 ajax-complete_name}", null]}}], "content": null, "kind": "routine",
 "payload": "ajax_complete_name"}
');
INSERT INTO t_items VALUES('_4wd221340d0_9y42sarswa2','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1636}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4wu033p0x2p_6kiar5yv3ws','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1224}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6zwuvxcfhhw_fdkwjd6mjwq",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_4yf619stz5m_51zzjhtktj0','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1247}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
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
     {"jtype": "node", "node": "_0zmdkdxj7kp_491yqpcuaz8", "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c",
"jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": {"item_ref": "_5vi29c2i54k_i2ufkty9kmp",
 "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_7sqk8vh89xr_6tj8dq7vqju",
  "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_53cuy70z4tf_86tzz364trd",
  "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c", "jtype": "item_ref", "space":
    ".predef"}]}]}]}, {"jtype": "node", "node": "_0zmdkdxj7kp_491yqpcuaz8",
      "sons": [{"item_ref": "_456hz6qd6x2_jyy24w6q84z", "jtype": "item_ref",
"space": ".root"}, {"jtype": "node", "node": {"item_ref": "_41xwu6cpvq9_ezp5wzq7t4x",
 "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref": "_5yfdp53cpi1_0i5k33wms7c",
  "jtype": "item_ref", "space": ".predef"}]}]}, {"jtype": "node", "node": {"item_ref":
       "_11xee72y1d3_t3cqzi5dq3k", "jtype": "item_ref", "space": ".root"},
      "sons": [{"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref",
"space": ".root"}, 0, {"item_ref": "_456hz6qd6x2_jyy24w6q84z", "jtype": "item_ref",
"space": ".root"}, {"jtype": "node", "node": "_0ihu411vkua_z4sh56hicdt", "sons": [{"jtype":
  "node", "node": "_0zmdkdxj7kp_491yqpcuaz8", "sons": [{"item_ref": "_70aer7teeui_kvzkiqq2rd2",
    "jtype": "item_ref", "space": ".root"}, {"jtype": "node", "node": {"item_ref":
     "_80wxf4c8q92_qq8k6xc0xxj", "jtype": "item_ref", "space": ".root"}, "sons": [{"item_ref":
      "_5yfdp53cpi1_0i5k33wms7c", "jtype": "item_ref", "space": ".predef"},
     {"item_ref": "_9wwqwxqcm4p_y7di7fs8tsk", "jtype": "item_ref", "space": ".root"}]}]},
 {"jtype": "node", "node": {"item_ref": "_3dqr46p2xf4_29kf5vdtw4z", "jtype": "item_ref",
   "space": ".root"}, "sons": [{"item_ref": "_3j3s2e0510a_096chqpijq7", "jtype": "item_ref",
    "space": ".predef"}, {"item_ref": "_6qp266amrz7_izi5rx6ukuk", "jtype": "item_ref",
    "space": ".root"}, {"item_ref": "_0afqepa7jkr_qky26hpv98d", "jtype": "item_ref",
    "space": ".root"}, {"item_ref": "_70aer7teeui_kvzkiqq2rd2", "jtype": "item_ref",
    "space": ".root"}, {"item_ref": "_8y1sw8z084j_4ts0y0jydha", "jtype": "item_ref",
    "space": ".root"}]}]}]}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4z5ma494cak_8711cf2mtuc','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1119}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_4ztwemps3vr_y2i1hadm57s','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes signed char type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_502084k6w6w_f3qsz61zmi9','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2561}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
    "sons": [{"item_ref": "_6fiizwecy7v_pxd0wxx8c1m", "jtype": "item_ref",
      "space": ".predef"}, 0, 4294967295]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_50t6rk9yjpk_qv9jr1c5hfh','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1830}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
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
INSERT INTO t_items VALUES('_550t6pz1jt3_s87h5uvaifv','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 624}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
      "_6jyxveszh5w_28ruvjudjiw", "jtype": "item_ref", "space": ".root"},
     0, 4294967295]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_55vhwzi1rmq_5w9fpczxsv3','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1333}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
"jtype": "item_ref", "space": ".predef"}, -128, 127]}]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_57vx8i37ar4_1346w0cep34','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the C bool type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_590trid9ycw_f6kaajwca63','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "primitive to make an item"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_3j3s2e0510a_096chqpijq7"]}},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": [{"item_ref": "_3j3s2e0510a_096chqpijq7", "jtype": "item_ref",
      "space": ".predef"}, " = (momval_t) mom_make_item()"]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5a4xtuk8i56_2u3zth4hwae','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2622}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_6u8mxz2694e_chvs7zhz76x", "sons": [{"item_ref": "_7qf542wffvf_70mp29p178v",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref",
"space": ".predef"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref":
    "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5c0uvw732ra_eu0u137fimi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC method to define some type"},
  {"attr": "_0hpzi8m7wym_1y4ypmm9y47", "val": {"item_ref": "_413ff3d1ajz_0f0vvj7m1e3",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": null,
 "payload": null}
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
INSERT INTO t_items VALUES('_5hsdpmacvqa_0mciep57uq6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1124}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_87axj4q44z5_xddqmrkw875", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5hxitvwqm6k_w9u892ah0i7','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1002}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9y82yz8z01a_x0dx4kx0x18",
      "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0", "jtype": "item_ref",
"space": ".root"}]}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5i5s2hp2yua_e8uryu4yw6c','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1223}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6zwuvxcfhhw_fdkwjd6mjwq",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_5jsey8w8cfz_h1hqcq0d5sd','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 562}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_1jriw29kezf_4wx1rtck86x",
"jtype": "item_ref", "space": ".root"}, 0, 255]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
"jtype": "item_ref", "space": ".predef"}, -128, 127]}]}}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5kjhr97p5kk_kdq3zwkvwx4','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1831}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5pqcxpi6u8p_kj5sykfd2zy','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2620}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5qv1t9k1psk_q2tzcydk0i7','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 773}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_34jvm2m8mam_imy67jedrhs",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}]}]}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
    "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5rcj09rsct4_dz814x5hvps','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1738}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref",
      "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref":
    "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5rhi4wffs94_fejzfvq6d11','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2255}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_0z10v92cc0m_ydhtvzrujjz",
      "sons": [{"item_ref": "_7m7zirztmat_f97uf1cs0ys", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_5ruw4vm56dh_jfjm70uqzdc','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 776}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_5s59qeamxta_70k0mt77r9i','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the size, e.g. in editors"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5sw59dauckp_8eustjwf58u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to be used inside display items to give the origin"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5ucepmx82y3_56j67dkfif5','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1546}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5ur74wyu66i_61t0h33dm1m','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1171}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_6jyxveszh5w_28ruvjudjiw",
"jtype": "item_ref", "space": ".root"}, 0, 4294967295]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5v6t2ciu9ru_74uh3hsj7ut','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1261}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_5v7duzmxp55_rz409rqeyki','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1898}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
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
INSERT INTO t_items VALUES('_5zzuk12vu67_4ti500vktvf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1887}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_2x2u9vdare0_5dj5y8zt6ww", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_60ist2ad22c_cfpjp5ay6uj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "third result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_61arma4wcjy_4um1arwy09m','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes unsigned char type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_626yhtpekvc_eyqv9eyzmkf','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_62wa0xphufr_h91i8z2mm2x','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1536}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_63a4vrwsivd_xz25q0jk17d','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 485}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_64tmzjxwipa_fsdreiukaxw','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1302}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_651tc5kk57y_33ey8kje63e','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2625}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_65961crktpj_vtt30qeqv21','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "first module, should become able to translate itself."},
  {"attr": "_5yfdp53cpi1_0i5k33wms7c", "val": {"jtype": "set", "set": ["_0afqepa7jkr_qky26hpv98d",
     "_4yxdswc8qwf_vxzy95hd399"]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_66sez2vrdz5_wmm1tkw9976','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1550}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
      "_24i91kxrvwm_10cmfd12dck", "jtype": "item_ref", "space": ".predef"},
     9223372036854775807, 9223372036854775807]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_66xrheq8zyk_q1z7866a9pp','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC method to define some field"},
  {"attr": "_0hpzi8m7wym_1y4ypmm9y47", "val": {"item_ref": "_1ij3y9cmvk5_8tkiqiy17ws",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6c9cs2mx512_exp6t0yzhys','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1329}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6d4dwqa6m09_c5vtjswfpfi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC routine to declare some name"}],
 "content": null, "kind": "closure", "payload": {"closed_values": ["json_rpc_meltmom_declare_name spare 0",
   "json_rpc_meltmom_declare_name spare 1", "json_rpc_meltmom_declare_name spare 2"],
  "closure_routine": "json_rpc_meltmom_declare_name"}}
');
INSERT INTO t_items VALUES('_6djzuwz5pav_cri386ywjhj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "notably for error message in JSONRPC"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6e18yr2yyj5_a00up23ijwi','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1126}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6e5mdevhq6q_s5y5p7q3rjw','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2555}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
"sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"},
 -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_57vx8i37ar4_1346w0cep34",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6f9870y6v8t_kp8fcmq2ezv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the state of Json for frames of tasklets."},
  {"attr": "_0h331ch957p_j6a8i7v4e6y", "val": {"item_ref": "_6p6v25323aq_97d9ude6j12",
    "jtype": "item_ref", "space": ".root"}}, {"attr": "_0hpzi8m7wym_1y4ypmm9y47",
   "val": {"item_ref": "_6p6v25323aq_97d9ude6j12", "jtype": "item_ref", "space": ".root"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6fiizwecy7v_pxd0wxx8c1m','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "type for 32 bits unsigned"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6hf2vzmrsee_t35suhjvtj4','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item reference, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6jyxveszh5w_28ruvjudjiw','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 187}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6me6iv1ieat_mw0zucsqqm5','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for maximum"}], "content":
 null, "kind": null, "payload": null}
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
INSERT INTO t_items VALUES('_6p6v25323aq_97d9ude6j12','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives thru JSONRPC the state of monimelt"}],
 "content": null, "kind": "closure", "payload": {"closed_values": ["{spare closed-value json-rpc-status-0}",
   "{spare closed-value json-rpc-status-1}"], "closure_routine": "json_rpc_status"}}
');
INSERT INTO t_items VALUES('_6p8um0xyf9c_171y0d2e40a','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 186}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6peadks7j6e_jpj05ykzxet','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2594}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_0mwp3hwhqt3_48vf8kavu1q",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6pqzy8r1siz_dv8dqhqadud','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2275}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_0z10v92cc0m_ydhtvzrujjz",
      "sons": [{"item_ref": "_7m7zirztmat_f97uf1cs0ys", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6qcw93kypcv_0iiepqtk73j','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "high bound"}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6qp266amrz7_izi5rx6ukuk','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6qw6i838e83_7tjmqp3f5tr','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1839}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi", "jtype": "item_ref",
      "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi", "jtype": "item_ref",
      "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype":
    "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6tra7xc8iww_0rwpua90935','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1530}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}, {"jtype": "node", "node": "_0z10v92cc0m_ydhtvzrujjz",
      "sons": [{"item_ref": "_7tfqkjxktup_1t1a8csf66a", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_6u8mxz2694e_chvs7zhz76x','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for function type in C"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6ump1mcjjzi_jzsmdc52ckm','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 156}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
"jtype": "item_ref", "space": ".predef"}]}, {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
      "sons": [{"item_ref": "_8eydfzivw1p_4hss3rfff4y", "jtype": "item_ref",
"space": ".predef"}, 0, 9223372036854775807]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}, {"item_ref": "_89ejvxupprm_f219pqwz13s", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref",
      "space": ".predef"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6up2yfutdp6_e1zp422hjat','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1885}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
    "sons": [{"item_ref": "_2x2u9vdare0_5dj5y8zt6ww", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6vpjccw3f7e_x5d6qrdteuk','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1551}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_89ejvxupprm_f219pqwz13s", "jtype": "item_ref", "space": ".predef"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6w3dvx83dfw_xzc6aif6isv','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the attribute[s], at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6x2wxwzakid_dh3cphz4ch0','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_6zm92afs4yc_60a8ujmi1ef','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine for JSONRPC dump & exit"}],
 "content": null, "kind": "closure", "payload": {"closed_values": ["{spare closed-value json-rpc-dump-exit-0}",
   "{spare closed-value json-rpc-dump-exit-1}"], "closure_routine": "json_rpc_dump_exit"}}
');
INSERT INTO t_items VALUES('_6zwuvxcfhhw_fdkwjd6mjwq','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_705c10s4fte_27kjcrw2rxr','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 510}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70aer7teeui_kvzkiqq2rd2','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "current procedure, etc..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_70k93z1hdzc_jy75actscqt','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1255}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}}],
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
INSERT INTO t_items VALUES('_71way4djcmw_csfzeuthhk7','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 992}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_72jta9t9709_pptutv772qi','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2553}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
"sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"},
 -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_72psq1j5keh_d0z70e7tzxu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes char type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_73c7kw1h9pe_04qr4vpyref','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 547}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_73im2zryfij_a7zmkketcfc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to edit a value during edition in ajax_objects"}],
 "content": null, "kind": "routine", "payload": "edit_value"}
');
INSERT INTO t_items VALUES('_747s72xs8ia_ypcz5ru04k5','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 310}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_626yhtpekvc_eyqv9eyzmkf",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_74wq08fxk3d_73rcjemj9c9','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1680}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref",
"space": ".predef"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref":
    "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_75fi7pj3dxj_ewpadt0mi62','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1829}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_77xdevxp3zp_yeryy3x1aea','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes unsigned type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_78smjp9q3d8_sjt628xxm61','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2416}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
"sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"},
 -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_79d3xfh2uxj_9carvpjiivq','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1547}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_79vm7uxit6c_53qt1qi2wuj','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "low bound"}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7a9sxskjhcp_kpf30ka97ex','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for JSONRPC and elsewhere"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7dekms4ck3h_e7c1xm3p74c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "give result type of a Monimelt function"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7diyc1cwj8z_x630afccr8e','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Group together all noticed values in dump outcome."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7dmk5xffeys_iuh2t5z3a1m','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 544}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
      "_6jyxveszh5w_28ruvjudjiw", "jtype": "item_ref", "space": ".root"},
     0, 4294967295]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7e1ak9qwf9h_wp5tptm4uyi','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes int type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7ex85peztf2_2pjqfw8s3wi','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1895}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_7fthpma881d_sz1p2pvd518','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1327}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7fwerdk2440_xzwtukysz1h','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1701}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7ijk39sk4xp_xxpihysxrp0','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1690}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi", "jtype": "item_ref", "space": ".predef"}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_7jzvaihqxfw_0c2y7t976tu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the C void type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7kkh6qiq1vc_e69zp2feuhe','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the rank"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7m7zirztmat_f97uf1cs0ys','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7p5sss87933_imypcwzwdx1','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1249}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_7pp8wmd1x1e_5y3z7km7sx4','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 507}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7pyjxst21ce_vhc0tk0em0u','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "routine to display a value during edition in ajax_objects"}],
 "content": null, "kind": "routine", "payload": "display_value"}
');
INSERT INTO t_items VALUES('_7q3w2656w3m_uvawydu8p7k','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1543}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7qf542wffvf_70mp29p178v','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2621}], "content": null,
 "kind": null, "payload": null}
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
INSERT INTO t_items VALUES('_7scq5tujqpv_itz5521v6cs','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 188}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7sqk8vh89xr_6tj8dq7vqju','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "module to be compiled..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7tfqkjxktup_1t1a8csf66a','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7tz4p0m0ruv_8aavqevrrp0','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1259}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7urjeiw3evy_m7k72uv6790','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, give the type of a value"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_7vdyjv5ff4q_1uf7uhs8h7m','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1854}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
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
INSERT INTO t_items VALUES('_812uff8y1ms_rh196jde36k','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2368}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_81aqakatxk9_t51yy21uiw5','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 771}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_34jvm2m8mam_imy67jedrhs",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_835qk04icz0_e6srx044eqq','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 509}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_83p6a79fewy_z0hvpk042sz','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 838}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_705c10s4fte_27kjcrw2rxr", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_83vmhzt7vum_rri3v062yc8','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 511}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_842ec6pjy69_6ecqi2w8dv6','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_849cwhw4kch_25auqas037a','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 923}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_57vx8i37ar4_1346w0cep34",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
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
INSERT INTO t_items VALUES('_862u2w97uth_dvy6mmywjyq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for standard FILE"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_866txyivqvm_t3uv5pca91y','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1335}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_19fei8jwqtj_0mqfvi8s172",
"jtype": "item_ref", "space": ".root"}]}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_86ft82euar7_cm50jcthhwe','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "start an associative item"},
  {"attr": "_8um1q4shitk_tpcmedvsfzu", "val": {"jtype": "tuple", "tuple": ["_53748kde7s1_pkz810exr27"]}},
  {"attr": "_967fch1xu4h_i87qjq1zt1h", "val": {"jtype": "node", "node": "_2vxxtir316j_meap5sq6ykr",
    "sons": ["mom_item_start_assoc((momitem_t*)", {"item_ref": "_53748kde7s1_pkz810exr27",
      "jtype": "item_ref", "space": ".predef"}, ")"]}}, {"attr": "_70ty9z1tm4p_eccsxmyfe25",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_877fh31zk63_2jkwam13i0h','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for 64 bits int"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_87axj4q44z5_xddqmrkw875','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 513}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_87txzmmu09p_a74dcdv01uh','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2624}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_8907u3qf13t_52v4s8uc60r','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2562}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
    "sons": [{"item_ref": "_0yavvxi653k_e1ui813cih8", "jtype": "item_ref",
      "space": ".predef"}, 0, 9223372036854775807]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_89ejvxupprm_f219pqwz13s','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for double values and displays"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_89jhda3v3wj_hvzvs5w6hui','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "give arguments of a Monimelt function"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_89su1uthsk0_zs7yiqr2ir4','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1539}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8a0ufiqavhz_rdi6iaa3cam','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 587}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_877fh31zk63_2jkwam13i0h",
"jtype": "item_ref", "space": ".predef"}, 9223372036854775807, 9223372036854775807]}]}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
    "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
      "space": ".root"}]}}], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8ctu5dah9yj_0cem5haspqz','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1542}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8eydfzivw1p_4hss3rfff4y','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes size_t type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8hvs0h5a7v9_1ixv3jf2j6j','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the line number"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8irjvyepyi7_86k7f5aek7k','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC routine to define some function"}],
 "content": null, "kind": "closure", "payload": {"closed_values": ["json_rpc_meltmom_define_function spare 0",
   "json_rpc_meltmom_define_function spare 1", "json_rpc_meltmom_define_function spare 2"],
  "closure_routine": "json_rpc_meltmom_define_function"}}
');
INSERT INTO t_items VALUES('_8j516kuv89j_4hc4w6ykmr6','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "in JSON dumps, used for long chunked strings"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8m10x56wqmm_8zi1ijvqkya','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1698}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_8m84uy7v7zv_ps04h13q61u','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1855}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_8mu6t9f63zq_i5kcq3pctjj','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 769}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_34jvm2m8mam_imy67jedrhs",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8mwycdzxapu_x586t0atqjh','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1692}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8p7vfxvyy4x_uvsm9yxvch5','{"attr": [], "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8s2dqe9qc05_k59cqpuh1wm','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1635}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8s357rq2dzk_k8ze95tikjm','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "gives the item content, at least in dumped JSON..."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8s4wcve2u49_252vwyzyrxd','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "to be used inside display items for null"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8t2i77157uk_86ms9ue1p2x','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2592}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_0mwp3hwhqt3_48vf8kavu1q",
  "jtype": "item_ref", "space": ".root"}]}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_8u5ar84utwm_99k5mq2d589','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "second result in tasklet"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8um1q4shitk_tpcmedvsfzu','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "arguments of a routine"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8vj9h91xm9h_7a16r3wayq0','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1912}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_87axj4q44z5_xddqmrkw875",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_8vkj288fipd_djv0q33vwxx','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1736}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref",
"space": ".predef"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref":
    "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8xfj0sc82ux_thsk0iw2773','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for types"}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8y1sw8z084j_4ts0y0jydha','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "value containing the translation"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_8ycesefrfu9_au4qeqk68jk','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 778}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref":
  "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"}, -128,
 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_8zi3kizdedc_uywiajiewid','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1893}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"jtype": "node",
    "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
      "jtype": "item_ref", "space": ".root"}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_91pketvc5pz_wq0v0wpauw8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "Gives the values of Json for frames of tasklets."}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_91y0xjj7s7k_4p5aavisea9','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 987}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_05utk0hrpcw_usi08r18z82",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_92cvj51j4ws_wwddihuzyj7','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 506}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_92ti1wff0e6_hw240mrry10','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "describes long type"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_938riht0qyi_hp7wh9ys3ur','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2028}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_9arrtc64f96_z86ukm7u5sf", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_93s0y61hi9e_zvm47r6yhyc','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2628}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": []}},
  {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_94jks4a7y80_cuwu4t0ytjj','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 194}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_95jrd5tjcsu_41chfihe6za','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "type for va_list in C variadic functions"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_967fch1xu4h_i87qjq1zt1h','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for the expansion of a primitive"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_96x9fqiw2qa_65hxuf4xp0x','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1836}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0",
      "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_97rr8dy09zw_zfzt4yc5ssq','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1843}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref":
      "_42cx8xskxe3_vu23eezcksq", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_97zkxf62r11_6eedwwv3eu8','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure payload"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_984m70p3jfc_2385qzu6x15','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 263}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9arrtc64f96_z86ukm7u5sf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 514}], "content": null,
 "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9asdk3kditj_4y9tyc48mr6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 982}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_05utk0hrpcw_usi08r18z82",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9dcxaqk8tqe_fam9mcxme9w','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "the routines in a module"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9dsak0qcy0v_1c5z9th7x3i','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "indicates the HTTP GET method"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9e6ecm3ykps_ip69cm3q129','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1013}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_2x2u9vdare0_5dj5y8zt6ww", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9hrcziczpvp_7yw63a21ev8','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1859}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61",
      "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x", "jtype": "item_ref",
"space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c", "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu",
    "jtype": "item_ref", "space": ".predef"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_9j151ceapqf_dae1vrdueqa','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for Monimelt types"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9jeymqk2732_wiq5kyczi9c','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for integer values and displays"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9jv516up4ee_20as3ydvwjp','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 836}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"item_ref":
      "_77xdevxp3zp_yeryy3x1aea", "jtype": "item_ref", "space": ".predef"},
     {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_2qmau9u6ie1_7ry5vh1vhe0", "sons": [{"item_ref": "_42cx8xskxe3_vu23eezcksq",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_705c10s4fte_27kjcrw2rxr", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9m5hjhjuyfr_a6iiw2uu5jj','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2023}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_21xtrp170v1_17c7pk1rh4r", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_9arrtc64f96_z86ukm7u5sf", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9mxi9e605ay_ihpjyrwq250','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSON for closure values"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9sd1mh9q1zf_3duewi6fsaq','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for exited processes with exit code >0"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9u0043iaaef_mdrw04fihi6','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 767}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_34jvm2m8mam_imy67jedrhs",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9w63yccr7ra_7vj115zs4d8','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1009}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0",
  "jtype": "item_ref", "space": ".root"}]}]}, {"item_ref": "_77xdevxp3zp_yeryy3x1aea",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_6x2wxwzakid_dh3cphz4ch0",
"jtype": "item_ref", "space": ".root"}]}]}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_9wuqw5cmxxq_6f6844dd0vs','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 1549}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref": "_28jrsus3ti3_da1ztz7ex3x",
"jtype": "item_ref", "space": ".root"}]}, {"item_ref": "_7e1ak9qwf9h_wp5tptm4uyi",
      "jtype": "item_ref", "space": ".predef"}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
      "node", "node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_24i91kxrvwm_10cmfd12dck",
"jtype": "item_ref", "space": ".predef"}, 9223372036854775807, 9223372036854775807]}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9wwqwxqcm4p_y7di7fs8tsk','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "common index number"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9x9ue95fzi5_6fqmyywe9yc','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "JSONRPC method to define some function"},
  {"attr": "_0hpzi8m7wym_1y4ypmm9y47", "val": {"item_ref": "_8irjvyepyi7_86k7f5aek7k",
    "jtype": "item_ref", "space": ".root"}}], "content": null, "kind": null,
 "payload": null}
');
INSERT INTO t_items VALUES('_9xv11saaxcd_3w8s6fmxfxr','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 626}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"item_ref":
      "_05utk0hrpcw_usi08r18z82", "jtype": "item_ref", "space": ".root"}]}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9y2ss9s9q73_c31m29ckxuf','{"attr": [{"attr": "_8hvs0h5a7v9_1ixv3jf2j6j", "val": 2586}, {"attr": "_89jhda3v3wj_hvzvs5w6hui",
   "val": {"jtype": "node", "node": "_89jhda3v3wj_hvzvs5w6hui", "sons": [{"jtype":
      "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node",
"node": "_9jeymqk2732_wiq5kyczi9c", "sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu",
  "jtype": "item_ref", "space": ".predef"}, -128, 127]}]}, {"jtype": "node",
      "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype": "node", "node": "_9jeymqk2732_wiq5kyczi9c",
"sons": [{"item_ref": "_72psq1j5keh_d0z70e7tzxu", "jtype": "item_ref", "space": ".predef"},
 -128, 127]}]}, {"jtype": "node", "node": "_1vd2t9p4krp_yjsuuc88k61", "sons": [{"jtype":
"node", "node": "_9y82yz8z01a_x0dx4kx0x18", "sons": [{"item_ref": "_1k0aik92ucm_zrq3tuk876s",
  "jtype": "item_ref", "space": ".root"}]}]}]}}, {"attr": "_7dekms4ck3h_e7c1xm3p74c",
   "val": {"item_ref": "_7jzvaihqxfw_0c2y7t976tu", "jtype": "item_ref", "space": ".predef"}}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_items VALUES('_9y82yz8z01a_x0dx4kx0x18','{"attr": [{"attr": "_41u1utcxyek_22cftxt3xxm", "val": "for monimelt struct prefixed with mom"}],
 "content": null, "kind": null, "payload": null}
');
INSERT INTO t_names VALUES('FILE','_862u2w97uth_dvy6mmywjyq','.predef');
INSERT INTO t_names VALUES('GET','_9dsak0qcy0v_1c5z9th7x3i','.predef');
INSERT INTO t_names VALUES('HEAD','_47fatww79x6_vh8ap22c0ch','.predef');
INSERT INTO t_names VALUES('POST','_5wmusj136kq_u5qpehp89aq','.predef');
INSERT INTO t_names VALUES('agenda','_02u53qxa7dm_sttmhffpchr','.predef');
INSERT INTO t_names VALUES('ajax_complete_name','_4v93t3jzrtz_srt9ear8fm8','.root');
INSERT INTO t_names VALUES('ajax_edit','_35a1p2kdx9h_ap5pe704vtr','.root');
INSERT INTO t_names VALUES('ajax_objects','_6mwwr0i4y9p_5aupdxjxdk1','.root');
INSERT INTO t_names VALUES('ajax_system','_3xz3qrc6mfy_4r51up6u3pa','.root');
INSERT INTO t_names VALUES('arguments','_8um1q4shitk_tpcmedvsfzu','.predef');
INSERT INTO t_names VALUES('assign','_0zmdkdxj7kp_491yqpcuaz8','.predef');
INSERT INTO t_names VALUES('assoc_get','_70mt4fvrva2_pk76eevwada','.root');
INSERT INTO t_names VALUES('assoc_put','_5c5jh9185sv_qru5amf9v18','.root');
INSERT INTO t_names VALUES('attr','_6w3dvx83dfw_xzc6aif6isv','.predef');
INSERT INTO t_names VALUES('block','_2a1fq7ks2ak_wvxk68qe4mv','.predef');
INSERT INTO t_names VALUES('body','_0ihu411vkua_z4sh56hicdt','.predef');
INSERT INTO t_names VALUES('bool','_57vx8i37ar4_1346w0cep34','.predef');
INSERT INTO t_names VALUES('buffer','_3zqd7ai3rtu_md9athkx17u','.predef');
INSERT INTO t_names VALUES('call_at_state','_3dqr46p2xf4_29kf5vdtw4z','.root');
INSERT INTO t_names VALUES('char','_72psq1j5keh_d0z70e7tzxu','.predef');
INSERT INTO t_names VALUES('closed_values','_9mxi9e605ay_ihpjyrwq250','.predef');
INSERT INTO t_names VALUES('closure','_97zkxf62r11_6eedwwv3eu8','.predef');
INSERT INTO t_names VALUES('closure_routine','_28941cvehx8_9rf4udyeq8v','.predef');
INSERT INTO t_names VALUES('code','_0yyp8vmw4si_wf49m4d4zwq','.predef');
INSERT INTO t_names VALUES('comment','_41u1utcxyek_22cftxt3xxm','.predef');
INSERT INTO t_names VALUES('content','_8s357rq2dzk_k8ze95tikjm','.predef');
INSERT INTO t_names VALUES('display','_085krqf192t_z1m3zs77ww5','.predef');
INSERT INTO t_names VALUES('display_value','_7pyjxst21ce_vhc0tk0em0u','.root');
INSERT INTO t_names VALUES('double','_89ejvxupprm_f219pqwz13s','.predef');
INSERT INTO t_names VALUES('doubles','_17spwr8dkzv_tsf2s8diazu','.predef');
INSERT INTO t_names VALUES('dump','_24w2ce2eq1z_pddi9j2czci','.root');
INSERT INTO t_names VALUES('edit_value','_73im2zryfij_a7zmkketcfc','.root');
INSERT INTO t_names VALUES('editor','_38s7ihasu0m_xzipyerxm3j','.predef');
INSERT INTO t_names VALUES('editors','_7qk90k9vx0u_31ivff77td7','.root');
INSERT INTO t_names VALUES('empty','_8s4wcve2u49_252vwyzyrxd','.predef');
INSERT INTO t_names VALUES('error','_4qcw2mwjswm_j9q0k9d04hm','.predef');
INSERT INTO t_names VALUES('exited','_3v4d7uzex6f_euek4pztiuh','.predef');
INSERT INTO t_names VALUES('expansion','_2vxxtir316j_meap5sq6ykr','.predef');
INSERT INTO t_names VALUES('failed','_9sd1mh9q1zf_3duewi6fsaq','.predef');
INSERT INTO t_names VALUES('first_module','_65961crktpj_vtt30qeqv21','.root');
INSERT INTO t_names VALUES('for_each_up_to','_11xee72y1d3_t3cqzi5dq3k','.root');
INSERT INTO t_names VALUES('frames','_4cw8jv45vsk_4mh9ex64904','.predef');
INSERT INTO t_names VALUES('function_type','_6u8mxz2694e_chvs7zhz76x','.predef');
INSERT INTO t_names VALUES('get_attribute','_5vi29c2i54k_i2ufkty9kmp','.root');
INSERT INTO t_names VALUES('gives','_70ty9z1tm4p_eccsxmyfe25','.predef');
INSERT INTO t_names VALUES('high','_6qcw93kypcv_0iiepqtk73j','.root');
INSERT INTO t_names VALUES('id','_7a9sxskjhcp_kpf30ka97ex','.predef');
INSERT INTO t_names VALUES('input','_356014y9ueu_xv6j0eskszw','.predef');
INSERT INTO t_names VALUES('int','_7e1ak9qwf9h_wp5tptm4uyi','.predef');
INSERT INTO t_names VALUES('int32_t','_0i2zv78m8mm_zrzwkmuv9fy','.predef');
INSERT INTO t_names VALUES('int64_t','_877fh31zk63_2jkwam13i0h','.predef');
INSERT INTO t_names VALUES('integer','_9jeymqk2732_wiq5kyczi9c','.predef');
INSERT INTO t_names VALUES('intptr_t','_24i91kxrvwm_10cmfd12dck','.predef');
INSERT INTO t_names VALUES('item','_53748kde7s1_pkz810exr27','.predef');
INSERT INTO t_names VALUES('item_ref','_6hf2vzmrsee_t35suhjvtj4','.predef');
INSERT INTO t_names VALUES('ix','_9wwqwxqcm4p_y7di7fs8tsk','.root');
INSERT INTO t_names VALUES('jit','_24yt56xf3d5_4w80i326kjz','.predef');
INSERT INTO t_names VALUES('json_array','_35vp60aw7em_d436vfie4ud','.predef');
INSERT INTO t_names VALUES('json_false','_4mha85xcfwi_9zqcvkiy3dk','.predef');
INSERT INTO t_names VALUES('json_object','_3xpyd539p4m_23h7wi59xi9','.predef');
INSERT INTO t_names VALUES('json_rpc_dump_exit','_6zm92afs4yc_60a8ujmi1ef','.root');
INSERT INTO t_names VALUES('json_rpc_meltmom_declare_name','_6d4dwqa6m09_c5vtjswfpfi','.root');
INSERT INTO t_names VALUES('json_rpc_meltmom_define_field','_1ij3y9cmvk5_8tkiqiy17ws','.root');
INSERT INTO t_names VALUES('json_rpc_meltmom_define_function','_8irjvyepyi7_86k7f5aek7k','.root');
INSERT INTO t_names VALUES('json_rpc_meltmom_define_type','_413ff3d1ajz_0f0vvj7m1e3','.root');
INSERT INTO t_names VALUES('json_rpc_status','_6p6v25323aq_97d9ude6j12','.root');
INSERT INTO t_names VALUES('json_true','_2vmrrvq5kdk_9um63pstcu9','.predef');
INSERT INTO t_names VALUES('jsonrpc','_0h331ch957p_j6a8i7v4e6y','.predef');
INSERT INTO t_names VALUES('jsonrpc_handler','_0hpzi8m7wym_1y4ypmm9y47','.predef');
INSERT INTO t_names VALUES('jtype','_7urjeiw3evy_m7k72uv6790','.predef');
INSERT INTO t_names VALUES('kind','_06yp8ueq6yf_5ts408yww29','.predef');
INSERT INTO t_names VALUES('len','_456hz6qd6x2_jyy24w6q84z','.root');
INSERT INTO t_names VALUES('locals','_3eu0rdq4upj_dp5ptr6hj04','.predef');
INSERT INTO t_names VALUES('long','_92ti1wff0e6_hw240mrry10','.predef');
INSERT INTO t_names VALUES('low','_79vm7uxit6c_53qt1qi2wuj','.root');
INSERT INTO t_names VALUES('make_item','_590trid9ycw_f6kaajwca63','.root');
INSERT INTO t_names VALUES('max','_6me6iv1ieat_mw0zucsqqm5','.predef');
INSERT INTO t_names VALUES('meltmom_declare_name','_1y91p3us9s9_p88di58xuek','.root');
INSERT INTO t_names VALUES('meltmom_define_field','_66xrheq8zyk_q1z7866a9pp','.root');
INSERT INTO t_names VALUES('meltmom_define_function','_9x9ue95fzi5_6fqmyywe9yc','.root');
INSERT INTO t_names VALUES('meltmom_define_type','_5c0uvw732ra_eu0u137fimi','.root');
INSERT INTO t_names VALUES('message','_4jp2meuzru2_a58afyxwxa2','.predef');
INSERT INTO t_names VALUES('method','_3hv5ymapjed_y8q6hsvhw8u','.predef');
INSERT INTO t_names VALUES('min','_2fjizx76kia_567e7h9s69z','.predef');
INSERT INTO t_names VALUES('misc','_85rz4j0q982_67im8sstj9s','.root');
INSERT INTO t_names VALUES('module','_7sqk8vh89xr_6tj8dq7vqju','.predef');
INSERT INTO t_names VALUES('module_routines','_9dcxaqk8tqe_fam9mcxme9w','.predef');
INSERT INTO t_names VALUES('mom_add_tasklet_to_agenda_back','_27k36a8racw_as8j0p646ia','.root');
INSERT INTO t_names VALUES('mom_add_tasklet_to_agenda_front','_5pqcxpi6u8p_kj5sykfd2zy','.root');
INSERT INTO t_names VALUES('mom_alpha_ordered_tuple_of_named_items','_5hsdpmacvqa_0mciep57uq6','.root');
INSERT INTO t_names VALUES('mom_close_json_parser','_81aqakatxk9_t51yy21uiw5','.root');
INSERT INTO t_names VALUES('mom_continue_working','_651tc5kk57y_33ey8kje63e','.root');
INSERT INTO t_names VALUES('mom_cstring_hash','_550t6pz1jt3_s87h5uvaifv','.root');
INSERT INTO t_names VALUES('mom_debug_at','_5rhi4wffs94_fejzfvq6d11','.root');
INSERT INTO t_names VALUES('mom_debug_en','_7m7zirztmat_f97uf1cs0ys','.root');
INSERT INTO t_names VALUES('mom_debugprintf_at','_6pqzy8r1siz_dv8dqhqadud','.root');
INSERT INTO t_names VALUES('mom_dump_add_scanned_item','_3h0cu9jtev8_fi1tqhqtpk4','.root');
INSERT INTO t_names VALUES('mom_dump_emit_json','_1tkmhtxtu7f_y3h9kp9jvje','.root');
INSERT INTO t_names VALUES('mom_dump_notice','_8t2i77157uk_86ms9ue1p2x','.root');
INSERT INTO t_names VALUES('mom_dump_require_module','_4a62sfh041d_ews4tzh4apy','.root');
INSERT INTO t_names VALUES('mom_dump_scan_value','_6peadks7j6e_jpj05ykzxet','.root');
INSERT INTO t_names VALUES('mom_dumper_st','_0mwp3hwhqt3_48vf8kavu1q','.root');
INSERT INTO t_names VALUES('mom_dumpoutcome_st','_1k0aik92ucm_zrq3tuk876s','.root');
INSERT INTO t_names VALUES('mom_elapsed_real_time','_1mv28mq6p9s_aairsjxey8i','.root');
INSERT INTO t_names VALUES('mom_emit_short_item_json','_350vk8y5jau_wscjm4tmxfr','.root');
INSERT INTO t_names VALUES('mom_end_json_parser','_8mu6t9f63zq_i5kcq3pctjj','.root');
INSERT INTO t_names VALUES('mom_enum','_0z10v92cc0m_ydhtvzrujjz','.predef');
INSERT INTO t_names VALUES('mom_fatal_at','_812uff8y1ms_rh196jde36k','.root');
INSERT INTO t_names VALUES('mom_fataprintf_at','_1dmki2tps3h_fzcs0sj24m9','.root');
INSERT INTO t_names VALUES('mom_finalize_buffer_output','_747s72xs8ia_ypcz5ru04k5','.root');
INSERT INTO t_names VALUES('mom_forget_name','_1he6qfkkr8e_8x22eu10cus','.root');
INSERT INTO t_names VALUES('mom_full_dump','_9y2ss9s9q73_c31m29ckxuf','.root');
INSERT INTO t_names VALUES('mom_generate_c_module','_1ky66221jjr_atfxmj1zi16','.root');
INSERT INTO t_names VALUES('mom_get_item_bool','_23kika6z9qv_7uj6hwy5qdf','.root');
INSERT INTO t_names VALUES('mom_get_item_of_ident','_91y0xjj7s7k_4p5aavisea9','.root');
INSERT INTO t_names VALUES('mom_get_item_of_identcstr','_0cte3uqc744_m1674pufa83','.root');
INSERT INTO t_names VALUES('mom_get_item_of_name_hash','_1upqvx9m5si_pe93kw913pf','.root');
INSERT INTO t_names VALUES('mom_get_item_of_name_or_ident_cstr_hash','_5ur74wyu66i_61t0h33dm1m','.root');
INSERT INTO t_names VALUES('mom_inform_at','_4dexizaxjta_1vvp4hchyu6','.root');
INSERT INTO t_names VALUES('mom_informprintf_at','_3qf9kmsmfyx_9w5fsqa2tzm','.root');
INSERT INTO t_names VALUES('mom_initial_load','_2m5kpyivq0q_dp274hd64ur','.root');
INSERT INTO t_names VALUES('mom_initialize_buffer_output','_3k72ktm822u_8exaz46p3y9','.root');
INSERT INTO t_names VALUES('mom_initialize_json_parser','_41ciqzka19m_wjq6xe7u28h','.root');
INSERT INTO t_names VALUES('mom_initialize_signals','_93s0y61hi9e_zvm47r6yhyc','.root');
INSERT INTO t_names VALUES('mom_item_assoc_get','_8m84uy7v7zv_ps04h13q61u','.root');
INSERT INTO t_names VALUES('mom_item_assoc_put','_0pcyvpuhi7w_3965ke5dwyf','.root');
INSERT INTO t_names VALUES('mom_item_assoc_remove','_9hrcziczpvp_7yw63a21ev8','.root');
INSERT INTO t_names VALUES('mom_item_assoc_reserve','_7vdyjv5ff4q_1uf7uhs8h7m','.root');
INSERT INTO t_names VALUES('mom_item_assoc_set_attrs','_2w8v4htahij_su1zqvfuyd6','.root');
INSERT INTO t_names VALUES('mom_item_buffer_out','_4wd221340d0_9y42sarswa2','.root');
INSERT INTO t_names VALUES('mom_item_clear_payload','_4z5ma494cak_8711cf2mtuc','.root');
INSERT INTO t_names VALUES('mom_item_closure_length','_28qsshh7hwr_k2jdce4atqt','.root');
INSERT INTO t_names VALUES('mom_item_closure_nth','_7fthpma881d_sz1p2pvd518','.root');
INSERT INTO t_names VALUES('mom_item_closure_routine_name','_55vhwzi1rmq_5w9fpczxsv3','.root');
INSERT INTO t_names VALUES('mom_item_closure_set_nth','_1t5e93sh0uj_fz3qakh4d1i','.root');
INSERT INTO t_names VALUES('mom_item_closure_values','_6c9cs2mx512_exp6t0yzhys','.root');
INSERT INTO t_names VALUES('mom_item_generate_jit_routine','_09c2urv85vr_cj9f93j15z1','.root');
INSERT INTO t_names VALUES('mom_item_get_idstr','_107fvfyjvty_fm8a4q6eika','.root');
INSERT INTO t_names VALUES('mom_item_get_name','_6e18yr2yyj5_a00up23ijwi','.root');
INSERT INTO t_names VALUES('mom_item_get_name_or_idstr','_1hc511xzwc4_3uyk52zy4v5','.root');
INSERT INTO t_names VALUES('mom_item_queue_add_back','_7p5sss87933_imypcwzwdx1','.root');
INSERT INTO t_names VALUES('mom_item_queue_add_front','_12jd44c02kt_aqmepaxvrqc','.root');
INSERT INTO t_names VALUES('mom_item_queue_is_empty','_0mvqzz7radr_zk1errj4eus','.root');
INSERT INTO t_names VALUES('mom_item_queue_length','_70k93z1hdzc_jy75actscqt','.root');
INSERT INTO t_names VALUES('mom_item_queue_peek_back','_7tz4p0m0ruv_8aavqevrrp0','.root');
INSERT INTO t_names VALUES('mom_item_queue_peek_front','_4pjyeaerj5v_j1me6wz2qfp','.root');
INSERT INTO t_names VALUES('mom_item_queue_pop_front','_5v6t2ciu9ru_74uh3hsj7ut','.root');
INSERT INTO t_names VALUES('mom_item_routinedescr','_866txyivqvm_t3uv5pca91y','.root');
INSERT INTO t_names VALUES('mom_item_start_assoc','_02mtzeca0pf_kc9d1i34ap4','.root');
INSERT INTO t_names VALUES('mom_item_start_buffer','_8s2dqe9qc05_k59cqpuh1wm','.root');
INSERT INTO t_names VALUES('mom_item_start_closure_named','_0pijqmm7krh_sxxavschmpi','.root');
INSERT INTO t_names VALUES('mom_item_start_closure_of_routine','_4986vckxzau_i0zcf008zy0','.root');
INSERT INTO t_names VALUES('mom_item_start_queue','_4yf619stz5m_51zzjhtktj0','.root');
INSERT INTO t_names VALUES('mom_item_start_routine','_64tmzjxwipa_fsdreiukaxw','.root');
INSERT INTO t_names VALUES('mom_item_start_tasklet','_0ph6457k18e_jp473ckp3zs','.root');
INSERT INTO t_names VALUES('mom_item_start_vector','_1989w3c3m0k_2mw7tizf5h1','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_depth','_89su1uthsk0_zs7yiqr2ir4','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_closure','_7q3w2656w3m_uvawydu8p7k','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_doubles_pointer','_6vpjccw3f7e_x5d6qrdteuk','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_ints_pointer','_9wuqw5cmxxq_6f6844dd0vs','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_nb_doubles','_5ucepmx82y3_56j67dkfif5','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_nb_ints','_1wjk5me9u9c_y0ksm1sj8j7','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_nb_values','_1akf45cmq0w_pxm7mz4vz4x','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_nth_double','_4qywz13t3m7_jkxp06jp1u8','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_nth_int','_66sez2vrdz5_wmm1tkw9976','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_nth_value','_1ep6tdcx90e_cexqjti73wx','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_state','_8ctu5dah9yj_0cem5haspqz','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_frame_values_pointer','_79d3xfh2uxj_9carvpjiivq','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_pop_frame','_62wa0xphufr_h91i8z2mm2x','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_push_frame','_6tra7xc8iww_0rwpua90935','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_replace_top_frame','_18jez9x90m0_zawc40wpwvz','.root');
INSERT INTO t_names VALUES('mom_item_tasklet_reserve','_0v6se8r3uf4_eqye8sq0r3q','.root');
INSERT INTO t_names VALUES('mom_item_vector_append1','_50t6rk9yjpk_qv9jr1c5hfh','.root');
INSERT INTO t_names VALUES('mom_item_vector_append_from_array','_4fkajpe0f90_vzy4efaffse','.root');
INSERT INTO t_names VALUES('mom_item_vector_append_from_node','_96x9fqiw2qa_65hxuf4xp0x','.root');
INSERT INTO t_names VALUES('mom_item_vector_append_sized','_5kjhr97p5kk_kdq3zwkvwx4','.root');
INSERT INTO t_names VALUES('mom_item_vector_append_til_nil','_2ir1y95wiuk_j879f5utux9','.root');
INSERT INTO t_names VALUES('mom_item_vector_reserve','_75fi7pj3dxj_ewpadt0mi62','.root');
INSERT INTO t_names VALUES('mom_itemattributes_st','_6x2wxwzakid_dh3cphz4ch0','.root');
INSERT INTO t_names VALUES('mom_json_cmp','_5ruw4vm56dh_jfjm70uqzdc','.root');
INSERT INTO t_names VALUES('mom_json_cstr_cmp','_8ycesefrfu9_au4qeqk68jk','.root');
INSERT INTO t_names VALUES('mom_json_parser_data','_9u0043iaaef_mdrw04fihi6','.root');
INSERT INTO t_names VALUES('mom_jsonentry_st','_842ec6pjy69_6ecqi2w8dv6','.root');
INSERT INTO t_names VALUES('mom_jsonob_get_def','_2d28hsmu8zm_fziq009d8q8','.root');
INSERT INTO t_names VALUES('mom_jsonob_getstr','_3k7s4156a7a_1p1rar740pk','.root');
INSERT INTO t_names VALUES('mom_jsonparser_st','_34jvm2m8mam_imy67jedrhs','.root');
INSERT INTO t_names VALUES('mom_jsonrpc_error','_2aeczcr3u9f_cz273p64kkj','.root');
INSERT INTO t_names VALUES('mom_jsonrpc_reply','_5rcj09rsct4_dz814x5hvps','.root');
INSERT INTO t_names VALUES('mom_load_item_json','_5i5s2hp2yua_e8uryu4yw6c','.root');
INSERT INTO t_names VALUES('mom_load_module','_6e5mdevhq6q_s5y5p7q3rjw','.root');
INSERT INTO t_names VALUES('mom_load_plugin','_72jta9t9709_pptutv772qi','.root');
INSERT INTO t_names VALUES('mom_load_value_json','_4wu033p0x2p_6kiar5yv3ws','.root');
INSERT INTO t_names VALUES('mom_loader_st','_6zwuvxcfhhw_fdkwjd6mjwq','.root');
INSERT INTO t_names VALUES('mom_lock_item_at','_849cwhw4kch_25auqas037a','.root');
INSERT INTO t_names VALUES('mom_looks_like_random_id_cstr','_16z19uecsa0_9ppuyipkrdx','.root');
INSERT INTO t_names VALUES('mom_make_double','_1kqu73x9qhf_h0hkrfedk8z','.root');
INSERT INTO t_names VALUES('mom_make_integer','_8a0ufiqavhz_rdi6iaa3cam','.root');
INSERT INTO t_names VALUES('mom_make_item','_4kkhcwtk7e8_e26xhjapp7j','.root');
INSERT INTO t_names VALUES('mom_make_item_of_ident','_9asdk3kditj_4y9tyc48mr6','.root');
INSERT INTO t_names VALUES('mom_make_item_of_identcstr','_2eq3e457rtx_zu76dzrdk98','.root');
INSERT INTO t_names VALUES('mom_make_json_array','_4rjaerz2211_6h0xzvp4v4v','.root');
INSERT INTO t_names VALUES('mom_make_json_array_count','_9jv516up4ee_20as3ydvwjp','.root');
INSERT INTO t_names VALUES('mom_make_json_array_til_nil','_83p6a79fewy_z0hvpk042sz','.root');
INSERT INTO t_names VALUES('mom_make_json_object','_3ru7qrk03jv_v76f8ity5i6','.root');
INSERT INTO t_names VALUES('mom_make_node_from_array','_3jrcmvrfmaw_ypdwhej35cx','.root');
INSERT INTO t_names VALUES('mom_make_node_from_item_vector','_1up5sjxcj8i_cdkus86zxhc','.root');
INSERT INTO t_names VALUES('mom_make_node_from_item_vector_slice','_6qw6i838e83_7tjmqp3f5tr','.root');
INSERT INTO t_names VALUES('mom_make_node_sized','_938riht0qyi_hp7wh9ys3ur','.root');
INSERT INTO t_names VALUES('mom_make_node_til_nil','_9m5hjhjuyfr_a6iiw2uu5jj','.root');
INSERT INTO t_names VALUES('mom_make_random_idstr','_1uc4z4204z5_tjmdyvsmc78','.root');
INSERT INTO t_names VALUES('mom_make_set_from_array','_46uqew6jxj7_m9te8h258md','.root');
INSERT INTO t_names VALUES('mom_make_set_from_item_vector','_0rmhd2s0u3f_z773fur038v','.root');
INSERT INTO t_names VALUES('mom_make_set_from_item_vector_slice','_97rr8dy09zw_zfzt4yc5ssq','.root');
INSERT INTO t_names VALUES('mom_make_set_intersection','_7ex85peztf2_2pjqfw8s3wi','.root');
INSERT INTO t_names VALUES('mom_make_set_sized','_5zzuk12vu67_4ti500vktvf','.root');
INSERT INTO t_names VALUES('mom_make_set_til_nil','_0vz73hu7aph_cyhxumztwzy','.root');
INSERT INTO t_names VALUES('mom_make_set_union','_8zi3kizdedc_uywiajiewid','.root');
INSERT INTO t_names VALUES('mom_make_set_variadic','_6up2yfutdp6_e1zp422hjat','.root');
INSERT INTO t_names VALUES('mom_make_set_without','_5v7duzmxp55_rz409rqeyki','.root');
INSERT INTO t_names VALUES('mom_make_string','_9xv11saaxcd_3w8s6fmxfxr','.root');
INSERT INTO t_names VALUES('mom_make_string_len','_1ttzd8kyed8_15ww8d9c24p','.root');
INSERT INTO t_names VALUES('mom_make_tuple_from_array','_2fvh1ti34mc_sqjvu53d3i5','.root');
INSERT INTO t_names VALUES('mom_make_tuple_from_item_vector','_031ar4875we_s1xarptwct2','.root');
INSERT INTO t_names VALUES('mom_make_tuple_from_item_vector_slice','_4uqfx2kqr91_ha9rqdwhcwe','.root');
INSERT INTO t_names VALUES('mom_make_tuple_from_slice','_47sm0wm5q8f_7kexsx1q63k','.root');
INSERT INTO t_names VALUES('mom_make_tuple_insertion','_8vj9h91xm9h_7a16r3wayq0','.root');
INSERT INTO t_names VALUES('mom_make_tuple_sized','_3r17a3jsysq_8vspi2t8cph','.root');
INSERT INTO t_names VALUES('mom_make_tuple_til_nil','_41r244ahp3q_j09a33vry98','.root');
INSERT INTO t_names VALUES('mom_make_tuple_variadic','_0f6fadaa09t_cwsaijsevaq','.root');
INSERT INTO t_names VALUES('mom_out_at','_0frfrj16j0j_tpz51c5ffrj','.root');
INSERT INTO t_names VALUES('mom_outstring_at','_32dzf3hj8yw_t4yj6mayh8p','.root');
INSERT INTO t_names VALUES('mom_outva_at','_3ei0fm012ue_2aryjhv3v0t','.root');
INSERT INTO t_names VALUES('mom_parse_json','_5qv1t9k1psk_q2tzcydk0i7','.root');
INSERT INTO t_names VALUES('mom_payljsonrpc_finalize','_8vkj288fipd_djv0q33vwxx','.root');
INSERT INTO t_names VALUES('mom_paylwebx_finalize','_74wq08fxk3d_73rcjemj9c9','.root');
INSERT INTO t_names VALUES('mom_plugin_init','_3szfdhp0656_2tar36yeia5','.root');
INSERT INTO t_names VALUES('mom_pushframedirective_en','_7tfqkjxktup_1t1a8csf66a','.root');
INSERT INTO t_names VALUES('mom_put_attribute','_5hxitvwqm6k_w9u892ah0i7','.root');
INSERT INTO t_names VALUES('mom_random_32','_502084k6w6w_f3qsz61zmi9','.root');
INSERT INTO t_names VALUES('mom_random_64','_1mdwwz1zc1k_uui6fjawdxw','.root');
INSERT INTO t_names VALUES('mom_random_nonzero_32','_39xwz6y59rr_muu52mx672m','.root');
INSERT INTO t_names VALUES('mom_random_nonzero_64','_8907u3qf13t_52v4s8uc60r','.root');
INSERT INTO t_names VALUES('mom_register_item_named','_71way4djcmw_csfzeuthhk7','.root');
INSERT INTO t_names VALUES('mom_remove_attribute','_2xv0dpvp5md_6u53m54kw0i','.root');
INSERT INTO t_names VALUES('mom_rename_if_content_changed','_78smjp9q3d8_sjt628xxm61','.root');
INSERT INTO t_names VALUES('mom_reserve_attribute','_9w63yccr7ra_7vj115zs4d8','.root');
INSERT INTO t_names VALUES('mom_routine_sig_t','_11hm4aw0exy_hfhij5sayy3','.root');
INSERT INTO t_names VALUES('mom_run_workers','_3z5f1d6uqxq_29m1cfv36rf','.root');
INSERT INTO t_names VALUES('mom_set_attributes','_9e6ecm3ykps_ip69cm3q129','.root');
INSERT INTO t_names VALUES('mom_set_of_named_items','_4qd5iv2jk0x_t09911fyth3','.root');
INSERT INTO t_names VALUES('mom_start_web','_3ztr5wx3aws_fkquc120ajj','.root');
INSERT INTO t_names VALUES('mom_stop_event_loop','_87txzmmu09p_a74dcdv01uh','.root');
INSERT INTO t_names VALUES('mom_stop_work_with_todo','_5a4xtuk8i56_2u3zth4hwae','.root');
INSERT INTO t_names VALUES('mom_strftime_centi','_6ump1mcjjzi_jzsmdc52ckm','.root');
INSERT INTO t_names VALUES('mom_struct','_9y82yz8z01a_x0dx4kx0x18','.predef');
INSERT INTO t_names VALUES('mom_todoafterstop_fun_t','_7qf542wffvf_70mp29p178v','.root');
INSERT INTO t_names VALUES('mom_type_cstring','_5jsey8w8cfz_h1hqcq0d5sd','.root');
INSERT INTO t_names VALUES('mom_union','_2qmau9u6ie1_7ry5vh1vhe0','.predef');
INSERT INTO t_names VALUES('mom_unlock_item_at','_4qyqy53wv00_vky0cm98ryd','.root');
INSERT INTO t_names VALUES('mom_value_cmp','_73c7kw1h9pe_04qr4vpyref','.root');
INSERT INTO t_names VALUES('mom_value_hash','_7dmk5xffeys_iuh2t5z3a1m','.root');
INSERT INTO t_names VALUES('mom_warning_at','_0pppmw2yfdj_vpumuxt09y3','.root');
INSERT INTO t_names VALUES('mom_warnprintf_at','_4v3d25zvxed_mycxxj2mcjr','.root');
INSERT INTO t_names VALUES('mom_webx_fullpath','_4hvu6yexjt2_w338z9c3dk0','.root');
INSERT INTO t_names VALUES('mom_webx_jsob_post','_8mwycdzxapu_x586t0atqjh','.root');
INSERT INTO t_names VALUES('mom_webx_jsob_query','_23ucm00wpiw_hhyz7px0c73','.root');
INSERT INTO t_names VALUES('mom_webx_method','_7fwerdk2440_xzwtukysz1h','.root');
INSERT INTO t_names VALUES('mom_webx_out_at','_4363z4ey9pa_zvc1w07qctv','.root');
INSERT INTO t_names VALUES('mom_webx_post_arg','_1xvf96xdsje_ttcp13u7rw6','.root');
INSERT INTO t_names VALUES('mom_webx_query_arg','_8m10x56wqmm_8zi1ijvqkya','.root');
INSERT INTO t_names VALUES('mom_webx_reply','_7ijk39sk4xp_xxpihysxrp0','.root');
INSERT INTO t_names VALUES('momdouble_t','_7pp8wmd1x1e_5y3z7km7sx4','.root');
INSERT INTO t_names VALUES('momflags_t','_94jks4a7y80_cuwu4t0ytjj','.root');
INSERT INTO t_names VALUES('momhash_t','_6jyxveszh5w_28ruvjudjiw','.root');
INSERT INTO t_names VALUES('momint_t','_92cvj51j4ws_wwddihuzyj7','.root');
INSERT INTO t_names VALUES('momitem_t','_28jrsus3ti3_da1ztz7ex3x','.root');
INSERT INTO t_names VALUES('momjsonarray_t','_705c10s4fte_27kjcrw2rxr','.root');
INSERT INTO t_names VALUES('momjsonobject_t','_835qk04icz0_e6srx044eqq','.root');
INSERT INTO t_names VALUES('momnode_t','_9arrtc64f96_z86ukm7u5sf','.root');
INSERT INTO t_names VALUES('momout_st','_626yhtpekvc_eyqv9eyzmkf','.root');
INSERT INTO t_names VALUES('momout_t','_984m70p3jfc_2385qzu6x15','.root');
INSERT INTO t_names VALUES('momoutdir_t','_63a4vrwsivd_xz25q0jk17d','.root');
INSERT INTO t_names VALUES('momoutflags_t','_0xv5kv0peuq_iepz5x197q1','.root');
INSERT INTO t_names VALUES('momroutinedescr_st','_19fei8jwqtj_0mqfvi8s172','.root');
INSERT INTO t_names VALUES('momseqitem_t','_83vmhzt7vum_rri3v062yc8','.root');
INSERT INTO t_names VALUES('momset_t','_2x2u9vdare0_5dj5y8zt6ww','.root');
INSERT INTO t_names VALUES('momspaceid_t','_6p8um0xyf9c_171y0d2e40a','.root');
INSERT INTO t_names VALUES('momstring_t','_05utk0hrpcw_usi08r18z82','.root');
INSERT INTO t_names VALUES('momtuple_t','_87axj4q44z5_xddqmrkw875','.root');
INSERT INTO t_names VALUES('momtynum_t','_1jriw29kezf_4wx1rtck86x','.root');
INSERT INTO t_names VALUES('momusize_t','_7scq5tujqpv_itz5521v6cs','.root');
INSERT INTO t_names VALUES('momval_t','_3vvtdeqxssw_dthyjy2dz5t','.root');
INSERT INTO t_names VALUES('momvaltype_t','_266cwehdrjc_144jy18dwh1','.root');
INSERT INTO t_names VALUES('momvalueptr_un','_42cx8xskxe3_vu23eezcksq','.root');
INSERT INTO t_names VALUES('momvflags_t','_1wevxkfudp9_cpeu1adxcrp','.root');
INSERT INTO t_names VALUES('monimelt_arguments','_89jhda3v3wj_hvzvs5w6hui','.predef');
INSERT INTO t_names VALUES('monimelt_line','_8hvs0h5a7v9_1ixv3jf2j6j','.predef');
INSERT INTO t_names VALUES('monimelt_result','_7dekms4ck3h_e7c1xm3p74c','.predef');
INSERT INTO t_names VALUES('monimelt_type','_9j151ceapqf_dae1vrdueqa','.predef');
INSERT INTO t_names VALUES('monimelt_variadic','_21xtrp170v1_17c7pk1rh4r','.predef');
INSERT INTO t_names VALUES('node','_4m7x6811f6j_t480zu575mz','.predef');
INSERT INTO t_names VALUES('noop','_240dwt57s08_a8uy366sev5','.root');
INSERT INTO t_names VALUES('notice','_7diyc1cwj8z_x630afccr8e','.predef');
INSERT INTO t_names VALUES('numbers','_3fw5acswe59_9016fqe4d41','.predef');
INSERT INTO t_names VALUES('origin','_5sw59dauckp_8eustjwf58u','.predef');
INSERT INTO t_names VALUES('params','_4215uc2u6qk_52kqyra86y5','.predef');
INSERT INTO t_names VALUES('parent','_36tp2s8s5s2_jzjm0cxdpjz','.predef');
INSERT INTO t_names VALUES('payload','_41v0erax6my_m6pytj0793u','.predef');
INSERT INTO t_names VALUES('perform','_1t7vvf9p1qm_qy53em9d5p8','.predef');
INSERT INTO t_names VALUES('pointer','_1vd2t9p4krp_yjsuuc88k61','.predef');
INSERT INTO t_names VALUES('primitive','_967fch1xu4h_i87qjq1zt1h','.predef');
INSERT INTO t_names VALUES('proc','_70aer7teeui_kvzkiqq2rd2','.root');
INSERT INTO t_names VALUES('procedures','_5yfdp53cpi1_0i5k33wms7c','.predef');
INSERT INTO t_names VALUES('put_attribute','_547q7emtfsk_ect0yratp6e','.root');
INSERT INTO t_names VALUES('rank','_7kkh6qiq1vc_e69zp2feuhe','.predef');
INSERT INTO t_names VALUES('res1','_3j3s2e0510a_096chqpijq7','.predef');
INSERT INTO t_names VALUES('res2','_8u5ar84utwm_99k5mq2d589','.predef');
INSERT INTO t_names VALUES('res3','_60ist2ad22c_cfpjp5ay6uj','.predef');
INSERT INTO t_names VALUES('result','_6djzuwz5pav_cri386ywjhj','.predef');
INSERT INTO t_names VALUES('routine','_0acmecj244a_6krws4rx7v1','.predef');
INSERT INTO t_names VALUES('seq','_2mayc646pdu_w4d18fmx8u3','.root');
INSERT INTO t_names VALUES('sequence_length','_41xwu6cpvq9_ezp5wzq7t4x','.root');
INSERT INTO t_names VALUES('sequence_nth','_80wxf4c8q92_qq8k6xc0xxj','.root');
INSERT INTO t_names VALUES('set','_2v75mmyph64_4h4kys78740','.predef');
INSERT INTO t_names VALUES('short','_3u5f8v43w3k_acw3tuers2u','.predef');
INSERT INTO t_names VALUES('signed_char','_4ztwemps3vr_y2i1hadm57s','.predef');
INSERT INTO t_names VALUES('size','_5s59qeamxta_70k0mt77r9i','.predef');
INSERT INTO t_names VALUES('size_t','_8eydfzivw1p_4hss3rfff4y','.predef');
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
INSERT INTO t_names VALUES('type','_8xfj0sc82ux_thsk0iw2773','.predef');
INSERT INTO t_names VALUES('uint32_t','_6fiizwecy7v_pxd0wxx8c1m','.predef');
INSERT INTO t_names VALUES('uint64_t','_0yavvxi653k_e1ui813cih8','.predef');
INSERT INTO t_names VALUES('unique_node','_8p7vfxvyy4x_uvsm9yxvch5','.predef');
INSERT INTO t_names VALUES('unsigned','_77xdevxp3zp_yeryy3x1aea','.predef');
INSERT INTO t_names VALUES('unsigned_char','_61arma4wcjy_4um1arwy09m','.predef');
INSERT INTO t_names VALUES('unsigned_long','_23ey21s44px_iarreqeqhr5','.predef');
INSERT INTO t_names VALUES('unsigned_short','_0yeu3tyzw9z_jqhcu2z2x34','.predef');
INSERT INTO t_names VALUES('update_display_value','_1f94j87qumw_mhzkriesx7c','.root');
INSERT INTO t_names VALUES('updated','_5x41iah0kis_x8rrv3ww44t','.predef');
INSERT INTO t_names VALUES('va_list','_95jrd5tjcsu_41chfihe6za','.predef');
INSERT INTO t_names VALUES('val','_7wk9y7e7r0z_575esi8ys5x','.predef');
INSERT INTO t_names VALUES('values','_91pketvc5pz_wq0v0wpauw8','.predef');
INSERT INTO t_names VALUES('var','_2d7i21ihwd8_xjcp4uhs11u','.root');
INSERT INTO t_names VALUES('verbatim','_53cuy70z4tf_86tzz364trd','.predef');
INSERT INTO t_names VALUES('void','_7jzvaihqxfw_0c2y7t976tu','.predef');
INSERT INTO t_names VALUES('web_handler','_7sav6zery1v_24sa6jwwu6c','.predef');
COMMIT;
-- state-monimelt end dump 
