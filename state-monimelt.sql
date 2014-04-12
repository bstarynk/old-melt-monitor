-- state-monimelt dump 2014 Apr 12
CREATE TABLE t_item (uid CHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE, type VARCHAR(60) NOT NULL, jbuild TEXT NOT NULL, jfill TEXT NOT NULL);
CREATE TABLE t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, nuid CHAR(40) UNIQUE NOT NULL REFERENCES t_id(uid));
-- state-monimelt tables contents
INSERT INTO t_item VALUES('11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','json_name','','');
INSERT INTO t_item VALUES('1fca3bc7-7316-4597-9d9f-ff97bc4f4353','json_name','','');
INSERT INTO t_item VALUES('b9664990-11a1-49a5-bf59-be714626ed6d','json_name','','');
INSERT INTO t_item VALUES('d7a1e1a4-f401-436a-887f-980f66643a24','json_name','','');
INSERT INTO t_name VALUES('json_array','d7a1e1a4-f401-436a-887f-980f66643a24');
INSERT INTO t_name VALUES('json_name_item','1fca3bc7-7316-4597-9d9f-ff97bc4f4353');
INSERT INTO t_name VALUES('json_object','11da23ab-3b9e-4ea0-bc13-4a9aa46e2995');
INSERT INTO t_name VALUES('jtype','b9664990-11a1-49a5-bf59-be714626ed6d');
-- state-monimelt end dump 
