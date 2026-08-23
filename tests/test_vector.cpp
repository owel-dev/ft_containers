#include "test.hpp"
#include "vector.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <list>
#include <new>
#include <stdexcept>
#include <vector>

static const int kSeq[] = {1, 2, 3, 4, 5, 6, 7};
static const int kSeqLen = sizeof(kSeq) / sizeof(kSeq[0]);

typedef std::vector<int> std_vec;
typedef ft::vector<int> ft_vec;

// ---------------------------------------------------------------------------
// 생성자와 대입
// ---------------------------------------------------------------------------

static void vector_default_constructor_is_empty() {
  ft_vec v;
  CHECK(v.empty());
  CHECK_SIZE(v, 0);
  CHECK_EQ(v.capacity(), std::size_t(0));
  CHECK(v.begin() == v.end());
}

static void vector_fill_constructor_repeats_value() {
  std_vec ref(4, 9);
  ft_vec mine(4, 9);
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(mine.capacity() >= mine.size());
}

static void vector_fill_constructor_with_zero_count_is_empty() {
  ft_vec v(0, 7);
  CHECK(v.empty());
  CHECK(v.begin() == v.end());
}

// vector(5, 3) 처럼 정수 두 개를 주면 범위 생성자가 아니라 채우기 생성자여야 합니다.
// enable_if 와 is_integral 로 템플릿 오버로드를 걸러내지 않으면 여기서 틀립니다.
static void vector_two_integer_arguments_pick_fill_constructor() {
  ft_vec v(5, 3);
  std_vec ref(5, 3);
  CHECK_SAME_SEQUENCE(ref, v);
}

static void vector_range_constructor_copies_from_pointers() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_range_constructor_accepts_bidirectional_iterators() {
  std::list<int> source(kSeq, kSeq + kSeqLen);
  std_vec ref(source.begin(), source.end());
  ft_vec mine(source.begin(), source.end());
  CHECK_SAME_SEQUENCE(ref, mine);
}

// 멤버를 초기화하지 않는 생성자는 스택이 우연히 0 으로 차 있으면 들키지 않습니다.
// 그래서 0xAB 로 미리 채운 저장소 위에 placement new 로 만들어 결정적으로 검사합니다.
static void vector_range_constructor_with_empty_range_is_empty() {
  void *storage[(sizeof(ft_vec) + sizeof(void *) - 1) / sizeof(void *)];
  std::memset(storage, 0xAB, sizeof(storage));
  ft_vec *v = new (storage) ft_vec(kSeq, kSeq);

  CHECK(v->empty());
  CHECK_SIZE(*v, 0);
  CHECK(v->begin() == v->end());
  CHECK_EQ(v->capacity(), std::size_t(0));

  // 멤버가 쓰레기 값이라면 소멸자가 엉뚱한 메모리를 해제하므로 부르지 않습니다.
  if (!test::current_test_failed())
    v->~ft_vec();
}

static void vector_copy_constructor_makes_independent_copy() {
  ft_vec original(kSeq, kSeq + kSeqLen);
  ft_vec copy(original);
  CHECK_SAME_SEQUENCE(original, copy);
  CHECK(copy.begin() != original.begin()); // 저장소를 공유하지 않습니다.

  copy[0] = 100;
  CHECK_EQ(original[0], 1);
}

static void vector_copy_assignment_replaces_contents() {
  ft_vec small(2, 1);
  ft_vec large(kSeq, kSeq + kSeqLen);

  ft_vec target(3, 0);
  target = large;
  CHECK_SAME_SEQUENCE(large, target);

  target = small;
  CHECK_SAME_SEQUENCE(small, target);
  CHECK(target.capacity() >= target.size());
}

static void vector_self_assignment_keeps_contents() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec &alias = v; // 컴파일러의 자기 대입 경고를 피하면서 같은 객체를 대입합니다.
  v = alias;
  CHECK_SAME_SEQUENCE(ref, v);
}

