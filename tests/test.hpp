#ifndef TEST_HPP
#define TEST_HPP

#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

#include "etc.hpp"

namespace test {

// ---------------------------------------------------------------------------
// 검사 집계
// ---------------------------------------------------------------------------

// 터미널이 아니거나 NO_COLOR 가 설정되어 있으면 모두 빈 문자열이라,
// CI 로그나 파일로 보낼 때 이스케이프 코드가 섞이지 않습니다.
struct Palette {
  const char *red;
  const char *green;
  const char *cyan;
  const char *bold;
  const char *reset;
};

inline const Palette &color() {
  static Palette palette = {"", "", "", "", ""};
  static bool decided = false;
  if (!decided) {
    decided = true;
    if (isatty(STDOUT_FILENO) && std::getenv("NO_COLOR") == 0) {
      palette.red = "\033[1;31m";
      palette.green = "\033[1;32m";
      palette.cyan = "\033[1;36m";
      palette.bold = "\033[1m";
      palette.reset = "\033[0m";
    }
  }
  return palette;
}

inline bool &current_test_failed() {
  static bool failed = false;
  return failed;
}

// 줄 하나를 끝낸 뒤 잠깐 멈춥니다. 터미널에서 결과가 한꺼번에 출력되지 않고
// 한 줄씩 보이게 하기 위한 것이며, 파이프나 파일로 보낼 때는 멈추지 않습니다.
const unsigned int kLineDelayMicros = 50 * 1000;

inline void end_line() {
  std::cout << std::endl;
  if (isatty(STDOUT_FILENO))
    usleep(kLineDelayMicros);
}

// 실패한 검사 하나를 출력합니다. detail 은 CHECK_EQ 의 실제 값처럼 식만으로는
// 알 수 없는 정보가 있을 때만 다음 줄에 붙습니다.
inline void fail(const char *file, int line, const std::string &expr,
                 const std::string &detail = "") {
  current_test_failed() = true;
  std::cout << "  " << color().red << "FAIL" << color().reset << " " << file
            << " line " << line << ": " << expr;
  end_line();
  if (!detail.empty()) {
    std::cout << "       " << detail;
    end_line();
  }
}

// ---------------------------------------------------------------------------
// 값 출력과 비교
//
// std::pair 와 ft::pair 처럼 타입은 달라도 내용이 같은지 보고 싶을 때가 많아서,
// == 대신 equal_value 를 거치고 출력도 print_value 를 거칩니다.
// ---------------------------------------------------------------------------

template <class T> void print_value(std::ostream &os, const T &v) { os << v; }

template <class A, class B>
void print_value(std::ostream &os, const std::pair<A, B> &p) {
  os << "(";
  print_value(os, p.first);
  os << ", ";
  print_value(os, p.second);
  os << ")";
}

template <class A, class B>
void print_value(std::ostream &os, const ft::pair<A, B> &p) {
  os << "(";
  print_value(os, p.first);
  os << ", ";
  print_value(os, p.second);
  os << ")";
}

template <class T> std::string to_string(const T &v) {
  std::ostringstream os;
  print_value(os, v);
  return os.str();
}

template <class A, class B> bool equal_value(const A &a, const B &b) {
  return a == b;
}

template <class A, class B, class C, class D>
bool equal_value(const std::pair<A, B> &a, const ft::pair<C, D> &b) {
  return equal_value(a.first, b.first) && equal_value(a.second, b.second);
}

template <class A, class B>
std::string got_expected(const A &actual, const B &expected) {
  std::ostringstream os;
  os << "got " << to_string(actual) << ", expected " << to_string(expected);
  return os.str();
}

// 두 컨테이너를 처음부터 끝까지 비교합니다. 같으면 빈 문자열, 다르면 어디가 다른지 설명합니다.
template <class Ref, class Mine>
std::string sequence_mismatch(const Ref &ref, const Mine &mine) {
  if (ref.size() != mine.size()) {
    std::ostringstream os;
    os << "size differs: expected " << ref.size() << ", got " << mine.size();
    return os.str();
  }
  typename Ref::const_iterator a = ref.begin();
  typename Mine::const_iterator b = mine.begin();
  std::size_t index = 0;
  for (; a != ref.end(); ++a, ++b, ++index) {
    if (!equal_value(*a, *b)) {
      std::ostringstream os;
      os << "element " << index << " differs: expected " << to_string(*a)
         << ", got " << to_string(*b);
      return os.str();
    }
  }
  return "";
}

// ---------------------------------------------------------------------------
// 테스트에서 쓰는 보조 타입
// ---------------------------------------------------------------------------

// 컴파일 시점 타입 비교. CHECK(same_type<A, B>::value) 형태로 씁니다.
template <class A, class B> struct same_type {
  static const bool value = false;
};
template <class A> struct same_type<A, A> {
  static const bool value = true;
};

// 생성자와 소멸자 호출을 세는 타입입니다. 컨테이너가 원소의 수명을 올바르게
// 관리하는지, 즉 지운 원소는 파괴하고 살아 있는 원소는 정확히 size 개인지 검사할 때 씁니다.
// copies() 는 복사 생성 횟수로, swap 처럼 원소를 옮기지 않아야 하는 연산을 검사할 때 씁니다.
struct Counted {
  int value;

