/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Mario Luzeiro <mrluzeiro@ua.pt>
 * Copyright (C) 1992-2019 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "eda_3d_viewer.h"
#include "3d_viewer_settings.h"

#include "../3d_viewer_id.h"
#include "../common_ogl/cogl_att_list.h"
#include <3d_actions.h>
#include <bitmaps.h>
#include <board_stackup_manager/class_board_stackup.h>
#include <board_stackup_manager/stackup_predefined_prms.h>
#include <class_board.h>
#include <dpi_scaling.h>
#include <gestfich.h>
#include <hotkeys_basic.h>
#include <layers_id_colors_and_visibility.h>
#include <pgm_base.h>
#include <project.h>
#include <settings/common_settings.h>
#include <settings/settings_manager.h>
#include <tool/common_control.h>
#include <tool/tool_manager.h>
#include <wildcards_and_files_ext.h>
#include <wx/colordlg.h>
#include <wx/colour.h>
#include <wx/string.h>
#include <wx/toolbar.h>


/**
 * Flag to enable 3D viewer main frame window debug tracing.
 *
 * Use "KI_TRACE_EDA_3D_VIEWER" to enable.
 *
 * @ingroup trace_env_vars
 */
const wxChar * EDA_3D_VIEWER::m_logTrace = wxT( "KI_TRACE_EDA_3D_VIEWER" );


BEGIN_EVENT_TABLE( EDA_3D_VIEWER, EDA_BASE_FRAME )

    EVT_ACTIVATE( EDA_3D_VIEWER::OnActivate )
    EVT_SET_FOCUS( EDA_3D_VIEWER::OnSetFocus )

    EVT_TOOL_RANGE( ID_ZOOM_IN, ID_ZOOM_REDRAW, EDA_3D_VIEWER::ProcessZoom )

    EVT_TOOL_RANGE( ID_START_COMMAND_3D, ID_MENU_COMMAND_END,
                    EDA_3D_VIEWER::Process_Special_Functions )

    EVT_TOOL( ID_TOOL_SET_VISIBLE_ITEMS, EDA_3D_VIEWER::Install3DViewOptionDialog )

    EVT_MENU( wxID_CLOSE, EDA_3D_VIEWER::Exit3DFrame )
    EVT_MENU( ID_RENDER_CURRENT_VIEW, EDA_3D_VIEWER::OnRenderEngineSelection )
    EVT_MENU( ID_DISABLE_RAY_TRACING, EDA_3D_VIEWER::OnDisableRayTracing )

    EVT_MENU_RANGE( ID_MENU3D_GRID, ID_MENU3D_GRID_END, EDA_3D_VIEWER::On3DGridSelection )

    EVT_UPDATE_UI( ID_RENDER_CURRENT_VIEW, EDA_3D_VIEWER::OnUpdateUIEngine )
    EVT_UPDATE_UI_RANGE( ID_MENU3D_FL_RENDER_MATERIAL_MODE_NORMAL,
                         ID_MENU3D_FL_RENDER_MATERIAL_MODE_CAD_MODE,
                         EDA_3D_VIEWER::OnUpdateUIMaterial )

    EVT_CLOSE( EDA_3D_VIEWER::OnCloseWindow )
END_EVENT_TABLE()


EDA_3D_VIEWER::EDA_3D_VIEWER( KIWAY *aKiway, PCB_BASE_FRAME *aParent,
                              const wxString &aTitle, long style ) :
                KIWAY_PLAYER( aKiway, aParent,
                              FRAME_PCB_DISPLAY3D, aTitle,
                              wxDefaultPosition, wxDefaultSize,
                              style, QUALIFIED_VIEWER3D_FRAMENAME( aParent ) )
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::EDA_3D_VIEWER %s", aTitle );

    m_canvas = NULL;
    m_disable_ray_tracing = false;
    m_mainToolBar = nullptr;
    m_AboutTitle = "3D Viewer";

    // Give it an icon
    wxIcon icon;
    icon.CopyFromBitmap( KiBitmap( icon_3d_xpm ) );
    SetIcon( icon );

    auto config = Pgm().GetSettingsManager().GetAppSettings<EDA_3D_VIEWER_SETTINGS>();
    LoadSettings( config );
    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    // Create the status line
    static const int status_dims[5] = { -1, -1, 130, 130, 170 };

    wxStatusBar *status_bar = CreateStatusBar( arrayDim( status_dims ) );
    SetStatusWidths( arrayDim( status_dims ), status_dims );

    m_canvas = new EDA_3D_CANVAS( this,
                                  COGL_ATT_LIST::GetAttributesList( true ),
                                  aParent->GetBoard(),
                                  m_settings,
                                  Prj().Get3DCacheManager() );

    if( m_canvas )
        m_canvas->SetStatusBar( status_bar );

    // Some settings need the canvas
    loadCommonSettings();

    // Create the manager
    m_toolManager = new TOOL_MANAGER;
    m_toolManager->SetEnvironment( nullptr, nullptr, nullptr, this );

    // Register tools
    m_toolManager->RegisterTool( new COMMON_CONTROL );
    m_actions = new EDA_3D_ACTIONS();
    m_toolManager->InitTools();

    CreateMenuBar();
    ReCreateMainToolbar();

    m_auimgr.SetManagedWindow( this );

    m_auimgr.AddPane( m_mainToolBar, EDA_PANE().HToolbar().Name( "MainToolbar" ).Top().Layer( 6 ) );
    m_auimgr.AddPane( m_canvas, EDA_PANE().Canvas().Name( "DrawFrame" ).Center() );

    m_auimgr.Update();

    m_mainToolBar->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( EDA_3D_VIEWER::OnKeyEvent ),
                            NULL, this );

    // Fixes bug in Windows (XP and possibly others) where the canvas requires the focus
    // in order to receive mouse events.  Otherwise, the user has to click somewhere on
    // the canvas before it will respond to mouse wheel events.
    if( m_canvas )
        m_canvas->SetFocus();
}


