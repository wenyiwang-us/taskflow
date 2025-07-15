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


## benchmark 1: fibonacci
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


## benchmark 2: nqueens
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

## benchmark 3: async_task
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}async_task
  ${TF_SELECTED_BENCHMARKS}/async_task/main.cpp
  ${TF_SELECTED_BENCHMARKS}/async_task/omp.cpp
  # ${TF_SELECTED_BENCHMARKS}/async_task/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/async_task/std.cpp
  ${TF_SELECTED_BENCHMARKS}/async_task/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}async_task PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}async_task
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}async_task PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}async_task PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}async_task PRIVATE TF_ENABLE_TASK_POOL=1)
endif()

## benchmark 4: skynet
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}skynet
  ${TF_SELECTED_BENCHMARKS}/skynet/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/skynet/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/skynet/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/skynet/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}skynet PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}skynet
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}skynet PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}skynet PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}skynet PRIVATE TF_ENABLE_TASK_POOL=1)
endif()

## benchmark 5: integrate
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}integrate
  ${TF_SELECTED_BENCHMARKS}/integrate/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/integrate/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/integrate/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/integrate/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}integrate PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}integrate
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}integrate PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}integrate PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}integrate PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 6: binary_tree
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree
  ${TF_SELECTED_BENCHMARKS}/binary_tree/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/binary_tree/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/binary_tree/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/binary_tree/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}binary_tree PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 7: black_scholes
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}black_scholes
  ${TF_SELECTED_BENCHMARKS}/black_scholes/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/black_scholes/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/black_scholes/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/black_scholes/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}black_scholes PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}black_scholes
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}black_scholes PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}black_scholes PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}black_scholes PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 8: data_pipeline
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}data_pipeline
  ${TF_SELECTED_BENCHMARKS}/data_pipeline/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/data_pipeline/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/data_pipeline/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}data_pipeline PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}data_pipeline
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}data_pipeline PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}data_pipeline PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}data_pipeline PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 9: deferred_pipeline
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}deferred_pipeline
  ${TF_SELECTED_BENCHMARKS}/deferred_pipeline/main.cpp
  ${TF_SELECTED_BENCHMARKS}/deferred_pipeline/pthread.cpp
  ${TF_SELECTED_BENCHMARKS}/deferred_pipeline/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}deferred_pipeline PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}deferred_pipeline
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}deferred_pipeline PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})  
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}deferred_pipeline PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}deferred_pipeline PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 10: for_each
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}for_each
  ${TF_SELECTED_BENCHMARKS}/for_each/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/for_each/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/for_each/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/for_each/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}for_each PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}for_each
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}for_each PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}for_each PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}for_each PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 11: graph_pipeline
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}graph_pipeline
  ${TF_SELECTED_BENCHMARKS}/graph_pipeline/main.cpp
  ${TF_SELECTED_BENCHMARKS}/graph_pipeline/omp.cpp
  # ${TF_SELECTED_BENCHMARKS}/graph_pipeline/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/graph_pipeline/taskflow.cpp
  ${TF_SELECTED_BENCHMARKS}/graph_pipeline/gold.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}graph_pipeline PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}graph_pipeline
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}graph_pipeline PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}graph_pipeline PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}graph_pipeline PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 12: graph_traversal
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}graph_traversal
  ${TF_SELECTED_BENCHMARKS}/graph_traversal/main.cpp
  ${TF_SELECTED_BENCHMARKS}/graph_traversal/omp.cpp
  # ${TF_SELECTED_BENCHMARKS}/graph_traversal/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/graph_traversal/seq.cpp
  ${TF_SELECTED_BENCHMARKS}/graph_traversal/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}graph_traversal PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}graph_traversal
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}graph_traversal PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}graph_traversal PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}graph_traversal PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 13: linear_chain
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}linear_chain
  ${TF_SELECTED_BENCHMARKS}/linear_chain/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/linear_chain/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/linear_chain/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/linear_chain/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}linear_chain PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)  
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}linear_chain
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}linear_chain PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}linear_chain PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}linear_chain PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 14: linear_pipeline
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}linear_pipeline
  ${TF_SELECTED_BENCHMARKS}/linear_pipeline/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/linear_pipeline/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/linear_pipeline/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/linear_pipeline/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}linear_pipeline PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)   
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}linear_pipeline
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
) 
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}linear_pipeline PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}linear_pipeline PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}linear_pipeline PRIVATE TF_ENABLE_TASK_POOL=1)
endif() 

