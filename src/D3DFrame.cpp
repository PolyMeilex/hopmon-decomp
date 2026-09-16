// Copyright (c) 1996-1999 Microsoft Corporation. All rights reserved

#include "common.h"
#include <tchar.h>
#include "D3DFrame.h"
#include "D3DUtil.h"




// 0x00409260, 143 bytes.
CD3DFramework7::CD3DFramework7()
{
    m_hWnd           = NULL;
    m_bIsFullscreen  = FALSE;
    m_bIsStereo      = FALSE;
    m_dwRenderWidth  = 0L;
    m_dwRenderHeight = 0L;

    m_pddsFrontBuffer    = NULL;
    m_pddsBackBuffer     = NULL;
    m_pddsBackBufferLeft = NULL;

    m_pddsZBuffer     = NULL;
    m_pd3dDevice      = NULL;
    m_pDD             = NULL;
    m_pD3D            = NULL;
    m_dwDeviceMemType = NULL;
}




// 0x004092F0, 66 bytes.
CD3DFramework7::~CD3DFramework7()
{
    DestroyObjects();
}




// 0x004090E0, 374 bytes.
HRESULT CD3DFramework7::DestroyObjects()
{
    LONG nDD  = 0L;
    LONG nD3D = 0L;

    if( m_pDD )
    {
        m_pDD->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL );
    }

    if( m_pd3dDevice )
        if( 0 < ( nD3D = m_pd3dDevice->Release() ) )
            DEBUG_MSG( _T("Error: D3DDevice object is still referenced!") );
    m_pd3dDevice = NULL;

    SAFE_RELEASE( m_pddsBackBuffer );
    SAFE_RELEASE( m_pddsBackBufferLeft );
    SAFE_RELEASE( m_pddsZBuffer );
    SAFE_RELEASE( m_pddsFrontBuffer );
    SAFE_RELEASE( m_pD3D );

    if( m_pDD )
    {
        if( 0 < ( nDD = m_pDD->Release() ) )
            DEBUG_MSG( _T("Error: DDraw object is still referenced!") );
    }
    m_pDD = NULL;

    return ( nDD==0 && nD3D==0 ) ? S_OK : D3DFWERR_NONZEROREFCOUNT;
}




// 0x00409020, 190 bytes.
HRESULT CD3DFramework7::Initialize( HWND hWnd, GUID* pDriverGUID,
                                    GUID* pDeviceGUID, DDSURFACEDESC2* pMode,
                                    DWORD dwFlags )
{
    HRESULT hr;

    if( ( NULL==hWnd ) || ( NULL==pDeviceGUID ) ||
        ( NULL==pMode && (dwFlags&D3DFW_FULLSCREEN) ) )
        return E_INVALIDARG;

    m_hWnd          = hWnd;
    m_bIsStereo     = FALSE;
    m_bIsFullscreen = ( dwFlags & D3DFW_FULLSCREEN ) ? TRUE : FALSE;

    if( ( dwFlags & D3DFW_STEREO ) && ( dwFlags & D3DFW_FULLSCREEN ) )
        if( pMode->ddsCaps.dwCaps2 & DDSCAPS2_STEREOSURFACELEFT )
            m_bIsStereo = TRUE;

    if( FAILED( hr = CreateEnvironment( pDriverGUID, pDeviceGUID, pMode,
                                        dwFlags ) ) )
    {
        DestroyObjects();
        return hr;
    }

    return S_OK;
}




// 0x00408DB0, 255 bytes.
HRESULT CD3DFramework7::CreateEnvironment( GUID* pDriverGUID, GUID* pDeviceGUID,
                                           DDSURFACEDESC2* pMode, DWORD dwFlags )
{
    HRESULT hr;

    if( IsEqualIID( *pDeviceGUID, IID_IDirect3DHALDevice) )
        m_dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
    else if( IsEqualIID( *pDeviceGUID, IID_IDirect3DTnLHalDevice) )
        m_dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
    else
        m_dwDeviceMemType = DDSCAPS_SYSTEMMEMORY;

    hr = CreateDirectDraw( pDriverGUID, dwFlags );
    if( FAILED( hr ) )
        return hr;

    if( dwFlags & D3DFW_FULLSCREEN )
        hr = CreateFullscreenBuffers( pMode );
    else
        hr = CreateWindowedBuffers();
    if( FAILED( hr ) )
        return hr;

    hr = CreateClipperForBackBuffer();
    if( FAILED( hr ) )
        return hr;

    hr = CreateDirect3D( pDeviceGUID );
    if( FAILED( hr ) )
        return hr;

    if( dwFlags & D3DFW_ZBUFFER )
        hr = CreateZBuffer( pDeviceGUID );
    if( FAILED( hr ) )
        return hr;

    return S_OK;
}




