#pragma once
#include "common.h"
#include <d3d.h>

class JObject3D;
class JObject3DList;
class JObject3D_Rect;
class XApplication;
class XCrystalList;
class XEnemyList;
class XMapObjectList;
class XPac;

// PDB type 0x1399
class XCamera {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        WAIT = 1,
        ROTATE_LEFT = 2,
        ROTATE_RIGHT = 3,
        LEVEL_COMPLETED = 4,
        GAME_OVER = 5,
    };

    XCamera(XApplication* _app);
    virtual ~XCamera(); // vtable 0x00
    void ResetForGame(XPac* pac);
    void ResetForTitle();
    void DoSetTransform();
    STATE GetState() const;
    int GetViewDirection() const;
    const D3DMATRIX& GetViewMatrix() const;
    void StartRotatingLeft();
    void StartRotatingRight();
    void StartLevelCompleted();
    void StartGameOver();
    void IncrVerticalAngle(unsigned long frameCount);
    void DecrVerticalAngle(unsigned long frameCount);
    void ProcessFrame(XPac* pac);
protected:
    void UpdateViewMatrix();
private:
    XApplication* app; // 0x04
    STATE state; // 0x08
    unsigned int localTime; // 0x0C
    D3DVECTOR lookAtPos; // 0x10
    D3DVECTOR refPos; // 0x1C
    float distance; // 0x28
    float verticalAngle; // 0x2C
    float horizontalAngle; // 0x30
    int viewDir; // 0x34
    D3DMATRIX viewMatrix; // 0x38
};
ASSERT_SIZE(XCamera, 0x78);

// PDB type 0x1549
class XPreviewMap {
    friend struct LayoutCheck;
public:
    XPreviewMap(XApplication* _app, int resId);
    virtual ~XPreviewMap(); // vtable 0x00
    void ProcessFrame(unsigned long frameCount);
    void Render();
protected:
    void _LoadMapArray(int resId);
    void _CreateMapObject();
    int _ConvertMapItemToInt(unsigned char mapItem);
    void _CreateFloor(JObject3DList& tempObjList, unsigned char type, float posX, float posZ);
    void _CreateWalls(JObject3DList& tempObjList, int level, int x, int z, float posX, float posZ);
private:
    XApplication* app; // 0x04
    unsigned char* mapArray; // 0x08
    JObject3D* mapObject; // 0x0C
    float chipSize; // 0x10
    int mapSizeX; // 0x14
    int mapSizeZ; // 0x18
    unsigned short mapTextureId; // 0x1C
};
ASSERT_SIZE(XPreviewMap, 0x20);

// PDB type 0x18A1
class XMap {
    friend struct LayoutCheck;
public:
    XMap(XApplication* _app, int resId, XPac* pac, XEnemyList* enemyList, XMapObjectList* mapObjList, XCrystalList* crystalList);
    virtual ~XMap(); // vtable 0x00
    void ProcessFrame();
    void Render();
    const float& GetChipSize() const;
    int IsBlocked_Pac(int mapX, int mapZ) const;
    int IsBlocked_Fire(int mapX, int mapZ) const;
    int IsBlocked_Enemy(int mapX, int mapZ) const;
    int IsBlocked_UFO(int mapX, int mapZ) const;
    int IsWarpZone(int mapX, int mapZ) const;
    int IsArrow(int mapX, int mapZ) const;
    int GetArrowDirection(int mapX, int mapZ) const;
    int ConvertPosXToMapX(float posX);
    int ConvertPosZToMapZ(float posZ);
    void UpdateEnemyMap(int oldPosX, int oldPosZ, int newPosX, int newPosZ);
    void ZeroEnemyMap(int mapX, int mapZ);
    void UpdateUFOMap(int oldPosX, int oldPosZ, int newPosX, int newPosZ);
protected:
    void _LoadMapArray(int resId);
    void _CreateMapObject(XPac* pac, XEnemyList* enemyList, XMapObjectList* mapObjList, XCrystalList* crystalList);
    void _CreateBackground();
    int _ConvertMapItemToInt(unsigned char mapItem);
    void _CreateFloor(JObject3DList& tempObjList, unsigned char type, float posX, float posZ);
    void _CreateWalls(JObject3DList& tempObjList, int level, int x, int z, float posX, float posZ);
    int _IsOutsideMap(int mapX, int mapZ) const;
private:
    XApplication* app; // 0x04
    unsigned char* mapArray; // 0x08
    unsigned char* enemyMapArray; // 0x0C
    unsigned char* ufoMapArray; // 0x10
    JObject3D* mapObject; // 0x14
    JObject3D_Rect* background; // 0x18
    float chipSize; // 0x1C
    int mapSizeX; // 0x20
    int mapSizeZ; // 0x24
    unsigned short mapTextureId; // 0x28
};
ASSERT_SIZE(XMap, 0x2C);

unsigned short GetMapResId(XApplication* app);
