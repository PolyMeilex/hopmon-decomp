// Copyright (c) 1996-1999 Microsoft Corporation. All rights reserved

#include "common.h"
#include <tchar.h>
#include "D3DUtil.h"
#include "D3DMath.h"
#include "D3DTextr.h"
#include "dxfile.h"
#include "rmxfguid.h"
#include "D3DFile.h"
#include <float.h>

#include "rmxftmpl.h"
#define D3DRM_XTEMPLATE_BYTES 3278




// 0x00407040, 54 bytes.
DWORD* GetFace( DWORD* pFaceData, DWORD dwFace )
{
    for( DWORD i=0; i<dwFace; i++ )
        pFaceData += (*pFaceData) + 1;

    return pFaceData;
}




// 0x00407080, 75 bytes.
DWORD GetNumIndices( DWORD* pFaceData, DWORD dwNumFaces )
{
    DWORD dwNumIndices = 0;
    while( dwNumFaces-- > 0 )
    {
        dwNumIndices += (*pFaceData-2)*3;
        pFaceData += *pFaceData + 1;
    }

    return dwNumIndices;
}




// 0x00406390, 292 bytes.
CD3DFileObject::CD3DFileObject( TCHAR* strName )
{
    m_pNext        = NULL;
    m_pChild       = NULL;
    m_strName[0]   = 0;
    m_bHasMeshData = FALSE;

    if( strName )
        lstrcpy( m_strName, strName );

    D3DUtil_SetIdentityMatrix( m_mat );

    D3DUtil_InitMaterial( m_Material[0].m_mtrl, 1.0f, 1.0f, 1.0f );
    ZeroMemory( m_Material, sizeof(m_Material) );
    m_dwNumMaterials = 0;
    m_bHasAlpha      = FALSE;

    m_dwNumVertices = 0L;
    m_pVertices     = NULL;
    m_dwNumIndices  = 0L;
    m_pIndices      = NULL;
}




// 0x004064C0, 227 bytes.
CD3DFileObject::~CD3DFileObject()
{
    SAFE_DELETE( m_pChild );
    SAFE_DELETE( m_pNext );

    SAFE_DELETE( m_pVertices );
    SAFE_DELETE( m_pIndices );
}




// 0x00405870, 48 bytes.
VOID CD3DFileObject::AddNext( CD3DFileObject* pNext )
{
    if( m_pNext )
        m_pNext->AddNext( pNext );
    else
        m_pNext = pNext;
}




// 0x004058A0, 48 bytes.
VOID CD3DFileObject::AddChild( CD3DFileObject* pChild )
{
    if( m_pChild )
        m_pChild->AddNext( pChild );
    else
        m_pChild = pChild;
}




// 0x004058D0, 140 bytes.
VOID CD3DFileObject::SetNormals( D3DVECTOR* pNormals )
{
    for( DWORD i=0; i<m_dwNumVertices; i++ )
    {
        m_pVertices[i].nx = pNormals[i].x;
        m_pVertices[i].ny = pNormals[i].y;
        m_pVertices[i].nz = pNormals[i].z;
    }
}




// 0x00405960, 112 bytes.
VOID CD3DFileObject::SetTextureCoords( FLOAT* pTexCoords )
{
    for( DWORD i=0; i<m_dwNumVertices; i++ )
    {
        m_pVertices[i].tu = pTexCoords[2*i+0];
        m_pVertices[i].tv = pTexCoords[2*i+1];
    }
}




// 0x004059D0, 110 bytes.
VOID CD3DFileObject::SetMaterialData( DWORD dwMaterial, D3DMATERIAL7* pmtrl,
                                      TCHAR* strName )
{
    if( dwMaterial < MAX_MATERIAL )
    {
        m_Material[dwMaterial].m_mtrl = *pmtrl;
        lstrcpyn( m_Material[dwMaterial].m_strTexture, strName, MAX_TEXTURE_NAME );

        if( pmtrl->diffuse.a < 1.0f )
            m_bHasAlpha = TRUE;
    }
}




