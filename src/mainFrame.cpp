/*===================================================================================
                                 Parabolic Design App
                          Copyright Kerry R. Loux 2021

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  mainFrame.cpp
// Date:  1/22/2021
// Auth:  K. Loux
// Desc:  The main GUI class for the application.

// Standard C++ headers
#include <algorithm>

// Local headers
#include "mainFrame.h"
#include "parabolicDesignApp.h"
#include "latexGenerator.h"

// LibPlot2D headers
#include <lp2d/renderer/plotRenderer.h>
#include <lp2d/utilities/guiUtilities.h>
#include <lp2d/libPlot2D.h>

// wxWidgets headers
#include <wx/valnum.h>

//==========================================================================
// Class:			MainFrame
// Function:		Constant declarations
//
// Description:		Constant declarations for MainFrame class.
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
const unsigned long long MainFrame::mHighQualityCurvePointLimit(10000);

//==========================================================================
// Class:			MainFrame
// Function:		MainFrame
//
// Description:		Constructor for MainFrame class.  Initializes the form
//					and creates the controls, etc.
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
MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, wxEmptyString,
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE), mShapePlotInterface(this), mResponsePlotInterface(this)
{
	CreateControls();
	SetProperties();
	initialized = true;

	TransferDataFromWindow();
	UpdateCalculations();// TODO:  For some reason this breaks under MSW but works under Linux?  Canvas not yet shown on screen?
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateControls
//
// Description:		Creates sizers and controls and lays them out in the window.
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
void MainFrame::CreateControls()
{
	wxSizer *topSizer(new wxBoxSizer(wxHORIZONTAL));
	wxPanel *panel(new wxPanel(this));
	topSizer->Add(panel, wxSizerFlags().Expand());

	wxSizer *leftSizer(new wxBoxSizer(wxVERTICAL));
	panel->SetSizer(leftSizer);

	wxSizer *sizer(new wxBoxSizer(wxVERTICAL));
	sizer->Add(CreateTextInputs(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	sizer->Add(CreateTextOutputs(panel), wxSizerFlags().Border(wxALL, 5).Expand());
	leftSizer->Add(sizer, wxSizerFlags().Expand().Border(wxALL, 5));
	
	leftSizer->Add(new wxButton(panel, idWriteShape, _T("Save Template")), wxSizerFlags().Border(wxALL, 5));
	leftSizer->AddStretchSpacer();
	
	leftSizer->Add(CreateVersionText(panel), wxSizerFlags().Border(wxALL, 5));

	wxSizer *plotSizer(new wxBoxSizer(wxVERTICAL));
	topSizer->Add(plotSizer, wxSizerFlags().Expand().Proportion(1));
	
	mShapePlotArea = CreatePlotArea(this, mShapePlotInterface);
	mResponsePlotArea = CreatePlotArea(this, mResponsePlotInterface);
	plotSizer->Add(mShapePlotArea, wxSizerFlags().Expand().Proportion(1));
	plotSizer->Add(mResponsePlotArea, wxSizerFlags().Expand().Proportion(1));

	SetSizerAndFit(topSizer);
	TransferDataToWindow();
}

//==========================================================================
// Class:			MainFrame
// Function:		CreatePlotArea
//
// Description:		Creates the main plot control.
//
// Input Arguments:
//		parent		= wxWindow*
//		interface	= LibPlot2D::GuiInterface&
//
// Output Arguments:
//		None
//
// Return Value:
//		LibPlot2D::PlotRenderer* pointing to plotArea
//
//==========================================================================
LibPlot2D::PlotRenderer* MainFrame::CreatePlotArea(wxWindow *parent, LibPlot2D::GuiInterface& interface)
{
	wxGLAttributes displayAttributes;
	displayAttributes.PlatformDefaults().RGBA().DoubleBuffer().SampleBuffers(1).Samplers(4).Stencil(1).EndList();
	assert(wxGLCanvas::IsDisplaySupported(displayAttributes));
	LibPlot2D::PlotRenderer* plotArea(new LibPlot2D::PlotRenderer(interface, *parent, wxID_ANY, displayAttributes));

	plotArea->SetMinSize(wxSize(650, 320));
	plotArea->SetMajorGridOn();
	plotArea->SetCurveQuality(LibPlot2D::PlotRenderer::CurveQuality::HighWrite);

	return plotArea;
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateTextInputs
//
// Description:		Creates the input text controls.
//
// Input Arguments:
//		parent	= wxWindow*
//
// Output Arguments:
//		None
//
// Return Value:
//		wxSizer*
//
//==========================================================================
wxSizer* MainFrame::CreateTextInputs(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Inputs")));
	wxSizer* subSizer(new wxFlexGridSizer(3, 5, 5));
	sizer->Add(subSizer, wxSizerFlags().Border(wxALL, 5));
	
	diameterText = new wxTextCtrl(sizer->GetStaticBox(), idParabolaInputs);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Diameter")));
	subSizer->Add(diameterText);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("(in)")));
	diameterText->SetValidator(wxFloatingPointValidator<double>(3, &parabolaInfo.diameter, wxNUM_VAL_NO_TRAILING_ZEROES));

	focusPositionText = new wxTextCtrl(sizer->GetStaticBox(), idParabolaInputs);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Focus Position")));
	subSizer->Add(focusPositionText);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("(in)")));
	focusPositionText->SetValidator(wxFloatingPointValidator<double>(3, &parabolaInfo.focusPosition, wxNUM_VAL_NO_TRAILING_ZEROES));

	numberOfFacetsText = new wxTextCtrl(sizer->GetStaticBox(), idParabolaInputs);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Number of Facets")));
	subSizer->Add(numberOfFacetsText);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("(-)")));
	numberOfFacetsText->SetValidator(wxIntegerValidator<unsigned int>(&parabolaInfo.facetCount));

	paperWidthText = new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Paper Width")));
	subSizer->Add(paperWidthText);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("(in)")));
	paperWidthText->SetValidator(wxFloatingPointValidator<double>(3, &paperWidth, wxNUM_VAL_NO_TRAILING_ZEROES));
	
	paperHeightText = new wxTextCtrl(sizer->GetStaticBox(), wxID_ANY);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Paper Height")));
	subSizer->Add(paperHeightText);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("(in)")));
	paperHeightText->SetValidator(wxFloatingPointValidator<double>(3, &paperHeight, wxNUM_VAL_NO_TRAILING_ZEROES));

	return sizer;
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateTextOutputs
//
// Description:		Creates the output text controls.
//
// Input Arguments:
//		parent	= wxWindow*
//
// Output Arguments:
//		None
//
// Return Value:
//		wxSizer*
//
//==========================================================================
wxSizer* MainFrame::CreateTextOutputs(wxWindow* parent)
{
	wxStaticBoxSizer* sizer(new wxStaticBoxSizer(wxVERTICAL, parent, _T("Outputs")));
	wxSizer* subSizer(new wxFlexGridSizer(2, 5, 5));
	sizer->Add(subSizer, wxSizerFlags().Border(wxALL, 5));

	const wxString dummyQuantity(_T("9000.000"));
	depthText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, dummyQuantity);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Parabola Depth")));
	subSizer->Add(depthText);

	maxDesignErrorText = new wxStaticText(sizer->GetStaticBox(), wxID_ANY, dummyQuantity);
	subSizer->Add(new wxStaticText(sizer->GetStaticBox(), wxID_ANY, _T("Max. Design Error")));
	subSizer->Add(maxDesignErrorText);

	return sizer;
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateVersionText
//
// Description:		Builds a static text control containing version
//					information.
//
// Input Arguments:
//		parent	= wxWindow*
//
// Output Arguments:
//		None
//
// Return Value:
//		wxWindow*
//
//==========================================================================
wxWindow* MainFrame::CreateVersionText(wxWindow *parent)
{
	wxString appVersionString(ParabolicDesignApp::versionString
		+ _T(" (") + ParabolicDesignApp::gitHash + _T(")"));
	wxString lp2dVersionString(LibPlot2D::versionString
		+ _T(" (") + LibPlot2D::gitHash + _T(")"));

	wxStaticText* versionInfo(new wxStaticText(parent, wxID_ANY,
		appVersionString));
	versionInfo->SetToolTip(_T("parabolicDesign ") + appVersionString
		+ _T("\nLibPlot2D ") + lp2dVersionString);

	return versionInfo;
}

//==========================================================================
// Class:			MainFrame
// Function:		SetProperties
//
// Description:		Sets the window properties for this window.  Includes
//					title, frame size, and default fonts.
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
void MainFrame::SetProperties()
{
	SetTitle(ParabolicDesignApp::appTitle);
	SetName(ParabolicDesignApp::appName);
	mShapePlotInterface.SetApplicationTitle(ParabolicDesignApp::appTitle);
	mResponsePlotInterface.SetApplicationTitle(ParabolicDesignApp::appTitle);
	Center();

/*#ifdef __WXMSW__
	SetIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE));
#elif __WXGTK__
	SetIcon(wxIcon(plots16_xpm));
	SetIcon(wxIcon(plots24_xpm));
	SetIcon(wxIcon(plots32_xpm));
	SetIcon(wxIcon(plots48_xpm));
	SetIcon(wxIcon(plots64_xpm));
	SetIcon(wxIcon(plots128_xpm));
#endif*/
}

