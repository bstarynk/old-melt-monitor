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
var tabul_mom;
var editvalul_mom;
var curval_mom;

var system_menu_mom;
// jquery ready function for our document
$(function(){
    maindiv_mom = $('#mom_maindiv');
    tabdiv_mom = $('#mom_tabdiv');
    tabul_mom = $('#mom_tabul');
    editvalul_mom = $('#mom_editval_ul');
    //
    // create the system button with its menu
    var systembut = $('#mom_system_but');
    var systemul = $('#mom_system_ul');
    ///
    systembut.button({
	text: true,
	icons: {
	    primary: "ui-icon-triangle-1-s"
	}
    });
    systemul.menu({
	select: function (ev, ui) {
	    var idui= $(ui.item).attr("id");
	    console.debug ("systemul menu select ev=", ev,
			   " ui=", ui, " idui=", idui);
	    $.ajax({ url: '/ajax_system',
 		     method: 'POST',
 		     data: { todo_mom: idui },
		     dataType: 'html',
		     success: function (gotdata) {
			 console.debug ("systemul ajax gotdata=", gotdata);
			 maindiv_mom.html(gotdata);
			 systemul.delay(500).hide();
		     }
		   });
	},
	role: null 
    });
    systembut.click(function (ev) {
	console.debug ("systembut click ev=", ev);
	systemul.toggle().position({ my: "left top", at: "left bottom", of: this });
    });
    //////
    // create the object button with its menu
    var objectbut = $('#mom_object_but');
    var objectul = $('#mom_object_ul');
    objectbut.button({
	text: true,
	icons: {
	    primary: "ui-icon-triangle-1-s"
	}
    });
    objectul.menu({
	select: function (ev, ui) {
	    var idui= $(ui.item).attr("id");
	    console.debug ("objectul menu select ev=", ev, " ui=", ui,
			   " idui=", idui);
	    $.ajax({ url: '/ajax_objects',
 		     method: 'POST',
 		     data: { todo_mom: idui },
		     dataType: 'html',
		     success: function (gotdata) {
			 console.debug ("objectul ajax gotdata=", gotdata);
			 maindiv_mom.html(gotdata);
			 objectul.delay(500).hide();
		     }
		   });
	},
	role: null
    });
    objectbut.click(function (ev) {
	console.debug ("objectbut click ev=", ev);
	objectul.toggle().position({ my: "left top", at: "left bottom", of: this });
    });
    /////
    /// create the editval menu
    editvalul_mom.menu({
	select: function (ev, ui) {
	    var idui= $(ui.item).attr("id");
	    console.debug ("editvalul menu select ev=", ev, " ui=", ui,
			   " idui=", idui);
	},
	role: null
    });
    /////
    // initialize the tabs
    tabdiv_mom.tabs();
    tabdiv_mom.on('contextmenu', function(ev) {
	var valev = mom_containing_val($(ev.target));
	console.debug ("tabdiv_mom contextmenu ev=", ev,
		       " valev=", valev, "; curval_mom=", curval_mom);
	if (valev) {
	    mom_set_current_val(valev,true);
	    editvalul_mom.css({top: ev.pageY, left: ev.pageX,
			       'z-index': 10000}).show();
	    console.debug ("tabdiv_mom contextmenu editvalul_mom=",
			   editvalul_mom);
	    $(document).one("click", function() {
		console.debug ("tabdiv_mom hiding editvalul");
		editvalul_mom.hide();
	    });
	}
	else mom_set_current_val(null,true);
	return valev == null;
    });
    tabdiv_mom.on('mousemove', function(ev) {
	var valev = mom_containing_val($(ev.target));
	mom_set_current_val(valev,false);
    });
    ///// initial system request
    $.ajax({ url: '/ajax_system',
 	     method: 'POST',
 	     data: { todo_mom: "mom_initial_system" },
 	     dataType: 'html',
 	     success: function (gotdata) {
 		 console.debug("mom_initial_system ajax_system gotdata=", gotdata);
 		 maindiv_mom.html(gotdata);
 	     }
 	   });
    //
});

/* When the "Obj -> Named" menu is selected [mom_menuitem_obj_named]
  The ajax_object is replying by creating the mom_name_input, ended by
  a <script> element invoking the following function: */
function mom_set_name_entry(inp)
{
    console.debug ("mom_set_name_entry inp=", inp);
    $.ajax({ url: '/ajax_complete_name',
	     method: 'POST',
	     dataType: 'json',
	     success: function (gotjsarr) {
		 console.debug ("mom_set_name_entry completename gotjsarr=",
				gotjsarr);
		 inp.autocomplete({
		     source: gotjsarr
		 });
	     }
	   });
}

// the "Obj -> Named" menu replied by creating the mom_name_input
// whose onChange calls this function
function mom_name_input_changed(inp)
{
    console.debug ("mom_name_input_changed inp=", inp,
		   " of value=", inp.value);
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     dataType: 'json',
	     data: { todo_mom: "mom_doeditnamed",
		     name_mom: inp.value
		   },
	     success: function (gotdata) {
		 /** we are expecting a large JSON reply */
		 console.debug ("mom_name_input_changed gotdata=", gotdata);
		 mom_install_editor(gotdata);
	     }
	   });
}

function mom_do_menu_valedit(itm) {
    console.debug ("mom_do_menu_valedit itm=", itm);
}

