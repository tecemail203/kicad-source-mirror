/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_print.h
// Purpose:
// Author:      jean-pierre Charras
// Modified by:
// Created:     28/02/2006 18:30:16
// RCS-ID:
// Copyright:   License GNU
// Licence:
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 28/02/2006 18:30:16

#ifndef _DIALOG_PRINT_H_
#define _DIALOG_PRINT_H_

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define ID_EXCLUDE_EDGES_PCB 10005
#define ID_SET_PRINT_SCALE 10004
#define ID_TEXTCTRL 10009
#define ID_TEXTCTRL1 10010
#define ID_PRINT_REF 10006
#define ID_CHECK_PRINT_MIROR 10011
#define ID_SET_BW 10007
#define ID_PRINT_ALL_IN_ONE 10008
#define ID_PRINT_ALL 10008
#define ID_PRINT_SETUP 10001
#define ID_PRINT_PREVIEW 10002
#define ID_PRINT_EXECUTE 10003
#define SYMBOL_WINEDA_PRINTFRAME_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|MAYBE_RESIZE_BORDER
#define SYMBOL_WINEDA_PRINTFRAME_TITLE _("Print")
#define SYMBOL_WINEDA_PRINTFRAME_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_PRINTFRAME_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_PRINTFRAME_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_PrintFrame class declaration
 */

class WinEDA_PrintFrame: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( WinEDA_PrintFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_PrintFrame( );
    WinEDA_PrintFrame( WinEDA_DrawFrame* parent, wxWindowID id = SYMBOL_WINEDA_PRINTFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_PRINTFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_PRINTFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_PRINTFRAME_SIZE, long style = SYMBOL_WINEDA_PRINTFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_PRINTFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_PRINTFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_PRINTFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_PRINTFRAME_SIZE, long style = SYMBOL_WINEDA_PRINTFRAME_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_PrintFrame event handler declarations

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_SET_PRINT_SCALE
    void OnSetPrintScaleSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBOX_SELECTED event handler for ID_SET_BW
    void OnSetBwSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PRINT_SETUP
    void OnPrintSetupClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PRINT_PREVIEW
    void OnPrintPreviewClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PRINT_EXECUTE
    void OnPrintExecuteClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end WinEDA_PrintFrame event handler declarations

////@begin WinEDA_PrintFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_PrintFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

	void OnClosePrintDialog();
	void OnPrintSetup(wxCommandEvent& event);
	void OnPrintPreview(wxCommandEvent& event);
	void EDA_PrintPage(wxCommandEvent& event);
	void SetColorOrBlack(wxCommandEvent& event);
	void SetScale(wxCommandEvent& event);
	int SetLayerMaskFromListSelection();
	wxString BuildPrintTitle();
	void SetOthersDatas();
	void SetPenWidth();


////@begin WinEDA_PrintFrame member variables
    wxBoxSizer* m_FullDialogBowSizer;
    wxBoxSizer* m_LeftBoxSizer;
    wxBoxSizer* m_LayersSelectionsBoxSizer;
    wxBoxSizer* m_CopperLayersBoxSizer;
    wxBoxSizer* m_TechLayersBoxSizer;
    wxCheckBox* m_Exclude_Edges_Pcb;
    wxBoxSizer* m_ScaleBoxSizer;
    wxRadioBox* m_ScaleOption;
    wxStaticText* m_FineAdjustXscaleTitle;
    wxTextCtrl* m_FineAdjustXscaleOpt;
    wxStaticText* m_FineAdjustYscaleTitle;
    wxTextCtrl* m_FineAdjustYscaleOpt;
    wxStaticBox* m_OptionsBoxSizer;
    wxBoxSizer* m_DialogPenWidthSizer;
    wxCheckBox* m_Print_Sheet_Ref;
    wxCheckBox* m_Print_Mirror;
    wxRadioBox* m_ColorOption;
    wxRadioBox* m_PagesOptionPcb;
    wxRadioBox* m_PagesOptionEeschema;
    wxBoxSizer* m_ButtonsBoxSizer;
    wxButton* m_CloseButton;
////@end WinEDA_PrintFrame member variables

	WinEDA_DrawFrame * m_Parent;
    wxRadioBox* m_PagesOption;
	WinEDA_ValueCtrl * m_DialogPenWidth;
	wxCheckBox * m_BoxSelecLayer[32];
	double m_XScaleAdjust, m_YScaleAdjust;
    wxConfig * m_Config;
};

#endif
    // _DIALOG_PRINT_H_
