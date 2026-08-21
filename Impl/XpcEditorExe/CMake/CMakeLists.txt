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

target_link_libraries(${PROJECT_NAME} ${ACF_APPLICATION_LINK_SCOPE}
	icmpstr
	AcfSlnLoc
	Acf::AcfLoc
	Acf::iloggui
)

if(APPLE)
	# Use the same bundle metadata as the QMake build (QMAKE_INFO_PLIST and ICON).
	# Without a valid Info.plist the generated bundle has no bundle identifier,
	# which breaks macOS services used by AppKit, e.g. the native file panels.
	set(MACOSX_BUNDLE_ICON_FILE "${PROJECT_NAME}.icns")
	set(BUNDLE_ICON_FILE "${PROJECT_SOURCE_DIR}/../Mac/${MACOSX_BUNDLE_ICON_FILE}")

	target_sources(${PROJECT_NAME} PRIVATE ${BUNDLE_ICON_FILE})

	set_source_files_properties(${BUNDLE_ICON_FILE}
		PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

	set_target_properties(${PROJECT_NAME} PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/../Mac/Info.plist"
		MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
		MACOSX_BUNDLE_GUI_IDENTIFIER "com.imagingtools.${PROJECT_NAME}")
endif()

mac_deploy_qt(
	REMOVE_DEBUGSYM
	TARGET ${PROJECT_NAME}
	TARGET_FILE_NAME ${PROJECT_NAME}
)

