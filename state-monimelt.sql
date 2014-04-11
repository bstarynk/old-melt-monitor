-- state-monimelt dump 2014 Apr 11 23:02:32 MEST
CREATE TABLE t_item (uid CHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE, type VARCHAR(60) NOT NULL, jbuild TEXT, jfill TEXT);
CREATE TABLE t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, nuid CHAR(40) UNIQUE NOT NULL REFERENCES t_id(uid));
INSERT INTO t_item VALUES('d1f11920-92a6-40e3-bcf9-bbfe23958d5a','json_name',NULL,NULL);
INSERT INTO t_name VALUES('jtype','d1f11920-92a6-40e3-bcf9-bbfe23958d5a');
-- state-monimelt end dump 