// ---------------------------------------------------------------------------
// 원소 접근
// ---------------------------------------------------------------------------

static void vector_index_and_at_read_elements() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  CHECK_EQ(v[0], 1);
  CHECK_EQ(v[6], 7);
  CHECK_EQ(v.at(3), 4);
}

static void vector_index_and_at_write_elements() {
  ft_vec v(3, 0);
  v[1] = 10;
  v.at(2) = 20;
  const int expected[] = {0, 10, 20};
  std_vec ref(expected, expected + 3);
  CHECK_SAME_SEQUENCE(ref, v);
}

static void vector_at_throws_out_of_range_for_bad_index() {
  ft_vec v(3, 0);
  CHECK_THROWS(v.at(3), std::out_of_range);
  CHECK_THROWS(v.at(100), std::out_of_range);

  ft_vec empty;
  CHECK_THROWS(empty.at(0), std::out_of_range);
}

static void vector_front_and_back_return_ends() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  CHECK_EQ(v.front(), 1);
  CHECK_EQ(v.back(), 7);
  v.front() = 11;
  v.back() = 77;
  CHECK_EQ(v[0], 11);
  CHECK_EQ(v[6], 77);
}

static void vector_const_access_works_through_const_reference() {
  ft_vec storage(kSeq, kSeq + kSeqLen);
  const ft_vec &v = storage;
  CHECK_EQ(v[1], 2);
  CHECK_EQ(v.at(1), 2);
  CHECK_EQ(v.front(), 1);
  CHECK_EQ(v.back(), 7);

  ft_vec::const_iterator it = v.begin();
  CHECK_EQ(*it, 1);
  CHECK_EQ(static_cast<long>(v.end() - v.begin()), static_cast<long>(kSeqLen));
}

// ---------------------------------------------------------------------------
// 반복자
// ---------------------------------------------------------------------------

static void vector_iterators_traverse_in_order() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std_vec visited;
  for (ft_vec::iterator it = v.begin(); it != v.end(); ++it)
    visited.push_back(*it);
  std_vec ref(kSeq, kSeq + kSeqLen);
  CHECK_SAME_SEQUENCE(ref, visited);
}

static void vector_iterator_is_random_access() {
  CHECK_SAME_TYPE(ft::iterator_traits<ft_vec::iterator>::iterator_category,
                  std::random_access_iterator_tag);
  ft_vec v(kSeq, kSeq + kSeqLen);
  ft_vec::iterator it = v.begin();
  CHECK_EQ(*(it + 3), 4);
  CHECK_EQ(it[5], 6);
  CHECK_EQ(v.end() - v.begin(), static_cast<long>(kSeqLen));
  it += 2;
  CHECK_EQ(*it, 3);
  it -= 1;
  CHECK_EQ(*it, 2);
  CHECK(v.begin() < v.end());
}

static void vector_iterator_converts_to_const_iterator() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  ft_vec::iterator it = v.begin();
  ft_vec::const_iterator cit = it;
  CHECK(cit == it);
  CHECK_EQ(*cit, 1);
}

static void vector_iterators_work_with_std_algorithms() {
  const int unsorted[] = {5, 3, 9, 1, 7};
  ft_vec v(unsorted, unsorted + 5);

  std::sort(v.begin(), v.end());
  const int sorted[] = {1, 3, 5, 7, 9};
  std_vec ref(sorted, sorted + 5);
  CHECK_SAME_SEQUENCE(ref, v);

  std::reverse(v.begin(), v.end());
  CHECK_EQ(v.front(), 9);

  ft_vec::iterator found = std::find(v.begin(), v.end(), 5);
  CHECK(found != v.end());
  CHECK_EQ(found - v.begin(), 2L);

  ft_vec copied;
  std::copy(v.begin(), v.end(), std::back_inserter(copied));
  CHECK_SAME_SEQUENCE(v, copied);
}

