#include <atomic>
#include <memory>

template <typename T>
class lock_free_stack {
 private:
  struct node {
    std::shared_ptr<T> data;
    node *next;

    node(const T &data_) : data(std::make_shared<T>(data_)) {}
  };

  std::atomic<node *> head{nullptr};  // 栈顶指针
  std::atomic<node *> to_be_deleted{nullptr};  // 用于记录待删除的节点
  std::atomic<uint32_t> threads_in_pop{0};  // 记录正在执行 pop 操作的线程数量

 public:
  void push(const T &data) {
    node *new_node = new node{data};
    new_node->next = head.load();
    while (!head.compare_exchange_weak(new_node->next, new_node));
  }

  std::shared_ptr<T> pop() {
    ++threads_in_pop;
    node *old_head = head.load();
    while (old_head && !head.compare_exchange_weak(old_head, old_head->next));
    std::shared_ptr<T> res{nullptr};
    if (old_head) {
      res.swap(old_head->data);
    }
    // 尝试删除节点
    try_reclaim(old_head);
    return res;
  }

 private:
  void delete_nodes(node *nodes) {
    while (nodes) {
      auto next = nodes->next;
      delete nodes;
      nodes = next;
    }
  }

  /// 将待删除列表中的节点加入到新的待删除列表中
  /// @param first 待删除列表中的第一个节点
  /// @param last 待删除列表中的最后一个节点
  void chain_pending_nodes(node *first, node *last) {
    last->next = to_be_deleted;
    while (!to_be_deleted.compare_exchange_weak(last->next, first));
  }

  /// 将待删除列表中的节点加入到新的待删除列表中
  /// @param nodes 待删除列表中的节点
  void chain_pending_nodes(node *nodes) {
    node *last = nodes;
    while (node *const next = last->next) {
      last = next;
    }
    // 将待删除列表中的节点加入到新的待删除列表中
    chain_pending_nodes(nodes, last);
  }

  /// 将待删除列表中的节点加入到新的待删除列表中
  /// @param n 待删除列表中的节点
  void chain_pending_node(node *n) {
    chain_pending_nodes(n, n);
  }

  void try_reclaim(node *old_head) {
    if (threads_in_pop == 1) {
      // 取出待删除列表中的节点
      node *pending_node = to_be_deleted.exchange(nullptr);
      if (!--threads_in_pop) {
        // 如果没有其他线程在执行 pop 操作，直接删除待删除列表中的节点
        delete_nodes(pending_node);
      } else {
        // 需要将待删除列表中的节点加入到新的待删除列表中
        chain_pending_nodes(pending_node);
      }
      delete old_head;
    } else {
      // 加入到待删除列表中
      chain_pending_node(old_head);
      --threads_in_pop;
    }
  }
};