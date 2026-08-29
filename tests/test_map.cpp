#include "test.hpp"
#include "map.hpp"
#include "vector.hpp"

#include <functional>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>

typedef std::map<int, std::string> std_map;
typedef ft::map<int, std::string> ft_map;
typedef ft::pair<int, std::string> entry;

static const int kKeys[] = {10, 5, 15, 17, 19, 21, 23, 3, 8, 1, 12, 30};
static const int kKeyCount = sizeof(kKeys) / sizeof(kKeys[0]);

static std::string label(int key) {
  std::ostringstream os;
  os << "value-" << key;
  return os.str();
}

static void fill(std_map &ref, ft_map &mine) {
  for (int i = 0; i < kKeyCount; ++i) {
    ref.insert(std::make_pair(kKeys[i], label(kKeys[i])));
    mine.insert(ft::make_pair(kKeys[i], label(kKeys[i])));
  }
}

static ft_map make_map() {
  std_map unused;
  ft_map mine;
  fill(unused, mine);
  return mine;
}

template <class Map> static bool map_tree_is_valid(Map &m) {
  return test::rb_tree_is_valid(m.end().base().base());
}

template <class Map> static std::vector<int> keys_of(const Map &m) {
  std::vector<int> keys;
  for (typename Map::const_iterator it = m.begin(); it != m.end(); ++it)
    keys.push_back(it->first);
  return keys;
}

// ---------------------------------------------------------------------------
// 생성, 복사, 대입
// ---------------------------------------------------------------------------

static void map_default_constructor_is_empty() {
  ft_map m;
  CHECK(m.empty());
  CHECK_SIZE(m, 0);
  CHECK(m.begin() == m.end());
  CHECK(map_tree_is_valid(m));
}

static void map_member_types_follow_the_standard() {
  typedef ft::pair<const int, std::string> expected_value_type;
  CHECK_SAME_TYPE(ft_map::key_type, int);
  CHECK_SAME_TYPE(ft_map::mapped_type, std::string);
  CHECK_SAME_TYPE(ft_map::value_type, expected_value_type);
  CHECK_SAME_TYPE(ft_map::key_compare, std::less<int>);
  CHECK_SAME_TYPE(ft_map::iterator::iterator_category,
                  std::bidirectional_iterator_tag);
}

static void map_range_constructor_inserts_every_pair() {
  ft::vector<entry> source;
  for (int i = 0; i < kKeyCount; ++i)
    source.push_back(entry(kKeys[i], label(kKeys[i])));
  ft_map mine(source.begin(), source.end());

  std_map ref;
  ft_map unused;
  fill(ref, unused);
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void map_copy_constructor_makes_independent_copy() {
  ft_map original = make_map();
  ft_map copy(original);
  CHECK_SAME_SEQUENCE(original, copy);
  CHECK(map_tree_is_valid(copy));

  copy[5] = "changed";
  copy.erase(10);
  copy[99] = "new";
  CHECK_EQ(original.find(5)->second, label(5));
  CHECK(original.find(10) != original.end());
  CHECK(original.find(99) == original.end());
}

static void map_assignment_replaces_contents() {
  ft_map source = make_map();
  ft_map target;
  target[-1] = "stale";
  target = source;
  CHECK_SAME_SEQUENCE(source, target);
  CHECK(target.find(-1) == target.end());

  ft_map &alias = target;
  target = alias;
  CHECK_SAME_SEQUENCE(source, target);
}

// ---------------------------------------------------------------------------
// 삽입
// ---------------------------------------------------------------------------

static void map_insert_new_key_returns_iterator_and_true() {
  ft_map m;
  ft::pair<ft_map::iterator, bool> result = m.insert(entry(1, "one"));
  CHECK(result.second);
  CHECK_EQ(result.first->first, 1);
  CHECK_EQ(result.first->second, std::string("one"));
  CHECK_SIZE(m, 1);
}

static void map_insert_duplicate_key_keeps_original_and_returns_false() {
  ft_map m;
  m.insert(entry(1, "one"));
  ft::pair<ft_map::iterator, bool> result = m.insert(entry(1, "uno"));
  CHECK(!result.second);
  CHECK_EQ(result.first->second, std::string("one"));
  CHECK(result.first == m.find(1));
  CHECK_SIZE(m, 1);
}

static void map_insert_with_hint_returns_iterator_to_element() {
  ft_map m = make_map();
  ft_map::iterator hint = m.find(12);
  ft_map::iterator inserted = m.insert(hint, entry(11, "eleven"));
  CHECK_EQ(inserted->first, 11);
  CHECK_SIZE(m, kKeyCount + 1);

  inserted = m.insert(m.begin(), entry(25, "wrong hint")); // 틀린 힌트
  CHECK_EQ(inserted->first, 25);

  inserted = m.insert(m.end(), entry(12, "duplicate")); // 이미 있는 키
  CHECK_EQ(inserted->second, label(12));
  CHECK_SIZE(m, kKeyCount + 2);
  CHECK(test::strictly_sorted(m.begin(), m.end(), m.value_comp()));
  CHECK(map_tree_is_valid(m));
}

static void map_insert_range_adds_only_new_keys() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  std::vector<std::pair<int, std::string> > ref_extra;
  ft::vector<entry> mine_extra;
  const int extra_keys[] = {5, 6, 7, 30, 31};
  for (int i = 0; i < 5; ++i) {
    ref_extra.push_back(std::make_pair(extra_keys[i], std::string("extra")));
    mine_extra.push_back(entry(extra_keys[i], "extra"));
  }
  ref.insert(ref_extra.begin(), ref_extra.end());
  mine.insert(mine_extra.begin(), mine_extra.end());
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine.find(5)->second, label(5)); // 기존 키의 값은 유지됩니다.
}

