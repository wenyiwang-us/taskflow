#pragma once

#include "declarations.hpp"
#include "tsq.hpp"
#include "atomic_notifier.hpp"
#include "nonblocking_notifier.hpp"


/**
@file worker.hpp
@brief worker include file
*/

namespace tf {

// ----------------------------------------------------------------------------
// Default Notifier
// ----------------------------------------------------------------------------


/**
@private
*/
#ifdef TF_ENABLE_ATOMIC_NOTIFIER
  using DefaultNotifier = AtomicNotifier;
#elif TF_ENABLE_NONBLOCKING_NOTIFIER_V1
  using DefaultNotifier = NonblockingNotifierV1;
#elif TF_ENABLE_NONBLOCKING_NOTIFIER_V2
  using DefaultNotifier = NonblockingNotifierV2;
#else
  #if __cplusplus >= TF_CPP20
    using DefaultNotifier = AtomicNotifier;
  #else
    using DefaultNotifier = NonblockingNotifierV2;
  #endif
#endif

// ----------------------------------------------------------------------------
// Class Definition: Worker
// ----------------------------------------------------------------------------

/**
@class Worker

@brief class to create a worker in an executor

The class is primarily used by the executor to perform work-stealing algorithm.
Users can access a worker object and alter its property
(e.g., changing the thread affinity in a POSIX-like system)
using tf::WorkerInterface.
*/

class Worker {

  friend class Executor;
  friend class Runtime;
  friend class WorkerView;

  public:
    /**
    @brief queries the worker id associated with its parent executor

    A worker id is a unsigned integer in the range <tt>[0, N)</tt>,
    where @c N is the number of workers spawned at the construction
    time of the executor.
    */
    inline size_t id() const { return _id; }

    /**
    @brief queries the size of the queue (i.e., number of enqueued tasks to
           run) associated with the worker
    */

    #if !TF_USE_XQUEUE
    inline size_t queue_size() const { return _wsq.size(); }
    #endif // TF_USE_XQUEUE
    
    /**
    @brief queries the current capacity of the queue
    */
    #if TF_USE_XQUEUE
    inline size_t queue_capacity() const { return static_cast<size_t>(_xq->capacity()); }
    #else
    inline size_t queue_capacity() const { return static_cast<size_t>(_wsq.capacity()); }
    #endif // TF_USE_XQUEUE
    
    /**
    @brief acquires the associated executor
    */
    inline Executor* executor() { return _executor; }

    /**
    @brief acquires the associated thread
    */
    std::thread& thread() { return _thread; }
    
#if TF_USE_XQUEUE

    inline void xq_init(const size_t nworkers, const size_t worker_id) {
      TF_DEBUG(_id, "xq_init, size: %ld", nworkers);
      _xq = new BoundedXQueue<Node*>(nworkers, worker_id);
    }

    inline void xq_destroy() {
      delete _xq;
    }

    ~Worker() {
      if(_xq) {
        xq_destroy();
      }
    }

    BoundedXQueue<Node*> *_xq {nullptr};
#endif // TF_USE_XQUEUE

  private:
  
  #if __cplusplus >= TF_CPP20
    std::atomic_flag _done = ATOMIC_FLAG_INIT; 
  #else
    std::atomic<bool> _done {false};
  #endif

    size_t _id;
    size_t _vtm;
    Executor* _executor {nullptr};
    DefaultNotifier::Waiter* _waiter;
    std::thread _thread;
    
    std::default_random_engine _rdgen;
    //std::uniform_int_distribution<size_t> _udist;
#if !TF_USE_XQUEUE
    BoundedTaskQueue<Node*> _wsq;
#endif // !TF_USE_XQUEUE

#if TF_ENABLE_STATS
    uint64_t nexec_from_self{0};
    uint64_t nexec_from_remote {0};
    uint64_t nexec_from_executor {0};
#endif

    //TF_FORCE_INLINE size_t _rdvtm() {
    //  auto r = _udist(_rdgen);
    //  return r + (r >= _id);
    //}

};

// ----------------------------------------------------------------------------
// Per-thread
// ----------------------------------------------------------------------------

namespace pt {

/**
@private
*/
  inline thread_local Worker* this_worker {nullptr};

// #ifdef TF_ENABLE_STATS
  // inline thread_local uint64_t ntasks_pushed_self {0};
  // inline thread_local uint64_t ntasks_pushed_remote {0};
  // inline thread_local uint64_t ntasks_not_pushed {0}; // full queue


  // inline thread_local uint64_t ntasks_popped_self {0};
  // inline thread_local uint64_t ntasks_popped_remote {0};
  // inline thread_local uint64_t ntasks_not_popped {0}; // empty queue

