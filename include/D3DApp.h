//-----------------------------------------------------------------------------
// File: D3DApp.h
//
// Desc: Application class for the Direct3D samples framework library.
//
// Copyright (c) 1998-1999 Microsoft Corporation. All rights reserved.
//
// Hopmon's modified copy: declarations checked against Hopmon.pdb.
//-----------------------------------------------------------------------------
#ifndef  D3DAPP_H
#define  D3DAPP_H
#define  D3D_OVERLOADS
#include <d3d.h>
#include "D3DFrame.h"
#include "D3DEnum.h"
#include "D3DUtil.h"
#include "D3DRes.h"




//-----------------------------------------------------------------------------
// Name: class CD3DApplication
// Desc:
//-----------------------------------------------------------------------------
// PDB type 0x15FD
class CD3DApplication
{
    friend struct LayoutCheck;

    // Internal variables and member functions
    CD3DFramework7* m_pFramework; // 0x04
    BOOL            m_bActive; // 0x08
    BOOL            m_bReady; // 0x0C

    BOOL            m_bFrameMoving; // 0x10
    BOOL            m_bSingleStep; // 0x14
    DWORD           m_dwBaseTime; // 0x18
    DWORD           m_dwStopTime; // 0x1C
    DWORD           m_dwLastFrameTime; // 0x20
    FLOAT           m_fFPS; // 0x24

    HRESULT Initialize3DEnvironment();
    HRESULT Change3DEnvironment();

    HRESULT Render3DEnvironment();
    VOID    Cleanup3DEnvironment();
    VOID    DisplayFrameworkError( HRESULT, DWORD );

protected:
    HWND                 m_hWnd; // 0x28
    D3DEnum_DeviceInfo*  m_pDeviceInfo; // 0x2C
    LPDIRECTDRAW7        m_pDD; // 0x30
    LPDIRECT3D7          m_pD3D; // 0x34
    LPDIRECT3DDEVICE7    m_pd3dDevice; // 0x38
    LPDIRECTDRAWSURFACE7 m_pddsRenderTarget; // 0x3C
    LPDIRECTDRAWSURFACE7 m_pddsRenderTargetLeft; // 0x40, for stereo modes
    DDSURFACEDESC2       m_ddsdRenderTarget; // 0x44

    // Overridable variables for the app
    TCHAR*               m_strWinClassName; // 0xC0
    TCHAR*               m_strWindowTitle; // 0xC4
    BOOL                 m_bAppUseZBuffer; // 0xC8
    BOOL                 m_bAppUseStereo; // 0xCC
    BOOL                 m_bShowStats; // 0xD0
    BOOL                 m_bUseAccelKeys; // 0xD4
    BOOL                 m_bStartHEL; // 0xD8
    BOOL                 m_bStartFullScreen; // 0xDC
    WORD                 m_wScreenWidth; // 0xE0
    WORD                 m_wScreenHeight; // 0xE2
    HRESULT              (*m_fnConfirmDevice)(DDCAPS*, D3DDEVICEDESC7*); // 0xE4

    // Overridable functions for the 3D scene created by the app
    virtual HRESULT OneTimeSceneInit()     { return S_OK; } // vtable 0x00
    virtual HRESULT InitDeviceObjects()    { return S_OK; } // vtable 0x04
    virtual HRESULT DeleteDeviceObjects()  { return S_OK; } // vtable 0x08
    virtual HRESULT Render()               { return S_OK; } // vtable 0x0C
    virtual HRESULT FrameMove( DWORD )     { return S_OK; } // vtable 0x10
    virtual HRESULT RestoreSurfaces()      { return S_OK; } // vtable 0x14
    virtual HRESULT FinalCleanup()         { return S_OK; } // vtable 0x18

    // Overridable power management (APM) functions
    virtual LRESULT OnQuerySuspend( DWORD dwFlags ); // vtable 0x1C
    virtual LRESULT OnResumeSuspend( DWORD dwData ); // vtable 0x20

    // View control functions (for stereo-enabled applications)
    D3DMATRIX m_matLeftView; // 0xE8
    D3DMATRIX m_matRightView; // 0x128
    D3DMATRIX m_matView; // 0x168
    VOID    SetAppLeftViewMatrix( D3DMATRIX mat )  { m_matLeftView  = mat; }
    VOID    SetAppRightViewMatrix( D3DMATRIX mat ) { m_matRightView = mat; }
    VOID    SetAppViewMatrix( D3DMATRIX mat )      { m_matView      = mat; }
    VOID    SetViewParams( D3DVECTOR* vEyePt, D3DVECTOR* vLookatPt,
                           D3DVECTOR* vUpVec, FLOAT fEyeDistance );

    // Miscellaneous functions
    VOID    UpdateStats();
    VOID    OutputText( DWORD x, DWORD y, TCHAR* str );

public:
    // Functions to create, run, pause, and clean up the application
    virtual HRESULT Create( HINSTANCE, TCHAR* ); // vtable 0x24
    virtual INT     Run(); // vtable 0x28
    virtual LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ); // vtable 0x2C
    virtual VOID    Pause( BOOL bPause ); // vtable 0x30

    // Class constructor
    CD3DApplication();

    HWND                 GetHWND() const             { return m_hWnd; }
    LPDIRECT3DDEVICE7    GetD3DDevice() const         { return m_pd3dDevice; }
    LPDIRECTDRAW7        GetDDraw() const             { return m_pDD; }
    LPDIRECTDRAWSURFACE7 GetBackBuffer() const        { return m_pddsRenderTarget; }
    INT                  GetScreenWidth() const       { return m_pFramework->GetRenderWidth(); }
    INT                  GetScreenHeight() const      { return m_pFramework->GetRenderHeight(); }
    INT                  GetFPS() const               { return (INT)(m_fFPS + 0.5f); }
    BOOL                 DisplayFPS() const           { if( m_pDeviceInfo ) return m_pDeviceInfo->bDispFPS; return FALSE; }
};
ASSERT_SIZE(CD3DApplication, 0x1A8);




#endif // D3DAPP_H