static void map_subscript_inserts_default_value_for_missing_key() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  CHECK(ref[100].empty());
  CHECK(mine[100].empty());
  CHECK_SIZE(mine, kKeyCount + 1);
  CHECK_SAME_SEQUENCE(ref, mine);
}

static void map_subscript_returns_reference_to_existing_value() {
  ft_map m = make_map();
  m[5] = "five";
  CHECK_EQ(m.find(5)->second, std::string("five"));
  m[5] += "!";
  CHECK_EQ(m[5], std::string("five!"));
  CHECK_SIZE(m, kKeyCount);
}

// ---------------------------------------------------------------------------
// 순회
// ---------------------------------------------------------------------------

static void map_iterates_keys_in_ascending_order() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(test::strictly_sorted(mine.begin(), mine.end(), mine.value_comp()));
}

static void map_iterator_arrow_and_star_give_key_and_mutable_value() {
  ft_map m = make_map();
  ft_map::iterator it = m.begin();
  CHECK_EQ(it->first, 1);
  CHECK_EQ((*it).second, label(1));
  it->second = "rewritten";
  CHECK_EQ(m[1], std::string("rewritten"));
}

static void map_iterator_moves_both_directions() {
  ft_map m = make_map();
  ft_map::iterator it = m.begin();
  ++it;
  CHECK_EQ(it->first, 3);
  it++;
  CHECK_EQ(it->first, 5);
  --it;
  CHECK_EQ(it->first, 3);
  it--;
  CHECK(it == m.begin());

  ft_map::iterator last = m.end();
  --last;
  CHECK_EQ(last->first, 30);
}

static void map_reverse_iterators_walk_descending() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  std::vector<int> expected;
  for (std_map::reverse_iterator r = ref.rbegin(); r != ref.rend(); ++r)
    expected.push_back(r->first);

  std::vector<int> actual;
  for (ft_map::reverse_iterator r = mine.rbegin(); r != mine.rend(); ++r)
    actual.push_back(r->first);

  CHECK_SAME_SEQUENCE(expected, actual);
}

static void map_const_iteration_through_const_reference() {
  ft_map m = make_map();
  const ft_map &view = m;

  std::vector<int> through_const = keys_of(view);
  std::vector<int> through_mutable = keys_of(m);
  CHECK_SAME_SEQUENCE(through_mutable, through_const);

  ft_map::const_iterator converted = m.begin();
  CHECK(converted == view.begin());
  CHECK_EQ(converted->first, 1);
  CHECK_EQ(view.find(5)->second, label(5));
  CHECK_EQ(view.count(5), std::size_t(1));
  CHECK_EQ(view.lower_bound(4)->first, 5);
  CHECK_EQ(view.upper_bound(5)->first, 8);
}

