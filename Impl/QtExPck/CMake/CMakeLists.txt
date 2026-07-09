project(QtExPck LANGUAGES CXX)


include(${ACFDIR}/Config/CMake/ComponentConfig.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

target_link_libraries(${PROJECT_NAME} ${ACF_PACKAGE_LINK_SCOPE}
	iqtex
	iservice
)

if(QT_VERSION_MAJOR EQUAL 5)
	target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::XmlPatterns)
endif()

if(WIN32)
	set(listFiles \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.arp\")

	windeploy(${PROJECT_NAME} "" "${listFiles}")
	unset(listOptions)
endif()

