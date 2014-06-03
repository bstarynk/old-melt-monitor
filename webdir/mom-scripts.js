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

var maindiv_mom;		// the main division
var tabdiv_mom;			// the tab division
// jquery ready function for our document
$(function(){
    maindiv_mom= $('#mom_maindiv');
    tabdiv_mom= $('#mom_tabdiv');
    console.debug("maindiv_mom=", maindiv_mom, " tabdiv_mom=", tabdiv_mom);
    $.ajax({ url: '/ajax_system',
	     method: 'POST',
	     data: { todo_mom: "mom_initial_system" },
	     dataType: 'html',
	     success: function (gotdata) {
		 console.debug("mom_initial_system ajax_system gotdata=", gotdata);
		 maindiv_mom.html(gotdata);
	     }
	   });
    tabdiv_mom.tabs({
	border: true
    });
});

function mom_do_menu_system(itm) {
    console.debug ("mom_do_menu_system itm=", itm);
    $.ajax({ url: '/ajax_system',
	     method: 'POST',
	     data: { todo_mom: itm.id },
	     dataType: 'html',
	     success: function (gotdata) {
		 console.debug("mom_do_menu_system ajax_system gotdata=", gotdata);
		 maindiv_mom.html(gotdata);
	     }
	   });
};

function mom_do_menu_objects(itm) {
    console.debug ("mom_do_menu_objects itm=", itm);
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: itm.id },
	     dataType: 'html',
	     success: function (gotdata) {
		 console.debug("mom_do_menu_objects ajax_objects gotdata=", gotdata);
		 maindiv_mom.html(gotdata);
	     }
	   });
}


function mom_install_editor(hdata) {
    console.debug ("mom_install_editor hdata=", hdata);
    tabdiv_mom.append(hdata);
}

function mom_add_editor_tab(divtab) {
    console.debug ("mom_add_editor_tab divtab=", divtab);
    tabdiv_mom.tabs("add",{
	title: divtab.attr('title'),
	content: divtab.html(),
	closable: true
    });
}

function mom_name_entry_selected(rec) {
    console.debug ("mom_name_entry_selected rec=", rec);
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: "mom_doeditnamed",
		     name_mom: rec.name,
		     id_mom: rec.id },
	     dataType: 'html',
	     success: function (gotdata) {
		 console.debug ("mom_name_entry_selected doeditnamed gotdata=", gotdata);
		 mom_install_editor(gotdata);
	     }
	   });
}


function mom_set_name_entry(inp)
{
    console.debug ("mom_set_name_entry inp=", inp);
    $(function(){
	console.debug ("mom_set_name_entry delayed inp=", inp);
	inp.combobox({
	    url:"ajax_complete_name",
	    valueField:'id',
	    textField:'name',
	    onSelect: mom_name_entry_selected
	});
	console.debug ("mom_set_name_entry delayed after combobox inp=", inp);
    });
}

function mom_make_named()
{
    var newinp = $('#mom_name_new');
    var comminp = $('#mom_comment');
    console.debug ("mom_make_named newinp.val=", newinp.val(), " comminp.val=", comminp.val());
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: "mom_domakenamed",
		     name_mom: newinp.val(),
		     comment_mom: comminp.val() },
	     dataType: 'html',
	     success: function (gotdata) {
		 console.debug ("mom_make_named gotdata=", gotdata);
		 mom_install_editor(gotdata);
	     }
	   });
}

function mom_erase_maindiv() {
    console.debug ("mom_erase_maindiv");
    maindiv_mom.empty();
}
    