static void map_iterators_work_with_std_distance_and_advance() {
  ft_map m = make_map();
  CHECK_EQ(std::distance(m.begin(), m.end()), static_cast<long>(kKeyCount));
  ft_map::iterator it = m.begin();
  std::advance(it, 4);
  CHECK_EQ(it->first, 10);
}

// ---------------------------------------------------------------------------
// 탐색
// ---------------------------------------------------------------------------

static void map_find_and_count_distinguish_present_and_missing_keys() {
  ft_map m = make_map();
  CHECK(m.find(17) != m.end());
  CHECK_EQ(m.find(17)->second, label(17));
  CHECK(m.find(18) == m.end());
  CHECK_EQ(m.count(17), std::size_t(1));
  CHECK_EQ(m.count(18), std::size_t(0));
}

static void map_bounds_and_equal_range_match_std_for_every_key() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  for (int key = -1; key <= 32; ++key) {
    std_map::iterator ref_lower = ref.lower_bound(key);
    ft_map::iterator mine_lower = mine.lower_bound(key);
    CHECK_EQ(mine_lower == mine.end(), ref_lower == ref.end());
    if (ref_lower != ref.end() && mine_lower != mine.end())
      CHECK_EQ(mine_lower->first, ref_lower->first);

    std_map::iterator ref_upper = ref.upper_bound(key);
    ft_map::iterator mine_upper = mine.upper_bound(key);
    CHECK_EQ(mine_upper == mine.end(), ref_upper == ref.end());
    if (ref_upper != ref.end() && mine_upper != mine.end())
      CHECK_EQ(mine_upper->first, ref_upper->first);

    ft::pair<ft_map::iterator, ft_map::iterator> range = mine.equal_range(key);
    CHECK(range.first == mine_lower);
    CHECK(range.second == mine_upper);
  }
}

// ---------------------------------------------------------------------------
// 삭제
// ---------------------------------------------------------------------------

static void map_erase_by_key_returns_number_of_removed_elements() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  CHECK_EQ(mine.erase(15), std::size_t(1));
  ref.erase(15);
  CHECK_EQ(mine.erase(15), std::size_t(0));
  CHECK_EQ(mine.erase(1000), std::size_t(0));
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(map_tree_is_valid(mine));
}

static void map_erase_by_iterator_removes_that_element() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  ref.erase(ref.find(8));
  mine.erase(mine.find(8));
  CHECK_SAME_SEQUENCE(ref, mine);

  ref.erase(ref.begin());
  mine.erase(mine.begin());
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine.begin()->first, 3);
}

static void map_erase_range_removes_half_open_interval() {
  std_map ref;
  ft_map mine;
  fill(ref, mine);

  ref.erase(ref.find(5), ref.find(19));
  mine.erase(mine.find(5), mine.find(19));
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(mine.find(19) != mine.end());
  CHECK(map_tree_is_valid(mine));
}

static void map_erase_everything_then_reuse() {
  ft_map m = make_map();
  m.erase(m.begin(), m.end());
  CHECK(m.empty());
  CHECK(m.begin() == m.end());
  m[1] = "again";
  CHECK_SIZE(m, 1);
  CHECK_EQ(m.begin()->second, std::string("again"));
}

static void map_clear_empties_map() {
  ft_map m = make_map();
  m.clear();
  CHECK(m.empty());
  CHECK_SIZE(m, 0);
  CHECK(m.begin() == m.end());
  CHECK(map_tree_is_valid(m));
}

static void map_swap_exchanges_contents_and_keeps_iterators_valid() {
  ft_map a;
  ft_map b;
  a[1] = "a1";
  a[2] = "a2";
  b[7] = "b7";
  ft_map::iterator into_a = a.begin();

  a.swap(b);

  CHECK_SIZE(a, 1);
  CHECK_SIZE(b, 2);
  CHECK_EQ(a.begin()->first, 7);
  CHECK_EQ(into_a->second, std::string("a1"));
  CHECK(into_a == b.begin());
}

