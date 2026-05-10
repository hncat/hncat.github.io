---
title: 无锁并发数据结构-栈
date: 2026-04-18
tags:
    - 并发
    - c/c++
categories:
    - 并发
    - c/c++
isOriginal: true
order: 7
dir:
    order: 7
---
## 1.1 无锁的线程安全栈
> [!tip]
> 栈: 查询顺序是添加顺序的逆序-先入后出(LIFO)。

> [!important]
> 在单线程情况下，使用链表的方式，添加一个节点非常简单:
> 1. 创建新节点
> 2. 新节点的next指针指向当前的head节点
> 3. head节点指向新节点
> 但是多线程情况就完全不一样了:
> 当有两个线程同时添加节点的时候，第2步和第3步的时候会产生条件竞争：一个线程可能在修改head值时，另一个线程正在执行第2步，并且在第3步中对head进行更新。就会使之前那个线程的结果被丢弃，亦或是造成更加糟糕的后果。如何解决这个条件竞争之前，还要注意一个事：当head更新并指向了新节点时，另一个线程就能读取到这个节点了。因此，head设置为指向新节点前，新节点完全准备就绪就变很重要；因为，在这之后就不能对节点进行修改了。
> 如何解决这个条件竞争？第3步的时候使用原子“比较/交换”操作，来保证步骤2对head进行读取时，不会对head进行修改；有修改时可以循环“比较/交换”操作。

![栈结构的push实现](/thread/thread_12.svg)

```c++
#include <atomic>

template <typename T>
class lock_free_stack {
 public:
  void push(const T &data) {
    node *const new_node = new node{data}; // 2
    new_node->next = head.load(); //3
    while (!head.compare_exchange_weak(new_node->next, new_node)); // 4
  }
  
 private:
  struct node {
    T data;
    node *next;

    node (const T &data_) : data{data_} {} // 1
  };

  std::atomic<node *> head;
};
```
> 上面代码几乎能匹配之前所说的三个步骤：创建一个新节点②，设置新节点的next指针指向当前head③，并设置head指针指向新节点④。node结构用其自身的构造函数来进行数据填充①，必须保证节点在构造完成后随时能被弹出。之后需要使用compare_exchange_weak()来保证在被存储到new_node->next的head指针和之前的一样③。代码的亮点是使用“比较/交换”操作：返回false时，因为比较失败(例如，head被其他线程锁修改)，会使用head中的内容更新new_node->next(第一个参数)的内容。循环中不需要每次都重新加载head指针，因为编译器会完成这件事。同样，因为循环可能直接就失败了，所以使用compare_exchange_weak要好于使用compare_exchange_strong。

![无锁线程安全栈push](/thread/thread_11.svg)

> [!important]
> 在单线程情况下，使用链表的方式，实现删除。
> 1. 读取当前head指针的值
> 2. 读取head->next
> 3. 设置head到head->next
> 4. 通过索引node，返回data数据
> 5. 删除索引节点

![栈结构的pop实现](/thread/thread_13.svg)

```c++
template <typename T>
class lock_free_stack {
 public:
  void pop(T &result) {
    node *old_head = head.load();
    while (!head.compare_exchange_weak(old_head, old_head->next));
    result = old_head->data;
  }
};
```

> [!tip]
> 这段代码很优雅，但有两个节点泄露的问题。
> 1. 这段代码在空链表时不工作：当head指针是空指针时，要访问next指针时，将引起未定义行为。很容易通过对nullptr的检查进行修复(在while循环中)，要不对空栈抛出一个异常，要不返回一个bool值来表明成功与否。
> 2. 第二个问题就是异常安全。了解了在返回值的时候会出现异常安全问题：有异常被抛出时，复制的值将丢失。这种情况下，传入引用是一种可以接受的解决方案；这样就能保证当有异常抛出时，栈上的数据不会丢失。不幸的是，不能这样做；只能在单一线程对值进行返回时，才进行拷贝以确保拷贝操作的安全性，这就意味着在拷贝结束后这个节点就会被删除。因此，通过引用获取返回值的方式没有任何优点：直接返回也是可以的。若想要安全的返回，必须使用其他方法：返回指向数据值的(智能)指针。
> 当返回的是智能指针时，返回nullptr以表明没有值可返回，但是要求在堆上对智能指针进行内存分配。将分配过程做为pop()的一部分时(也没有更好的选择了)，堆分配内存时可能会抛出一个异常。与此相反，在push()操作中对内存进行分配——无论怎样，都需要对node进行内存分配。返回一个 std::shared_ptr<> 不会抛出异常，所以在pop()中进行内存分配是安全的。将上面的点放在一起，就能看到如下的代码。

