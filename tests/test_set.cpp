#include "test.hpp"
#include "set.hpp"
#include "vector.hpp"

#include <functional>
#include <iterator>
#include <set>
#include <string>
#include <vector>

typedef std::set<int> std_set;
typedef ft::set<int> ft_set;

static const int kValues[] = {10, 5, 15, 17, 19, 21, 23, 3, 8, 1, 12, 30};
static const int kValueCount = sizeof(kValues) / sizeof(kValues[0]);

static ft_set make_set() { return ft_set(kValues, kValues + kValueCount); }

template <class Set> static bool set_tree_is_valid(Set &s) {
  return test::rb_tree_is_valid(s.end().base().base());
}

// ---------------------------------------------------------------------------
// 생성, 복사, 대입
// ---------------------------------------------------------------------------

static void set_default_constructor_is_empty() {
  ft_set s;
  CHECK(s.empty());
  CHECK_SIZE(s, 0);
  CHECK(s.begin() == s.end());
  CHECK(set_tree_is_valid(s));
}

static void set_member_types_follow_the_standard() {
  CHECK_SAME_TYPE(ft_set::key_type, int);
  CHECK_SAME_TYPE(ft_set::value_type, int);
  CHECK_SAME_TYPE(ft_set::key_compare, std::less<int>);
  CHECK_SAME_TYPE(ft_set::value_compare, std::less<int>);
  CHECK_SAME_TYPE(ft_set::iterator::iterator_category,
                  std::bidirectional_iterator_tag);
}

// set 의 원소는 정렬 키이므로 반복자를 통해 바꿀 수 없어야 합니다.
static void set_iterator_only_gives_const_access() {
  CHECK_SAME_TYPE(ft_set::iterator::reference, const int &);
  CHECK_SAME_TYPE(ft_set::iterator::pointer, const int *);
  CHECK_SAME_TYPE(ft_set::const_iterator::reference, const int &);
}

static void set_range_constructor_inserts_every_value() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine(kValues, kValues + kValueCount);
  CHECK_SAME_SEQUENCE(ref, mine);

  std::vector<int> duplicated(kValues, kValues + kValueCount);
  duplicated.insert(duplicated.end(), kValues, kValues + kValueCount);
  ft_set from_duplicates(duplicated.begin(), duplicated.end());
  CHECK_SAME_SEQUENCE(ref, from_duplicates);
}

static void set_copy_constructor_makes_independent_copy() {
  ft_set original = make_set();
  ft_set copy(original);
  CHECK_SAME_SEQUENCE(original, copy);
  CHECK(set_tree_is_valid(copy));

  copy.erase(10);
  copy.insert(99);
  CHECK(original.find(10) != original.end());
  CHECK(original.find(99) == original.end());
}

static void set_assignment_replaces_contents() {
  ft_set source = make_set();
  ft_set target;
  target.insert(-1);
  target = source;
  CHECK_SAME_SEQUENCE(source, target);
  CHECK(target.find(-1) == target.end());
}

// ---------------------------------------------------------------------------
// 삽입
// ---------------------------------------------------------------------------

static void set_insert_new_value_returns_iterator_and_true() {
  ft_set s;
  ft::pair<ft_set::iterator, bool> result = s.insert(42);
  CHECK(result.second);
  CHECK_EQ(*result.first, 42);
  CHECK_SIZE(s, 1);
}

static void set_insert_duplicate_returns_existing_and_false() {
  ft_set s = make_set();
  ft::pair<ft_set::iterator, bool> result = s.insert(15);
  CHECK(!result.second);
  CHECK(result.first == s.find(15));
  CHECK_SIZE(s, kValueCount);
}

static void set_insert_with_hint_returns_iterator_to_value() {
  ft_set s = make_set();
  ft_set::iterator inserted = s.insert(s.find(12), 11);
  CHECK_EQ(*inserted, 11);
  inserted = s.insert(s.begin(), 25); // 틀린 힌트
  CHECK_EQ(*inserted, 25);
  inserted = s.insert(s.end(), 12); // 이미 있는 값
  CHECK(inserted == s.find(12));
  CHECK_SIZE(s, kValueCount + 2);
  CHECK(test::strictly_sorted(s.begin(), s.end(), std::less<int>()));
  CHECK(set_tree_is_valid(s));
}

