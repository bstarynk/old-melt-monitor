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

var message_domelem;
var workzone_domelem;
var periodicstate_domelem;
var periodic_timeout;
var period_domelem;

function give_message(htmlmessage) {
    console.debug('give_message htmlmessage=', htmlmessage, ' message_domelem=', message_domelem);
    message_domelem.html(htmlmessage);
}

// jquery ready function for our document
$(function(){
    message_domelem = $('#mom_message_id');
    console.debug("message_domelem=", message_domelem);
    workzone_domelem = $('#workzone_id');
    console.debug("workzone_domelem=", workzone_domelem);
    period_domelem = $('#period_id');
    console.debug("period_domelem=", period_domelem);
    
    // if there is a momstart_id element, fill it by an Ajax query
    $.ajax({ url: '/ajax_start',
	     method: 'GET',
	     dataType: 'html',
	     success: function(data){
		 console.debug('ajax_start got data=', data);
		 $('#momstart_id').html(data);
	     }
	   });

    // install the exit menu
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
			give_message(d);
		    }});
	}});

    // install the named menu
    $('#named_drop_id').jui_dropdown({
	launcher_id: 'named_launcher_id',
	launcher_container_id: 'named_container_id',
	menu_id: 'named_menu_id',
	containerClass: 'menu_container_cl',
	menuClass: 'menu_cl',
	onSelect: function (event, data) {
	    console.debug ("nameddrop select event=", event, "; data=", data);
	    console.trace();
	    $.ajax({url: '/ajax_named',
		    method: 'POST',
		    data: data,
		    dataType: 'script',
		    success: function(data,status,qx) {
			console.debug ("nameddrop success data=", data, '; status=', status, ' qx=', qx);
			console.trace();
			eval(data);
			console.debug ("nameddrop success done data=", data);
		    }});
	}
    });
    

    // install the routine menu
    $('#routine_drop_id').jui_dropdown({
	launcher_id: 'routine_launcher_id',
	launcher_container_id: 'routine_container_id',
	menu_id: 'routine_menu_id',
	containerClass: 'menu_container_cl',
	menuClass: 'menu_cl',
	onSelect: function (event, data) {
	    console.debug ("routinedrop select event=", event, "; data=", data);
	    $.ajax({url: '/ajax_routine',
		    method: 'POST',
		    data: data,
		    dataType: 'html',
		    success: function(d) {
			console.debug ("routinedrop success d=", d);
			give_message (d);
		    }});
	}});

    periodicstate_domelem = $('#periodic_state_id');
    update_period();
    
});				// end of ready function


////////////////
function install_routine_completer(jq) {
    console.debug('install_routine_completer jq=', jq);
    $(jq).autocomplete({
	minLength: 2,
	source: "/ajax_complete_routine_name"
    });
}

function put_edited_routine(htmltitle) {
    console.debug('put_edited_routine htmltitle=', htmltitle);
    $('#toped_cl').html('<div class="routine_cl"><h3 id="routinetitle_id">routine <tt>'+htmltitle+'</tt></h3></div>');
}

////////////////
function install_create_named_form(datestr) {
    console.debug('install_create_named_form datestr=', datestr, ' workzone_domelem=', workzone_domelem);
    console.trace('install_create_named_form');
    console.debug('install_create_named_form datestr=', datestr);
     workzone_domelem
	.html('<b>create named:</b><input type="text" id="wcreatename_id" pattern="[A-Za-z_][A-Za-z0-9_]*" size="45"/>'
	      +'&nbsp; <b>comment:</b><input type="text" id="wcreatecomment_id" size="60"/>'
	      +'&nbsp; <b>type:</b><select id="wcreatetype_id">'
	      +'<option>assoc</option>'
	      +'<option>box</option>'
	      +'<option>buffer</option>'
	      +'<option>dictionnary</option>'
	      +'<option>json_name</option>'
	      +'<option>queue</option>'
	      +'<option>routine</option>'
	      +'<option>vector</option>'
	      +'</select>'
	      +'<br/><input type="submit" value="create" name="create named" onclick="send_create_named()"/>'
	      +'&nbsp; <input type="submit" value="cancel" name="cancel" onclick="erase_work_zone()"/>'
	     +'<br/><small>at <i>'+datestr+'</i></small>');
}