EDA_3D_VIEWER::~EDA_3D_VIEWER()
{
    m_mainToolBar->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( EDA_3D_VIEWER::OnKeyEvent ),
                               NULL, this );

    m_auimgr.UnInit();

    // m_canvas delete will be called by wxWidget manager
    //delete m_canvas;
    //m_canvas = nullptr;
}


void EDA_3D_VIEWER::ReloadRequest()
{
    // This will schedule a request to load later
    if( m_canvas )
        m_canvas->ReloadRequest( GetBoard(), Prj().Get3DCacheManager() );
}


void EDA_3D_VIEWER::NewDisplay( bool aForceImmediateRedraw )
{
    ReloadRequest();

    // After the ReloadRequest call, the refresh often takes a bit of time,
    // and it is made here only on request.
    if( aForceImmediateRedraw )
        m_canvas->Refresh();
}


void EDA_3D_VIEWER::Exit3DFrame( wxCommandEvent &event )
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::Exit3DFrame" );

    Close( true );
}


void EDA_3D_VIEWER::OnCloseWindow( wxCloseEvent &event )
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::OnCloseWindow" );

    if( m_canvas )
        m_canvas->Close();

    // m_canvas delete will be called by wxWidget manager
    //delete m_canvas;
    //m_canvas = nullptr;

    COLOR_SETTINGS* colors = Pgm().GetSettingsManager().GetColorSettings();
    Pgm().GetSettingsManager().SaveColorSettings( colors, "3d_viewer" );

    Destroy();
    event.Skip( true );
}

#define ROT_ANGLE 10.0

