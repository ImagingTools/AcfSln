# Acf
if(NOT DEFINED ACFDIR)
	set(ACFDIR "$ENV{ACFDIR}")
endif()

include(${ACFDIR}/Config/CMake/AcfEnv.cmake)

# ACF-Solutions
if(NOT DEFINED ACFSLNDIR)
	set(ACFSLNDIR "$ENV{ACFSLNDIR}")
endif()

if(DEFINED ENV{ACFSLNDIR_BUILD})
	set(ACFSLNDIR_BUILD "$ENV{ACFSLNDIR_BUILD}")
else()
	set(ACFSLNDIR_BUILD ${ACFSLNDIR})
endif()

# AuxInclude for generated files — always needed.
include_directories("${ACFSLNDIR_BUILD}/AuxInclude/${TARGETNAME}")

# Legacy mode: global include/link dirs. Skipped when ACF_MODERN_CMAKE is ON.
if(NOT ACF_MODERN_CMAKE)
	include_directories("${ACFSLNDIR}/Include")
	include_directories("${ACFSLNDIR}/Impl")
	link_directories(${ACFSLNDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME})
endif()

message(VERBOSE "AcfSln link_directories ${ACFSLNDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")

