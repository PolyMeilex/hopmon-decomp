#pragma once
#include "common.h"
#include <d3d.h>

class CD3DApplication;
class CD3DFile;
class JBaseList;
class JObject3DBase;

enum JAXIS {
    JAXIS_X = 0,
    JAXIS_Y = 1,
    JAXIS_Z = 2,
};

// PDB type 0x1661
class JObject3DList {
    friend struct LayoutCheck;
public:
    JObject3DList();
    virtual ~JObject3DList(); // vtable 0x00
    void Append(JObject3DBase* item);
    void Empty();
    void Remove(unsigned long index) const;
    unsigned long Count() const;
    JObject3DBase* GetAt(unsigned long index) const;
    void Render();
private:
    JBaseList* list; // 0x04
};
ASSERT_SIZE(JObject3DList, 0x8);

// PDB type 0x145F
class JObject3DBase {
    friend struct LayoutCheck;
public:
    JObject3DBase(CD3DApplication* _app);
    virtual ~JObject3DBase(); // vtable 0x00
    void SetWorldPos(JAXIS axis, float value);
    void SetWorldPos(const D3DVECTOR& pos);
    void ShiftWorldPos(JAXIS axis, float amount);
    void ShiftWorldPos(const D3DVECTOR& vector);
    void ShiftWorldPos_OnGround();
    D3DVECTOR GetWorldPos() const;
    void SetCollisionRadius(float radius);
    int CheckCollisionRadius(JObject3DBase* obj);
    void ScaleTo(float scale);
    void RotateTo(JAXIS axis, float angleDest);
    virtual void Shift(const D3DVECTOR& vector) = 0; // vtable 0x04
    virtual void ScaleBy(float scale) = 0; // vtable 0x08
    virtual void RotateBy(JAXIS axis, float angleAmount) = 0; // vtable 0x0C
    virtual void GetBox(D3DVECTOR& minVertex, D3DVECTOR& maxVertex) = 0; // vtable 0x10
    virtual void Render() = 0; // vtable 0x14
protected:
    virtual void _SetRenderState(); // vtable 0x18
    virtual void _ResetRenderState(); // vtable 0x1C
    void _SetWorldTransform();
    void _ResetWorldTransform();
    CD3DApplication* app; // 0x04
    D3DVECTOR worldPos; // 0x08
    float collisionRadius; // 0x14
    float rotatedAngleX; // 0x18
    float rotatedAngleY; // 0x1C
    float rotatedAngleZ; // 0x20
    float scaledSize; // 0x24
};
ASSERT_SIZE(JObject3DBase, 0x28);

// PDB type 0x1730
class JObject3DX : public JObject3DBase {
    friend struct LayoutCheck;
public:
    JObject3DX(CD3DApplication* _app, unsigned short resId);
    virtual ~JObject3DX();
    virtual void Shift(const D3DVECTOR& vector);
    virtual void ScaleBy(float scale);
    virtual void RotateBy(JAXIS axis, float angleAmount);
    virtual void GetBox(D3DVECTOR& minVertex, D3DVECTOR& maxVertex);
    virtual void Render();
    void AppendExtraFrame(unsigned short resId);
    void SetCurrentFrame(int frame);
    void SetFlatShade(int _flatShade);
protected:
    virtual void _SetRenderState();
    virtual void _ResetRenderState();
    int currentFrame; // 0x28
    CD3DFile* objArray[10]; // 0x2C
private:
    int flatShade; // 0x54
    unsigned long stateShadeMode; // 0x58
};
ASSERT_SIZE(JObject3DX, 0x5C);

// PDB type 0x15C4
class JObject3D : public JObject3DBase {
    friend struct LayoutCheck;
public:
    enum FILTER_STATE {
        FILTER_NONE = 0,
        FILTER_POINT = 1,
        FILTER_LINEAR = 2,
    };

    enum ALPHA_STATE {
        ALPHA_NONE = 0,
        ALPHA_PARTICLE = 1,
        ALPHA_SHADOW = 2,
        ALPHA_TRANSPARENT = 3,
    };

