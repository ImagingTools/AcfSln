# ---------------------------------------------------------------------------
# Clean, target-based inter-library dependency graph for AcfSln.
#
# This mirrors the approach introduced for the ACF foundation (Acf) in
# Config/CMake/AcfLibraryDependencies.cmake: instead of relying on the final
# executable link to resolve symbols and on a hand-tuned build order, the
# dependencies between the AcfSln libraries - and their dependencies onto the
# underlying Acf:: libraries - are declared here as target usage requirements.
# Include paths and link order then propagate transitively and automatically,
# both for the in-tree build and for downstream consumers that use
# find_package(AcfSln) and link a single AcfSln::<lib> target.
#
# The plain target_link_libraries() signature is used deliberately: the rest of
# the AcfSln/Acf CMake files use the plain signature too, and CMake forbids
# mixing the plain and keyword signatures on the same target. For static
# libraries the plain signature still records the dependency in the target's
# link interface, so it propagates transitively to consumers.
#
# The dependencies below are derived from the #include graph of each library.
# Dependencies onto the underlying ACF foundation are expressed through the
# Acf::<lib> imported targets published by find_package(Acf); entries whose
# target does not exist in the current configuration (for example when Acf is
# still consumed through the legacy environment-variable shim rather than
# find_package) are silently ignored.
#
# Included once, centrally, from Build/CMake/CMakeLists.txt after all library
# targets have been created.
# ---------------------------------------------------------------------------

# Declare the dependencies of an AcfSln library, ignoring any entry whose target
# does not exist in the current configuration (for example platform-specific or
# feature-gated libraries, or Acf:: targets that are not available because the
# legacy shim is used instead of find_package(Acf)).
function(acfsln_declare_library_dependencies target)
	if(NOT TARGET ${target})
		return()
	endif()

	foreach(dependency IN LISTS ARGN)
		if(TARGET ${dependency})
			target_link_libraries(${target} ${ACF_LIBRARY_LINK_SCOPE} ${dependency})
		endif()
	endforeach()
endfunction()

# --- Foundation / algorithms ------------------------------------------------
acfsln_declare_library_dependencies(ialgo		Acf::iimg)
acfsln_declare_library_dependencies(iauth		Acf::iprm)
acfsln_declare_library_dependencies(iweb		Acf::iprm)
acfsln_declare_library_dependencies(iprod		Acf::ibase)
acfsln_declare_library_dependencies(ihotf		Acf::iprm Acf::imath)
acfsln_declare_library_dependencies(isig		Acf::iprm Acf::imath)
acfsln_declare_library_dependencies(imm			Acf::iimg)

# --- Processing / measurement core ------------------------------------------
acfsln_declare_library_dependencies(iproc		Acf::ibase)
acfsln_declare_library_dependencies(iinsp		AcfSln::iproc)
acfsln_declare_library_dependencies(imeas		AcfSln::iproc Acf::iimg)
acfsln_declare_library_dependencies(iwiz		AcfSln::iproc)
acfsln_declare_library_dependencies(icalib		AcfSln::imeas)
acfsln_declare_library_dependencies(ibarcode	AcfSln::iipr)
acfsln_declare_library_dependencies(icam		AcfSln::icalib AcfSln::iinsp AcfSln::isig)
acfsln_declare_library_dependencies(iipr		AcfSln::ialgo AcfSln::icam)
acfsln_declare_library_dependencies(iblob		AcfSln::iipr)
acfsln_declare_library_dependencies(iedge		AcfSln::iipr)
acfsln_declare_library_dependencies(ifileproc	AcfSln::iinsp Acf::iimg)
acfsln_declare_library_dependencies(idocproc	AcfSln::iproc Acf::iqtdoc)
acfsln_declare_library_dependencies(icomm		Acf::iqt)
acfsln_declare_library_dependencies(icmpstr		Acf::ipackage Acf::iqt2d Acf::iqtdoc Qt${QT_VERSION_MAJOR}::PrintSupport)

# --- Qt integration and GUI -------------------------------------------------
acfsln_declare_library_dependencies(iqtex		Acf::iqtgui)
if(QT_VERSION_MAJOR EQUAL 5)
	acfsln_declare_library_dependencies(iqtex	Qt${QT_VERSION_MAJOR}::XmlPatterns)
endif()

acfsln_declare_library_dependencies(iqtauth		AcfSln::iauth Acf::iqtgui)
acfsln_declare_library_dependencies(iqtmeas		AcfSln::iinsp AcfSln::imeas Acf::iqtgui)
acfsln_declare_library_dependencies(icalibgui	AcfSln::icalib Acf::iqt2d)
acfsln_declare_library_dependencies(iqtinsp		AcfSln::iauth AcfSln::icalib AcfSln::iinsp Acf::iqt2d)
acfsln_declare_library_dependencies(iqtipr		AcfSln::iipr AcfSln::iqtinsp)
acfsln_declare_library_dependencies(iqtcam		AcfSln::iipr AcfSln::iqtinsp)
acfsln_declare_library_dependencies(iqtsig		AcfSln::imeas AcfSln::isig Acf::iqtgui)
acfsln_declare_library_dependencies(iqtmm		AcfSln::icam AcfSln::imm Acf::iqtgui Qt${QT_VERSION_MAJOR}::Svg)
acfsln_declare_library_dependencies(iblobgui	AcfSln::iblob AcfSln::iqtipr)
acfsln_declare_library_dependencies(iedgegui	AcfSln::iedge AcfSln::iqtipr)
acfsln_declare_library_dependencies(ihotfgui	AcfSln::ifileproc AcfSln::ihotf Acf::iqtgui)
acfsln_declare_library_dependencies(iprocgui	AcfSln::idocproc)
acfsln_declare_library_dependencies(iwizgui		AcfSln::iwiz Acf::iqtgui)

# --- Service helper ---------------------------------------------------------
acfsln_declare_library_dependencies(iservice	Acf::iqtgui)

# --- Arxc-generated static libraries ----------------------------------------
acfsln_declare_library_dependencies(AcfSlnLoc	Acf::icomp)