//==========================================================================
// Class:			MainFrame
// Function:		Event Table
//
// Description:		Links GUI events with event handler functions.
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
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_TEXT(idParabolaInputs,		MainFrame::TextChangedEvent)
	EVT_BUTTON(idWriteShape,		MainFrame::OnWriteShapeClicked)
END_EVENT_TABLE();

//==========================================================================
// Class:			MainFrame
// Function:		TextChangedEvent
//
// Description:		Event fires when user changes text control inputs.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::TextChangedEvent(wxCommandEvent& WXUNUSED(event))
{
	TransferDataFromWindow();
	UpdateCalculations();
}

//==========================================================================
// Class:			MainFrame
// Function:		OnWriteShapeClicked
//
// Description:		Event fires when user clicks write shape button.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnWriteShapeClicked(wxCommandEvent& WXUNUSED(event))
{
	calculator.SetParabolaInfo(parabolaInfo);
	
	wxFileDialog dialog(this, _T("Save As"), wxEmptyString, wxEmptyString, _T("LaTeX Source (*.tex)|*.tex"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_CANCEL)
		return;
		
	std::string fileName(dialog.GetPath().ToStdString());
	
	const unsigned int pointCount(2000);
	auto pattern(calculator.GetFacetShape(pointCount));
	for (auto& p : pattern)// LaTeX generator expects mm, so do the conversion
		p *= 25.4;
	
	LaTeXGenerator generator;
	generator.SetPageSize(paperWidth, paperHeight);
	if (!generator.WriteFlatPatterns(pattern, fileName))
		wxMessageBox(_T("Failed to write template to '") + fileName + _T("'"));
}

void MainFrame::UpdateCalculations()
{
	if (!initialized)
		return;
		
	// Validate inputs
	if (parabolaInfo.diameter <= 0.0 ||
		parabolaInfo.facetCount < 3 ||
		parabolaInfo.focusPosition <= 0.0)
		return;

	calculator.SetParabolaInfo(parabolaInfo);

	depthText->SetLabel(wxString::Format(_T("%0.2f in"), calculator.GetParabolaDepth()));
	maxDesignErrorText->SetLabel(wxString::Format(_T("%0.2f in"), calculator.GetMaxDesignError()));

	const unsigned int pointCount(500);
	const double maxFrequency(20000.0);
	auto parabolaShape(calculator.GetParabolaShape(pointCount));
	const auto facetShape(calculator.GetFacetShape(pointCount));
	const auto frequencyResponse(calculator.GetResponse(pointCount, maxFrequency));

	mShapePlotInterface.ClearAllCurves();
	mResponsePlotInterface.ClearAllCurves();

	//mShapePlotInterface.ForceEqualAxisScaling();// TODO:  Needs bug fix in LibPlot2D
	mShapePlotInterface.SetXDataLabel(_T("(in)"));
	mShapePlotArea->SetLeftYLabel(_T("(in)"));
	
	mResponsePlotArea->SetXLogarithmic(true);
	mResponsePlotArea->SetBottomMinorGrid(true);
	mResponsePlotInterface.SetXDataLabel(_T("Frequency (Hz)"));
	mResponsePlotArea->SetLeftYLabel(_T("Gain (dB)"));
	mResponsePlotArea->SetTitle(_T("Frequency Response"));
	
	double minParabolaY(std::numeric_limits<double>::max());
	double maxParabolaY(std::numeric_limits<double>::min());
	double maxFacetY(std::numeric_limits<double>::min());
	for (unsigned int i = 0; i < pointCount; ++i)
	{
		if (parabolaShape[i](1) < minParabolaY)
			minParabolaY = parabolaShape[i](1);
		if (parabolaShape[i](1) > maxParabolaY)
			maxParabolaY = parabolaShape[i](1);
		if (facetShape[i](1) > maxFacetY)
			maxFacetY = facetShape[i](1);
	}

	const double offset(maxFacetY - minParabolaY + 0.1 * (maxParabolaY - minParabolaY));
	for (auto& p : parabolaShape)
		p(1) += offset;

	mShapePlotInterface.AddCurve(std::move(ConvertToDataset(parabolaShape)), _T("Parabola Shape"));
	mShapePlotInterface.AddCurve(std::move(ConvertToDataset(facetShape)), _T("Facet Shape"));
	mResponsePlotInterface.AddCurve(std::move(ConvertToDataset(frequencyResponse)), _T("Frequency Response"));
}

std::unique_ptr<LibPlot2D::Dataset2D> MainFrame::ConvertToDataset(const ParabolaCalculator::Vector2DVectors& v)
{
	auto d(std::make_unique<LibPlot2D::Dataset2D>(v.size()));
	for (unsigned int i = 0; i < v.size(); ++i)
	{
		d->GetX()[i] = v[i](0);
		d->GetY()[i] = v[i](1);
	}
	
	return d;
}
