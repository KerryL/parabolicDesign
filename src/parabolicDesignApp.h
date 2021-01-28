/*===================================================================================
                                 Parabolic Design App
                          Copyright Kerry R. Loux 2021

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  parabolicDesignApp.h
// Date:  1/2/2021
// Auth:  K. Loux
// Desc:  The application class.

#ifndef PARABOLIC_DESIGN_APP_H_
#define PARABOLIC_DESIGN_APP_H_

// wxWidgets headers
#include <wx/wx.h>

// Local forward declarations
class MainFrame;

// The application class
class ParabolicDesignApp : public wxApp
{
public:
	// Initialization function
	bool OnInit();

	// The name of the application
	static const wxString appTitle;// As displayed
	static const wxString appName;// Internal
	static const wxString creator;
	static const wxString versionString;
	static const wxString gitHash;

private:
	// The main class for the application - this object is the parent for all other objects
	MainFrame *mainFrame = nullptr;
};

// Declare the application object (have wxWidgets create the wxGetApp() function)
DECLARE_APP(ParabolicDesignApp);

#endif// PARABOLIC_DESIGN_APP_H_
