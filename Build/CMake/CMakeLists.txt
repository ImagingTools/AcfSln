cmake_minimum_required(VERSION 3.26)

project(AcfSln)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
if(QT_VERSION_MAJOR EQUAL 6)
	find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core Widgets Gui Xml Network Svg SvgWidgets Concurrent PrintSupport REQUIRED)
else()
	find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core Widgets Gui Xml Network Svg Concurrent PrintSupport REQUIRED)
endif()

set(BUILD_TESTING ON CACHE BOOL "Configures projects for testing")

# Enable modern target-based architecture:
# find_package(Acf) provides Acf::<lib> imported targets with transitive include dirs and link deps.
# Legacy global include_directories()/link_directories() in the Env chain are skipped.
set(ACF_MODERN_CMAKE ON CACHE BOOL "Enable modern target-based architecture")

if(NOT DEFINED ACFDIR)
	if(DEFINED ENV{ACFDIR})
		file(TO_CMAKE_PATH "$ENV{ACFDIR}" ACFDIR)
	else()
		set(ACFDIR "${CMAKE_CURRENT_LIST_DIR}/../../../Acf")
	endif()
endif()

set(ACFSLNDIR "${CMAKE_CURRENT_LIST_DIR}/../../../AcfSln")

include(${ACFSLNDIR}/Config/CMake/AcfSlnEnv.cmake)

if(NOT DEFINED ACFSLNDIR_BUILD)
	set(ACFSLNDIR_BUILD "${ACFSLNDIR}")
endif()

# Collect AcfSln libraries in their own export set so they are published as the
# AcfSln:: package. Read by StaticConfig.cmake / acf_register_library().
set(ACF_PACKAGE_NAME "AcfSln")

if(ACF_MODERN_CMAKE)
	acf_define_link_scope_var(ACF_QT_MODULE_LINK_SCOPE "PRIVATE" "Link scope used by acf_use_qt_base_modules and acf_use_qt_graphics_modules")
	acf_define_link_scope_var(ACF_LIBRARY_LINK_SCOPE "PUBLIC" "Link scope used by AcfSln inter-library dependencies")
	acf_define_link_scope_var(ACF_PACKAGE_LINK_SCOPE "PRIVATE" "Link scope used by package (Pck) libraries linking their dependencies")
	acf_define_link_scope_var(ACF_APPLICATION_LINK_SCOPE "PRIVATE" "Link scope used by executables linking their dependencies")
endif()

# Discover the Acf package published by the Acf build tree.
set(Acf_DIR "${ACFDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake" CACHE PATH "Path to the Acf build-tree CMake package")
find_package(Acf REQUIRED GLOBAL)

set(INCLUDE_DIR "${ACFSLNDIR_BUILD}/Include")
set(IMPL_DIR "${ACFSLNDIR_BUILD}/Impl")
set(AUX_INCLUDE_DIR "${ACFSLNDIR_BUILD}/AuxInclude/${TARGETNAME}/GeneratedFiles")

