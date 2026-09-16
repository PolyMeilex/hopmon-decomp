#include "XMap.h"
#include "D3DApp.h"
#include "D3DUtil.h"
#include "JObject3D.h"
#include "JString.h"
#include "JStringList.h"
#include "XAppMain.h"
#include "XChara.h"
#include "XEnemy.h"
#include "XMapObject.h"
#include "XPac.h"
#include <math.h>
#include <string.h>

// 0x00421280, 199 bytes.
XCamera::XCamera(XApplication* _app)
    : app(_app), state(NON), localTime(0), lookAtPos(0.0f, 0.0f, 0.0f), refPos(0.0f, 0.0f, 0.0f),
      distance(0.0f), verticalAngle(0.0f), horizontalAngle(0.0f), viewDir(0)
{
    D3DUtil_SetIdentityMatrix(viewMatrix);
}

// 0x00421350, 20 bytes.
XCamera::~XCamera()
{
}

// 0x00421370, 235 bytes.
void XCamera::ResetForGame(XPac* pac)
{
    distance = 420.0f;
    verticalAngle = 0.7853982f;
    horizontalAngle = 3.1415927f;
    viewDir = 0;

    D3DVECTOR pacPos = pac->GetWorldPos();
    lookAtPos = D3DVECTOR(pacPos.x, 0.0f, pacPos.z);
    refPos = D3DVECTOR(pacPos.x, 0.0f, pacPos.z);

    UpdateViewMatrix();
    state = WAIT;
    localTime = 0;
}

// 0x00421460, 208 bytes.
void XCamera::ResetForTitle()
{
    distance = 500.0f;
    verticalAngle = 0.0f;
    horizontalAngle = 3.1415927f;
    viewDir = 0;

    lookAtPos = D3DVECTOR(0.0f, 0.0f, 0.0f);
    refPos = D3DVECTOR(0.0f, 0.0f, 0.0f);

    UpdateViewMatrix();
    state = WAIT;
    localTime = 0;
}

// 0x00421530, 59 bytes.
void XCamera::DoSetTransform()
{
    if (state != NON) {
        app->GetD3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &viewMatrix);
    }
}

// 0x00421570, 17 bytes.
XCamera::STATE XCamera::GetState() const
{
    return state;
}

// 0x00421590, 17 bytes.
int XCamera::GetViewDirection() const
{
    return viewDir;
}

// 0x004215B0, 17 bytes.
const D3DMATRIX& XCamera::GetViewMatrix() const
{
    return viewMatrix;
}

// 0x004215D0, 59 bytes.
void XCamera::StartRotatingLeft()
{
    viewDir = (viewDir + 3) % 4;
    state = ROTATE_LEFT;
    localTime = 0;
}

// 0x00421610, 59 bytes.
void XCamera::StartRotatingRight()
{
    viewDir = (viewDir + 1) % 4;
    state = ROTATE_RIGHT;
    localTime = 0;
}

// 0x00421650, 31 bytes.
void XCamera::StartLevelCompleted()
{
    state = LEVEL_COMPLETED;
    localTime = 0;
}

// 0x00421670, 31 bytes.
void XCamera::StartGameOver()
{
    state = GAME_OVER;
    localTime = 0;
}

// 0x00421690, 121 bytes.
void XCamera::IncrVerticalAngle(unsigned long frameCount)
{
    const float angleStep = 0.008726646f;
    verticalAngle += (float)frameCount * angleStep;
    if (verticalAngle > 1.4207964f) {
        verticalAngle = 1.4207964f;
    }
}

// 0x00421710, 125 bytes.
void XCamera::DecrVerticalAngle(unsigned long frameCount)
{
    const float angleStep = 0.008726646f;
    verticalAngle -= (float)frameCount * angleStep;
    if (verticalAngle < 0.2f) {
        verticalAngle = 0.2f;
    }
}

