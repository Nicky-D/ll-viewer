include_guard()
include(FetchContent)

if( USE_SYSROOT )
  set( SYSROOT_NAME prebuild_files )
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	FetchContent_Declare( ${SYSROOT_NAME}
	  URL https://github.com/slviewer-3p/_build-all/releases/download/v0.7/sysroot.tar.xz
	  URL_HASH MD5=444c4da625fcc36591702f7c18b2da7a
	  DOWNLOAD_NO_PROGRESS ON
	  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	  )
  endif()
  FetchContent_MakeAvailable( ${SYSROOT_NAME} )

  function (use_prebuilt_binary _binary)
  endfunction()
endif()