static void set_insert_range_adds_only_new_values() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine = make_set();
  const int extra[] = {5, 6, 7, 30, 31};
  ref.insert(extra, extra + 5);
  mine.insert(extra, extra + 5);
  CHECK_SAME_SEQUENCE(ref, mine);
}

// ---------------------------------------------------------------------------
// 순회
// ---------------------------------------------------------------------------

static void set_iterates_in_ascending_order() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine = make_set();
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(test::strictly_sorted(mine.begin(), mine.end(), std::less<int>()));
}

static void set_iterator_moves_both_directions() {
  ft_set s = make_set();
  ft_set::iterator it = s.begin();
  ++it;
  CHECK_EQ(*it, 3);
  it++;
  CHECK_EQ(*it, 5);
  --it;
  CHECK_EQ(*it, 3);
  it--;
  CHECK(it == s.begin());
  ft_set::iterator last = s.end();
  --last;
  CHECK_EQ(*last, 30);
}

static void set_reverse_iterators_walk_descending() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine = make_set();
  std::vector<int> expected(ref.rbegin(), ref.rend());
  std::vector<int> actual;
  for (ft_set::reverse_iterator r = mine.rbegin(); r != mine.rend(); ++r)
    actual.push_back(*r);
  CHECK_SAME_SEQUENCE(expected, actual);
}

static void set_const_iteration_through_const_reference() {
  ft_set s = make_set();
  const ft_set &view = s;
  std::vector<int> through_const(view.begin(), view.end());
  std::vector<int> through_mutable(s.begin(), s.end());
  CHECK_SAME_SEQUENCE(through_mutable, through_const);

  ft_set::const_iterator converted = s.begin();
  CHECK(converted == view.begin());
  CHECK(view.find(5) != view.end());
  CHECK_EQ(view.count(5), std::size_t(1));
  CHECK_EQ(*view.lower_bound(4), 5);
  CHECK_EQ(*view.upper_bound(5), 8);
}

static void set_iterators_work_with_std_distance_and_advance() {
  ft_set s = make_set();
  CHECK_EQ(std::distance(s.begin(), s.end()), static_cast<long>(kValueCount));
  ft_set::iterator it = s.begin();
  std::advance(it, 4);
  CHECK_EQ(*it, 10);
}

// ---------------------------------------------------------------------------
// 탐색
// ---------------------------------------------------------------------------

static void set_find_and_count_distinguish_present_and_missing_values() {
  ft_set s = make_set();
  CHECK(s.find(17) != s.end());
  CHECK(s.find(18) == s.end());
  CHECK_EQ(s.count(17), std::size_t(1));
  CHECK_EQ(s.count(18), std::size_t(0));
}

static void set_bounds_and_equal_range_match_std_for_every_value() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine = make_set();

  for (int value = -1; value <= 32; ++value) {
    std_set::iterator ref_lower = ref.lower_bound(value);
    ft_set::iterator mine_lower = mine.lower_bound(value);
    CHECK_EQ(mine_lower == mine.end(), ref_lower == ref.end());
    if (ref_lower != ref.end() && mine_lower != mine.end())
      CHECK_EQ(*mine_lower, *ref_lower);

    std_set::iterator ref_upper = ref.upper_bound(value);
    ft_set::iterator mine_upper = mine.upper_bound(value);
    CHECK_EQ(mine_upper == mine.end(), ref_upper == ref.end());
    if (ref_upper != ref.end() && mine_upper != mine.end())
      CHECK_EQ(*mine_upper, *ref_upper);

    ft::pair<ft_set::iterator, ft_set::iterator> range =
        mine.equal_range(value);
    CHECK(range.first == mine_lower);
    CHECK(range.second == mine_upper);
  }
}

// ---------------------------------------------------------------------------
// 삭제
// ---------------------------------------------------------------------------

