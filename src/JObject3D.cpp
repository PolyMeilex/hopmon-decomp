#include "JObject3D.h"
#include "JList.h"
#include "D3DApp.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "D3DMath.h"
#include "D3DTextr.h"
#include "JStandard.h"

// 0x0040F3B0, 135 bytes.
JObject3DList::JObject3DList()
    : list(new JBaseList())
{
}

// 0x0040F440, 137 bytes.
JObject3DList::~JObject3DList()
{
    Empty();
    if (list != NULL) {
        delete list;
        list = NULL;
    }
}

// 0x0040F4D0, 28 bytes.
void JObject3DList::Append(JObject3DBase* item)
{
    list->Append(item);
}

// 0x0040F4F0, 113 bytes.
void JObject3DList::Empty()
{
    for (unsigned long i = 0; i < Count(); i++) {
        JObject3DBase* item = (JObject3DBase*)list->GetData(i);
        delete item;
    }
    list->Empty();
}

// 0x0040F570, 86 bytes.
void JObject3DList::Remove(unsigned long index) const
{
    JObject3DBase* item = (JObject3DBase*)list->GetData(index);
    delete item;
    list->Remove(index);
}

// 0x0040F5D0, 22 bytes.
unsigned long JObject3DList::Count() const
{
    return list->Count();
}

// 0x0040F5F0, 28 bytes.
JObject3DBase* JObject3DList::GetAt(unsigned long index) const
{
    return (JObject3DBase*)list->GetData(index);
}

// 0x0040F610, 83 bytes.
void JObject3DList::Render()
{
    for (unsigned long i = 0; i < Count(); i++) {
        JObject3DBase* item = (JObject3DBase*)list->GetData(i);
        if (item != NULL) {
            item->Render();
        }
    }
}

// 0x0040F670, 132 bytes.
JObject3DBase::JObject3DBase(CD3DApplication* _app)
    : app(_app), worldPos(0.0f, 0.0f, 0.0f), collisionRadius(0.0f),
      rotatedAngleX(0.0f), rotatedAngleY(0.0f), rotatedAngleZ(0.0f), scaledSize(1.0f)
{
}

// 0x0040F700, 20 bytes.
JObject3DBase::~JObject3DBase()
{
}

// 0x0040F720, 38 bytes.
void JObject3DBase::SetWorldPos(const D3DVECTOR& pos)
{
    worldPos = pos;
}

// 0x0040F750, 68 bytes.
void JObject3DBase::SetWorldPos(JAXIS axis, float value)
{
    if (axis == JAXIS_X) {
        worldPos.x = value;
    } else if (axis == JAXIS_Y) {
        worldPos.y = value;
    } else {
        worldPos.z = value;
    }
}

// 0x0040F7A0, 60 bytes.
void JObject3DBase::ShiftWorldPos(const D3DVECTOR& vector)
{
    worldPos = worldPos + vector;
}

// 0x0040F7E0, 92 bytes.
void JObject3DBase::ShiftWorldPos(JAXIS axis, float amount)
{
    if (axis == JAXIS_X) {
        worldPos.x = worldPos.x + amount;
    } else if (axis == JAXIS_Y) {
        worldPos.y = worldPos.y + amount;
    } else {
        worldPos.z = worldPos.z + amount;
    }
}

// 0x0040F840, 70 bytes.
void JObject3DBase::ShiftWorldPos_OnGround()
{
    D3DVECTOR minVertex;
    D3DVECTOR maxVertex;
    GetBox(minVertex, maxVertex);
    worldPos.y = -minVertex.y;
}

// 0x0040F890, 41 bytes.
D3DVECTOR JObject3DBase::GetWorldPos() const
{
    return worldPos;
}

// 0x0040F8C0, 26 bytes.
void JObject3DBase::SetCollisionRadius(float radius)
{
    collisionRadius = radius;
}

// 0x0040F8E0, 81 bytes.
int JObject3DBase::CheckCollisionRadius(JObject3DBase* obj)
{
    return D3DFindDistance(worldPos, obj->worldPos) < collisionRadius + obj->collisionRadius;
}

// 0x0040F940, 84 bytes.
void JObject3DBase::ScaleTo(float scale)
{
    float inverseScale = 1.0f / scaledSize;
    ScaleBy(inverseScale);
    ScaleBy(scale);
}