void EDA_3D_VIEWER::Process_Special_Functions( wxCommandEvent &event )
{
    int     id = event.GetId();
    bool    isChecked = event.IsChecked();

    wxLogTrace( m_logTrace,
                "EDA_3D_VIEWER::Process_Special_Functions id %d isChecked %d",
                id, isChecked );

    if( m_canvas == NULL )
        return;

    switch( id )
    {
    case ID_RELOAD3D_BOARD:
        NewDisplay( true );
        break;

    case ID_ROTATE3D_X_POS:
        m_settings.CameraGet().RotateX( glm::radians(ROT_ANGLE) );

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();

        break;

    case ID_ROTATE3D_X_NEG:
        m_settings.CameraGet().RotateX( -glm::radians(ROT_ANGLE) );

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();

        break;

    case ID_ROTATE3D_Y_POS:
        m_settings.CameraGet().RotateY( glm::radians(ROT_ANGLE) );

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();

        break;

    case ID_ROTATE3D_Y_NEG:
        m_settings.CameraGet().RotateY( -glm::radians(ROT_ANGLE) );

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();

        break;

    case ID_ROTATE3D_Z_POS:
        m_settings.CameraGet().RotateZ( glm::radians(ROT_ANGLE) );

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();

        break;

    case ID_ROTATE3D_Z_NEG:
        m_settings.CameraGet().RotateZ( -glm::radians(ROT_ANGLE) );

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();

        break;

    case ID_MOVE3D_LEFT:
        m_canvas->SetView3D( WXK_LEFT );
        return;

    case ID_MOVE3D_RIGHT:
        m_canvas->SetView3D( WXK_RIGHT );
        return;

    case ID_MOVE3D_UP:
        m_canvas->SetView3D( WXK_UP );
        return;

    case ID_MOVE3D_DOWN:
        m_canvas->SetView3D( WXK_DOWN );
        return;

    case ID_ORTHO:
        m_settings.CameraGet().ToggleProjection();

        if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
            m_canvas->Request_refresh();
        else
            m_canvas->RenderRaytracingRequest();
        return;

    case ID_TOOL_SCREENCOPY_TOCLIBBOARD:
    case ID_MENU_SCREENCOPY_PNG:
    case ID_MENU_SCREENCOPY_JPEG:
        takeScreenshot( event );
        return;

    case ID_MENU3D_BGCOLOR_BOTTOM:
        if( Set3DColorFromUser( m_settings.m_BgColorBot, _( "Background Color, Bottom" ),
                                nullptr ) )
        {
            if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
                m_canvas->Request_refresh();
            else
                NewDisplay( true );
        }
        return;

    case ID_MENU3D_BGCOLOR_TOP:
        if( Set3DColorFromUser( m_settings.m_BgColorTop, _( "Background Color, Top" ), nullptr ) )
        {
            if( m_settings.RenderEngineGet() == RENDER_ENGINE::OPENGL_LEGACY )
                m_canvas->Request_refresh();
            else
                NewDisplay( true );
        }
        return;

    case ID_MENU3D_SILKSCREEN_COLOR:
        Set3DSilkScreenColorFromUser();
        return;

    case ID_MENU3D_SOLDERMASK_COLOR:
        Set3DSolderMaskColorFromUser();
        return;

    case ID_MENU3D_SOLDERPASTE_COLOR:
        Set3DSolderPasteColorFromUser();
        return;

    case ID_MENU3D_COPPER_COLOR:
        Set3DCopperColorFromUser();
        break;

    case ID_MENU3D_PCB_BODY_COLOR:
        Set3DBoardBodyColorFromUser();
        break;

    case ID_MENU3D_STACKUP_COLORS:
        SynchroniseColoursWithBoard();
        NewDisplay( true );
        break;

    case ID_MENU3D_REALISTIC_MODE:
        m_settings.SetFlag( FL_USE_REALISTIC_MODE, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_RENDER_MATERIAL_MODE_NORMAL:
        m_settings.MaterialModeSet( MATERIAL_MODE::NORMAL );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_RENDER_MATERIAL_MODE_DIFFUSE_ONLY:
        m_settings.MaterialModeSet( MATERIAL_MODE::DIFFUSE_ONLY );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_RENDER_MATERIAL_MODE_CAD_MODE:
        m_settings.MaterialModeSet( MATERIAL_MODE::CAD_MODE );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_OPENGL_RENDER_COPPER_THICKNESS:
        m_settings.SetFlag( FL_RENDER_OPENGL_COPPER_THICKNESS, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_OPENGL_RENDER_SHOW_MODEL_BBOX:
        m_settings.SetFlag( FL_RENDER_OPENGL_SHOW_MODEL_BBOX, isChecked );
        m_canvas->Request_refresh();
        return;

    case ID_MENU3D_FL_RAYTRACING_RENDER_SHADOWS:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_SHADOWS, isChecked );
        m_canvas->Request_refresh();
        return;

    case ID_MENU3D_FL_RAYTRACING_PROCEDURAL_TEXTURES:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_PROCEDURAL_TEXTURES, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_RAYTRACING_BACKFLOOR:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_BACKFLOOR, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_RAYTRACING_REFRACTIONS:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_REFRACTIONS, isChecked );
        m_canvas->Request_refresh();
        return;

    case ID_MENU3D_FL_RAYTRACING_REFLECTIONS:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_REFLECTIONS, isChecked );
        m_canvas->Request_refresh();
        return;

    case ID_MENU3D_FL_RAYTRACING_POST_PROCESSING:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_POST_PROCESSING, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_FL_RAYTRACING_ANTI_ALIASING:
        m_settings.SetFlag( FL_RENDER_RAYTRACING_ANTI_ALIASING, isChecked );
        m_canvas->Request_refresh();
        return;

    case ID_MENU3D_SHOW_BOARD_BODY:
        m_settings.SetFlag( FL_SHOW_BOARD_BODY, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_AXIS_ONOFF:
        m_settings.SetFlag( FL_AXIS, isChecked );
        m_canvas->Request_refresh();
        return;

    case ID_MENU3D_MODULE_ONOFF_ATTRIBUTES_NORMAL:
        m_settings.SetFlag( FL_MODULE_ATTRIBUTES_NORMAL, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_MODULE_ONOFF_ATTRIBUTES_NORMAL_INSERT:
        m_settings.SetFlag( FL_MODULE_ATTRIBUTES_NORMAL_INSERT, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_MODULE_ONOFF_ATTRIBUTES_VIRTUAL:
        m_settings.SetFlag( FL_MODULE_ATTRIBUTES_VIRTUAL, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_ZONE_ONOFF:
        m_settings.SetFlag( FL_ZONE, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_ADHESIVE_ONOFF:
        m_settings.SetFlag( FL_ADHESIVE, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_SILKSCREEN_ONOFF:
        m_settings.SetFlag( FL_SILKSCREEN, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_SOLDER_MASK_ONOFF:
        m_settings.SetFlag( FL_SOLDERMASK, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_SOLDER_PASTE_ONOFF:
        m_settings.SetFlag( FL_SOLDERPASTE, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_COMMENTS_ONOFF:
        m_settings.SetFlag( FL_COMMENTS, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_ECO_ONOFF:
        m_settings.SetFlag( FL_ECO, isChecked );
        NewDisplay( true );
        return;

    case ID_MENU3D_RESET_DEFAULTS:
    {
        auto cfg = Pgm().GetSettingsManager().GetAppSettings<EDA_3D_VIEWER_SETTINGS>();
        cfg->ResetToDefaults();
        LoadSettings( cfg );

        // Tell canvas that we (may have) changed the render engine
        RenderEngineChanged();

        NewDisplay( true );
    }
        return;

    default:
        wxFAIL_MSG( "Invalid event in EDA_3D_VIEWER::Process_Special_Functions()" );
        return;
    }
}


void EDA_3D_VIEWER::On3DGridSelection( wxCommandEvent &event )
{
    int id = event.GetId();

    wxASSERT( id < ID_MENU3D_GRID_END );
    wxASSERT( id > ID_MENU3D_GRID );

    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::On3DGridSelection id %d", id );

    switch( id )
    {
    case ID_MENU3D_GRID_NOGRID:
        m_settings.GridSet( GRID3D_TYPE::NONE );
        break;
    case ID_MENU3D_GRID_10_MM:
        m_settings.GridSet( GRID3D_TYPE::GRID_10MM );
        break;
    case ID_MENU3D_GRID_5_MM:
        m_settings.GridSet( GRID3D_TYPE::GRID_5MM );
        break;
    case ID_MENU3D_GRID_2P5_MM:
        m_settings.GridSet( GRID3D_TYPE::GRID_2P5MM );
        break;
    case ID_MENU3D_GRID_1_MM:
        m_settings.GridSet( GRID3D_TYPE::GRID_1MM );
        break;

    default:
        wxFAIL_MSG( "Invalid event in EDA_3D_VIEWER::On3DGridSelection()" );
    }

    int menu_ids[]
    {
        ID_MENU3D_GRID_NOGRID, ID_MENU3D_GRID_10_MM, ID_MENU3D_GRID_5_MM,
        ID_MENU3D_GRID_2P5_MM, ID_MENU3D_GRID_1_MM
    };

    // Refresh checkmarks
    wxMenuBar* menuBar = GetMenuBar();

    for( int ii = 0; ii < 5; ii++ )
    {
        wxMenuItem* item = menuBar->FindItem( menu_ids[ii] );
        item->Check( menu_ids[ii] == id );
    }

    if( m_canvas )
        m_canvas->Request_refresh();
}


void EDA_3D_VIEWER::OnRenderEngineSelection( wxCommandEvent &event )
{
    const RENDER_ENGINE old_engine = m_settings.RenderEngineGet();

    if( old_engine == RENDER_ENGINE::OPENGL_LEGACY )
        m_settings.RenderEngineSet( RENDER_ENGINE::RAYTRACING );
    else
        m_settings.RenderEngineSet( RENDER_ENGINE::OPENGL_LEGACY );

    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::OnRenderEngineSelection type %s ",
            ( m_settings.RenderEngineGet() == RENDER_ENGINE::RAYTRACING ) ? "Ray Trace" :
                                                                            "OpenGL Legacy" );

    if( old_engine != m_settings.RenderEngineGet() )
    {
        RenderEngineChanged();
    }
}


void EDA_3D_VIEWER::ProcessZoom( wxCommandEvent &event )
{
    int id = event.GetId();

    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::ProcessZoom id:%d", id );

    if( m_canvas == NULL )
        return;

    switch( id )
    {
    case ID_ZOOM_PAGE:   m_canvas->SetView3D( WXK_HOME ); break;
    case ID_ZOOM_IN:     m_canvas->SetView3D( WXK_F1 );   break;
    case ID_ZOOM_OUT:    m_canvas->SetView3D( WXK_F2 );   break;
    case ID_ZOOM_REDRAW: m_canvas->Request_refresh();     break;
    default: wxFAIL_MSG( "Invalid event in EDA_3D_VIEWER::ProcessZoom()" );
    }

    m_canvas->DisplayStatus();
}


void EDA_3D_VIEWER::OnDisableRayTracing( wxCommandEvent& aEvent )
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::%s disabling ray tracing.", __WXFUNCTION__ );

    m_disable_ray_tracing = true;
    m_settings.RenderEngineSet( RENDER_ENGINE::OPENGL_LEGACY );
}


void EDA_3D_VIEWER::OnActivate( wxActivateEvent &event )
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::OnActivate" );

    if( m_canvas )
    {
        // Reload data if 3D frame shows a board,
        // because it can be changed since last frame activation
        if( m_canvas->IsReloadRequestPending() )
            m_canvas->Request_refresh();

        // Activates again the focus of the canvas so it will catch mouse and key events
        m_canvas->SetFocus();
    }

    event.Skip();    // required under wxMAC
}


void EDA_3D_VIEWER::OnSetFocus(wxFocusEvent &event)
{
    // Activates again the focus of the canvas so it will catch mouse and key events
    if( m_canvas )
        m_canvas->SetFocus();

    event.Skip();
}


void EDA_3D_VIEWER::LoadSettings( APP_SETTINGS_BASE *aCfg )
{
    EDA_BASE_FRAME::LoadSettings( aCfg );

    auto cfg = dynamic_cast<EDA_3D_VIEWER_SETTINGS*>( aCfg );
    wxASSERT( cfg );

    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::LoadSettings" );

    COLOR_SETTINGS* colors = Pgm().GetSettingsManager().GetColorSettings();

    auto set_color = [] ( const COLOR4D& aColor, SFVEC3D& aTarget )
    {
        aTarget.r = aColor.r;
        aTarget.g = aColor.g;
        aTarget.b = aColor.b;
    };

    set_color( colors->GetColor( LAYER_3D_BACKGROUND_BOTTOM ), m_settings.m_BgColorBot );
    set_color( colors->GetColor( LAYER_3D_BACKGROUND_TOP ),    m_settings.m_BgColorTop );
    set_color( colors->GetColor( LAYER_3D_BOARD ),             m_settings.m_BoardBodyColor );
    set_color( colors->GetColor( LAYER_3D_COPPER ),            m_settings.m_CopperColor );
    set_color( colors->GetColor( LAYER_3D_SILKSCREEN_BOTTOM ), m_settings.m_SilkScreenColorBot );
    set_color( colors->GetColor( LAYER_3D_SILKSCREEN_TOP ),    m_settings.m_SilkScreenColorTop );
    set_color( colors->GetColor( LAYER_3D_SOLDERMASK ),        m_settings.m_SolderMaskColorBot );
    set_color( colors->GetColor( LAYER_3D_SOLDERMASK ),        m_settings.m_SolderMaskColorTop );
    set_color( colors->GetColor( LAYER_3D_SOLDERPASTE ),       m_settings.m_SolderPasteColor );

    m_settings.SetFlag( FL_USE_REALISTIC_MODE, cfg->m_Render.realistic );

    m_settings.SetFlag( FL_SUBTRACT_MASK_FROM_SILK, cfg->m_Render.subtract_mask_from_silk );

    // OpenGL options
    m_settings.SetFlag( FL_RENDER_OPENGL_COPPER_THICKNESS,  cfg->m_Render.opengl_copper_thickness );

    m_settings.SetFlag( FL_RENDER_OPENGL_SHOW_MODEL_BBOX,   cfg->m_Render.opengl_show_model_bbox );

    // Raytracing options
    m_settings.SetFlag( FL_RENDER_RAYTRACING_SHADOWS,       cfg->m_Render.raytrace_shadows );

    m_settings.SetFlag( FL_RENDER_RAYTRACING_BACKFLOOR,     cfg->m_Render.raytrace_backfloor );

    m_settings.SetFlag( FL_RENDER_RAYTRACING_REFRACTIONS,   cfg->m_Render.raytrace_refractions );

    m_settings.SetFlag( FL_RENDER_RAYTRACING_REFLECTIONS,   cfg->m_Render.raytrace_reflections );

    m_settings.SetFlag( FL_RENDER_RAYTRACING_POST_PROCESSING,
            cfg->m_Render.raytrace_post_processing );

    m_settings.SetFlag( FL_RENDER_RAYTRACING_ANTI_ALIASING, cfg->m_Render.raytrace_anti_aliasing );

    m_settings.SetFlag( FL_RENDER_RAYTRACING_PROCEDURAL_TEXTURES,
            cfg->m_Render.raytrace_procedural_textures );

    m_settings.SetFlag( FL_AXIS, cfg->m_Render.show_axis );

    m_settings.SetFlag( FL_MODULE_ATTRIBUTES_NORMAL, cfg->m_Render.show_footprints_normal );

    m_settings.SetFlag( FL_MODULE_ATTRIBUTES_NORMAL_INSERT, cfg->m_Render.show_footprints_insert );

    m_settings.SetFlag( FL_MODULE_ATTRIBUTES_VIRTUAL, cfg->m_Render.show_footprints_virtual );

    m_settings.SetFlag( FL_ZONE, cfg->m_Render.show_zones );

    m_settings.SetFlag( FL_ADHESIVE, cfg->m_Render.show_adhesive );

    m_settings.SetFlag( FL_SILKSCREEN, cfg->m_Render.show_silkscreen );

    m_settings.SetFlag( FL_SOLDERMASK, cfg->m_Render.show_soldermask );

    m_settings.SetFlag( FL_SOLDERPASTE, cfg->m_Render.show_solderpaste );

    m_settings.SetFlag( FL_COMMENTS, cfg->m_Render.show_comments );

    m_settings.SetFlag( FL_ECO, cfg->m_Render.show_eco );

    m_settings.SetFlag( FL_SHOW_BOARD_BODY, cfg->m_Render.show_board_body );

    m_settings.GridSet( static_cast<GRID3D_TYPE>( cfg->m_Render.grid_type ) );

    RENDER_ENGINE engine = static_cast<RENDER_ENGINE>( cfg->m_Render.engine );
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::LoadSettings render setting %s",
            ( engine == RENDER_ENGINE::RAYTRACING ) ? "Ray Trace" : "OpenGL" );
    m_settings.RenderEngineSet( engine );

    m_settings.MaterialModeSet( static_cast<MATERIAL_MODE>( cfg->m_Render.material_mode ) );
}


void EDA_3D_VIEWER::SaveSettings( APP_SETTINGS_BASE *aCfg )
{
    auto cfg = Pgm().GetSettingsManager().GetAppSettings<EDA_3D_VIEWER_SETTINGS>();

    EDA_BASE_FRAME::SaveSettings( cfg );

    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::SaveSettings" );

    COLOR_SETTINGS* colors = Pgm().GetSettingsManager().GetColorSettings();

    auto save_color = [colors] ( SFVEC3D& aSource, LAYER_3D_ID aTarget )
    {
        colors->SetColor( aTarget, COLOR4D( aSource.r, aSource.g, aSource.b, 1.0 ) );
    };

    save_color( m_settings.m_BgColorBot,         LAYER_3D_BACKGROUND_BOTTOM );
    save_color( m_settings.m_BgColorTop,         LAYER_3D_BACKGROUND_TOP );
    save_color( m_settings.m_BoardBodyColor,     LAYER_3D_BOARD );
    save_color( m_settings.m_CopperColor,        LAYER_3D_COPPER );
    save_color( m_settings.m_SilkScreenColorBot, LAYER_3D_SILKSCREEN_BOTTOM );
    save_color( m_settings.m_SilkScreenColorTop, LAYER_3D_SILKSCREEN_TOP );
    save_color( m_settings.m_SolderMaskColorTop, LAYER_3D_SOLDERMASK );
    save_color( m_settings.m_SolderPasteColor,   LAYER_3D_SOLDERPASTE );

    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::SaveSettings render setting %s",
            ( m_settings.RenderEngineGet() == RENDER_ENGINE::RAYTRACING ) ? "Ray Trace" :
                                                                            "OpenGL" );

    cfg->m_Render.engine        = static_cast<int>( m_settings.RenderEngineGet() );
    cfg->m_Render.grid_type     = static_cast<int>( m_settings.GridGet() );
    cfg->m_Render.material_mode = static_cast<int>( m_settings.MaterialModeGet() );

    cfg->m_Render.opengl_copper_thickness = m_settings.GetFlag( FL_RENDER_OPENGL_COPPER_THICKNESS );
    cfg->m_Render.opengl_show_model_bbox = m_settings.GetFlag( FL_RENDER_OPENGL_SHOW_MODEL_BBOX );
    cfg->m_Render.raytrace_anti_aliasing = m_settings.GetFlag( FL_RENDER_RAYTRACING_ANTI_ALIASING );
    cfg->m_Render.raytrace_backfloor     = m_settings.GetFlag( FL_RENDER_RAYTRACING_BACKFLOOR );
    cfg->m_Render.raytrace_post_processing =
            m_settings.GetFlag( FL_RENDER_RAYTRACING_POST_PROCESSING );
    cfg->m_Render.raytrace_procedural_textures =
            m_settings.GetFlag( FL_RENDER_RAYTRACING_PROCEDURAL_TEXTURES );
    cfg->m_Render.raytrace_reflections    = m_settings.GetFlag( FL_RENDER_RAYTRACING_REFLECTIONS );
    cfg->m_Render.raytrace_refractions    = m_settings.GetFlag( FL_RENDER_RAYTRACING_REFRACTIONS );
    cfg->m_Render.raytrace_shadows        = m_settings.GetFlag( FL_RENDER_RAYTRACING_SHADOWS );
    cfg->m_Render.realistic               = m_settings.GetFlag( FL_USE_REALISTIC_MODE );
    cfg->m_Render.show_adhesive           = m_settings.GetFlag( FL_ADHESIVE );
    cfg->m_Render.show_axis               = m_settings.GetFlag( FL_AXIS );
    cfg->m_Render.show_board_body         = m_settings.GetFlag( FL_SHOW_BOARD_BODY );
    cfg->m_Render.show_comments           = m_settings.GetFlag( FL_COMMENTS );
    cfg->m_Render.show_eco                = m_settings.GetFlag( FL_ECO );
    cfg->m_Render.show_footprints_insert = m_settings.GetFlag( FL_MODULE_ATTRIBUTES_NORMAL_INSERT );
    cfg->m_Render.show_footprints_normal  = m_settings.GetFlag( FL_MODULE_ATTRIBUTES_NORMAL );
    cfg->m_Render.show_footprints_virtual = m_settings.GetFlag( FL_MODULE_ATTRIBUTES_VIRTUAL );
    cfg->m_Render.show_silkscreen         = m_settings.GetFlag( FL_SILKSCREEN );
    cfg->m_Render.show_soldermask         = m_settings.GetFlag( FL_SOLDERMASK );
    cfg->m_Render.show_solderpaste        = m_settings.GetFlag( FL_SOLDERPASTE );
    cfg->m_Render.show_zones              = m_settings.GetFlag( FL_ZONE );
    cfg->m_Render.subtract_mask_from_silk = m_settings.GetFlag( FL_SUBTRACT_MASK_FROM_SILK );
}


void EDA_3D_VIEWER::SynchroniseColoursWithBoard( void )
{
    BOARD*                 brd       = GetBoard();
    const FAB_LAYER_COLOR* stdColors = GetColorStandardList();
    wxColour               color;
    if( brd )
    {
        BOARD_STACKUP stckp = brd->GetDesignSettings().GetStackupDescriptor();

        for( BOARD_STACKUP_ITEM* stckpItem : stckp.GetList() )
        {
            wxString colorName = stckpItem->GetColor();

            if( colorName.StartsWith( "#" ) ) // This is a user defined color.
            {
                color.Set( colorName );
            }
            else
            {
                for( int i = 0; i < GetColorStandardListCount(); i++ )
                {
                    if( stdColors[i].m_ColorName == colorName )
                    {
                        color = stdColors[i].m_Color;
                        break;
                    }
                }
            }

            if( color.IsOk() )
            {
                switch( stckpItem->GetBrdLayerId() )
                {
                case F_SilkS:
                    m_settings.m_SilkScreenColorTop.r = color.Red() / 255.0;
                    m_settings.m_SilkScreenColorTop.g = color.Green() / 255.0;
                    m_settings.m_SilkScreenColorTop.b = color.Blue() / 255.0;
                    break;
                case B_SilkS:
                    m_settings.m_SilkScreenColorBot.r = color.Red() / 255.0;
                    m_settings.m_SilkScreenColorBot.g = color.Green() / 255.0;
                    m_settings.m_SilkScreenColorBot.b = color.Blue() / 255.0;
                    break;
                case F_Mask:
                    m_settings.m_SolderMaskColorTop.r = color.Red() / 255.0;
                    m_settings.m_SolderMaskColorTop.g = color.Green() / 255.0;
                    m_settings.m_SolderMaskColorTop.b = color.Blue() / 255.0;
                    break;
                case B_Mask:
                    m_settings.m_SolderMaskColorBot.r = color.Red() / 255.0;
                    m_settings.m_SolderMaskColorBot.g = color.Green() / 255.0;
                    m_settings.m_SolderMaskColorBot.b = color.Blue() / 255.0;
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void EDA_3D_VIEWER::CommonSettingsChanged( bool aEnvVarsChanged )
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::CommonSettingsChanged" );

    // Regen menu bars, etc
    EDA_BASE_FRAME::CommonSettingsChanged( aEnvVarsChanged );

    // There is no base class that handles toolbars for this frame
    ReCreateMainToolbar();

    loadCommonSettings();

    NewDisplay( true );
}


void EDA_3D_VIEWER::OnKeyEvent( wxKeyEvent& event )
{
    if( m_canvas )
        return m_canvas->OnKeyEvent( event );

    event.Skip();
}


void EDA_3D_VIEWER::takeScreenshot( wxCommandEvent& event )
{
    wxString   fullFileName;
    bool       fmt_is_jpeg = false;

    if( event.GetId() == ID_MENU_SCREENCOPY_JPEG )
        fmt_is_jpeg = true;

    if( event.GetId() != ID_TOOL_SCREENCOPY_TOCLIBBOARD )
    {
        // Remember path between saves during this session only.
        const wxString wildcard = fmt_is_jpeg ? JpegFileWildcard() : PngFileWildcard();
        const wxString ext = fmt_is_jpeg ? JpegFileExtension : PngFileExtension;

        // First time path is set to the project path.
        if( !m_defaultSaveScreenshotFileName.IsOk() )
            m_defaultSaveScreenshotFileName = Parent()->Prj().GetProjectFullName();

        m_defaultSaveScreenshotFileName.SetExt( ext );

        wxFileDialog dlg( this, _( "3D Image File Name" ),
                          m_defaultSaveScreenshotFileName.GetPath(),
                          m_defaultSaveScreenshotFileName.GetFullName(), wildcard,
                          wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

        if( dlg.ShowModal() == wxID_CANCEL )
            return;

        m_defaultSaveScreenshotFileName = dlg.GetPath();

        if( m_defaultSaveScreenshotFileName.GetExt().IsEmpty() )
            m_defaultSaveScreenshotFileName.SetExt( ext );

        fullFileName = m_defaultSaveScreenshotFileName.GetFullPath();

        // Be sure the screen area destroyed by the file dialog is redrawn
        // before making a screen copy.
        // Without this call, under Linux the screen refresh is made to late.
        wxYield();
    }

    // Be sure we have the latest 3D view (remember 3D view is buffered)
    m_canvas->Request_refresh( true );
    wxYield();

    // Build image from the 3D buffer
    wxWindowUpdateLocker noUpdates( this );

    wxImage screenshotImage;

    if( m_canvas )
        m_canvas->GetScreenshot( screenshotImage );

    if( event.GetId() == ID_TOOL_SCREENCOPY_TOCLIBBOARD )
    {
        wxBitmap bitmap( screenshotImage );

        if( wxTheClipboard->Open() )
        {
            wxBitmapDataObject* dobjBmp = new wxBitmapDataObject( bitmap );

            if( !wxTheClipboard->SetData( dobjBmp ) )
                wxMessageBox( _( "Failed to copy image to clipboard" ) );

            wxTheClipboard->Flush();    /* the data in clipboard will stay
                                         * available after the application exits */
            wxTheClipboard->Close();
        }
    }
    else
    {
        if( !screenshotImage.SaveFile( fullFileName,
                             fmt_is_jpeg ? wxBITMAP_TYPE_JPEG : wxBITMAP_TYPE_PNG ) )
            wxMessageBox( _( "Can't save file" ) );

        screenshotImage.Destroy();
    }

}


void EDA_3D_VIEWER::RenderEngineChanged()
{
    wxLogTrace( m_logTrace, "EDA_3D_VIEWER::RenderEngineChanged()" );

    if( m_canvas )
        m_canvas->RenderEngineChanged();
}


bool EDA_3D_VIEWER::Set3DColorFromUser( SFVEC3D &aColor, const wxString& aTitle,
                                        CUSTOM_COLORS_LIST* aPredefinedColors )
{
    KIGFX::COLOR4D newcolor;
    KIGFX::COLOR4D oldcolor( aColor.r,aColor.g, aColor.b, 1.0 );

    DIALOG_COLOR_PICKER picker( this, oldcolor, false, aPredefinedColors );

    if( picker.ShowModal() != wxID_OK )
        return false;

    newcolor = picker.GetColor();

    if( newcolor == oldcolor )
        return false;

    aColor.r = newcolor.r;
    aColor.g = newcolor.g;
    aColor.b = newcolor.b;

    return true;
}


bool EDA_3D_VIEWER::Set3DSilkScreenColorFromUser()
{
    CUSTOM_COLORS_LIST colors;

    colors.push_back( CUSTOM_COLOR_ITEM( 241.0/255.0, 241.0/255.0, 241.0/255.0, "White" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 4.0/255.0, 18.0/255.0, 21.0/255.0, "Dark" ) );

    if( Set3DColorFromUser( m_settings.m_SilkScreenColorTop, _( "Silkscreen Color" ), &colors ) )
    {
        m_settings.m_SilkScreenColorBot = m_settings.m_SilkScreenColorTop;
        NewDisplay( true );
        return true;
    }

    return false;
}


bool EDA_3D_VIEWER::Set3DSolderMaskColorFromUser()
{
    CUSTOM_COLORS_LIST colors;

    colors.push_back( CUSTOM_COLOR_ITEM(  20/255.0,  51/255.0,  36/255.0, "Green" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  91/255.0, 168/255.0,  12/255.0, "Light Green" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  13/255.0, 104/255.0,  11/255.0, "Saturated Green" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 181/255.0,  19/255.0,  21/255.0, "Red" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 239/255.0,  53/255.0,  41/255.0, "Red Light Orange" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 210/255.0,  40/255.0,  14/255.0, "Red 2" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(   2/255.0,  59/255.0, 162/255.0, "Blue" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  54/255.0,  79/255.0, 116/255.0, "Light blue 1" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  61/255.0,  85/255.0, 130/255.0, "Light blue 2" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  21/255.0,  70/255.0,  80/255.0, "Green blue (dark)" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  11/255.0,  11/255.0,  11/255.0, "Black" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 245/255.0, 245/255.0, 245/255.0, "White" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 119/255.0,  31/255.0,  91/255.0, "Purple" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  32/255.0,   2/255.0,  53/255.0, "Purple Dark" ) );

    if( Set3DColorFromUser( m_settings.m_SolderMaskColorTop, _( "Solder Mask Color" ), &colors ) )
    {
        m_settings.m_SolderMaskColorBot = m_settings.m_SolderMaskColorTop;
        NewDisplay( true );
        return true;
    }

    return false;
}


bool EDA_3D_VIEWER::Set3DCopperColorFromUser()
{
    CUSTOM_COLORS_LIST colors;

    colors.push_back( CUSTOM_COLOR_ITEM( 184/255.0, 115/255.0,  50/255.0, "Copper" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 178/255.0, 156/255.0,       0.0, "Gold" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 213/255.0, 213/255.0, 213/255.0, "Silver" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 160/255.0, 160/255.0, 160/255.0, "Tin" ) );

    if( Set3DColorFromUser( m_settings.m_CopperColor, _( "Copper Color" ), &colors ) )
    {
        NewDisplay( true );
        return true;
    }

    return false;
}


bool EDA_3D_VIEWER::Set3DBoardBodyColorFromUser()
{
    CUSTOM_COLORS_LIST colors;

    colors.push_back( CUSTOM_COLOR_ITEM(  51/255.0,  43/255.0, 22/255.0, "FR4 natural, dark" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 109/255.0, 116/255.0, 75/255.0, "FR4 natural" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  78/255.0,  14/255.0,  5/255.0, "brown/red" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 146/255.0,  99/255.0, 47/255.0, "brown 1" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 160/255.0, 123/255.0, 54/255.0, "brown 2" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 146/255.0,  99/255.0, 47/255.0, "brown 3" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  63/255.0, 126/255.0, 71/255.0, "green 1" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 117/255.0, 122/255.0, 90/255.0, "green 2" ) );

    if( Set3DColorFromUser( m_settings.m_BoardBodyColor, _( "Board Body Color" ), &colors ) )
    {
        NewDisplay( true );
        return true;
    }

    return false;
}


bool EDA_3D_VIEWER::Set3DSolderPasteColorFromUser()
{
    CUSTOM_COLORS_LIST colors;

    colors.push_back( CUSTOM_COLOR_ITEM( 128/255.0, 128/255.0, 128/255.0, "grey" ) );
    colors.push_back( CUSTOM_COLOR_ITEM( 213/255.0, 213/255.0, 213/255.0, "Silver" ) );
    colors.push_back( CUSTOM_COLOR_ITEM(  90/255.0,  90/255.0,  90/255.0, "grey 2" ) );

    if( Set3DColorFromUser( m_settings.m_SolderPasteColor, _( "Solder Paste Color" ), &colors ) )
    {
        NewDisplay( true );
        return true;
    }

    return false;
}


void EDA_3D_VIEWER::OnUpdateUIEngine( wxUpdateUIEvent& aEvent )
{
    aEvent.Check( m_settings.RenderEngineGet() != RENDER_ENGINE::OPENGL_LEGACY );
}


void EDA_3D_VIEWER::OnUpdateUIMaterial( wxUpdateUIEvent& aEvent )
{
    // Set the state of toggle menus according to the current display options
    switch( aEvent.GetId() )
    {
    case ID_MENU3D_FL_RENDER_MATERIAL_MODE_NORMAL:
        aEvent.Check( m_settings.MaterialModeGet() == MATERIAL_MODE::NORMAL );
        break;

    case ID_MENU3D_FL_RENDER_MATERIAL_MODE_DIFFUSE_ONLY:
        aEvent.Check( m_settings.MaterialModeGet() == MATERIAL_MODE::DIFFUSE_ONLY );
        break;

    case ID_MENU3D_FL_RENDER_MATERIAL_MODE_CAD_MODE:
        aEvent.Check( m_settings.MaterialModeGet() == MATERIAL_MODE::CAD_MODE );
        break;

    default:
        wxFAIL_MSG( "Invalid event in EDA_3D_VIEWER::OnUpdateUIMaterial()" );
    }
}


void EDA_3D_VIEWER::loadCommonSettings()
{
    wxCHECK_RET( m_canvas, "Cannot load settings to null canvas" );

    COMMON_SETTINGS* settings = Pgm().GetCommonSettings();

    const DPI_SCALING dpi{ settings, this };
    m_canvas->SetScaleFactor( dpi.GetScaleFactor() );
    m_settings.SetFlag( FL_MOUSEWHEEL_PANNING, settings->m_Input.mousewheel_pan );
}