    JObject3D(CD3DApplication* _app, unsigned long _vertexCount, unsigned long _indexCount);
    virtual ~JObject3D();
    virtual void Shift(const D3DVECTOR& vector);
    virtual void ScaleBy(float scale);
    virtual void RotateBy(JAXIS axis, float angleAmount);
    virtual void GetBox(D3DVECTOR& minVertex, D3DVECTOR& maxVertex);
    virtual void Render();
    // Texture resource `resId`, mapped over [u1,u2] x [v1,v2].
    virtual void SetTexture(unsigned short resId, float u1, float v1, float u2, float v2); // vtable 0x20
    void SetColor(unsigned short r, unsigned short g, unsigned short b, unsigned short alpha);
    void SetAlpha(unsigned short alpha);
    void SetAlphaFactor(float alphaFactor);
    unsigned long GetVertexCount() const;
    D3DLVERTEX GetVertexAt(unsigned long vertexNo) const;
    void SetVertexAt(unsigned long vertexNo, D3DLVERTEX vertex);
    unsigned long GetIndexCount() const;
    unsigned short GetIndexAt(unsigned long indexNo) const;
    void SetIndexAt(unsigned long indexNo, unsigned short indexValue);
    void UsePerspective(int tf);
    void SetAlphaState(ALPHA_STATE state);
    void SetFilterState(FILTER_STATE state);
protected:
    virtual void _SetRenderState();
    virtual void _ResetRenderState();
    unsigned long vertexCount; // 0x28
    D3DLVERTEX* vertexList; // 0x2C
    unsigned long indexCount; // 0x30
    unsigned short* indexList; // 0x34
    unsigned short textureResId; // 0x38
private:
    unsigned long stateLighting; // 0x3C
    unsigned long statePerspective; // 0x40
    unsigned long stateAlphaBlend; // 0x44
    unsigned long stateSrcBlend; // 0x48
    unsigned long stateDestBlend; // 0x4C
    unsigned long stateSpecular; // 0x50
    unsigned long stateZWriteEnable; // 0x54
    unsigned long stateColorOp; // 0x58
    unsigned long stateColorArg1; // 0x5C
    unsigned long stateColorArg2; // 0x60
    unsigned long stateAlphaOp; // 0x64
    unsigned long stateAlphaArg1; // 0x68
    unsigned long stateAlphaArg2; // 0x6C
    unsigned long stateMinFilter; // 0x70
    unsigned long stateMagFilter; // 0x74
    unsigned long stateMipFilter; // 0x78
    int usePerspective; // 0x7C
    ALPHA_STATE alphaState; // 0x80
    FILTER_STATE filterState; // 0x84
};
ASSERT_SIZE(JObject3D, 0x88);

// PDB type 0x1557. A textured quad.
class JObject3D_Rect : public JObject3D {
    friend struct LayoutCheck;
public:
    JObject3D_Rect(CD3DApplication* _app, float sizeX, float sizeZ);
    virtual ~JObject3D_Rect();
    virtual void SetTexture(unsigned short resId, float u1, float v1, float u2, float v2);
    virtual void Render2(const D3DMATRIX& _viewMatrix); // vtable 0x24
protected:
    void _SetWorldTransform2(const D3DMATRIX& _viewMatrix);
};
ASSERT_SIZE(JObject3D_Rect, 0x88);

// PDB type 0x18E2
class JObject3D_Shadow : public JObject3D_Rect {
    friend struct LayoutCheck;
public:
    JObject3D_Shadow(CD3DApplication* _app, float sizeX, float sizeZ);
    virtual ~JObject3D_Shadow();
};
ASSERT_SIZE(JObject3D_Shadow, 0x88);

// PDB type 0x17F2
class JObject3D_Particle : public JObject3D_Rect {
    friend struct LayoutCheck;
public:
    JObject3D_Particle(CD3DApplication* _app, float sizeX, float sizeZ);
    virtual ~JObject3D_Particle();
};
ASSERT_SIZE(JObject3D_Particle, 0x88);

void SetLVertexFromVector(D3DLVERTEX& lvertex, const D3DVECTOR& vector);
void SetLVertexFromVertex(D3DLVERTEX& lvertex, const D3DVERTEX& vertex);
void SetVertexFromLVertex(D3DVERTEX& vertex, const D3DLVERTEX& lvertex);