static void vector_empty_vector_iterates_nothing() {
  ft_vec v;
  int visits = 0;
  for (ft_vec::iterator it = v.begin(); it != v.end(); ++it)
    ++visits;
  CHECK_EQ(visits, 0);
  CHECK(v.rbegin() == v.rend());
}

// ---------------------------------------------------------------------------
// 용량
// ---------------------------------------------------------------------------

static void vector_reserve_grows_capacity_and_keeps_elements() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std_vec ref(kSeq, kSeq + kSeqLen);
  v.reserve(100);
  CHECK(v.capacity() >= 100);
  CHECK_SIZE(v, kSeqLen);
  CHECK_SAME_SEQUENCE(ref, v);
}

static void vector_reserve_below_capacity_changes_nothing() {
  ft_vec v;
  v.reserve(50);
  std::size_t capacity = v.capacity();
  ft_vec::iterator begin = v.begin();
  v.reserve(10);
  CHECK_EQ(v.capacity(), capacity);
  CHECK(v.begin() == begin); // 재할당이 없었습니다.
}

static void vector_reserve_beyond_max_size_throws_length_error() {
  ft_vec v;
  CHECK_THROWS(v.reserve(v.max_size() + 1), std::length_error);
}

static void vector_max_size_is_large_and_positive() {
  ft_vec v(3, 0);
  CHECK(v.max_size() > 0);
  CHECK(v.max_size() >= v.size());
  CHECK(v.max_size() > 1000000);
}

static void vector_resize_grows_with_value_and_shrinks() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);

  ref.resize(10, 42);
  mine.resize(10, 42);
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.resize(3);
  mine.resize(3);
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.resize(6); // 기본값 0 으로 채워집니다.
  mine.resize(6);
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.resize(0);
  mine.resize(0);
  CHECK(mine.empty());
  CHECK(mine.capacity() >= 6);
}

// push_back 은 분할 상환 O(1) 이어야 하므로, 재할당이 매번 일어나면 안 됩니다.
static void vector_push_back_reallocates_geometrically() {
  ft_vec v;
  std::size_t last_capacity = v.capacity();
  int reallocations = 0;
  for (int i = 0; i < 1000; ++i) {
    v.push_back(i);
    if (v.capacity() != last_capacity) {
      ++reallocations;
      last_capacity = v.capacity();
    }
    CHECK(v.capacity() >= v.size());
    if (test::current_test_failed())
      break;
  }
  CHECK_SIZE(v, 1000);
  CHECK(reallocations <= 20);
  CHECK(reallocations >= 1);
}

// ---------------------------------------------------------------------------
// 수정
// ---------------------------------------------------------------------------

static void vector_push_back_appends_in_order() {
  std_vec ref;
  ft_vec mine;
  for (int i = 0; i < 100; ++i) {
    ref.push_back(i * i);
    mine.push_back(i * i);
  }
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine.back(), 99 * 99);
}

static void vector_pop_back_removes_last_element() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std::size_t capacity = v.capacity();
  v.pop_back();
  v.pop_back();
  std_vec ref(kSeq, kSeq + kSeqLen - 2);
  CHECK_SAME_SEQUENCE(ref, v);
  CHECK_EQ(v.capacity(), capacity); // pop_back 은 용량을 줄이지 않습니다.
}

static void vector_clear_empties_but_keeps_capacity() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std::size_t capacity = v.capacity();
  v.clear();
  CHECK(v.empty());
  CHECK_EQ(v.capacity(), capacity);
  v.push_back(1); // clear 뒤에도 정상적으로 쓸 수 있어야 합니다.
  CHECK_SIZE(v, 1);
}

