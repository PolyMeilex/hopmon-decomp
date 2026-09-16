#pragma once
#include "common.h"
#include <d3d.h>

void Randomize();
long Random(long value);
long Round(float value);
D3DVECTOR D3DVectorMultiply(const D3DVECTOR& vector, int value);
D3DVECTOR D3DVectorMultiply(const D3DVECTOR& vector, float value);
float D3DFindDistance(const D3DVECTOR& point1, const D3DVECTOR& point2);
