-- state-monimelt dump 2014 Apr 26

 --   Copyright (C) 2014 Free Software Foundation, Inc.
 --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
 --  This file is part of GCC.
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
CREATE TABLE t_item (uid VARCHAR(38) PRIMARY KEY ASC NOT NULL UNIQUE, type VARCHAR(60) NOT NULL, jbuild TEXT NOT NULL, jfill TEXT NOT NULL);
CREATE TABLE t_name (name TEXT PRIMARY KEY ASC NOT NULL UNIQUE, nuid VARCHAR(38) UNIQUE NOT NULL REFERENCES t_id(uid), spacenam VARCHAR(30) NOT NULL);
CREATE TABLE t_param (parname VARCHAR(35) PRIMARY KEY ASC NOT NULL UNIQUE, parvalue TEXT NOT NULL);
CREATE TABLE t_module (modname VARCHAR(100) PRIMARY KEY ASC NOT NULL UNIQUE);
-- state-monimelt tables contents
INSERT INTO t_item VALUES('03c1ace3-3293-43bb-9541-19b4357cbc3c','json_name','{"jtype":"json_name_item","name":"conn"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','json_name','{"jtype":"json_name_item","name":"json_object"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('18d95093-b523-4ee1-8bca-ded252e91235','json_name','{"jtype":"json_name_item","name":"itemref"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('1fca3bc7-7316-4597-9d9f-ff97bc4f4353','json_name','{"jtype":"json_name_item","name":"json_name_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('3265e0f0-f3f7-4ef1-b157-50ec4fd4ce41','json_name','{"jtype":"json_name_item","name":"set"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('326c3317-5c04-47dc-bf74-f4fc776eab72','json_name','{"jtype":"json_name_item","name":"assoc_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('35461f69-eeb6-4562-806b-f37537d6ce7a','json_name','{"jtype":"json_name_item","name":"vector_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('3b19b433-29b0-473e-9827-7756ee219573','bool','{"jtype":"bool_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('402d1108-9e25-4065-b81d-1bea8b06f269','json_name','{"jtype":"json_name_item","name":"vector"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('4871e619-947e-47da-bd0b-e7026f818072','json_name','{"jtype":"json_name_item","name":"dictionnary"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('52d29f69-0bec-435b-af7c-f5cbeb51a80b','json_name','{"jtype":"json_name_item","name":"state"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('566acfce-866a-4730-884d-30310b2fdf92','json_name','{"jtype":"json_name_item","name":"space"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('5e77140e-2467-4e85-aa28-8ca734c2aa07','json_name','{"jtype":"json_name_item","name":"refitem"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('670d0025-401e-4c78-9d8b-62fad2ad26ec','json_name','{"jtype":"json_name_item","name":"associations"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('69cbd072-7d8e-4f62-bef4-5cd0cc847578','bool','{"jtype":"bool_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('77d7a663-11db-4c19-84df-7aef868c532a','json_name','{"jtype":"json_name_item","name":"queue_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('7b00e12a-c591-4ed7-83ca-cf2bb973316a','json_name','{"jtype":"json_name_item","name":"buffer_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('7e8b7206-2548-427d-9b7c-9d5b5067c78a','json_name','{"jtype":"json_name_item","name":"values"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('80e337da-7a05-40e4-86ec-806ac78dbd03','json_name','{"jtype":"json_name_item","name":"frames"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('84961ab4-f2ff-4e10-9479-2e62148c54bc','json_name','{"jtype":"json_name_item","name":"numbers"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('8dba998c-341b-4a66-900f-545ac9dd06c4','json_name','{"jtype":"json_name_item","name":"sons"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('97ec1834-c510-4d38-967a-6f8665a39b46','json_name','{"jtype":"json_name_item","name":"box_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('9a05b9fa-17d3-4294-9253-e67406e52d4a','json_name','{"jtype":"json_name_item","name":"content"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('a75bef70-596b-4fc5-99ca-e60b5287feec','json_name','{"jtype":"json_name_item","name":"uuid"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('abdd3eea-22da-4e09-b1f8-a05b80783924','json_name','{"jtype":"json_name_item","name":"name"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('b039b53b-5198-4770-ac05-7eeded55fdeb','json_name','{"jtype":"json_name_item","name":"tasklet_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('b29bc8a3-1517-4a28-b566-39e447c80250','json_name','{"jtype":"json_name_item","name":"attributes"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('b5a0ef97-7eb2-4cf3-924e-8bc5de7e49bc','queue','{"jtype":"queue_item"}','{"attributes":null,"queue":[],"content":null}');
INSERT INTO t_item VALUES('b9664990-11a1-49a5-bf59-be714626ed6d','json_name','{"jtype":"json_name_item","name":"jtype"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('c52ead7c-63bf-4e65-a873-486de4e4db41','json_name','{"jtype":"json_name_item","name":"tuple"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('cddd3209-6d5f-4ac5-ada3-fc9ed0053728','json_name','{"jtype":"json_name_item","name":"routine_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('d39e1047-71fb-4c62-b13d-a18852d844ef','json_name','{"jtype":"json_name_item","name":"bool_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('d7a1e1a4-f401-436a-887f-980f66643a24','json_name','{"jtype":"json_name_item","name":"json_array"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('d8aa61cc-2543-4659-bff9-2b0bfc0f779a','json_name','{"jtype":"json_name_item","name":"val"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('dc6e0602-32b4-4416-a675-d45fb894f7eb','json_name','{"jtype":"json_name_item","name":"box"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('e29d6d73-6a40-41bd-bfd4-df5183ffc764','json_name','{"jtype":"json_name_item","name":"attr"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('e6727e08-6232-4beb-8c17-3b9cdf5045fb','json_name','{"jtype":"json_name_item","name":"doubles"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('eaada55e-bcfb-4b87-b335-65b31ed2fdac','json_name','{"jtype":"json_name_item","name":"queue"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('eadcbe90-fc81-4500-92e6-dde0a81e775e','json_name','{"jtype":"json_name_item","name":"dictionnary_item"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('ebb71e9a-ef65-4fcd-aec9-f4a5e780e333','json_name','{"jtype":"json_name_item","name":"buffer"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('f8a3568d-ae4a-4486-a362-577e0ec91474','json_name','{"jtype":"json_name_item","name":"closure"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('f99051be-30bf-4efc-8c35-b4344c4e6d2c','json_name','{"jtype":"json_name_item","name":"node"}','{"attributes":null,"content":null}');
INSERT INTO t_name VALUES('agenda','b5a0ef97-7eb2-4cf3-924e-8bc5de7e49bc','.');
INSERT INTO t_name VALUES('assoc_item','326c3317-5c04-47dc-bf74-f4fc776eab72','.');
INSERT INTO t_name VALUES('associations','670d0025-401e-4c78-9d8b-62fad2ad26ec','.');
INSERT INTO t_name VALUES('attr','e29d6d73-6a40-41bd-bfd4-df5183ffc764','.');
INSERT INTO t_name VALUES('attributes','b29bc8a3-1517-4a28-b566-39e447c80250','.');
INSERT INTO t_name VALUES('bool_item','d39e1047-71fb-4c62-b13d-a18852d844ef','.');
INSERT INTO t_name VALUES('box','dc6e0602-32b4-4416-a675-d45fb894f7eb','.');
INSERT INTO t_name VALUES('box_item','97ec1834-c510-4d38-967a-6f8665a39b46','.');
INSERT INTO t_name VALUES('buffer','ebb71e9a-ef65-4fcd-aec9-f4a5e780e333','.');
INSERT INTO t_name VALUES('buffer_item','7b00e12a-c591-4ed7-83ca-cf2bb973316a','.');
INSERT INTO t_name VALUES('closure','f8a3568d-ae4a-4486-a362-577e0ec91474','.');
INSERT INTO t_name VALUES('conn','03c1ace3-3293-43bb-9541-19b4357cbc3c','.');
INSERT INTO t_name VALUES('content','9a05b9fa-17d3-4294-9253-e67406e52d4a','.');
INSERT INTO t_name VALUES('dictionnary','4871e619-947e-47da-bd0b-e7026f818072','.');
INSERT INTO t_name VALUES('dictionnary_item','eadcbe90-fc81-4500-92e6-dde0a81e775e','.');
INSERT INTO t_name VALUES('doubles','e6727e08-6232-4beb-8c17-3b9cdf5045fb','.');
INSERT INTO t_name VALUES('false','69cbd072-7d8e-4f62-bef4-5cd0cc847578','.');
INSERT INTO t_name VALUES('frames','80e337da-7a05-40e4-86ec-806ac78dbd03','.');
INSERT INTO t_name VALUES('itemref','18d95093-b523-4ee1-8bca-ded252e91235','.');
INSERT INTO t_name VALUES('json_array','d7a1e1a4-f401-436a-887f-980f66643a24','.');
INSERT INTO t_name VALUES('json_name_item','1fca3bc7-7316-4597-9d9f-ff97bc4f4353','.');
INSERT INTO t_name VALUES('json_object','11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','.');
INSERT INTO t_name VALUES('jtype','b9664990-11a1-49a5-bf59-be714626ed6d','.');
INSERT INTO t_name VALUES('name','abdd3eea-22da-4e09-b1f8-a05b80783924','.');
INSERT INTO t_name VALUES('node','f99051be-30bf-4efc-8c35-b4344c4e6d2c','.');
INSERT INTO t_name VALUES('numbers','84961ab4-f2ff-4e10-9479-2e62148c54bc','.');
INSERT INTO t_name VALUES('queue','eaada55e-bcfb-4b87-b335-65b31ed2fdac','.');
INSERT INTO t_name VALUES('queue_item','77d7a663-11db-4c19-84df-7aef868c532a','.');
INSERT INTO t_name VALUES('refitem','5e77140e-2467-4e85-aa28-8ca734c2aa07','.');
INSERT INTO t_name VALUES('routine_item','cddd3209-6d5f-4ac5-ada3-fc9ed0053728','.');
INSERT INTO t_name VALUES('set','3265e0f0-f3f7-4ef1-b157-50ec4fd4ce41','.');
INSERT INTO t_name VALUES('sons','8dba998c-341b-4a66-900f-545ac9dd06c4','.');
INSERT INTO t_name VALUES('space','566acfce-866a-4730-884d-30310b2fdf92','.');
INSERT INTO t_name VALUES('state','52d29f69-0bec-435b-af7c-f5cbeb51a80b','.');
INSERT INTO t_name VALUES('tasklet_item','b039b53b-5198-4770-ac05-7eeded55fdeb','.');
INSERT INTO t_name VALUES('true','3b19b433-29b0-473e-9827-7756ee219573','.');
INSERT INTO t_name VALUES('tuple','c52ead7c-63bf-4e65-a873-486de4e4db41','.');
INSERT INTO t_name VALUES('uuid','a75bef70-596b-4fc5-99ca-e60b5287feec','.');
INSERT INTO t_name VALUES('val','d8aa61cc-2543-4659-bff9-2b0bfc0f779a','.');
INSERT INTO t_name VALUES('values','7e8b7206-2548-427d-9b7c-9d5b5067c78a','.');
INSERT INTO t_name VALUES('vector','402d1108-9e25-4065-b81d-1bea8b06f269','.');
INSERT INTO t_name VALUES('vector_item','35461f69-eeb6-4562-806b-f37537d6ce7a','.');
INSERT INTO t_param VALUES('dump_format_version','MoniMelt2014A');
COMMIT;
-- state-monimelt end dump 
