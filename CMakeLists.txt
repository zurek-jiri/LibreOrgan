# LibreOrgan - portable CMake build (Windows, macOS, Linux).
#
#   git clone --recurse-submodules <repo>      (JUCE is a submodule in JUCE/)
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --config Release
#
# The Visual Studio project under LibreOrgan/Builds (Projucer-generated from
# LibreOrgan/LibreOrgan.jucer) is the Windows developer's build; this file is
# what the GitHub Actions workflow and the macOS/Linux builds use. Both compile
# the same sources with the same defines.
cmake_minimum_required(VERSION 3.22)
project(LibreOrgan VERSION 0.9.14 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# THE RULE (30 Oct 2026): the released executable depends on NOTHING outside a plain operating
# system - no Visual C++ redistributable, no DLL beside the exe, no installer. The C and C++ runtime
# is linked statically into LibreOrgan.exe and into every library built here (the Projucer build
# does the same: useRuntimeLibDLL="0"). Before this the CI exe was /MD and a clean Windows without
# the 2015-2022 redistributable could not start it (MSVCP140.dll / VCRUNTIME140.dll missing).
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found in JUCE/. Clone with --recurse-submodules or run: git submodule update --init")
endif()
add_subdirectory(JUCE)

set(ORGAN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/LibreOrgan/Source")

juce_add_gui_app(LibreOrgan
    PRODUCT_NAME "LibreOrgan"
    COMPANY_NAME "LibreOrgan"
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "org.libreorgan.sampler"
    ICON_BIG "${ORGAN_SRC}/AppIcon.png"
    ICON_SMALL "${ORGAN_SRC}/Icons/icon-32.png"   # the hand-tuned small icon of the Windows .ico (Tools/make_small_icons.ps1)
    NEEDS_MIDI_INPUT TRUE
    MICROPHONE_PERMISSION_ENABLED FALSE)

juce_generate_juce_header(LibreOrgan)

target_sources(LibreOrgan PRIVATE
    "${ORGAN_SRC}/Main.cpp"
    "${ORGAN_SRC}/MainComponent.cpp"
    "${ORGAN_SRC}/SamplerEngine.cpp"
    "${ORGAN_SRC}/voicer.cpp"
    "${ORGAN_SRC}/ArchiveExtract.cpp"
    "${ORGAN_SRC}/SampleFiles.cpp"
    "${ORGAN_SRC}/HauptwerkOdf.cpp"
    "${ORGAN_SRC}/MainComponentMerge.cpp"
    "${ORGAN_SRC}/LinuxWindowPlacement.cpp"
    "${ORGAN_SRC}/AlignedNew.cpp")

# libarchive (BSD licence, git submodule ThirdParty/libarchive): RAR archives are
# unpacked inside the program (ArchiveExtract.cpp). Built from source as a static
# library with its readers only - no tar/cpio tools, no tests, no external
# compression or crypto libraries, so every platform builds it alike. (The
# Projucer/Visual Studio build links the prebuilt ThirdParty/libarchive-prebuilt
# instead, made from the same sources with the same options.)
foreach(option ENABLE_TEST ENABLE_INSTALL ENABLE_TAR ENABLE_CPIO ENABLE_CAT ENABLE_UNZIP ENABLE_OPENSSL ENABLE_ZLIB ENABLE_BZip2
        ENABLE_LZMA ENABLE_ZSTD ENABLE_LZ4 ENABLE_LZO ENABLE_LIBXML2 ENABLE_EXPAT ENABLE_ICONV ENABLE_CNG ENABLE_XATTR ENABLE_ACL
        ENABLE_LIBB2 ENABLE_PCREPOSIX ENABLE_PCRE2POSIX ENABLE_WIN32_XMLLITE ENABLE_MBEDTLS ENABLE_NETTLE BUILD_SHARED_LIBS)
    set(${option} OFF CACHE BOOL "" FORCE)
endforeach()
add_subdirectory(ThirdParty/libarchive EXCLUDE_FROM_ALL)
target_include_directories(LibreOrgan PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libarchive/libarchive")
target_compile_definitions(LibreOrgan PRIVATE LIBARCHIVE_STATIC=1)
target_link_libraries(LibreOrgan PRIVATE archive_static)
# unarr (LGPL-3.0, git submodule ThirdParty/unarr, pinned to the commit Masterpiece
# uses): the RAR 4 files libarchive cannot unpack - PPMd blocks (text: an organ
# definition packed with the "best" method) and the RAR VM filters of the WinRAR
# versions of 2011 (the Dom Bedos set) - are read through it (SampleFiles.cpp,
# 0.8.217). Built from the same file list as Masterpiece's, no zlib/bzip2/lzma, RAR
# only; the header unarr.h is the checked-in ThirdParty/unarr-include/unarr.h
# (Tools/make_unarr_header.py from unarr.h.in). Without the submodule the program
# builds without it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/unarr/rar/rar.c")
    set(UNARR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/unarr")
    add_library(organ_unarr STATIC
        "${UNARR_DIR}/common/conv.c" "${UNARR_DIR}/common/crc32.c" "${UNARR_DIR}/common/stream.c" "${UNARR_DIR}/common/unarr.c"
        "${UNARR_DIR}/lzmasdk/CpuArch.c" "${UNARR_DIR}/lzmasdk/Ppmd7.c" "${UNARR_DIR}/lzmasdk/Ppmd7Dec.c" "${UNARR_DIR}/lzmasdk/Ppmd7aDec.c"
        "${UNARR_DIR}/rar/filter-rar.c" "${UNARR_DIR}/rar/huffman-rar.c" "${UNARR_DIR}/rar/parse-rar.c" "${UNARR_DIR}/rar/rar.c"
        "${UNARR_DIR}/rar/rarvm.c" "${UNARR_DIR}/rar/uncompress-rar.c")
    target_include_directories(organ_unarr PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/unarr-include" PRIVATE "${UNARR_DIR}" "${UNARR_DIR}/common")
    target_compile_definitions(organ_unarr PRIVATE _FILE_OFFSET_BITS=64 $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
    if(UNIX)
        set_source_files_properties("${UNARR_DIR}/common/stream.c" PROPERTIES COMPILE_DEFINITIONS _POSIX_C_SOURCE=200809L)
    endif()
    set_target_properties(organ_unarr PROPERTIES C_STANDARD 99 POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(LibreOrgan PRIVATE organ_unarr)
    target_compile_definitions(LibreOrgan PRIVATE ORGAN_HAVE_UNARR=1)
endif()
# The Mac compatibility file defines symbols of the C++ library under their own
# names; as a native (non-LTO) object the linker treats them like the library's.
if(APPLE)
    set_source_files_properties("${ORGAN_SRC}/AlignedNew.cpp" PROPERTIES COMPILE_OPTIONS "-fno-lto")
endif()

# The icons as drawn per size (16-128 hand-tuned outlines, 256-1024 the picture),
# not JUCE's rescaling of the two icons above: JUCE writes icon.ico / AppIcon.icns
# into its generated-sources folder at configure time, and ours replace them here.
get_target_property(organ_generated LibreOrgan JUCE_GENERATED_SOURCES_DIRECTORY)
if(WIN32)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LibreOrgan/LibreOrgan_icon.ico" "${organ_generated}/icon.ico" COPYONLY)
elseif(APPLE)
    configure_file("${ORGAN_SRC}/Icons/LibreOrgan.icns" "${organ_generated}/AppIcon.icns" COPYONLY)
endif()

# Images used by the UI (BinaryData::ProspektSP_png etc.), like the Projucer resources.
juce_add_binary_data(LibreOrganData SOURCES
    "${ORGAN_SRC}/ProspektSP.png"
    "${ORGAN_SRC}/ProspektSP_2.jpg"
    "${ORGAN_SRC}/_Prospekt3.ico"
    "${ORGAN_SRC}/AppIcon.png"
    "${ORGAN_SRC}/Icons/icon-16.png"
    "${ORGAN_SRC}/Icons/icon-24.png"
    "${ORGAN_SRC}/Icons/icon-32.png"
    "${ORGAN_SRC}/Icons/icon-48.png"
    "${ORGAN_SRC}/Icons/icon-64.png"
    "${ORGAN_SRC}/Icons/icon-128.png"
    "${ORGAN_SRC}/Icons/icon-256.png")

target_include_directories(LibreOrgan PRIVATE
    "${ORGAN_SRC}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/signalsmith")

# ASIO exists on Windows only (Steinberg ASIO SDK 2.3.4, GPLv3 option, in ASIOSDK/).
if(WIN32)
    target_sources(LibreOrgan PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK/common/asio.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK/host/asiodrivers.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK/host/pc/asiolist.cpp")
    target_include_directories(LibreOrgan PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK/host"
        "${CMAKE_CURRENT_SOURCE_DIR}/ASIOSDK/host/pc")
    target_compile_definitions(LibreOrgan PRIVATE JUCE_ASIO=1 JUCE_ASIO_DEBUGGING=1 _USE_MATH_DEFINES=1 NOMINMAX=1)   # M_PI and std::min/max for Rubber Band under MSVC
    target_link_libraries(LibreOrgan PRIVATE dbghelp bcrypt)   # bcrypt: libarchive's random numbers (BCryptGenRandom)
endif()

# Linux: JACK (through PipeWire's libjack, loaded at start) shows every channel
# of every card; the ALSA list is filtered of the template PCMs (PlatformAudio.h).
if(UNIX AND NOT APPLE)
    target_compile_definitions(LibreOrgan PRIVATE JUCE_JACK=1)
    # libX11 itself: JUCE loads X11 through dlopen (nothing of ours links it), but
    # LinuxWindowPlacement.cpp calls Xlib directly (0.8.145: undefined XOpenDisplay at link time).
    find_package(X11 REQUIRED)
    target_link_libraries(LibreOrgan PRIVATE ${CMAKE_DL_LIBS} ${X11_LIBRARIES})
endif()

target_compile_definitions(LibreOrgan PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    # Sample folders are WAV (FLAC/Ogg readers stay); MP3, Opus and WebP only add code.
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_USE_OPUS=0
    JUCE_USE_WEBP=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:LibreOrgan,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:LibreOrgan,JUCE_VERSION>")

target_link_libraries(LibreOrgan PRIVATE
    LibreOrganData
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_processors_headless
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_midi_ci
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# The data tables (temperaments, licence texts) are committed as generated headers;
# on Windows the PowerShell generators refresh them before the build when present.
find_program(ORGAN_PWSH NAMES pwsh powershell)
if(ORGAN_PWSH)
    add_custom_command(TARGET LibreOrgan PRE_BUILD
        COMMAND "${ORGAN_PWSH}" -NoProfile -ExecutionPolicy Bypass -File "${ORGAN_SRC}/GenerateTemperamentsData.ps1"
        COMMENT "Embedding temperaments.csv and the licence texts")
endif()
