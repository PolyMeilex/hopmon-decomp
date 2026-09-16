#include "JStandard.h"
#include <math.h>
#include <stdlib.h>

// 0x004136A0, 20 bytes.
void Randomize()
{
    srand(GetTickCount());
}

// 0x004136C0, 26 bytes.
long Random(long value)
{
    if (value <= 0)
        return 0;
    return rand() % value;
}

// 0x004136E0, 57 bytes.
long Round(float value)
{
    if (value >= 0.0f)
        return (long)(value + 0.5f);
    else
        return (long)(value - 0.5f);
}

// 0x00413720, 94 bytes.
D3DVECTOR D3DVectorMultiply(const D3DVECTOR& vector, int value)
{
    D3DVECTOR result;
    result.x = (float)value * vector.x;
    result.y = (float)value * vector.y;
    result.z = (float)value * vector.z;
    return result;
}

// 0x00413780, 94 bytes.
D3DVECTOR D3DVectorMultiply(const D3DVECTOR& vector, float value)
{
    D3DVECTOR result;
    result.x = vector.x * value;
    result.y = vector.y * value;
    result.z = vector.z * value;
    return result;
}

// 0x004137E0, 159 bytes.
float D3DFindDistance(const D3DVECTOR& point1, const D3DVECTOR& point2)
{
    float dx = fabsf(point1.x - point2.x);
    float dy = fabsf(point1.y - point2.y);
    float dz = fabsf(point1.z - point2.z);
    return sqrtf(dx * dx + dy * dy + dz * dz);
}
