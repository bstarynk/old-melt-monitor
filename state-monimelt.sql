-- state-monimelt dump 2014 May 11

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
INSERT INTO t_item VALUES('0229c553-c443-49a9-9244-5f8d7d3c79cf','dictionnary','{"jtype":"dictionnary"}','{"attributes":null,"content":null,"dictionnary":[{"name":"ajax_complete_name",
 "val":{"conn":{"jtype":"itemref","space":".","uuid":"9a4eeb30-2c39-4d6d-b737-59cb3e428fd0"},
 "jtype":"closure","sons":["Gap*Ajax_Complete_Name"]}},{"name":"ajax_complete_routine_name",
 "val":{"conn":{"jtype":"itemref","space":".","uuid":"8e5f1892-9c0b-42c0-8390-e384e471d3fd"},
 "jtype":"closure","sons":["Gap*Ajax_Complete_Routine_Name"]}},{"name":"ajax_exit",
 "val":{"conn":{"jtype":"itemref","space":".","uuid":"16b844e6-0277-492d-aee9-cb687571a99c"},
 "jtype":"closure","sons":["Gap*Ajax_Exit"]}},{"name":"ajax_named","val":{"conn":{"jtype":"itemref",
  "space":".","uuid":"84220ca7-346c-4e8e-a5c8-0ec0367248d7"},"jtype":"closure",
 "sons":["Gap*Ajax_Named"]}},
{"name":"ajax_routine","val":{"conn":{"jtype":"itemref","space":".","uuid":"84a82dc6-f049-42c4-ac69-ec458b8054ec"},
 "jtype":"closure","sons":["Gap*Ajax_Routine"]}},{"name":"ajax_start","val":{"conn":{"jtype":"itemref",
  "space":".","uuid":"0e750860-7063-4ad7-8488-ee57d4aba2a8"},"jtype":"closure",
 "sons":["Gap*Ajax_Start"]}}]}
');
INSERT INTO t_item VALUES('03c1ace3-3293-43bb-9541-19b4357cbc3c','json_name','{"jtype":"json_name","name":"conn"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('0403e866-c514-4ee1-aa47-fc8b476f937e','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('0e750860-7063-4ad7-8488-ee57d4aba2a8','routine','{"jtype":"routine","name":"ajax_start"}','{"attributes":[{"attr":{"jtype":"itemref","space":".","uuid":"1e299c1c-4d29-4616-b30b-258c72722484"},
 "val":"for ajax_start GET request at HTML page initialization"}],"content":null}');
