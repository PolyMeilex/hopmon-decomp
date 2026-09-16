# cmake -B build -G Ninja --toolchain cmake/msvc-wine.cmake
# cmake --build build

cmake_minimum_required(VERSION 3.29)
project(Hopmon LANGUAGES CXX RC)

if(NOT MSVC)
  message(FATAL_ERROR "Hopmon needs MSVC, VS 16.11.5")
endif()

option(HOPMON_FHD "Enable widescreen/FHD resolution support patch" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/game")

# We only support the config used by Hopmon.exe
set(CMAKE_BUILD_TYPE "")

# Compile flags extracted from Hopmon.pdb
set(CMAKE_CXX_FLAGS "\
/Z7 /W3 /WX- /Od /Oy- /DWIN32 /DNDEBUG /D_WINDOWS /D_CRT_SECURE_NO_DEPRECATE /D_MBCS \
/GF- /Gm- /EHsc /MT /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Gd /TP /FC /Zl")

# We define all flags manually, so don't let cmake interfere
set(CMAKE_MSVC_RUNTIME_LIBRARY "")
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "")

set(CMAKE_RC_FLAGS "/nologo /l 0x0409")

add_executable(Hopmon WIN32
    # Files from the DX7 SDK samples CD, with Hopmon's modifications
    src/D3DApp.cpp
    src/D3DEnum.cpp
    src/D3DFile.cpp
    src/D3DFrame.cpp
    src/D3DMath.cpp
    src/D3DTextr.cpp
    src/D3DUtil.cpp
    src/ddutil.cpp

    src/JAscii.cpp
    src/JDInput.cpp
    src/JList.cpp
    src/JMidi.cpp
    src/JObject3D.cpp
    src/JSound.cpp
    src/JSprite.cpp
    src/JStandard.cpp
    src/JString.cpp
    src/JStringList.cpp
    src/JSurface.cpp
    src/JWinReg.cpp

    src/XAppMain.cpp
    src/XChara.cpp
    src/XEnemy.cpp
    src/XGame.cpp
    src/XMap.cpp
    src/XMapObject.cpp
    src/XPac.cpp
    src/XRegister.cpp
    src/XTitle.cpp

    res/hopmon.rc)

target_include_directories(Hopmon PRIVATE include)
target_link_directories(Hopmon PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/dx7sdk/lib")

if(HOPMON_FHD)
  target_compile_definitions(Hopmon PRIVATE HOPMON_FHD)
endif()

target_link_libraries(Hopmon PRIVATE
    kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib
    ddraw.lib dsound.lib dxguid.lib
    dinput.lib d3dxof.lib
    libcmt.lib libvcruntime.lib libucrt.lib)

target_link_options(Hopmon PRIVATE
    # Hopmon.exe's own image base, so that the PDB's addresses match
    /base:0x00400000
    /entry:WinMainCRTStartup
    /safeseh:no
    /incremental:no
    /debug)
