# ---------------------------------------------------------------------------
# Generate and export the "AcfSln" CMake package.
#
# This mirrors the ACF package export (Config/CMake/AcfPackageExport.cmake in
# the Acf module). After this runs, downstream projects (IAcf, and transitively
# ImtCore) can discover AcfSln with a single call:
#
#     find_package(AcfSln REQUIRED)
#     target_link_libraries(myTarget PUBLIC AcfSln::iipr AcfSln::iqtinsp ...)
#
# and inherit include directories and inter-library dependencies transitively.
# The generated config pulls in the underlying ACF package through
# find_dependency(Acf).
#
# Two flavours of the package are produced:
#  * a build-tree package (no install step required), written next to the
#    compiled libraries so it matches the existing in-tree build layout, and
#  * an install-tree package for a relocatable `cmake --install` deployment.
#
# The AcfSln library targets are collected in the ${ACF_EXPORT_SET} export set,
# which Build/CMake/CMakeLists.txt sets to "AcfSlnTargets" before the library
# subdirectories are added (the shared StaticConfig.cmake / acf_register_library
# machinery from Acf registers each library into that set).
#
# Included once, centrally, from Build/CMake/CMakeLists.txt after all library
# targets and their dependencies have been declared.
# ---------------------------------------------------------------------------

include(CMakePackageConfigHelpers)

if(NOT DEFINED ACF_EXPORT_SET)
	set(ACF_EXPORT_SET "AcfSlnTargets")
endif()

# Package version. project(AcfSln) does not set one, so fall back to a default
# that still lets consumers request a version and use find_package version
# checks.
if(NOT DEFINED AcfSln_VERSION OR AcfSln_VERSION STREQUAL "")
	if(DEFINED PROJECT_VERSION AND NOT PROJECT_VERSION STREQUAL "")
		set(AcfSln_VERSION "${PROJECT_VERSION}")
	else()
		set(AcfSln_VERSION "1.0.0")
	endif()
endif()

# Build-tree location: alongside the produced libraries, mirroring the existing
# Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME} layout.
set(ACFSLN_LIB_OUTPUT_DIR "${ACFSLNDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
set(ACFSLN_PACKAGE_BUILD_DIR "${ACFSLN_LIB_OUTPUT_DIR}/cmake")

# --- Build-tree export ------------------------------------------------------
export(EXPORT ${ACF_EXPORT_SET}
	NAMESPACE AcfSln::
	FILE "${ACFSLN_PACKAGE_BUILD_DIR}/AcfSlnTargets.cmake")

# Qt major version this package was built against. Baked into the generated
# AcfSlnConfig.cmake so consumers resolve the matching Qt imported targets.
set(ACFSLN_QT_VERSION_MAJOR "${QT_VERSION_MAJOR}")

configure_package_config_file(
	"${ACFSLNDIR}/Config/CMake/AcfSlnConfig.cmake.in"
	"${ACFSLN_PACKAGE_BUILD_DIR}/AcfSlnConfig.cmake"
	INSTALL_DESTINATION "${ACFSLN_PACKAGE_BUILD_DIR}"
	NO_SET_AND_CHECK_MACRO)

write_basic_package_version_file(
	"${ACFSLN_PACKAGE_BUILD_DIR}/AcfSlnConfigVersion.cmake"
	VERSION "${AcfSln_VERSION}"
	COMPATIBILITY SameMajorVersion)

# Allow find_package(AcfSln) to locate the build-tree package directly (e.g. via
# CMAKE_PREFIX_PATH=<build>/Lib/<config>/cmake or AcfSln_DIR).
message(STATUS "AcfSln: build-tree package written to ${ACFSLN_PACKAGE_BUILD_DIR}")

# --- Install-tree export ----------------------------------------------------
install(EXPORT ${ACF_EXPORT_SET}
	NAMESPACE AcfSln::
	DESTINATION "lib/cmake/AcfSln"
	FILE "AcfSlnTargets.cmake")

install(FILES
	"${ACFSLN_PACKAGE_BUILD_DIR}/AcfSlnConfig.cmake"
	"${ACFSLN_PACKAGE_BUILD_DIR}/AcfSlnConfigVersion.cmake"
	DESTINATION "lib/cmake/AcfSln")
