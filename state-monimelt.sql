-- state-monimelt dump 2014 Apr 18
CREATE TABLE t_item (uid CHAR(40) PRIMARY KEY ASC NOT NULL UNIQUE, type VARCHAR(60) NOT NULL, jbuild TEXT NOT NULL, jfill TEXT NOT NULL);
CREATE TABLE t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, nuid CHAR(40) UNIQUE NOT NULL REFERENCES t_id(uid), spacenam VARCHAR(16) NOT NULL);
-- state-monimelt tables contents
INSERT INTO t_item VALUES('03c1ace3-3293-43bb-9541-19b4357cbc3c','json_name','','');
INSERT INTO t_item VALUES('11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','json_name','','');
INSERT INTO t_item VALUES('18d95093-b523-4ee1-8bca-ded252e91235','json_name','','');
INSERT INTO t_item VALUES('1fca3bc7-7316-4597-9d9f-ff97bc4f4353','json_name','','');
INSERT INTO t_item VALUES('3265e0f0-f3f7-4ef1-b157-50ec4fd4ce41','json_name','','');
INSERT INTO t_item VALUES('3b19b433-29b0-473e-9827-7756ee219573','bool','','');
INSERT INTO t_item VALUES('566acfce-866a-4730-884d-30310b2fdf92','json_name','','');
INSERT INTO t_item VALUES('5e77140e-2467-4e85-aa28-8ca734c2aa07','json_name','','');
INSERT INTO t_item VALUES('69cbd072-7d8e-4f62-bef4-5cd0cc847578','bool','','');
INSERT INTO t_item VALUES('8dba998c-341b-4a66-900f-545ac9dd06c4','json_name','','');
INSERT INTO t_item VALUES('a75bef70-596b-4fc5-99ca-e60b5287feec','json_name','','');
INSERT INTO t_item VALUES('b9664990-11a1-49a5-bf59-be714626ed6d','json_name','','');
INSERT INTO t_item VALUES('c52ead7c-63bf-4e65-a873-486de4e4db41','json_name','','');
INSERT INTO t_item VALUES('d7a1e1a4-f401-436a-887f-980f66643a24','json_name','','');
INSERT INTO t_item VALUES('f8a3568d-ae4a-4486-a362-577e0ec91474','json_name','','');
INSERT INTO t_item VALUES('f99051be-30bf-4efc-8c35-b4344c4e6d2c','json_name','','');
INSERT INTO t_name VALUES('closure','f8a3568d-ae4a-4486-a362-577e0ec91474','.');
INSERT INTO t_name VALUES('conn','03c1ace3-3293-43bb-9541-19b4357cbc3c','.');
INSERT INTO t_name VALUES('false','69cbd072-7d8e-4f62-bef4-5cd0cc847578','.');
INSERT INTO t_name VALUES('itemref','18d95093-b523-4ee1-8bca-ded252e91235','.');
INSERT INTO t_name VALUES('json_array','d7a1e1a4-f401-436a-887f-980f66643a24','.');
INSERT INTO t_name VALUES('json_name_item','1fca3bc7-7316-4597-9d9f-ff97bc4f4353','.');
INSERT INTO t_name VALUES('json_object','11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','.');
INSERT INTO t_name VALUES('jtype','b9664990-11a1-49a5-bf59-be714626ed6d','.');
INSERT INTO t_name VALUES('node','f99051be-30bf-4efc-8c35-b4344c4e6d2c','.');
INSERT INTO t_name VALUES('refitem','5e77140e-2467-4e85-aa28-8ca734c2aa07','.');
INSERT INTO t_name VALUES('set','3265e0f0-f3f7-4ef1-b157-50ec4fd4ce41','.');
INSERT INTO t_name VALUES('sons','8dba998c-341b-4a66-900f-545ac9dd06c4','.');
INSERT INTO t_name VALUES('space','566acfce-866a-4730-884d-30310b2fdf92','.');
INSERT INTO t_name VALUES('true','3b19b433-29b0-473e-9827-7756ee219573','.');
INSERT INTO t_name VALUES('tuple','c52ead7c-63bf-4e65-a873-486de4e4db41','.');
INSERT INTO t_name VALUES('uuid','a75bef70-596b-4fc5-99ca-e60b5287feec','.');
-- state-monimelt end dump 