// 0x00421790, 852 bytes.
void XCamera::ProcessFrame(XPac* pac)
{
    localTime++;

    if (state == WAIT) {
        D3DVECTOR pacPos = pac->GetWorldPos();
        lookAtPos = D3DVECTOR(pacPos.x, 0.0f, pacPos.z);
        refPos = D3DVECTOR(pacPos.x, 0.0f, pacPos.z);
        UpdateViewMatrix();
    } else if (state == ROTATE_LEFT || state == ROTATE_RIGHT) {
        const float angleStep = 0.02617994f;
        float delta = angleStep;
        if (state == ROTATE_LEFT) {
            delta = -1.0f * delta;
        }
        horizontalAngle += delta;

        D3DVECTOR pacPos = pac->GetWorldPos();
        lookAtPos = D3DVECTOR(pacPos.x, 0.0f, pacPos.z);
        refPos = D3DVECTOR(pacPos.x, 0.0f, pacPos.z);
        UpdateViewMatrix();

        if (localTime >= 60) {
            state = WAIT;
            localTime = 0;
        }
    } else if (state == LEVEL_COMPLETED) {
        lookAtPos.y += 0.7f;
        refPos.y += 0.7f;
        distance -= 0.5f;
        if (distance < 140.0f) {
            distance = 140.0f;
        }

        const float verticalStep = 0.0012217305f;
        verticalAngle += verticalStep;
        if (verticalAngle > 1.4207964f) {
            verticalAngle = 1.4207964f;
        }

        const float horizontalStep = -0.008726646f;
        horizontalAngle += horizontalStep;
        UpdateViewMatrix();
    } else if (state == GAME_OVER) {
        distance += 0.5f;

        const float verticalStep = 0.0012217305f;
        verticalAngle += verticalStep;
        if (verticalAngle > 1.4207964f) {
            verticalAngle = 1.4207964f;
        }

        const float horizontalStep = -0.008726646f;
        horizontalAngle += horizontalStep;
        UpdateViewMatrix();
    }
}

// 0x00421AF0, 288 bytes.
void XCamera::UpdateViewMatrix()
{
    float heightOffset = sinf(verticalAngle) * distance;
    float radius = cosf(verticalAngle) * distance;

    D3DVECTOR eyePos;
    eyePos.y = refPos.y + heightOffset;
    eyePos.x = sinf(horizontalAngle) * radius + refPos.x;
    eyePos.z = cosf(horizontalAngle) * radius + refPos.z;

    D3DUtil_SetViewMatrix(viewMatrix, eyePos, lookAtPos, D3DVECTOR(0.0f, 1.0f, 0.0f));
}

// 0x00421C10, 225 bytes.
XPreviewMap::XPreviewMap(XApplication* _app, int resId)
    : app(_app), mapArray(NULL), mapObject(NULL), chipSize(10.0f), mapSizeX(0), mapSizeZ(0), mapTextureId(0)
{
    if (app->GetCurrentWorld() == 0) {
        mapTextureId = 0x9c;
    } else if (app->GetCurrentWorld() == 1) {
        mapTextureId = 0xfb;
    } else {
        mapTextureId = 0xfc;
    }
    _LoadMapArray(resId);
    _CreateMapObject();
    mapObject->RotateBy(JAXIS_X, -0.5235988f);
}

// 0x00421D00, 122 bytes.
XPreviewMap::~XPreviewMap()
{
    if (mapArray != NULL) {
        delete[] mapArray;
        mapArray = NULL;
    }
    if (mapObject != NULL) {
        delete mapObject;
        mapObject = NULL;
    }
}

// 0x00421D80, 156 bytes.
void XPreviewMap::ProcessFrame(unsigned long frameCount)
{
    mapObject->RotateBy(JAXIS_X, 0.5235988f);
    mapObject->RotateBy(JAXIS_Y, (float)frameCount * 0.0052359877f);
    mapObject->RotateBy(JAXIS_X, -0.5235988f);
}

// 0x00421E20, 30 bytes.
void XPreviewMap::Render()
{
    mapObject->Render();
}

// 0x00421E40, 635 bytes.
void XPreviewMap::_LoadMapArray(int resId)
{
    JStringList lines;
    {
        JStringList rawLines;
        int discarded = GetStringListFromResource(rawLines, resId);
        for (unsigned long i = 0; i < rawLines.Count(); i++) {
            JString line = rawLines.GetString(i);
            if (line.Length() > 0 && line.c_str()[0] != '_') {
                lines.Append(line);
            } else {
                break;
            }
        }
    }

    mapSizeX = 0;
    for (unsigned long i = 0; i < lines.Count(); i++) {
        if (lines.GetString(i).Length() > (unsigned int)mapSizeX) {
            mapSizeX = lines.GetString(i).Length();
        }
    }
    mapSizeX += 2;
    mapSizeZ = lines.Count() + 2;

    mapArray = new unsigned char[mapSizeZ * mapSizeX];
    memset(mapArray, '.', mapSizeZ * mapSizeX);

    for (int z = 1; z < mapSizeZ - 1; z++) {
        for (int x = 1; x < mapSizeX - 1; x++) {
            int index = z * mapSizeX + x;
            JString line = lines.GetString(z - 1);
            if (x - 1 < line.Length()) {
                mapArray[index] = line.c_str()[x - 1];
            }
        }
    }
}

