// Copyright (c) 1996-1999 Microsoft Corporation. All rights reserved

#include "common.h"
#include <mmsystem.h>
#include <tchar.h>
#include <stdio.h>
#include "D3DApp.h"
#include "D3DEnum.h"
#include "D3DUtil.h"
#include "D3DRes.h"

enum APPMSGTYPE { MSG_NONE, MSGERR_APPMUSTEXIT, MSGWARN_SWITCHEDTOSOFTWARE };

static CD3DApplication* g_pD3DApp;

// 0x00403EC0, 75 bytes.
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( g_pD3DApp )
        return g_pD3DApp->MsgProc( hWnd, uMsg, wParam, lParam );

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

// 0x00403E60, 92 bytes.
INT CALLBACK AboutProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if( WM_COMMAND == uMsg )
        if( IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam) )
            EndDialog( hWnd, TRUE );

    return WM_INITDIALOG == uMsg ? TRUE : FALSE;
}




// 0x00403BF0, 342 bytes.
CD3DApplication::CD3DApplication()
{
    m_pFramework = NULL;
    m_hWnd       = NULL;
    m_pDD        = NULL;
    m_pD3D       = NULL;
    m_pd3dDevice = NULL;

    m_pddsRenderTarget     = NULL;
    m_pddsRenderTargetLeft = NULL;

    m_bActive      = FALSE;
    m_bReady       = FALSE;
    m_bFrameMoving = TRUE;
    m_bSingleStep  = FALSE;
    m_fFPS         = 0.0f;

    m_strWinClassName  = _T("D3D Window Class");
    m_strWindowTitle   = _T("D3D Application");
    m_bAppUseZBuffer   = FALSE;
    m_bAppUseStereo    = FALSE;
    m_bShowStats       = FALSE;
    m_bUseAccelKeys    = FALSE;
    m_bStartHEL        = FALSE;
    m_bStartFullScreen = FALSE;
    m_wScreenWidth     = 640;
    m_wScreenHeight    = 480;
    m_fnConfirmDevice  = NULL;

    g_pD3DApp = this;
}




