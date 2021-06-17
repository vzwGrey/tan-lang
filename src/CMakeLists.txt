add_executable(tan main.c)
set_property(TARGET tan PROPERTY C_STANDARD 11)

target_include_directories(tan PRIVATE /usr/include/readline)
target_link_libraries(tan PUBLIC readline)

if(MSVC)
  target_compile_options(tan PRIVATE /W4)
else()
  target_compile_options(tan PRIVATE -Wall -Wextra -pedantic)
endif()
