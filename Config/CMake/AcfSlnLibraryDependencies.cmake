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
acfsln_declare_library_dependencies(ialgo		Acf::i2d Acf::iimg Acf::imath Acf::istd)
acfsln_declare_library_dependencies(iauth		Acf::icomp Acf::imod Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iweb		Acf::iprm Acf::istd)
acfsln_declare_library_dependencies(iprod		Acf::ibase Acf::ifile Acf::ilog Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(ihotf		Acf::icomp Acf::imath Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(isig		Acf::icomp Acf::imath Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(imm			Acf::ibase Acf::ifile Acf::iimg Acf::istd)

# --- Processing / measurement core ------------------------------------------
acfsln_declare_library_dependencies(iproc		Acf::ibase Acf::icomp Acf::ifile Acf::ilog Acf::imod Acf::iprm Acf::istd)
acfsln_declare_library_dependencies(iinsp		AcfSln::iproc Acf::icomp Acf::ilog Acf::imod Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(imeas		AcfSln::iproc Acf::icomp Acf::ifile Acf::iimg Acf::imath Acf::imod Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iwiz		AcfSln::iproc Acf::icomp Acf::imod Acf::iprm Acf::istd)
acfsln_declare_library_dependencies(icalib		AcfSln::imeas Acf::i2d Acf::icomp Acf::iimg Acf::imath Acf::imod Acf::iser Acf::istd)
acfsln_declare_library_dependencies(ibarcode	AcfSln::iipr Acf::i2d Acf::imath Acf::iser Acf::istd)
acfsln_declare_library_dependencies(icam		AcfSln::icalib AcfSln::iinsp AcfSln::imeas AcfSln::iproc AcfSln::isig Acf::i2d Acf::icomp Acf::idoc Acf::ifile Acf::iimg Acf::ilog Acf::imath Acf::imod Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iipr		AcfSln::ialgo AcfSln::icalib AcfSln::icam AcfSln::iinsp AcfSln::imeas AcfSln::iproc Acf::i2d Acf::ibase Acf::icmm Acf::icomp Acf::iimg Acf::ilog Acf::imath Acf::imod Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iblob		AcfSln::iipr AcfSln::imeas AcfSln::iproc Acf::i2d Acf::icomp Acf::iimg Acf::ilog Acf::imath Acf::imod Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iedge		AcfSln::icalib AcfSln::iinsp AcfSln::iipr AcfSln::imeas AcfSln::iproc Acf::i2d Acf::ibase Acf::icomp Acf::iimg Acf::ilog Acf::imath Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(ifileproc	AcfSln::iinsp AcfSln::iproc Acf::i2d Acf::ibase Acf::icomp Acf::ifile Acf::iimg Acf::ilog Acf::imath Acf::iprm Acf::iser Acf::istd)
acfsln_declare_library_dependencies(idocproc	AcfSln::iproc Acf::icomp Acf::idoc Acf::iimg Acf::imod Acf::iprm Acf::iqtdoc Acf::iqtgui Acf::istd)
acfsln_declare_library_dependencies(icomm		Acf::ibase Acf::icomp Acf::ifile Acf::ilog Acf::imod Acf::iprm Acf::iqt Acf::iser Acf::istd)
acfsln_declare_library_dependencies(icmpstr		Acf::i2d Acf::ibase Acf::icomp Acf::idoc Acf::ifile Acf::ilog Acf::imod Acf::ipackage Acf::iqt Acf::iqt2d Acf::iqtdoc Acf::iqtgui Acf::iser Acf::istd Acf::iwidgets Qt${QT_VERSION_MAJOR}::PrintSupport)

# --- Qt integration and GUI -------------------------------------------------
acfsln_declare_library_dependencies(iqtex		Acf::ifile Acf::imod Acf::iqt Acf::iqtgui Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iqtauth		AcfSln::iauth Acf::ilog Acf::imod Acf::iqt Acf::iqtgui Acf::iser Acf::istd)
acfsln_declare_library_dependencies(iqtmeas		AcfSln::iinsp AcfSln::imeas Acf::imath Acf::imod Acf::iqt Acf::iqtgui Acf::istd)
acfsln_declare_library_dependencies(icalibgui	AcfSln::icalib Acf::i2d Acf::imod Acf::iqt Acf::iqt2d Acf::istd Acf::iview)
acfsln_declare_library_dependencies(iqtinsp		AcfSln::iauth AcfSln::icalib AcfSln::iinsp Acf::i2d Acf::ibase Acf::icomp Acf::ifile Acf::ilog Acf::imod Acf::iprm Acf::iqt Acf::iqt2d Acf::iqtgui Acf::iser Acf::istd Acf::iview)
acfsln_declare_library_dependencies(iqtipr		AcfSln::iipr AcfSln::imeas AcfSln::iqtinsp Acf::i2d Acf::ifile Acf::iimg Acf::imath Acf::imod Acf::iprm Acf::iqt Acf::iqtgui Acf::iser Acf::istd Acf::iview)
acfsln_declare_library_dependencies(iqtcam		AcfSln::icam AcfSln::iinsp AcfSln::iipr AcfSln::iproc AcfSln::iqtinsp Acf::i2d Acf::icmm Acf::ifile Acf::iimg Acf::ilog Acf::imath Acf::imod Acf::iprm Acf::iqt Acf::iqt2d Acf::iqtgui Acf::istd Acf::iview)
acfsln_declare_library_dependencies(iqtsig		AcfSln::imeas AcfSln::iproc AcfSln::isig Acf::ifile Acf::imod Acf::iprm Acf::iqt Acf::iqtgui Acf::istd)
acfsln_declare_library_dependencies(iqtmm		AcfSln::icam AcfSln::imm AcfSln::iproc Acf::ibase Acf::icomp Acf::ifile Acf::iimg Acf::ilog Acf::imod Acf::iqt Acf::iqtgui Acf::istd Qt${QT_VERSION_MAJOR}::Svg)
acfsln_declare_library_dependencies(iblobgui	AcfSln::iblob AcfSln::iipr AcfSln::iqtinsp AcfSln::iqtipr Acf::i2d Acf::imath Acf::imod Acf::iqt Acf::iqtgui Acf::istd Acf::iview)
acfsln_declare_library_dependencies(iedgegui	AcfSln::iedge AcfSln::iqtinsp AcfSln::iqtipr Acf::icmm Acf::ifile Acf::imath Acf::imod Acf::iqtgui Acf::iview)
acfsln_declare_library_dependencies(ihotfgui	AcfSln::ifileproc AcfSln::ihotf Acf::icomp Acf::ifile Acf::ilog Acf::imod Acf::iprm Acf::iqt Acf::iqtgui Acf::istd Acf::iwidgets)
acfsln_declare_library_dependencies(iprocgui	AcfSln::idocproc AcfSln::iproc Acf::ibase Acf::icomp Acf::idoc Acf::ifile Acf::ilog Acf::imod Acf::iprm Acf::iqt Acf::iqtdoc Acf::iqtgui Acf::istd)
acfsln_declare_library_dependencies(iwizgui		AcfSln::iwiz Acf::iqt Acf::iqtgui Acf::istd)

# --- Service helper ---------------------------------------------------------
acfsln_declare_library_dependencies(iservice	Acf::ibase Acf::icomp Acf::ilog Acf::iqtgui)

# --- Arxc-generated static libraries ----------------------------------------
acfsln_declare_library_dependencies(AcfSlnLoc	Acf::ipackage)
