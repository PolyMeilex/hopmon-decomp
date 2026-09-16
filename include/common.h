// Shared by all decompiled sources.
#pragma once

// Hopmon's D3DApp.h defines this before <d3d.h>: D3DVECTOR and D3DMATRIX get
// constructors and operators. Defined here so include order doesn't matter.
#define D3D_OVERLOADS
#include <windows.h>
#include <stddef.h>

// Class layouts must match the original exactly while original and
// decompiled code share objects. Values come from the PDB (see README).
#define ASSERT_SIZE(type, size) \
    static_assert(sizeof(type) == (size), "sizeof(" #type ") != " #size)

// Private members can be checked from a `friend struct LayoutCheck;`.
#define ASSERT_OFFSET(type, member, offset) \
    static_assert(offsetof(type, member) == (offset), #type "::" #member " offset != " #offset)