```c++
template <typename T>
class lock_free_stack {
 public:
  void push(const T &data) {
    node *const new_node = new node{data};
    new_node->next = head.load();
    while (!head.compare_exchange_weak(node->next, node));
  }

  std::shared_ptr<T> pop() {
    node *old_head = head.load();
    while (old_head && // 3 在解引用前检查old_head是否为空指针
      !head.compare_exchange_weak(old_head, old_head->next));
    return old_head ? old_head->data : std::shared_ptr<T>(); // 4
  }

 private:
  struct node {
    std::shared_ptr<T> data; // 1 指针获取数据
    node *next;

    node(const T &data_) : data{std::make_shared<T>(data_)} {} // 2 让std::sharedptr 指向新分配出来的T
  };

  std::atomic<node *> head;
};
```

> 智能指针指向当前数据①，必须在堆上为数据分配内存(在node结构体中)②。而后，compare_exchage_weak()循环中③，需要在old_head指针前，检查指针是否为空。如果存在相关节点，将会返回相关节点的值；不存在时，将返回一个空指针④。注意，结构是无锁的，但并不是无等待的，因为在push()和pop()函数中都有while循环，当compare_exchange_weak()总是失败的时候，循环将会持续下去。

![线程安全的无锁栈](/thread/lock_free_stack_complete.svg)

## 1.2 在无锁数据结构中管理内存
```c++
template <typename T>
class lock_free_stack {
 private:
  std::atomic<unsigned> threads_in_pop; // 1 原子变量
  void try_reclaim(node *old_head);

 public:
  std::shraed_ptr<T> pop() {
    ++threads_in_pop; // 2 计数+1
    node *old_head = head.load();
    while (!head.compare_exchange(old_head, old_head->next));
    std::shared_ptr<T> res;
    if (old_head) {
      res.swap(old_head->data); // 3 回收删除节点
    }
    try_reclaim(old_head); // 4 从节点中直接提取数据，而非拷贝指针
    return res;
  }
};
```

> threads_in_pop①原子变量用来记录有多少线程试图弹出栈中的元素。调用pop()②函数时，计数器加一；调用try_reclaim()时，计数器减一；这个函数被节点调用时，说明节点已被删除④。因为暂时不需要将节点删除，可以通过swap()函数来删除节点上的数据③(而非只是拷贝指针)，当不再需要这些数据的时候，这些数据会自动删除，而不是持续存在着(因为还有对未删除节点的引用)。接下来看一下try_reclaim()是如何实现的。

```c++
template <typename T>
class lock_free_stack {
 private:
  std::atomic<node *> to_be_deleted;

  static void delete_nodes(node *nodes) {
    while (nodes) {
      node *next = nodes->next;
      delete nodes;
      nodes = next;
    }
  }

  void try_reclaim(node *old_head) {
    if (threads_in_pop == 1) { // 1
      node *nodes_to_delete = to_be_deleted.exchange(nullptr); // 2 声明“可删除”列表
      if (!--threads_in_pop) { // 3 是否只有一个线程调用pop()?
        delete_nodes(nodes_to_delete); // 4
      } else if (nodes_to_delete) { // 5
        chain_pending_nodes(nodes_to_delete); // 6
      }
      delete old_head; // 7
    } else {
      chain_pending_node(old_head); // 8
      --threads_in_pop;
    }
  }

  void chain_pending_nodes(node *nodes) {
    node *last = nodes;
    while (node *const next = last->next) { // 9 让next指针指向链表的末尾
      last = next;
    }
    chain_pending_nodes(nodes, last);
  }

  void chain_pending_nodes(node *first, node *last) {
    last->next = to_be_deleted; // 10
    while (!to_be_deleted.compare_exchange_weak(last->next, first)); // 11 用循环来保证last->next的正确性
  }

  void chain_pending_node(node *n) {
    chain_pending_nodes(n, n) // 12
  }
};
```