// 0x0040F9A0, 155 bytes.
void JObject3DBase::RotateTo(JAXIS axis, float angleDest)
{
    float negateAngle = 0.0f;
    if (axis == JAXIS_X) {
        negateAngle = -rotatedAngleX;
    } else if (axis == JAXIS_Y) {
        negateAngle = -rotatedAngleY;
    } else {
        negateAngle = -rotatedAngleZ;
    }
    RotateBy(axis, negateAngle);
    RotateBy(axis, angleDest);
}

// 0x0040FA40, 11 bytes.
void JObject3DBase::_SetRenderState()
{
}

// 0x0040FA50, 11 bytes.
void JObject3DBase::_ResetRenderState()
{
}

// 0x0040FA60, 109 bytes.
void JObject3DBase::_SetWorldTransform()
{
    D3DMATRIX worldMatrix;
    D3DUtil_SetTranslateMatrix(worldMatrix, worldPos.x, worldPos.y, worldPos.z);
    app->GetD3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &worldMatrix);
}

// 0x0040FAD0, 67 bytes.
void JObject3DBase::_ResetWorldTransform()
{
    D3DMATRIX identityMatrix;
    D3DUtil_SetIdentityMatrix(identityMatrix);
    app->GetD3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &identityMatrix);
}

// 0x0040FB20, 273 bytes.
JObject3DX::JObject3DX(CD3DApplication* _app, unsigned short resId)
    : JObject3DBase(_app), currentFrame(0), flatShade(0), stateShadeMode(0)
{
    for (int i = 0; i < 10; i++) {
        objArray[i] = NULL;
    }
    if (resId != 0xFFFF) {
        objArray[0] = new CD3DFile();
        objArray[0]->LoadFromResource(resId);
    }
}

// 0x0040FC40, 124 bytes.
JObject3DX::~JObject3DX()
{
    for (int i = 0; i < 10; i++) {
        if (objArray[i] != NULL) {
            delete objArray[i];
            objArray[i] = NULL;
        }
    }
}

// 0x0040FCC0, 95 bytes.
void JObject3DX::Shift(const D3DVECTOR& vector)
{
    for (int i = 0; i < 10; i++) {
        if (objArray[i] == NULL) {
            break;
        }
        objArray[i]->Shift(vector);
    }
}

// 0x0040FD20, 103 bytes.
void JObject3DX::ScaleBy(float scale)
{
    scaledSize = scaledSize * scale;
    for (int i = 0; i < 10; i++) {
        if (objArray[i] == NULL) {
            break;
        }
        objArray[i]->Scale(scale);
    }
}

// 0x0040FD90, 298 bytes.
void JObject3DX::RotateBy(JAXIS axis, float angleAmount)
{
    if (axis == JAXIS_X) {
        rotatedAngleX = rotatedAngleX + angleAmount;
        for (int i = 0; i < 10; i++) {
            if (objArray[i] == NULL) {
                break;
            }
            objArray[i]->RotateX(angleAmount);
        }
    } else if (axis == JAXIS_Y) {
        rotatedAngleY = rotatedAngleY + angleAmount;
        for (int i = 0; i < 10; i++) {
            if (objArray[i] == NULL) {
                break;
            }
            objArray[i]->RotateY(angleAmount);
        }
    } else {
        rotatedAngleZ = rotatedAngleZ + angleAmount;
        for (int i = 0; i < 10; i++) {
            if (objArray[i] == NULL) {
                break;
            }
            objArray[i]->RotateZ(angleAmount);
        }
    }
}

// 0x0040FEC0, 191 bytes.
void JObject3DX::GetBox(D3DVECTOR& minVertex, D3DVECTOR& maxVertex)
{
    if (objArray[0] == NULL) {
        minVertex = D3DVECTOR(0.0f, 0.0f, 0.0f);
        maxVertex = D3DVECTOR(0.0f, 0.0f, 0.0f);
    } else {
        objArray[0]->GetBoxMin(minVertex);
        objArray[0]->GetBoxMax(maxVertex);
    }
}

// 0x0040FF80, 99 bytes.
void JObject3DX::Render()
{
    if (objArray[currentFrame] != NULL) {
        _SetWorldTransform();
        _SetRenderState();
        objArray[currentFrame]->Render(app->GetD3DDevice());
        _ResetRenderState();
        _ResetWorldTransform();
    }
}

