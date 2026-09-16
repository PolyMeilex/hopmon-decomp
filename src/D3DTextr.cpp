//-----------------------------------------------------------------------------
// File: D3DTextr.cpp
//
// Desc: Functions to manage textures, including creating (loading from a
//       resource), restoring lost surfaces, invalidating, and destroying.
//
//       Note: the implementation of these functions maintains an internal list
//       of loaded textures.
//
// Copyright (c) 1996-1999 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#include "D3DTextr.h"
#include "D3DUtil.h"

//-----------------------------------------------------------------------------
// Linked list structure to hold info per texture (Hopmon modified: textures
// are loaded by resource ID rather than filename, and the container has a
// virtual dtor)
//-----------------------------------------------------------------------------
struct TextureContainer {
    TextureContainer* m_pNext;    // 0x04, linked list ptr

    WORD    m_wResId;             // 0x08, resource ID (doubles as the texture's name)
    DWORD   m_dwWidth;             // 0x0C
    DWORD   m_dwHeight;            // 0x10
    DWORD   m_dwStage;             // 0x14, texture stage (for multitexture devices)
    DWORD   m_dwBPP;               // 0x18
    DWORD   m_dwFlags;             // 0x1C
    BOOL    m_bHasAlpha;           // 0x20

    LPDIRECTDRAWSURFACE7 m_pddsSurface; // 0x24, surface of the texture
    HBITMAP m_hbmBitmap;                // 0x28, bitmap containing texture image
    DWORD*  m_pRGBAData;                 // 0x2C

public:
    HRESULT LoadImageData();
    HRESULT Restore( LPDIRECT3DDEVICE7 pd3dDevice );
    HRESULT CopyBitmapToSurface();
    HRESULT CopyRGBADataToSurface();

    TextureContainer( WORD wResId, DWORD dwStage, DWORD dwFlags );
    virtual ~TextureContainer();
};

// Local list of textures
static TextureContainer* g_ptcTextureList = NULL;

//-----------------------------------------------------------------------------
// Name: CD3DTextureManager
// Desc: Class used to automatically construct and destruct the static
//       texture engine class.
//-----------------------------------------------------------------------------
class CD3DTextureManager
{
public:
    CD3DTextureManager();
    ~CD3DTextureManager();
};

// 0x0040BEB0, 14 bytes.
CD3DTextureManager::CD3DTextureManager()
{
}

// 0x0040BEC0, 62 bytes.
CD3DTextureManager::~CD3DTextureManager()
{
    if (g_ptcTextureList) {
        delete g_ptcTextureList;
    }
}




//-----------------------------------------------------------------------------
// Name: struct TEXTURESEARCHINFO
// Desc: Structure used to search for texture formats
//-----------------------------------------------------------------------------
struct TEXTURESEARCHINFO
{
    DWORD dwDesiredBPP;   // Input for texture format search
    BOOL  bUseAlpha;
    BOOL  bUsePalette;
    BOOL  bFoundGoodFormat;

    DDPIXELFORMAT* pddpf; // Output of texture format search
};




