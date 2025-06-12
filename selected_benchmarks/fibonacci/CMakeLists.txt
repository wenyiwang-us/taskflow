add_executable(fibonacci main.cpp)
target_link_libraries(fibonacci ${PROJECT_NAME} ${ATOMIC_LIBRARY} tf::default_settings)

# Add debug flags
target_compile_options(fibonacci PRIVATE
  $<$<CONFIG:Debug>:-g -O0 -fsanitize=address -fno-omit-frame-pointer>
)

# Link with ASan in debug mode
target_link_options(fibonacci PRIVATE
  $<$<CONFIG:Debug>:-fsanitize=address>
) 
