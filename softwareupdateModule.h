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

#ifndef _softwareupdatemodule_H
#define _softwareupdatemodule_H 1

#include <opencore/moduleapp.h>
#include <grace/application.h>

//  -------------------------------------------------------------------------
/// Main application class.
//  -------------------------------------------------------------------------
class softwareupdateModuleApp : public moduleapp
{
public:
		 	 softwareupdateModuleApp (void) :
				moduleapp ("openpanel.module")
			 {
			 }
			~softwareupdateModuleApp (void)
			 {
			 }

	int		 main (void);
	void	 listobjects (void);
	void	 updateObject (const value &);
	void	 sendRootObject (void);
	void	 trackdependency (value &, const value &,
							  const statstring &, bool);
};

#endif