// 0x004220C0, 2118 bytes.
void XPreviewMap::_CreateMapObject()
{
    float offsetX = ((float)mapSizeX * chipSize) / 2.0f;
    float offsetZ = ((float)mapSizeZ * chipSize) / 2.0f;
    JObject3DList tempObjList;

    for (int z = 0; z < mapSizeZ; z++) {
        for (int x = 0; x < mapSizeX; x++) {
            int index = z * mapSizeX + x;
            float posX = ((float)x * chipSize + chipSize / 2.0f) - offsetX;
            float posZ = ((float)(mapSizeZ - z) * chipSize - chipSize / 2.0f) - offsetZ;
            if (mapArray[index] == '.') {
                continue;
            }
            if (mapArray[index] == '@') {
                mapArray[index] = '-';
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] == '$') {
                mapArray[index] = '-';
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] >= 'A' && mapArray[index] <= 'Z') {
                _CreateFloor(tempObjList, '-', posX, posZ);
            } else if (mapArray[index] >= 'a' && mapArray[index] <= 'z') {
                mapArray[index] = '-';
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] == '-' || mapArray[index] == '+' || mapArray[index] == '!' ||
                       mapArray[index] == '[' || mapArray[index] == ']' || mapArray[index] == '>' ||
                       mapArray[index] == '<' || mapArray[index] == '#' || mapArray[index] == '=' ||
                       mapArray[index] == '*') {
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] >= '1' && mapArray[index] <= '3') {
                float wallHeight = 0.0f;
                if (mapArray[index] == '1') {
                    _CreateWalls(tempObjList, 1, x, z, posX, posZ);
                    wallHeight = chipSize;
                } else if (mapArray[index] == '2') {
                    _CreateWalls(tempObjList, 1, x, z, posX, posZ);
                    _CreateWalls(tempObjList, 2, x, z, posX, posZ);
                    wallHeight = chipSize * 2.0f;
                } else if (mapArray[index] == '3') {
                    _CreateWalls(tempObjList, 1, x, z, posX, posZ);
                    _CreateWalls(tempObjList, 2, x, z, posX, posZ);
                    _CreateWalls(tempObjList, 3, x, z, posX, posZ);
                    wallHeight = chipSize * 3.0f;
                }
                JObject3D_Rect* door = new JObject3D_Rect(app, chipSize, chipSize);
                door->SetTexture(mapTextureId, 0.25f, 0.0f, 0.5f, 0.25f);
                door->Shift(D3DVECTOR(posX, wallHeight, posZ));
                tempObjList.Append(door);
            }
        }
    }

    unsigned long totalVertexCount = 0;
    unsigned long totalIndexCount = 0;
    for (unsigned long objIndex = 0; objIndex < tempObjList.Count(); objIndex++) {
        JObject3D* obj = (JObject3D*)tempObjList.GetAt(objIndex);
        totalVertexCount = obj->GetVertexCount() + totalVertexCount;
        totalIndexCount = obj->GetIndexCount() + totalIndexCount;
    }

    mapObject = new JObject3D(app, totalVertexCount, totalIndexCount);
    mapObject->SetTexture(mapTextureId, 0.0f, 0.0f, 1.0f, 1.0f);

    unsigned long vertexOut = 0;
    unsigned long indexOut = 0;
    for (unsigned long objIndex2 = 0; objIndex2 < tempObjList.Count(); objIndex2++) {
        JObject3D* obj = (JObject3D*)tempObjList.GetAt(objIndex2);
        for (unsigned long indexIn = 0; indexIn < obj->GetIndexCount(); indexIn++) {
            unsigned short index = obj->GetIndexAt(indexIn) + (unsigned short)vertexOut;
            mapObject->SetIndexAt(indexOut, index);
            indexOut++;
        }
        for (unsigned long vertexIn = 0; vertexIn < obj->GetVertexCount(); vertexIn++) {
            D3DLVERTEX vertex = obj->GetVertexAt(vertexIn);
            mapObject->SetVertexAt(vertexOut, vertex);
            vertexOut++;
        }
    }
}

// 0x00422910, 42 bytes.
int XPreviewMap::_ConvertMapItemToInt(unsigned char mapItem)
{
    if (mapItem >= '1' && mapItem <= '3') {
        return mapItem - '0';
    }
    return 0;
}

// 0x00422940, 1007 bytes.
void XPreviewMap::_CreateFloor(JObject3DList& tempObjList, unsigned char type, float posX, float posZ)
{
    JObject3D_Rect* floor = new JObject3D_Rect(app, chipSize, chipSize);
    if (type == '!') {
        floor->SetTexture(mapTextureId, 0.75f, 0.25f, 1.0f, 0.5f);
    } else if (type == '[') {
        floor->SetTexture(mapTextureId, 0.0f, 0.5f, 0.25f, 0.75f);
    } else if (type == '>') {
        floor->SetTexture(mapTextureId, 0.25f, 0.5f, 0.5f, 0.75f);
    } else if (type == ']') {
        floor->SetTexture(mapTextureId, 0.5f, 0.5f, 0.75f, 0.75f);
    } else if (type == '<') {
        floor->SetTexture(mapTextureId, 0.75f, 0.5f, 1.0f, 0.75f);
    } else if (type == '#') {
        floor->SetTexture(mapTextureId, 0.0f, 0.25f, 0.25f, 0.5f);
    } else if (type == '=') {
        floor->SetTexture(mapTextureId, 0.25f, 0.25f, 0.5f, 0.5f);
    } else if (type == '*') {
        floor->SetTexture(mapTextureId, 0.5f, 0.25f, 0.75f, 0.5f);
    } else {
        floor->SetTexture(mapTextureId, 0.0f, 0.0f, 0.25f, 0.25f);
    }
    floor->Shift(D3DVECTOR(posX, 0.0f, posZ));
    tempObjList.Append(floor);
}

