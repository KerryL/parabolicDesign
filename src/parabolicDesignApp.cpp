/*===================================================================================
                                 Parabolic Design App
                          Copyright Kerry R. Loux 2021

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  parabolicDesignApp.cpp
// Date:  1/2/2021
// Auth:  K. Loux
// Desc:  The application class.

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "parabolicDesignApp.h"
#include "mainFrame.h"

// Implement the application (have wxWidgets set up the appropriate entry points, etc.)
IMPLEMENT_APP(ParabolicDesignApp);

//==========================================================================
// Class:			ParabolicDesignApp
// Function:		Constant Declarations
//
// Description:		Constant declarations for the ParabolicDesignApp class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
const wxString ParabolicDesignApp::appTitle = _T("Parabolic Reflector Designer");
const wxString ParabolicDesignApp::appName = _T("ParabolicDesignApplication");
const wxString ParabolicDesignApp::creator = _T("Kerry Loux");
// gitHash and versionString are defined in gitHash.cpp, which is automatically generated during the build

//==========================================================================
// Class:			ParabolicDesignApp
// Function:		OnInit
//
// Description:		Initializes the application window.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool = true for successful window initialization, false for error
//
//==========================================================================
bool ParabolicDesignApp::OnInit()
{
	SetAppName(appName);
	SetVendorName(creator);

	mainFrame = new MainFrame();

	if (!mainFrame)
		return false;

	mainFrame->Show(true);

	return true;
}