// 0x004030E0, 884 bytes.
HRESULT CD3DApplication::Create( HINSTANCE hInst, TCHAR* strCmdLine )
{
    HRESULT hr;

    if( FAILED( hr = D3DEnum_EnumerateDevices( m_fnConfirmDevice ) ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        return hr;
    }

    DWORD dwFlags = ( m_bStartHEL ? D3DENUM_SOFTWAREONLY : 0L );
    dwFlags |= ( m_bStartFullScreen ? D3DENUM_FULLSCREENONLY : 0L );

    if( FAILED( hr = D3DEnum_SelectDefaultDevice( &m_pDeviceInfo, dwFlags ) ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        return hr;
    }

    if( FAILED( D3DEnum_UserChangeDeviceOnDesktop( &m_pDeviceInfo ) ) )
        return E_FAIL;

    if( FAILED( hr = OneTimeSceneInit() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        return hr;
    }

    if( NULL == ( m_pFramework = new CD3DFramework7() ) )
    {
        DisplayFrameworkError( E_OUTOFMEMORY, MSGERR_APPMUSTEXIT );
        return E_OUTOFMEMORY;
    }

    WNDCLASS wndClass;
    wndClass.style         = 0;
    wndClass.lpfnWndProc   = WndProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hInstance     = hInst;
    wndClass.hIcon         = LoadIcon( hInst, MAKEINTRESOURCE(IDI_MAIN_ICON) );
    wndClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = m_strWinClassName;
    RegisterClass( &wndClass );

    DWORD dwStyle = WS_POPUP|WS_VISIBLE;
    m_hWnd = CreateWindowEx( 0, m_strWinClassName, m_strWindowTitle, dwStyle,
                             CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL,
                             hInst, NULL );
    UpdateWindow( m_hWnd );

    RECT rc;
    SetRect( &rc, 0, 0, m_wScreenWidth, m_wScreenHeight );
    BOOL bMenu = ( GetMenu( m_hWnd ) != NULL );
    AdjustWindowRectEx( &rc, GetWindowLong( m_hWnd, GWL_STYLE ), bMenu,
                        GetWindowLong( m_hWnd, GWL_EXSTYLE ) );

    INT nWidth  = rc.right  - rc.left;
    INT nHeight = rc.bottom - rc.top;
    INT nX = ( GetSystemMetrics(SM_CXSCREEN) - nWidth )  / 2;
    INT nY = ( GetSystemMetrics(SM_CYSCREEN) - nHeight ) / 2;
    SetWindowPos( m_hWnd, NULL, nX, nY, nWidth, nHeight,
                  SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOACTIVATE );

    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        Cleanup3DEnvironment();
        return E_FAIL;
    }

    m_dwBaseTime      = timeGetTime();
    m_dwLastFrameTime = timeGetTime();
    m_bReady          = TRUE;

    return S_OK;
}




// 0x00403460, 241 bytes.
INT CD3DApplication::Run()
{
    HACCEL hAccel = LoadAccelerators( NULL, MAKEINTRESOURCE(IDR_MAIN_ACCEL) );

    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message )
    {
        if( m_bActive )
            bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        else
            bGotMsg = GetMessage( &msg, NULL, 0U, 0U );

        if( bGotMsg )
        {
            INT nRet;
            if( m_bUseAccelKeys )
                nRet = TranslateAccelerator( m_hWnd, hAccel, &msg );
            else
                nRet = 0;

            INT nHandled = nRet;
            if( 0 == nHandled )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            if( m_bActive && m_bReady )
            {
                if( FAILED( Render3DEnvironment() ) )
                    DestroyWindow( m_hWnd );
            }
        }
    }

    return msg.wParam;
}




// 0x00403560, 1480 bytes.
LRESULT CD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( uMsg )
    {
        case WM_PAINT:
            if( m_pFramework && !m_bReady )
            {
                if( m_pDeviceInfo->bWindowed )
                    m_pFramework->ShowFrame();
                else
                    m_pFramework->FlipToGDISurface( TRUE );
            }
            break;

        case WM_MOVE:
            if( m_pFramework && m_bActive && m_bReady && m_pDeviceInfo->bWindowed )
                m_pFramework->Move( (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam) );
            break;

        case WM_SIZE:
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                m_bActive = FALSE;
            else
                m_bActive = TRUE;

            if( m_bActive && m_bReady && m_pDeviceInfo->bWindowed )
            {
                m_bReady = FALSE;

                if( FAILED( hr = Change3DEnvironment() ) )
                    return 0;

                m_bReady = TRUE;
            }
            break;

        case WM_SETCURSOR:
            if( m_bActive && m_bReady && !m_pDeviceInfo->bWindowed )
            {
                SetCursor(NULL);
                return 1;
            }
            break;

        case WM_ENTERMENULOOP:
            Pause(TRUE);
            break;
        case WM_EXITMENULOOP:
            Pause(FALSE);
            break;

        case WM_ENTERSIZEMOVE:
            if( m_bFrameMoving )
                m_dwStopTime = timeGetTime();
            break;
        case WM_EXITSIZEMOVE:
            if( m_bFrameMoving )
                m_dwBaseTime += timeGetTime() - m_dwStopTime;
            break;

        case WM_NCHITTEST:
            if( !m_pDeviceInfo->bWindowed )
                return HTCLIENT;
            break;

        case WM_POWERBROADCAST:
            switch( wParam )
            {
                case PBT_APMQUERYSUSPEND:
                    return OnQuerySuspend( (DWORD)lParam );

                case PBT_APMRESUMESUSPEND:
                    return OnResumeSuspend( (DWORD)lParam );
            }
            break;

        case WM_SYSCOMMAND:
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_MONITORPOWER:
                    if( FALSE == m_pDeviceInfo->bWindowed )
                        return 1;
                    break;
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDM_TOGGLESTART:
                    m_bFrameMoving = !m_bFrameMoving;

                    if( m_bFrameMoving )
                        m_dwBaseTime += timeGetTime() - m_dwStopTime;
                    else
                        m_dwStopTime = timeGetTime();
                    break;

                case IDM_SINGLESTEP:
                    if( FALSE == m_bFrameMoving )
                        m_dwBaseTime += timeGetTime() - ( m_dwStopTime + 100 );

                    m_dwStopTime   = timeGetTime();
                    m_bFrameMoving = FALSE;
                    m_bSingleStep  = TRUE;
                    break;

                case IDM_CHANGEDEVICE:
                    if( m_bActive && m_bReady )
                    {
                        Pause(TRUE);

                        if( SUCCEEDED( D3DEnum_UserChangeDevice( &m_pDeviceInfo ) ) )
                        {
                            if( FAILED( hr = Change3DEnvironment() ) )
                                return 0;
                        }
                        Pause(FALSE);
                    }
                    return 0;

                case IDM_TOGGLEFULLSCREEN:
                    if( m_bActive && m_bReady )
                    {
                        m_bReady = FALSE;

                        m_pDeviceInfo->bWindowed = !m_pDeviceInfo->bWindowed;

                        if( FAILED( hr = Change3DEnvironment() ) )
                            return 0;

                        m_bReady = TRUE;
                    }
                    return 0;

                case IDM_ABOUT:
                    Pause(TRUE);
                    DialogBox( (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE ),
                               MAKEINTRESOURCE(IDD_ABOUT), hWnd, (DLGPROC)AboutProc );
                    Pause(FALSE);
                    return 0;

                case IDM_EXIT:
                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    return 0;
            }
            break;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
            break;

        case WM_CLOSE:
            DestroyWindow( hWnd );
            return 0;

        case WM_DESTROY:
            Cleanup3DEnvironment();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}




