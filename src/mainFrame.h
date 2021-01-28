/*===================================================================================
                                 Parabolic Design App
                          Copyright Kerry R. Loux 2021

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  mainFrame.h
// Date:  1/22/2021
// Auth:  K. Loux
// Desc:  The main GUI class for the application.

#ifndef MAIN_FRAME_H_
#define MAIN_FRAME_H_

// Local headers
#include "parabolaCalculator.h"

// LibPlot2D headers
#include <lp2d/gui/guiInterface.h>

// wxWidgets headers
#include <wx/wx.h>

// Standard C++ headers
#include <vector>

// LibPlot2D forward declarations
namespace LibPlot2D
{
class PlotRenderer;
}

// The main frame class
class MainFrame : public wxFrame
{
public:
	MainFrame();
	~MainFrame() = default;

private:
	static const unsigned long long mHighQualityCurvePointLimit;

	LibPlot2D::GuiInterface mShapePlotInterface;
	LibPlot2D::GuiInterface mResponsePlotInterface;

	// Functions that do some of the frame initialization and control positioning
	void CreateControls();
	void SetProperties();

	LibPlot2D::PlotRenderer* CreatePlotArea(wxWindow *parent, LibPlot2D::GuiInterface& interface);
	wxWindow* CreateVersionText(wxWindow* parent);
	wxSizer* CreateTextInputs(wxWindow* parent);
	wxSizer* CreateTextOutputs(wxWindow* parent);
	
	ParabolaCalculator calculator;
	ParabolaCalculator::ParabolaInfo parabolaInfo;
	
	double paperWidth = 11.0;// [in]
	double paperHeight = 14.0;// [in]

	// Controls
	wxTextCtrl* diameterText;
	wxTextCtrl* focusPositionText;
	wxTextCtrl* numberOfFacetsText;
	
	wxTextCtrl* paperHeightText;
	wxTextCtrl* paperWidthText;

	wxStaticText* depthText;
	wxStaticText* minAmplifiedFrequencyText;
	wxStaticText* maxDesignErrorText;

	LibPlot2D::PlotRenderer *mShapePlotArea;
	LibPlot2D::PlotRenderer *mResponsePlotArea;

	// The event IDs
	enum MainFrameEventID
	{
		idWriteShape = wxID_HIGHEST + 500
	};

	void TextChangedEvent(wxCommandEvent& event);
	void OnWriteShapeClicked(wxCommandEvent& event);

	void UpdateCalculations();
	bool initialized = false;
	
	static std::unique_ptr<LibPlot2D::Dataset2D> ConvertToDataset(const ParabolaCalculator::Vector2DVectors& v);

	DECLARE_EVENT_TABLE();
};

#endif// MAIN_FRAME_H_