INSERT INTO t_item VALUES('11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','json_name','{"jtype":"json_name","name":"json_object"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('16b844e6-0277-492d-aee9-cb687571a99c','routine','{"jtype":"routine","name":"ajax_exit"}','{"attributes":[{"attr":{"jtype":"itemref","space":".","uuid":"1e299c1c-4d29-4616-b30b-258c72722484"},
 "val":"ajax processing of exit"}],"content":null}');
INSERT INTO t_item VALUES('18d95093-b523-4ee1-8bca-ded252e91235','json_name','{"jtype":"json_name","name":"itemref"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('1e299c1c-4d29-4616-b30b-258c72722484','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('3265e0f0-f3f7-4ef1-b157-50ec4fd4ce41','json_name','{"jtype":"json_name","name":"set"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('3b19b433-29b0-473e-9827-7756ee219573','bool','{"jtype":"boolean"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('402d1108-9e25-4065-b81d-1bea8b06f269','json_name','{"jtype":"json_name","name":"vector"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('40dcfdf6-6005-4dc3-9947-c1743ab1c41e','json_name','{"jtype":"json_name","name":"json_name"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('43714c87-bb83-4413-bc7b-41fc01976f76','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('4566a0da-5cc8-45e5-9f1a-1cecfd00097b','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('460ac072-865c-4587-b5e0-be9c83511ba8','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('4871e619-947e-47da-bd0b-e7026f818072','json_name','{"jtype":"json_name","name":"dictionnary"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('49c854e1-d91f-43b4-8a49-428fa8dceec4','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('4a77f6cf-ee8d-43d5-8af4-b273594e23cd','json_name','{"jtype":"json_name","name":"boolean"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('4b1faa71-01c2-4dc9-b9b8-f894ae54a1d0','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('52d29f69-0bec-435b-af7c-f5cbeb51a80b','json_name','{"jtype":"json_name","name":"state"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('5446420a-4d37-4505-91a1-4c3bc5db01c1','json_name','{"jtype":"json_name","name":"tasklet"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('566acfce-866a-4730-884d-30310b2fdf92','json_name','{"jtype":"json_name","name":"space"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('5e77140e-2467-4e85-aa28-8ca734c2aa07','json_name','{"jtype":"json_name","name":"refitem"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('670d0025-401e-4c78-9d8b-62fad2ad26ec','json_name','{"jtype":"json_name","name":"associations"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('69cbd072-7d8e-4f62-bef4-5cd0cc847578','bool','{"jtype":"boolean"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('6dd4683e-cb4d-434b-b425-e45eed24d35e','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('764e6c3e-0e1d-4462-9dfc-a590123b2755','json_name','{"jtype":"json_name","name":"assoc"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('7e8b7206-2548-427d-9b7c-9d5b5067c78a','json_name','{"jtype":"json_name","name":"values"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('80e337da-7a05-40e4-86ec-806ac78dbd03','json_name','{"jtype":"json_name","name":"frames"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('84220ca7-346c-4e8e-a5c8-0ec0367248d7','routine','{"jtype":"routine","name":"ajax_named"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('84961ab4-f2ff-4e10-9479-2e62148c54bc','json_name','{"jtype":"json_name","name":"numbers"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('84a82dc6-f049-42c4-ac69-ec458b8054ec','routine','{"jtype":"routine","name":"ajax_routine"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('88aeaf2b-192e-4936-bdbb-b9a186d86cd4','routine','{"jtype":"routine","name":"proc_compilation"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('8dba998c-341b-4a66-900f-545ac9dd06c4','json_name','{"jtype":"json_name","name":"sons"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('8e3cbe87-244d-49e7-ac72-e6f8a09afd7c','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('8e5f1892-9c0b-42c0-8390-e384e471d3fd','routine','{"jtype":"routine","name":"ajax_complete_routine_name"}','{"attributes":[{"attr":{"jtype":"itemref","space":".","uuid":"1e299c1c-4d29-4616-b30b-258c72722484"},
 "val":"for autocompletion of routine names"}],"content":null}');
INSERT INTO t_item VALUES('93abbd1c-2d33-4b86-8e16-e5b0c14842dc','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('9a05b9fa-17d3-4294-9253-e67406e52d4a','json_name','{"jtype":"json_name","name":"content"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('9a4eeb30-2c39-4d6d-b737-59cb3e428fd0','routine','{"jtype":"routine","name":"ajax_complete_name"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('9dccb4ab-45d0-4fa3-a3d0-8bb722a83478','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('9ef8915a-d57e-42b5-8e6d-507470a69996','json_name','{"jtype":"json_name","name":"string"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('a75bef70-596b-4fc5-99ca-e60b5287feec','json_name','{"jtype":"json_name","name":"uuid"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('abdd3eea-22da-4e09-b1f8-a05b80783924','json_name','{"jtype":"json_name","name":"name"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('b29bc8a3-1517-4a28-b566-39e447c80250','json_name','{"jtype":"json_name","name":"attributes"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('b5a0ef97-7eb2-4cf3-924e-8bc5de7e49bc','queue','{"jtype":"queue"}','{"attributes":null,"content":null,"queue":[]}');
INSERT INTO t_item VALUES('b6e41f84-c5f6-48d2-af55-6638c3cc9a64','routine','{"jtype":"routine","name":"ajax_complete_name"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('b9664990-11a1-49a5-bf59-be714626ed6d','json_name','{"jtype":"json_name","name":"jtype"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('c3b77d8b-ff65-4717-b9d5-f2b5c22fca71','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('c52ead7c-63bf-4e65-a873-486de4e4db41','json_name','{"jtype":"json_name","name":"tuple"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('c5d8949e-8348-4166-82b8-5bf9ce3a6778','json_name','{"jtype":"json_name","name":"routine"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('d7a1e1a4-f401-436a-887f-980f66643a24','json_name','{"jtype":"json_name","name":"json_array"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('d7d288ae-7a8e-4709-99d0-6dfba1c9565f','routine','{"jtype":"routine","name":"cold_routine_emit"}','{"attributes":[{"attr":{"jtype":"itemref","space":".","uuid":"1e299c1c-4d29-4616-b30b-258c72722484"},
 "val":"the first emitter of C code for our first routines."}],"content":null}');
INSERT INTO t_item VALUES('d8aa61cc-2543-4659-bff9-2b0bfc0f779a','json_name','{"jtype":"json_name","name":"val"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('dc6e0602-32b4-4416-a675-d45fb894f7eb','json_name','{"jtype":"json_name","name":"box"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('e29d6d73-6a40-41bd-bfd4-df5183ffc764','json_name','{"jtype":"json_name","name":"attr"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('e6727e08-6232-4beb-8c17-3b9cdf5045fb','json_name','{"jtype":"json_name","name":"doubles"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('eaada55e-bcfb-4b87-b335-65b31ed2fdac','json_name','{"jtype":"json_name","name":"queue"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('ebb71e9a-ef65-4fcd-aec9-f4a5e780e333','json_name','{"jtype":"json_name","name":"buffer"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('ed78ae41-bd10-44bf-9bc7-57be044fe610','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_item VALUES('f8a3568d-ae4a-4486-a362-577e0ec91474','json_name','{"jtype":"json_name","name":"closure"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('f99051be-30bf-4efc-8c35-b4344c4e6d2c','json_name','{"jtype":"json_name","name":"node"}','{"attributes":null,"content":null}');
INSERT INTO t_item VALUES('fa97d5b8-40cb-4884-9e77-27803c6a4deb','box','{"jtype":"box"}','{"attributes":null,"box":null,"content":null}');
INSERT INTO t_name VALUES('GET','460ac072-865c-4587-b5e0-be9c83511ba8','.');
INSERT INTO t_name VALUES('HEAD','4566a0da-5cc8-45e5-9f1a-1cecfd00097b','.');
INSERT INTO t_name VALUES('OPTIONS','4b1faa71-01c2-4dc9-b9b8-f894ae54a1d0','.');
INSERT INTO t_name VALUES('POST','9dccb4ab-45d0-4fa3-a3d0-8bb722a83478','.');
INSERT INTO t_name VALUES('PUT','ed78ae41-bd10-44bf-9bc7-57be044fe610','.');
INSERT INTO t_name VALUES('agenda','b5a0ef97-7eb2-4cf3-924e-8bc5de7e49bc','.');
INSERT INTO t_name VALUES('ajax_complete_name','b6e41f84-c5f6-48d2-af55-6638c3cc9a64','.');
INSERT INTO t_name VALUES('ajax_complete_routine_name','8e5f1892-9c0b-42c0-8390-e384e471d3fd','.');
INSERT INTO t_name VALUES('ajax_exit','16b844e6-0277-492d-aee9-cb687571a99c','.');
INSERT INTO t_name VALUES('ajax_named','84220ca7-346c-4e8e-a5c8-0ec0367248d7','.');
INSERT INTO t_name VALUES('ajax_routine','84a82dc6-f049-42c4-ac69-ec458b8054ec','.');
INSERT INTO t_name VALUES('ajax_start','0e750860-7063-4ad7-8488-ee57d4aba2a8','.');
INSERT INTO t_name VALUES('assoc','764e6c3e-0e1d-4462-9dfc-a590123b2755','.');
INSERT INTO t_name VALUES('associations','670d0025-401e-4c78-9d8b-62fad2ad26ec','.');
INSERT INTO t_name VALUES('attr','e29d6d73-6a40-41bd-bfd4-df5183ffc764','.');
INSERT INTO t_name VALUES('attributes','b29bc8a3-1517-4a28-b566-39e447c80250','.');
INSERT INTO t_name VALUES('boolean','4a77f6cf-ee8d-43d5-8af4-b273594e23cd','.');
INSERT INTO t_name VALUES('box','dc6e0602-32b4-4416-a675-d45fb894f7eb','.');
INSERT INTO t_name VALUES('buffer','ebb71e9a-ef65-4fcd-aec9-f4a5e780e333','.');
INSERT INTO t_name VALUES('closure','f8a3568d-ae4a-4486-a362-577e0ec91474','.');
INSERT INTO t_name VALUES('cold_routine_emit','d7d288ae-7a8e-4709-99d0-6dfba1c9565f','.');
INSERT INTO t_name VALUES('cold_state','0403e866-c514-4ee1-aa47-fc8b476f937e','.');
INSERT INTO t_name VALUES('comment','1e299c1c-4d29-4616-b30b-258c72722484','.');
INSERT INTO t_name VALUES('conn','03c1ace3-3293-43bb-9541-19b4357cbc3c','.');
INSERT INTO t_name VALUES('content','9a05b9fa-17d3-4294-9253-e67406e52d4a','.');
INSERT INTO t_name VALUES('dictionnary','4871e619-947e-47da-bd0b-e7026f818072','.');
INSERT INTO t_name VALUES('doubles','e6727e08-6232-4beb-8c17-3b9cdf5045fb','.');
INSERT INTO t_name VALUES('exited','c3b77d8b-ff65-4717-b9d5-f2b5c22fca71','.');
INSERT INTO t_name VALUES('false_value','69cbd072-7d8e-4f62-bef4-5cd0cc847578','.');
INSERT INTO t_name VALUES('first_module','93abbd1c-2d33-4b86-8e16-e5b0c14842dc','.');
INSERT INTO t_name VALUES('frames','80e337da-7a05-40e4-86ec-806ac78dbd03','.');
INSERT INTO t_name VALUES('heart_beat','6dd4683e-cb4d-434b-b425-e45eed24d35e','.');
INSERT INTO t_name VALUES('itemref','18d95093-b523-4ee1-8bca-ded252e91235','.');
INSERT INTO t_name VALUES('json_array','d7a1e1a4-f401-436a-887f-980f66643a24','.');
INSERT INTO t_name VALUES('json_name','40dcfdf6-6005-4dc3-9947-c1743ab1c41e','.');
INSERT INTO t_name VALUES('json_object','11da23ab-3b9e-4ea0-bc13-4a9aa46e2995','.');
INSERT INTO t_name VALUES('jtype','b9664990-11a1-49a5-bf59-be714626ed6d','.');
INSERT INTO t_name VALUES('name','abdd3eea-22da-4e09-b1f8-a05b80783924','.');
INSERT INTO t_name VALUES('node','f99051be-30bf-4efc-8c35-b4344c4e6d2c','.');
INSERT INTO t_name VALUES('numbers','84961ab4-f2ff-4e10-9479-2e62148c54bc','.');
INSERT INTO t_name VALUES('proc_compilation','88aeaf2b-192e-4936-bdbb-b9a186d86cd4','.');
INSERT INTO t_name VALUES('queue','eaada55e-bcfb-4b87-b335-65b31ed2fdac','.');
INSERT INTO t_name VALUES('refitem','5e77140e-2467-4e85-aa28-8ca734c2aa07','.');
INSERT INTO t_name VALUES('routine','c5d8949e-8348-4166-82b8-5bf9ce3a6778','.');
INSERT INTO t_name VALUES('routine_emitter','8e3cbe87-244d-49e7-ac72-e6f8a09afd7c','.');
INSERT INTO t_name VALUES('routine_preparator','43714c87-bb83-4413-bc7b-41fc01976f76','.');
INSERT INTO t_name VALUES('routines','49c854e1-d91f-43b4-8a49-428fa8dceec4','.');
INSERT INTO t_name VALUES('set','3265e0f0-f3f7-4ef1-b157-50ec4fd4ce41','.');
INSERT INTO t_name VALUES('sons','8dba998c-341b-4a66-900f-545ac9dd06c4','.');
INSERT INTO t_name VALUES('space','566acfce-866a-4730-884d-30310b2fdf92','.');
INSERT INTO t_name VALUES('state','52d29f69-0bec-435b-af7c-f5cbeb51a80b','.');
INSERT INTO t_name VALUES('string','9ef8915a-d57e-42b5-8e6d-507470a69996','.');
INSERT INTO t_name VALUES('tasklet','5446420a-4d37-4505-91a1-4c3bc5db01c1','.');
INSERT INTO t_name VALUES('terminated','fa97d5b8-40cb-4884-9e77-27803c6a4deb','.');
INSERT INTO t_name VALUES('true_value','3b19b433-29b0-473e-9827-7756ee219573','.');
INSERT INTO t_name VALUES('tuple','c52ead7c-63bf-4e65-a873-486de4e4db41','.');
INSERT INTO t_name VALUES('uuid','a75bef70-596b-4fc5-99ca-e60b5287feec','.');
INSERT INTO t_name VALUES('val','d8aa61cc-2543-4659-bff9-2b0bfc0f779a','.');
INSERT INTO t_name VALUES('values','7e8b7206-2548-427d-9b7c-9d5b5067c78a','.');
INSERT INTO t_name VALUES('vector','402d1108-9e25-4065-b81d-1bea8b06f269','.');
INSERT INTO t_name VALUES('web_dictionnary','0229c553-c443-49a9-9244-5f8d7d3c79cf','.');
INSERT INTO t_param VALUES('dump_format_version','MoniMelt2014A');
COMMIT;
-- state-monimelt end dump 
