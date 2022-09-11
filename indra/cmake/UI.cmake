# -*- cmake -*-
include(Prebuilt)
include(FreeType)
include(GLIB)

if (USESYSTEMLIBS)
  include(FindPkgConfig)
    
  if (LINUX)
    set(PKGCONFIG_PACKAGES
        atk
        cairo
        gdk-2.0
        gdk-pixbuf-2.0
        glib-2.0
        gmodule-2.0
        gtk+-2.0
        gthread-2.0
        libpng
        pango
        pangoft2
        pangox
        pangoxft
        sdl
        )
  endif (LINUX)

  foreach(pkg ${PKGCONFIG_PACKAGES})
    pkg_check_modules(${pkg} REQUIRED ${pkg})
    include_directories(${${pkg}_INCLUDE_DIRS})
    link_directories(${${pkg}_LIBRARY_DIRS})
    list(APPEND UI_LIBRARIES ${${pkg}_LIBRARIES})
    add_definitions(${${pkg}_CFLAGS_OTHERS})
  endforeach(pkg)
else (USESYSTEMLIBS)
  if (LINUX)
    use_prebuilt_binary(fltk)
  endif (LINUX)

  if (LINUX)
    set(UI_LIB_NAMES
        libfltk.a
        libfreetype.a
        )

    foreach(libname ${UI_LIB_NAMES})
      find_library(UI_LIB_${libname}
                   NAMES ${libname}
                   PATHS
                     optimized ${LIBS_PREBUILT_DIR}/lib/release
                   NO_DEFAULT_PATH
        )
      set(UI_LIBRARIES ${UI_LIBRARIES} ${UI_LIB_${libname}})
    endforeach(libname)
    set(UI_LIBRARIES ${UI_LIBRARIES} Xinerama X11)
	include_directories ( ${GLIB_INCLUDE_DIRS}  )
  endif (LINUX)

  include_directories (
      ${LIBS_PREBUILT_DIR}/include
      ${LIBS_PREBUILT_DIR}/include
      )
  foreach(include ${${LL_ARCH}_INCLUDES})
      include_directories(${LIBS_PREBUILT_DIR}/include/${include})
  endforeach(include)
endif (USESYSTEMLIBS)

if (LINUX)
  add_definitions(-DLL_X11=1 -DLL_FLTK=1)
endif (LINUX)
