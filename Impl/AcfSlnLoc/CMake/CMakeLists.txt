project(AcfSlnLoc LANGUAGES CXX)


set(ARXC_CONFIG "${ACFDIR}/Config/AcfConfig.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ACF_CONVERT_FILES "")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST OFF)

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
include(${ACFDIR}/Config/CMake/StaticConfig.cmake)

