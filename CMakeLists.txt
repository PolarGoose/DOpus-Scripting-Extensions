cmake_minimum_required(VERSION 3.31)
project(DOpusScriptingExtensions LANGUAGES C CXX RC)
include(FetchContent)
set(INSTALLER_VERSION "0.0" CACHE STRING "DOpusScriptingExtensions installer version")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

find_package(Boost CONFIG REQUIRED COMPONENTS dll filesystem locale process)
find_package(unofficial-libmagic CONFIG REQUIRED)
find_package(MediaInfoLib CONFIG REQUIRED)
find_package(uchardet CONFIG REQUIRED)

find_program(MIDL_EXECUTABLE midl REQUIRED)
find_program(MSBUILD_EXECUTABLE msbuild REQUIRED)

FetchContent_Populate(cmake_template GIT_REPOSITORY https://github.com/cpp-best-practices/cmake_template
                                     GIT_TAG a3971f5b4588a5f57a44317a0f7e29827add13f1)
include(${cmake_template_SOURCE_DIR}/cmake/CompilerWarnings.cmake)

FetchContent_Populate(trid2lib URL "https://github.com/Papyrus7914/TrId2-file-type-identifier/releases/download/v1.0/TrId2Lib.zip")
FetchContent_Populate(exiftool URL "https://downloads.sourceforge.net/project/exiftool/exiftool-13.59_64.zip")
FetchContent_Populate(mediainfo URL "https://github.com/MediaArea/MediaInfo/archive/refs/tags/v26.05.zip")
file(RENAME "${exiftool_SOURCE_DIR}/exiftool(-k).exe" "${exiftool_SOURCE_DIR}/exiftool.exe" RESULT ignore_result)

set(midl_generated_dir "${CMAKE_CURRENT_BINARY_DIR}/midl_generated/midl_generated")
set(midl_generated_sources "${midl_generated_dir}/DOpusScriptingExtensions_i.h"
                           "${midl_generated_dir}/DOpusScriptingExtensions_i.c"
                           "${midl_generated_dir}/DOpusScriptingExtensions.tlb")
add_custom_command(VERBATIM COMMENT "Generating COM interfaces and type library"
  OUTPUT ${midl_generated_sources}
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${midl_generated_dir}"
  COMMAND "${MIDL_EXECUTABLE}" /nologo /env x64 /target NT100 /char signed
          /h "DOpusScriptingExtensions_i.h"
          /iid "DOpusScriptingExtensions_i.c"
          /proxy NUL
          /tlb "DOpusScriptingExtensions.tlb"
          /out "${midl_generated_dir}"
          "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/DOpusScriptingExtensions.idl"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/DOpusScriptingExtensions.idl")
add_library(midl_generated OBJECT ${midl_generated_sources})
target_compile_options(midl_generated PRIVATE /W0)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/*.h"
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/*.cpp"
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/*.rgs"
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/*.rc"
                                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/*.def")

add_library(DOpusScriptingExtensions SHARED "${source_files}")
target_include_directories(DOpusScriptingExtensions PRIVATE "src")
target_include_directories(DOpusScriptingExtensions SYSTEM PRIVATE "${midl_generated_dir}/.." "${trid2lib_SOURCE_DIR}")
target_precompile_headers(DOpusScriptingExtensions PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/DOpusScriptingExtensions/pch.h")
target_compile_definitions(DOpusScriptingExtensions PRIVATE UNICODE
                                                            _WINDLL
                                                            WIN32_LEAN_AND_MEAN
                                                            VC_EXTRALEAN
                                                            NOMINMAX
                                                            STRICT
                                                            _ATL_APARTMENT_THREADED
                                                            _ATL_NO_AUTOMATIC_NAMESPACE
                                                            _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
                                                            ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
                                                            $<$<CONFIG:Debug>:DEBUG>)
target_compile_options(DOpusScriptingExtensions PRIVATE /utf-8
                                                        /WX
                                                        /W4
                                                        /analyze
                                                        /analyze:external-
                                                        /bigobj)
target_link_libraries(DOpusScriptingExtensions PRIVATE midl_generated
                                                       Boost::dll
                                                       Boost::filesystem
                                                       Boost::locale
                                                       Boost::process
                                                       unofficial::libmagic::libmagic
                                                       mediainfo
                                                       uchardet::libuchardet
                                                       ntdll
                                                       crypt32
                                                       secur32
                                                       iphlpapi
                                                       rpcrt4)
add_library(DOpusScriptingExtensions_warnings INTERFACE)
myproject_set_project_warnings(DOpusScriptingExtensions_warnings ON "" "" "" "")
target_link_libraries(DOpusScriptingExtensions PUBLIC DOpusScriptingExtensions_warnings)

add_custom_target(installer ALL USES_TERMINAL COMMENT "Build installer" VERBATIM
  COMMAND
    "${MSBUILD_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Installer/Installer.wixproj"
    /nologo
    /restore
    /verbosity:minimal
    /property:Platform=x64
    "/property:InstallerVersion=${INSTALLER_VERSION}"
    "/property:CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    "/property:MagicMgcFile=${unofficial-libmagic_DICTIONARY}"
    "/property:OutputPath=${CMAKE_CURRENT_BINARY_DIR}"
    "/property:BaseIntermediateOutputPath=${CMAKE_CURRENT_BINARY_DIR}/installer_obj/"
    "/property:IntermediateOutputPath=${CMAKE_CURRENT_BINARY_DIR}/installer_obj/"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Installer/Installer.wixproj"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Installer/Package.wxs"
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Installer.msi")
add_dependencies(installer DOpusScriptingExtensions)
