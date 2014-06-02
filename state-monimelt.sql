-- state-monimelt dump 2014 Jun 02

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
INSERT INTO t_items VALUES('_02u53qxa7dm_sttmhffpchr','{"kind": "queue", "payload": [], "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "The agenda is central to Monimelt.\nIt is the queue of taskets to be executed by worker threads."}],
 "content": null}
');
INSERT INTO t_items VALUES('_06yp8ueq6yf_5ts408yww29','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, give the kind of the payload of an item"}], "content": null}
');
INSERT INTO t_items VALUES('_17spwr8dkzv_tsf2s8diazu','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "Gives the double floating-point numbers of Json for frames of tasklets."}],
 "content": null}
');
INSERT INTO t_items VALUES('_240dwt57s08_a8uy366sev5','{"kind": "routine", "payload": "noop", "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "useless noop routine"}], "content": {"sons": ["{spare1 noop}",
   "{spare2 noop}", null], "node": "_240dwt57s08_a8uy366sev5", "jtype": "node"}}
');
INSERT INTO t_items VALUES('_2v75mmyph64_4h4kys78740','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, indicate sets of item, or give their array of elements"}],
 "content": null}
');
INSERT INTO t_items VALUES('_2vmrrvq5kdk_9um63pstcu9','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "The true of JSON.\nWe cannot use true because it is a #define-ed macro."}],
 "content": null}
