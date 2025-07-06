#pragma once

#include "../utility/macros.hpp"
#include "../utility/traits.hpp"

/**
@file tsq.hpp
@brief task queue include file
*/

#ifndef TF_DEFAULT_BOUNDED_TASK_QUEUE_LOG_SIZE 
  /**
  @def TF_DEFAULT_BOUNDED_TASK_QUEUE_LOG_SIZE
  
  This macro defines the default size of the bounded task queue in Log2. 
  Bounded task queue is used by each worker.
  */
  #define TF_DEFAULT_BOUNDED_TASK_QUEUE_LOG_SIZE 8
#endif

#ifndef TF_DEFAULT_UNBOUNDED_TASK_QUEUE_LOG_SIZE 
  /**
  @def TF_DEFAULT_UNBOUNDED_TASK_QUEUE_LOG_SIZE
  
  This macro defines the default size of the unbounded task queue in Log2.
  Unbounded task queue is used by the executor.
  */
  #define TF_DEFAULT_UNBOUNDED_TASK_QUEUE_LOG_SIZE 10
#endif

#ifndef TF_DEFAULT_BOUNDED_XQUEUE_LOG_SIZE
  #define TF_DEFAULT_BOUNDED_XQUEUE_LOG_SIZE 5
#endif

namespace tf {


#if TF_USE_XQUEUE
  // enum class for task queue push return code
  enum class TaskQueueCode {
    TASK_PUSHED, // returned by xq.push and xq.load_balance
    TASK_NOT_PUSHED, // returned by xq.push
    NO_REQUEST // returned by xq.load_balance
  };

  enum class WorkerState {
    IDLE,
    REQUEST_STEAL,
  };

// ----------------------------------------------------------------------------
// XQueue implementation
// ----------------------------------------------------------------------------
  template <typename T, size_t LogSize = TF_DEFAULT_BOUNDED_XQUEUE_LOG_SIZE>
  class BoundedXQueue {
    // Dequeue data structure
    struct XDequeue {
      T* dequeue; // Make sure it is get sequentially allocated
      int64_t head {0};
      int64_t tail {0};
      // alignas(2 * TF_CACHELINE_SIZE) int64_t head {0};
      // alignas(2 * TF_CACHELINE_SIZE) int64_t tail {0};
    };

    static_assert(std::is_pointer_v<T>, "<XQueue>: T must be a pointer type.");

    // Longer Master Dequeue
    constexpr static size_t MasterDequeueSize = int64_t{1} << (LogSize + 3);
    constexpr static size_t MasterDequeueMask = (MasterDequeueSize - 1);

    // Auxiliary Dequeue
    constexpr static int64_t DequeueSize = int64_t{1} << LogSize;
    constexpr static int64_t DequeueMask = (DequeueSize - 1);

    static_assert((MasterDequeueSize >= 2) &&
                  ((MasterDequeueSize & (MasterDequeueSize - 1)) == 0));
    
    static_assert((DequeueSize >= 2) &&
                  ((DequeueSize & (DequeueSize - 1)) == 0));
    

   
    // 2D array of type T with dynamic outer dimension
    // WW: This now is allocated on the heap while TF's bounded task queue is
    // allocated on the stack WW: Need to justify if this is a good idea
    XDequeue *_dequeues {nullptr};
    size_t _nworkers {-1}; // Store the outer dimension size
    size_t _worker_id {-1}; // Current worker id
    size_t _last_q {0};    // Points to the last queue that was used to push a task
    uint64_t _nops_empty {0}; // number of empty queue pop, used to periodically send request
    size_t _last_q_popped {0}; // points to the last queue that was popped from

    // Shared by all other workers
    alignas(2 * TF_CACHELINE_SIZE) size_t _last_q_accessed {0}; // points to the last queue that was accessed
    alignas(2 * TF_CACHELINE_SIZE) uint64_t _steal_request {0}; // steal request from other workers (thief)
    alignas(2 * TF_CACHELINE_SIZE) uint64_t _round {1}; // round number of reading the steal request

  public:

    // Constructor
    BoundedXQueue(const size_t nworkers, const size_t worker_id);
    ~BoundedXQueue();

    /**
    @brief queries the capacity of the queue
    */
    constexpr size_t capacity() const {
      return static_cast<size_t>(DequeueSize);
    }