// 0x00405A40, 332 bytes.
VOID CD3DFileObject::AddFace( DWORD dwMaterial, DWORD* pFaceData,
                              DWORD dwNumFaces )
{
    if( dwMaterial >= MAX_MATERIAL)
        return;

    if( m_dwNumMaterials < dwMaterial+1 )
        m_dwNumMaterials = dwMaterial+1;

    WORD* pIndices = m_pIndices;
    for( DWORD i=0; i<=dwMaterial; i++ )
        pIndices += m_Material[i].m_dwNumIndices;

    while( dwNumFaces-- )
    {
        DWORD dwNumVerticesPerFace = *pFaceData++;

        for( DWORD i=2; i<dwNumVerticesPerFace; i++ )
        {
            m_Material[dwMaterial].m_dwNumIndices += 3;
            *pIndices++ = (WORD)pFaceData[0];
            *pIndices++ = (WORD)pFaceData[i-1];
            *pIndices++ = (WORD)pFaceData[i];
        }

        pFaceData += dwNumVerticesPerFace;
    }
}




// 0x00405B90, 778 bytes.
HRESULT CD3DFileObject::ComputeNormals()
{
    D3DVECTOR* pNormals = new D3DVECTOR[m_dwNumVertices];
    ZeroMemory( pNormals, sizeof(D3DVECTOR)*m_dwNumVertices );

    for( DWORD i=0; i<m_dwNumIndices; i+=3 )
    {
        WORD a = m_pIndices[i+0];
        WORD b = m_pIndices[i+1];
        WORD c = m_pIndices[i+2];

        D3DVECTOR* v1 = (D3DVECTOR*)&m_pVertices[a];
        D3DVECTOR* v2 = (D3DVECTOR*)&m_pVertices[b];
        D3DVECTOR* v3 = (D3DVECTOR*)&m_pVertices[c];

        D3DVECTOR n = Normalize( CrossProduct( *v2-*v1, *v3-*v2 ) );

        pNormals[a] += n;
        pNormals[b] += n;
        pNormals[c] += n;
    }

    for( DWORD i=0; i<m_dwNumVertices; i++ )
    {
        if( Magnitude( pNormals[i] ) < 0.1f )
            pNormals[i] = D3DVECTOR( 0.0f, 0.0f, 1.0f );

        pNormals[i] = Normalize( pNormals[i] );
        m_pVertices[i].nx = pNormals[i].x;
        m_pVertices[i].ny = pNormals[i].y;
        m_pVertices[i].nz = pNormals[i].z;
    }

    delete pNormals;

    return S_OK;
}




// 0x00405EA0, 472 bytes.
HRESULT CD3DFileObject::SetMeshGeometry( D3DVECTOR* pVertexData,
                                     DWORD dwNumVertices, DWORD* pFaceData,
                                     DWORD dwNumFaces )
{
    m_dwNumVertices = dwNumVertices;
    m_pVertices     = new D3DVERTEX[m_dwNumVertices];
    if( NULL == m_pVertices )
        return E_FAIL;

    for( DWORD i=0; i< m_dwNumVertices; i++ )
    {
        ZeroMemory( &m_pVertices[i], sizeof(D3DVERTEX) );
        m_pVertices[i].x = pVertexData[i].x;
        m_pVertices[i].y = pVertexData[i].y;
        m_pVertices[i].z = pVertexData[i].z;
    }

    m_dwNumIndices = GetNumIndices( pFaceData, dwNumFaces );

    m_pIndices = new WORD[m_dwNumIndices];

    if( NULL == m_pIndices )
        return E_FAIL;

    m_bHasMeshData  = TRUE;

    return S_OK;
}




// 0x00406080, 95 bytes.
HRESULT CD3DFileObject::GetMeshGeometry( D3DVERTEX** ppVertices, DWORD* pdwNumVertices,
                                         WORD** ppIndices, DWORD* pdwNumIndices )
{
    if( ppVertices )     *ppVertices     = m_pVertices;
    if( pdwNumVertices ) *pdwNumVertices = m_dwNumVertices;
    if( ppIndices )      *ppIndices      = m_pIndices;
    if( pdwNumIndices )  *pdwNumIndices  = m_dwNumIndices;

    return S_OK;
}




