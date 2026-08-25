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

# --- Foundation / algorithms ------------------------------------------------
declare_target_dependencies(ialgo		Acf::iimg)
declare_target_dependencies(iauth		Acf::iprm)
declare_target_dependencies(iweb		Acf::iprm)
declare_target_dependencies(iprod		Acf::ibase)
declare_target_dependencies(ihotf		Acf::iprm Acf::imath)
declare_target_dependencies(isig		Acf::iprm Acf::imath)
declare_target_dependencies(imm			Acf::iimg)

# --- Processing / measurement core ------------------------------------------
declare_target_dependencies(iproc		Acf::ibase)
declare_target_dependencies(iinsp		AcfSln::iproc)
declare_target_dependencies(imeas		AcfSln::iproc Acf::iimg)
declare_target_dependencies(iwiz		AcfSln::iproc)
declare_target_dependencies(icalib		AcfSln::imeas)
declare_target_dependencies(ibarcode	AcfSln::iipr)
declare_target_dependencies(icam		AcfSln::icalib AcfSln::iinsp AcfSln::isig)
declare_target_dependencies(iipr		AcfSln::ialgo AcfSln::icam)
declare_target_dependencies(iblob		AcfSln::iipr)
declare_target_dependencies(iedge		AcfSln::iipr)
declare_target_dependencies(ifileproc	AcfSln::iinsp Acf::iimg)
declare_target_dependencies(idocproc	AcfSln::iproc Acf::iqtdoc)
declare_target_dependencies(icomm		Acf::iqt)
declare_target_dependencies(icmpstr		Acf::ipackage Acf::iqt2d Acf::iqtdoc Qt${QT_VERSION_MAJOR}::PrintSupport)

# --- Qt integration and GUI -------------------------------------------------
declare_target_dependencies(iqtex		Acf::iqtgui)
if(QT_VERSION_MAJOR EQUAL 5)
	declare_target_dependencies(iqtex	Qt${QT_VERSION_MAJOR}::XmlPatterns)
endif()

declare_target_dependencies(iqtauth		AcfSln::iauth Acf::iqtgui)
declare_target_dependencies(iqtmeas		AcfSln::iinsp AcfSln::imeas Acf::iqtgui)
declare_target_dependencies(icalibgui	AcfSln::icalib Acf::iqt2d)
declare_target_dependencies(iqtinsp		AcfSln::iauth AcfSln::icalib AcfSln::iinsp Acf::iqt2d)
declare_target_dependencies(iqtipr		AcfSln::iipr AcfSln::iqtinsp)
declare_target_dependencies(iqtcam		AcfSln::iipr AcfSln::iqtinsp)
declare_target_dependencies(iqtsig		AcfSln::imeas AcfSln::isig Acf::iqtgui)
declare_target_dependencies(iqtmm		AcfSln::icam AcfSln::imm Acf::iqtgui Qt${QT_VERSION_MAJOR}::Svg)
declare_target_dependencies(iblobgui	AcfSln::iblob AcfSln::iqtipr)
declare_target_dependencies(iedgegui	AcfSln::iedge AcfSln::iqtipr)
declare_target_dependencies(ihotfgui	AcfSln::ifileproc AcfSln::ihotf Acf::iqtgui)
declare_target_dependencies(iprocgui	AcfSln::idocproc)
declare_target_dependencies(iwizgui		AcfSln::iwiz Acf::iqtgui)

# --- Service helper ---------------------------------------------------------
declare_target_dependencies(iservice	Acf::iqtgui)

# --- Arxc-generated static libraries ----------------------------------------
declare_target_dependencies(AcfSlnLoc	Acf::AcfLoc)