// 0x00422D30, 1590 bytes.
void XPreviewMap::_CreateWalls(JObject3DList& tempObjList, int level, int x, int z, float posX, float posZ)
{
    float wallY = (float)(level - 1) * chipSize + chipSize / 2.0f;
    int idxWest = z * mapSizeX - 1 + x;
    int idxEast = z * mapSizeX + 1 + x;
    int idxNorth = (z - 1) * mapSizeX + x;
    int idxSouth = (z + 1) * mapSizeX + x;

    char diffWest = (char)(_ConvertMapItemToInt(mapArray[idxWest]) - level);
    char diffEast = (char)(_ConvertMapItemToInt(mapArray[idxEast]) - level);
    char diffNorth = (char)(_ConvertMapItemToInt(mapArray[idxNorth]) - level);
    char diffSouth = (char)(_ConvertMapItemToInt(mapArray[idxSouth]) - level);

    if (diffWest < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.5f, 0.0f, 0.75f, 0.25f);
        wall->RotateBy(JAXIS_Y, 1.5707964f);
        wall->RotateBy(JAXIS_Z, 1.5707964f);
        wall->Shift(D3DVECTOR(posX - chipSize / 2.0f, wallY, posZ));
        tempObjList.Append(wall);
    }
    if (diffEast < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.75f, 0.0f, 1.0f, 0.25f);
        wall->RotateBy(JAXIS_Y, -1.5707964f);
        wall->RotateBy(JAXIS_Z, -1.5707964f);
        wall->Shift(D3DVECTOR(chipSize / 2.0f + posX, wallY, posZ));
        tempObjList.Append(wall);
    }
    if (diffNorth < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.75f, 0.0f, 1.0f, 0.25f);
        wall->RotateBy(JAXIS_Y, 3.1415927f);
        wall->RotateBy(JAXIS_X, 1.5707964f);
        wall->Shift(D3DVECTOR(posX, wallY, chipSize / 2.0f + posZ));
        tempObjList.Append(wall);
    }
    if (diffSouth < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.5f, 0.0f, 0.75f, 0.25f);
        wall->RotateBy(JAXIS_X, -1.5707964f);
        wall->Shift(D3DVECTOR(posX, wallY, posZ - chipSize / 2.0f));
        tempObjList.Append(wall);
    }
}

// 0x00423370, 244 bytes.
XMap::XMap(XApplication* _app, int resId, XPac* pac, XEnemyList* enemyList, XMapObjectList* mapObjList, XCrystalList* crystalList)
    : app(_app), mapArray(NULL), enemyMapArray(NULL), ufoMapArray(NULL), mapObject(NULL), background(NULL),
      chipSize(40.0f), mapSizeX(0), mapSizeZ(0), mapTextureId(0)
{
    if (app->GetCurrentWorld() == 0) {
        mapTextureId = 0x9c;
    } else if (app->GetCurrentWorld() == 1) {
        mapTextureId = 0xfb;
    } else {
        mapTextureId = 0xfc;
    }
    _LoadMapArray(resId);
    _CreateMapObject(pac, enemyList, mapObjList, crystalList);
    _CreateBackground();
}

// 0x00423470, 262 bytes.
XMap::~XMap()
{
    if (mapArray != NULL) {
        delete[] mapArray;
        mapArray = NULL;
    }
    if (enemyMapArray != NULL) {
        delete[] enemyMapArray;
        enemyMapArray = NULL;
    }
    if (ufoMapArray != NULL) {
        delete[] ufoMapArray;
        ufoMapArray = NULL;
    }
    if (mapObject != NULL) {
        delete mapObject;
        mapObject = NULL;
    }
    if (background != NULL) {
        delete background;
        background = NULL;
    }
}

// 0x00423580, 11 bytes.
void XMap::ProcessFrame()
{
}

// 0x00423590, 49 bytes.
void XMap::Render()
{
    background->Render();
    mapObject->Render();
}

// 0x004235D0, 17 bytes.
const float& XMap::GetChipSize() const
{
    return chipSize;
}

// 0x004235F0, 174 bytes.
int XMap::IsBlocked_Pac(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return 1;
    }
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    unsigned char e = enemyMapArray[mapZ * mapSizeX + mapX];
    if (c == '.' || (c >= 'A' && c <= 'Z') || (c >= '1' && c <= '3')) {
        return 1;
    }
    if (e == 'x' || e == 'y') {
        return 1;
    }
    return 0;
}