# Include
add_subdirectory("${INCLUDE_DIR}/ialgo/CMake" "${AUX_INCLUDE_DIR}/ialgo")
add_subdirectory("${INCLUDE_DIR}/iauth/CMake" "${AUX_INCLUDE_DIR}/iauth")
add_subdirectory("${INCLUDE_DIR}/ibarcode/CMake" "${AUX_INCLUDE_DIR}/ibarcode")
add_subdirectory("${INCLUDE_DIR}/icomm/CMake" "${AUX_INCLUDE_DIR}/icomm")
add_subdirectory("${INCLUDE_DIR}/ihotf/CMake" "${AUX_INCLUDE_DIR}/ihotf")
add_subdirectory("${INCLUDE_DIR}/ihotfgui/CMake" "${AUX_INCLUDE_DIR}/ihotfgui")
add_subdirectory("${INCLUDE_DIR}/imeas/CMake" "${AUX_INCLUDE_DIR}/imeas")
add_subdirectory("${INCLUDE_DIR}/iqtmeas/CMake" "${AUX_INCLUDE_DIR}/iqtmeas")
add_subdirectory("${INCLUDE_DIR}/iqtex/CMake" "${AUX_INCLUDE_DIR}/iqtex")
add_subdirectory("${INCLUDE_DIR}/iqtmm/CMake" "${AUX_INCLUDE_DIR}/iqtmm")
add_subdirectory("${INCLUDE_DIR}/icalib/CMake" "${AUX_INCLUDE_DIR}/icalib")
add_subdirectory("${INCLUDE_DIR}/icalibgui/CMake" "${AUX_INCLUDE_DIR}/icalibgui")
add_subdirectory("${INCLUDE_DIR}/icam/CMake" "${AUX_INCLUDE_DIR}/icam")
add_subdirectory("${INCLUDE_DIR}/iqtcam/CMake" "${AUX_INCLUDE_DIR}/iqtcam")
add_subdirectory("${INCLUDE_DIR}/iqtauth/CMake" "${AUX_INCLUDE_DIR}/iqtauth")
add_subdirectory("${INCLUDE_DIR}/iinsp/CMake" "${AUX_INCLUDE_DIR}/iinsp")
add_subdirectory("${INCLUDE_DIR}/iqtinsp/CMake" "${AUX_INCLUDE_DIR}/iqtinsp")
add_subdirectory("${INCLUDE_DIR}/iipr/CMake" "${AUX_INCLUDE_DIR}/iipr")
add_subdirectory("${INCLUDE_DIR}/iqtipr/CMake" "${AUX_INCLUDE_DIR}/iqtipr")
add_subdirectory("${INCLUDE_DIR}/isig/CMake" "${AUX_INCLUDE_DIR}/isig")
add_subdirectory("${INCLUDE_DIR}/iqtsig/CMake" "${AUX_INCLUDE_DIR}/iqtsig")
add_subdirectory("${INCLUDE_DIR}/ifileproc/CMake" "${AUX_INCLUDE_DIR}/ifileproc")
add_subdirectory("${INCLUDE_DIR}/iprod/CMake" "${AUX_INCLUDE_DIR}/iprod")
add_subdirectory("${INCLUDE_DIR}/iproc/CMake" "${AUX_INCLUDE_DIR}/iproc")
add_subdirectory("${INCLUDE_DIR}/iprocgui/CMake" "${AUX_INCLUDE_DIR}/iprocgui")
add_subdirectory("${INCLUDE_DIR}/iweb/CMake" "${AUX_INCLUDE_DIR}/iweb")
add_subdirectory("${INCLUDE_DIR}/iwiz/CMake" "${AUX_INCLUDE_DIR}/iwiz")
add_subdirectory("${INCLUDE_DIR}/iwizgui/CMake" "${AUX_INCLUDE_DIR}/iwizgui")
add_subdirectory("${INCLUDE_DIR}/iedge/CMake" "${AUX_INCLUDE_DIR}/iedge")
add_subdirectory("${INCLUDE_DIR}/iedgegui/CMake" "${AUX_INCLUDE_DIR}/iedgegui")
add_subdirectory("${INCLUDE_DIR}/icmpstr/CMake" "${AUX_INCLUDE_DIR}/icmpstr")
add_subdirectory("${INCLUDE_DIR}/idocproc/CMake" "${AUX_INCLUDE_DIR}/idocproc")
add_subdirectory("${INCLUDE_DIR}/iblob/CMake" "${AUX_INCLUDE_DIR}/iblob")
add_subdirectory("${INCLUDE_DIR}/iblobgui/CMake" "${AUX_INCLUDE_DIR}/iblobgui")
add_subdirectory("${INCLUDE_DIR}/iservice/CMake" "${AUX_INCLUDE_DIR}/iservice")
add_subdirectory("${INCLUDE_DIR}/imm/CMake" "${AUX_INCLUDE_DIR}/imm")