// 0x00402300, 441 bytes.
HRESULT CD3DApplication::Initialize3DEnvironment()
{
    HRESULT hr;
    DWORD   dwFrameworkFlags = 0L;
    dwFrameworkFlags |= ( !m_pDeviceInfo->bWindowed ? D3DFW_FULLSCREEN : 0L );
    dwFrameworkFlags |= (  m_pDeviceInfo->bStereo   ? D3DFW_STEREO     : 0L );
    dwFrameworkFlags |= (  m_bAppUseZBuffer         ? D3DFW_ZBUFFER    : 0L );

    if( SUCCEEDED( hr = m_pFramework->Initialize( m_hWnd,
                     m_pDeviceInfo->pDriverGUID, m_pDeviceInfo->pDeviceGUID,
                     &m_pDeviceInfo->ddsdFullscreenMode, dwFrameworkFlags ) ) )
    {
        m_pDD        = m_pFramework->GetDirectDraw();
        m_pD3D       = m_pFramework->GetDirect3D();
        m_pd3dDevice = m_pFramework->GetD3DDevice();

        m_pddsRenderTarget     = m_pFramework->GetRenderSurface();
        m_pddsRenderTargetLeft = m_pFramework->GetRenderSurfaceLeft();

        m_ddsdRenderTarget.dwSize = sizeof(m_ddsdRenderTarget);
        m_pddsRenderTarget->GetSurfaceDesc( &m_ddsdRenderTarget );

        if( SUCCEEDED( hr = InitDeviceObjects() ) )
            return S_OK;
        else
        {
            DeleteDeviceObjects();
            m_pFramework->DestroyObjects();
        }
    }

    if( m_pDeviceInfo->bHardware )
    {
        DisplayFrameworkError( hr, MSGWARN_SWITCHEDTOSOFTWARE );
        D3DEnum_SelectDefaultDevice( &m_pDeviceInfo, D3DENUM_SOFTWAREONLY|D3DENUM_FULLSCREENONLY );
        return Initialize3DEnvironment();
    }

    return hr;
}