// ---------------------------------------------------------------------------
// 비교와 비교자
// ---------------------------------------------------------------------------

static void map_relational_operators_match_std() {
  std_map ref_a, ref_b, ref_c;
  ft_map mine_a, mine_b, mine_c;
  fill(ref_a, mine_a);
  fill(ref_b, mine_b);
  fill(ref_c, mine_c);
  ref_c.erase(30);
  mine_c.erase(30);
  ref_b[5] = "zzz";
  mine_b[5] = "zzz";

  CHECK(mine_a == mine_a);
  CHECK_EQ(mine_a == mine_b, ref_a == ref_b);
  CHECK_EQ(mine_a != mine_b, ref_a != ref_b);
  CHECK_EQ(mine_a < mine_b, ref_a < ref_b);
  CHECK_EQ(mine_a <= mine_b, ref_a <= ref_b);
  CHECK_EQ(mine_a > mine_b, ref_a > ref_b);
  CHECK_EQ(mine_a >= mine_b, ref_a >= ref_b);
  CHECK_EQ(mine_a < mine_c, ref_a < ref_c);
  CHECK_EQ(mine_c < mine_a, ref_c < ref_a);
  CHECK_EQ(mine_a == mine_c, ref_a == ref_c);
}

static void map_with_greater_comparator_iterates_descending() {
  typedef ft::map<int, int, std::greater<int> > desc_map;
  std::map<int, int, std::greater<int> > ref;
  desc_map mine;
  for (int i = 0; i < kKeyCount; ++i) {
    ref[kKeys[i]] = i;
    mine[kKeys[i]] = i;
  }
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine.begin()->first, 30);
  CHECK_EQ(mine.lower_bound(16)->first, 15);
  CHECK(mine.key_comp()(2, 1));
  CHECK(test::rb_tree_is_valid(mine.end().base().base()));
}

static void map_key_comp_and_value_comp_compare_by_key() {
  ft_map m;
  CHECK(m.key_comp()(1, 2));
  CHECK(!m.key_comp()(2, 1));
  ft_map::value_compare comp = m.value_comp();
  CHECK(comp(ft_map::value_type(1, "zzz"), ft_map::value_type(2, "aaa")));
  CHECK(!comp(ft_map::value_type(2, "aaa"), ft_map::value_type(1, "zzz")));
}

static void map_with_string_keys_orders_lexicographically() {
  std::map<std::string, int> ref;
  ft::map<std::string, int> mine;
  const char *words[] = {"pear", "apple", "fig", "banana", "apple"};
  for (int i = 0; i < 5; ++i) {
    ref[words[i]] = i;
    mine[words[i]] = i;
  }
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK_EQ(mine.begin()->first, std::string("apple"));
  CHECK_EQ(mine["apple"], 4);
}

static void map_max_size_is_positive() {
  ft_map m;
  CHECK(m.max_size() > 0);
  CHECK(m.max_size() > 1000000);
}

// ---------------------------------------------------------------------------
// 무작위 연산: std::map 과 한 단계씩 비교하고 트리 불변식을 확인합니다.
// ---------------------------------------------------------------------------

static void map_random_operations_match_std_and_keep_tree_valid() {
  test::Random rng(99);
  std::map<int, int> ref;
  ft::map<int, int> mine;

  for (int step = 0; step < 5000; ++step) {
    const int key = rng.below(400);
    switch (rng.below(4)) {
    case 0:
    case 1:
      CHECK_EQ(mine.insert(ft::make_pair(key, step)).second,
               ref.insert(std::make_pair(key, step)).second);
      break;
    case 2:
      CHECK_EQ(mine.erase(key), ref.erase(key));
      break;
    case 3:
      ref[key] = step;
      mine[key] = step;
      break;
    }
    CHECK_EQ(mine.size(), ref.size());
    if (step % 100 == 0) {
      CHECK_SAME_SEQUENCE(ref, mine);
      CHECK(map_tree_is_valid(mine));
    }
    if (test::current_test_failed())
      break;
  }
  CHECK_SAME_SEQUENCE(ref, mine);
  CHECK(map_tree_is_valid(mine));
  CHECK(test::rb_tree_height(mine.end().base().base()->left) <=
        test::rb_max_height(mine.size()));
}

