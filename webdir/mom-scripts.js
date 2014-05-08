// file melt-script.js

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
// jquery ready function for our document
$(function(){
    // if there is a momstart_id element, fill it by an Ajax query
    $.ajax({ url: '/ajax_start',
	     method: 'GET',
	     dataType: 'html',
	     success: function(data){
		 console.debug('ajax_start got data=', data);
		 $('#momstart_id').html(data);
	     }
	   });
});