// 0x004236A0, 112 bytes.
int XMap::IsBlocked_Fire(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return 1;
    }
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    if ((c >= 'A' && c <= 'Z') || (c >= '1' && c <= '3')) {
        return 1;
    }
    return 0;
}

// 0x00423710, 174 bytes.
int XMap::IsBlocked_Enemy(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return 1;
    }
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    unsigned char e = enemyMapArray[mapZ * mapSizeX + mapX];
    if (c == '.' || c == '+' || (c >= 'A' && c <= 'Z') || (c >= '1' && c <= '3')) {
        return 1;
    }
    if (e != '0') {
        return 1;
    }
    return 0;
}

// 0x004237C0, 144 bytes.
int XMap::IsBlocked_UFO(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return 1;
    }
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    unsigned char u = ufoMapArray[mapZ * mapSizeX + mapX];
    if (c == '.' || c == 'P' || c == '3') {
        return 1;
    }
    if (u != '0') {
        return 1;
    }
    return 0;
}

// 0x00423850, 92 bytes.
int XMap::IsWarpZone(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return 0;
    }
    int result;
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    if (c == '*') {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x004238B0, 119 bytes.
int XMap::IsArrow(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return 0;
    }
    int result;
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    if (c != '[' && c != ']' && c != '<' && c != '>') {
        result = 0;
    } else {
        result = 1;
    }
    return result;
}

// 0x00423930, 135 bytes.
int XMap::GetArrowDirection(int mapX, int mapZ) const
{
    if (_IsOutsideMap(mapX, mapZ)) {
        return -1;
    }
    unsigned char c = mapArray[mapZ * mapSizeX + mapX];
    if (c == '[') {
        return 0;
    } else if (c == '>') {
        return 1;
    } else if (c == ']') {
        return 2;
    } else if (c == '<') {
        return 3;
    }
    return -1;
}

// 0x004239C0, 66 bytes.
int XMap::ConvertPosXToMapX(float posX)
{
    float offset = (float)mapSizeX * chipSize / 2.0f;
    return (int)((posX + offset) / chipSize);
}

// 0x00423A10, 77 bytes.
int XMap::ConvertPosZToMapZ(float posZ)
{
    float offset = (float)mapSizeZ * chipSize / 2.0f;
    return (mapSizeZ - 1) - (int)((posZ + offset) / chipSize);
}

// 0x00423A60, 88 bytes.
void XMap::UpdateEnemyMap(int oldPosX, int oldPosZ, int newPosX, int newPosZ)
{
    unsigned char c = enemyMapArray[oldPosZ * mapSizeX + oldPosX];
    enemyMapArray[oldPosZ * mapSizeX + oldPosX] = '0';
    enemyMapArray[newPosZ * mapSizeX + newPosX] = c;
}

// 0x00423AC0, 36 bytes.
void XMap::ZeroEnemyMap(int mapX, int mapZ)
{
    enemyMapArray[mapZ * mapSizeX + mapX] = '0';
}

// 0x00423AF0, 88 bytes.
void XMap::UpdateUFOMap(int oldPosX, int oldPosZ, int newPosX, int newPosZ)
{
    unsigned char c = ufoMapArray[oldPosZ * mapSizeX + oldPosX];
    ufoMapArray[oldPosZ * mapSizeX + oldPosX] = '0';
    ufoMapArray[newPosZ * mapSizeX + newPosX] = c;
}

// 0x00424AF0, 42 bytes.
int XMap::_ConvertMapItemToInt(unsigned char mapItem)
{
    if (mapItem >= '1' && mapItem <= '3') {
        return mapItem - '0';
    }
    return 0;
}

// 0x00425550, 63 bytes.
int XMap::_IsOutsideMap(int mapX, int mapZ) const
{
    if (mapX < 0 || mapX >= mapSizeX) {
        return 1;
    }
    if (mapZ < 0 || mapZ >= mapSizeZ) {
        return 1;
    }
    return 0;
}