// 0x0040FFF0, 203 bytes.
void JObject3DX::AppendExtraFrame(unsigned short resId)
{
    int i = 0;
    for (i = 0; i < 10; i++) {
        if (objArray[i] == NULL) {
            break;
        }
    }
    if (i < 10) {
        objArray[i] = new CD3DFile();
        objArray[i]->LoadFromResource(resId);
    }
}

// 0x004100C0, 22 bytes.
void JObject3DX::SetCurrentFrame(int frame)
{
    currentFrame = frame;
}

// 0x004100E0, 22 bytes.
void JObject3DX::SetFlatShade(int _flatShade)
{
    flatShade = _flatShade;
}

// 0x00410100, 115 bytes.
void JObject3DX::_SetRenderState()
{
    app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_SHADEMODE, &stateShadeMode);
    unsigned long dwShadeMode;
    dwShadeMode = flatShade != 0 ? D3DSHADE_FLAT : D3DSHADE_GOURAUD;
    app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SHADEMODE, dwShadeMode);
}

// 0x00410180, 50 bytes.
void JObject3DX::_ResetRenderState()
{
    app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SHADEMODE, stateShadeMode);
}

// 0x004101C0, 568 bytes.
JObject3D::JObject3D(CD3DApplication* _app, unsigned long _vertexCount, unsigned long _indexCount)
    : JObject3DBase(_app), vertexCount(_vertexCount), vertexList(NULL), indexCount(_indexCount), indexList(NULL),
      textureResId(0), stateLighting(0), statePerspective(0), stateAlphaBlend(0), stateSrcBlend(0),
      stateDestBlend(0), stateSpecular(0), stateZWriteEnable(0), stateColorOp(0), stateColorArg1(0),
      stateColorArg2(0), stateAlphaOp(0), stateAlphaArg1(0), stateAlphaArg2(0), stateMinFilter(0),
      stateMagFilter(0), stateMipFilter(0), usePerspective(0), alphaState(ALPHA_NONE), filterState(FILTER_NONE)
{
    vertexList = new D3DLVERTEX[vertexCount];
    memset(vertexList, 0, vertexCount * sizeof(D3DLVERTEX));
    indexList = new unsigned short[indexCount];
    memset(indexList, 0, indexCount * sizeof(unsigned short));
    SetColor(0xff, 0xff, 0xff, 0xff);
}

// 0x00410400, 110 bytes.
JObject3D::~JObject3D()
{
    if (vertexList != NULL) {
        delete[] vertexList;
        vertexList = NULL;
    }
    if (indexList != NULL) {
        delete[] indexList;
        indexList = NULL;
    }
}

// 0x00410470, 168 bytes.
void JObject3D::Shift(const D3DVECTOR& vector)
{
    D3DMATRIX translateMatrix;
    D3DUtil_SetTranslateMatrix(translateMatrix, const_cast<D3DVECTOR&>(vector));
    D3DVERTEX dstVertex;
    D3DVERTEX srcVertex;
    for (unsigned long i = 0; i < vertexCount; i++) {
        SetVertexFromLVertex(srcVertex, vertexList[i]);
        D3DMath_VertexMatrixMultiply(dstVertex, srcVertex, translateMatrix);
        SetLVertexFromVertex(vertexList[i], dstVertex);
    }
}

// 0x00410520, 218 bytes.
void JObject3D::ScaleBy(float scale)
{
    scaledSize = scaledSize * scale;
    D3DMATRIX scaleMatrix;
    D3DUtil_SetScaleMatrix(scaleMatrix, scale, scale, scale);
    D3DVERTEX dstVertex;
    D3DVERTEX srcVertex;
    for (unsigned long i = 0; i < vertexCount; i++) {
        SetVertexFromLVertex(srcVertex, vertexList[i]);
        D3DMath_VertexMatrixMultiply(dstVertex, srcVertex, scaleMatrix);
        SetLVertexFromVertex(vertexList[i], dstVertex);
    }
}

