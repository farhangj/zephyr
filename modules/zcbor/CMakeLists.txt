if(CONFIG_ZCBOR)
  add_library(ZCBOR INTERFACE)

  zephyr_include_directories(
    ${ZEPHYR_ZCBOR_MODULE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
  )
  target_link_libraries(ZCBOR INTERFACE zephyr_interface)

  zephyr_library()
  zephyr_library_sources(
    ${ZEPHYR_ZCBOR_MODULE_DIR}/src/zcbor_common.c
    ${ZEPHYR_ZCBOR_MODULE_DIR}/src/zcbor_decode.c
    ${ZEPHYR_ZCBOR_MODULE_DIR}/src/zcbor_encode.c
  )
endif()
