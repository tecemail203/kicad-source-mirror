///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug  7 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_edit_component_in_schematic_fbp.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* upperSizer;
	upperSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxStaticBoxSizer* optionsSizer;
	optionsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Options") ), wxVERTICAL );
	
	wxStaticBoxSizer* unitSizer;
	unitSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Unit") ), wxVERTICAL );
	
	wxString unitChoiceChoices[] = { _("1"), _("2"), _("3"), _("4"), _("5"), _("6"), _("7"), _("8"), _("9"), _("10"), _("11"), _("12"), _("13"), _("14"), _("15"), _("16"), _("17"), _("18"), _("19"), _("20"), _("21"), _("22"), _("23"), _("24"), _("25"), _("26") };
	int unitChoiceNChoices = sizeof( unitChoiceChoices ) / sizeof( wxString );
	unitChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, unitChoiceNChoices, unitChoiceChoices, 0 );
	unitChoice->SetSelection( 0 );
	unitSizer->Add( unitChoice, 1, wxALL|wxEXPAND, 5 );
	
	optionsSizer->Add( unitSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 8 );
	
	wxBoxSizer* orientationSizer;
	orientationSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxString orientationRadioBoxChoices[] = { _("0"), _("+90"), _("180"), _("-90") };
	int orientationRadioBoxNChoices = sizeof( orientationRadioBoxChoices ) / sizeof( wxString );
	orientationRadioBox = new wxRadioBox( this, wxID_ANY, _("Orientation (Degrees)"), wxDefaultPosition, wxDefaultSize, orientationRadioBoxNChoices, orientationRadioBoxChoices, 1, wxRA_SPECIFY_COLS );
	orientationRadioBox->SetSelection( 0 );
	orientationRadioBox->SetToolTip( _("Select if the component is to be rotated when drawn") );
	
	orientationSizer->Add( orientationRadioBox, 1, wxALL, 8 );
	
	optionsSizer->Add( orientationSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 0 );
	
	wxBoxSizer* mirrorSizer;
	mirrorSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxString mirrorRadioBoxChoices[] = { _("Normal"), _("Mirror ---"), _("Mirror |") };
	int mirrorRadioBoxNChoices = sizeof( mirrorRadioBoxChoices ) / sizeof( wxString );
	mirrorRadioBox = new wxRadioBox( this, wxID_ANY, _("Mirror"), wxDefaultPosition, wxDefaultSize, mirrorRadioBoxNChoices, mirrorRadioBoxChoices, 1, wxRA_SPECIFY_COLS );
	mirrorRadioBox->SetSelection( 0 );
	mirrorRadioBox->SetToolTip( _("Pick the graphical transformation to be used when displaying the component, if any") );
	
	mirrorSizer->Add( mirrorRadioBox, 1, wxALL, 8 );
	
	optionsSizer->Add( mirrorSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 0 );
	
	wxStaticBoxSizer* chipnameSizer;
	chipnameSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Chip Name") ), wxHORIZONTAL );
	
	chipnameTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	chipnameTextCtrl->SetMaxLength( 32 ); 
	chipnameTextCtrl->SetToolTip( _("The name of the symbol in the library from which this component came") );
	
	chipnameSizer->Add( chipnameTextCtrl, 1, wxALL|wxEXPAND, 5 );
	
	optionsSizer->Add( chipnameSizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 8 );
	
	convertCheckBox = new wxCheckBox( this, wxID_ANY, _("Convert"), wxDefaultPosition, wxDefaultSize, 0 );
	convertCheckBox->SetToolTip( _("No Friggin Idea what this is!") );
	
	optionsSizer->Add( convertCheckBox, 0, wxALL, 8 );
	
	partsAreLockedLabel = new wxStaticText( this, wxID_ANY, _("Parts are locked"), wxDefaultPosition, wxDefaultSize, 0 );
	partsAreLockedLabel->Wrap( -1 );
	optionsSizer->Add( partsAreLockedLabel, 0, wxALL|wxEXPAND, 8 );
	
	upperSizer->Add( optionsSizer, 0, wxALIGN_TOP|wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* fieldsSizer;
	fieldsSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Fields") ), wxHORIZONTAL );
	
	wxStaticBoxSizer* gridStaticBoxSizer;
	gridStaticBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxVERTICAL );
	
	fieldListCtrl = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_HRULES|wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES );
	gridStaticBoxSizer->Add( fieldListCtrl, 1, wxALL|wxEXPAND, 8 );
	
	addFieldButton = new wxButton( this, wxID_ANY, _("Add Field"), wxDefaultPosition, wxDefaultSize, 0 );
	addFieldButton->SetToolTip( _("Add a new custom field") );
	
	gridStaticBoxSizer->Add( addFieldButton, 0, wxALL|wxEXPAND, 5 );
	
	deleteFieldButton = new wxButton( this, wxID_ANY, _("Delete Field"), wxDefaultPosition, wxDefaultSize, 0 );
	deleteFieldButton->SetToolTip( _("Delete one of the optional fields") );
	
	gridStaticBoxSizer->Add( deleteFieldButton, 0, wxALL|wxEXPAND, 5 );
	
	moveUpButton = new wxButton( this, wxID_ANY, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	moveUpButton->SetToolTip( _("Move the selected optional fields up one position") );
	
	gridStaticBoxSizer->Add( moveUpButton, 0, wxALL|wxEXPAND, 5 );
	
	fieldsSizer->Add( gridStaticBoxSizer, 5, wxALL|wxEXPAND, 8 );
	
	wxBoxSizer* fieldEditBoxSizer;
	fieldEditBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticBoxSizer* visibilitySizer;
	visibilitySizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Visibility") ), wxHORIZONTAL );
	
	showCheckBox = new wxCheckBox( this, wxID_ANY, _("Show"), wxDefaultPosition, wxDefaultSize, 0 );
	showCheckBox->SetToolTip( _("Check if you want this field visible") );
	
	visibilitySizer->Add( showCheckBox, 1, wxALL, 5 );
	
	rotateCheckBox = new wxCheckBox( this, wxID_ANY, _("Rotate"), wxDefaultPosition, wxDefaultSize, 0 );
	rotateCheckBox->SetToolTip( _("Check if you want this field's text rotated 90 degrees") );
	
	visibilitySizer->Add( rotateCheckBox, 1, wxALL, 5 );
	
	fieldEditBoxSizer->Add( visibilitySizer, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* fieldNameBoxSizer;
	fieldNameBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	fieldNameLabel = new wxStaticText( this, wxID_ANY, _("Field Name"), wxDefaultPosition, wxDefaultSize, 0 );
	fieldNameLabel->Wrap( -1 );
	fieldNameBoxSizer->Add( fieldNameLabel, 0, 0, 5 );
	
	fieldNameTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fieldNameTextCtrl->SetToolTip( _("The text (or value) of the currently selected field") );
	
	fieldNameBoxSizer->Add( fieldNameTextCtrl, 0, wxEXPAND, 5 );
	
	fieldEditBoxSizer->Add( fieldNameBoxSizer, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* fieldTextBoxSizer;
	fieldTextBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	fieldValueLabel = new wxStaticText( this, wxID_ANY, _("Field Value"), wxDefaultPosition, wxDefaultSize, 0 );
	fieldValueLabel->Wrap( -1 );
	fieldTextBoxSizer->Add( fieldValueLabel, 0, 0, 5 );
	
	fieldValueTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fieldValueTextCtrl->SetToolTip( _("The text (or value) of the currently selected field") );
	
	fieldTextBoxSizer->Add( fieldValueTextCtrl, 0, wxEXPAND, 5 );
	
	fieldEditBoxSizer->Add( fieldTextBoxSizer, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* textSizeBoxSizer;
	textSizeBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	textSizeLabel = new wxStaticText( this, wxID_ANY, _("Size(\")"), wxDefaultPosition, wxDefaultSize, 0 );
	textSizeLabel->Wrap( -1 );
	textSizeBoxSizer->Add( textSizeLabel, 0, 0, 5 );
	
	textSizeTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	textSizeTextCtrl->SetToolTip( _("The vertical height of the currently selected field's text in the schematic") );
	
	textSizeBoxSizer->Add( textSizeTextCtrl, 0, wxEXPAND, 5 );
	
	fieldEditBoxSizer->Add( textSizeBoxSizer, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* positionBoxSizer;
	positionBoxSizer = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* posXBoxSizer;
	posXBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	posXLabel = new wxStaticText( this, wxID_ANY, _("PosX(\")"), wxDefaultPosition, wxDefaultSize, 0 );
	posXLabel->Wrap( -1 );
	posXBoxSizer->Add( posXLabel, 0, 0, 5 );
	
	posXTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	posXBoxSizer->Add( posXTextCtrl, 0, wxEXPAND, 5 );
	
	positionBoxSizer->Add( posXBoxSizer, 1, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* posYBoxSizer;
	posYBoxSizer = new wxBoxSizer( wxVERTICAL );
	
	posYLabel = new wxStaticText( this, wxID_ANY, _("PosY(\")"), wxDefaultPosition, wxDefaultSize, 0 );
	posYLabel->Wrap( -1 );
	posYBoxSizer->Add( posYLabel, 0, 0, 5 );
	
	posYTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	posYTextCtrl->SetToolTip( _("The Y coordinate of the text relative to the component") );
	
	posYBoxSizer->Add( posYTextCtrl, 0, wxEXPAND, 5 );
	
	positionBoxSizer->Add( posYBoxSizer, 1, wxALL|wxEXPAND, 5 );
	
	fieldEditBoxSizer->Add( positionBoxSizer, 1, wxEXPAND, 5 );
	
	
	fieldEditBoxSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	defaultsButton = new wxButton( this, wxID_ANY, _("Reset to Library Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
	fieldEditBoxSizer->Add( defaultsButton, 0, wxALL|wxEXPAND, 5 );
	
	
	fieldEditBoxSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	fieldsSizer->Add( fieldEditBoxSizer, 3, wxEXPAND, 5 );
	
	upperSizer->Add( fieldsSizer, 1, wxALL|wxEXPAND, 5 );
	
	mainSizer->Add( upperSizer, 1, wxEXPAND, 5 );
	
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizerOK = new wxButton( this, wxID_OK );
	stdDialogButtonSizer->AddButton( stdDialogButtonSizerOK );
	stdDialogButtonSizerCancel = new wxButton( this, wxID_CANCEL );
	stdDialogButtonSizer->AddButton( stdDialogButtonSizerCancel );
	stdDialogButtonSizer->Realize();
	mainSizer->Add( stdDialogButtonSizer, 0, wxALL|wxEXPAND, 8 );
	
	this->SetSizer( mainSizer );
	this->Layout();
	
	// Connect Events
	fieldListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnListItemDeselected ), NULL, this );
	fieldListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnListItemSelected ), NULL, this );
	defaultsButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::SetInitCmp ), NULL, this );
	stdDialogButtonSizerCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnCancelButtonClick ), NULL, this );
	stdDialogButtonSizerOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnOKButtonClick ), NULL, this );
}

DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::~DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP()
{
	// Disconnect Events
	fieldListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnListItemDeselected ), NULL, this );
	fieldListCtrl->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnListItemSelected ), NULL, this );
	defaultsButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::SetInitCmp ), NULL, this );
	stdDialogButtonSizerCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnCancelButtonClick ), NULL, this );
	stdDialogButtonSizerOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_EDIT_COMPONENT_IN_SCHEMATIC_FBP::OnOKButtonClick ), NULL, this );
}
