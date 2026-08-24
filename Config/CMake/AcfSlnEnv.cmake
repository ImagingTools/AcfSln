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

if(NOT ACF_MODERN_CMAKE)
	# Legacy mode: global include/link dirs. Skipped when ACF_MODERN_CMAKE is ON.
	include_directories("${ACFSLNDIR}/Include")
	include_directories("${ACFSLNDIR}/Impl")
	link_directories(${ACFSLNDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME})
elseif(NOT TARGET Acf::istd)
	# Discover the Acf package published by the Acf build tree.
	# In a composite build (Acf and AcfSln in the same CMake tree),
	# Acf:: alias targets are already visible - skip find_package to avoid requiring
	# the not-yet-generated AcfTargets.cmake export file.
	if(NOT DEFINED ACFDIR_BUILD)
		set(ACFDIR_BUILD "${ACFDIR}")
	endif()

	set(Acf_DIR "${ACFDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake")
	message(VERBOSE "AcfSln: find_package(Acf) from ${Acf_DIR}")

	find_package(Acf REQUIRED GLOBAL)
endif()

message(VERBOSE "AcfSln link_directories ${ACFSLNDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")