// 0x00408580, 707 bytes.
HRESULT WINAPI EnumZBufferFormatsCallback( DDPIXELFORMAT* pddpf,
                                           VOID* pContext )
{
    DDPIXELFORMAT* pddpfOut = (DDPIXELFORMAT*)pContext;

    if( pddpfOut->dwRGBBitCount == pddpf->dwRGBBitCount )
    {
        (*pddpfOut) = (*pddpf);
        return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}




// 0x00408BD0, 256 bytes.
HRESULT CD3DFramework7::CreateDirectDraw( GUID* pDriverGUID, DWORD dwFlags )
{
    if( FAILED( DirectDrawCreateEx( pDriverGUID, (VOID**)&m_pDD,
                                    IID_IDirectDraw7, NULL ) ) )
    {
        return D3DFWERR_NODIRECTDRAW;
    }

    DWORD dwCoopFlags = DDSCL_NORMAL;
    if( m_bIsFullscreen )
        dwCoopFlags = DDSCL_ALLOWREBOOT|DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN;

    if( 0L == ( dwFlags & D3DFW_NO_FPUSETUP ) )
        dwCoopFlags |= DDSCL_FPUSETUP;

    if( FAILED( m_pDD->SetCooperativeLevel( m_hWnd, dwCoopFlags ) ) )
    {
        return D3DFWERR_COULDNTSETCOOPLEVEL;
    }

    if( !m_bIsFullscreen )
    {
        DDSURFACEDESC2 ddsd;
        ddsd.dwSize = sizeof(ddsd);
        m_pDD->GetDisplayMode( &ddsd );
        if( ddsd.ddpfPixelFormat.dwRGBBitCount <= 8 )
            return D3DFWERR_INVALIDMODE;
    }

    return S_OK;
}




// 0x00408580, 707 bytes.
HRESULT CD3DFramework7::CreateFullscreenBuffers( DDSURFACEDESC2* pddsd )
{
    HRESULT hr;

    SetRect( &m_rcScreenRect, 0, 0, pddsd->dwWidth, pddsd->dwHeight );
    m_dwRenderWidth  = m_rcScreenRect.right  - m_rcScreenRect.left;
    m_dwRenderHeight = m_rcScreenRect.bottom - m_rcScreenRect.top;

    DWORD dwModeFlags = 0;

    if( (320==m_dwRenderWidth) && (200==m_dwRenderHeight) &&
        (8==pddsd->ddpfPixelFormat.dwRGBBitCount) )
        dwModeFlags |= DDSDM_STANDARDVGAMODE;

    if( FAILED( m_pDD->SetDisplayMode( m_dwRenderWidth, m_dwRenderHeight,
                                pddsd->ddpfPixelFormat.dwRGBBitCount,
                                pddsd->dwRefreshRate, dwModeFlags ) ) )
    {
        return D3DFWERR_BADDISPLAYMODE;
    }

    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize            = sizeof(ddsd);
    ddsd.dwFlags           = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
    ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE |
                             DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    ddsd.dwBackBufferCount = 1;

    if( m_bIsStereo )
    {
        ddsd.ddsCaps.dwCaps  |= DDSCAPS_VIDEOMEMORY;
        ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_STEREOSURFACELEFT;
    }

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
    {
        if( hr != DDERR_OUTOFVIDEOMEMORY )
            return D3DFWERR_NOPRIMARY;
        return DDERR_OUTOFVIDEOMEMORY;
    }

    DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };
    if( FAILED( hr = m_pddsFrontBuffer->GetAttachedSurface( &ddscaps,
                                                &m_pddsBackBuffer ) ) )
    {
        return D3DFWERR_NOBACKBUFFER;
    }

    m_pddsBackBuffer->AddRef();

    if( m_bIsStereo )
    {
        DDSCAPS2 ddscaps = { 0, DDSCAPS2_STEREOSURFACELEFT, 0, 0 };
        if( FAILED( hr = m_pddsBackBuffer->GetAttachedSurface( &ddscaps,
                                                    &m_pddsBackBufferLeft ) ) )
        {
            return D3DFWERR_NOBACKBUFFER;
        }
        m_pddsBackBufferLeft->AddRef();
    }

    return S_OK;
}