// 0x004060E0, 464 bytes.
VOID CD3DFileObject::Render( LPDIRECT3DDEVICE7 pd3dDevice, BOOL bAlpha )
{
    if( m_bHasMeshData )
    {
        WORD* pIndices = m_pIndices;
        for( DWORD i=0; i <= m_dwNumMaterials; i++ )
        {
            if( 0L == m_Material[i].m_dwNumIndices )
                continue;

            if( bAlpha == m_bHasAlpha )
            {
                TCHAR* strTexture   = m_Material[i].m_strTexture;
                DWORD  dwNumIndices = m_Material[i].m_dwNumIndices;

                pd3dDevice->SetMaterial( &m_Material[i].m_mtrl );
                pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, D3DFVF_VERTEX,
                                                  m_pVertices, m_dwNumVertices,
                                                  pIndices, dwNumIndices, NULL );
            }

            pIndices += m_Material[i].m_dwNumIndices;
        }
    }
    else
    {
        if( m_pChild )
        {
            D3DMATRIX matWorldOld, matWorldNew;
            pd3dDevice->GetTransform( D3DTRANSFORMSTATE_WORLD, &matWorldOld );

            matWorldNew = m_mat * matWorldOld;
            pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matWorldNew );

            m_pChild->Render( pd3dDevice, bAlpha );

            pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matWorldOld );
        }
    }

    if( m_pNext )
        m_pNext->Render( pd3dDevice, bAlpha );
}




// 0x004062B0, 209 bytes.
BOOL CD3DFileObject::EnumObjects( BOOL (*fnCallback)(CD3DFileObject*,D3DMATRIX*,VOID*),
                                  D3DMATRIX* pmat, VOID* pContext )
{
    if( fnCallback( this, pmat, pContext ) == TRUE )
        return TRUE;

    if( m_pChild )
    {
        D3DMATRIX matSave = (*pmat);
        (*pmat) = (*pmat) * m_mat;

        if( m_pChild->EnumObjects( fnCallback, pmat, pContext ) == TRUE )
            return TRUE;

        (*pmat) = matSave;
    }

    if( m_pNext )
        if( m_pNext->EnumObjects( fnCallback, pmat, pContext ) == TRUE )
            return TRUE;

    return FALSE;
}




// 0x004065B0, 63 bytes.
HRESULT CD3DFile::GetMeshVertices( TCHAR* strName, D3DVERTEX** ppVertices,
                                   DWORD* pdwNumVertices )
{
    CD3DFileObject* pObject = FindObject( strName );
    if( pObject )
        return pObject->GetMeshGeometry( ppVertices, pdwNumVertices, NULL, NULL );

    return E_FAIL;
}




// 0x004065F0, 63 bytes.
HRESULT CD3DFile::GetMeshIndices( TCHAR* strName, WORD** ppIndices,
                                  DWORD* pdwNumIndices )
{
    CD3DFileObject* pObject = FindObject( strName );
    if( pObject )
        return pObject->GetMeshGeometry( NULL, NULL, ppIndices, pdwNumIndices );

    return E_FAIL;
}




// 0x00406630, 63 bytes.
CD3DFileObject* CD3DFile::FindObject( TCHAR* strName )
{
    if( NULL == strName )
        return m_pRoot;

    struct FINDMESHRECORD
    {
        TCHAR*          strName;
        CD3DFileObject* pObject;
    };

    FINDMESHRECORD data = { strName, NULL };

    EnumObjects( FindMeshCB, NULL, (VOID*)&data );
    return data.pObject;
}




// 0x00406670, 90 bytes.
VOID CD3DFile::EnumObjects( BOOL (*fnCallback)(CD3DFileObject*,D3DMATRIX*,VOID*),
                            D3DMATRIX* pmat, VOID* pContext )
{
    if( m_pRoot )
    {
        D3DMATRIX mat;

        if( pmat )
            mat = *pmat;
        else
            D3DUtil_SetIdentityMatrix( mat );

        m_pRoot->EnumObjects( fnCallback, &mat, pContext );
    }
}




