/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009-2016 Jean-Pierre Charras, jean-pierre.charras at wanadoo.fr
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <fctsys.h>
#include <confirm.h>
#include <pcb_edit_frame.h>
#include <class_board.h>
#include <class_track.h>
#include <connectivity/connectivity_data.h>
#include <view/view.h>
#include <pcb_layer_box_selector.h>
#include <tool/tool_manager.h>
#include <tool/selection.h>
#include <tools/selection_tool.h>
#include <tools/global_edit_tool.h>
#include "dialog_global_edit_tracks_and_vias_base.h"

// Columns of netclasses grid
enum {
    GRID_NAME = 0,
    GRID_TRACKSIZE,
    GRID_VIASIZE,
    GRID_VIADRILL,
    GRID_uVIASIZE,
    GRID_uVIADRILL,
    GRID_DIFF_PAIR_WIDTH,        // not currently included in grid
    GRID_DIFF_PAIR_GAP,          // not currently included in grid
    GRID_DIFF_PAIR_VIA_GAP       // not currently included in grid
};


// Globals to remember control settings during a session
static bool         g_modifyTracks = true;
static bool         g_modifyVias = true;
static bool         g_filterByNetclass;
static wxString     g_netclassFilter;
static bool         g_filterByNet;
static wxString     g_netFilter;
static bool         g_filterByLayer;
static LAYER_NUM    g_layerFilter;


class DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS : public DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS_BASE
{
private:
    PCB_EDIT_FRAME* m_parent;
    BOARD*          m_brd;
    int*            m_originalColWidths;
    bool            m_failedDRC;

public:
    DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS( PCB_EDIT_FRAME* aParent );
    ~DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS() override;

private:
    void visitItem( PICKED_ITEMS_LIST* aUndoList, TRACK* aItem );
    void processItem( PICKED_ITEMS_LIST* aUndoList, TRACK* aItem );

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    void OnUpdateUI( wxUpdateUIEvent& event ) override;
    void OnSizeNetclassGrid( wxSizeEvent& event ) override;
    void AdjustNetclassGridColumns( int aWidth );

    void OnNetFilterSelect( wxCommandEvent& event )
    {
        m_netFilterOpt->SetValue( true );
    }
    void OnNetclassFilterSelect( wxCommandEvent& event ) override
    {
        m_netclassFilterOpt->SetValue( true );
    }
    void OnLayerFilterSelect( wxCommandEvent& event ) override
    {
        m_layerFilterOpt->SetValue( true );
    }

    void buildNetclassesGrid();
    void buildFilterLists();
};


DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS( PCB_EDIT_FRAME* aParent ) :
    DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS_BASE( aParent )
{
    m_parent  = aParent;
    m_brd = m_parent->GetBoard();

    m_originalColWidths = new int[ m_netclassGrid->GetNumberCols() ];

    for( int i = 0; i < m_netclassGrid->GetNumberCols(); ++i )
        m_originalColWidths[ i ] = m_netclassGrid->GetColSize( i );

    m_failedDRC = false;

    buildFilterLists();

    m_parent->UpdateTrackWidthSelectBox( m_trackWidthSelectBox, false );
    m_parent->UpdateViaSizeSelectBox( m_viaSizesSelectBox, false );

    m_layerBox->SetBoardFrame( m_parent );
    m_layerBox->SetLayersHotkeys( false );
    m_layerBox->SetNotAllowedLayerSet( LSET::AllNonCuMask() );
    m_layerBox->Resync();

    wxFont infoFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    infoFont.SetSymbolicSize( wxFONTSIZE_SMALL );
    m_netclassGrid->SetDefaultCellFont( infoFont );
    buildNetclassesGrid();

    m_netclassGrid->SetCellHighlightPenWidth( 0 );
    m_sdbSizerOK->SetDefault();

    m_netFilter->Connect( NET_SELECTED, wxCommandEventHandler( DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::OnNetFilterSelect ), NULL, this );

    FinishDialogSettings();
}


DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::~DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS()
{
    g_modifyTracks = m_tracks->GetValue();
    g_modifyVias = m_vias->GetValue();
    g_filterByNetclass = m_netclassFilterOpt->GetValue();
    g_netclassFilter = m_netclassFilter->GetStringSelection();
    g_filterByNet = m_netFilterOpt->GetValue();
    g_netFilter = m_netFilter->GetSelectedNetname();
    g_filterByLayer = m_layerFilterOpt->GetValue();
    g_layerFilter = m_layerFilter->GetLayerSelection();

    m_netFilter->Disconnect( NET_SELECTED, wxCommandEventHandler( DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::OnNetFilterSelect ), NULL, this );

    delete[] m_originalColWidths;
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::buildFilterLists()
{
    // Populate the net filter list with net names
    m_netFilter->SetNetInfo( &m_brd->GetNetInfo() );
    m_netFilter->SetSelectedNetcode( m_brd->GetHighLightNetCode() );

    // Populate the netclass filter list with netclass names
    wxArrayString netclassNames;
    NETCLASSES&   netclasses = m_brd->GetDesignSettings().m_NetClasses;

    netclassNames.push_back(netclasses.GetDefault()->GetName() );

    for( NETCLASSES::const_iterator nc = netclasses.begin(); nc != netclasses.end(); ++nc )
        netclassNames.push_back( nc->second->GetName() );

    m_netclassFilter->Set( netclassNames );
    m_netclassFilter->SetStringSelection( m_brd->GetDesignSettings().GetCurrentNetClassName() );

    // Populate the layer filter list
    m_layerFilter->SetBoardFrame( m_parent );
    m_layerFilter->SetLayersHotkeys( false );
    m_layerFilter->SetNotAllowedLayerSet( LSET::AllNonCuMask() );
    m_layerFilter->Resync();
    m_layerFilter->SetLayerSelection( m_parent->GetActiveLayer() );
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::buildNetclassesGrid()
{
#define SET_NETCLASS_VALUE( row, col, val ) \
        m_netclassGrid->SetCellValue( row, col, StringFromValue( GetUserUnits(), val, true, true ) )

    m_netclassGrid->SetCellValue( 0, GRID_TRACKSIZE, _( "Track Width" ) );
    m_netclassGrid->SetCellValue( 0, GRID_VIASIZE, _( "Via Size" ) );
    m_netclassGrid->SetCellValue( 0, GRID_VIADRILL, _( "Via Drill" ) );
    m_netclassGrid->SetCellValue( 0, GRID_uVIASIZE, _( "uVia Size" ) );
    m_netclassGrid->SetCellValue( 0, GRID_uVIADRILL, _( "uVia Drill" ) );

    NETCLASSES& netclasses = m_brd->GetDesignSettings().m_NetClasses;
    NETCLASSPTR defaultNetclass = m_brd->GetDesignSettings().GetDefault();
    m_netclassGrid->AppendRows( netclasses.GetCount() + 1 );

    m_netclassGrid->SetCellValue( 1, GRID_NAME, defaultNetclass->GetName() );
    SET_NETCLASS_VALUE( 1, GRID_TRACKSIZE, defaultNetclass->GetTrackWidth() );
    SET_NETCLASS_VALUE( 1, GRID_VIASIZE, defaultNetclass->GetViaDiameter() );
    SET_NETCLASS_VALUE( 1, GRID_VIADRILL, defaultNetclass->GetViaDrill() );
    SET_NETCLASS_VALUE( 1, GRID_uVIASIZE, defaultNetclass->GetuViaDiameter() );
    SET_NETCLASS_VALUE( 1, GRID_uVIADRILL, defaultNetclass->GetuViaDrill() );

    int row = 2;
    for( const auto& netclass : netclasses )
    {
        m_netclassGrid->SetCellValue( row, GRID_NAME, netclass.first );
        SET_NETCLASS_VALUE( row, GRID_TRACKSIZE, netclass.second->GetTrackWidth() );
        SET_NETCLASS_VALUE( row, GRID_VIASIZE, netclass.second->GetViaDiameter() );
        SET_NETCLASS_VALUE( row, GRID_VIADRILL, netclass.second->GetViaDrill() );
        SET_NETCLASS_VALUE( row, GRID_uVIASIZE, netclass.second->GetuViaDiameter() );
        SET_NETCLASS_VALUE( row, GRID_uVIADRILL, netclass.second->GetuViaDrill() );
        row++;
    }
}


bool DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::TransferDataToWindow()
{
    PCBNEW_SELECTION& selection = m_parent->GetToolManager()->GetTool<SELECTION_TOOL>()->GetSelection();
    auto              item = dynamic_cast<BOARD_CONNECTED_ITEM*>( selection.Front() );

    m_tracks->SetValue( g_modifyTracks );
    m_vias->SetValue( g_modifyVias );

    if( g_filterByNetclass && m_netclassFilter->SetStringSelection( g_netclassFilter ) )
        m_netclassFilterOpt->SetValue( true );
    else if( item )
        m_netclassFilter->SetStringSelection( item->GetNet()->GetClassName() );

    if( g_filterByNet && m_brd->FindNet( g_netFilter ) != NULL )
    {
        m_netFilter->SetSelectedNet( g_netFilter );
        m_netFilterOpt->SetValue( true );
    }
    else if( item )
        m_netFilter->SetSelectedNetcode( item->GetNetCode() );

    if( g_filterByLayer && m_layerFilter->SetLayerSelection( g_layerFilter ) != wxNOT_FOUND )
        m_layerFilterOpt->SetValue( true );
    else if( item )
        m_layerFilter->SetLayerSelection( item->GetLayer() );

    return true;
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::OnUpdateUI( wxUpdateUIEvent&  )
{
    m_trackWidthSelectBox->Enable( m_setToSpecifiedValues->GetValue() );
    m_viaSizesSelectBox->Enable( m_setToSpecifiedValues->GetValue() );

    if( m_failedDRC )
    {
        m_failedDRC = false;
        DisplayError( this, _( "Some items failed DRC and were not modified." ) );
    }
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::processItem( PICKED_ITEMS_LIST* aUndoList, TRACK* aItem )
{
    BOARD_DESIGN_SETTINGS& brdSettings = m_brd->GetDesignSettings();

    if( m_setToSpecifiedValues->GetValue() )
    {
        unsigned int prevTrackWidthIndex = brdSettings.GetTrackWidthIndex();
        unsigned int prevViaSizeIndex = brdSettings.GetViaSizeIndex();
        {
            brdSettings.SetTrackWidthIndex( (unsigned) m_trackWidthSelectBox->GetSelection() );
            brdSettings.SetViaSizeIndex( (unsigned) m_viaSizesSelectBox->GetSelection() );

            if( m_parent->SetTrackSegmentWidth( aItem, aUndoList, false ) == TRACK_ACTION_DRC_ERROR )
                m_failedDRC = true;
        }
        brdSettings.SetTrackWidthIndex( prevTrackWidthIndex );
        brdSettings.SetViaSizeIndex( prevViaSizeIndex );

        if( m_layerBox->GetLayerSelection() != UNDEFINED_LAYER && aItem->Type() == PCB_TRACE_T )
        {
            if( aUndoList->FindItem( aItem ) < 0 )
            {
                ITEM_PICKER picker( aItem, UR_CHANGED );
                picker.SetLink( aItem->Clone() );
                aUndoList->PushItem( picker );
            }

            aItem->SetLayer( ToLAYER_ID( m_layerBox->GetLayerSelection() ) );
            m_parent->GetBoard()->GetConnectivity()->Update( aItem );
        }
    }
    else
    {
        if( m_parent->SetTrackSegmentWidth( aItem, aUndoList, true ) == TRACK_ACTION_DRC_ERROR )
            m_failedDRC = true;
    }
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::visitItem( PICKED_ITEMS_LIST* aUndoList, TRACK* aItem )
{
    if( m_netFilterOpt->GetValue() && m_netFilter->GetSelectedNetcode() >= 0 )
    {
        if( aItem->GetNetCode() != m_netFilter->GetSelectedNetcode() )
            return;
    }

    if( m_netclassFilterOpt->GetValue() && !m_netclassFilter->GetStringSelection().IsEmpty() )
    {
        if( aItem->GetNetClassName() != m_netclassFilter->GetStringSelection() )
            return;
    }

    if( m_layerFilterOpt->GetValue() && m_layerFilter->GetLayerSelection() != UNDEFINED_LAYER )
    {
        if( aItem->GetLayer() != m_layerFilter->GetLayerSelection() )
            return;
    }

    processItem( aUndoList, aItem );
}


bool DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::TransferDataFromWindow()
{
    PICKED_ITEMS_LIST itemsListPicker;
    wxBusyCursor dummy;

    // Examine segments
    for( auto segment : m_brd->Tracks() )
    {
        if( m_tracks->GetValue() && segment->Type() == PCB_TRACE_T )
            visitItem( &itemsListPicker, segment );
        else if (m_vias->GetValue() && segment->Type() == PCB_VIA_T )
            visitItem( &itemsListPicker, segment );
    }

    if( itemsListPicker.GetCount() > 0 )
    {
        m_parent->SaveCopyInUndoList( itemsListPicker, UR_CHANGED );

        for( auto segment : m_brd->Tracks() )
            m_parent->GetCanvas()->GetView()->Update( segment );
    }

    return !m_failedDRC;
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::AdjustNetclassGridColumns( int aWidth )
{
    for( int i = 1; i < m_netclassGrid->GetNumberCols(); i++ )
    {
        m_netclassGrid->SetColSize( i, m_originalColWidths[ i ] );
        aWidth -= m_originalColWidths[ i ];
    }

    m_netclassGrid->SetColSize( 0, aWidth );
}


void DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS::OnSizeNetclassGrid( wxSizeEvent& event )
{
    AdjustNetclassGridColumns( event.GetSize().GetX() );
    event.Skip();
}


int GLOBAL_EDIT_TOOL::EditTracksAndVias( const TOOL_EVENT& aEvent )
{
    PCB_EDIT_FRAME* editFrame = getEditFrame<PCB_EDIT_FRAME>();
    DIALOG_GLOBAL_EDIT_TRACKS_AND_VIAS dlg( editFrame );
    
    dlg.ShowQuasiModal();       // QuasiModal required for NET_SELECTOR
    return 0;
}