// 0x00423B50, 765 bytes.
void XMap::_LoadMapArray(int resId)
{
    JStringList lines;
    {
        JStringList rawLines;
        int discarded = GetStringListFromResource(rawLines, resId);
        for (unsigned long i = 0; i < rawLines.Count(); i++) {
            JString line = rawLines.GetString(i);
            if (line.Length() > 0 && line.c_str()[0] != '_') {
                lines.Append(line);
            } else {
                break;
            }
        }
    }

    mapSizeX = 0;
    for (unsigned long i = 0; i < lines.Count(); i++) {
        if (lines.GetString(i).Length() > (unsigned int)mapSizeX) {
            mapSizeX = lines.GetString(i).Length();
        }
    }
    mapSizeX += 2;
    mapSizeZ = lines.Count() + 2;

    mapArray = new unsigned char[mapSizeZ * mapSizeX];
    memset(mapArray, '.', mapSizeZ * mapSizeX);
    enemyMapArray = new unsigned char[mapSizeZ * mapSizeX];
    memset(enemyMapArray, '0', mapSizeZ * mapSizeX);
    ufoMapArray = new unsigned char[mapSizeZ * mapSizeX];
    memset(ufoMapArray, '0', mapSizeZ * mapSizeX);

    for (int z = 1; z < mapSizeZ - 1; z++) {
        for (int x = 1; x < mapSizeX - 1; x++) {
            int index = z * mapSizeX + x;
            JString line = lines.GetString(z - 1);
            if (x - 1 < line.Length()) {
                mapArray[index] = line.c_str()[x - 1];
            }
        }
    }
}

// 0x00423E50, 2842 bytes.
void XMap::_CreateMapObject(XPac* pac, XEnemyList* enemyList, XMapObjectList* mapObjList, XCrystalList* crystalList)
{
    float offsetX = ((float)mapSizeX * chipSize) / 2.0f;
    float offsetZ = ((float)mapSizeZ * chipSize) / 2.0f;
    JObject3DList tempObjList;

    for (int z = 0; z < mapSizeZ; z++) {
        for (int x = 0; x < mapSizeX; x++) {
            int index = z * mapSizeX + x;
            float posX = ((float)x * chipSize + chipSize / 2.0f) - offsetX;
            float posZ = ((float)(mapSizeZ - z) * chipSize - chipSize / 2.0f) - offsetZ;
            if (mapArray[index] == '.') {
                continue;
            }
            if (mapArray[index] == '@') {
                pac->SetWorldPos(JAXIS_X, posX);
                pac->SetWorldPos(JAXIS_Z, posZ);
                pac->SetMapPos(x, z);
                mapArray[index] = '-';
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] == '$') {
                XCrystal* crystal = crystalList->CreateCrystal();
                crystal->SetWorldPos(JAXIS_X, posX);
                crystal->SetWorldPos(JAXIS_Z, posZ);
                mapArray[index] = '-';
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] >= 'A' && mapArray[index] <= 'Z') {
                XMapObjectBase* obj = mapObjList->CreateMapObject(mapArray[index]);
                if (obj != NULL) {
                    obj->SetWorldPos(JAXIS_X, posX);
                    obj->SetWorldPos(JAXIS_Z, posZ);
                }
                _CreateFloor(tempObjList, '-', posX, posZ);
                if (mapArray[index] == 'V') {
                    mapArray[(z - 1) * mapSizeX - 1 + x] = 'Z';
                    mapArray[(z - 1) * mapSizeX + x] = 'Z';
                    mapArray[(z - 1) * mapSizeX + 1 + x] = 'Z';
                    mapArray[z * mapSizeX - 1 + x] = 'Z';
                    mapArray[z * mapSizeX + 1 + x] = 'Z';
                    mapArray[(z + 1) * mapSizeX - 1 + x] = 'Z';
                    mapArray[(z + 1) * mapSizeX + x] = 'Z';
                    mapArray[(z + 1) * mapSizeX + 1 + x] = 'Z';
                }
            } else if (mapArray[index] >= 'a' && mapArray[index] <= 'z') {
                XEnemyBase* enemy = enemyList->CreateEnemy(mapArray[index]);
                enemy->SetWorldPos(JAXIS_X, posX);
                enemy->SetWorldPos(JAXIS_Z, posZ);
                enemy->SetMapPos(x, z);
                if (mapArray[index] == 'd') {
                    ufoMapArray[index] = mapArray[index];
                } else {
                    enemyMapArray[index] = mapArray[index];
                }
                mapArray[index] = '-';
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
            } else if (mapArray[index] == '-' || mapArray[index] == '+' || mapArray[index] == '!' ||
                       mapArray[index] == '[' || mapArray[index] == ']' || mapArray[index] == '>' ||
                       mapArray[index] == '<' || mapArray[index] == '#' || mapArray[index] == '=' ||
                       mapArray[index] == '*') {
                _CreateFloor(tempObjList, mapArray[index], posX, posZ);
                if (mapArray[index] == '!') {
                    XMapObjectBase* obj = mapObjList->CreateMapObject(mapArray[index]);
                    if (obj != NULL) {
                        obj->SetWorldPos(JAXIS_X, posX);
                        obj->SetWorldPos(JAXIS_Z, posZ);
                    }
                }
            } else if (mapArray[index] >= '1' && mapArray[index] <= '3') {
                float wallHeight = 0.0f;
                if (mapArray[index] == '1') {
                    _CreateWalls(tempObjList, 1, x, z, posX, posZ);
                    wallHeight = chipSize;
                } else if (mapArray[index] == '2') {
                    _CreateWalls(tempObjList, 1, x, z, posX, posZ);
                    _CreateWalls(tempObjList, 2, x, z, posX, posZ);
                    wallHeight = chipSize * 2.0f;
                } else if (mapArray[index] == '3') {
                    _CreateWalls(tempObjList, 1, x, z, posX, posZ);
                    _CreateWalls(tempObjList, 2, x, z, posX, posZ);
                    _CreateWalls(tempObjList, 3, x, z, posX, posZ);
                    wallHeight = chipSize * 3.0f;
                }
                JObject3D_Rect* door = new JObject3D_Rect(app, chipSize, chipSize);
                door->SetTexture(mapTextureId, 0.25f, 0.0f, 0.5f, 0.25f);
                door->Shift(D3DVECTOR(posX, wallHeight, posZ));
                tempObjList.Append(door);
            }
        }
    }

    unsigned long totalVertexCount = 0;
    unsigned long totalIndexCount = 0;
    for (unsigned long objIndex = 0; objIndex < tempObjList.Count(); objIndex++) {
        JObject3D* obj = (JObject3D*)tempObjList.GetAt(objIndex);
        totalVertexCount = obj->GetVertexCount() + totalVertexCount;
        totalIndexCount = obj->GetIndexCount() + totalIndexCount;
    }

    mapObject = new JObject3D(app, totalVertexCount, totalIndexCount);
    mapObject->SetTexture(mapTextureId, 0.0f, 0.0f, 1.0f, 1.0f);

    unsigned long vertexOut = 0;
    unsigned long indexOut = 0;
    for (unsigned long objIndex2 = 0; objIndex2 < tempObjList.Count(); objIndex2++) {
        JObject3D* obj = (JObject3D*)tempObjList.GetAt(objIndex2);
        for (unsigned long indexIn = 0; indexIn < obj->GetIndexCount(); indexIn++) {
            unsigned short index = obj->GetIndexAt(indexIn) + (unsigned short)vertexOut;
            mapObject->SetIndexAt(indexOut, index);
            indexOut++;
        }
        for (unsigned long vertexIn = 0; vertexIn < obj->GetVertexCount(); vertexIn++) {
            D3DLVERTEX vertex = obj->GetVertexAt(vertexIn);
            mapObject->SetVertexAt(vertexOut, vertex);
            vertexOut++;
        }
    }
}