static void set_erase_by_value_returns_number_of_removed_elements() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine = make_set();
  CHECK_EQ(mine.erase(15), std::size_t(1));
  ref.erase(15);
  CHECK_EQ(mine.erase(15), std::size_t(0));
  CHECK_EQ(mine.erase(1000), std::size_t(0));
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(set_tree_is_valid(mine));
}

static void set_erase_by_iterator_and_range() {
  std_set ref(kValues, kValues + kValueCount);
  ft_set mine = make_set();

  ref.erase(ref.find(8));
  mine.erase(mine.find(8));
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.erase(ref.find(5), ref.find(19));
  mine.erase(mine.find(5), mine.find(19));
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(mine.find(19) != mine.end());
  CHECK(set_tree_is_valid(mine));
}

static void set_erase_everything_then_reuse() {
  ft_set s = make_set();
  s.erase(s.begin(), s.end());
  CHECK(s.empty());
  CHECK(s.begin() == s.end());
  s.insert(1);
  CHECK_SIZE(s, 1);
  CHECK_EQ(*s.begin(), 1);
}

static void set_clear_empties_set() {
  ft_set s = make_set();
  s.clear();
  CHECK(s.empty());
  CHECK(s.begin() == s.end());
  CHECK(set_tree_is_valid(s));
}

static void set_swap_exchanges_contents_and_keeps_iterators_valid() {
  ft_set a(kValues, kValues + 3);
  ft_set b(kValues + 3, kValues + kValueCount);
  ft_set::iterator into_a = a.begin();
  const int first_of_a = *into_a;

  a.swap(b);

  CHECK_SIZE(a, kValueCount - 3);
  CHECK_SIZE(b, 3);
  CHECK_EQ(*into_a, first_of_a);
  CHECK(into_a == b.begin());
}

// ---------------------------------------------------------------------------
// 비교와 비교자
// ---------------------------------------------------------------------------

static void set_relational_operators_match_std() {
  std_set ref_a(kValues, kValues + kValueCount);
  std_set ref_b(kValues, kValues + kValueCount - 1);
  std_set ref_c(kValues, kValues + kValueCount);
  ref_c.erase(1);
  ft_set mine_a = make_set();
  ft_set mine_b(kValues, kValues + kValueCount - 1);
  ft_set mine_c = make_set();
  mine_c.erase(1);

  CHECK(mine_a == mine_a);
  CHECK_EQ(mine_a == mine_b, ref_a == ref_b);
  CHECK_EQ(mine_a != mine_b, ref_a != ref_b);
  CHECK_EQ(mine_a < mine_b, ref_a < ref_b);
  CHECK_EQ(mine_a <= mine_b, ref_a <= ref_b);
  CHECK_EQ(mine_a > mine_b, ref_a > ref_b);
  CHECK_EQ(mine_a >= mine_b, ref_a >= ref_b);
  CHECK_EQ(mine_a < mine_c, ref_a < ref_c);
  CHECK_EQ(mine_c < mine_a, ref_c < ref_a);
}

static void set_with_greater_comparator_iterates_descending() {
  typedef ft::set<int, std::greater<int> > desc_set;
  std::set<int, std::greater<int> > ref(kValues, kValues + kValueCount);
  desc_set mine(kValues, kValues + kValueCount);
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(*mine.begin(), 30);
  CHECK_EQ(*mine.lower_bound(16), 15);
  CHECK(mine.key_comp()(2, 1));
  CHECK(mine.value_comp()(2, 1));
  CHECK(test::rb_tree_is_valid(mine.end().base().base()));
}

static void set_of_strings_orders_lexicographically() {
  std::set<std::string> ref;
  ft::set<std::string> mine;
  const char *words[] = {"pear", "apple", "fig", "banana", "apple"};
  for (int i = 0; i < 5; ++i) {
    ref.insert(words[i]);
    mine.insert(words[i]);
  }
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(*mine.begin(), std::string("apple"));
  CHECK_SIZE(mine, 4);
}