// 0x004066D0, 32 bytes.
VOID CD3DFile::Scale( FLOAT fScale )
{
    EnumObjects( ScaleMeshCB, NULL, (VOID*)&fScale );
}




// 0x004066F0, 32 bytes.
VOID CD3DFile::Shift( D3DVECTOR vector )
{
    EnumObjects( ShiftMeshCB, NULL, (VOID*)&vector );
}




// 0x00406710, 32 bytes.
VOID CD3DFile::RotateX( FLOAT angle )
{
    EnumObjects( RotateXMeshCB, NULL, (VOID*)&angle );
}




// 0x00406730, 32 bytes.
VOID CD3DFile::RotateY( FLOAT angle )
{
    EnumObjects( RotateYMeshCB, NULL, (VOID*)&angle );
}




// 0x00406750, 32 bytes.
VOID CD3DFile::RotateZ( FLOAT angle )
{
    EnumObjects( RotateZMeshCB, NULL, (VOID*)&angle );
}




// 0x00406770, 103 bytes.
VOID CD3DFile::GetBoxMin( D3DVECTOR& minVertex )
{
    minVertex = D3DVECTOR( FLT_MAX, FLT_MAX, FLT_MAX );
    EnumObjects( GetBoxMinMeshCB, NULL, (VOID*)&minVertex );
}




// 0x004067E0, 103 bytes.
VOID CD3DFile::GetBoxMax( D3DVECTOR& maxVertex )
{
    maxVertex = D3DVECTOR( FLT_MIN, FLT_MIN, FLT_MIN );
    EnumObjects( GetBoxMaxMeshCB, NULL, (VOID*)&maxVertex );
}




// 0x00406850, 906 bytes. Renamed from the framework's Load().
HRESULT CD3DFile::LoadFromFile( TCHAR* strFilename )
{
    HRESULT                 hr;
    LPDIRECTXFILE           pDXFile;
    LPDIRECTXFILEENUMOBJECT pEnumObj;
    LPDIRECTXFILEDATA       pFileData;
    const GUID*             pGUID;
    CD3DFileObject*         pFrame = NULL;

    SAFE_DELETE( m_pRoot );

    if( FAILED( DirectXFileCreate( &pDXFile ) ) )
        return E_FAIL;
    if( FAILED( pDXFile->RegisterTemplates( (VOID*)D3DRM_XTEMPLATES,
                                            D3DRM_XTEMPLATE_BYTES ) ) )
    {
        pDXFile->Release();
        return E_FAIL;
    }

    hr = pDXFile->CreateEnumObject( strFilename, DXFILELOAD_FROMFILE, &pEnumObj );

    if( FAILED(hr) )
    {
        TCHAR strPath[512] = _T("");
        lstrcat( strPath, D3DUtil_GetDXSDKMediaPath() );
        lstrcat( strPath, strFilename );

        hr = pDXFile->CreateEnumObject( strPath, DXFILELOAD_FROMFILE,
                                        &pEnumObj );
        if( FAILED(hr) )
        {
            pDXFile->Release();
            return hr;
        }
    }

    m_pRoot = new CD3DFileObject( _T("D3DFile_Root") );

    while( SUCCEEDED( hr = pEnumObj->GetNextDataObject( &pFileData ) ) )
    {
        pFileData->GetType( &pGUID );

        if( *pGUID == TID_D3DRMFrame )
            ParseFrame( pFileData, m_pRoot );

        if( *pGUID == TID_D3DRMMesh )
            ParseMesh( pFileData, m_pRoot );

        pFileData->Release();
    }

    if( DXFILEERR_NOMOREOBJECTS == hr )
        hr = S_OK;
    else
        SAFE_DELETE( m_pRoot );

    pEnumObj->Release();
    pDXFile->Release();

    return hr;
}