static void vector_assign_count_value_replaces_contents() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);

  ref.assign(3, 8); // 줄어드는 경우
  mine.assign(3, 8);
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.assign(20, 9); // 용량을 넘어 늘어나는 경우
  mine.assign(20, 9);
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_assign_range_replaces_contents() {
  std_vec ref(3, 0);
  ft_vec mine(3, 0);
  std::list<int> source(kSeq, kSeq + kSeqLen);

  ref.assign(source.begin(), source.end());
  mine.assign(source.begin(), source.end());
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.assign(kSeq, kSeq + 2);
  mine.assign(kSeq, kSeq + 2);
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_insert_single_in_middle_shifts_tail() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  ref.reserve(20);
  mine.reserve(20);

  std_vec::iterator ref_it = ref.insert(ref.begin() + 2, 99);
  ft_vec::iterator mine_it = mine.insert(mine.begin() + 2, 99);

  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(*mine_it, 99);
  CHECK_EQ(mine_it - mine.begin(), ref_it - ref.begin());
}

static void vector_insert_single_at_begin_and_end() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);

  ref.insert(ref.begin(), 0);
  mine.insert(mine.begin(), 0);
  ref.insert(ref.end(), 8);
  mine.insert(mine.end(), 8);

  CHECK_SAME_SEQUENCE(ref, mine);
}

// 재할당이 일어나면 insert 가 돌려주는 반복자는 새 저장소를 가리켜야 합니다.
static void vector_insert_that_reallocates_returns_iterator_into_new_storage() {
  ft_vec v;
  v.reserve(3);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  CHECK_EQ(v.capacity(), v.size()); // 다음 insert 는 반드시 재할당합니다.

  ft_vec::iterator it = v.insert(v.begin() + 1, 99);
  CHECK(it == v.begin() + 1);
  CHECK(it >= v.begin() && it < v.end());
  CHECK_EQ(*it, 99);

  const int expected[] = {1, 99, 2, 3};
  std_vec ref(expected, expected + 4);
  CHECK_SAME_SEQUENCE(ref, v);
}

static void vector_insert_count_in_middle() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);

  // C++98 의 std::vector::insert(pos, n, val) 은 void 를 돌려주므로 위치는 직접 확인합니다.
  ref.insert(ref.begin() + 3, 4, 0);
  ft_vec::iterator mine_it = mine.insert(mine.begin() + 3, 4, 0);

  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine_it - mine.begin(), 3L);
  CHECK_EQ(*mine_it, 0);
}

static void vector_insert_count_at_end_appends() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  ref.insert(ref.end(), 3, 5);
  mine.insert(mine.end(), 3, 5);
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_insert_zero_count_changes_nothing() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec::iterator it = v.insert(v.begin() + 2, 0, 42);
  CHECK_SAME_SEQUENCE(ref, v);
  CHECK(it == v.begin() + 2);
}

static void vector_insert_range_in_middle() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  const int extra[] = {100, 200, 300};

  ref.insert(ref.begin() + 1, extra, extra + 3);
  ft_vec::iterator mine_it = mine.insert(mine.begin() + 1, extra, extra + 3);

  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine_it - mine.begin(), 1L);
  CHECK_EQ(*mine_it, 100);
}

static void vector_insert_range_from_bidirectional_iterators() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  std::list<int> extra(3, -1);

  ref.insert(ref.end(), extra.begin(), extra.end());
  mine.insert(mine.end(), extra.begin(), extra.end());
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.insert(ref.begin(), extra.begin(), extra.end());
  mine.insert(mine.begin(), extra.begin(), extra.end());
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_erase_single_shifts_tail_and_returns_next() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);

  std_vec::iterator ref_it = ref.erase(ref.begin() + 2);
  ft_vec::iterator mine_it = mine.erase(mine.begin() + 2);

  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(*mine_it, *ref_it);
  CHECK_EQ(mine_it - mine.begin(), ref_it - ref.begin());
}

