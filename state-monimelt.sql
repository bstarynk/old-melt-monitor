-- state-monimelt dump 2014 Apr 12 05:17:30 MEST
CREATE TABLE t_item (uid CHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE, type VARCHAR(60) NOT NULL, jbuild TEXT, jfill TEXT);
CREATE TABLE t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, nuid CHAR(40) UNIQUE NOT NULL REFERENCES t_id(uid));
-- state-monimelt tables contents
INSERT INTO t_item VALUES('b9664990-11a1-49a5-bf59-be714626ed6d','json_name',NULL,NULL);
INSERT INTO t_name VALUES('jtype','b9664990-11a1-49a5-bf59-be714626ed6d');
-- state-monimelt end dump 
