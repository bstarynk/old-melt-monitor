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
var edititemul_mom;
var editattrul_mom;
var commondl_mom;
var commonbufferdd_mom;
var curval_mom = null;
var curitem_mom = null;
var namescompletion_mom = null;
var addattrdlg_mom;
var addedattrinp_mom;

var system_menu_mom;
// jquery ready function for our document
$(function(){
    maindiv_mom = $('#mom_maindiv');
    tabdiv_mom = $('#mom_tabdiv');
    tabul_mom = $('#mom_tabul');
    editvalul_mom = $('#mom_editval_ul');
    edititemul_mom = $('#mom_edititem_ul');
    editattrul_mom = $('#mom_editattr_ul');
    commondl_mom = $('#mom_commondl');
    commonbufferdd_mom= $('#mom_commonbuffer_dd');
    addattrdlg_mom= $('#mom_addattr_dlg');
    addedattrinp_mom= $('#mom_addedattr_input');
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
	role: null
    });
    editvalul_mom.on("menuselect",function(ev,ui) {
	var idui= $(ui.item).attr("id");
	var datavaleditid= editvalul_mom.attr("data-momeditedval");
	console.debug ("editvalul menuselect ev=", ev, " ui=", ui,
		       " idui=", idui, " curval_mom=", curval_mom,
		       " datavaleditid=", datavaleditid);
	$.ajax({ url: '/ajax_edit',
 		 method: 'POST',
 		 data: { todo_mom: idui,
			 idval_mom: datavaleditid
		       },
		 dataType: 'json',
		 success: function (gotdata) {
		     console.debug("editvalul menuselect ajax_edit gotdata=", gotdata);
		     mom_ajax_edit_got(gotdata,ev,idui,curval_mom);
		 },
		 error: function (jq,status,errmsg) {
		     console.error ("editvalul menuselect ajax_edit error jq=", jq,
				    " status=", status,
				    " errmsg=", errmsg);
		 }
	       });
    });
    /////
    /// create the edititem menu
    edititemul_mom.menu({
	role: null
    });
    edititemul_mom.on("menuselect",function(ev,ui) {
	var idui= $(ui.item).attr("id");
	console.debug ("edititemul menu menuselect ev=", ev, " ui=", ui,
		       " idui=", idui, " curitem_mom=", curitem_mom);
	$.ajax({ url: '/ajax_edit',
 		 method: 'POST',
 		 data: { todo_mom: idui,
			 iditem_mom: curitem_mom.attr("data-momitemid")
		       },
		 dataType: 'json',
		 success: function (gotdata) {
		     console.debug("edititemul menuselect ajax_edit gotdata=", gotdata);
		     mom_ajax_edit_got(gotdata,ev,idui,curval_mom);
		 },
		 error: function (jq,status,errmsg) {
		     console.error ("edititemul menuselect ajax_edit error jq=", jq,
				    " status=", status,
				    " errmsg=", errmsg);
		 }
	       });
    });
    /////
    /// create the editattr menu
    editattrul_mom.menu({
	role: null
    });
    editattrul_mom.on("menuselect",function(ev,ui) {
	console.debug ("editattrul menu menuselect ev=", ev, " ui=", ui);
	var idui= $(ui.item).attr("id");
	var editorid = editattrul_mom.attr("data-momeditorid");
	console.debug ("editattrul menu menuselect idui=", idui, " editorid=", editorid);
	if (idui == 'mom_menuitem_editattr_add') 
	    mom_editor_attr_add(editorid);
    });
    /////
    /// create the addattr dialog and input
    addattrdlg_mom.dialog({
	autoOpen: false,
	open: function(ev, ui) {
	    addedattrinp_mom.autocomplete("option","source", mom_names_completion());
	},
	close: function (ev,ui) {
	    addattrdlg_mom.attr("data-momeditor",null);
	},
	modal: true,
	buttons: [
	    { text: 'Add',
	      click: function () {
		  var editorid= addattrdlg_mom.attr("data-momeditor");
		  var attrinp= addedattrinp_mom.val();
		  console.debug ("addattrdlg add editorid=", editorid, " attrinp=", attrinp);
		  $.ajax({ url: '/ajax_edit',
 			   method: 'POST',
 			   data: { todo_mom: 'mom_add_attribute',
				   editor_mom: editorid,
				   attr_mom: attrinp
				 },
			   error: function (jq,status,errmsg) {
			       console.error ("addattrdlg add ajax_edit error jq=", jq,
					      " status=", status,
					      " errmsg=", errmsg);
			   },
			   success: function (gotdata) {
			       console.debug ("addattrdlg add ajax_edit gotdata=", gotdata);
			       if (gotdata.momedit_do == 'momedit_dispnewattr') {
				   mom_add_new_attr(gotdata);
				   addattrdlg_mom.dialog("close");
			       }
			       else if (gotdata.momedit_do == 'momedit_badnewattr') {
				   attrinp.val("");
				   var warn = $('#mom_addedattr_input').after("<b class='mom_warning_cl'>bad attribute!</b>");
				   warn.delay(600).effect("fade").remove();
			       }
			       else console.error("addattrdlg add ajax_edit strange gotdatado:", gotdata.momedit_do);
			   }
			 });
	      }},
	    { text: 'Cancel',
	      click: function () {
		  $(this).dialog("close");
	      }}
	]
    });
    addedattrinp_mom.autocomplete({
	delay: 300,
	minLength: 2,
	source: mom_names_completion()
    });
    /////
    // initialize the tabs
    tabdiv_mom.tabs();
    tabdiv_mom.on('contextmenu', function(ev) {
	var valev = null;
	var itemev = null;
	// remove all but the first & second children (keep title & copy menuitems)
	editvalul_mom.children().slice(2).remove();
	valev = mom_containing_val($(ev.target));
	if (valev == null)
	    itemev = mom_containing_item($(ev.target));
	console.debug ("tabdiv_mom contextmenu ev=", ev,
		       " valev=", valev, "; curval_mom=", curval_mom,
		       " itemev=", itemev, "; curitem_mom=", curitem_mom);
	if (valev) {
	    mom_set_current_item(null,true);
	    mom_set_current_val(valev,true);
	    var valevid =  valev.attr('id');
	    console.debug ("tabdiv_mom contextmenu editvalul_mom=",
			   editvalul_mom, "; valev=", valev, "; valevid=", valevid);
	    //// make an synchronous ajax_edit call here, to ask what are the
	    //// possible editions...
	    $.ajax({ url: '/ajax_edit',
 		     method: 'POST',
		     async: false,
 		     data: { todo_mom: 'mom_prepare_editval_menu',
		             idval_mom: valevid
			   },
		     success: function (jdata) {
			 console.debug ("editval menuselect contextmenu prepared jdata=", jdata);
			 editvalul_mom.attr("data-momeditedval", valevid);
			 if (jdata.momedit_do == "momedit_add_to_editval_menu") {
			     var items = jdata.momedit_menuval;
			     console.debug ("editval menuselect contextmenu items=", items);
			     var nbitems = items.length;
			     for (var ix=0; ix<nbitems; ix++) {
				 var elem = items[ix];
				 console.debug ("editval menuselect contextmenu ix=", ix, " elem=", elem);
				 editvalul_mom.append(elem);
			     };
			     editvalul_mom.menu("refresh");
			 }
		     },
		     error:  function (jq,status,errmsg) {
			 console.error ("editval menuselect contextmenu prepared error jq=", jq,
					" status=", status,
					" errmsg=", errmsg);
		     }
		   });
	    editvalul_mom.css({top: ev.pageY, left: ev.pageX,
			       'z-index': 10000}).show();
	    $(document).one("click", function() {
		console.debug ("tabdiv_mom hiding editvalul");
		editvalul_mom.delay(350).hide();
		mom_set_current_val(valev,false);
	    });
	}
	else if (itemev) {
	    mom_set_current_val(null,true);
	    mom_set_current_item(itemev,true);
	    edititemul_mom.css({top: ev.pageY, left: ev.pageX,
				'z-index': 10000}).show();
	    console.debug ("tabdiv_mom contextmenu edititemul_mom=",
			   edititemul_mom, " itemev=", itemev);
	    $(document).one("click", function() {
		console.debug ("tabdiv_mom hiding editvalitem");
		edititemul_mom.delay(350).hide();
		console.debug ("tabdiv_mom hided editvalitem");
		mom_set_current_item(itemev,false);
	    });
	}
	else if ($(ev.target).parents(".mom_attrtitle_cl")) {
	    console.debug ("tabdiv_mom parents attrtitle ev=", ev, " target=", $(ev.target));
	    var editortab = $(ev.target).parents(".mom_editor_cl");
	    console.debug ("tabdiv_mom parents editortab=", editortab);
	    var editorid = editortab.attr("id").replace("momeditor","");
	    console.debug ("tabdiv_mom parents attrtitle editorid=", editorid);
	    mom_set_current_val(null,true);
	    mom_set_current_item(null,true);
	    editattrul_mom.css({top: ev.pageY, left: ev.pageX,
				'z-index': 10000}).show();
	    editattrul_mom.attr("data-momeditorid", editorid);
	    console.debug ("tabdiv_mom contextmenu editattrul_mom=",
			   editattrul_mom);
	    $(document).one("click", function() {
		console.debug ("tabdiv_mom hiding editattr");
		editattrul_mom.delay(350).hide();
		console.debug ("tabdiv_mom hided editattr");
		editattrul_mom.attr("data-momeditorid", null);
	    });
	    return false;
	}
	else {
	    mom_set_current_val(null,true);
	    mom_set_current_item(null,true);
	}
	return valev == null && itemev == null;
    });
    tabdiv_mom.on('mousemove', function(ev) {
	var valev = mom_containing_val($(ev.target));
	if (valev) {
	    mom_set_current_val(valev,false);
	    mom_set_current_item(null,false);
	} else {
	    var itmev = mom_containing_item($(ev.target));
	    if (itmev) {
		mom_set_current_item(itmev,false);
		mom_set_current_val(null,false);
	    }
	    else {
		mom_set_current_item(null,false);
		mom_set_current_val(null,false);
	    }
	}
    });
    ///// initialize the names completion
    mom_names_completion();
    ///// initial system request
    console.debug ("mom_initial_system before initial ajax_system");
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