  Counted() : value(0) { ++live(); }
  Counted(int v) : value(v) { ++live(); }
  Counted(const Counted &other) : value(other.value) {
    ++live();
    ++copies();
  }
  ~Counted() { --live(); }
  Counted &operator=(const Counted &other) {
    value = other.value;
    return *this;
  }

  static int &live() {
    static int n = 0;
    return n;
  }
  static int &copies() {
    static int n = 0;
    return n;
  }
};

inline bool operator<(const Counted &a, const Counted &b) {
  return a.value < b.value;
}

// 할당과 해제 횟수를 세는 allocator 입니다. 실제 메모리는 std::allocator 에 맡기고 횟수만
// 기록합니다. 깊은 복사가 allocate 한 번으로 끝나는지, swap 이 할당 없이 포인터만 바꾸는지,
// 컨테이너가 템플릿 인자로 받은 allocator 를 실제로 쓰는지 확인할 때 씁니다.
// 횟수는 원소 타입과 무관하게 한 곳에 모으므로 rebind 된 allocator 의 할당도 같이 셉니다.
struct AllocationCount {
  static int &allocations() {
    static int n = 0;
    return n;
  }
  static int &deallocations() {
    static int n = 0;
    return n;
  }
  static void reset() {
    allocations() = 0;
    deallocations() = 0;
  }
};

template <class T> class CountingAllocator : public std::allocator<T> {
public:
  typedef std::allocator<T> base;
  typedef typename base::pointer pointer;
  typedef typename base::size_type size_type;

  template <class U> struct rebind {
    typedef CountingAllocator<U> other;
  };

  CountingAllocator() {}
  CountingAllocator(const CountingAllocator &other) : base(other) {}
  template <class U>
  CountingAllocator(const CountingAllocator<U> &other) : base(other) {}

  pointer allocate(size_type n, const void * = 0) {
    ++AllocationCount::allocations();
    return base::allocate(n);
  }

  void deallocate(pointer p, size_type n) {
    ++AllocationCount::deallocations();
    base::deallocate(p, n);
  }
};

// 플랫폼마다 수열이 다른 rand() 대신 쓰는 결정적 난수입니다.
// 같은 seed 를 주면 macOS 와 리눅스에서 같은 순서로 같은 값이 나오므로
// 무작위 테스트가 실패했을 때 그대로 재현할 수 있습니다.
class Random {
public:
  explicit Random(unsigned int seed) : _state(seed) {}

  unsigned int next() {
    _state = _state * 1664525u + 1013904223u;
    return _state >> 8;
  }

