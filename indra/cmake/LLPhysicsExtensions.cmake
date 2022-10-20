# -*- cmake -*-
include(Prebuilt)

# There are three possible solutions to provide the llphysicsextensions:
# - The full source package, selected by -DHAVOK:BOOL=ON
# - The stub source package, selected by -DHAVOK:BOOL=OFF 
# - The prebuilt package available to those with sublicenses, selected by -DHAVOK_TPV:BOOL=ON

if (INSTALL_PROPRIETARY)
   set(HAVOK ON CACHE BOOL "Use Havok physics library")
endif (INSTALL_PROPRIETARY)


# Note that the use_prebuilt_binary macros below do not in fact include binaries;
# the llphysicsextensions_* packages are source only and are built here.
# The source package and the stub package both build libraries of the same name.

if (HAVOK)
   include(Havok)
   use_prebuilt_binary(llphysicsextensions_source)
   set(LLPHYSICSEXTENSIONS_SRC_DIR ${LIBS_PREBUILT_DIR}/llphysicsextensions/src)
   set(LLPHYSICSEXTENSIONS_LIBRARIES    llphysicsextensions)

elseif (HAVOK_TPV)
   use_prebuilt_binary(llphysicsextensions_tpv)
   set(LLPHYSICSEXTENSIONS_LIBRARIES    llphysicsextensions_tpv)

else (HAVOK)

  FetchContent_Declare( ndMeshDecomposition 
    GIT_REPOSITORY https://github.com/Nicky-D/ndMeshDecomposing.git
    GIT_TAG v1.0)
  FetchContent_Declare( ndNavMesh
    GIT_REPOSITORY https://github.com/Nicky-D/ndNavMesh.git
    GIT_TAG v1.0)
  FetchContent_MakeAvailable( ndMeshDecomposition ndNavMesh )

  set(LLPHYSICSEXTENSIONS_LIBRARIES ndMeshDecomposing ndNavmesh )

endif (HAVOK)

set(LLPHYSICSEXTENSIONS_INCLUDE_DIRS ${LIBS_PREBUILT_DIR}/include/llphysicsextensions)
