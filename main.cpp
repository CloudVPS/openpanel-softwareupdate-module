// This file is part of OpenPanel - The Open Source Control Panel
// OpenPanel is free software: you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation, using version 3 of the License.
//
// Please note that use of the OpenPanel trademark may be subject to additional 
// restrictions. For more information, please visit the Legal Information 
// section of the OpenPanel website on http://www.openpanel.com/


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
	int count = 0;
	
	foreach (update, v)
	{
		count++;
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
		
		out["uuid"] = "5973053a-2cd1-41a9-8ff8-00000000%04x" %format (count);
		out["metaid"] = update.id();
		oud["id"] = update.id();
		
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
	string oid = obj["id"];
	
	if (oid.strncmp ("5973053a", 8) == 0)
	{
		oid.cropafterlast ("-");
		int pos = oid.toint (16);
		oid = v[pos].id();
	}
	
	statstring objid = oid;
	
	
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