> 回收节点时①，threads_in_pop的数值是1，当前线程正在对pop()进行访问，这时就可以安全的将节点删除了⑦(将等待节点删除也是安全的)。当数值不是1时，删除任何节点都不安全，所以需要向等待列表中继续添加节点⑧。
> 假设某一时刻，threads_in_pop的值为1。就可以尝试回收等待列表，如果不回收，节点就会继续等待，直到整个栈被销毁。要做到回收，首先要通过原子exchange操作声明②删除列表，并将计数器减一③。如果之后计数的值为0，意味着没有其他线程访问等待节点链表。不必为出现新的等待节点而烦恼，因为它们会被安全的回收。而后，可以使用delete_nodes对链表进行迭代，并将其删除④。
> 计数值在减后不为0时，回收节点就不安全；如果存在⑤，就需要将其挂在等待删除链表之后⑥，这种情况会发生在多个线程同时访问数据结构的时候。一些线程在第一次测试threads_in_pop①和对“回收”链表的声明②操作间调用pop()，这可能会将一个已经被线程访问的节点新填入到链表中。

![延迟删除节点](/thread/lock_free_stack_reclaim.svg)

完整源码实现: [lock_free_stack_reclaim.h](/thread/code/lock_free_stack_reclaim.h)

## 1.3 使用风险指针检测不可回收的节点
> [!tip]
> 当有线程去访问要被(其他线程)删除的对象时，会先设置对这个对象设置风险指针，而后通知其他线程——使用这个指针是危险的行为。当这个对象不再被需要，那么就可以清除风险指针了。
> 当线程想要删除一个对象，就必须检查系统中其他线程是否持有风险指针。当没有风险指针时，就可以安全删除对象。否则，就必须等待风险指针消失。这样，线程就需要周期性的检查要删除的对象是否能安全删除。

```c++
std::shared_ptr<T> pop() {
  std::atomic<void *> &hp = get_hazard_pointer_for_current_thread();
  node *old_head = head.load(); // 1
  node *temp;
  do {
    temp = old_head;
    hp.store(old_head); // 2
    old_head = head.load();
  } while (old_head != temp); // 3
  // ...
}
```
> while循环能保证node不会在读取旧head指针①时，以及在设置风险指针的时被删除。这种模式下，其他线程不知道有线程对这个节点进行了访问。幸运的是，旧head节点要被删除时，head本身会发生变化，所以需要对head进行检查并持续循环，直到head指针中的值与风险指针中的值相同③。使用风险指针，如同依赖对已删除对象的引用。使用默认的new和delete操作对风险指针进行操作时，会出现未定义行为，所以需要确定实现是否支持这样的操作，或使用自定义内存分配器来保证用法的正确性。

![风险指针](/thread/hazard_pointer_protect_validate.svg)

```c++
std::shared_ptr<T> pop() {
  std::atomic<void *> &hp = get_hazard_pointer_for_current_thread();
  node *old_head = head.load();
  do {
    node *temp;
    do { // 1 直到将风险指针设为head指针
      temp = old_head;
      hp.store(old_head);
      old_head = head.load();
    } while (old_head != temp);
  } while (old_head && !head.compare_exchange_strong(old_head, old_head->next));
  hp.store(nullptr); // 2 当声明完成，清除风险指针
  std::shared_ptr<T> res;
  if (old_head) {
    res.swap(old_head->data);
    if (outstanding_hazard_pointers_for(old_head)) { // 3 在删除之前堆风险指针引用的节点进行检查
      reclaim_later(old_head); // 4
    } else {
      delete old_head; // 5
    }
    delete_nodes_with_no_hazards(); // 6
  }
  return res;
}
```