  int below(int n) {
    return static_cast<int>(next() % static_cast<unsigned int>(n));
  }

private:
  unsigned int _state;
};

// ---------------------------------------------------------------------------
// 레드블랙 트리 검사
//
// ft::rb_node 처럼 left, right, parent, is_black 멤버를 가진 노드라면 어떤 트리든
// 검사할 수 있습니다. 확인하는 성질은 다음 네 가지입니다.
//
//   1. 루트는 검정이다.
//   2. 빨강 노드의 자식은 모두 검정이다. (빨강이 연달아 나오지 않는다)
//   3. 어느 노드에서 출발하든 리프까지 내려가는 모든 경로의 검정 노드 수가 같다.
//   4. 자식의 parent 포인터는 자기 부모를 가리킨다.
//
// 1 부터 3 이 지켜지면 트리 높이는 2 * log2(n + 1) 을 넘지 않으므로,
// 삽입과 삭제와 탐색이 O(log n) 임이 보장됩니다.
// ---------------------------------------------------------------------------

// 부분 트리의 검정 높이를 돌려줍니다. 위반이 있으면 -1 입니다.
template <class NodePtr>
int rb_black_height(NodePtr node, NodePtr expected_parent) {
  if (node == 0)
    return 1; // 비어 있는 자리(NIL)는 검정으로 셉니다.
  if (node->parent != expected_parent)
    return -1;
  if (!node->is_black) {
    if (node->left != 0 && !node->left->is_black)
      return -1;
    if (node->right != 0 && !node->right->is_black)
      return -1;
  }
  int left = rb_black_height(node->left, node);
  int right = rb_black_height(node->right, node);
  if (left < 0 || right < 0 || left != right)
    return -1;
  return left + (node->is_black ? 1 : 0);
}

// ft::rb_tree 는 end 센티널 노드의 left 가 루트입니다. 그 센티널을 받습니다.
template <class NodePtr> bool rb_tree_is_valid(NodePtr end_node) {
  NodePtr root = end_node->left;
  if (root == 0)
    return true;
  if (!root->is_black)
    return false;
  return rb_black_height(root, end_node) > 0;
}

template <class NodePtr> int rb_tree_height(NodePtr node) {
  if (node == 0)
    return 0;
  int left = rb_tree_height(node->left);
  int right = rb_tree_height(node->right);
  return 1 + (left > right ? left : right);
}

// 노드 수가 n 인 레드블랙 트리가 가질 수 있는 최대 높이입니다.
inline int rb_max_height(std::size_t n) {
  int log2 = 0;
  for (std::size_t v = n + 1; v > 1; v >>= 1)
    ++log2;
  return 2 * (log2 + 1);
}

// 키 순서는 반복자로 확인합니다. 인접한 두 원소가 comp 기준으로 엄격히 증가해야 합니다.
template <class Iterator, class Compare>
bool strictly_sorted(Iterator first, Iterator last, Compare comp) {
  if (first == last)
    return true;
  Iterator prev = first;
  ++first;
  for (; first != last; ++first, ++prev)
    if (!comp(*prev, *first))
      return false;
  return true;
}

// ---------------------------------------------------------------------------
// 실행기
// ---------------------------------------------------------------------------

inline const char *signal_name(int sig) {
  switch (sig) {
  case SIGSEGV:
    return "SIGSEGV";
  case SIGBUS:
    return "SIGBUS";
  case SIGABRT:
    return "SIGABRT";
  case SIGFPE:
    return "SIGFPE";
  case SIGILL:
    return "SIGILL";
  default:
    return "signal";
  }
}

class Suite {
public:
  explicit Suite(const char *name)
      : _name(name), _tests(0), _failed(0),
        _isolate(std::getenv("TEST_NO_FORK") == 0) {
    std::cout << color().bold << _name << color().reset;
    end_line();
  }

