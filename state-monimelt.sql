-- state-monimelt dump 2014 May 22

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
INSERT INTO t_params VALUES('dump_reason','after predefined notice');
INSERT INTO t_items VALUES('_06yp8ueq6yf_5ts408yww29','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_2v75mmyph64_4h4kys78740','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_2vmrrvq5kdk_9um63pstcu9','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_35vp60aw7em_d436vfie4ud','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_3xpyd539p4m_23h7wi59xi9','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_41u1utcxyek_22cftxt3xxm','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_41v0erax6my_m6pytj0793u','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_4ezpkss1akd_94f4h25sqe4','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_4m7x6811f6j_t480zu575mz','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_4mha85xcfwi_9zqcvkiy3dk','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_6hf2vzmrsee_t35suhjvtj4','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_6w3dvx83dfw_xzc6aif6isv','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_7diyc1cwj8z_x630afccr8e','{"kind": null, "payload": null, "attr": [{"attr": "_41u1utcxyek_22cftxt3xxm",
   "val": "notice after dumping"}], "content": null}
');
INSERT INTO t_items VALUES('_7rf7axuc9h4_2aw6utwmsas','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_7urjeiw3evy_m7k72uv6790','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_7vw56h18sw0_hv77m6q8uxu','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_7wk9y7e7r0z_575esi8ys5x','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_8j516kuv89j_4hc4w6ykmr6','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_items VALUES('_8s357rq2dzk_k8ze95tikjm','{"kind": null, "payload": null, "attr": [], "content": null}
');
INSERT INTO t_names VALUES('attr','_6w3dvx83dfw_xzc6aif6isv','.predef');
INSERT INTO t_names VALUES('comment','_41u1utcxyek_22cftxt3xxm','.predef');
INSERT INTO t_names VALUES('content','_8s357rq2dzk_k8ze95tikjm','.predef');
INSERT INTO t_names VALUES('item_ref','_6hf2vzmrsee_t35suhjvtj4','.predef');
INSERT INTO t_names VALUES('json_array','_35vp60aw7em_d436vfie4ud','.predef');
INSERT INTO t_names VALUES('json_false','_4mha85xcfwi_9zqcvkiy3dk','.predef');
INSERT INTO t_names VALUES('json_object','_3xpyd539p4m_23h7wi59xi9','.predef');
INSERT INTO t_names VALUES('json_true','_2vmrrvq5kdk_9um63pstcu9','.predef');
INSERT INTO t_names VALUES('jtype','_7urjeiw3evy_m7k72uv6790','.predef');
INSERT INTO t_names VALUES('kind','_06yp8ueq6yf_5ts408yww29','.predef');
INSERT INTO t_names VALUES('node','_4m7x6811f6j_t480zu575mz','.predef');
INSERT INTO t_names VALUES('notice','_7diyc1cwj8z_x630afccr8e','.predef');
INSERT INTO t_names VALUES('payload','_41v0erax6my_m6pytj0793u','.predef');
INSERT INTO t_names VALUES('set','_2v75mmyph64_4h4kys78740','.predef');
INSERT INTO t_names VALUES('sons','_4ezpkss1akd_94f4h25sqe4','.predef');
INSERT INTO t_names VALUES('space','_7rf7axuc9h4_2aw6utwmsas','.predef');
INSERT INTO t_names VALUES('string','_8j516kuv89j_4hc4w6ykmr6','.predef');
INSERT INTO t_names VALUES('tuple','_7vw56h18sw0_hv77m6q8uxu','.predef');
INSERT INTO t_names VALUES('val','_7wk9y7e7r0z_575esi8ys5x','.predef');
COMMIT;
-- state-monimelt end dump 