> 首先，循环内部会对风险指针进行设置。“比较/交换”操作失败时会重载old_head，再次进行设置①。因为需要在循环内部做一些实际的工作，所以要使用compare_exchange_strong()：当compare_exchange_weak()伪失败后，风险指针将被重置(没有必要)。过程能保证风险指针在解引用(old_head)之前被正确的设置。已声明了一个风险指针时，就可以将其清除了②。如果想要获取一个节点，就需要检查其他线程上的风险指针，检查是否有其他指针引用该节点③。如果有，就不能删除节点，只能将其放回链表中，之后再进行回收④；如果没有，就能直接将这个节点删除⑤。最后需要对任意节点进行检查，可以调用reclaim_later()。如果链表上没有任何风险指针引用节点，就可以安全的删除这些节点⑥。当有节点持有风险指针，就只能等待下一个调用pop()的线程退出。

```c++
unsigned const max_hazard_pointers = 100;

struct hazard_pointer {
  std::atomic<std::thread::id> id;
  std::atomic<void *> pointer;
};

hazard_pointer hazard_pointers[max_hazard_pointers];

class hp_owner {
  hazard_pointer *hp;

 public:
  hp_owner(hp_owner const &) = delete;
  hp_owner &operator=(hp_owner const &) = delete;
  hp_owner() : hp{nullptr} {
    for (unsigned i = 0; i < max_hazard_pointers; ++i) {
      std::thread::id old_id;
      if (hazard_pointers[i].id.compare_exchange_strong( // 6 尝试声明风险指针的所有权
        old_id, std::this_thread::get_id())) {
        hp = &hazard_pointers[i];
        break; // 7
      }
    }
    if (!hp) { // 1
      throw std::runtime_error("no hazard pointers available");
    }
  }

  std::atomic<void *> &get_pointer() {
    return hp->pointer;
  }

  ~hp_owner() { // 2
    hp->pointer.store(nullptr); // 8
    hp->id.store(std::thread::id()); // 9
  }
};

std::atomc<void *> &get_hazard_pointer_for_current_thread() { // 3
  thread_local static hp_owner hazard; // 4 每个线程都有自己的风险指针
  return hazard.get_pointer(); // 5
}

bool outstanding_hazard_pointers_for(void *p) {
  for (unsigned i = 0; i < max_hazard_pointers; ++i) {
    if (hazard_pointers[i].pointer.load() == p) {
      return true;
    }
  }
  return false;
}
```

> get_hazard_pointer_for_current_thread()的实现看起来很简单③：一个hp_owner④类型的thread_local(本线程所有)变量，用来存储当前线程的风险指针，返回这个变量所持有的指针⑤。之后的工作：有线程第一次调用这个函数时，新hp_owner实例就被创建。这个实例的构造函数⑥会通过查询“所有者/指针”表，寻找没有所有者的记录。用compare_exchange_strong()来检查某个记录是否有所有者，并进行析构②。compare_exchange_strong()失败时，其他线程也可拥有这个记录，所以可以继续执行下去。当交换成功，当前线程就拥有了这些记录，而后进行存储并停止搜索⑦。遍历了列表也没有找到物所有权的记录①时，就说明有很多线程在使用风险指针，所以会抛出一个异常。
> 当hp_owner实例被一个给定的线程所创建，之后的访问会很快，因为指针在缓存中，所以表不需要再次遍历。
> 当线程退出时，hp_owner的实例将会被销毁。析构函数会在 std::thread::id() 设置拥有者ID前，将指针重置为 nullptr，这样就允许其他线程对这条记录进行复用⑧⑨。

