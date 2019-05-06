add_library(factorial2kr SHARED
  ${CMAKE_CURRENT_SOURCE_DIR}/configuration.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/input.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/measure.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/object.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/stat.cc
)

#target_link_libraries(factorial2kr
#)

add_executable(main
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
)

target_link_libraries(main
  factorial2kr
)