// ---------------------------------------------------------------------------
// 값 수명: 빈 map 을 만든 직후를 기준으로, 넣은 값이 지워질 때 정확히 파괴되는지 봅니다.
// ---------------------------------------------------------------------------

typedef ft::map<int, test::Counted> counted_map;

static void map_destroys_values_on_erase_clear_and_destruction() {
  const int outside = test::Counted::live();
  {
    counted_map m;
    const int empty = test::Counted::live();
    for (int i = 0; i < 5; ++i)
      m.insert(ft::make_pair(i, test::Counted(i)));
    CHECK_EQ(test::Counted::live() - empty, 5);

    m.erase(2);
    CHECK_EQ(test::Counted::live() - empty, 4);

    m.erase(m.begin(), m.end());
    CHECK_EQ(test::Counted::live() - empty, 0);

    m[7]; // operator[] 가 기본값을 하나 만듭니다.
    CHECK_EQ(test::Counted::live() - empty, 1);

    m.clear();
    CHECK_EQ(test::Counted::live() - empty, 0);
  }
  CHECK_EQ(test::Counted::live() - outside, 0);
}

static void map_rejected_duplicate_insert_does_not_leak_value() {
  counted_map m;
  const int empty = test::Counted::live();
  m.insert(ft::make_pair(1, test::Counted(1)));
  m.insert(ft::make_pair(1, test::Counted(2)));
  m.insert(m.begin(), ft::make_pair(1, test::Counted(3)));
  m[1];
  CHECK_SIZE(m, 1);
  CHECK_EQ(test::Counted::live() - empty, 1);
}

// ---------------------------------------------------------------------------
// 반복자 유효성과 swap
//
// 평가지는 insert 와 erase 가 기존 반복자를 무효화하지 않는지, swap 이 데이터를 옮기지 않고
// 노드만 바꾸는지 확인합니다. 반복자가 가리키는 키와 값의 주소가 그대로인지로 확인합니다.
// ---------------------------------------------------------------------------

static void map_insert_keeps_existing_iterators_and_references_valid() {
  ft_map m;
  std::vector<ft_map::iterator> kept;
  std::vector<const std::string *> addresses;
  for (int key = 0; key < 100; key += 2) {
    kept.push_back(m.insert(ft::make_pair(key, label(key))).first);
    addresses.push_back(&kept.back()->second);
  }

  // 사이사이에 홀수 키를 넣어 트리가 여러 번 회전하게 합니다.
  for (int key = 1; key < 100; key += 2)
    m.insert(ft::make_pair(key, label(key)));
  CHECK_SIZE(m, 100);
  CHECK(map_tree_is_valid(m));

  for (std::size_t i = 0; i < kept.size(); ++i) {
    const int key = static_cast<int>(i) * 2;
    CHECK_EQ(kept[i]->first, key);
    CHECK_EQ(kept[i]->second, label(key));
    CHECK(&kept[i]->second == addresses[i]);
  }

  // 기존 반복자에서 ++ 하면 새로 들어온 홀수 키가 보여야 합니다.
  ft_map::iterator next = kept[0];
  ++next;
  CHECK_EQ(next->first, 1);
}

static void map_erase_keeps_iterators_to_remaining_elements_valid() {
  ft_map m;
  for (int key = 0; key < 100; ++key)
    m.insert(ft::make_pair(key, label(key)));

  std::vector<ft_map::iterator> kept;
  std::vector<const std::string *> addresses;
  for (int key = 0; key < 100; key += 2) {
    kept.push_back(m.find(key));
    addresses.push_back(&kept.back()->second);
  }

  for (int key = 1; key < 100; key += 2)
    CHECK_EQ(m.erase(key), std::size_t(1));
  CHECK_SIZE(m, 50);
  CHECK(map_tree_is_valid(m));

  ft_map::iterator walk = m.begin();
  for (std::size_t i = 0; i < kept.size(); ++i, ++walk) {
    CHECK_EQ(kept[i]->first, static_cast<int>(i) * 2);
    CHECK(&kept[i]->second == addresses[i]);
    CHECK(walk == kept[i]);
  }
  CHECK(walk == m.end());

  // 반복자로 지운 뒤에도 이웃 반복자는 그대로 쓸 수 있습니다.
  m.erase(kept[10]);
  ft_map::iterator after_gap = kept[9];
  ++after_gap;
  CHECK(after_gap == kept[11]);
}

