-- state-monimelt dump 2014 Apr 12 05:10:18 MEST
CREATE TABLE t_item (uid CHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE, type VARCHAR(60) NOT NULL, jbuild TEXT, jfill TEXT);
CREATE TABLE t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, nuid CHAR(40) UNIQUE NOT NULL REFERENCES t_id(uid));
-- state-monimelt tables contents
-- state-monimelt end dump 