// 0x00410600, 528 bytes.
void JObject3D::RotateBy(JAXIS axis, float angleAmount)
{
    D3DMATRIX rotateMatrix;
    D3DVERTEX dstVertex;
    D3DVERTEX srcVertex;
    if (axis == JAXIS_X) {
        rotatedAngleX = rotatedAngleX + angleAmount;
        D3DUtil_SetRotateXMatrix(rotateMatrix, angleAmount);
        for (unsigned long i = 0; i < vertexCount; i++) {
            SetVertexFromLVertex(srcVertex, vertexList[i]);
            D3DMath_VertexMatrixMultiply(dstVertex, srcVertex, rotateMatrix);
            SetLVertexFromVertex(vertexList[i], dstVertex);
        }
    } else if (axis == JAXIS_Y) {
        rotatedAngleY = rotatedAngleY + angleAmount;
        D3DUtil_SetRotateYMatrix(rotateMatrix, angleAmount);
        for (unsigned long i = 0; i < vertexCount; i++) {
            SetVertexFromLVertex(srcVertex, vertexList[i]);
            D3DMath_VertexMatrixMultiply(dstVertex, srcVertex, rotateMatrix);
            SetLVertexFromVertex(vertexList[i], dstVertex);
        }
    } else {
        rotatedAngleZ = rotatedAngleZ + angleAmount;
        D3DUtil_SetRotateZMatrix(rotateMatrix, angleAmount);
        for (unsigned long i = 0; i < vertexCount; i++) {
            SetVertexFromLVertex(srcVertex, vertexList[i]);
            D3DMath_VertexMatrixMultiply(dstVertex, srcVertex, rotateMatrix);
            SetLVertexFromVertex(vertexList[i], dstVertex);
        }
    }
}

// 0x00410810, 477 bytes.
void JObject3D::GetBox(D3DVECTOR& minVertex, D3DVECTOR& maxVertex)
{
    minVertex = D3DVECTOR(3.4028235e+38f, 3.4028235e+38f, 3.4028235e+38f);
    maxVertex = D3DVECTOR(1.1754944e-38f, 1.1754944e-38f, 1.1754944e-38f);
    for (unsigned long i = 0; i < vertexCount; i++) {
        if (vertexList[i].x < minVertex.x) {
            minVertex.x = vertexList[i].x;
        }
        if (vertexList[i].y < minVertex.y) {
            minVertex.y = vertexList[i].y;
        }
        if (vertexList[i].z < minVertex.z) {
            minVertex.z = vertexList[i].z;
        }
        if (vertexList[i].x > maxVertex.x) {
            maxVertex.x = vertexList[i].x;
        }
        if (vertexList[i].y > maxVertex.y) {
            maxVertex.y = vertexList[i].y;
        }
        if (vertexList[i].z > maxVertex.z) {
            maxVertex.z = vertexList[i].z;
        }
    }
}

// 0x004109F0, 219 bytes.
void JObject3D::Render()
{
    _SetWorldTransform();
    if (textureResId != 0) {
        app->GetD3DDevice()->SetTexture(0, D3DTextr_GetSurface(textureResId));
    }
    _SetRenderState();
    HRESULT hr = app->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertexList, vertexCount, indexList, indexCount, 0);
    app->GetD3DDevice()->SetTexture(0, NULL);
    _ResetRenderState();
    _ResetWorldTransform();
}

// 0x00410AD0, 24 bytes.
void JObject3D::SetTexture(unsigned short resId, float u1, float v1, float u2, float v2)
{
    textureResId = resId;
}

// 0x00410AF0, 93 bytes.
void JObject3D::SetColor(unsigned short r, unsigned short g, unsigned short b, unsigned short alpha)
{
    for (unsigned long i = 0; i < vertexCount; i++) {
        vertexList[i].color = (alpha << 24) | (r << 16) | (g << 8) | b;
    }
}

// 0x00410B50, 99 bytes.
void JObject3D::SetAlpha(unsigned short alpha)
{
    for (unsigned long i = 0; i < vertexCount; i++) {
        unsigned long oldColor = vertexList[i].color;
        vertexList[i].color = (alpha << 24) | (oldColor & 0xFFFFFF);
    }
}

// 0x00410BC0, 59 bytes.
void JObject3D::SetAlphaFactor(float alphaFactor)
{
    unsigned short alpha = (unsigned short)Round(alphaFactor * 255.0f);
    SetAlpha(alpha);
}

// 0x00410C00, 17 bytes.
unsigned long JObject3D::GetVertexCount() const
{
    return vertexCount;
}