static void vector_erase_first_element() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  ref.erase(ref.begin());
  mine.erase(mine.begin());
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_erase_last_element_returns_end() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);
  ref.erase(ref.end() - 1);
  ft_vec::iterator it = mine.erase(mine.end() - 1);
  CHECK(it == mine.end());
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void vector_erase_range_removes_elements() {
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec mine(kSeq, kSeq + kSeqLen);

  std_vec::iterator ref_it = ref.erase(ref.begin() + 1, ref.begin() + 4);
  ft_vec::iterator mine_it = mine.erase(mine.begin() + 1, mine.begin() + 4);

  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(*mine_it, *ref_it);
}

static void vector_erase_whole_range_empties_vector() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  ft_vec::iterator it = v.erase(v.begin(), v.end());
  CHECK(v.empty());
  CHECK(it == v.end());
}

static void vector_erase_empty_range_changes_nothing() {
  ft_vec v(kSeq, kSeq + kSeqLen);
  std_vec ref(kSeq, kSeq + kSeqLen);
  ft_vec::iterator it = v.erase(v.begin() + 2, v.begin() + 2);
  CHECK_SAME_SEQUENCE(ref, v);
  CHECK(it == v.begin() + 2);
}

// swap 은 저장소를 통째로 맞바꾸므로, 기존 반복자는 상대 컨테이너에서 계속 유효해야 합니다.
static void vector_swap_exchanges_storage_and_keeps_iterators_valid() {
  ft_vec a(3, 1);
  ft_vec b(5, 2);
  ft_vec::iterator into_a = a.begin();

  a.swap(b);

  CHECK_SIZE(a, 5);
  CHECK_SIZE(b, 3);
  CHECK_EQ(a[0], 2);
  CHECK_EQ(b[0], 1);
  CHECK(into_a == b.begin());
  CHECK_EQ(*into_a, 1);
}

// ---------------------------------------------------------------------------
// 할당자와 할당 횟수
//
// 평가지는 깊은 복사가 allocate 한 번으로 끝나는지, swap 이 데이터를 옮기지 않고
// 저장소만 바꾸는지 확인합니다. 횟수를 세는 allocator 를 템플릿 인자로 넘겨 확인합니다.
// ---------------------------------------------------------------------------

typedef ft::vector<int, test::CountingAllocator<int> > counting_vec;
typedef ft::vector<test::Counted, test::CountingAllocator<test::Counted> >
    counting_counted_vec;

static void vector_uses_allocator_template_argument_for_storage() {
  test::AllocationCount::reset();
  {
    counting_vec v;
    for (int i = 0; i < 10; ++i)
      v.push_back(i);
    CHECK(test::AllocationCount::allocations() >= 1);
  }
  CHECK_EQ(test::AllocationCount::deallocations(),
           test::AllocationCount::allocations());
}

static void vector_copy_constructor_allocates_exactly_once() {
  counting_vec src;
  for (int i = 0; i < 100; ++i)
    src.push_back(i); // 여러 번 재할당되어 capacity 가 size 보다 큽니다.

  test::AllocationCount::reset();
  counting_vec copy(src);

  CHECK_EQ(test::AllocationCount::allocations(), 1);
  CHECK_SAME_SEQUENCE(src, copy);
  CHECK(copy.capacity() >= copy.size());
}

static void vector_copy_assignment_allocates_once_only_when_capacity_is_short() {
  counting_vec src;
  for (int i = 0; i < 100; ++i)
    src.push_back(i);

  counting_vec small;
  test::AllocationCount::reset();
  small = src;
  CHECK_EQ(test::AllocationCount::allocations(), 1);
  CHECK_SAME_SEQUENCE(src, small);

  counting_vec roomy;
  roomy.reserve(200);
  test::AllocationCount::reset();
  roomy = src;
  CHECK_EQ(test::AllocationCount::allocations(), 0);
  CHECK_SAME_SEQUENCE(src, roomy);
  CHECK(roomy.capacity() >= 200);
}