static void set_max_size_is_positive() {
  ft_set s;
  CHECK(s.max_size() > 0);
  CHECK(s.max_size() > 1000000);
}

// ---------------------------------------------------------------------------
// 무작위 연산
// ---------------------------------------------------------------------------

static void set_random_operations_match_std_and_keep_tree_valid() {
  test::Random rng(2718);
  std_set ref;
  ft_set mine;

  for (int step = 0; step < 5000; ++step) {
    const int value = rng.below(400);
    if (rng.below(3) == 0)
      CHECK_EQ(mine.erase(value), ref.erase(value));
    else
      CHECK_EQ(mine.insert(value).second, ref.insert(value).second);
    CHECK_EQ(mine.size(), ref.size());
    if (step % 100 == 0) {
      CHECK_SAME_SEQUENCE(ref, mine);
      CHECK(set_tree_is_valid(mine));
    }
    if (test::current_test_failed())
      break;
  }
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(set_tree_is_valid(mine));
  CHECK(test::rb_tree_height(mine.end().base().base()->left) <=
        test::rb_max_height(mine.size()));
}

// ---------------------------------------------------------------------------
// 값 수명: 빈 set 을 만든 직후를 기준으로 셉니다.
// ---------------------------------------------------------------------------

typedef ft::set<test::Counted> counted_set;

static void set_destroys_values_on_erase_clear_and_destruction() {
  const int outside = test::Counted::live();
  {
    counted_set s;
    const int empty = test::Counted::live();
    for (int i = 0; i < 5; ++i)
      s.insert(test::Counted(i));
    CHECK_EQ(test::Counted::live() - empty, 5);

    s.erase(test::Counted(2));
    CHECK_EQ(test::Counted::live() - empty, 4);

    s.clear();
    CHECK_EQ(test::Counted::live() - empty, 0);
  }
  CHECK_EQ(test::Counted::live() - outside, 0);
}

static void set_rejected_duplicate_insert_does_not_leak_value() {
  counted_set s;
  const int empty = test::Counted::live();
  s.insert(test::Counted(1));
  s.insert(test::Counted(1));
  s.insert(s.begin(), test::Counted(1));
  CHECK_SIZE(s, 1);
  CHECK_EQ(test::Counted::live() - empty, 1);
}

// ---------------------------------------------------------------------------
// 반복자 유효성과 swap
//
// map 과 같은 평가지 항목입니다. insert 와 erase 가 기존 반복자를 무효화하지 않는지,
// swap 이 원소를 옮기지 않고 노드만 바꾸는지 원소의 주소로 확인합니다.
// ---------------------------------------------------------------------------

static void set_insert_keeps_existing_iterators_and_references_valid() {
  ft_set s;
  std::vector<ft_set::iterator> kept;
  std::vector<const int *> addresses;
  for (int value = 0; value < 100; value += 2) {
    kept.push_back(s.insert(value).first);
    addresses.push_back(&*kept.back());
  }

  // 사이사이에 홀수 값을 넣어 트리가 여러 번 회전하게 합니다.
  for (int value = 1; value < 100; value += 2)
    s.insert(value);
  CHECK_SIZE(s, 100);
  CHECK(set_tree_is_valid(s));

  for (std::size_t i = 0; i < kept.size(); ++i) {
    CHECK_EQ(*kept[i], static_cast<int>(i) * 2);
    CHECK(&*kept[i] == addresses[i]);
  }

  // 기존 반복자에서 ++ 하면 새로 들어온 홀수 값이 보여야 합니다.
  ft_set::iterator next = kept[0];
  ++next;
  CHECK_EQ(*next, 1);
}