// 0x00424970, 383 bytes.
void XMap::_CreateBackground()
{
    D3DVECTOR minVertex;
    D3DVECTOR maxVertex;
    mapObject->GetBox(minVertex, maxVertex);

    float width = (float)(mapSizeX + 4) * chipSize;
    float depth = (float)(mapSizeZ + 4) * chipSize;
    background = new JObject3D_Rect(app, width, depth);
    background->ShiftWorldPos(JAXIS_Y, -100.0f);
    background->SetTexture(0xc1, 0.0f, 0.0f, (float)(mapSizeX + 4), (float)(mapSizeZ + 4));
    background->UsePerspective(1);
    background->SetFilterState(JObject3D::FILTER_LINEAR);
}

// 0x00424B20, 1007 bytes.
void XMap::_CreateFloor(JObject3DList& tempObjList, unsigned char type, float posX, float posZ)
{
    JObject3D_Rect* floor = new JObject3D_Rect(app, chipSize, chipSize);
    if (type == '!') {
        floor->SetTexture(mapTextureId, 0.75f, 0.25f, 1.0f, 0.5f);
    } else if (type == '[') {
        floor->SetTexture(mapTextureId, 0.0f, 0.5f, 0.25f, 0.75f);
    } else if (type == '>') {
        floor->SetTexture(mapTextureId, 0.25f, 0.5f, 0.5f, 0.75f);
    } else if (type == ']') {
        floor->SetTexture(mapTextureId, 0.5f, 0.5f, 0.75f, 0.75f);
    } else if (type == '<') {
        floor->SetTexture(mapTextureId, 0.75f, 0.5f, 1.0f, 0.75f);
    } else if (type == '#') {
        floor->SetTexture(mapTextureId, 0.0f, 0.25f, 0.25f, 0.5f);
    } else if (type == '=') {
        floor->SetTexture(mapTextureId, 0.25f, 0.25f, 0.5f, 0.5f);
    } else if (type == '*') {
        floor->SetTexture(mapTextureId, 0.5f, 0.25f, 0.75f, 0.5f);
    } else {
        floor->SetTexture(mapTextureId, 0.0f, 0.0f, 0.25f, 0.25f);
    }
    floor->Shift(D3DVECTOR(posX, 0.0f, posZ));
    tempObjList.Append(floor);
}