## benchmark 15: mandelbrot
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}mandelbrot
  ${TF_SELECTED_BENCHMARKS}/mandelbrot/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/mandelbrot/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/mandelbrot/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/mandelbrot/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}mandelbrot PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}mandelbrot
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
) 
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}mandelbrot PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}mandelbrot PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}mandelbrot PRIVATE TF_ENABLE_TASK_POOL=1)
endif()

## benchmark 16: matrix_multiplication
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication
  ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/matrix_multiplication/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}matrix_multiplication PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 17: mnist
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}mnist
  ${TF_SELECTED_BENCHMARKS}/mnist/main.cpp
  ${TF_SELECTED_BENCHMARKS}/mnist/omp.cpp
  # ${TF_SELECTED_BENCHMARKS}/mnist/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/mnist/seq.cpp
  ${TF_SELECTED_BENCHMARKS}/mnist/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}mnist PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}mnist
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES}
  stdc++fs
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}mnist PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})   
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}mnist PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}mnist PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 18: primes
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}primes
  ${TF_SELECTED_BENCHMARKS}/primes/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/primes/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/primes/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/primes/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}primes PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)  
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}primes
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
) 
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}primes PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}primes PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}primes PRIVATE TF_ENABLE_TASK_POOL=1)
endif() 


## benchmark 19: scan
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}scan
  ${TF_SELECTED_BENCHMARKS}/scan/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/scan/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/scan/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/scan/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}scan PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)  
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}scan
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
) 
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}scan PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})    
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}scan PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}scan PRIVATE TF_ENABLE_TASK_POOL=1)
endif()   


## benchmark 20: wavefront
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}wavefront
  ${TF_SELECTED_BENCHMARKS}/wavefront/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/wavefront/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/wavefront/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/wavefront/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}wavefront PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)   
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}wavefront
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}wavefront PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}wavefront PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}wavefront PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 21: reduce_sum
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum
  ${TF_SELECTED_BENCHMARKS}/reduce_sum/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/reduce_sum/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/reduce_sum/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/reduce_sum/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})  
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}reduce_sum PRIVATE TF_ENABLE_TASK_POOL=1)
endif()


## benchmark 22: sort
add_executable(
  ${TF_SELECTED_BENCHMARKS_PREFIX}sort
  ${TF_SELECTED_BENCHMARKS}/sort/main.cpp
  # ${TF_SELECTED_BENCHMARKS}/sort/tbb.cpp
  ${TF_SELECTED_BENCHMARKS}/sort/omp.cpp
  ${TF_SELECTED_BENCHMARKS}/sort/taskflow.cpp
)
target_include_directories(${TF_SELECTED_BENCHMARKS_PREFIX}sort PRIVATE ${PROJECT_SOURCE_DIR}/3rd-party/CLI11)
target_link_libraries(
  ${TF_SELECTED_BENCHMARKS_PREFIX}sort
  ${PROJECT_NAME} 
  ${TBB_IMPORTED_TARGETS} 
  ${OpenMP_CXX_LIBRARIES} 
  tf::default_settings
)
set_target_properties(${TF_SELECTED_BENCHMARKS_PREFIX}sort PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})  
if(TF_USE_XQUEUE)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}sort PRIVATE TF_USE_XQUEUE=1)
endif()
if(TF_ENABLE_TASK_POOL)
  target_compile_definitions(${TF_SELECTED_BENCHMARKS_PREFIX}sort PRIVATE TF_ENABLE_TASK_POOL=1)
endif()






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