```c++
template <typename T>
void do_delete(void *p) {
  delete static_cast<T *>(p);
}

struct data_to_reclaim {
  void *data;
  std::function<void (void *)> deleter;
  data_to_reclaim *next;

  template <typename T>
  data_to_reclaim(T *p) // 1
    : data{p},
      deleter{&do_delete<T>},
      next{0} {}

  ~data_to_reclaim() {
    deleter(data); // 2
  }
};

std::atomic<data_to_reclaim *> nodes_to_reclaim;

void add_to_reclaim_list(data_to_reclaim *node) { // 3
  node->next = nodes_to_reclaim.load();
  while (nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

template <typename T>
void reclaim_late(T *data) { // 4
  add_to_reclaim_list(new data_to_reclaim(data)); // 5
}

void delete_nodes_with_no_hazards() {
  data_to_reclaim *current = nodes_to_reclaim.exchange(nullptr); // 6
  while (current) {
    data_to_reclaim *const next = current->next;
    if (!outstanding_hazard_pointers_for(current->data)) { // 7
      delete current; // 8
    } else {
      add_to_reclaim_list(current); // 9
    }
  }
  current = next;
}
```
> 首先，reclaim_later()是一个函数模板④。风险指针是一个通用解决方案，不能将栈节点的类型写死。使用 std::atomic<void*> 对风险指针进行存储，需要对任意类型的指针进行处理。不过不能使用void *形式，因为当要删除数据项时，delete操作只能对实际类型指针进行操作。data_to_reclaim的构造函数就很优雅：reclaim_later()只创建一个data_to_reclaim的实例，并且将实例添加到回收链表中⑤。add_to_reclaim_list()③就是使用compare_exchange_weak()循环来访问链表头(就如你之前看到的那样)。
> 回到data_to_reclaim的构造函数①：构造函数也是模板函数。会删除的成员数据类型为void * ，并为do _ deltete()函数提供一个合适的指针实例——将void *类型转换成要删除的类型，然后删除指针所指向的对象。 std::function<>可以安全的产生一个函数指针，所以data_to_reclaim的析构函数可以通过调用存储的函数对数据进行删除②。
> 将节点添加入链表时，data_to_reclaim的析构函数不会被调用；析构函数会在没有风险指针指向节点的时候调用，这也就是delete_nodes_with_no_hazards()的作用。delete_nodes_with_no_hazards()将已声明的链表节点进行回收，使用的是exchange()函数⑥(这个步骤简单 且关键，是为了保证只有一个线程回收这些节点)。这样，其他线程就能自由将节点添加到链表中，或在不影响回收指定节点线程的情况下对节点进行回收。只要有节点存在于链表中，就需要检查每个节点，查看节点是否被风险指针所指向⑦。如果没有风险指针，就可以安全的将记录删除(并且清除存储的数据)⑧。否则，就只能将这个节点添加到链表的后面，再进行回收⑨。虽然实现很简单，也的确安全的回收了被删除的节点，不过开销增加了很多。遍历风险指针数组需要检查max_hazard_pointers原子变量，并且每次pop()调用时，都需要再检查一遍。原子操作很耗时——台式CPU上，100次原子操作要比100次非原子操作慢——所以，pop()成为了性能瓶颈。这种方式很糟糕，不仅需要遍历节点的风险 指针链表，还要遍历等待链表上的每一个节点。有max_hazard_pointers在链表中时，就需要检查 max_hazard_pointers多个已存储的风险指针。

## 1.4 使用引用计数的节点
```c++
template <typename T>
class lock_free_stack {
 private:
  struct node;

  struct counted_node_ptr {
    int external_count; // 1 外部引用计数
    node *ptr;
  };

  struct node {
    std::shared_ptr<T> data;
    std::atomic<int> internal_count; // 2 内部引用计数
    counted_node_ptr next; // 3

    node(T const &data_) :
      data{std::make_shared<T>(data_)},
      internal_count{0} {}
  };

  std::atomic<counted_node_ptr> head; // 4

 public:
  ~lock_free_stack() {
    while (pop());
  }

  void push(T const &data) { // 5
    counted_node_ptr new_node;
    new_node.ptr = new node{data};
    new_node.external_count = 1;
    new_node.ptr->next = head.load();
    while (!head.compare_exchange_weak(new_node.ptr->next, new_node));
  }
};
```
> push()相对简单⑤，可构造一个counted_node_ptr实例，去引用新分配出来的(带有相关数据的)node，并且将node中的next指针设置为当前head。之后使用compare_exchange_weak()对head的值进行设置，就像之前代码清单中所示。因为internal_count刚被设置其值为0，并且external_count是1。因为这是一个新节点只有一个外部引用(head指针)。