/** The json data is built in ajax_objects routine file routines.c,
    starting at beginedit state; we expect something like
   { momeditorj_id: "_0u15i1z87ei_jf2wwmpim72",       // editor id
     momeditorj_tabtitle: "<span...",      // the HTML for the tab title
     momeditorj_tabcontent: "<div..."      // the HTML for the tab content
   }
**/
function mom_install_editor(jdata) {
    console.debug ("mom_install_editor jdata=", jdata);
    var editorid = jdata.momeditorj_id;
    if (typeof(editorid) != "string")
	console.error("mom_install_editor bad editorid=", editorid);
    var tabtitle = jdata.momeditorj_tabtitle;
    var tabcontent = jdata.momeditorj_tabcontent;
    mom_add_editor_tab(editorid,tabtitle,tabcontent);
}

function mom_containing_val(elem) {
    if (elem == null) return null;
    if (elem.hasClass("mom_value_cl")) return elem;
    else {
	var elc = elem.parents(".mom_value_cl:first");
	if (elc.hasClass("mom_value_cl")) return elc;
	else return null;
    }
}


function mom_containg_item(elem) {
    if (elem == null) return null;
    if (typeof(elem.attr("data-momitemid")) == 'string')
	return elem;
    var par = elem.parent();
    if (typeof(par.attr("data-momitemid")) == 'string')
	return par;
    return null;
}

var curval_mom;

function mom_set_current_val(elem,strong)
{
    if (curval_mom == elem) return;
    if (curval_mom) {
	curval_mom.removeClass("mom_selvalue_cl");
	curval_mom.removeClass("mom_hovervalue_cl");
	curval_mom = null;
    }
    if (elem && elem.hasClass("mom_value_cl")) {
	if (strong) elem.addClass("mom_selvalue_cl");
	else elem.addClass("mom_hovervalue_cl");
	curval_mom = elem;
    }
}

function mom_add_editor_tab(editorid, tabtitle, tabcontent) {
    console.debug ("mom_add_editor_tab_id editorid=", editorid,
		   "; tabtitle=", tabtitle,
		   ";\n tabcontent=", tabcontent);
    var divtabid = "momeditab" + editorid;
    var tablistr = "<li id='" + divtabid
	+ "' class='mom_tabtitle_cl'><a href='#momeditor"  + editorid
	+ "'>" + tabtitle
	+ "</a><span class='ui-icon ui-icon-close' role='presentation'>Untab</span>"
	+ "</li>";
    console.debug ("mom_add_editor_tab tablistr=", tablistr);
    divtab = $(tabcontent);
    console.debug ("mom_add_editor_tab divtab=", divtab);
    tabul_mom.append(tablistr);
    tabdiv_mom.append(divtab);
    var divindex = divtab.index();
    console.debug ("mom_add_editor_tab_id divindex=", divindex);
    tabdiv_mom.tabs("refresh");
    console.debug ("mom_add_editor_tab_id done tabdiv_mom=", tabdiv_mom);
    tabdiv_mom.tabs("option","active",divindex);
    tabdiv_mom.delegate( "span.ui-icon-close", "click", function(ev) {
	var pantab = $(this).closest("li");
	console.debug ("mom_add_editor_tab wantremove ev=", ev,
		       "; pantab=", pantab);
	var panid = pantab.attr('id');
	var editorid = panid.replace("momeditab","momeditor");
	console.debug ("mom_add_editor_tab wantremove panid=", panid,
		       " editorid=", editorid);
	mom_set_current_val(null,true);
	editortab= $('#'+editorid);
	console.debug ("mom_add_editor_tab wantremove editortab=", editortab);
	editortab.remove();
	pantab.remove();
	$.ajax({ url: '/ajax_objects',
		 method: 'POST',
		 data: { todo_mom: "mom_doeditorclose",
			 closedid_mom: editorid },
		 dataType: 'html',
		 success: function (gotdata) {
		     console.debug ("mom_before_close_editor_tab doeditorclose gotdata=",
				    gotdata);
		     maindiv_mom.html(gotdata);
		 }
	       });
	tabdiv_mom.tabs("refresh");	
    });
}


function mom_before_close_editor_tab(title,index) {
    console.debug ("mom_before_close_editor_tab title=", title, " index=", index);
    var closedtab = tabdiv_mom.tabs("getTab", index);
    var closedid = closedtab.attr("id");
    console.debug ("mom_before_close_editor_tab closedtab=", closedtab, " closedid=", closedid);
    var editorid = closedid.replace ("momeditab","momeditor");
    console.debug ("mom_before_close_editor_tab editorid=", editorid);
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: "mom_doeditorclose",
		     closedid_mom: editorid },
	     dataType: 'html',
	     success: function (gotdata) {
		 console.debug ("mom_before_close_editor_tab doeditorclose gotdata=",
				gotdata);
		 maindiv_mom.html(gotdata);
	     }
	   });
    return true;		// to permit the closing
}

function mom_name_entry_selected(rec) {
    console.debug ("mom_name_entry_selected rec=", rec);
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: "mom_doeditnamed",
		     name_mom: rec.name,
		     id_mom: rec.id },
	     dataType: 'json',
	     success: function (gotdata) {
		 console.debug ("mom_name_entry_selected doeditnamed gotdata=", gotdata);
		 mom_install_editor(gotdata);
	     }
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
	     dataType: 'json',
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
    
