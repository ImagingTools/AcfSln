project(iservice LANGUAGES CXX)


#include(${ACFDIR}/Config/CMake/QtGuiBaseConfig.cmake)
include(${ACFDIR}/Config/CMake/GeneralConfig.cmake)


file(GLOB HEADER_FILES "${PROJECT_SOURCE_DIR}/../*.h")
file(GLOB SOURCES_FILES "${PROJECT_SOURCE_DIR}/../*.cpp")
file(GLOB QRC_FILES "${PROJECT_SOURCE_DIR}/../*.qrc")




if (WIN32)
	add_compile_options(/utf-8)
	ADD_DEFINITIONS(-DUNICODE)
	ADD_DEFINITIONS(-D_UNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /UMBCS /D_UNICODE /DUNICODE")
	file(GLOB HEADER_FILES
		"${PROJECT_SOURCE_DIR}/../CServiceApplicationComp.h"
		"${PROJECT_SOURCE_DIR}/../qtservice.h"
		"${PROJECT_SOURCE_DIR}/../qtservice_win.h"
		"${PROJECT_SOURCE_DIR}/../qtservicestarter.h"
		)
	file(GLOB SOURCES_FILES
		"${PROJECT_SOURCE_DIR}/../CServiceApplicationComp.cpp"
		"${PROJECT_SOURCE_DIR}/../qtservice.cpp"
		"${PROJECT_SOURCE_DIR}/../qtservice_win.cpp"
		"${PROJECT_SOURCE_DIR}/../qtservicestarter.cpp"
		)
else()
	file(GLOB HEADER_FILES
		"${PROJECT_SOURCE_DIR}/../CServiceApplicationComp.h"
	)
	file(GLOB SOURCES_FILES
		"${PROJECT_SOURCE_DIR}/../CServiceApplicationComp.cpp"
	)
endif()


file(GLOB PROJECT_SRC
		${HEADER_FILES}
		${SOURCES_FILES}
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})

acf_create_moc(MOC_SOURCES "${HEADER_FILES}")

target_sources(${PROJECT_NAME} PRIVATE ${MOC_SOURCES})

set(outlibdir "${AUX_INCLUDE_DIR}/../../../Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")

set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY ARCHIVE_OUTPUT_DIRECTORY ${outlibdir})

set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY LIBRARY_OUTPUT_DIRECTORY ${outlibdir})

set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY COMPILE_PDB_OUTPUT_DIRECTORY "${outlibdir}")

set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY COMPILE_PDB_NAME "${PROJECT_NAME}")

set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${outlibdir})
set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY LIBRARY_OUTPUT_PATH ${outlibdir})
set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY CMAKE_RUNTIME_OUTPUT_DIRECTORY ${outlibdir})
set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY POSITION_INDEPENDENT_CODE ON)

acf_use_qt_base_modules()

# Turn this manually-defined library into a first-class, exportable target, just
# like the StaticConfig.cmake based libraries: attach transitive include
# directories, define the Acf::<lib> alias and register it for the AcfSln
# package export. Guarded so the in-tree build keeps working with an Acf that
# predates the acf_register_library() helper (Acf PR #228).
if(COMMAND acf_register_library)
	acf_register_library(${PROJECT_NAME})
endif()