//-----------------------------------------------------------------------------
// Name: TextureSearchCallback()
// Desc: Enumeration callback routine to find a best-matching texture format.
//       The param data is the DDPIXELFORMAT of the best-so-far matching
//       texture. Note: the desired BPP is passed in the dwSize field, and the
//       default BPP is passed in the dwFlags field.
//-----------------------------------------------------------------------------
// 0x0040BD40, 291 bytes. Not in functions.csv (S_LPROC32 in PDB).
HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf,
                                        VOID* param )
{
    if( NULL==pddpf || NULL==param )
        return DDENUMRET_OK;

    TEXTURESEARCHINFO* ptsi = (TEXTURESEARCHINFO*)param;

    // Skip any funky modes
    if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;

    // Check for palettized formats
    if( ptsi->bUsePalette )
    {
        if( !( pddpf->dwFlags & DDPF_PALETTEINDEXED8 ) )
            return DDENUMRET_OK;

        // Accept the first 8-bit palettized format we get
        memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
        ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    // Else, skip any paletized formats (all modes under 16bpp)
    if( pddpf->dwRGBBitCount < 16 )
        return DDENUMRET_OK;

    // Skip any FourCC formats
    if( pddpf->dwFourCC != 0 )
        return DDENUMRET_OK;

    // Skip any ARGB 4444 formats (which are best used for pre-authored
    // content designed speciafically for an ARGB 4444 format).
    if( pddpf->dwRGBAlphaBitMask == 0x0000f000 )
        return DDENUMRET_OK;

    // Make sure current alpha format agrees with requested format type
    if( (ptsi->bUseAlpha==TRUE) && !(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;
    if( (ptsi->bUseAlpha==FALSE) && (pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;

    // Check if we found a good match
    if( pddpf->dwRGBBitCount == ptsi->dwDesiredBPP )
    {
        memcpy( ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT) );
        ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: FindTexture()
// Desc: Searches the internal list of textures for a texture specified by
//       its resource ID. Returns the structure associated with that texture.
//-----------------------------------------------------------------------------
// 0x0040BE70, 55 bytes. Not in functions.csv (S_LPROC32 in PDB).
TextureContainer* FindTexture( WORD wResId )
{
    TextureContainer* ptcTexture = g_ptcTextureList;

    while( ptcTexture )
    {
        if( wResId == ptcTexture->m_wResId )
            return ptcTexture;
        ptcTexture = ptcTexture->m_pNext;
    }

    return NULL;
}




//-----------------------------------------------------------------------------
// Name: TextureContainer()
// Desc: Constructor for a texture object
//-----------------------------------------------------------------------------
// 0x0040BBC0, 144 bytes.
TextureContainer::TextureContainer( WORD wResId, DWORD dwStage, DWORD dwFlags )
{
    m_wResId      = wResId;
    m_dwWidth     = 0;
    m_dwHeight    = 0;
    m_dwStage     = dwStage;
    m_dwBPP       = 0;
    m_dwFlags     = dwFlags;
    m_bHasAlpha   = 0;

    m_pddsSurface = NULL;
    m_hbmBitmap   = NULL;
    m_pRGBAData   = NULL;

    // Add the texture to the head of the global texture list
    m_pNext = g_ptcTextureList;
    g_ptcTextureList = this;
}




//-----------------------------------------------------------------------------
// Name: ~TextureContainer()
// Desc: Destructs the contents of the texture container
//-----------------------------------------------------------------------------
// 0x0040BC50, 238 bytes.
TextureContainer::~TextureContainer()
{
    SAFE_RELEASE( m_pddsSurface );
    SAFE_DELETE(  m_pRGBAData );
    DeleteObject( m_hbmBitmap );

    // Remove the texture container from the global list
    if( g_ptcTextureList == this )
        g_ptcTextureList = m_pNext;
    else
    {
        for( TextureContainer* ptc=g_ptcTextureList; ptc; ptc=ptc->m_pNext )
            if( ptc->m_pNext == this )
                ptc->m_pNext = m_pNext;
    }
}




//-----------------------------------------------------------------------------
// Name: LoadImageData()
// Desc: Loads the texture map's image data from the executable's resources.
//       (Hopmon modified: textures are always loaded from a resource, the
//       file/DirectX SDK media path fallbacks were removed)
//-----------------------------------------------------------------------------
// 0x0040AAD0, 69 bytes.
HRESULT TextureContainer::LoadImageData()
{
    m_hbmBitmap = (HBITMAP)LoadImage( GetModuleHandle(NULL), MAKEINTRESOURCE(m_wResId),
                                      IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION );
    if( m_hbmBitmap )
        return S_OK;

    return E_NOTIMPL;
}




//-----------------------------------------------------------------------------
// Name: Restore()
// Desc: Rebuilds the texture surface using the new device.
//-----------------------------------------------------------------------------
// 0x0040AB20, 1169 bytes.
HRESULT TextureContainer::Restore( LPDIRECT3DDEVICE7 pd3dDevice )
{
    // Release any previously created objects
    SAFE_RELEASE( m_pddsSurface );

    // Check params
    if( NULL == pd3dDevice )
        return DDERR_INVALIDPARAMS;

    // Get the device caps
    D3DDEVICEDESC7 ddDesc;
    if( FAILED( pd3dDevice->GetCaps( &ddDesc) ) )
        return E_FAIL;

    // Setup the new surface desc
    DDSURFACEDESC2 ddsd;
    D3DUtil_InitSurfaceDesc( ddsd );
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
                           DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
    ddsd.dwTextureStage  = m_dwStage;
    ddsd.dwWidth         = m_dwWidth;
    ddsd.dwHeight        = m_dwHeight;

    // Turn on texture management for hardware devices
    if( ddDesc.deviceGUID == IID_IDirect3DHALDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else if( ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    // Adjust width and height to be powers of 2, if the device requires it
    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        for( ddsd.dwWidth=1;  m_dwWidth>ddsd.dwWidth;   ddsd.dwWidth<<=1 );
        for( ddsd.dwHeight=1; m_dwHeight>ddsd.dwHeight; ddsd.dwHeight<<=1 );
    }

    // Limit max texture sizes, if the driver can't handle large textures
    DWORD dwMaxWidth  = ddDesc.dwMaxTextureWidth;
    DWORD dwMaxHeight = ddDesc.dwMaxTextureHeight;
    ddsd.dwWidth  = min( ddsd.dwWidth,  ( dwMaxWidth  ? dwMaxWidth  : 256 ) );
    ddsd.dwHeight = min( ddsd.dwHeight, ( dwMaxHeight ? dwMaxHeight : 256 ) );

    // Make the texture square, if the driver requires it
    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
    {
        if( ddsd.dwWidth > ddsd.dwHeight ) ddsd.dwHeight = ddsd.dwWidth;
        else                               ddsd.dwWidth  = ddsd.dwHeight;
    }

    // Setup the structure to be used for texture enumration.
    TEXTURESEARCHINFO tsi;
    tsi.bFoundGoodFormat = FALSE;
    tsi.pddpf            = &ddsd.ddpfPixelFormat;
    tsi.dwDesiredBPP     = m_dwBPP;
    tsi.bUsePalette      = ( m_dwBPP <= 8 );
    tsi.bUseAlpha        = m_bHasAlpha;
    if( m_dwFlags & D3DTEXTR_16BITSPERPIXEL )
        tsi.dwDesiredBPP = 16;
    else if( m_dwFlags & D3DTEXTR_32BITSPERPIXEL )
        tsi.dwDesiredBPP = 32;

    if( m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) )
    {
        if( tsi.bUsePalette )
        {
            if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE )
            {
                tsi.bUseAlpha   = TRUE;
                tsi.bUsePalette = TRUE;
            }
            else
            {
                tsi.bUseAlpha   = TRUE;
                tsi.bUsePalette = FALSE;
            }
        }
    }

    // Enumerate the texture formats, and find the closest device-supported
    // texture pixel format
    pd3dDevice->EnumTextureFormats( TextureSearchCallback, &tsi );

    // If we couldn't find a format, let's try a default format
    if( FALSE == tsi.bFoundGoodFormat )
    {
        tsi.bUsePalette  = FALSE;
        tsi.dwDesiredBPP = 16;
        pd3dDevice->EnumTextureFormats( TextureSearchCallback, &tsi );

        // If we still fail, we cannot create this texture
        if( FALSE == tsi.bFoundGoodFormat )
            return E_FAIL;
    }

    // Get the DirectDraw interface for creating surfaces
    LPDIRECTDRAW7        pDD;
    LPDIRECTDRAWSURFACE7 pddsRender;
    pd3dDevice->GetRenderTarget( &pddsRender );
    pddsRender->GetDDInterface( (VOID**)&pDD );
    pddsRender->Release();

    // Create a new surface for the texture
    HRESULT hr = pDD->CreateSurface( &ddsd, &m_pddsSurface, NULL );

    // Done with DDraw
    pDD->Release();

    if( FAILED(hr) )
        return hr;

    // For bitmap-based textures, copy the bitmap image.
    if( m_hbmBitmap )
        return CopyBitmapToSurface();

    if( m_pRGBAData )
        return CopyRGBADataToSurface();

    // At this point, code can be added to handle other file formats (such as
    // .dds files, .jpg files, etc.).
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CopyBitmapToSurface()
// Desc: Copies the image of a bitmap into a surface
//-----------------------------------------------------------------------------
// 0x0040AFC0, 1634 bytes.
HRESULT TextureContainer::CopyBitmapToSurface()
{
    // Get a DDraw object to create a temporary surface
    LPDIRECTDRAW7 pDD;
    m_pddsSurface->GetDDInterface( (VOID**)&pDD );

    // Get the bitmap structure (to extract width, height, and bpp)
    BITMAP bm;
    GetObject( m_hbmBitmap, sizeof(BITMAP), &bm );

    // Setup the new surface desc
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsSurface->GetSurfaceDesc( &ddsd );
    ddsd.dwFlags          = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|
                            DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps   = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
    ddsd.ddsCaps.dwCaps2  = 0L;
    ddsd.dwWidth          = bm.bmWidth;
    ddsd.dwHeight         = bm.bmHeight;

    // Create a new surface for the texture
    LPDIRECTDRAWSURFACE7 pddsTempSurface;
    HRESULT hr;
    if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsTempSurface, NULL ) ) )
    {
        pDD->Release();
        return hr;
    }

    // Get a DC for the bitmap
    HDC hdcBitmap = CreateCompatibleDC( NULL );
    if( NULL == hdcBitmap )
    {
        pddsTempSurface->Release();
        pDD->Release();
        return hr;
    }
    SelectObject( hdcBitmap, m_hbmBitmap );

    // Handle palettized textures. Need to attach a palette
    if( ddsd.ddpfPixelFormat.dwRGBBitCount == 8 )
    {
        LPDIRECTDRAWPALETTE  pPalette;
        DWORD dwPaletteFlags = DDPCAPS_8BIT|DDPCAPS_ALLOW256;
        DWORD pe[256];
        WORD  wNumColors     = GetDIBColorTable( hdcBitmap, 0, 256, (RGBQUAD*)pe );

        // Create the color table
        for( WORD i=0; i<wNumColors; i++ )
        {
            pe[i] = RGB( GetBValue(pe[i]), GetGValue(pe[i]), GetRValue(pe[i]) );

            // Handle textures with transparent pixels
            if( m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) )
            {
                // Set alpha for opaque pixels
                if( m_dwFlags & D3DTEXTR_TRANSPARENTBLACK )
                {
                    if( pe[i] != 0x00000000 )
                        pe[i] |= 0xff000000;
                }
                else if( m_dwFlags & D3DTEXTR_TRANSPARENTWHITE )
                {
                    if( pe[i] != 0x00ffffff )
                        pe[i] |= 0xff000000;
                }
            }
        }
        // Add DDPCAPS_ALPHA flag for textures with transparent pixels
        if( m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) )
            dwPaletteFlags |= DDPCAPS_ALPHA;

        // Create & attach a palette
        pDD->CreatePalette( dwPaletteFlags, (PALETTEENTRY*)pe, &pPalette, NULL );
        pddsTempSurface->SetPalette( pPalette );
        m_pddsSurface->SetPalette( pPalette );
        SAFE_RELEASE( pPalette );
    }

    // Copy the bitmap image to the surface.
    HDC hdcSurface;
    if( SUCCEEDED( pddsTempSurface->GetDC( &hdcSurface ) ) )
    {
        BitBlt( hdcSurface, 0, 0, bm.bmWidth, bm.bmHeight, hdcBitmap, 0, 0,
                SRCCOPY );
        pddsTempSurface->ReleaseDC( hdcSurface );
    }
    DeleteDC( hdcBitmap );

    // Copy the temp surface to the real texture surface
    m_pddsSurface->Blt( NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL );

    // Done with the temp surface
    pddsTempSurface->Release();

    // For textures with real alpha (not palettized), set transparent bits
    if( ddsd.ddpfPixelFormat.dwRGBAlphaBitMask )
    {
        if( m_dwFlags & (D3DTEXTR_TRANSPARENTWHITE|D3DTEXTR_TRANSPARENTBLACK) )
        {
            // Lock the texture surface
            DDSURFACEDESC2 ddsd;
            ddsd.dwSize = sizeof(ddsd);
            while( m_pddsSurface->Lock( NULL, &ddsd, 0, NULL ) ==
                   DDERR_WASSTILLDRAWING );

            DWORD dwAlphaMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
            DWORD dwRGBMask   = ( ddsd.ddpfPixelFormat.dwRBitMask |
                                  ddsd.ddpfPixelFormat.dwGBitMask |
                                  ddsd.ddpfPixelFormat.dwBBitMask );
            DWORD dwColorkey  = 0x00000000; // Colorkey on black
            if( m_dwFlags & D3DTEXTR_TRANSPARENTWHITE )
                dwColorkey = dwRGBMask;     // Colorkey on white

            // Add an opaque alpha value to each non-colorkeyed pixel
            for( DWORD y=0; y<ddsd.dwHeight; y++ )
            {
                WORD*  p16 =  (WORD*)((BYTE*)ddsd.lpSurface + y*ddsd.lPitch);
                DWORD* p32 = (DWORD*)((BYTE*)ddsd.lpSurface + y*ddsd.lPitch);

                for( DWORD x=0; x<ddsd.dwWidth; x++ )
                {
                    if( ddsd.ddpfPixelFormat.dwRGBBitCount == 16 )
                    {
                        if( ( *p16 &= dwRGBMask ) != dwColorkey )
                            *p16 |= dwAlphaMask;
                        p16++;
                    }
                    if( ddsd.ddpfPixelFormat.dwRGBBitCount == 32 )
                    {
                        if( ( *p32 &= dwRGBMask ) != dwColorkey )
                            *p32 |= dwAlphaMask;
                        p32++;
                    }
                }
            }
            m_pddsSurface->Unlock( NULL );
        }
    }

    pDD->Release();

    return S_OK;;
}




//-----------------------------------------------------------------------------
// Name: CopyRGBADataToSurface()
// Desc: Invalidates the current texture objects and rebuilds new ones
//       using the new device.
//-----------------------------------------------------------------------------
// 0x0040B630, 1418 bytes.
HRESULT TextureContainer::CopyRGBADataToSurface()
{
    // Get a DDraw object to create a temporary surface
    LPDIRECTDRAW7 pDD;
    m_pddsSurface->GetDDInterface( (VOID**)&pDD );

    // Setup the new surface desc
    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsSurface->GetSurfaceDesc( &ddsd );
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|
                           DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
    ddsd.ddsCaps.dwCaps2 = 0L;
    ddsd.dwWidth         = m_dwWidth;
    ddsd.dwHeight        = m_dwHeight;

    // Create a new surface for the texture
    LPDIRECTDRAWSURFACE7 pddsTempSurface;
    HRESULT hr;
    if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsTempSurface, NULL ) ) )
    {
        pDD->Release();
        return NULL;
    }

    while( pddsTempSurface->Lock( NULL, &ddsd, 0, 0 ) == DDERR_WASSTILLDRAWING );
    DWORD lPitch = ddsd.lPitch;
    BYTE* pBytes = (BYTE*)ddsd.lpSurface;

    DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
    DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
    DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
    DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

    DWORD dwRShiftL = 8, dwRShiftR = 0;
    DWORD dwGShiftL = 8, dwGShiftR = 0;
    DWORD dwBShiftL = 8, dwBShiftR = 0;
    DWORD dwAShiftL = 8, dwAShiftR = 0;

    DWORD dwMask;
    for( dwMask=dwRMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwRShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwRShiftL--;

    for( dwMask=dwGMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwGShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwGShiftL--;

    for( dwMask=dwBMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwBShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwBShiftL--;

    for( dwMask=dwAMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwAShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwAShiftL--;

    for( DWORD y=0; y<ddsd.dwHeight; y++ )
    {
        DWORD* pDstData32 = (DWORD*)pBytes;
        WORD*  pDstData16 = (WORD*)pBytes;

        for( DWORD x=0; x<ddsd.dwWidth; x++ )
        {
            DWORD dwPixel = m_pRGBAData[y*ddsd.dwWidth+x];

            BYTE r = (BYTE)((dwPixel>>24)&0x000000ff);
            BYTE g = (BYTE)((dwPixel>>16)&0x000000ff);
            BYTE b = (BYTE)((dwPixel>> 8)&0x000000ff);
            BYTE a = (BYTE)((dwPixel>> 0)&0x000000ff);

            DWORD dr = ((r>>(dwRShiftL))<<dwRShiftR)&dwRMask;
            DWORD dg = ((g>>(dwGShiftL))<<dwGShiftR)&dwGMask;
            DWORD db = ((b>>(dwBShiftL))<<dwBShiftR)&dwBMask;
            DWORD da = ((a>>(dwAShiftL))<<dwAShiftR)&dwAMask;

            if( 32 == ddsd.ddpfPixelFormat.dwRGBBitCount )
                pDstData32[x] = (DWORD)(dr+dg+db+da);
            else
                pDstData16[x] = (WORD)(dr+dg+db+da);
        }

        pBytes += ddsd.lPitch;
    }

    pddsTempSurface->Unlock(0);

    // Copy the temp surface to the real texture surface
    m_pddsSurface->Blt( NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL );

    // Done with the temp objects
    pddsTempSurface->Release();
    pDD->Release();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_CreateTextureFromResource()
// Desc: Is passed a resource ID and creates a local bitmap from that
//       resource. The texture can not be used until it is restored, however.
//-----------------------------------------------------------------------------
// 0x0040A840, 300 bytes.
HRESULT D3DTextr_CreateTextureFromResource( WORD resId, DWORD dwFlags, DWORD dwStage )
{
    // Check parameters
    if( 0 == resId )
        return E_INVALIDARG;

    // Check first to see if the texture is already loaded
    if( NULL != FindTexture( resId ) )
        return S_OK;

    // Allocate and add the texture to the linked list of textures;
    TextureContainer* ptcTexture = new TextureContainer( resId, dwStage, dwFlags );
    if( NULL == ptcTexture )
        return E_OUTOFMEMORY;

    // Create a bitmap and load the texture resource into it,
    if( FAILED( ptcTexture->LoadImageData() ) )
    {
        delete ptcTexture;
        return E_FAIL;
    }

    // Save the image's dimensions
    if( ptcTexture->m_hbmBitmap )
    {
        BITMAP bm;
        GetObject( ptcTexture->m_hbmBitmap, sizeof(BITMAP), &bm );
        ptcTexture->m_dwWidth  = (DWORD)bm.bmWidth;
        ptcTexture->m_dwHeight = (DWORD)bm.bmHeight;
        ptcTexture->m_dwBPP    = (DWORD)bm.bmBitsPixel;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_CreateEmptyTexture()
// Desc: Creates an empty texture.
//-----------------------------------------------------------------------------
// 0x0040A970, 257 bytes.
HRESULT D3DTextr_CreateEmptyTexture( WORD id, DWORD dwWidth, DWORD dwHeight,
                                     DWORD dwFlags, DWORD dwStage )
{
    // Check parameters
    if( 0 == id )
        return E_INVALIDARG;

    // Check first to see if the texture is already loaded
    if( NULL != FindTexture( id ) )
        return E_FAIL;

    // Allocate and add the texture to the linked list of textures;
    TextureContainer* ptcTexture = new TextureContainer( id, dwStage, dwFlags );
    if( NULL == ptcTexture )
        return E_OUTOFMEMORY;

    // Save dimensions
    ptcTexture->m_dwWidth  = dwWidth;
    ptcTexture->m_dwHeight = dwHeight;
    ptcTexture->m_dwBPP    = 16;
    if( ptcTexture->m_dwFlags & D3DTEXTR_32BITSPERPIXEL )
        ptcTexture->m_dwBPP = 32;

    // Save alpha usage flag
    if( dwFlags & D3DTEXTR_CREATEWITHALPHA )
        ptcTexture->m_bHasAlpha = TRUE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_Restore()
// Desc: Invalidates the current texture objects and rebuilds new ones
//       using the new device.
//-----------------------------------------------------------------------------
// 0x0040A770, 49 bytes.
HRESULT D3DTextr_Restore( WORD resId, LPDIRECT3DDEVICE7 pd3dDevice )
{
    TextureContainer* ptcTexture = FindTexture( resId );
    if( NULL == ptcTexture )
        return DDERR_NOTFOUND;

    // Restore the texture (this recreates the new surface for this device).
    return ptcTexture->Restore( pd3dDevice );
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_RestoreAllTextures()
// Desc: This function is called when a mode is changed. It updates all
//       texture objects to be valid with the new device.
//-----------------------------------------------------------------------------
// 0x0040A800, 55 bytes.
HRESULT D3DTextr_RestoreAllTextures( LPDIRECT3DDEVICE7 pd3dDevice )
{
    TextureContainer* ptcTexture = g_ptcTextureList;

    while( ptcTexture )
    {
        D3DTextr_Restore( ptcTexture->m_wResId, pd3dDevice );
        ptcTexture = ptcTexture->m_pNext;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_Invalidate()
// Desc: Used to bump a texture out of (video) memory, this function
//       actually destroys the d3dtexture and ddsurface of the texture
//-----------------------------------------------------------------------------
// 0x0040A720, 78 bytes.
HRESULT D3DTextr_Invalidate( WORD resId )
{
    TextureContainer* ptcTexture = FindTexture( resId );
    if( NULL == ptcTexture )
        return DDERR_NOTFOUND;

    SAFE_RELEASE( ptcTexture->m_pddsSurface );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_InvalidateAllTextures()
// Desc: This function is called when a mode is changed. It invalidates
//       all texture objects so their device can be safely released.
//-----------------------------------------------------------------------------
// 0x0040A7B0, 74 bytes.
HRESULT D3DTextr_InvalidateAllTextures()
{
    TextureContainer* ptcTexture = g_ptcTextureList;

    while( ptcTexture )
    {
        SAFE_RELEASE( ptcTexture->m_pddsSurface );
        ptcTexture = ptcTexture->m_pNext;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_DestroyTexture()
// Desc: Frees the resources for the specified texture container
//-----------------------------------------------------------------------------
// 0x0040AA80, 79 bytes.
HRESULT D3DTextr_DestroyTexture( WORD resId )
{
    TextureContainer* ptcTexture = FindTexture( resId );

    SAFE_DELETE( ptcTexture );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DTextr_GetSurface()
// Desc: Returns a pointer to a d3dSurface from the resource ID of the texture
//-----------------------------------------------------------------------------
// 0x0040A6E0, 53 bytes.
LPDIRECTDRAWSURFACE7 D3DTextr_GetSurface( WORD resId )
{
    TextureContainer* ptcTexture = FindTexture( resId );

    return ptcTexture ? ptcTexture->m_pddsSurface : NULL;
}