static void vector_swap_exchanges_storage_without_allocating_or_copying() {
  counting_counted_vec a(3, test::Counted(1));
  counting_counted_vec b(5, test::Counted(2));
  test::Counted *a_storage = &a[0];
  test::Counted *b_storage = &b[0];

  test::AllocationCount::reset();
  const int copies = test::Counted::copies();
  const int live = test::Counted::live();

  a.swap(b);

  CHECK_EQ(test::AllocationCount::allocations(), 0);
  CHECK_EQ(test::AllocationCount::deallocations(), 0);
  CHECK_EQ(test::Counted::copies(), copies);
  CHECK_EQ(test::Counted::live(), live);
  CHECK(&a[0] == b_storage);
  CHECK(&b[0] == a_storage);
  CHECK_SIZE(a, 5);
  CHECK_SIZE(b, 3);
}

// ---------------------------------------------------------------------------
// 비교 연산자
// ---------------------------------------------------------------------------

static void vector_equality_compares_size_and_elements() {
  ft_vec a(kSeq, kSeq + kSeqLen);
  ft_vec b(kSeq, kSeq + kSeqLen);
  ft_vec shorter(kSeq, kSeq + kSeqLen - 1);
  ft_vec different(kSeq, kSeq + kSeqLen);
  different[3] = 0;

  CHECK(a == b);
  CHECK(!(a != b));
  CHECK(a != shorter);
  CHECK(a != different);
}

static void vector_relational_operators_are_lexicographic() {
  const int seqs[][3] = {{1, 2, 3}, {1, 2, 4}, {1, 2, 0}, {2, 0, 0}};
  const std::size_t lengths[] = {3, 3, 2, 1};
  const int n = sizeof(lengths) / sizeof(lengths[0]);

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      std_vec a(seqs[i], seqs[i] + lengths[i]);
      std_vec b(seqs[j], seqs[j] + lengths[j]);
      ft_vec x(seqs[i], seqs[i] + lengths[i]);
      ft_vec y(seqs[j], seqs[j] + lengths[j]);
      CHECK_EQ(x == y, a == b);
      CHECK_EQ(x != y, a != b);
      CHECK_EQ(x < y, a < b);
      CHECK_EQ(x <= y, a <= b);
      CHECK_EQ(x > y, a > b);
      CHECK_EQ(x >= y, a >= b);
    }
  }
}

// ---------------------------------------------------------------------------
// 무작위 연산을 std::vector 와 한 단계씩 비교합니다.
// ---------------------------------------------------------------------------

static void vector_random_operations_match_std() {
  test::Random rng(20240101);
  std_vec ref;
  ft_vec mine;

  for (int step = 0; step < 2000; ++step) {
    const int value = rng.below(1000);
    switch (rng.below(5)) {
    case 0:
    case 1:
      ref.push_back(value);
      mine.push_back(value);
      break;
    case 2:
      if (!ref.empty()) {
        ref.pop_back();
        mine.pop_back();
      }
      break;
    case 3: {
      const int pos = rng.below(static_cast<int>(ref.size()) + 1);
      ref.insert(ref.begin() + pos, value);
      mine.insert(mine.begin() + pos, value);
      break;
    }
    case 4:
      if (!ref.empty()) {
        const int pos = rng.below(static_cast<int>(ref.size()));
        ref.erase(ref.begin() + pos);
        mine.erase(mine.begin() + pos);
      }
      break;
    }
    CHECK_SAME_SEQUENCE(ref, mine);
    CHECK(mine.capacity() >= mine.size());
    if (test::current_test_failed())
      break; // 첫 불일치만 보고합니다.
  }
}

