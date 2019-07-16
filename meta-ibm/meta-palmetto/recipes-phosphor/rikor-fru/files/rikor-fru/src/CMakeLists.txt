

set (rikor-fru_src main.c dictionary.c iniparser.c)
set (rikor-fru_libsrc rikor-fru.c crc16.c)


# include_directories($ENV{SINC})
# include_directories($ENV{SINC}/glib-2.0)
# include_directories($ENV{SLIB}/glib-2.0/include)

# find_library(GIO gio-2.0)
# find_library(GLIB glib-2.0)

add_executable (rikor-fru-util ${rikor-fru_src})

target_link_libraries(rikor-fru-util
  # ${GIO}
  # ${GLIB}
  rikor-fru
)


add_library (rikor-fru SHARED ${rikor-fru_libsrc})
set_target_properties(rikor-fru PROPERTIES PUBLIC_HEADER "rikor-fru.h")

target_link_libraries(rikor-fru
  # ${GIO}
  # ${GLIB}
)

# The rule to install binary
install (TARGETS rikor-fru-util RUNTIME DESTINATION bin)
install (TARGETS rikor-fru 
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include)