function mom_names_completion()
{
    if (!namescompletion_mom) {
	console.debug ("mom_names_completion ajaxing");
	$.ajax({ url: '/ajax_complete_name',
		 method: 'POST',
		 async: false,
		 dataType: 'json',
		 success: function (gotjsarr) {
		     namescompletion_mom= gotjsarr;
		     console.debug ("mom_names_completion gotjsarr=", gotjsarr);
		 },
		 error:  function (jq,status,errmsg) {
		     console.error ("mom_names_completion failed jq=", jq,
				    " status=", status, " errmsg=", errmsg);
		 }
	       });
    }
    return namescompletion_mom;
}

/* When the "Obj -> Display Named" menu is selected [mom_menuitem_obj_dispnamed]
  The ajax_object is replying by creating the mom_name_input */
function mom_set_name_entry(inp)
{
    console.debug ("mom_set_name_entry inp=", inp, " before ajax_complete_name");
    var namcomp = mom_names_completion();
    console.debug ("mom_set_name_entry namcomp=", namcomp);
    inp.autocomplete({
	minLength: 3,
	delay: 250,
	source: namcomp
    });
}



// the "Obj -> Named" menu replied by creating the mom_name_input
// whose onChange calls this function
function mom_display_name_input_changed(inp)
{
    namescompletion_mom = null;
    console.debug ("mom_display_name_input_changed inp=", inp,
		   " of value=", inp.value, " before ajax_object mom_dodisplaynamed");
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: "mom_dodisplaynamed",
		     name_mom: inp.value
		   },
	     dataType: 'json',
	     success: function (gotdata) {
		 /** we are expecting a large JSON reply */
		 console.debug ("mom_display_name_input_changed  ajax_object mom_dodisplaynamed",
				" gotdata=", gotdata);
		 mom_install_display(gotdata);
	     },
	     error: function (jq,status,errmsg) {
		 /* should put the HTML message on the screen */
		 console.error ("mom_display_name_input_changed ajax_object mom_dodisplaynamed",
				" error jq=", jq, " status=", status, " errmsg=", errmsg);
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


function mom_install_display(jdata) {
    console.debug ("mom_install_display jdata=", jdata);
    var editorid = jdata.momeditorj_id;
    if (typeof(editorid) != "string")
	console.error("mom_install_display bad editorid=", editorid);
    var tabtitle = jdata.momeditorj_tabtitle;
    var tabcontent = jdata.momeditorj_tabcontent;
    mom_add_editor_tab(editorid,tabtitle,tabcontent);
    console.debug ("mom_install_display added editorid=", editorid,
		   " tabtitle=", tabtitle, " tabcontent=", tabcontent);
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


function mom_containing_item(elem) {
    if (elem == null) return null;
    if (typeof(elem.attr("data-momitemid")) == 'string')
	return elem;
    var par = elem.parent();
    if (typeof(par.attr("data-momitemid")) == 'string')
	return par;
    return null;
}


function mom_set_current_val(elem,strong)
{
    if (curval_mom == elem) {
	if (elem == null) return;
	if (strong) {
	    elem.addClass("mom_selvalue_cl");
	    elem.removeClass("mom_hovervalue_cl");
	}
	else {
	    elem.addClass("mom_hovervalue_cl");
	    elem.removeClass("mom_selvalue_cl");
	}
	return;
    }
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

function mom_set_current_item(elem,strong)
{
    if (curitem_mom == elem) {
	if (elem == null) return;
	if (strong) {
	    elem.addClass("mom_selitem_cl");
	    elem.removeClass("mom_hoveritem_cl");
	} else {
	    elem.addClass("mom_hoveritem_cl");
	    elem.removeClass("mom_selitem_cl");
	}
	return;
    };
    if (curitem_mom) {
	curitem_mom.removeClass("mom_selitem_cl");
	curitem_mom.removeClass("mom_hoveritem_cl");
	curitem_mom = null;
    }
    if (elem && elem.attr("data-momitemid")) {
	if (strong) elem.addClass("mom_selitem_cl");
	else elem.addClass("mom_hoveritem_cl");
	curitem_mom = elem;
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
    console.debug ("mom_add_editor_tab_id done tabdiv_mom=", tabdiv_mom);
    tabdiv_mom.tabs("refresh");
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
	mom_set_current_item(null,true);
	editortab= $('#'+editorid);
	console.debug ("mom_add_editor_tab wantremove editortab=", editortab);
	editortab.remove();
	pantab.remove();
	console.debug ("mom_add_editor_tab before ajax_object mom_doeditorclose",
		       " editorid=", editorid);
	$.ajax({ url: '/ajax_objects',
		 method: 'POST',
		 data: { todo_mom: "mom_doeditorclose",
			 closedid_mom: editorid },
		 dataType: 'html',
		 success: function (gotdata) {
		     console.debug ("mom_add_editor_tab doeditorclose gotdata=",
				    gotdata);
		     maindiv_mom.html(gotdata);
		 },
		 error: function (jq,status,errmsg) {
		     console.error ("mom_add_editor_tab  ajax_object doeditorclose",
				    " error jq=", jq, " status=", status, " errmsg=", errmsg);
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
	     },
	     error: function (jq,status,errmsg) {
		 console.error ("mom_before_close_editor_tab  ajax_object doeditorclose",
				    " error jq=", jq, " status=", status, " errmsg=", errmsg);
	     }
	   });
    return true;		// to permit the closing
}


/* this is the onclick function of an input tag dynamically output */
function mom_make_disp_named()
{
    var newinp = $('#mom_name_new');
    var comminp = $('#mom_comment');
    console.debug ("mom_make_disp_named newinp.val=", newinp.val(), " comminp.val=", comminp.val());
    namescompletion_mom = null;
    $.ajax({ url: '/ajax_objects',
	     method: 'POST',
	     data: { todo_mom: "mom_domakedispnamed",
		     name_mom: newinp.val(),
		     comment_mom: comminp.val() },
	     dataType: 'json',
	     success: function (gotdata) {
		 console.debug ("mom_make_disp_named gotdata=", gotdata);
		 mom_install_editor(gotdata);
	     },
	     error: function (jq,status,errmsg) {
		 console.error ("mom_make_disp_named ajax_object mom_domakenamed",
				" error jq=", jq, " status=", status, " errmsg=", errmsg);
	     }
	   });
}


function mom_erase_maindiv() {
    console.debug ("mom_erase_maindiv");
    maindiv_mom.empty();
}
    
function mom_ajax_edit_got(jdata,ev,idui,elem)
{
    console.debug("mom_ajax_edit_got jdata=", jdata, " ev=", ev,
		  " idui=", idui, " elem=", elem);
    if (jdata.momedit_do == "momedit_copytobuffer") {
	var editorsid = jdata.momedit_editors_id;
	var content = jdata.momedit_content;
	console.debug("mom_ajax_edit_got copytobuffer content=", content,
		      "; commonbufferdd_mom=", commonbufferdd_mom);
	commonbufferdd_mom.html(content);
	console.debug("mom_ajax_edit_got copytobuffer set commonbufferdd_mom=",
		      commonbufferdd_mom);
    }
    else if (jdata.momedit_do == "momedit_replaceinput") {
	var oldid = jdata.momedit_oldid;
	var newid = jdata.momedit_newid;
	console.debug ("mom_ajax_edit_got momedit_replaceinput oldid=", oldid,
		       " newid=", newid);
	var newinphtml = "<input class='mom_newvalinput_cl' type='text' id='" + newid +"'/>";
	$('#' + oldid).replaceWith(newinphtml);
	var newinp = $('#' + newid);
	mom_install_new_input(newinp,newid);
	console.debug ("mom_ajax_edit_got momedit_replaceinput final newinp=", newinp);
    }
}

function mom_install_new_input(newinp,newid)
{
    // should be autocompleted and notify its input
    var namescompl = mom_names_completion();
    var nbcompl = namescompl.length;
    console.debug ("mom_install_new_input nbcompl=", nbcompl,
		   " newinp=", newinp, " newid=", newid);
    newinp.autocomplete({
	minLength: 3,
	delay: 250,
	source: function (requ,responsefun) {
	    console.debug ("mom_install_new_input source newinp=", newinp,
			   " requ=", requ, " responsefun=", responsefun);
	    console.debug ("mom_install_new_input source newid=", newid,
			   " requ=", requ, " responsefun=", responsefun);
	    var curterm = requ.term;
	    var lastwordregexp = /([A-Za-z]\w*)$/;
	    if (curterm.match(lastwordregexp)) {
		var lastword = RegExp.$1;
		var lastwordlen = lastword.length;
		var termprefix = curterm.slice(0, curterm.length - lastwordlen);
		console.debug ("mom_install_new_input lastword=", lastword,
			       " termprefix=", termprefix);
		var subnames = namescompl.filter (function (nam,ix,_) {
		    return nam.length>=lastwordlen && nam.slice(0, lastwordlen)==lastword;
		});
		console.debug ("mom_install_new_input subnames=", subnames);
		var rescomp = subnames.map(function(nam,ix,_){return {label:nam, value:termprefix+nam};});
		console.debug ("mom_install_new_input rescomp=", rescomp);
		responsefun(rescomp);
	    }
	    else {
		console.debug ("mom_install_new_input same curterm=", curterm);
		responsefun([curterm]);
	    }
	},
	change: function (ev, ui) {
	    console.debug ("mom_install_new_input change ev=", ev,
			   " ui=", ui, " val=", newinp.val(), " newinp=", newinp);
	    $.ajax({ url: '/ajax_edit',
		     method: 'POST',
		     data: { todo_mom: "momedit_newinput",
			     idval_mom: newinp.attr("id"),
			     input_mom: newinp.val() },
		     dataType: 'json',
		     success: function (gotdata) {
			 console.debug ("mom_install_new_input change gotdata=", gotdata);
			 mom_ajax_edit_input (gotdata, newinp);
		     },
		     error: function (jq,status,errmsg) {
			 console.error ("mom_install_new_input change",
					" error jq=", jq, " status=", status, " errmsg=", errmsg);
		     }
		   });
	}
    });
    newinp.tooltip({tooltipClass: "mom_newvaltooltip_cl",
		    content: "<b>value input:</b> e.g.<br/><ul>"
		    +"<li><tt class='mom_example_cl'>agenda</tt> for an <i>item</i></li>"
		    +"<li><tt class='mom_example_cl'>-12</tt> for an <i>integer</i></li>"
		    +"<li><tt class='mom_example_cl'>\"ab c</tt> for a <i>string</i></li>"
		    +"<li><tt class='mom_example_cl'>*attr</tt> for a <i>node</i> of given connective <code>attr</code></li>"
		    +"</ul>",
		    items:'#'+newid});
    newinp.on("input", function (ev) {
	console.debug ("mom_install_new_input input newinp=", newinp, " input ev=", ev);
    });
}

function mom_ajax_edit_input(jdata,inp) {
    console.debug ("mom_ajax_edit_input jdata=", jdata, " inp=", inp);
}


function mom_editor_attr_add(editorid) {
    console.debug ("mom_editor_attr_add editorid=", editorid);
    addattrdlg_mom.attr("data-momeditor",editorid);
    addattrdlg_mom.dialog("open");
}

function mom_editor_add_update_buttons(editorid) {
    var editordiv = $('#momeditor'+editorid);
    console.debug ("mom_editor_show_update_buttons editorid=", editorid, " editordiv=", editordiv);
    // dont add the buttons if they are already here.
    if (editordiv.find(".mom_editbuttons_cl"))
	return;
    // find the mom_editdate_cl inside the editor
    var editdate= editordiv.find(".mom_editdate_cl");
    console.debug ("mom_editor_show_update_buttons editdate=", editdate);
    editdate.after("<input type='submit' class='mom_editbuttons_cl' name='update' value='Update' onclick='mom_editor_update(\""
		   + editorid + "\")'/>"
		  + "<input type='submit' class='mom_editbuttons_cl' name='revert' value='Revert' onclick='mom_editor_revert(\""
		   + editorid + "\")'/>");
}

function mom_editor_update(editorid) {
    var editordiv = $('#momeditor'+editorid);
    console.debug ("mom_editor_update editorid=", editorid, " editordiv=", editordiv);
}

function mom_editor_revert(editorid) {
    var editordiv = $('#momeditor'+editorid);
    console.debug ("mom_editor_revert editorid=", editorid, " editordiv=", editordiv);
}

function mom_add_new_attr(jdata) {
    console.debug ("mom_add_new_attr jdata=", jdata);
    var editorid=jdata.momedit_editorid;
    var attrid=jdata.momedit_attrid;
    var inputid=jdata.momedit_inputid;
    var attrlihtml=jdata.momedit_attrlihtml;
    var editordiv = $('#momeditor'+editorid);
    // get the mom_attrlist_cl inside the mom_attributes_cl inside the editordiv
    console.debug("mom_add_new_attr editordiv=", editordiv, " jdata=", jdata);
    var attrlist = editordiv.find('.mom_attributes_cl').find('.mom_attrlist_cl');
    attrlist.append(attrlihtml);
    var newinp = $('#'+inputid);
    mom_install_new_input(newinp,newvalid);
    mom_editor_add_update_buttons(editorid);
}
