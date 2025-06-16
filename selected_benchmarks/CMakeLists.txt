# -----------------------------------------------------------------------------
# eigen package 
# -----------------------------------------------------------------------------
if(NOT DEFINED EIGEN_ROOT)
  set(EIGEN_ROOT ${PROJECT_SOURCE_DIR}/3rd-party/eigen-3.3.7)
endif()
include_directories(${EIGEN_ROOT})

# -----------------------------------------------------------------------------
# find OpenMP package
# -----------------------------------------------------------------------------
include(FindOpenMP)

if(NOT OpenMP_CXX_FOUND)
  message(FATAL_ERROR "OpenMP not found")
endif()
  
message(STATUS "OpenMP_VERSION: ${OpenMP_VERSION}")
message(STATUS "OpenMP_CXX_FLAGS: ${OpenMP_CXX_FLAGS}")
message(STATUS "OpenMP_CXX_LIBRARIES: ${OpenMP_CXX_LIBRARIES}")

# -----------------------------------------------------------------------------
# tbb package
# -----------------------------------------------------------------------------
cmake_policy(PUSH)
cmake_policy(SET CMP0074 NEW)
if(NOT DEFINED TBB_ROOT_DIR)
  set(TBB_ROOT_DIR ${TF_3RD_PARTY_DIR}/tbb)
endif()

include(${TBB_ROOT_DIR}/cmake/TBBBuild.cmake)
#tbb_build(TBB_ROOT ${TBB_ROOT_DIR} CONFIG_DIR TBB_DIR MAKE_ARGS tbb_cpf=1)
tbb_build(TBB_ROOT ${TBB_ROOT_DIR} CONFIG_DIR TBB_DIR)
unset(TBB_ROOT)  # unset is needed to pop CMP0074 without warning
cmake_policy(POP)
#find_package(TBB REQUIRED tbb_preview)
find_package(TBB REQUIRED)
#cmake_policy(POP)

# -----------------------------------------------------------------------------
# SelectedBenchmarks
# -----------------------------------------------------------------------------


## benchmark 3: fibonacci
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}fibonacci
  ${TF_SELECTED_BENCHMARKS}/fibonacci/main.cpp
  ${TF_SELECTED_BENCHMARKS}/fibonacci/omp.cpp
  # ${TF_SELECTED_BENCHMARKS}/fibonacci/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/fibonacci/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}fibonacci PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}fibonacci
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}fibonacci PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}fibonacci PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}fibonacci PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 5: nqueens
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}nqueens
  ${TF_SELECTED_BENCHMARKS}/nqueens/main.cpp
  ${TF_SELECTED_BENCHMARKS}/nqueens/omp.cpp
  # ${TF_SELECTED_BENCHMARKS}/nqueens/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/nqueens/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}nqueens PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}nqueens
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}nqueens PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}nqueens PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}nqueens PRIVATE TF_ENABLE_TASK_POOL=1)
endif()

# ## benchmark 1: async_task
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}async_task
#   ${TF_SELECTED_BENCHMARKS}/async_task/main.cpp
#   ${TF_SELECTED_BENCHMARKS}/async_task/omp.cpp
#   ${TF_SELECTED_BENCHMARKS}/async_task/tbb.cpp
#   ${TF_SELECTED_BENCHMARKS}/async_task/std.cpp
#   ${TF_SELECTED_BENCHMARKS}/async_task/taskflow.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}async_task PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}async_task
#   ${PROJECT_NAME} 
#   ${TBB_IMPORTED_TARGETS} 
#   ${OpenMP_CXX_LIBRARIES} 
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}async_task PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})

# ## benchmark 2: binary_tree
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree
#   ${TF_SELECTED_BENCHMARKS}/binary_tree/main.cpp
#   ${TF_SELECTED_BENCHMARKS}/binary_tree/tbb.cpp
#   ${TF_SELECTED_BENCHMARKS}/binary_tree/omp.cpp
#   ${TF_SELECTED_BENCHMARKS}/binary_tree/taskflow.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree
#   ${PROJECT_NAME} 
#   ${TBB_IMPORTED_TARGETS} 
#   ${OpenMP_CXX_LIBRARIES} 
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})


# ## benchmark 4: matrix_multiplication
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication
#   ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/main.cpp
#   ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/omp.cpp
#   ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/tbb.cpp
#   ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/taskflow.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication
#   ${PROJECT_NAME} 
#   ${TBB_IMPORTED_TARGETS} 
#   ${OpenMP_CXX_LIBRARIES} 
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})

# ## benchmark 5: nqueens
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}nqueens
#   ${TF_SELECTED_BENCHMARKS}/nqueens/main.cpp
#   ${TF_SELECTED_BENCHMARKS}/nqueens/omp.cpp
#   ${TF_SELECTED_BENCHMARKS}/nqueens/tbb.cpp
#   ${TF_SELECTED_BENCHMARKS}/nqueens/taskflow.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}nqueens PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}nqueens
#   ${PROJECT_NAME} 
#   ${TBB_IMPORTED_TARGETS} 
#   ${OpenMP_CXX_LIBRARIES} 
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}nqueens PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})

# ## benchmark 6: reduce_sum
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum
#   ${TF_SELECTED_BENCHMARKS}/reduce_sum/main.cpp
#   ${TF_SELECTED_BENCHMARKS}/reduce_sum/omp.cpp
#   ${TF_SELECTED_BENCHMARKS}/reduce_sum/tbb.cpp
#   ${TF_SELECTED_BENCHMARKS}/reduce_sum/taskflow.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum
#   ${PROJECT_NAME} 
#   ${TBB_IMPORTED_TARGETS} 
#   ${OpenMP_CXX_LIBRARIES} 
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})

# ## benchmark 7: sort
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}sort
#   ${TF_SELECTED_BENCHMARKS}/sort/main.cpp
#   ${TF_SELECTED_BENCHMARKS}/sort/omp.cpp
#   ${TF_SELECTED_BENCHMARKS}/sort/tbb.cpp
#   ${TF_SELECTED_BENCHMARKS}/sort/taskflow.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}sort PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}sort
#   ${PROJECT_NAME} 
#   ${TBB_IMPORTED_TARGETS} 
#   ${OpenMP_CXX_LIBRARIES} 
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}sort PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})

# ## benchmark 8: thread_pool
# add_executable(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}thread_pool
#   ${TF_SELECTED_BENCHMARKS}/thread_pool/thread_pool.cpp
# )
# target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}thread_pool  PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
# target_link_libraries(
#   ${TF_SELECTED_BENCHMARKS_PREFIX}thread_pool
#   ${PROJECT_NAME}
#   ${TBB_IMPORTED_TARGETS}
#   ${OpenMP_CXX_LIBRARIES}
#   tf::default_settings
# )
# set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}thread_pool PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