// 0x00410C20, 44 bytes.
D3DLVERTEX JObject3D::GetVertexAt(unsigned long vertexNo) const
{
    return vertexList[vertexNo];
}

// 0x00410C50, 41 bytes.
void JObject3D::SetVertexAt(unsigned long vertexNo, D3DLVERTEX vertex)
{
    vertexList[vertexNo] = vertex;
}

// 0x00410C80, 17 bytes.
unsigned long JObject3D::GetIndexCount() const
{
    return indexCount;
}

// 0x00410CA0, 26 bytes.
unsigned short JObject3D::GetIndexAt(unsigned long indexNo) const
{
    return indexList[indexNo];
}

// 0x00410CC0, 30 bytes.
void JObject3D::SetIndexAt(unsigned long indexNo, unsigned short indexValue)
{
    indexList[indexNo] = indexValue;
}

// 0x00410CE0, 22 bytes.
void JObject3D::UsePerspective(int tf)
{
    usePerspective = tf;
}

// 0x00410D00, 25 bytes.
void JObject3D::SetAlphaState(ALPHA_STATE state)
{
    alphaState = state;
}

// 0x00410D20, 25 bytes.
void JObject3D::SetFilterState(FILTER_STATE state)
{
    filterState = state;
}

// 0x00410D40, 1917 bytes.
void JObject3D::_SetRenderState()
{
    app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_LIGHTING, &stateLighting);
    app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, &statePerspective);
    if (alphaState != ALPHA_NONE) {
        app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, &stateAlphaBlend);
        app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_SRCBLEND, &stateSrcBlend);
        app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_DESTBLEND, &stateDestBlend);
        app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_SPECULARENABLE, &stateSpecular);
        app->GetD3DDevice()->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &stateZWriteEnable);
        if (alphaState == ALPHA_TRANSPARENT) {
            app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_COLOROP, &stateColorOp);
            app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_COLORARG1, &stateColorArg1);
            app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_COLORARG2, &stateColorArg2);
            app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_ALPHAOP, &stateAlphaOp);
            app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_ALPHAARG1, &stateAlphaArg1);
            app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_ALPHAARG2, &stateAlphaArg2);
        }
    }
    if (filterState != FILTER_NONE) {
        app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_MINFILTER, &stateMinFilter);
        app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_MAGFILTER, &stateMagFilter);
        app->GetD3DDevice()->GetTextureStageState(0, D3DTSS_MIPFILTER, &stateMipFilter);
    }
    app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_LIGHTING, 0);
    app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, usePerspective);
    if (alphaState == ALPHA_PARTICLE) {
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, 0);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0);
    } else if (alphaState == ALPHA_SHADOW) {
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, 0);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0);
    } else if (alphaState == ALPHA_TRANSPARENT) {
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, 1);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, 0);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    }
    if (filterState == FILTER_POINT) {
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_POINT);
    } else if (filterState == FILTER_LINEAR) {
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
    }
}

// 0x004114C0, 697 bytes.
void JObject3D::_ResetRenderState()
{
    app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_LIGHTING, stateLighting);
    app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, statePerspective);
    if (alphaState != ALPHA_NONE) {
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, stateAlphaBlend);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, stateSrcBlend);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, stateDestBlend);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, stateSpecular);
        app->GetD3DDevice()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, stateZWriteEnable);
        if (alphaState == ALPHA_TRANSPARENT) {
            app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_COLOROP, stateColorOp);
            app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, stateColorArg1);
            app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, stateColorArg2);
            app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, stateAlphaOp);
            app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, stateAlphaArg1);
            app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, stateAlphaArg2);
        }
    }
    if (filterState != FILTER_NONE) {
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MINFILTER, stateMinFilter);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MAGFILTER, stateMagFilter);
        app->GetD3DDevice()->SetTextureStageState(0, D3DTSS_MIPFILTER, stateMipFilter);
    }
}

// 0x00411780, 527 bytes.
JObject3D_Rect::JObject3D_Rect(CD3DApplication* _app, float sizeX, float sizeZ)
    : JObject3D(_app, 4, 6)
{
    float halfX = sizeX / 2.0f;
    float halfZ = sizeZ / 2.0f;
    SetLVertexFromVector(vertexList[0], D3DVECTOR(-halfX, 0.0f, halfZ));
    SetLVertexFromVector(vertexList[1], D3DVECTOR(halfX, 0.0f, halfZ));
    SetLVertexFromVector(vertexList[2], D3DVECTOR(-halfX, 0.0f, -halfZ));
    SetLVertexFromVector(vertexList[3], D3DVECTOR(halfX, 0.0f, -halfZ));
    unsigned short indices[6] = {0, 1, 3, 0, 3, 2};
    for (unsigned long i = 0; i < indexCount; i++) {
        indexList[i] = indices[i];
    }
}