// 0x004024C0, 393 bytes.
HRESULT CD3DApplication::Change3DEnvironment()
{
    HRESULT hr;
    static BOOL  bOldWindowedState = TRUE;
    static DWORD dwSavedStyle;
    static RECT  rcSaved;

    DeleteDeviceObjects();

    if( FAILED( hr = m_pFramework->DestroyObjects() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
        return hr;
    }

    if( bOldWindowedState != m_pDeviceInfo->bWindowed )
    {
        if( m_pDeviceInfo->bWindowed )
        {
            SetWindowLong( m_hWnd, GWL_STYLE, dwSavedStyle );
            SetWindowPos( m_hWnd, HWND_NOTOPMOST, rcSaved.left, rcSaved.top,
                          ( rcSaved.right - rcSaved.left ),
                          ( rcSaved.bottom - rcSaved.top ), SWP_SHOWWINDOW );
        }
        else
        {
            dwSavedStyle = GetWindowLong( m_hWnd, GWL_STYLE );
            GetWindowRect( m_hWnd, &rcSaved );
            SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE );
        }

        bOldWindowedState = m_pDeviceInfo->bWindowed;
    }

    if( FAILED( hr = Initialize3DEnvironment() ) )
    {
        DisplayFrameworkError( hr, MSGERR_APPMUSTEXIT );
        SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
        return hr;
    }

    if( FALSE == m_bFrameMoving )
    {
        m_bSingleStep = TRUE;
        m_dwBaseTime += timeGetTime() - m_dwStopTime;
        m_dwStopTime  = timeGetTime();
    }

    return S_OK;
}




// 0x00402650, 684 bytes.
HRESULT CD3DApplication::Render3DEnvironment()
{
    HRESULT hr;

    DWORD dwCurTime = timeGetTime();
    if( dwCurTime < m_dwLastFrameTime )
        m_dwLastFrameTime = dwCurTime;

    DWORD dwElapsed = dwCurTime - m_dwLastFrameTime;
    if( dwElapsed < 10 )
        return S_OK;

    DWORD dwFrameCount = dwElapsed / 10;
    m_dwLastFrameTime = dwCurTime - dwElapsed % 10;

    if( FAILED( hr = m_pDD->TestCooperativeLevel() ) )
    {
        switch( hr )
        {
            case DDERR_EXCLUSIVEMODEALREADYSET:
            case DDERR_NOEXCLUSIVEMODE:
                return S_OK;

            case DDERR_WRONGMODE:
                if( m_pDeviceInfo->bWindowed )
                    return Change3DEnvironment();
                break;
        }
        return hr;
    }

    if( m_bFrameMoving || m_bSingleStep )
    {
        if( FAILED( hr = FrameMove( dwFrameCount ) ) )
            return hr;

        m_bSingleStep = FALSE;
    }

    if( m_bAppUseStereo && m_pDeviceInfo->bStereo && !m_pDeviceInfo->bWindowed )
    {
        m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_matLeftView );
        if( FAILED( hr = m_pd3dDevice->SetRenderTarget( m_pddsRenderTargetLeft, 0 ) ) )
            return hr;
        if( FAILED( hr = Render() ) )
            return hr;

        m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_matRightView );
        if( FAILED( hr = m_pd3dDevice->SetRenderTarget( m_pddsRenderTarget, 0 ) ) )
            return hr;
        if( FAILED( hr = Render() ) )
            return hr;
    }
    else
    {
        if( m_bAppUseStereo )
            m_pd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_matView );

        if( FAILED( hr = Render() ) )
            return hr;
    }

    UpdateStats();

    if( FAILED( hr = m_pFramework->ShowFrame() ) )
    {
        if( DDERR_SURFACELOST != hr )
            return hr;

        m_pFramework->RestoreSurfaces();
        RestoreSurfaces();
    }

    return S_OK;
}




// 0x00402900, 129 bytes.
VOID CD3DApplication::Cleanup3DEnvironment()
{
    m_bActive = FALSE;
    m_bReady  = FALSE;

    if( m_pFramework )
    {
        DeleteDeviceObjects();
        SAFE_DELETE( m_pFramework );

        FinalCleanup();
    }

    D3DEnum_FreeResources();
}




// 0x00403B30, 177 bytes.
VOID CD3DApplication::Pause( BOOL bPause )
{
    static DWORD dwAppPausedCount = 0L;

    dwAppPausedCount += ( bPause ? +1 : -1 );
    m_bReady          = ( dwAppPausedCount ? FALSE : TRUE );

    if( bPause && ( 1 == dwAppPausedCount ) )
    {
        if( m_pFramework )
            m_pFramework->FlipToGDISurface( TRUE );

        if( m_bFrameMoving )
            m_dwStopTime = timeGetTime();
    }

    if( 0 == dwAppPausedCount )
    {
        if( m_bFrameMoving )
            m_dwBaseTime += timeGetTime() - m_dwStopTime;
    }
}