// 0x00406BE0, 593 bytes.
HRESULT CD3DFile::LoadFromResource( WORD resId )
{
    LPDIRECTXFILE           pDXFile;
    LPDIRECTXFILEENUMOBJECT pEnumObj;
    LPDIRECTXFILEDATA       pFileData;
    const GUID*             pGUID;
    HRESULT                 hr = S_OK;

    SAFE_DELETE( m_pRoot );

    if( FAILED( DirectXFileCreate( &pDXFile ) ) )
        return E_FAIL;
    if( FAILED( pDXFile->RegisterTemplates( (VOID*)D3DRM_XTEMPLATES,
                                            D3DRM_XTEMPLATE_BYTES ) ) )
    {
        pDXFile->Release();
        return E_FAIL;
    }

    DXFILELOADRESOURCE dxlr;
    dxlr.hModule = NULL;
    dxlr.lpName  = MAKEINTRESOURCE( resId );
    dxlr.lpType  = RT_RCDATA;

    hr = pDXFile->CreateEnumObject( (VOID*)&dxlr, DXFILELOAD_FROMRESOURCE, &pEnumObj );
    if( FAILED(hr) )
    {
        pDXFile->Release();
        return hr;
    }

    m_pRoot = new CD3DFileObject( _T("D3DFile_Root") );

    while( SUCCEEDED( hr = pEnumObj->GetNextDataObject( &pFileData ) ) )
    {
        pFileData->GetType( &pGUID );

        if( *pGUID == TID_D3DRMFrame )
            ParseFrame( pFileData, m_pRoot );

        if( *pGUID == TID_D3DRMMesh )
            ParseMesh( pFileData, m_pRoot );

        pFileData->Release();
    }

    if( DXFILEERR_NOMOREOBJECTS == hr )
        hr = S_OK;
    else
        SAFE_DELETE( m_pRoot );

    pEnumObj->Release();
    pDXFile->Release();

    return hr;
}




// 0x00406E40, 394 bytes.
HRESULT CD3DFile::Render( LPDIRECT3DDEVICE7 pd3dDevice )
{
    LPDIRECTDRAWSURFACE7 pddsSavedTexture;
    D3DMATRIX    matSaved;
    D3DMATERIAL7 mtrlSaved;
    DWORD        dwAlphaState, dwSrcBlendState, dwDestBlendState;

    if( m_pRoot )
    {
        pd3dDevice->GetMaterial( &mtrlSaved );
        pd3dDevice->GetTexture( 0, &pddsSavedTexture );
        pd3dDevice->GetTransform( D3DTRANSFORMSTATE_WORLD, &matSaved );
        pd3dDevice->GetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, &dwAlphaState );
        pd3dDevice->GetRenderState( D3DRENDERSTATE_SRCBLEND,  &dwSrcBlendState );
        pd3dDevice->GetRenderState( D3DRENDERSTATE_DESTBLEND, &dwDestBlendState );

        m_pRoot->Render( pd3dDevice, FALSE );

        pd3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
        pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA );
        pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
        m_pRoot->Render( pd3dDevice, TRUE );

        pd3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, dwAlphaState );
        pd3dDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,  dwSrcBlendState );
        pd3dDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND, dwDestBlendState );
        pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matSaved );
        pd3dDevice->SetTexture( 0, pddsSavedTexture );
        pd3dDevice->SetMaterial( &mtrlSaved );

        if( pddsSavedTexture )
            pddsSavedTexture->Release();
    }

    return S_OK;
}




// 0x00406FD0, 23 bytes.
CD3DFile::CD3DFile()
{
    m_pRoot = NULL;
}




// 0x00406FF0, 66 bytes.
CD3DFile::~CD3DFile()
{
    SAFE_DELETE( m_pRoot );
}




