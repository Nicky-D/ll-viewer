# -*- cmake -*-

include(Prebuilt)

set(JSONCPP_FIND_QUIETLY ON)
set(JSONCPP_FIND_REQUIRED ON)

if (USESYSTEMLIBS)
  include(FindJsonCpp)
else (USESYSTEMLIBS)
  use_prebuilt_binary(jsoncpp)
  if (WINDOWS)
    set(JSONCPP_LIBRARIES 
      debug json_libmdd.lib
      optimized json_libmd.lib)
  elseif (DARWIN)
    set(JSONCPP_LIBRARIES libjson_darwin_libmt.a)
  elseif (LINUX)
    set(JSONCPP_LIBRARIES libjsoncpp.a)
  endif (WINDOWS)
  set(JSONCPP_INCLUDE_DIR "${LIBS_PREBUILT_DIR}/include")
endif (USESYSTEMLIBS)
