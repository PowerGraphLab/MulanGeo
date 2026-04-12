cmake_minimum_required(VERSION 3.20)
project(MulanGeo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ── OpenCASCADE (manual config to avoid VTK dependency) ──
set(OCCT_ROOT "F:/dev/opencascade-7.9.3-vc14-64")
set(OCCT_INCLUDE_DIR "${OCCT_ROOT}/inc")
set(OCCT_LIB_DIR "${OCCT_ROOT}/win64/vc14/lib")

# Only the toolkits we actually need
set(OCCT_LIBS
    TKernel TKMath
    TKG2d TKG3d TKGeomBase TKBRep
    TKGeomAlgo TKTopAlgo TKPrim TKBO TKShHealing TKMesh
    TKCDF TKLCAF TKCAF TKBinL TKBin TKStdL TKStd TKVCAF
    TKDE TKXSBase TKDESTEP TKXCAF TKDEIGES TKDESTL TKRWMesh TKDECascade TKBinXCAF
)

# ── Qt6 ──
set(CMAKE_PREFIX_PATH "F:/dev/qt6/6.8.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets)

# ── Subdirectories ──
add_subdirectory(src/IO)
add_subdirectory(src/QtApp)
