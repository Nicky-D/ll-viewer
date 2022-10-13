include_guard()
include(FetchContent)

# Only available with cmake > 3.24.0
# sets DOWNLOAD_EXTRACT_TIMESTAMP to FALSE, setting the timestamp of extracted files to time
# of extraction rather than using the time that is recorded in the archive
if( POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if( USE_SYSROOT )
  set( SYSROOT_NAME prebuild_files )
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	FetchContent_Declare( ${SYSROOT_NAME}
	  URL https://github.com/slviewer-3p/_build-all/releases/download/v0.10/sysroot.tar.xz
	  URL_HASH MD5=c5fde2e176ef7bd6ed5c366ce842be54
	  )
  endif()
  FetchContent_MakeAvailable( ${SYSROOT_NAME} )

  FetchContent_Declare( ndMeshDecomposition 
    GIT_REPOSITORY https://github.com/Nicky-D/ndMeshDecomposing.git
    GIT_TAG v1.0)
  FetchContent_Declare( ndNavMesh
    GIT_REPOSITORY https://github.com/Nicky-D/ndNavMesh.git
    GIT_TAG v1.0)
  FetchContent_MakeAvailable( ndMeshDecomposition ndNavMesh )
  
  
  function (use_prebuilt_binary _binary)
  endfunction()
endif()