    // Now it is the same as original xqueue implementation.
    // We may need to re-consider how to deal with queue full situation. Or
    // prioritize pushing to local
    // TODO: an alternative is to use callback to handle the queue full, 
    // but I suspect it will cause deep recursion.
    TaskQueueCode push(T item);

    /**
    @brief pops out an item from the queue
    @return the popped item or nullptr if the queue is empty
    */
    T pop();


    /**
    @brief 
    */

    std::vector<Worker> *_workers;
    std::default_random_engine _rdgen;
    std::uniform_int_distribution<size_t> _udist;


    #ifdef TF_ENABLE_STATS
    uint64_t ntasks_pushed_self {0};
    uint64_t ntasks_pushed_remote {0};
    uint64_t ntasks_not_pushed {0};
    uint64_t ntasks_popped_self {0};
    uint64_t ntasks_popped_remote {0};
    uint64_t ntasks_not_popped {0};

    #ifdef TF_ENABLE_WS
    // Thief side
    uint64_t nrequests_steal_called {0};
    uint64_t nrequests_attempted {0};
    uint64_t nrequests_sent {0};
    // Victim side
    uint64_t nhandled_attempted {0};
    uint64_t nhandled_stolen {0};
    uint64_t nhandled_not_stolen {0};

    #endif // TF_ENABLE_WS
    #endif // TF_ENABLE_STATS
    
    private:

    inline TaskQueueCode _do_load_balance(T item);
    inline void _request_steal();
  };

#endif // TF_USE_XQUEUE

// ----------------------------------------------------------------------------
// Task Queue
// ----------------------------------------------------------------------------


/**
@class: UnboundedTaskQueue

@tparam T data type (must be a pointer type)

@brief class to create a lock-free unbounded work-stealing queue

This class implements the work-stealing queue described in the paper,
<a href="https://www.di.ens.fr/~zappa/readings/ppopp13.pdf">Correct and Efficient Work-Stealing for Weak Memory Models</a>.

Only the queue owner can perform pop and push operations,
while others can steal data from the queue simultaneously.

*/
template <typename T>
class UnboundedTaskQueue {
  
  static_assert(std::is_pointer_v<T>, "T must be a pointer type");

  struct Array {

    int64_t C;
    int64_t M;
    std::atomic<T>* S;

    explicit Array(int64_t c) :
      C {c},
      M {c-1},
      S {new std::atomic<T>[static_cast<size_t>(C)]} {
    }

    ~Array() {
      delete [] S;
    }

    int64_t capacity() const noexcept {
      return C;
    }

    void push(int64_t i, T o) noexcept {
      S[i & M].store(o, std::memory_order_relaxed);
    }

    T pop(int64_t i) noexcept {
      return S[i & M].load(std::memory_order_relaxed);
    }

    Array* resize(int64_t b, int64_t t) {
      Array* ptr = new Array {2*C};
      for(int64_t i=t; i!=b; ++i) {
        ptr->push(i, pop(i));
      }
      return ptr;
    }

  };

  // Doubling the alignment by 2 seems to generate the most
  // decent performance.
  alignas(2*TF_CACHELINE_SIZE) std::atomic<int64_t> _top;
  alignas(2*TF_CACHELINE_SIZE) std::atomic<int64_t> _bottom;
  std::atomic<Array*> _array;
  std::vector<Array*> _garbage;

  public:

  /**
  @brief constructs the queue with the given size in the base-2 logarithm

  @param LogSize the base-2 logarithm of the queue size
  */
  explicit UnboundedTaskQueue(int64_t LogSize = TF_DEFAULT_UNBOUNDED_TASK_QUEUE_LOG_SIZE);

  /**
  @brief destructs the queue
  */
  ~UnboundedTaskQueue();

  /**
  @brief queries if the queue is empty at the time of this call
  */
  bool empty() const noexcept;

  /**
  @brief queries the number of items at the time of this call
  */
  size_t size() const noexcept;

  /**
  @brief queries the capacity of the queue
  */
  int64_t capacity() const noexcept;
  
  /**
  @brief inserts an item to the queue

  @param item the item to push to the queue
  
  Only the owner thread can insert an item to the queue.
  The operation can trigger the queue to resize its capacity
  if more space is required.
  */
  void push(T item);

  /**
  @brief pops out an item from the queue

  Only the owner thread can pop out an item from the queue.
  The return can be a @c nullptr if this operation failed (empty queue).
  */
  T pop();