// 0x004070D0, 530 bytes.
HRESULT ParseMaterial( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pMesh,
                       DWORD dwMaterial )
{
    LONG  pData;
    DWORD dwSize;
    TCHAR strTexture[128];

    if( FAILED( pFileData->GetData( NULL, &dwSize, (VOID**)&pData ) ) )
        return NULL;

    D3DMATERIAL7 mtrl;
    ZeroMemory( &mtrl, sizeof(mtrl) );
    memcpy( &mtrl.diffuse,  (VOID*)(pData+0),  sizeof(FLOAT)*4 );
    memcpy( &mtrl.ambient,  (VOID*)(pData+0),  sizeof(FLOAT)*4 );
    memcpy( &mtrl.power,    (VOID*)(pData+16), sizeof(FLOAT)*1 );
    memcpy( &mtrl.specular, (VOID*)(pData+20), sizeof(FLOAT)*3 );
    memcpy( &mtrl.emissive, (VOID*)(pData+32), sizeof(FLOAT)*3 );
    strTexture[0] = 0;

    LPDIRECTXFILEOBJECT pChildObj;
    if( SUCCEEDED( pFileData->GetNextObject(&pChildObj) ) )
    {
        LPDIRECTXFILEDATA pChildData;

        if( SUCCEEDED( pChildObj->QueryInterface( IID_IDirectXFileData,
                                                    (VOID**)&pChildData) ) )
        {
            const GUID* pguid;
            pChildData->GetType( &pguid );

            if( TID_D3DRMTextureFilename == *pguid )
            {
                TCHAR** string;

                if( FAILED( pChildData->GetData( NULL, &dwSize, (VOID**)&string ) ) )
                    return NULL;

                lstrcpyn( strTexture, *string, 128 );
            }

            pChildData->Release();
        }

        pChildObj->Release();
    }

    pMesh->SetMaterialData( dwMaterial, &mtrl, strTexture );

    return S_OK;
}




// 0x004072F0, 334 bytes.
HRESULT ParseMeshMaterialList( LPDIRECTXFILEDATA pFileData,
                               CD3DFileObject* pMesh )
{
    LPDIRECTXFILEOBJECT        pChildObj;
    LPDIRECTXFILEDATA          pChildData;
    LPDIRECTXFILEDATAREFERENCE pChildDataRef;
    DWORD                      dwMaterial = 0;

    while( SUCCEEDED( pFileData->GetNextObject( &pChildObj ) ) )
    {
        if( SUCCEEDED( pChildObj->QueryInterface( IID_IDirectXFileData,
                                                    (VOID**)&pChildData) ) )
        {
            const GUID* pguid;
            pChildData->GetType( &pguid );

            if( TID_D3DRMMaterial == *pguid )
            {
                ParseMaterial(pChildData, pMesh, dwMaterial++);
            }

            pChildData->Release();
        }

        if( SUCCEEDED( pChildObj->QueryInterface( IID_IDirectXFileDataReference,
                                                    (VOID**)&pChildDataRef) ) )
        {
            if( SUCCEEDED( pChildDataRef->Resolve( &pChildData ) ) )
            {
                const GUID* pguid;
                pChildData->GetType( &pguid );

                if( TID_D3DRMMaterial == *pguid )
                {
                    ParseMaterial( pChildData, pMesh, dwMaterial++ );
                }

                pChildData->Release();
            }
            pChildDataRef->Release();
        }

        pChildObj->Release();
    }
    return S_OK;
}