// 0x00424F10, 1590 bytes.
void XMap::_CreateWalls(JObject3DList& tempObjList, int level, int x, int z, float posX, float posZ)
{
    float wallY = (float)(level - 1) * chipSize + chipSize / 2.0f;
    int idxWest = z * mapSizeX - 1 + x;
    int idxEast = z * mapSizeX + 1 + x;
    int idxNorth = (z - 1) * mapSizeX + x;
    int idxSouth = (z + 1) * mapSizeX + x;

    char diffWest = (char)(_ConvertMapItemToInt(mapArray[idxWest]) - level);
    char diffEast = (char)(_ConvertMapItemToInt(mapArray[idxEast]) - level);
    char diffNorth = (char)(_ConvertMapItemToInt(mapArray[idxNorth]) - level);
    char diffSouth = (char)(_ConvertMapItemToInt(mapArray[idxSouth]) - level);

    if (diffWest < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.5f, 0.0f, 0.75f, 0.25f);
        wall->RotateBy(JAXIS_Y, 1.5707964f);
        wall->RotateBy(JAXIS_Z, 1.5707964f);
        wall->Shift(D3DVECTOR(posX - chipSize / 2.0f, wallY, posZ));
        tempObjList.Append(wall);
    }
    if (diffEast < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.75f, 0.0f, 1.0f, 0.25f);
        wall->RotateBy(JAXIS_Y, -1.5707964f);
        wall->RotateBy(JAXIS_Z, -1.5707964f);
        wall->Shift(D3DVECTOR(chipSize / 2.0f + posX, wallY, posZ));
        tempObjList.Append(wall);
    }
    if (diffNorth < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.75f, 0.0f, 1.0f, 0.25f);
        wall->RotateBy(JAXIS_Y, 3.1415927f);
        wall->RotateBy(JAXIS_X, 1.5707964f);
        wall->Shift(D3DVECTOR(posX, wallY, chipSize / 2.0f + posZ));
        tempObjList.Append(wall);
    }
    if (diffSouth < 0) {
        JObject3D_Rect* wall = new JObject3D_Rect(app, chipSize, chipSize);
        wall->SetTexture(mapTextureId, 0.5f, 0.0f, 0.75f, 0.25f);
        wall->RotateBy(JAXIS_X, -1.5707964f);
        wall->Shift(D3DVECTOR(posX, wallY, posZ - chipSize / 2.0f));
        tempObjList.Append(wall);
    }
}

// 0x00420EE0, 923 bytes.
unsigned short GetMapResId(XApplication* app)
{
    int world = app->GetCurrentWorld();
    int level = app->currentLevel % 15;
    if (world == 0) {
        if (level == 0) {
            return 0x11c;
        } else if (level == 1) {
            return 0x115;
        } else if (level == 2) {
            return 0x116;
        } else if (level == 3) {
            return 0x117;
        } else if (level == 4) {
            return 0x118;
        } else if (level == 5) {
            return 0x119;
        } else if (level == 6) {
            return 0x11a;
        } else if (level == 7) {
            return 0x11b;
        } else if (level == 8) {
            return 0x114;
        } else if (level == 9) {
            return 0x109;
        } else if (level == 10) {
            return 0x10a;
        } else if (level == 11) {
            return 0x10b;
        } else if (level == 12) {
            return 0x10c;
        } else if (level == 13) {
            return 0x10d;
        } else if (level == 14) {
            return 0x10e;
        }
    } else if (world == 1) {
        if (level == 0) {
            return 0x11d;
        } else if (level == 1) {
            return 0x11e;
        } else if (level == 2) {
            return 0x11f;
        } else if (level == 3) {
            return 0x120;
        } else if (level == 4) {
            return 0x121;
        } else if (level == 5) {
            return 0x122;
        } else if (level == 6) {
            return 0x123;
        } else if (level == 7) {
            return 0x124;
        } else if (level == 8) {
            return 0x125;
        } else if (level == 9) {
            return 0x126;
        } else if (level == 10) {
            return 0x127;
        } else if (level == 11) {
            return 0x128;
        } else if (level == 12) {
            return 0x129;
        } else if (level == 13) {
            return 0x12a;
        } else if (level == 14) {
            return 299;
        }
    } else if (world == 2) {
        if (level == 0) {
            return 300;
        } else if (level == 1) {
            return 0x12d;
        } else if (level == 2) {
            return 0x12e;
        } else if (level == 3) {
            return 0x12f;
        } else if (level == 4) {
            return 0x130;
        } else if (level == 5) {
            return 0x131;
        } else if (level == 6) {
            return 0x132;
        } else if (level == 7) {
            return 0x133;
        } else if (level == 8) {
            return 0x134;
        } else if (level == 9) {
            return 0x135;
        } else if (level == 10) {
            return 0x136;
        } else if (level == 11) {
            return 0x137;
        } else if (level == 12) {
            return 0x138;
        } else if (level == 13) {
            return 0x139;
        } else if (level == 14) {
            return 0x13a;
        }
    }
    return 0x11c;
}