// 0x00411990, 28 bytes.
JObject3D_Rect::~JObject3D_Rect()
{
}

// 0x004119B0, 342 bytes.
void JObject3D_Rect::SetTexture(unsigned short resId, float u1, float v1, float u2, float v2)
{
    JObject3D::SetTexture(resId, u1, v1, u2, v2);
    float u1Eps = u1 + 0.0001f;
    float v1Eps = v1 + 0.0001f;
    float u2Eps = u2 - 0.0001f;
    float v2Eps = v2 - 0.0001f;
    vertexList[0].tu = u1Eps;
    vertexList[0].tv = v1Eps;
    vertexList[1].tu = u2Eps;
    vertexList[1].tv = v1Eps;
    vertexList[2].tu = u1Eps;
    vertexList[2].tv = v2Eps;
    vertexList[3].tu = u2Eps;
    vertexList[3].tv = v2Eps;
}

// 0x00411B10, 225 bytes.
void JObject3D_Rect::Render2(const D3DMATRIX& _viewMatrix)
{
    _SetWorldTransform2(_viewMatrix);
    if (textureResId != 0) {
        app->GetD3DDevice()->SetTexture(0, D3DTextr_GetSurface(textureResId));
    }
    _SetRenderState();
    HRESULT hr = app->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertexList, vertexCount, indexList, indexCount, 0);
    app->GetD3DDevice()->SetTexture(0, NULL);
    _ResetRenderState();
    _ResetWorldTransform();
}

// 0x00411C00, 138 bytes.
void JObject3D_Rect::_SetWorldTransform2(const D3DMATRIX& _viewMatrix)
{
    D3DMATRIX invertedMatrix;
    D3DMATRIX viewMatrixCopy = _viewMatrix;
    D3DMath_MatrixInvert(invertedMatrix, viewMatrixCopy);
    invertedMatrix._41 = worldPos.x;
    invertedMatrix._42 = worldPos.y;
    invertedMatrix._43 = worldPos.z;
    app->GetD3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &invertedMatrix);
}

// 0x00411C90, 145 bytes.
JObject3D_Shadow::JObject3D_Shadow(CD3DApplication* _app, float sizeX, float sizeZ)
    : JObject3D_Rect(_app, sizeX, sizeZ)
{
    UsePerspective(1);
    SetAlphaState(ALPHA_SHADOW);
    SetFilterState(FILTER_LINEAR);
}

// 0x00411D30, 28 bytes.
JObject3D_Shadow::~JObject3D_Shadow()
{
}

// 0x00411D50, 169 bytes.
JObject3D_Particle::JObject3D_Particle(CD3DApplication* _app, float sizeX, float sizeZ)
    : JObject3D_Rect(_app, sizeX, sizeZ)
{
    RotateBy(JAXIS_X, -1.5707964f);
    UsePerspective(0);
    SetAlphaState(ALPHA_PARTICLE);
    SetFilterState(FILTER_LINEAR);
}

// 0x00411E00, 28 bytes.
JObject3D_Particle::~JObject3D_Particle()
{
}

// 0x00411E20, 39 bytes.
void SetLVertexFromVector(D3DLVERTEX& lvertex, const D3DVECTOR& vector)
{
    lvertex.x = vector.x;
    lvertex.y = vector.y;
    lvertex.z = vector.z;
}

// 0x00411E50, 39 bytes.
void SetLVertexFromVertex(D3DLVERTEX& lvertex, const D3DVERTEX& vertex)
{
    lvertex.x = vertex.x;
    lvertex.y = vertex.y;
    lvertex.z = vertex.z;
}

// 0x00411E80, 39 bytes.
void SetVertexFromLVertex(D3DVERTEX& vertex, const D3DLVERTEX& lvertex)
{
    vertex.x = lvertex.x;
    vertex.y = lvertex.y;
    vertex.z = lvertex.z;
}