```c++
template <typename T>
class lock_free_stack {
 private:
  void increase_head_count(counted_node_ptr &old_counter) {
    counted_node_ptr new_counter;
    do {
      new_counter = old_counter;
      ++new_counter.external_count;
    } while (!head.compare_exchange_strong(old_counter, new_counter)); // 1

    old_counter.external_count = new_counter.external_count;
  }

 public:
  std::shared_ptr<T> pop() {
    counted_node_ptr old_head = head.load();
    for (;;) {
      increase_head_count(old_head);
      node *const ptr = old_head.ptr; // 2
      if (!ptr) {
        return std::shared_ptr<T>();
      }
      if (head.compare_exchange_strong(old_head, ptr->next)) { // 3
        std::shared_ptr<T> res;
        res.swap(ptr->data);

        int const count_increase = old_head.external_count - 2; // 5

        if (ptr->internal_count.fetch_add(count_increase) == -count_increase) { // 6
          delete ptr;
        }
        return res; // 7
      } else if (ptr->internal_count.fetch_sub(1) == 1) {
        delete ptr; // 8
      }
    }
  }
};
```

> 当加载head值后就必须将外部引用加一，表明这个节点正在被引用，可保证解引用时的安全性。在引用计数增加前解引用指针，就会有线程能够访问这个节点，从而当前引用指针就成为了一个悬空指针。这就是将引用计数分离的主要原因：通过增加外部引用计数，保证指针在访问期间的合法性。compare_exchange_strong()的循环中①完成增加，通过比较和设置整个结构体来保证，指针不会在同一时间内被其他线程修改。
> 当计数增加就能安全的解引用ptr，并读取head指针的值，访问指向的节点②。当访问到链表的末尾，指针就是空指针。当指针不为空时，就能尝试对head调用compare_exchange_strong()来删除这个节点③。
> compare_exchange_strong()成功时就拥有对应节点的所有权，并且可以和data进行交换④后返回。这样数据就不会持续保存，因为其他线程也会对栈进行访问，所以会有其他指针指向这个节点。而后，可以使用原子操作fetch_add⑥，将外部计数加到内部计数中去。如果引用计数为0，那么之前的值(fetch_add返回的值)，在相加之前肯定是一个负数，这种情况下就可以将节点删除。这里需要注意的是，相加的值要比外部引用计数少2⑤；当节点已经从链表中删除，就要减少一次计数，并且这个线程无法再次访问指定节点，所以还要再减一。无论节点是否被删除，都能完成操作，所以可以将获取的数据进行返回⑦。
> “比较/交换”③失败时，就说明其他线程已经把对应节点删除了，或者其他线程添加了一个新的节点到栈中。无论是哪种原因，需要通过“比较/交换”的调用，对具有新值的head重新进行操作。不过，首先需要减少节点(要删除的节点)上的引用计数。这个线程将再也没有办法访问这个节点了。如果当前线程是最后一个持有引用(因为其他线程已经将这个节点从栈上删除了)的线程，那么内部引用计数将会为1，所以减一的操作将会让计数器为0。这样，就能在循环⑧之前将对应节点删除。

## 1.5 无锁栈上的内存模型
```c++
void push(T const &data) {
  counted_node_ptr new_node;
  new_node.ptr = new node{data};
  new_node.external_count = 1;
  new_node.ptr->next = head.load(std::memory_order_relaxed);
  while (!head.compare_exchange_weak(new_node.ptr->next, new_node,
    std::memory_order_release, std::memory_order_relaxed));
}
```
> 做push()的线程，会先构造数据项和节点，再设置head。做pop()的线程，会先加载head的值，再做在循环中对head做“比较/交换”操作，并增加引用计数，再读取对应的node节点，获取next的指向值，现在可以看到一组需求关系。next的值是普通的非原子对象，所以为了保证读取安全，必须确定存储(推送线程)和加载(弹出线程)的先行关系。因为原子操作就是push()函数中的compare_exchange_weak()，需要释放操作来获取两个线程间的先行关系，compare_exchange_weak()必须是 std::memory_order_release或更严格的内存序。当compare_exchange_weak()调用失败，什么都不会改变，并且可以持续循环下去，所以使用 std::memory_order_relaxed就足够了。