// 0x00408850, 582 bytes.
HRESULT CD3DFramework7::CreateWindowedBuffers()
{
    HRESULT hr;

    GetClientRect( m_hWnd, &m_rcScreenRect );
    ClientToScreen( m_hWnd, (POINT*)&m_rcScreenRect.left );
    ClientToScreen( m_hWnd, (POINT*)&m_rcScreenRect.right );
    m_dwRenderWidth  = m_rcScreenRect.right  - m_rcScreenRect.left;
    m_dwRenderHeight = m_rcScreenRect.bottom - m_rcScreenRect.top;

    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(ddsd) );
    ddsd.dwSize         = sizeof(ddsd);
    ddsd.dwFlags        = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsFrontBuffer, NULL ) ) )
    {
        if( hr != DDERR_OUTOFVIDEOMEMORY )
            return D3DFWERR_NOPRIMARY;
        return DDERR_OUTOFVIDEOMEMORY;
    }

    LPDIRECTDRAWCLIPPER pcClipper;
    if( FAILED( hr = m_pDD->CreateClipper( 0, &pcClipper, NULL ) ) )
    {
        return D3DFWERR_NOCLIPPER;
    }

    pcClipper->SetHWnd( 0, m_hWnd );
    m_pddsFrontBuffer->SetClipper( pcClipper );
    SAFE_RELEASE( pcClipper );

    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
    ddsd.dwWidth        = m_dwRenderWidth;
    ddsd.dwHeight       = m_dwRenderHeight;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsBackBuffer, NULL ) ) )
    {
        if( hr != DDERR_OUTOFVIDEOMEMORY )
            return D3DFWERR_NOBACKBUFFER;
        return DDERR_OUTOFVIDEOMEMORY;
    }

    return S_OK;
}




// 0x00408AA0, 293 bytes.
HRESULT CD3DFramework7::CreateClipperForBackBuffer()
{
    HRESULT hr;

    LPDIRECTDRAWCLIPPER pcClipper;
    if( FAILED( hr = m_pDD->CreateClipper( 0, &pcClipper, NULL ) ) )
    {
        return D3DFWERR_NOBACKBUFFER;
    }

    struct
    {
        RGNDATAHEADER rdh;
        RECT          rect;
    } rgnData;

    ZeroMemory( &rgnData, sizeof(rgnData) );
    rgnData.rdh.dwSize    = sizeof(RGNDATAHEADER);
    rgnData.rdh.iType     = RDH_RECTANGLES;
    rgnData.rdh.nCount    = 1;
    rgnData.rdh.nRgnSize  = sizeof(RECT);
    rgnData.rdh.rcBound.left   = 0;
    rgnData.rdh.rcBound.top    = 0;
    rgnData.rdh.rcBound.right  = m_dwRenderWidth;
    rgnData.rdh.rcBound.bottom = m_dwRenderHeight;
    rgnData.rect.left   = 0;
    rgnData.rect.top    = 0;
    rgnData.rect.right  = m_dwRenderWidth;
    rgnData.rect.bottom = m_dwRenderHeight;

    if( FAILED( hr = pcClipper->SetClipList( (RGNDATA*)&rgnData, 0 ) ) )
    {
        return D3DFWERR_NOBACKBUFFER;
    }

    if( FAILED( hr = m_pddsBackBuffer->SetClipper( pcClipper ) ) )
    {
        return D3DFWERR_NOBACKBUFFER;
    }

    SAFE_RELEASE( pcClipper );

    return S_OK;
}




// 0x00408CD0, 219 bytes.
HRESULT CD3DFramework7::CreateDirect3D( GUID* pDeviceGUID )
{
    if( FAILED( m_pDD->QueryInterface( IID_IDirect3D7, (VOID**)&m_pD3D ) ) )
    {
        return D3DFWERR_NODIRECT3D;
    }

    if( FAILED( m_pD3D->CreateDevice( *pDeviceGUID, m_pddsBackBuffer,
                                      &m_pd3dDevice) ) )
    {
        return D3DFWERR_NO3DDEVICE;
    }

    D3DVIEWPORT7 vp = { 0, 0, m_dwRenderWidth, m_dwRenderHeight, 0.0f, 1.0f };

    if( FAILED( m_pd3dDevice->SetViewport( &vp ) ) )
    {
        return D3DFWERR_NOVIEWPORT;
    }

    return S_OK;
}