');
INSERT INTO t_items VALUES('_35vp60aw7em_d436vfie4ud','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dump, jtype of JSON array values"}], "content": null}
');
INSERT INTO t_items VALUES('_3fw5acswe59_9016fqe4d41','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "Gives the integer numbers of Json for frames of tasklets."}], "content":
 null}
');
INSERT INTO t_items VALUES('_3jpt8yuzuyw_ti1pyz3me1c','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "for terminated processes"}], "content": null}
');
INSERT INTO t_items VALUES('_3v4d7uzex6f_euek4pztiuh','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "for exited processes"}], "content": null}
');
INSERT INTO t_items VALUES('_3xpyd539p4m_23h7wi59xi9','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dump, jtype of JSON object values"}], "content": null}
');
INSERT INTO t_items VALUES('_3xz3qrc6mfy_4r51up6u3pa','{"kind": "routine", "payload": "ajax_system", "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "handle \''ajax_system\'' webrequests"}, {"attr": "_7sav6zery1v_24sa6jwwu6c",
   "val": {"sons": ["{spare1 ajax-system}", "{spare2 ajax-system}", "{spare3 ajax-system}",
     null], "node": "_3xz3qrc6mfy_4r51up6u3pa", "jtype": "node"}}], "content": null}
');
INSERT INTO t_items VALUES('_41u1utcxyek_22cftxt3xxm','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "gives a human-readable comment"}], "content": null}
');
INSERT INTO t_items VALUES('_41v0erax6my_m6pytj0793u','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, give the payload of items"}], "content": null}
');
INSERT INTO t_items VALUES('_47fatww79x6_vh8ap22c0ch','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "indicates the HTTP HEAD method"}], "content": null}
');
INSERT INTO t_items VALUES('_4ezpkss1akd_94f4h25sqe4','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, give the sons of nodes"}], "content": null}
');
INSERT INTO t_items VALUES('_4m7x6811f6j_t480zu575mz','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, indicate nodes, or give their connective item"}],
 "content": null}
');
INSERT INTO t_items VALUES('_4mha85xcfwi_9zqcvkiy3dk','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "The false of JSON.\nWe cannot use false because it is a #define-ed macro."}],
 "content": null}
');
INSERT INTO t_items VALUES('_4v93t3jzrtz_srt9ear8fm8','{"kind": "routine", "payload": "ajax_complete_name", "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "handle \''ajax_complete_name\'' webrequests"}, {"attr": "_7sav6zery1v_24sa6jwwu6c",
   "val": {"sons": ["{spare1 ajax-complete_name}", "{spare2 ajax-complete_name}",
     "{spare3 ajax-complete_name}", null], "node": "_4v93t3jzrtz_srt9ear8fm8",
    "jtype": "node"}}], "content": null}
');
INSERT INTO t_items VALUES('_53748kde7s1_pkz810exr27','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "used in editor, etc. to reference some item"}], "content": null}
');
INSERT INTO t_items VALUES('_5wmusj136kq_u5qpehp89aq','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "indicates the HTTP POST method"}], "content": null}
');
INSERT INTO t_items VALUES('_6f9870y6v8t_kp8fcmq2ezv','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "Gives the state of Json for frames of tasklets."}], "content": null}
');
INSERT INTO t_items VALUES('_6hf2vzmrsee_t35suhjvtj4','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "gives the item reference, at least in dumped JSON..."}], "content": null}
');
INSERT INTO t_items VALUES('_6mwwr0i4y9p_5aupdxjxdk1','{"kind": "routine", "payload": "ajax_objects", "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "handle \''ajax_objects\'' webrequests"}, {"attr": "_7sav6zery1v_24sa6jwwu6c",
   "val": {"sons": [{"sons": ["{spare1-edit_value}"], "node": "_73im2zryfij_a7zmkketcfc",
      "jtype": "node"}, "{spare2-ajax_objects}", null], "node": "_6mwwr0i4y9p_5aupdxjxdk1",
    "jtype": "node"}}], "content": null}
');
INSERT INTO t_items VALUES('_6w3dvx83dfw_xzc6aif6isv','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "gives the attribute[s], at least in dumped JSON..."}], "content": null}
');
INSERT INTO t_items VALUES('_73im2zryfij_a7zmkketcfc','{"kind": "routine", "payload": "edit_value", "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "routine to edit a value during edition in ajax_objects"}], "content": null}
');
INSERT INTO t_items VALUES('_7diyc1cwj8z_x630afccr8e','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "Group together all noticed values in dump outcome."}], "content": null}
');
INSERT INTO t_items VALUES('_7rf7axuc9h4_2aw6utwmsas','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, give the space of items"}], "content": null}
');
INSERT INTO t_items VALUES('_7sav6zery1v_24sa6jwwu6c','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "attribute giving the web handler inside items"}], "content": null}
');
INSERT INTO t_items VALUES('_7urjeiw3evy_m7k72uv6790','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, give the type of a value"}], "content": null}
');
INSERT INTO t_items VALUES('_7vw56h18sw0_hv77m6q8uxu','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, used for tuples of items"}], "content": null}
');
INSERT INTO t_items VALUES('_7wk9y7e7r0z_575esi8ys5x','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, used for values in attribute lists of items"}],
 "content": null}
');
INSERT INTO t_items VALUES('_8j516kuv89j_4hc4w6ykmr6','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "in JSON dumps, used for long chunked strings"}], "content": null}
');
INSERT INTO t_items VALUES('_8s357rq2dzk_k8ze95tikjm','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "gives the item content, at least in dumped JSON..."}], "content": null}
');
INSERT INTO t_items VALUES('_91pketvc5pz_wq0v0wpauw8','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "Gives the values of Json for frames of tasklets."}], "content": null}
');
INSERT INTO t_items VALUES('_97zkxf62r11_6eedwwv3eu8','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "Gives the closure of Json for frames of tasklets"}], "content": null}
');
INSERT INTO t_items VALUES('_9dsak0qcy0v_1c5z9th7x3i','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "indicates the HTTP GET method"}], "content": null}
');
INSERT INTO t_items VALUES('_9sd1mh9q1zf_3duewi6fsaq','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "for exited processes with exit code >0"}], "content": null}
');
INSERT INTO t_names VALUES('GET','_9dsak0qcy0v_1c5z9th7x3i','.predef');
INSERT INTO t_names VALUES('HEAD','_47fatww79x6_vh8ap22c0ch','.predef');
INSERT INTO t_names VALUES('POST','_5wmusj136kq_u5qpehp89aq','.predef');
INSERT INTO t_names VALUES('agenda','_02u53qxa7dm_sttmhffpchr','.predef');
INSERT INTO t_names VALUES('ajax_complete_name','_4v93t3jzrtz_srt9ear8fm8','.root');
INSERT INTO t_names VALUES('ajax_objects','_6mwwr0i4y9p_5aupdxjxdk1','.root');
INSERT INTO t_names VALUES('ajax_system','_3xz3qrc6mfy_4r51up6u3pa','.root');
INSERT INTO t_names VALUES('attr','_6w3dvx83dfw_xzc6aif6isv','.predef');
INSERT INTO t_names VALUES('closure','_97zkxf62r11_6eedwwv3eu8','.predef');
INSERT INTO t_names VALUES('comment','_41u1utcxyek_22cftxt3xxm','.predef');
INSERT INTO t_names VALUES('content','_8s357rq2dzk_k8ze95tikjm','.predef');
INSERT INTO t_names VALUES('doubles','_17spwr8dkzv_tsf2s8diazu','.predef');
INSERT INTO t_names VALUES('edit_value','_73im2zryfij_a7zmkketcfc','.root');
INSERT INTO t_names VALUES('exited','_3v4d7uzex6f_euek4pztiuh','.predef');
INSERT INTO t_names VALUES('failed','_9sd1mh9q1zf_3duewi6fsaq','.predef');
INSERT INTO t_names VALUES('item','_53748kde7s1_pkz810exr27','.predef');
INSERT INTO t_names VALUES('item_ref','_6hf2vzmrsee_t35suhjvtj4','.predef');
INSERT INTO t_names VALUES('json_array','_35vp60aw7em_d436vfie4ud','.predef');
INSERT INTO t_names VALUES('json_false','_4mha85xcfwi_9zqcvkiy3dk','.predef');
INSERT INTO t_names VALUES('json_object','_3xpyd539p4m_23h7wi59xi9','.predef');
INSERT INTO t_names VALUES('json_true','_2vmrrvq5kdk_9um63pstcu9','.predef');
INSERT INTO t_names VALUES('jtype','_7urjeiw3evy_m7k72uv6790','.predef');
INSERT INTO t_names VALUES('kind','_06yp8ueq6yf_5ts408yww29','.predef');
INSERT INTO t_names VALUES('node','_4m7x6811f6j_t480zu575mz','.predef');
INSERT INTO t_names VALUES('noop','_240dwt57s08_a8uy366sev5','.root');
INSERT INTO t_names VALUES('notice','_7diyc1cwj8z_x630afccr8e','.predef');
INSERT INTO t_names VALUES('numbers','_3fw5acswe59_9016fqe4d41','.predef');
INSERT INTO t_names VALUES('payload','_41v0erax6my_m6pytj0793u','.predef');
INSERT INTO t_names VALUES('set','_2v75mmyph64_4h4kys78740','.predef');
INSERT INTO t_names VALUES('sons','_4ezpkss1akd_94f4h25sqe4','.predef');
INSERT INTO t_names VALUES('space','_7rf7axuc9h4_2aw6utwmsas','.predef');
INSERT INTO t_names VALUES('state','_6f9870y6v8t_kp8fcmq2ezv','.predef');
INSERT INTO t_names VALUES('string','_8j516kuv89j_4hc4w6ykmr6','.predef');
INSERT INTO t_names VALUES('terminated','_3jpt8yuzuyw_ti1pyz3me1c','.predef');
INSERT INTO t_names VALUES('tuple','_7vw56h18sw0_hv77m6q8uxu','.predef');
INSERT INTO t_names VALUES('val','_7wk9y7e7r0z_575esi8ys5x','.predef');
INSERT INTO t_names VALUES('values','_91pketvc5pz_wq0v0wpauw8','.predef');
INSERT INTO t_names VALUES('web_handler','_7sav6zery1v_24sa6jwwu6c','.predef');
COMMIT;
-- state-monimelt end dump 
