//-----------------------------------------------------------------------------
// File: D3DTextr.h
//
// Desc: Functions to manage textures, including creating (loading from a
//       file), restoring lost surfaces, invalidating, and destroying.
//
//       Note: the implementation of these fucntions maintain an internal list
//       of loaded textures. After creation, individual textures are referenced
//       via their resource IDs (Hopmon loads textures from resources).
//
// Copyright (c) 1997-1999 Microsoft Corporation. All rights reserved
//
// Hopmon's modified copy: declarations checked against Hopmon.pdb.
//-----------------------------------------------------------------------------
#ifndef D3DTEXTR_H
#define D3DTEXTR_H
#include <ddraw.h>
#include <d3d.h>




//-----------------------------------------------------------------------------
// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE7 D3DTextr_GetSurface( WORD resId );




//-----------------------------------------------------------------------------
// Texture invalidation and restoration functions
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Invalidate( WORD resId );
HRESULT D3DTextr_Restore( WORD resId, LPDIRECT3DDEVICE7 pd3dDevice );
HRESULT D3DTextr_InvalidateAllTextures();
HRESULT D3DTextr_RestoreAllTextures( LPDIRECT3DDEVICE7 pd3dDevice );




//-----------------------------------------------------------------------------
// Texture creation and deletion functions
//-----------------------------------------------------------------------------
#define D3DTEXTR_TRANSPARENTWHITE 0x00000001
#define D3DTEXTR_TRANSPARENTBLACK 0x00000002
#define D3DTEXTR_32BITSPERPIXEL   0x00000004
#define D3DTEXTR_16BITSPERPIXEL   0x00000008
#define D3DTEXTR_CREATEWITHALPHA  0x00000010


HRESULT D3DTextr_CreateTextureFromResource( WORD resId, DWORD dwFlags=0L,
                                            DWORD dwStage=0L );
HRESULT D3DTextr_CreateEmptyTexture( WORD id, DWORD dwWidth,
                                     DWORD dwHeight, DWORD dwFlags,
                                     DWORD dwStage );
HRESULT D3DTextr_DestroyTexture( WORD resId );



#endif // D3DTEXTR_H