// 0x00402D30, 33 bytes.
LRESULT CD3DApplication::OnQuerySuspend( DWORD dwFlags )
{
    Pause(TRUE);

    return TRUE;
}




// 0x00402D60, 33 bytes.
LRESULT CD3DApplication::OnResumeSuspend( DWORD dwData )
{
    Pause(FALSE);

    return TRUE;
}




// 0x00402D90, 258 bytes.
VOID CD3DApplication::SetViewParams( D3DVECTOR* vEyePt, D3DVECTOR* vLookatPt,
                                     D3DVECTOR* vUpVec, FLOAT fEyeDistance )
{
    D3DVECTOR vView = (*vLookatPt) - (*vEyePt);
    vView = CrossProduct( vView, (*vUpVec) );
    vView = Normalize( vView ) * fEyeDistance;

    D3DVECTOR vLeftEyePt  = (*vEyePt) + vView;
    D3DVECTOR vRightEyePt = (*vEyePt) - vView;

    D3DUtil_SetViewMatrix( m_matLeftView,  vLeftEyePt,  *vLookatPt, *vUpVec );
    D3DUtil_SetViewMatrix( m_matRightView, vRightEyePt, *vLookatPt, *vUpVec );
    D3DUtil_SetViewMatrix( m_matView,      *vEyePt,     *vLookatPt, *vUpVec );
}




// 0x00402EA0, 294 bytes.
VOID CD3DApplication::UpdateStats()
{
    static FLOAT fLastTime = 0.0f;
    static DWORD dwFrames  = 0L;

    FLOAT fTime = timeGetTime() * 0.001f;
    ++dwFrames;

    if( fTime - fLastTime > 1.0f )
    {
        m_fFPS    = dwFrames / (fTime - fLastTime);
        fLastTime = fTime;
        dwFrames  = 0L;
    }

    if( m_bShowStats )
    {
        TCHAR buffer[80];
        sprintf( buffer, _T("%7.02f fps (%dx%dx%d)"), m_fFPS,
                 m_ddsdRenderTarget.dwWidth, m_ddsdRenderTarget.dwHeight,
                 m_ddsdRenderTarget.ddpfPixelFormat.dwRGBBitCount );
        OutputText( 0, 0, buffer );
    }
}




// 0x00402FD0, 269 bytes.
VOID CD3DApplication::OutputText( DWORD x, DWORD y, TCHAR* str )
{
    HDC hDC;

    if( m_pddsRenderTarget )
    {
        if( SUCCEEDED( m_pddsRenderTarget->GetDC(&hDC) ) )
        {
            SetTextColor( hDC, RGB(255,255,0) );
            SetBkMode( hDC, TRANSPARENT );
            ExtTextOut( hDC, x, y, 0, NULL, str, lstrlen(str), NULL );
            m_pddsRenderTarget->ReleaseDC(hDC);
        }
    }

    if( m_pddsRenderTargetLeft )
    {
        if( SUCCEEDED( m_pddsRenderTargetLeft->GetDC( &hDC ) ) )
        {
            SetTextColor( hDC, RGB(255,0,255) );
            SetBkMode( hDC, TRANSPARENT );
            ExtTextOut( hDC, x, y, 0, NULL, str, lstrlen(str), NULL );
            m_pddsRenderTargetLeft->ReleaseDC(hDC);
        }
    }
}