```c++
void increase_head_count(counted_node_ptr &old_counter) {
  counted_node_ptr new_counter;
  do {
    new_counter = old_counter;
    ++new_counter.extern_count;
  } while(!head.compare_exchange_strong(old_counter, new_counter,
    std::memory_order_acquire, std::memory_order_relaxed));
  old_counter.external_count = new_counter.external_count;
}
```
> 为了确定先行关系，必须在访问next值之前使用 std::memory_order_acquire或更严格的内存序操作。因为，increase_head_count()中使用compare_exchange_strong()就获取next指针指向的旧值，所以要其获取成功就需要确定内存序。如同调用push()那样，当交换失败，循环会继续，所以在失败时使用松散的内存序：std::memory_order_relaxed。compare_exchange_strong()调用成功时，ptr中的值就被存到old_counter中。存储操作是push()中的一个释放操作，compare_exchange_strong()操作是一个获取操作，现在存储同步于加载，并且能够获取先行关系。因此，push()中存储ptr的值要先行于在pop()中对ptr->next的访问，现在的操作就安全了。
```c++
std::shared_ptr<T> pop() {
  counted_node_ptr old_head = head.load(std::memory_order_relaxed);
  for (;;) {
    increase_head_count(old_head);
    node *const ptr = old_head.ptr;
    if (!ptr) {
      return std::shared_ptr<T>{};
    }
    if (head.compare_exchange_strong(old_head, ptr->next, std::memory_order_relaxed)) {
      std::shared_ptr<T> res;
      res.swap(ptr->data);
      int const count_increase = old_head.external_count - 2;
      if (ptr->internal_count.fetch_add(count_increase, std::memory_order_release) == -count_increase) {
        delete ptr;
      }
      return res;
    } else if (ptr->internal_count.fetch_add(-1, std::memory_order_relaxed) == 1) {
      ptr->internal_count.load(std::memory_order_acquire);
      delete ptr;
    }
  }
}
```
> 内存序对head.load()的初始化并不妨碍分析，现在就可以使用 std::memory_order_relaxed 。
> 接下来compare_exchange_strong()将old_head.ptr->next设置为head。是否需要做什么来保证操作线程中的数据完整性呢？当交换成功就能访问ptr->data，所以需要保证在push()线程中已经对ptr->data进行了存储(在加载之前)。increase_head_count()中的获取操作时，保证与push()线程中的存储和“比较/交换”同步。这里的先行关系：在push()线程中存储数据，先行于存储head指针；调用increase_head_count()先行于对ptr->data的加载。即使，pop()中的“比较/交换”操作使用 std::memory_order_relaxed ，这些操作还是能正常运行。唯一不同的地方就是，调用swap()让ptr->data有所变化，且没有其他线程可以对同一节点进行操作(这就是“比较/交换”操作的作用)。compare_exchange_strong()失败时，新值就不会去更新old_head，并继续循环。已确定在increase_head_count()中使用 std::memory_order_acquire 内存序的可行性，所以使用 std::memory_order_relaxed 也可以。
> 其他线程呢？是否需要设置一些更为严格的内存序来保证其他线程的安全呢？回答是“不用”。因为，head只会因“比较/交换”操作有所改变；对于“读-改-写”操作来说，push()中的“比较/交换”操作是构成释放序列的一部分。因此，即使有很多线程在同一时间对head进行修改，push()中的compare_exchange_weak()与increase_head_count()(读取已存储的值)中的compare_exchange_strong()也是同步的。
> 剩余操作就可以用来处理fetch_add()操作(用来改变引用计数的操作)，因为已知其他线程不可能对该节点的数据进行修改，所以从节点中返回数据的线程可以继续执行。不过，当线程获取其他线程修改后的值时，就代表操作失败(swap()是用来提取数据项的引用)。为了避免数据竞争，需要保证swap()先行于delete操作。一种简单的解决办法：在“成功返回”分支中对fetch_add()使用 std::memory_order_release 内存序，在“再次循环”分支中对fetch_add()使用 std::memory_order_qcquire 内存序。不过，这就有点矫枉过正：只有一个线程做delete操作(将引用计数设置为0的线程)，所以只有这个线程需要获取操作。因为fetch_add()是一个“读-改-写”操作，是释放序列的一部分，所以可以使用一个额外的load()做获取。当“再次循环”分支将引用计数减为0时，fetch_add()可以重载引用计数，使用 std::memory_order_acquire 为了保持需求的同步关系；并且，fetch_add()本身可以使用 std::memory_order_relaxed 。