project(XpcEditor LANGUAGES CXX)


set(ARXC_CONFIG "${ACFSLNDIR}/Config/AcfSlnConfig.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST OFF)

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/XpcEditor.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../../../Partitura/AcfInfoCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
endif()

set(BUILD_ARGS "dupbuild=warn")

set(DEPS_CUSTOM_BUILD FileProcessingPck)
if (TARGET BasePck)
	list(APPEND DEPS_CUSTOM_BUILD BasePck)
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${ACFDIR}/Config/CMake/DeployQt.cmake)

target_link_libraries(${PROJECT_NAME} ${ACF_APPLICATION_LINK_SCOPE}
	icmpstr
	AcfSlnLoc
	Acf::AcfLoc
	Acf::iloggui
)

deploy_qt(TARGET ${PROJECT_NAME})

