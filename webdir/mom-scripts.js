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

    $('#exit_drop_id').jui_dropdown({
	launcher_id: 'exit_launcher_id',
	launcher_container_id: 'exit_container_id',
	menu_id: 'exit_menu_id',
	containerClass: 'menu_container_cl',
	menuClass: 'menu_cl',
	onSelect: function (event, data) {
	    console.debug ("exitdrop select event=", event, "; data=", data);
	    $.ajax({url: '/ajax_exit',
		    method: 'POST',
		    data: data,
		    dataType: 'html',
		    success: function(d) {
			console.debug ("exitdrop success d=", d);
			$('status_id').html(d);
		    }});
	}
    });
    
});

function install_routine_completer(jq) {
    console.debug('install_routine_completer jq=', jq);
    $(jq).autocomplete({
	minLength: 2,
	source: "/ajax_complete_routine_name"
    });
}

function give_message(htmlmessage) {
    console.debug('give_message htmlmessage=', htmlmessage);
    $('mom_message_id').html(htmlmessage);
}

function put_edited_routine(htmltitle) {
    console.debug('put_edited_routine htmltitle=', htmltitle);
    $('#toped_cl').html('<div class="routine_cl"><h3 id="routinetitle_id">routine <tt>'+htmltitle+'</tt></h3></div>');
}