// 0x00407440, 1063 bytes.
HRESULT ParseMesh( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pParentFrame )
{
    DWORD dwNameLen=80;
    TCHAR strName[80];
    if( FAILED( pFileData->GetName( strName, &dwNameLen ) ) )
        return E_FAIL;

    LONG  pData;
    DWORD dwSize;
    if( FAILED( pFileData->GetData( NULL, &dwSize, (VOID**)&pData ) ) )
        return E_FAIL;

    DWORD      dwNumVertices =    *((DWORD*)pData); pData += 4;
    D3DVECTOR* pVertices     = ((D3DVECTOR*)pData); pData += 12*dwNumVertices;
    DWORD      dwNumFaces    =    *((DWORD*)pData); pData += 4;
    DWORD*     pFaceData     =      (DWORD*)pData;

    CD3DFileObject* pMesh = new CD3DFileObject( strName );
    pMesh->SetMeshGeometry( pVertices, dwNumVertices, pFaceData, dwNumFaces );

    BOOL bHasNormals = FALSE;
    BOOL bHasMaterials = FALSE;

    LPDIRECTXFILEOBJECT pChildObj;
    while( SUCCEEDED( pFileData->GetNextObject( &pChildObj ) ) )
    {
        LPDIRECTXFILEDATA pChildData;

        if( SUCCEEDED( pChildObj->QueryInterface( IID_IDirectXFileData,
                                                  (VOID**)&pChildData ) ) )
        {
            const GUID* pGUID;
            LONG        pData;
            DWORD       dwSize;

            pChildData->GetType( &pGUID );
            if( FAILED( pChildData->GetData( NULL, &dwSize, (VOID**)&pData ) ) )
            {
                delete pMesh;
                return NULL;
            }

            if( TID_D3DRMMeshMaterialList == *pGUID )
            {
                DWORD  dwNumMaterials = *((DWORD*)pData);   pData += 4;
                DWORD  dwNumMatFaces  = *((DWORD*)pData);   pData += 4;
                DWORD* pMatFace       =   (DWORD*)pData;

                if( dwNumMaterials == 1 || dwNumMatFaces != dwNumFaces )
                {
                    pMesh->AddFace( 0, pFaceData, dwNumFaces );
                }
                else
                {
                    for( DWORD mat=0; mat<dwNumMaterials; mat++ )
                    {
                        for( DWORD face=0; face<dwNumMatFaces; face++ )
                        {
                            if( pMatFace[face] == mat )
                                pMesh->AddFace( mat, GetFace( pFaceData, face ), 1 );
                        }
                    }
                }

                ParseMeshMaterialList( pChildData, pMesh );
                bHasMaterials = TRUE;
            }

            if( TID_D3DRMMeshNormals == *pGUID )
            {
                DWORD      dwNumNormals = *((DWORD*)pData);
                D3DVECTOR* pNormals     = (D3DVECTOR*)(pData+4);

                if( dwNumNormals == dwNumVertices )
                {
                    pMesh->SetNormals( pNormals );
                    bHasNormals = TRUE;
                }
            }

            if( TID_D3DRMMeshTextureCoords == *pGUID )
            {
                DWORD  dwNumTexCoords = *((DWORD*)pData);
                FLOAT* pTexCoords     = (FLOAT*)(pData+4);

                if( dwNumTexCoords == dwNumVertices )
                    pMesh->SetTextureCoords( pTexCoords );
            }

            pChildData->Release();
        }

        pChildObj->Release();
    }

    if( FALSE == bHasMaterials )
        pMesh->AddFace( 0, pFaceData, dwNumFaces );

    if( FALSE == bHasNormals )
        pMesh->ComputeNormals();

    pParentFrame->AddChild( pMesh );
    return S_OK;
}




// 0x00407870, 659 bytes.
HRESULT ParseFrame( LPDIRECTXFILEDATA pFileData, CD3DFileObject* pParentFrame )
{
    DWORD dwNameLen=80;
    TCHAR strName[80];
    if( FAILED( pFileData->GetName( strName, &dwNameLen ) ) )
        return E_FAIL;

    CD3DFileObject* pFrame = new CD3DFileObject( strName );

    LPDIRECTXFILEOBJECT pChildObj;
    while( SUCCEEDED( pFileData->GetNextObject( &pChildObj ) ) )
    {
        LPDIRECTXFILEDATA pChildData;
        if( SUCCEEDED( pChildObj->QueryInterface( IID_IDirectXFileData,
                            (VOID**)&pChildData ) ) )
        {
            const GUID* pGUID;
            pChildData->GetType( &pGUID );

            if( TID_D3DRMFrame == *pGUID )
                ParseFrame( pChildData, pFrame );

            if( TID_D3DRMMesh == *pGUID )
                ParseMesh( pChildData, pFrame );

            if( TID_D3DRMFrameTransformMatrix == *pGUID )
            {
                DWORD dwSize;
                VOID* pData;
                if( FAILED( pChildData->GetData( NULL, &dwSize, &pData ) ) )
                {
                    delete pFrame;
                    return NULL;
                }

                if( dwSize == sizeof(D3DMATRIX) )
                {
                    D3DMATRIX* pmatFrame = (D3DMATRIX*)pData;
                    pmatFrame->_13 *= -1.0f;
                    pmatFrame->_31 *= -1.0f;
                    pmatFrame->_23 *= -1.0f;
                    pmatFrame->_32 *= -1.0f;
                    pmatFrame->_43 *= -1.0f;
                    pFrame->SetMatrix( pmatFrame );
                }
            }

            pChildData->Release();
        }

        pChildObj->Release();
    }

    pParentFrame->AddChild( pFrame );
    return S_OK;
}




