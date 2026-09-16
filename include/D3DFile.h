//-----------------------------------------------------------------------------
// File: D3DFile.h
//
// Desc: Support code for loading DirectX .X files.
//
// Copyright (c) 1997-1999 Microsoft Corporation. All rights reserved.
//
// Hopmon's modified copy: declarations checked against Hopmon.pdb.
//-----------------------------------------------------------------------------
#ifndef D3DFILE_H
#define D3DFILE_H
#include "common.h"
#include <d3d.h>
#include <dxfile.h>




//-----------------------------------------------------------------------------
// Name: struct MeshMaterialData
// Desc: Internal structure for holding material data for within a mesh. This
//       is used because multiple materials can be used in the same mesh.
//-----------------------------------------------------------------------------
#define MAX_MATERIAL        16
#define MAX_TEXTURE_NAME    80

// PDB type 0x18A4
struct MeshMaterialData
{
    friend struct LayoutCheck;

    D3DMATERIAL7 m_mtrl;                        // 0x00
    TCHAR        m_strTexture[MAX_TEXTURE_NAME]; // 0x44
    DWORD        m_dwNumIndices;                 // 0x94
};
ASSERT_SIZE(MeshMaterialData, 0x98);




//-----------------------------------------------------------------------------
// Name: class CD3DFileObject
// Desc: Internal class for objects in a .X file
//-----------------------------------------------------------------------------
// PDB type 0x1966
class CD3DFileObject
{
    friend struct LayoutCheck;

    // Common data
    TCHAR            m_strName[80];   // 0x00
    CD3DFileObject*  m_pNext;         // 0x50
    CD3DFileObject*  m_pChild;        // 0x54

    // For file frames
    D3DMATRIX        m_mat;           // 0x58

    // For file meshes
    BOOL             m_bHasMeshData;  // 0x98
    DWORD            m_dwNumVertices; // 0x9C
    D3DVERTEX*       m_pVertices;     // 0xA0
    DWORD            m_dwNumIndices;  // 0xA4
    WORD*            m_pIndices;      // 0xA8
    DWORD            m_dwNumMaterials; // 0xAC
    MeshMaterialData m_Material[MAX_MATERIAL]; // 0xB0
    BOOL             m_bHasAlpha;     // 0xA30

public:
    // Initializing functions
    VOID    AddNext( CD3DFileObject* );
    VOID    AddChild( CD3DFileObject* );
    VOID    SetName( TCHAR* strName )     { strcpy( m_strName, strName ); }
    VOID    SetMatrix( D3DMATRIX* pmat ) { m_mat = *pmat; }
    VOID    SetNormals( D3DVECTOR* pNormals );
    VOID    SetTextureCoords( FLOAT* pTexCoords );
    VOID    SetMaterialData( DWORD dwMaterial, D3DMATERIAL7* pmtrl, TCHAR*strName );
    VOID    AddFace( DWORD dwMaterial, DWORD* pFaceData, DWORD dwNumFaces );
    HRESULT ComputeNormals();
    HRESULT SetMeshGeometry( D3DVECTOR* pvVertices, DWORD dwNumVertices,
                             DWORD* pFaces, DWORD dwNumFaces );
    
    // Access functions
    TCHAR*          GetName()   { return m_strName; }
    CD3DFileObject* GetNext()   { return m_pNext; }
    CD3DFileObject* GetChild()  { return m_pChild; }
    D3DMATRIX*      GetMatrix() { return &m_mat; }
    HRESULT         GetMeshGeometry( D3DVERTEX** ppVertices,
                                     DWORD* pdwNumVertices, WORD** ppIndices,
                                     DWORD* pdwNumIndices );

    // Common functions
    VOID    Render( LPDIRECT3DDEVICE7 pd3dDevice , BOOL bAlpha );
    BOOL    EnumObjects( BOOL (*fnCallback)(CD3DFileObject*,D3DMATRIX*,VOID*),
                         D3DMATRIX* pmat, VOID* pContext );

    // Constuctor / destructor
    CD3DFileObject( TCHAR* strName );
    ~CD3DFileObject();
};
ASSERT_SIZE(CD3DFileObject, 0xA34);




//-----------------------------------------------------------------------------
// Name: class CD3DFile
// Desc:
//-----------------------------------------------------------------------------
// PDB type 0x18FF
class CD3DFile
{
    friend struct LayoutCheck;

    CD3DFileObject*   m_pRoot; // 0x00

public:
    HRESULT    GetMeshVertices( TCHAR* strName, D3DVERTEX** ppVertices,
                                DWORD* pdwNumVertices );
    HRESULT    GetMeshIndices( TCHAR* strName, WORD** ppIndices,
                               DWORD* pdwNumIndices );
    
    CD3DFileObject* FindObject( TCHAR* strName );
    VOID            EnumObjects( BOOL (*fnCallback)(CD3DFileObject*,D3DMATRIX*,VOID*),
                                 D3DMATRIX* pmat, VOID* pContext );
    VOID            Scale( FLOAT fScale );
    VOID            Shift( D3DVECTOR vector );
    VOID            RotateX( FLOAT angle );
    VOID            RotateY( FLOAT angle );
    VOID            RotateZ( FLOAT angle );
    VOID            GetBoxMin( D3DVECTOR& minVertex );
    VOID            GetBoxMax( D3DVECTOR& maxVertex );

    HRESULT LoadFromFile( TCHAR* strFilename );
    HRESULT LoadFromResource( WORD resId );
    HRESULT Render( LPDIRECT3DDEVICE7 );

    CD3DFile();
    ~CD3DFile();
};
ASSERT_SIZE(CD3DFile, 0x4);




//-----------------------------------------------------------------------------
// Name: ParseXXXX(), the .X file loader, and its EnumObjects callbacks
//-----------------------------------------------------------------------------
DWORD*  GetFace( DWORD* pFaceData, DWORD dwFace );
DWORD   GetNumIndices( DWORD* pFaceData, DWORD dwNumFaces );
HRESULT ParseMaterial( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pMesh, DWORD dwMaterial );
HRESULT ParseMeshMaterialList( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pMesh );
HRESULT ParseMesh( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pParentFrame );
HRESULT ParseFrame( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pParentFrame );

BOOL ScaleMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL ShiftMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL RotateXMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL RotateYMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL RotateZMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL GetBoxMinMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL GetBoxMaxMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );
BOOL FindMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext );




#endif