  // inline thread_local uint64_t ntasks_created {0};
  // inline thread_local uint64_t ntasks_executed_self {0};
  // inline thread_local uint64_t ntasks_executed_remote {0};
// #endif // TF_ENABLE_STATS
}

#if TF_USE_XQUEUE
// ----------------------------------------------------------------------------
// XQueue Functions Definitions
// ----------------------------------------------------------------------------

// TODO:
// []: Try if inline works as other functions in tf are inlined
// #if TF_IMPL_XQUEUE
template <typename T, size_t LogSize>
BoundedXQueue<T, LogSize>::BoundedXQueue(const size_t nworkers, const size_t worker_id) : _nworkers(nworkers), _worker_id(worker_id) {
  _dequeues = new XDequeue[_nworkers];
  for (int64_t i = 0; i < _nworkers; i++) {
    _dequeues[i].head = 0;
    _dequeues[i].tail = 0;
  }
}
template <typename T, size_t LogSize>
BoundedXQueue<T, LogSize>::~BoundedXQueue() {
  delete [] _dequeues;
}

template <typename T, size_t LogSize>
TaskQueueCode BoundedXQueue<T, LogSize>::push(T item) {
  int num_tries = 0;
  size_t target_worker_id = _worker_id + _last_q;
  target_worker_id = (target_worker_id > _nworkers - 1)
                          ? (target_worker_id - _nworkers)
                          : target_worker_id;
  
  Worker& target_worker = (*_workers)[target_worker_id];
  TF_DEBUG(_worker_id, "pushing task %p to worker[%ld]=%p", item, target_worker_id, &target_worker);
  while (target_worker._xq->_dequeues[_last_q]
              .dequeue[target_worker._xq->_dequeues[_last_q].head] !=
          nullptr) {
    num_tries++;
    if (num_tries < 25) {
      continue;
    }
    TF_DEBUG(_worker_id, "task not pushed: %p", item);
    #ifdef TF_ENABLE_STATS
    ntasks_not_pushed++;
    #endif // TF_ENABLE_STATS
    return TaskQueueCode::TASK_NOT_PUSHED;
  }
  auto& target_dequeue = target_worker._xq->_dequeues[_last_q];
  target_dequeue.dequeue[target_dequeue.head] = item;
  target_dequeue.head = (target_dequeue.head + 1) & DequeueMask;
  target_worker._xq->_last_q_accessed = _last_q;

  #ifdef TF_ENABLE_STATS
  if(_last_q == 0){
    ntasks_pushed_self++;
  }else{
    ntasks_pushed_remote++;
  }
  #endif // TF_ENABLE_STATS

  if(_last_q + 1 < _nworkers) {
    _last_q++;
  }else{
    _last_q = 0;
  }
  TF_DEBUG(_worker_id, "task pushed: %p", item);
  return tf::TaskQueueCode::TASK_PUSHED;
}

template <typename T, size_t LogSize>
T BoundedXQueue<T, LogSize>::pop(size_t &last_qid) {
  // TF_DEBUG(_worker_id, "pop task");
  T item {nullptr};
  // First, pop tasks from my own master queue
  if (_dequeues[0].dequeue[_dequeues[0].tail] != nullptr) {
    item = _dequeues[0].dequeue[_dequeues[0].tail];
    _dequeues[0].dequeue[_dequeues[0].tail] = nullptr;
    _dequeues[0].tail = (_dequeues[0].tail + 1) & DequeueMask;
    #ifdef TF_ENABLE_STATS
    ntasks_popped_self++;
    #endif // TF_ENABLE_STATS
    TF_DEBUG(_worker_id, "pop task: %p", item);
    return item;
  }

  // Then, pop tasks from the last accessed queue
  if (_last_q_accessed > 0) {
    auto& target_dequeue = _dequeues[_last_q_accessed];
    if (target_dequeue.dequeue[target_dequeue.tail] != nullptr) {
      item = target_dequeue.dequeue[target_dequeue.tail];
      target_dequeue.dequeue[target_dequeue.tail] = nullptr;
      target_dequeue.tail = (target_dequeue.tail + 1) & DequeueMask;
      last_qid = _last_q_accessed;
      #ifdef TF_ENABLE_STATS
      if(last_qid == 0){
        ntasks_popped_self++;
      }else{
        ntasks_popped_remote++;
      }
      #endif // TF_ENABLE_STATS
      TF_DEBUG(_worker_id, "pop task: %p", item);
      return item;
    }
  }

  // Then try pop from the last queue
  for (size_t qid = _nworkers - 1; qid > 0; qid--) {
    auto& target_dequeue = _dequeues[qid];
    if (target_dequeue.dequeue[target_dequeue.tail] != nullptr) {
      item = target_dequeue.dequeue[target_dequeue.tail];
      target_dequeue.dequeue[target_dequeue.tail] = nullptr;
      target_dequeue.tail = (target_dequeue.tail + 1) & DequeueMask;
      last_qid = qid;
      TF_DEBUG(_worker_id, "pop task: %p", item);
      #ifdef TF_ENABLE_STATS
      if(last_qid == 0){
        ntasks_popped_self++;
      }else{
        ntasks_popped_remote++;
      }
      #endif // TF_ENABLE_STATS
      return item;
    }
  }
  // Then try to pop from the rest of the queues
  for (size_t qid = _nworkers - 1; qid > last_qid; qid--) {
    auto& target_dequeue = _dequeues[qid];
    if (target_dequeue.dequeue[target_dequeue.tail] != nullptr) {
      item = target_dequeue.dequeue[target_dequeue.tail];
      target_dequeue.dequeue[target_dequeue.tail] = nullptr;
      target_dequeue.tail = (target_dequeue.tail + 1) & DequeueMask;
      last_qid = qid;
      TF_DEBUG(_worker_id, "pop task: %p", item);
      #ifdef TF_ENABLE_STATS
      if(last_qid == 0){
        ntasks_popped_self++;
      }else{
        ntasks_popped_remote++;
      }
      #endif // TF_ENABLE_STATS
      return item;
    }
  }
  if(item != nullptr) {
    TF_DEBUG(_worker_id, "pop task: %p", item);
  }
  #ifdef TF_ENABLE_STATS
  ntasks_not_popped++;
  #endif // TF_ENABLE_STATS
  return item; // which is a nullptr
}
// #endif // TF_IMPL_XQUEUE

#endif // TF_USE_XQUEUE



// ----------------------------------------------------------------------------
// Class Definition: WorkerView
// ----------------------------------------------------------------------------

/**
@class WorkerView

@brief class to create an immutable view of a worker 

An executor keeps a set of internal worker threads to run tasks.
A worker view provides users an immutable interface to observe
when a worker runs a task, and the view object is only accessible
from an observer derived from tf::ObserverInterface.
*/
class WorkerView {