static void map_swap_exchanges_nodes_without_copying_elements() {
  counted_map a;
  counted_map b;
  for (int i = 0; i < 5; ++i)
    a.insert(ft::make_pair(i, test::Counted(i)));
  for (int i = 10; i < 13; ++i)
    b.insert(ft::make_pair(i, test::Counted(i)));
  test::Counted *first_of_a = &a.begin()->second;
  test::Counted *first_of_b = &b.begin()->second;
  const int copies = test::Counted::copies();
  const int live = test::Counted::live();

  a.swap(b);

  CHECK_EQ(test::Counted::copies(), copies);
  CHECK_EQ(test::Counted::live(), live);
  CHECK(&a.begin()->second == first_of_b);
  CHECK(&b.begin()->second == first_of_a);
  CHECK_SIZE(a, 3);
  CHECK_SIZE(b, 5);
}

int main() {
  test::Suite suite("map");

  RUN_TEST(suite, map_default_constructor_is_empty);
  RUN_TEST(suite, map_member_types_follow_the_standard);
  RUN_TEST(suite, map_range_constructor_inserts_every_pair);
  RUN_TEST(suite, map_copy_constructor_makes_independent_copy);
  RUN_TEST(suite, map_assignment_replaces_contents);

  RUN_TEST(suite, map_insert_new_key_returns_iterator_and_true);
  RUN_TEST(suite, map_insert_duplicate_key_keeps_original_and_returns_false);
  RUN_TEST(suite, map_insert_with_hint_returns_iterator_to_element);
  RUN_TEST(suite, map_insert_range_adds_only_new_keys);
  RUN_TEST(suite, map_subscript_inserts_default_value_for_missing_key);
  RUN_TEST(suite, map_subscript_returns_reference_to_existing_value);

  RUN_TEST(suite, map_iterates_keys_in_ascending_order);
  RUN_TEST(suite, map_iterator_arrow_and_star_give_key_and_mutable_value);
  RUN_TEST(suite, map_iterator_moves_both_directions);
  RUN_TEST(suite, map_reverse_iterators_walk_descending);
  RUN_TEST(suite, map_const_iteration_through_const_reference);
  RUN_TEST(suite, map_iterators_work_with_std_distance_and_advance);

  RUN_TEST(suite, map_find_and_count_distinguish_present_and_missing_keys);
  RUN_TEST(suite, map_bounds_and_equal_range_match_std_for_every_key);

  RUN_TEST(suite, map_erase_by_key_returns_number_of_removed_elements);
  RUN_TEST(suite, map_erase_by_iterator_removes_that_element);
  RUN_TEST(suite, map_erase_range_removes_half_open_interval);
  RUN_TEST(suite, map_erase_everything_then_reuse);
  RUN_TEST(suite, map_clear_empties_map);
  RUN_TEST(suite, map_swap_exchanges_contents_and_keeps_iterators_valid);

  RUN_TEST(suite, map_relational_operators_match_std);
  RUN_TEST(suite, map_with_greater_comparator_iterates_descending);
  RUN_TEST(suite, map_key_comp_and_value_comp_compare_by_key);
  RUN_TEST(suite, map_with_string_keys_orders_lexicographically);
  RUN_TEST(suite, map_max_size_is_positive);

  RUN_TEST(suite, map_random_operations_match_std_and_keep_tree_valid);

  RUN_TEST(suite, map_destroys_values_on_erase_clear_and_destruction);
  RUN_TEST(suite, map_rejected_duplicate_insert_does_not_leak_value);

  RUN_TEST(suite, map_insert_keeps_existing_iterators_and_references_valid);
  RUN_TEST(suite, map_erase_keeps_iterators_to_remaining_elements_valid);
  RUN_TEST(suite, map_swap_exchanges_nodes_without_copying_elements);

  return suite.result();
}