# Impl
if (NOT ANDROID)
add_subdirectory("${IMPL_DIR}/FileProcessingPck/CMake" "${AUX_INCLUDE_DIR}/FileProcessingPck")
add_subdirectory("${IMPL_DIR}/ProcessingPck/CMake" "${AUX_INCLUDE_DIR}/ProcessingPck")
add_subdirectory("${IMPL_DIR}/AcfSlnLoc/CMake" "${AUX_INCLUDE_DIR}/AcfSlnLoc")
add_subdirectory("${IMPL_DIR}/AuthorizationPck/CMake" "${AUX_INCLUDE_DIR}/AuthorizationPck")
add_subdirectory("${IMPL_DIR}/BlobPck/CMake" "${AUX_INCLUDE_DIR}/BlobPck")
add_subdirectory("${IMPL_DIR}/BarcodePck/CMake" "${AUX_INCLUDE_DIR}/BarcodePck")
add_subdirectory("${IMPL_DIR}/CameraPck/CMake" "${AUX_INCLUDE_DIR}/CameraPck")
add_subdirectory("${IMPL_DIR}/CalibrationPck/CMake" "${AUX_INCLUDE_DIR}/CalibrationPck")
add_subdirectory("${IMPL_DIR}/CommunicationPck/CMake" "${AUX_INCLUDE_DIR}/CommunicationPck")
add_subdirectory("${IMPL_DIR}/EdgePck/CMake" "${AUX_INCLUDE_DIR}/EdgePck")
add_subdirectory("${IMPL_DIR}/HotfolderPck/CMake" "${AUX_INCLUDE_DIR}/HotfolderPck")
add_subdirectory("${IMPL_DIR}/InspectionPck/CMake" "${AUX_INCLUDE_DIR}/InspectionPck")
add_subdirectory("${IMPL_DIR}/IprPck/CMake" "${AUX_INCLUDE_DIR}/IprPck")
add_subdirectory("${IMPL_DIR}/MeasPck/CMake" "${AUX_INCLUDE_DIR}/MeasPck")
add_subdirectory("${IMPL_DIR}/SignalPck/CMake" "${AUX_INCLUDE_DIR}/SignalPck")
add_subdirectory("${IMPL_DIR}/WebPck/CMake" "${AUX_INCLUDE_DIR}/WebPck")
add_subdirectory("${IMPL_DIR}/WizardPck/CMake" "${AUX_INCLUDE_DIR}/WizardPck")
add_subdirectory("${IMPL_DIR}/QtAuthorizationPck/CMake" "${AUX_INCLUDE_DIR}/QtAuthorizationPck")
add_subdirectory("${IMPL_DIR}/QtCameraPck/CMake" "${AUX_INCLUDE_DIR}/QtCameraPck")
add_subdirectory("${IMPL_DIR}/CalibrationGuiPck/CMake" "${AUX_INCLUDE_DIR}/CalibrationGuiPck")
add_subdirectory("${IMPL_DIR}/QtExPck/CMake" "${AUX_INCLUDE_DIR}/QtExPck")
add_subdirectory("${IMPL_DIR}/QtInspectionPck/CMake" "${AUX_INCLUDE_DIR}/QtInspectionPck")
add_subdirectory("${IMPL_DIR}/QtIprPck/CMake" "${AUX_INCLUDE_DIR}/QtIprPck")
add_subdirectory("${IMPL_DIR}/QtMeasPck/CMake" "${AUX_INCLUDE_DIR}/QtMeasPck")
add_subdirectory("${IMPL_DIR}/QtMmPck/CMake" "${AUX_INCLUDE_DIR}/QtMmPck")
add_subdirectory("${IMPL_DIR}/QtSignalPck/CMake" "${AUX_INCLUDE_DIR}/QtSignalPck")
add_subdirectory("${IMPL_DIR}/WizardGuiPck/CMake" "${AUX_INCLUDE_DIR}/WizardGuiPck")
add_subdirectory("${IMPL_DIR}/HotfolderGuiPck/CMake" "${AUX_INCLUDE_DIR}/HotfolderGuiPck")
add_subdirectory("${IMPL_DIR}/ProcessingGuiPck/CMake" "${AUX_INCLUDE_DIR}/ProcessingGuiPck")
add_subdirectory("${IMPL_DIR}/CompositorPck/CMake" "${AUX_INCLUDE_DIR}/CompositorPck")
add_subdirectory("${IMPL_DIR}/DocumentProcessingPck/CMake" "${AUX_INCLUDE_DIR}/DocumentProcessingPck")
add_subdirectory("${IMPL_DIR}/ProductionPck/CMake" "${AUX_INCLUDE_DIR}/ProductionPck")
add_subdirectory("${IMPL_DIR}/XpcEditorExe/CMake" "${AUX_INCLUDE_DIR}/XpcEditor")
add_dependencies(XpcEditor FileProcessingPck)
add_subdirectory("${IMPL_DIR}/CompositorExe/CMake" "${AUX_INCLUDE_DIR}/Compositor")
add_dependencies(Compositor FileProcessingPck)
endif()


#-------- Tests -----------
if (BUILD_TESTING)
	find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Test REQUIRED)

	add_subdirectory("../../Tests/JsonArchiveIntergrationTest/CMake" "${AUX_INCLUDE_DIR}/Tests/JsonArchiveIntergrationTest")
	add_dependencies(JsonArchiveIntergrationTest imeas)
endif()

# Declare target-based deps so include paths/link order propagate transitively across AcfSln libraries.
include("${ACFSLNDIR}/Config/CMake/AcfSlnLibraryDependencies.cmake")

# Export AcfSln libraries as a find_package(AcfSln) package (build + install tree) so downstream
# modules (IAcf, ImtCore) consume them via AcfSln::<lib> targets instead of env vars/manual paths.
include("${ACFSLNDIR}/Config/CMake/AcfSlnPackageExport.cmake")

