// file monimelt-script.js

/**   Copyright (C)  2014 Free Software Foundation, Inc.
      MONIMELT is a monitor for MELT - see http://gcc-melt.org/
      This file is part of GCC.
      
      GCC is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 3, or (at your option)
      any later version.
      
      GCC is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
      You should have received a copy of the GNU General Public License
      along with GCC; see the file COPYING3.   If not see
      <http://www.gnu.org/licenses/>.
**/

var websocket_mom;
var dochref_mom;
var wsockhref_mom;
// jquery ready function for our document
$(function(){
    dochref_mom = $(location).attr('href');
    console.debug ("href=", dochref_mom); // for example, href=http://localhost:8086/
    wsockhref_mom = dochref_mom.replace(/^http/,"ws") + "websock";
    console.debug ("wsockhref=", wsockhref_mom);
    websocket_mom = $.websocket(wsockhref_mom);
    console.debug ("websocket=", websocket_mom);
});
