include_guard()
include(FetchContent)

if( USE_SYSROOT )
  set( SYSROOT_NAME prebuild_files )
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	FetchContent_Declare( ${SYSROOT_NAME}
	  URL https://github.com/slviewer-3p/_build-all/releases/download/v0.5/sysroot.tar.bz2
	  URL_HASH MD5=ab66c5ef670c50859ee99d979a524388
	  DOWNLOAD_NO_PROGRESS ON
	  )
  endif()
  FetchContent_MakeAvailable( ${SYSROOT_NAME} )

  function (use_prebuilt_binary _binary)
  endfunction()
endif()

