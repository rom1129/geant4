#------------------------------------------------------------------------------
# CMakeLists.txt
# Module : G4OpenInventor
# Package: Geant4.src.G4visualization.G4OpenInventor
#
# CMakeLists.txt for single level library that may be build global or granular
#
# Generated on : 24/9/2010
#
#
#------------------------------------------------------------------------------

include(Geant4MacroLibraryTargets)
if(GEANT4_BUILD_GRANULAR_LIBS)
    GEANT4_GRANULAR_LIBRARY_TARGET(COMPONENT sources.cmake)
else()
    GEANT4_GLOBAL_LIBRARY_TARGET(COMPONENTS sources.cmake)
endif()

# Fix for Inventor not yet supporting usage requirements
if(TARGET G4OpenInventor)
  target_include_directories(G4OpenInventor PUBLIC ${G4VIS_MODULE_OPENINVENTOR_INCLUDE_DIRS})
endif()

if(TARGET G4OpenInventor-static)
  target_include_directories(G4OpenInventor-static PUBLIC ${G4VIS_MODULE_OPENINVENTOR_INCLUDE_DIRS})
endif()