int main() {
  test::Suite suite("vector");

  RUN_TEST(suite, vector_default_constructor_is_empty);
  RUN_TEST(suite, vector_fill_constructor_repeats_value);
  RUN_TEST(suite, vector_fill_constructor_with_zero_count_is_empty);
  RUN_TEST(suite, vector_two_integer_arguments_pick_fill_constructor);
  RUN_TEST(suite, vector_range_constructor_copies_from_pointers);
  RUN_TEST(suite, vector_range_constructor_accepts_bidirectional_iterators);
  RUN_TEST(suite, vector_range_constructor_with_empty_range_is_empty);
  RUN_TEST(suite, vector_copy_constructor_makes_independent_copy);
  RUN_TEST(suite, vector_copy_assignment_replaces_contents);
  RUN_TEST(suite, vector_self_assignment_keeps_contents);

  RUN_TEST(suite, vector_index_and_at_read_elements);
  RUN_TEST(suite, vector_index_and_at_write_elements);
  RUN_TEST(suite, vector_at_throws_out_of_range_for_bad_index);
  RUN_TEST(suite, vector_front_and_back_return_ends);
  RUN_TEST(suite, vector_const_access_works_through_const_reference);

  RUN_TEST(suite, vector_iterators_traverse_in_order);
  RUN_TEST(suite, vector_iterator_is_random_access);
  RUN_TEST(suite, vector_iterator_converts_to_const_iterator);
  RUN_TEST(suite, vector_iterators_work_with_std_algorithms);
  RUN_TEST(suite, vector_empty_vector_iterates_nothing);

  RUN_TEST(suite, vector_reserve_grows_capacity_and_keeps_elements);
  RUN_TEST(suite, vector_reserve_below_capacity_changes_nothing);
  RUN_TEST(suite, vector_reserve_beyond_max_size_throws_length_error);
  RUN_TEST(suite, vector_max_size_is_large_and_positive);
  RUN_TEST(suite, vector_resize_grows_with_value_and_shrinks);
  RUN_TEST(suite, vector_push_back_reallocates_geometrically);

  RUN_TEST(suite, vector_push_back_appends_in_order);
  RUN_TEST(suite, vector_pop_back_removes_last_element);
  RUN_TEST(suite, vector_clear_empties_but_keeps_capacity);
  RUN_TEST(suite, vector_assign_count_value_replaces_contents);
  RUN_TEST(suite, vector_assign_range_replaces_contents);
  RUN_TEST(suite, vector_insert_single_in_middle_shifts_tail);
  RUN_TEST(suite, vector_insert_single_at_begin_and_end);
  RUN_TEST(suite, vector_insert_that_reallocates_returns_iterator_into_new_storage);
  RUN_TEST(suite, vector_insert_count_in_middle);
  RUN_TEST(suite, vector_insert_count_at_end_appends);
  RUN_TEST(suite, vector_insert_zero_count_changes_nothing);
  RUN_TEST(suite, vector_insert_range_in_middle);
  RUN_TEST(suite, vector_insert_range_from_bidirectional_iterators);
  RUN_TEST(suite, vector_erase_single_shifts_tail_and_returns_next);
  RUN_TEST(suite, vector_erase_first_element);
  RUN_TEST(suite, vector_erase_last_element_returns_end);
  RUN_TEST(suite, vector_erase_range_removes_elements);
  RUN_TEST(suite, vector_erase_whole_range_empties_vector);
  RUN_TEST(suite, vector_erase_empty_range_changes_nothing);
  RUN_TEST(suite, vector_swap_exchanges_storage_and_keeps_iterators_valid);

  RUN_TEST(suite, vector_uses_allocator_template_argument_for_storage);
  RUN_TEST(suite, vector_copy_constructor_allocates_exactly_once);
  RUN_TEST(suite, vector_copy_assignment_allocates_once_only_when_capacity_is_short);
  RUN_TEST(suite, vector_swap_exchanges_storage_without_allocating_or_copying);

  RUN_TEST(suite, vector_equality_compares_size_and_elements);
  RUN_TEST(suite, vector_relational_operators_are_lexicographic);

  RUN_TEST(suite, vector_random_operations_match_std);

  return suite.result();
}