// 0x00408370, 517 bytes.
HRESULT CD3DFramework7::CreateZBuffer( GUID* pDeviceGUID )
{
    HRESULT hr;

    D3DDEVICEDESC7 ddDesc;
    m_pd3dDevice->GetCaps( &ddDesc );
    if( ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )
        return S_OK;

    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsBackBuffer->GetSurfaceDesc( &ddsd );

    ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | m_dwDeviceMemType;
    ddsd.ddpfPixelFormat.dwSize = 0;

    m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                (VOID*)&ddsd.ddpfPixelFormat );
    if( 0 == ddsd.ddpfPixelFormat.dwSize )
    {
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
        m_pD3D->EnumZBufferFormats( *pDeviceGUID, EnumZBufferFormatsCallback,
                                    (VOID*)&ddsd.ddpfPixelFormat );

        if( 0 == ddsd.ddpfPixelFormat.dwSize )
        {
            return D3DFWERR_NOZBUFFER;
        }
    }

    if( FAILED( hr = m_pDD->CreateSurface( &ddsd, &m_pddsZBuffer, NULL ) ) )
    {
        if( hr != DDERR_OUTOFVIDEOMEMORY )
            return D3DFWERR_NOZBUFFER;
        return DDERR_OUTOFVIDEOMEMORY;
    }

    if( FAILED( m_pddsBackBuffer->AddAttachedSurface( m_pddsZBuffer ) ) )
    {
        return D3DFWERR_NOZBUFFER;
    }

    if( m_bIsStereo )
    {
        if( FAILED( m_pddsBackBufferLeft->AddAttachedSurface( m_pddsZBuffer ) ) )
        {
            return D3DFWERR_NOZBUFFER;
        }
    }

    if( FAILED( m_pd3dDevice->SetRenderTarget( m_pddsBackBuffer, 0L ) ) )
    {
        return D3DFWERR_NOZBUFFER;
    }

    return S_OK;
}




// 0x00408EB0, 33 bytes.
HRESULT CD3DFramework7::RestoreSurfaces()
{
    m_pDD->RestoreAllSurfaces();

    return S_OK;
}




// 0x00408FD0, 65 bytes.
VOID CD3DFramework7::Move( INT x, INT y )
{
    if( TRUE == m_bIsFullscreen )
        return;

    SetRect( &m_rcScreenRect, x, y, x + m_dwRenderWidth, y + m_dwRenderHeight );
}




// 0x00408F70, 92 bytes.
HRESULT CD3DFramework7::FlipToGDISurface( BOOL bDrawFrame )
{
    if( m_pDD && m_bIsFullscreen )
    {
        m_pDD->FlipToGDISurface();

        if( bDrawFrame )
        {
            DrawMenuBar( m_hWnd );
            RedrawWindow( m_hWnd, NULL, NULL, RDW_FRAME );
        }
    }

    return S_OK;
}




// 0x00408EE0, 144 bytes.
HRESULT CD3DFramework7::ShowFrame()
{
    if( NULL == m_pddsFrontBuffer )
        return D3DFWERR_NOTINITIALIZED;

    if( m_bIsFullscreen )
    {
        if( m_bIsStereo )
            return m_pddsFrontBuffer->Flip( NULL, DDFLIP_WAIT | DDFLIP_STEREO );
        else
            return m_pddsFrontBuffer->Flip( NULL, DDFLIP_WAIT );
    }
    else
    {
        return m_pddsFrontBuffer->Blt( &m_rcScreenRect, m_pddsBackBuffer,
                                       NULL, DDBLT_WAIT, NULL );
    }
}




// 0x00409340, 17 bytes.
DWORD CD3DFramework7::GetRenderWidth() const
{
    return m_dwRenderWidth;
}




// 0x00409360, 17 bytes.
DWORD CD3DFramework7::GetRenderHeight() const
{
    return m_dwRenderHeight;
}