static void set_erase_keeps_iterators_to_remaining_elements_valid() {
  ft_set s;
  for (int value = 0; value < 100; ++value)
    s.insert(value);

  std::vector<ft_set::iterator> kept;
  std::vector<const int *> addresses;
  for (int value = 0; value < 100; value += 2) {
    kept.push_back(s.find(value));
    addresses.push_back(&*kept.back());
  }

  for (int value = 1; value < 100; value += 2)
    CHECK_EQ(s.erase(value), std::size_t(1));
  CHECK_SIZE(s, 50);
  CHECK(set_tree_is_valid(s));

  ft_set::iterator walk = s.begin();
  for (std::size_t i = 0; i < kept.size(); ++i, ++walk) {
    CHECK_EQ(*kept[i], static_cast<int>(i) * 2);
    CHECK(&*kept[i] == addresses[i]);
    CHECK(walk == kept[i]);
  }
  CHECK(walk == s.end());

  // 반복자로 지운 뒤에도 이웃 반복자는 그대로 쓸 수 있습니다.
  s.erase(kept[10]);
  ft_set::iterator after_gap = kept[9];
  ++after_gap;
  CHECK(after_gap == kept[11]);
}

static void set_swap_exchanges_nodes_without_copying_elements() {
  counted_set a;
  counted_set b;
  for (int i = 0; i < 5; ++i)
    a.insert(test::Counted(i));
  for (int i = 10; i < 13; ++i)
    b.insert(test::Counted(i));
  const test::Counted *first_of_a = &*a.begin();
  const test::Counted *first_of_b = &*b.begin();
  const int copies = test::Counted::copies();
  const int live = test::Counted::live();

  a.swap(b);

  CHECK_EQ(test::Counted::copies(), copies);
  CHECK_EQ(test::Counted::live(), live);
  CHECK(&*a.begin() == first_of_b);
  CHECK(&*b.begin() == first_of_a);
  CHECK_SIZE(a, 3);
  CHECK_SIZE(b, 5);
}

int main() {
  test::Suite suite("set");

  RUN_TEST(suite, set_default_constructor_is_empty);
  RUN_TEST(suite, set_member_types_follow_the_standard);
  RUN_TEST(suite, set_iterator_only_gives_const_access);
  RUN_TEST(suite, set_range_constructor_inserts_every_value);
  RUN_TEST(suite, set_copy_constructor_makes_independent_copy);
  RUN_TEST(suite, set_assignment_replaces_contents);

  RUN_TEST(suite, set_insert_new_value_returns_iterator_and_true);
  RUN_TEST(suite, set_insert_duplicate_returns_existing_and_false);
  RUN_TEST(suite, set_insert_with_hint_returns_iterator_to_value);
  RUN_TEST(suite, set_insert_range_adds_only_new_values);

  RUN_TEST(suite, set_iterates_in_ascending_order);
  RUN_TEST(suite, set_iterator_moves_both_directions);
  RUN_TEST(suite, set_reverse_iterators_walk_descending);
  RUN_TEST(suite, set_const_iteration_through_const_reference);
  RUN_TEST(suite, set_iterators_work_with_std_distance_and_advance);

  RUN_TEST(suite, set_find_and_count_distinguish_present_and_missing_values);
  RUN_TEST(suite, set_bounds_and_equal_range_match_std_for_every_value);

  RUN_TEST(suite, set_erase_by_value_returns_number_of_removed_elements);
  RUN_TEST(suite, set_erase_by_iterator_and_range);
  RUN_TEST(suite, set_erase_everything_then_reuse);
  RUN_TEST(suite, set_clear_empties_set);
  RUN_TEST(suite, set_swap_exchanges_contents_and_keeps_iterators_valid);

  RUN_TEST(suite, set_relational_operators_match_std);
  RUN_TEST(suite, set_with_greater_comparator_iterates_descending);
  RUN_TEST(suite, set_of_strings_orders_lexicographically);
  RUN_TEST(suite, set_max_size_is_positive);

  RUN_TEST(suite, set_random_operations_match_std_and_keep_tree_valid);

  RUN_TEST(suite, set_destroys_values_on_erase_clear_and_destruction);
  RUN_TEST(suite, set_rejected_duplicate_insert_does_not_leak_value);

  RUN_TEST(suite, set_insert_keeps_existing_iterators_and_references_valid);
  RUN_TEST(suite, set_erase_keeps_iterators_to_remaining_elements_valid);
  RUN_TEST(suite, set_swap_exchanges_nodes_without_copying_elements);

  return suite.result();
}