  /**
  @brief steals an item from the queue

  Any threads can try to steal an item from the queue.
  The return can be a @c nullptr if this operation failed (not necessary empty).
  */
  T steal();

  /**
  @brief attempts to steal a task with a hint mechanism
  
  @param num_empty_steals a reference to a counter tracking consecutive empty steal attempts
  
  This function tries to steal a task from the queue. If the steal attempt
  is successful, the stolen task is returned. 
  Additionally, if the queue is empty, the provided counter `num_empty_steals` is incremented;
  otherwise, `num_empty_steals` is reset to zero.

  */
  T steal_with_hint(size_t& num_empty_steals);

  private:

  Array* resize_array(Array* a, int64_t b, int64_t t);
};

// Constructor
template <typename T>
UnboundedTaskQueue<T>::UnboundedTaskQueue(int64_t LogSize) {
  _top.store(0, std::memory_order_relaxed);
  _bottom.store(0, std::memory_order_relaxed);
  _array.store(new Array{(int64_t{1} << LogSize)}, std::memory_order_relaxed);
  _garbage.reserve(32);
}

// Destructor
template <typename T>
UnboundedTaskQueue<T>::~UnboundedTaskQueue() {
  for(auto a : _garbage) {
    delete a;
  }
  delete _array.load();
}

// Function: empty
template <typename T>
bool UnboundedTaskQueue<T>::empty() const noexcept {
  int64_t t = _top.load(std::memory_order_relaxed);
  int64_t b = _bottom.load(std::memory_order_relaxed);
  return (b <= t);
}

// Function: size
template <typename T>
size_t UnboundedTaskQueue<T>::size() const noexcept {
  int64_t t = _top.load(std::memory_order_relaxed);
  int64_t b = _bottom.load(std::memory_order_relaxed);
  return static_cast<size_t>(b >= t ? b - t : 0);
}

// Function: push
template <typename T>
void UnboundedTaskQueue<T>::push(T o) {

  int64_t b = _bottom.load(std::memory_order_relaxed);
  int64_t t = _top.load(std::memory_order_acquire);
  Array* a = _array.load(std::memory_order_relaxed);

  // queue is full with one additional item (b-t+1)
  if TF_UNLIKELY(a->capacity() - 1 < (b - t)) {
    a = resize_array(a, b, t);
  }

  a->push(b, o);
  std::atomic_thread_fence(std::memory_order_release);

  // original paper uses relaxed here but tsa complains
  _bottom.store(b + 1, std::memory_order_release);
}

// Function: pop
template <typename T>
T UnboundedTaskQueue<T>::pop() {

  int64_t b = _bottom.load(std::memory_order_relaxed) - 1;
  Array* a = _array.load(std::memory_order_relaxed);
  _bottom.store(b, std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t t = _top.load(std::memory_order_relaxed);

  T item {nullptr};

  if(t <= b) {
    item = a->pop(b);
    if(t == b) {
      // the last item just got stolen
      if(!_top.compare_exchange_strong(t, t+1,
                                               std::memory_order_seq_cst,
                                               std::memory_order_relaxed)) {
        item = nullptr;
      }
      _bottom.store(b + 1, std::memory_order_relaxed);
    }
  }
  else {
    _bottom.store(b + 1, std::memory_order_relaxed);
  }

  return item;
}

// Function: steal
template <typename T>
T UnboundedTaskQueue<T>::steal() {
  
  int64_t t = _top.load(std::memory_order_acquire);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t b = _bottom.load(std::memory_order_acquire);

  T item {nullptr};

  if(t < b) {
    Array* a = _array.load(std::memory_order_consume);
    item = a->pop(t);
    if(!_top.compare_exchange_strong(t, t+1,
                                     std::memory_order_seq_cst,
                                     std::memory_order_relaxed)) {
      return nullptr;
    }
  }

  return item;
}

// Function: steal
template <typename T>
T UnboundedTaskQueue<T>::steal_with_hint(size_t& num_empty_steals) {
  
  int64_t t = _top.load(std::memory_order_acquire);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t b = _bottom.load(std::memory_order_acquire);

  T item {nullptr};

  if(t < b) {
    num_empty_steals = 0;
    Array* a = _array.load(std::memory_order_consume);
    item = a->pop(t);
    if(!_top.compare_exchange_strong(t, t+1,
                                     std::memory_order_seq_cst,
                                     std::memory_order_relaxed)) {
      return nullptr;
    }
  }
  else {
    ++num_empty_steals;
  }
  return item;
}

// Function: capacity
template <typename T>
int64_t UnboundedTaskQueue<T>::capacity() const noexcept {
  return _array.load(std::memory_order_relaxed)->capacity();
}

template <typename T>
typename UnboundedTaskQueue<T>::Array*
UnboundedTaskQueue<T>::resize_array(Array* a, int64_t b, int64_t t) {

  //Array* tmp = a->resize(b, t);
  //_garbage.push_back(a);
  //std::swap(a, tmp);
  //_array.store(a, std::memory_order_release);
  //// Note: the original paper using relaxed causes t-san to complain
  ////_array.store(a, std::memory_order_relaxed);
  //return a;
  

  Array* tmp = a->resize(b, t);
  _garbage.push_back(a);
  _array.store(tmp, std::memory_order_release);
  // Note: the original paper using relaxed causes t-san to complain
  //_array.store(a, std::memory_order_relaxed);
  return tmp;
}

// ----------------------------------------------------------------------------
// BoundedTaskQueue
// ----------------------------------------------------------------------------

/**
@class: BoundedTaskQueue

@tparam T data type
@tparam LogSize the base-2 logarithm of the queue size

@brief class to create a lock-free bounded work-stealing queue

This class implements the work-stealing queue described in the paper, 
"Correct and Efficient Work-Stealing for Weak Memory Models,"
available at https://www.di.ens.fr/~zappa/readings/ppopp13.pdf.

Only the queue owner can perform pop and push operations,
while others can steal data from the queue.
*/
template <typename T, size_t LogSize = TF_DEFAULT_BOUNDED_TASK_QUEUE_LOG_SIZE>
class BoundedTaskQueue {
  
  static_assert(std::is_pointer_v<T>, "T must be a pointer type");
  
  constexpr static int64_t BufferSize = int64_t{1} << LogSize;
  constexpr static int64_t BufferMask = (BufferSize - 1);

  static_assert((BufferSize >= 2) && ((BufferSize & (BufferSize - 1)) == 0));

  alignas(2*TF_CACHELINE_SIZE) std::atomic<int64_t> _top {0};
  alignas(2*TF_CACHELINE_SIZE) std::atomic<int64_t> _bottom {0};
  alignas(2*TF_CACHELINE_SIZE) std::atomic<T> _buffer[BufferSize];

  public:
    
  /**
  @brief constructs the queue with a given capacity
  */
  BoundedTaskQueue() = default;

  /**
  @brief destructs the queue
  */
  ~BoundedTaskQueue() = default;
  
  /**
  @brief queries if the queue is empty at the time of this call
  */
  bool empty() const noexcept;
  
  /**
  @brief queries the number of items at the time of this call
  */
  size_t size() const noexcept;

  /**
  @brief queries the capacity of the queue
  */
  constexpr size_t capacity() const;
  
  /**
  @brief tries to insert an item to the queue

  @tparam O data type 
  @param item the item to perfect-forward to the queue
  @return `true` if the insertion succeed or `false` (queue is full)
  
  Only the owner thread can insert an item to the queue. 

  */
  template <typename O>
  bool try_push(O&& item);
  
  /**
  @brief tries to insert an item to the queue or invoke the callable if fails

  @tparam O data type 
  @tparam C callable type
  @param item the item to perfect-forward to the queue
  @param on_full callable to invoke when the queue is full (insertion fails)
  
  Only the owner thread can insert an item to the queue. 

  */
  template <typename O, typename C>
  void push(O&& item, C&& on_full);
  
  /**
  @brief pops out an item from the queue

  Only the owner thread can pop out an item from the queue. 
  The return can be a `nullptr` if this operation failed (empty queue).
  */
  T pop();
  
  /**
  @brief steals an item from the queue

  Any threads can try to steal an item from the queue.
  The return can be a `nullptr` if this operation failed (not necessary empty).
  */
  T steal();

  /**
  @brief attempts to steal a task with a hint mechanism
  
  @param num_empty_steals a reference to a counter tracking consecutive empty steal attempts
  
  This function tries to steal a task from the queue. If the steal attempt
  is successful, the stolen task is returned. 
  Additionally, if the queue is empty, the provided counter `num_empty_steals` is incremented;
  otherwise, `num_empty_steals` is reset to zero.
  */
  T steal_with_hint(size_t& num_empty_steals);

  #ifdef TF_ENABLE_STATS
  
  uint64_t ntasks_pushed_wsq {0};
  uint64_t ntasks_not_pushed_wsq {0};
  uint64_t ntasks_popped_wsq {0};
  uint64_t ntasks_not_popped_wsq {0};
  uint64_t ntasks_stolen_wsq {0};
  uint64_t ntasks_not_stolen_wsq {0};
  #endif // TF_ENABLE_STATS
};

// Function: empty
template <typename T, size_t LogSize>
bool BoundedTaskQueue<T, LogSize>::empty() const noexcept {
  int64_t t = _top.load(std::memory_order_relaxed);
  int64_t b = _bottom.load(std::memory_order_relaxed);
  return b <= t;
}

// Function: size
template <typename T, size_t LogSize>
size_t BoundedTaskQueue<T, LogSize>::size() const noexcept {
  int64_t t = _top.load(std::memory_order_relaxed);
  int64_t b = _bottom.load(std::memory_order_relaxed);
  return static_cast<size_t>(b >= t ? b - t : 0);
}

// Function: try_push
template <typename T, size_t LogSize>
template <typename O>
bool BoundedTaskQueue<T, LogSize>::try_push(O&& o) {

  int64_t b = _bottom.load(std::memory_order_relaxed);
  int64_t t = _top.load(std::memory_order_acquire);

  // queue is full with one additional item (b-t+1)
  if TF_UNLIKELY((b - t) > BufferSize - 1) {
    #ifdef TF_ENABLE_STATS
    ++ntasks_not_pushed_wsq;
    #endif // TF_ENABLE_STATS
    return false;
  }
  
  _buffer[b & BufferMask].store(std::forward<O>(o), std::memory_order_relaxed);

  std::atomic_thread_fence(std::memory_order_release);
  
  // original paper uses relaxed here but tsa complains
  _bottom.store(b + 1, std::memory_order_release);
  #ifdef TF_ENABLE_STATS
  ++ntasks_pushed_wsq;
  #endif // TF_ENABLE_STATS
  return true;
}

// Function: push
template <typename T, size_t LogSize>
template <typename O, typename C>
void BoundedTaskQueue<T, LogSize>::push(O&& o, C&& on_full) {

  int64_t b = _bottom.load(std::memory_order_relaxed);
  int64_t t = _top.load(std::memory_order_acquire);

  // queue is full with one additional item (b-t+1)
  if TF_UNLIKELY((b - t) > BufferSize - 1) {
    #ifdef TF_ENABLE_STATS
    ++ntasks_not_pushed_wsq;
    #endif // TF_ENABLE_STATS
    on_full();
    return;
  }
  
  _buffer[b & BufferMask].store(std::forward<O>(o), std::memory_order_relaxed);

  std::atomic_thread_fence(std::memory_order_release);
  
  // original paper uses relaxed here but tsa complains
  _bottom.store(b + 1, std::memory_order_release);
  #ifdef TF_ENABLE_STATS
  ++ntasks_pushed_wsq;
  // printf("q size: %lu\n", size());
  #endif // TF_ENABLE_STATS
}

// Function: pop
template <typename T, size_t LogSize>
T BoundedTaskQueue<T, LogSize>::pop() {

  int64_t b = _bottom.load(std::memory_order_relaxed) - 1;
  _bottom.store(b, std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t t = _top.load(std::memory_order_relaxed);

  T item {nullptr};

  if(t <= b) {
    item = _buffer[b & BufferMask].load(std::memory_order_relaxed);
    if(t == b) {
      // the last item just got stolen
      if(!_top.compare_exchange_strong(t, t+1, 
                                       std::memory_order_seq_cst, 
                                       std::memory_order_relaxed)) {
        item = nullptr;
      }
      _bottom.store(b + 1, std::memory_order_relaxed);
    }
  }
  else {
    _bottom.store(b + 1, std::memory_order_relaxed);
  }
  #ifdef TF_ENABLE_STATS
  if(item) {
    ++ntasks_popped_wsq;
  }
  else {
    ++ntasks_not_popped_wsq;
  }
  #endif // TF_ENABLE_STATS
  return item;
}

// Function: steal
template <typename T, size_t LogSize>
T BoundedTaskQueue<T, LogSize>::steal() {
  int64_t t = _top.load(std::memory_order_acquire);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t b = _bottom.load(std::memory_order_acquire);
  
  T item{nullptr};

  if(t < b) {
    item = _buffer[t & BufferMask].load(std::memory_order_relaxed);
    if(!_top.compare_exchange_strong(t, t+1,
                                     std::memory_order_seq_cst,
                                     std::memory_order_relaxed)) {
      return nullptr;
    }
  }
  #ifdef TF_ENABLE_STATS
  if(item) {
    ++ntasks_stolen_wsq;
  }
  else {
    ++ntasks_not_stolen_wsq;
  }
  #endif // TF_ENABLE_STATS

  return item;
}

// Function: steal
template <typename T, size_t LogSize>
T BoundedTaskQueue<T, LogSize>::steal_with_hint(size_t& num_empty_steals) {
  int64_t t = _top.load(std::memory_order_acquire);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t b = _bottom.load(std::memory_order_acquire);
  
  T item {nullptr};

  if(t < b) {
    num_empty_steals = 0;
    item = _buffer[t & BufferMask].load(std::memory_order_relaxed);
    if(!_top.compare_exchange_strong(t, t+1,
                                     std::memory_order_seq_cst,
                                     std::memory_order_relaxed)) {
      return nullptr;
    }
  }
  else {
    ++num_empty_steals;
  }
  #ifdef TF_ENABLE_STATS
  if(item) {
    ++ntasks_stolen_wsq;
  }
  else {
    ++ntasks_not_stolen_wsq;
  }
  #endif // TF_ENABLE_STATS
  return item;
}

// Function: capacity
template <typename T, size_t LogSize>
constexpr size_t BoundedTaskQueue<T, LogSize>::capacity() const {
  return static_cast<size_t>(BufferSize);
}



//-----------------------------------------------------------------------------

//template <typename T>
//class UnboundedTaskQueue2 {
//  
//  static_assert(std::is_pointer_v<T>, "T must be a pointer type");
//
//  struct Array {
//
//    int64_t C;
//    int64_t M;
//    std::atomic<T>* S;
//
//    explicit Array(int64_t c) :
//      C {c},
//      M {c-1},
//      S {new std::atomic<T>[static_cast<size_t>(C)]} {
//    }
//
//    ~Array() {
//      delete [] S;
//    }
//
//    int64_t capacity() const noexcept {
//      return C;
//    }
//
//    void push(int64_t i, T o) noexcept {
//      S[i & M].store(o, std::memory_order_relaxed);
//    }
//
//    T pop(int64_t i) noexcept {
//      return S[i & M].load(std::memory_order_relaxed);
//    }
//
//    Array* resize(int64_t b, int64_t t) {
//      Array* ptr = new Array {2*C};
//      for(int64_t i=t; i!=b; ++i) {
//        ptr->push(i, pop(i));
//      }
//      return ptr;
//    }
//
//  };
//
//  // Doubling the alignment by 2 seems to generate the most
//  // decent performance.
//  alignas(2*TF_CACHELINE_SIZE) std::atomic<int64_t> _top;
//  alignas(2*TF_CACHELINE_SIZE) std::atomic<int64_t> _bottom;
//  std::atomic<Array*> _array;
//  std::vector<Array*> _garbage;
//
//  static constexpr int64_t BOTTOM_LOCK = std::numeric_limits<int64_t>::min();
//  static constexpr int64_t BOTTOM_MASK = std::numeric_limits<int64_t>::max();
//
//  public:
//
//  /**
//  @brief constructs the queue with the given size in the base-2 logarithm
//
//  @param LogSize the base-2 logarithm of the queue size
//  */
//  explicit UnboundedTaskQueue2(int64_t LogSize = TF_DEFAULT_UNBOUNDED_TASK_QUEUE_LOG_SIZE);
//
//  /**
//  @brief destructs the queue
//  */
//  ~UnboundedTaskQueue2();
//
//  /**
//  @brief queries if the queue is empty at the time of this call
//  */
//  bool empty() const noexcept;
//
//  /**
//  @brief queries the number of items at the time of this call
//  */
//  size_t size() const noexcept;
//
//  /**
//  @brief queries the capacity of the queue
//  */
//  int64_t capacity() const noexcept;
//  
//  /**
//  @brief inserts an item to the queue
//
//  @param item the item to push to the queue
//  
//  Only the owner thread can insert an item to the queue.
//  The operation can trigger the queue to resize its capacity
//  if more space is required.
//  */
//  void push(T item);
//
//  /**
//  @brief steals an item from the queue
//
//  Any threads can try to steal an item from the queue.
//  The return can be a @c nullptr if this operation failed (not necessary empty).
//  */
//  T steal();
//
//  private:
//
//  Array* resize_array(Array* a, int64_t b, int64_t t);
//};
//
//// Constructor
//template <typename T>
//UnboundedTaskQueue2<T>::UnboundedTaskQueue2(int64_t LogSize) {
//  _top.store(0, std::memory_order_relaxed);
//  _bottom.store(0, std::memory_order_relaxed);
//  _array.store(new Array{(int64_t{1} << LogSize)}, std::memory_order_relaxed);
//  _garbage.reserve(32);
//}
//
//// Destructor
//template <typename T>
//UnboundedTaskQueue2<T>::~UnboundedTaskQueue2() {
//  for(auto a : _garbage) {
//    delete a;
//  }
//  delete _array.load();
//}
//
//// Function: empty
//template <typename T>
//bool UnboundedTaskQueue2<T>::empty() const noexcept {
//  int64_t b = _bottom.load(std::memory_order_relaxed) & BOTTOM_MASK;
//  int64_t t = _top.load(std::memory_order_relaxed);
//  return (b <= t);
//}
//
//// Function: size
//template <typename T>
//size_t UnboundedTaskQueue2<T>::size() const noexcept {
//  int64_t b = _bottom.load(std::memory_order_relaxed) & BOTTOM_MASK;
//  int64_t t = _top.load(std::memory_order_relaxed);
//  return static_cast<size_t>(b >= t ? b - t : 0);
//}
//
//// Function: push
//template <typename T>
//void UnboundedTaskQueue2<T>::push(T o) {
//  
//  // spin until getting an exclusive access to b
//  int64_t b = _bottom.load(std::memory_order_acquire) & BOTTOM_MASK;
//  while(!_bottom.compare_exchange_weak(b, b | BOTTOM_LOCK, std::memory_order_acquire,
//                                                           std::memory_order_relaxed)) {
//    b = b & BOTTOM_MASK;
//  }
//
//  // critical region
//  int64_t t = _top.load(std::memory_order_acquire);
//  Array* a = _array.load(std::memory_order_relaxed);
//
//  // queue is full
//  if TF_UNLIKELY(a->capacity() - 1 < (b - t)) {
//    a = resize_array(a, b, t);
//  }
//
//  a->push(b, o);
//  std::atomic_thread_fence(std::memory_order_release);
//
//  // original paper uses relaxed here but tsa complains
//  _bottom.store(b + 1, std::memory_order_release);
//}
//
//// Function: steal
//template <typename T>
//T UnboundedTaskQueue2<T>::steal() {
//  
//  int64_t t = _top.load(std::memory_order_acquire);
//  std::atomic_thread_fence(std::memory_order_seq_cst);
//  int64_t b = _bottom.load(std::memory_order_acquire) & BOTTOM_MASK;
//
//  T item {nullptr};
//
//  if(t < b) {
//    Array* a = _array.load(std::memory_order_consume);
//    item = a->pop(t);
//    if(!_top.compare_exchange_strong(t, t+1,
//                                     std::memory_order_seq_cst,
//                                     std::memory_order_relaxed)) {
//      return nullptr;
//    }
//  }
//
//  return item;
//}
//
//// Function: capacity
//template <typename T>
//int64_t UnboundedTaskQueue2<T>::capacity() const noexcept {
//  return _array.load(std::memory_order_relaxed)->capacity();
//}
//
//template <typename T>
//typename UnboundedTaskQueue2<T>::Array*
//UnboundedTaskQueue2<T>::resize_array(Array* a, int64_t b, int64_t t) {
//
//  Array* tmp = a->resize(b, t);
//  _garbage.push_back(a);
//  std::swap(a, tmp);
//  _array.store(a, std::memory_order_release);
//  // Note: the original paper using relaxed causes t-san to complain
//  //_array.store(a, std::memory_order_relaxed);
//  return a;
//}

}  // end of namespace tf -----------------------------------------------------