// 0x00402990, 928 bytes.
VOID CD3DApplication::DisplayFrameworkError( HRESULT hr, DWORD dwType )
{
    TCHAR strMsg[512];

    switch( hr )
    {
        case D3DENUMERR_NODIRECTDRAW:
            lstrcpy( strMsg, _T("Could not create DirectDraw!") );
            break;
        case D3DENUMERR_NOCOMPATIBLEDEVICES:
            lstrcpy( strMsg, _T("Could not find any compatible Direct3D\n"
                     "devices.") );
            break;
        case D3DENUMERR_SUGGESTREFRAST:
            lstrcpy( strMsg, _T("Could not find any compatible devices.\n\n"
                     "Try enabling the reference rasterizer using\n"
                     "EnableRefRast.reg.") );
            break;
        case D3DENUMERR_ENUMERATIONFAILED:
            lstrcpy( strMsg, _T("Enumeration failed. Your system may be in an\n"
                     "unstable state and need to be rebooted") );
            break;
        case D3DFWERR_INITIALIZATIONFAILED:
            lstrcpy( strMsg, _T("Generic initialization error.\n\nEnable "
                     "debug output for detailed information.") );
            break;
        case D3DFWERR_NODIRECTDRAW:
            lstrcpy( strMsg, _T("No DirectDraw") );
            break;
        case D3DFWERR_NODIRECT3D:
            lstrcpy( strMsg, _T("No Direct3D") );
            break;
        case D3DFWERR_INVALIDMODE:
            lstrcpy( strMsg, _T("This game requires a 16-bit (or higher) "
                                "display mode\nto run in a window.\n\nPlease "
                                "switch your desktop settings accordingly.") );
            break;
        case D3DFWERR_COULDNTSETCOOPLEVEL:
            lstrcpy( strMsg, _T("Could not set Cooperative Level") );
            break;
        case D3DFWERR_NO3DDEVICE:
            lstrcpy( strMsg, _T("Could not create the Direct3DDevice object.") );
            if( MSGWARN_SWITCHEDTOSOFTWARE == dwType )
                lstrcat( strMsg, _T("\nThe 3D hardware chipset may not support"
                                    "\nrendering in the current display mode.") );
            break;
        case D3DFWERR_NOZBUFFER:
            lstrcpy( strMsg, _T("No ZBuffer") );
            break;
        case D3DFWERR_INVALIDZBUFFERDEPTH:
            lstrcpy( strMsg, _T("Invalid Z-buffer depth. Try switching modes\n"
                     "from 16- to 32-bit (or vice versa)") );
            break;
        case D3DFWERR_NOVIEWPORT:
            lstrcpy( strMsg, _T("No Viewport") );
            break;
        case D3DFWERR_NOPRIMARY:
            lstrcpy( strMsg, _T("No primary") );
            break;
        case D3DFWERR_NOCLIPPER:
            lstrcpy( strMsg, _T("No Clipper") );
            break;
        case D3DFWERR_BADDISPLAYMODE:
            lstrcpy( strMsg, _T("Bad display mode") );
            break;
        case D3DFWERR_NOBACKBUFFER:
            lstrcpy( strMsg, _T("No backbuffer") );
            break;
        case D3DFWERR_NONZEROREFCOUNT:
            lstrcpy( strMsg, _T("A DDraw object has a non-zero reference\n"
                     "count (meaning it was not properly cleaned up)." ) );
            break;
        case D3DFWERR_NORENDERTARGET:
            lstrcpy( strMsg, _T("No render target") );
            break;
        case E_OUTOFMEMORY:
            lstrcpy( strMsg, _T("Not enough memory!") );
            break;
        case DDERR_OUTOFVIDEOMEMORY:
            lstrcpy( strMsg, _T("There was insufficient video memory "
                     "to use the\nhardware device.") );
            break;
        default:
            lstrcpy( strMsg, _T("Generic application error.\n\nEnable "
                     "debug output for detailed information.") );
    }

    if( MSGERR_APPMUSTEXIT == dwType )
    {
        lstrcat( strMsg, _T("\n\nThis game will now exit.") );
        MessageBox( NULL, strMsg, m_strWindowTitle, MB_ICONERROR|MB_OK );
    }
    else
    {
        if( MSGWARN_SWITCHEDTOSOFTWARE == dwType )
            lstrcat( strMsg, _T("\n\nSwitching to software rasterizer.") );
        MessageBox( NULL, strMsg, m_strWindowTitle, MB_ICONWARNING|MB_OK );
    }
}