  friend class Executor;

  public:

    /**
    @brief queries the worker id associated with its parent executor

    A worker id is a unsigned integer in the range <tt>[0, N)</tt>,
    where @c N is the number of workers spawned at the construction
    time of the executor.
    */
    size_t id() const;

    /**
    @brief queries the size of the queue (i.e., number of pending tasks to
           run) associated with the worker
    */
    size_t queue_size() const;

    /**
    @brief queries the current capacity of the queue
    */
    size_t queue_capacity() const;

  private:

    WorkerView(const Worker&);
    WorkerView(const WorkerView&) = default;

    const Worker& _worker;

};

// Constructor
inline WorkerView::WorkerView(const Worker& w) : _worker{w} {
}

// function: id
inline size_t WorkerView::id() const {
  return _worker._id;
}

#if !TF_USE_XQUEUE
// Function: queue_size
inline size_t WorkerView::queue_size() const {
  return _worker._wsq.size();
}

// Function: queue_capacity
inline size_t WorkerView::queue_capacity() const {
  return static_cast<size_t>(_worker._wsq.capacity());
}
#endif // !TF_USE_XQUEUE

// ----------------------------------------------------------------------------
// Class Definition: WorkerInterface
// ----------------------------------------------------------------------------

/**
@class WorkerInterface

@brief class to configure worker behavior in an executor

The tf::WorkerInterface class allows users to customize worker properties when creating an executor. 
Examples include binding workers to specific CPU cores or 
invoking custom methods before and after a worker enters or leaves the work-stealing loop.
When you create an executor, it spawns a set of workers to execute tasks
with the following logic:

@code{.cpp}
for(size_t n=0; n<num_workers; n++) {
  create_thread([](Worker& worker)

    // pre-processing executor-specific worker information
    // ...

    // enter the scheduling loop
    // Here, WorkerInterface::scheduler_prologue is invoked, if any
    worker_interface->scheduler_prologue(worker);
    
    try {
      while(1) {
        perform_work_stealing_algorithm();
        if(stop) {
          break;
        }
      }
    } catch(...) {
      exception_ptr = std::current_exception();
    }

    // leaves the scheduling loop and joins this worker thread
    // Here, WorkerInterface::scheduler_epilogue is invoked, if any
    worker_interface->scheduler_epilogue(worker, exception_ptr);
  );
}
@endcode

@attention
tf::WorkerInterface::scheduler_prologue and tf::WorkerInterface::scheduler_eiplogue 
are invoked by each worker simultaneously.

*/
class WorkerInterface {

  public:

  /**
  @brief default destructor
  */
  virtual ~WorkerInterface() = default;

  /**
  @brief method to call before a worker enters the scheduling loop
  @param worker a reference to the worker

  The method is called by the constructor of an executor.
  */
  virtual void scheduler_prologue(Worker& worker) = 0;

  /**
  @brief method to call after a worker leaves the scheduling loop
  @param worker a reference to the worker
  @param ptr an pointer to the exception thrown by the scheduling loop

  The method is called by the constructor of an executor.
  */
  virtual void scheduler_epilogue(Worker& worker, std::exception_ptr ptr) = 0;

};

/**
@brief helper function to create an instance derived from tf::WorkerInterface

@tparam T type derived from tf::WorkerInterface
@tparam ArgsT argument types to construct @c T

@param args arguments to forward to the constructor of @c T
*/
template <typename T, typename... ArgsT>
std::unique_ptr<T> make_worker_interface(ArgsT&&... args) {
  static_assert(
    std::is_base_of_v<WorkerInterface, T>,
    "T must be derived from WorkerInterface"
  );
  return std::make_unique<T>(std::forward<ArgsT>(args)...);
}


                                                                                 
                                                                                 
}  // end of namespact tf ------------------------------------------------------  


