#pragma once

// Feature macros for fine-tuning the performance of Taskflow at compile time
// 
// Disabled features by default:
// + TF_ENABLE_TASK_POOL       : enable task pool optimization
// + TF_ENABLE_ATOMIC_NOTIFIER : enable atomic notifier (required C++20)
//
// #define TF_USE_XQUEUE 1 // Now a cmake option, enabled with TF_USE_XQUEUE=ON

#ifdef TF_USE_XQUEUE

#pragma message "TF_USE_XQUEUE is enabled"

#define TF_ENABLE_WS 1

#ifdef TF_ENABLE_WS

#pragma message "TF_ENABLE_WS is enabled"

#define MAX_NOPS_EMPTY 40

#endif

#endif

// #define TF_ENABLE_STATS 1

#ifdef TF_ENABLE_STATS

#pragma message "TF_ENABLE_STATS is enabled"

#endif

#define TF_ENABLE_LOCKLESS 1 // slight modification to the original bounded queue.

// #define TF_ENABLE_PROFILER 1


#include "core/executor.hpp"
#include "core/runtime.hpp"
#include "core/async.hpp"
#include "algorithm/algorithm.hpp"

/**
@dir taskflow
@brief root taskflow include dir
*/

/**
@dir taskflow/core
@brief taskflow core include dir
*/

/**
@dir taskflow/algorithm
@brief taskflow algorithms include dir
*/

/**
@dir taskflow/cuda
@brief taskflow CUDA include dir
*/

/**
@file taskflow/taskflow.hpp
@brief main taskflow include file
*/



/**
@def TF_VERSION 

@brief version of the %Taskflow (currently 3.11.0)

The version system is made of a major version number, a minor version number,
and a patch number:
  + TF_VERSION % 100 is the patch level
  + TF_VERSION / 100 % 1000 is the minor version
  + TF_VERSION / 100000 is the major version
*/
#define TF_VERSION 301000

/**
@def TF_MAJOR_VERSION

@brief major version of %Taskflow, which is equal to `TF_VERSION/100000`
*/
#define TF_MAJOR_VERSION TF_VERSION/100000

/**
@def TF_MINOR_VERSION

@brief minor version of %Taskflow, which is equal to `TF_VERSION / 100 % 1000`
*/
#define TF_MINOR_VERSION TF_VERSION/100%1000

/**
@def TF_PATCH_VERSION

@brief patch version of %Taskflow, which is equal to `TF_VERSION % 100`
*/
#define TF_PATCH_VERSION TF_VERSION%100



/**
@brief taskflow namespace
*/
namespace tf {

/**
@private
*/
namespace detail { }


/**
@brief queries the version information in a string format @c major.minor.patch

Release notes are available here: https://taskflow.github.io/taskflow/Releases.html
*/
constexpr const char* version() {
  return "3.11.0";
}


}  // end of namespace tf -----------------------------------------------------





