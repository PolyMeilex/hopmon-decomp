# vsdownload.py \
#   --accept-license \
#   --manifest https://download.visualstudio.microsoft.com/download/pr/5a50b8ac-2c22-47f1-ba60-70d4257a78fa/d4e2bf84555eefd36cb47ff0bfdc9516ac6c1c1563b233a269528371627c5042/VisualStudio.vsman \
#   --msvc-version 16.11 \
#   --sdk-version 10.0.19041 \
#   --architecture x86 \
#   --dest hopmon-toolchain

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

set(MSVC_WINE_DIR "$ENV{HOME}/tmp/msvc-wine/hopmon-toolchain" CACHE PATH "msvc-wine installation to build with")
set(MSVC_WINE_TOOLSET 14.29.30133 CACHE STRING "MSVC toolset version inside MSVC_WINE_DIR")
set(MSVC_WINE_SDK 10.0.19041.0 CACHE STRING "Windows SDK version inside MSVC_WINE_DIR")

set(CMAKE_CXX_COMPILER "${MSVC_WINE_DIR}/bin/x86/cl")
set(CMAKE_RC_COMPILER "${MSVC_WINE_DIR}/bin/x86/rc")
set(CMAKE_EXE_LINKER_FLAGS_INIT "/MANIFEST:NO")