////////////////
function install_forget_named_form(datestr) {
    console.debug('install_forget_named_form datestr=', datestr, ' workzone_domelem=', workzone_domelem);
    console.trace('install_forget_named_form');
    workzone_domelem
	.html('<b>forget named:</b><input type="text" id="wforgetname_id" pattern="[A-Za-z_][A-Za-z0-9_]*" size="45"/>'
	      + '&nbsp; <input type="submit" name="do_forgetname" value="forget" onclick="send_forget_named();"/>\n'
	      + '&nbsp; <input type="submit" value="cancel" name="cancel" onclick="erase_work_zone()"/>'
	      +'<br/><small>at <i>'+datestr+'</i></small>');
    console.debug('install_forget_named_form before autocomplete ajax');
    $.ajax({ url: '/ajax_complete_name',
	     datatype: 'json',
	     method: 'POST',
	     success: function (resp) {
		 console.debug ('install_forget_named_form ajax_complete_name resp=', resp);
		 $('#wforgetname_id').autocomplete({
		     source: resp
		 });
		 console.debug ('install_forget_named_form ajax_complete_name autocomplete resp=', resp);
	     }});
}



function erase_work_zone() {
    console.debug('erase_work_zone workzone_domelem=', workzone_domelem);
    workzone_domelem.html('');
}

function send_create_named() {
    var nametxt = $('#wcreatename_id').val();
    var commtxt = $('#wcreatecomment_id').val();
    var typetxt = $('#wcreatetype_id').val();
    console.debug('send_create_named nametxt=', nametxt, ' commtxt=', commtxt, ' typetxt=', typetxt);
    $.ajax({ url: '/ajax_named',
	     method: 'POST',
	     dataType: 'html',
	     data: { id: 'do_create_named',
		     name: nametxt,
		     comment: commtxt,
		     type: typetxt },
	     success: function(data) {
		 console.debug('send_create_named got success data=',data);
		 erase_work_zone();
		 give_message(data);
	     },
	     error: function(xhr,status,err) {
		 var errtxt = xhr.responseText;
		 console.debug('send_create_named got error xhr=',xhr, ' status=', status, ' err=', err, ' errtxt=', errtxt);
		 give_message(errtxt);		 
	     }
	   });
}


function send_forget_named() {
    var forgnametxt = $('#wforgetname_id').val();
    console.debug('send_forget_named forgnametxt=', forgnametxt);
    $.ajax({ url: '/ajax_named',
	     method: 'POST',
	     dataType: 'html',
	     data: { id: 'do_forget_named',
		     name: forgnametxt },
	     success: function(data) {
		 console.debug('send_forget_named got success data=',data);
		 erase_work_zone();
		 give_message(data);
	     },
	     error: function(xhr,status,err) {
		 var errtxt = xhr.responseText;
		 console.debug('send_forget_named got error xhr=',xhr, ' status=', status, ' err=', err, ' errtxt=', errtxt);
		 give_message(errtxt);		 
	     }
	   });
}


function update_period() {
    var periodtxt=period_domelem.val();
    var periodmilli = 50 + parseInt(periodtxt)*1000;
    console.debug('update_period start periodtxt=', periodtxt, ' periodmilli=', periodmilli);
    $.ajax({ url: '/ajax_periodic',
	     method: 'POST',
	     dataType: 'html',
	     success: function (data) {
		 console.debug('update_period success data=', data);
		 periodicstate_domelem.html(data);
	     },
	     complete: function(jqx,status) {
		 console.debug('update_period complete jqx=', jqx, ' status=', status);
		 periodic_timeout = setTimeout(update_period, periodmilli);
	     }
	   });
    
}

function period_changed() {
    var periodtxt=period_domelem.val();
    console.debug('period_changed periodtxt=',period_domelem,
		  '; periodicstate_domelem=', periodicstate_domelem,
		  '; periodic_timeout=', periodic_timeout);
    if (periodic_timeout)
	clearTimeout(periodic_timeout);
    update_period();
}

function send_force_gc() {
    console.debug('send_force_gc');
    $.ajax({ url: '/ajax_periodic',
	     method: 'POST',
	     data: { id: 'do_force_gc' },
	     dataType: 'html',
	     success: function (data) {
		 console.debug('send_force_gc success data=', data);
		 periodicstate_domelem.html(data);
	     }
	   });
}