  void run(const char *test_name, void (*fn)()) {
    ++_tests;
    current_test_failed() = false;
    std::cout << "  " << color().cyan << test_name << color().reset;
    end_line();

    if (!_isolate) {
      fn();
      finish(current_test_failed());
      return;
    }

    std::cout.flush(); // 자식이 부모의 버퍼를 물려받아 두 번 찍지 않도록 비웁니다.
    pid_t child = fork();
    if (child < 0) {
      fn();
      finish(current_test_failed());
      return;
    }
    if (child == 0) {
      alarm(kTimeoutSeconds);
      fn();
      std::cout.flush();
      // _exit 가 아니라 exit 를 써야 LeakSanitizer 같은 종료 시점 검사가 돌아갑니다.
      std::exit(current_test_failed() ? kFailureExitCode : 0);
    }

    int status = 0;
    waitpid(child, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      finish(false);
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == kFailureExitCode) {
      finish(true); // 자식이 이미 FAIL 과 상세 내용을 출력했습니다.
    } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
      std::ostringstream why;
      why << "timed out after " << kTimeoutSeconds << "s";
      fail_outside_checks(test_name, why.str());
    } else if (WIFSIGNALED(status)) {
      fail_outside_checks(test_name, std::string("crashed with ") +
                                         signal_name(WTERMSIG(status)));
    } else {
      // 검사 실패도 시그널도 아닌 종료입니다. 보통 sanitizer 가 오류를 보고하고
      // 프로세스를 끝낸 경우이며, 자세한 내용은 stderr 에 있습니다.
      std::ostringstream why;
      why << "exited with status " << WEXITSTATUS(status)
          << ", see stderr for the sanitizer report";
      fail_outside_checks(test_name, why.str());
    }
  }

  // 요약을 출력하고 프로세스 종료 코드를 돌려줍니다.
  int result() const {
    std::cout << (_failed ? color().red : color().green) << _name << ": "
              << _tests << " tests, " << _failed << " failed" << color().reset;
    end_line();
    return _failed == 0 ? 0 : 1;
  }

private:
  static const unsigned int kTimeoutSeconds = 30;
  static const int kFailureExitCode = 3;

  void finish(bool failed) {
    if (failed)
      ++_failed;
  }

  // 검사가 아니라 프로세스 수준에서 실패한 테스트(크래시, 타임아웃)를 FAIL 로 적습니다.
  void fail_outside_checks(const char *test_name, const std::string &why) {
    std::cout << "  " << color().red << "FAIL" << color().reset << " "
              << test_name << ": " << why;
    end_line();
    finish(true);
  }

  const char *_name;
  int _tests;
  int _failed;
  bool _isolate;
};

} // namespace test

#define RUN_TEST(suite, fn) (suite).run(#fn, fn)

#define CHECK(expr)                                                            \
  do {                                                                         \
    if (!(expr))                                                               \
      ::test::fail(__FILE__, __LINE__, "CHECK(" #expr ")");                    \
  } while (0)

#define CHECK_EQ(actual, expected)                                             \
  do {                                                                         \
    if (!::test::equal_value((actual), (expected)))                            \
      ::test::fail(__FILE__, __LINE__, "CHECK_EQ(" #actual ", " #expected ")", \
                   ::test::got_expected((actual), (expected)));                \
  } while (0)

#define CHECK_SIZE(container, n)                                               \
  CHECK_EQ((container).size(), static_cast<std::size_t>(n))

#define CHECK_SAME_TYPE(a, b)                                                  \
  do {                                                                         \
    if (!(::test::same_type<a, b>::value))                                     \
      ::test::fail(__FILE__, __LINE__, "CHECK_SAME_TYPE(" #a ", " #b ")");     \
  } while (0)

#define CHECK_THROWS(statement, exception_type)                                \
  do {                                                                         \
    bool caught_expected_ = false;                                             \
    try {                                                                      \
      statement;                                                               \
    } catch (const exception_type &) {                                         \
      caught_expected_ = true;                                                 \
    } catch (...) {                                                            \
    }                                                                          \
    if (!caught_expected_)                                                     \
      ::test::fail(__FILE__, __LINE__,                                         \
                   "CHECK_THROWS(" #statement ", " #exception_type ")");       \
  } while (0)

#define CHECK_SAME_SEQUENCE(ref, mine)                                         \
  do {                                                                         \
    std::string mismatch_ = ::test::sequence_mismatch((ref), (mine));          \
    if (!mismatch_.empty())                                                    \
      ::test::fail(__FILE__, __LINE__,                                         \
                   "CHECK_SAME_SEQUENCE(" #ref ", " #mine ")", mismatch_);     \
  } while (0)

#endif