// 0x00407B10, 177 bytes.
BOOL ScaleMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            pVertices[i].x *= (*((FLOAT*)pContext));
            pVertices[i].y *= (*((FLOAT*)pContext));
            pVertices[i].z *= (*((FLOAT*)pContext));
        }
    }

    return FALSE;
}




// 0x00407BD0, 188 bytes.
BOOL ShiftMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            D3DVECTOR* pShift = (D3DVECTOR*)pContext;
            pVertices[i].x += pShift->x;
            pVertices[i].y += pShift->y;
            pVertices[i].z += pShift->z;
        }
    }

    return FALSE;
}




// 0x00407C90, 154 bytes.
BOOL RotateXMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        D3DMATRIX mat;
        D3DUtil_SetRotateXMatrix( mat, *((FLOAT*)pContext) );

        D3DVERTEX vTemp;
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            D3DMath_VertexMatrixMultiply( vTemp, pVertices[i], mat );
            pVertices[i] = vTemp;
        }
    }

    return FALSE;
}




// 0x00407D30, 154 bytes.
BOOL RotateYMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        D3DMATRIX mat;
        D3DUtil_SetRotateYMatrix( mat, *((FLOAT*)pContext) );

        D3DVERTEX vTemp;
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            D3DMath_VertexMatrixMultiply( vTemp, pVertices[i], mat );
            pVertices[i] = vTemp;
        }
    }

    return FALSE;
}




// 0x00407DD0, 154 bytes.
BOOL RotateZMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        D3DMATRIX mat;
        D3DUtil_SetRotateZMatrix( mat, *((FLOAT*)pContext) );

        D3DVERTEX vTemp;
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            D3DMath_VertexMatrixMultiply( vTemp, pVertices[i], mat );
            pVertices[i] = vTemp;
        }
    }

    return FALSE;
}




// 0x00407E70, 206 bytes.
BOOL GetBoxMinMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        D3DVECTOR* pMin = (D3DVECTOR*)pContext;
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            if( pVertices[i].x < pMin->x ) pMin->x = pVertices[i].x;
            if( pVertices[i].y < pMin->y ) pMin->y = pVertices[i].y;
            if( pVertices[i].z < pMin->z ) pMin->z = pVertices[i].z;
        }
    }

    return FALSE;
}




// 0x00407F40, 206 bytes.
BOOL GetBoxMaxMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    D3DVERTEX* pVertices;
    DWORD      dwNumVertices;

    if( SUCCEEDED( pFileObject->GetMeshGeometry( &pVertices, &dwNumVertices,
                                                 NULL, NULL ) ) )
    {
        D3DVECTOR* pMax = (D3DVECTOR*)pContext;
        for( DWORD i=0; i<dwNumVertices; i++ )
        {
            if( pVertices[i].x > pMax->x ) pMax->x = pVertices[i].x;
            if( pVertices[i].y > pMax->y ) pMax->y = pVertices[i].y;
            if( pVertices[i].z > pMax->z ) pMax->z = pVertices[i].z;
        }
    }

    return FALSE;
}




// 0x00408010, 57 bytes.
BOOL FindMeshCB( CD3DFileObject* pFileObject, D3DMATRIX*, VOID* pContext )
{
    struct FINDMESHRECORD
    {
        TCHAR*          strName;
        CD3DFileObject* pObject;
    };

    FINDMESHRECORD* data = (FINDMESHRECORD*)pContext;

    if( 0 == lstrcmpi( data->strName, pFileObject->GetName() ) )
    {
        data->pObject = pFileObject;
        return TRUE;
    }

    return FALSE;
}
