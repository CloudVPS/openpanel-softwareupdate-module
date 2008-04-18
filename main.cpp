// --------------------------------------------------------------------------
// OpenPanel - The Open Source Control Panel
// Copyright (c) 2006-2007 PanelSix
//
// This software and its source code are subject to version 2 of the
// GNU General Public License. Please be aware that use of the OpenPanel
// and PanelSix trademarks and the IconBase.com iconset may be subject
// to additional restrictions. For more information on these restrictions
// and for a copy of version 2 of the GNU General Public License, please
// visit the Legal and Privacy Information section of the OpenPanel
// website on http://www.openpanel.com/
// --------------------------------------------------------------------------

#include "softwareupdateModule.h"
#include <grace/filesystem.h>

#define PATH_DB "/var/opencore/db"
#define PATH_CACHEFILE PATH_DB "/softwareupdate.db"
#define PATH_LISTFILE PATH_DB "/updatelist.db"
#define PATH_STAGING "/var/opencore/conf/staging/SoftwareUpdate"

APPOBJECT(softwareupdateModuleApp);

//  =========================================================================
/// Main method.
//  =========================================================================
int softwareupdateModuleApp::main (void)
{
	caseselector (data["OpenCORE:Command"])
	{
		incaseof ("listobjects") :
			listobjects ();
			break;
		
		incaseof ("update") :
			updateObject (data);
			break;
			
		incaseof ("getconfig") :
			sendRootObject ();
			break;
			
		incaseof ("callmethod") :
			if (data["OpenCore:Session"]["method"] == "installupdates")
			{
				authd.osupdate ();
				sendresult (moderr::ok, "OK");
			}
			else
			{
				sendresult (moderr::err_command, "Unknown method");
			}
		
		defaultcase :
			sendresult (moderr::err_command, "Unknown command");
			return 0;
	}
	
	return 0;
}

void softwareupdateModuleApp::sendRootObject (void)
{
	value out;
	out["SoftwareUpdate"]("type") = "class";
	out[-1]["updates"]("type") = "object";
	out[-1][-1]["defaultaction"] = true;
	sendresult (moderr::ok, "OK", out);
}

void softwareupdateModuleApp::listobjects (void)
{
	if (! fs.exists ("/var/opencore/db/softwareupdate.db"))
	{
		sendresult (moderr::ok, "OK");
		return;
	}
	
	bool selectionupdate = false;
	
	value v, selection;
	v.loadshox (PATH_CACHEFILE);
	selection.loadshox (PATH_LISTFILE);
	
	if (! selection.exists ("default")) selection["default"] = true;
	
	value res;
	
	foreach (update, v)
	{
		value out = update;
		string deplist;
		out.rmval ("deps");
		out["newversion"] = out["version"];
		out.rmval ("version");
		
		foreach (dep, update["deps"])
		{
			if (deplist) deplist.strcat (", ");
			deplist.strcat (dep.id().sval());
		}
		if (deplist) out["deps"] = deplist;

		if (selection["flags"].exists (update.id()))
		{
			out["enabled"] = selection["flags"][update.id()].bval();
		}
		else
		{
			out["enabled"] = selection["default"].bval();
		}
		
		res["objects"]["SoftwareUpdate:UpdatePackage"]("type") = "class";
		res["objects"]["SoftwareUpdate:UpdatePackage"][update.id()] = out;
		res["objects"]["SoftwareUpdate:UpdatePackage"][update.id()]("type") = "object";
	}
	
	if (selectionupdate)
	{
		selection.saveshox (PATH_STAGING "/updatelist.db");
		if (authd.installfile ("updatelist.db", PATH_DB))
		{
			sendresult (moderr::err_writefile, "Error installing update list");
			return;
		}
	}
	
	authd.quit ();
	sendresult (moderr::ok, "OK", res);
}

void softwareupdateModuleApp::updateObject (const value &args)
{
	value v, selection;
	v.loadshox (PATH_CACHEFILE);
	selection.loadshox (PATH_STAGING "/updatelist.db");
	
	if (args["OpenCORE:Context"] == "SoftwareUpdate")
	{
		selection["default"] = args["SoftwareUpdate"]["defaultaction"].bval();
		selection.saveshox (PATH_STAGING "/updatelist.db");
		if (authd.installfile ("updatelist.db", PATH_DB))
		{
			authd.quit ();
			sendresult (moderr::err_writefile, "Error installing update list");
			return;
		}
		
		authd.quit ();
		sendresult (moderr::ok, "OK");
		return;
	}
	
	if (! args.exists ("SoftwareUpdate:UpdatePackage"))
	{
		sendresult (moderr::err_context, "Missing UpdatePackage object");
		return;
	}
	
	const value &obj = args["SoftwareUpdate:UpdatePackage"];
	statstring objid = obj["id"];
	
	if (! selection.exists ("default")) selection["default"] = true;
	
	if (v.exists (objid))
	{
		trackdependency (selection["flags"], v, objid, obj["enabled"].bval());

		selection.saveshox (PATH_STAGING "/updatelist.db");
		if (authd.installfile ("updatelist.db", PATH_DB))
		{
			sendresult (moderr::err_writefile, "Error installing update list");
			return;
		}
	}
	
	authd.quit ();
	sendresult (moderr::ok, "OK");
}

void softwareupdateModuleApp::trackdependency (value &selection,
											   const value &db,
											   const statstring &dep,
											   bool enabled)
{
	if (db.exists (dep))
	{
		selection[dep] = enabled;
		if (db[dep].exists ("deps")) foreach (subdep, db[dep]["deps"])
		{
			if ((! selection.exists (subdep.id())) ||
				(selection[subdep.id()].bval() != enabled))
				trackdependency (selection, db, subdep.id(), enabled);
		}
	}
}
