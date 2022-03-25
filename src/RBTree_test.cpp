#include "RBTree.hpp"

#include <functional>
#include <iostream>
#include <set>
#include <string>

typedef ft::rb_tree<int, std::less<int> > int_tree;
typedef ft::rb_tree<ft::pair<int, std::string>,
                    std::less<ft::pair<int, std::string> > > pair_tree;

static const int g_values[] = {10, 5, 15, 17, 19, 21, 23, 3, 8, 1, 12, 30};
static const int g_count = sizeof(g_values) / sizeof(g_values[0]);

template<class T>
void print_tree(T &t)
{
  for (typename T::iterator it = t.begin(); it != t.end(); ++it)
    std::cout << *it << " ";
  std::cout << std::endl;
}

template<class T>
void print_bound(T &t, int key)
{
  typename T::iterator f = t.find(key);
  typename T::iterator lb = t.lower_bound(key);
  typename T::iterator ub = t.upper_bound(key);

  std::cout << "key " << key << " find: ";
  if (f == t.end())
    std::cout << "end";
  else
    std::cout << *f;
  std::cout << " lower_bound: ";
  if (lb == t.end())
    std::cout << "end";
  else
    std::cout << *lb;
  std::cout << " upper_bound: ";
  if (ub == t.end())
    std::cout << "end";
  else
    std::cout << *ub;
  std::cout << std::endl;
}

void rbtree_insert()
{
  std::set<int> s;
  int_tree my_t;
  std::cout << "[ rbtree_insert ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  for (int i = 0; i < g_count; i++)
    s.insert(g_values[i]);
  std::pair<std::set<int>::iterator, bool> p = s.insert(15);
  std::cout << "std_set: "
            << "size: " << s.size() << " inserted: " << p.second
            << " value: " << *p.first << " | ";
  print_tree<std::set<int> >(s);

  for (int i = 0; i < g_count; i++)
    my_t.insert(g_values[i]);
  ft::pair<int_tree::iterator, bool> my_p = my_t.insert(15);
  std::cout << "my__tree: "
            << "size: " << my_t.size() << " inserted: " << my_p.second
            << " value: " << *my_p.first << " | ";
  print_tree<int_tree>(my_t);
  std::cout << std::endl;
}

void rbtree_insert_hint()
{
  std::set<int> s;
  int_tree my_t;
  int arr[3] = {4, 2, 6};
  std::cout << "[ rbtree_insert_hint ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  s.insert(s.end(), 5);
  s.insert(s.begin(), 1);
  s.insert(s.end(), 9);
  s.insert(s.find(9), 7);
  s.insert(s.find(5), 3);
  s.insert(s.begin(), 8);
  std::set<int>::iterator it = s.insert(s.end(), 5);
  std::cout << "std_set: "
            << "size: " << s.size() << " hint_dup: " << *it << " | ";
  print_tree<std::set<int> >(s);

  my_t.insert(my_t.end(), 5);
  my_t.insert(my_t.begin(), 1);
  my_t.insert(my_t.end(), 9);
  my_t.insert(my_t.find(9), 7);
  my_t.insert(my_t.find(5), 3);
  my_t.insert(my_t.begin(), 8);
  int_tree::iterator my_it = my_t.insert(my_t.end(), 5);
  std::cout << "my__tree: "
            << "size: " << my_t.size() << " hint_dup: " << *my_it << " | ";
  print_tree<int_tree>(my_t);

  s.insert(arr, arr + 3);
  std::cout << "std_set: "
            << "size: " << s.size() << " | ";
  print_tree<std::set<int> >(s);

  my_t.insert(arr, arr + 3);
  std::cout << "my__tree: "
            << "size: " << my_t.size() << " | ";
  print_tree<int_tree>(my_t);
  std::cout << std::endl;
}

void rbtree_iterator()
{
  std::set<int> s;
  int_tree my_t;
  std::cout << "[ rbtree_iterator ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  for (int i = 0; i < g_count; i++) {
    s.insert(g_values[i]);
    my_t.insert(g_values[i]);
  }

  std::cout << "std_set: forward: ";
  print_tree<std::set<int> >(s);
  std::cout << "my__tree: forward: ";
  print_tree<int_tree>(my_t);

  std::cout << "std_set: backward: ";
  std::set<int>::iterator it = s.end();
  do {
    --it;
    std::cout << *it << " ";
  } while (it != s.begin());
  std::cout << std::endl;

  std::cout << "my__tree: backward: ";
  int_tree::iterator my_it = my_t.end();
  do {
    --my_it;
    std::cout << *my_it << " ";
  } while (my_it != my_t.begin());
  std::cout << std::endl;

  it = s.begin();
  it++;
  it++;
  it--;
  std::cout << "std_set: post_inc_dec: " << *it << std::endl;
  my_it = my_t.begin();
  my_it++;
  my_it++;
  my_it--;
  std::cout << "my__tree: post_inc_dec: " << *my_it << std::endl;

  const std::set<int> &cs = s;
  std::cout << "std_set: const_iterator: ";
  for (std::set<int>::const_iterator cit = cs.begin(); cit != cs.end(); ++cit)
    std::cout << *cit << " ";
  std::cout << std::endl;

  const int_tree &my_ct = my_t;
  std::cout << "my__tree: const_iterator: ";
  for (int_tree::const_iterator cit = my_ct.begin(); cit != my_ct.end(); ++cit)
    std::cout << *cit << " ";
  std::cout << std::endl;
  std::cout << std::endl;
}

void rbtree_find_bound()
{
  std::set<int> s;
  int_tree my_t;
  int keys[5] = {15, 16, 0, 30, 31};
  std::cout << "[ rbtree_find_bound ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  for (int i = 0; i < g_count; i++) {
    s.insert(g_values[i]);
    my_t.insert(g_values[i]);
  }

  for (int i = 0; i < 5; i++) {
    std::cout << "std_set: ";
    print_bound<std::set<int> >(s, keys[i]);
    std::cout << "my__tree: ";
    print_bound<int_tree>(my_t, keys[i]);
  }
  std::cout << std::endl;
}

void rbtree_erase()
{
  std::set<int> s;
  int_tree my_t;
  int targets[4] = {1, 15, 30, 10};
  std::cout << "[ rbtree_erase ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  for (int i = 0; i < g_count; i++) {
    s.insert(g_values[i]);
    my_t.insert(g_values[i]);
  }

  for (int i = 0; i < 4; i++) {
    s.erase(s.find(targets[i]));
    std::cout << "std_set: erase " << targets[i] << " size: " << s.size()
              << " | ";
    print_tree<std::set<int> >(s);

    my_t.erase(my_t.find(targets[i]));
    std::cout << "my__tree: erase " << targets[i] << " size: " << my_t.size()
              << " | ";
    print_tree<int_tree>(my_t);
  }

  while (s.size() > 0)
    s.erase(s.begin());
  std::cout << "std_set: erase all size: " << s.size()
            << " empty: " << (s.begin() == s.end()) << std::endl;

  while (my_t.size() > 0)
    my_t.erase(my_t.begin());
  std::cout << "my__tree: erase all size: " << my_t.size()
            << " empty: " << (my_t.begin() == my_t.end()) << std::endl;

  s.insert(42);
  std::cout << "std_set: reinsert size: " << s.size() << " | ";
  print_tree<std::set<int> >(s);
  my_t.insert(42);
  std::cout << "my__tree: reinsert size: " << my_t.size() << " | ";
  print_tree<int_tree>(my_t);
  std::cout << std::endl;
}

void rbtree_copy_swap_clear()
{
  std::set<int> s;
  int_tree my_t;
  std::cout << "[ rbtree_copy_swap_clear ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  for (int i = 0; i < 5; i++) {
    s.insert(g_values[i]);
    my_t.insert(g_values[i]);
  }

  std::set<int> s_copy(s);
  std::set<int> s_assign;
  s_assign.insert(100);
  s_assign = s;
  s_copy.insert(42);
  std::cout << "std_set: origin size: " << s.size() << " | ";
  print_tree<std::set<int> >(s);
  std::cout << "std_set: copy size: " << s_copy.size() << " | ";
  print_tree<std::set<int> >(s_copy);
  std::cout << "std_set: assign size: " << s_assign.size() << " | ";
  print_tree<std::set<int> >(s_assign);

  int_tree my_copy(my_t);
  int_tree my_assign;
  my_assign.insert(100);
  my_assign = my_t;
  my_copy.insert(42);
  std::cout << "my__tree: origin size: " << my_t.size() << " | ";
  print_tree<int_tree>(my_t);
  std::cout << "my__tree: copy size: " << my_copy.size() << " | ";
  print_tree<int_tree>(my_copy);
  std::cout << "my__tree: assign size: " << my_assign.size() << " | ";
  print_tree<int_tree>(my_assign);

  std::set<int> s_other;
  s_other.insert(7);
  s_other.insert(8);
  s.swap(s_other);
  std::cout << "std_set: swap size: " << s.size() << " | ";
  print_tree<std::set<int> >(s);
  std::cout << "std_set: swap other size: " << s_other.size() << " | ";
  print_tree<std::set<int> >(s_other);

  int_tree my_other;
  my_other.insert(7);
  my_other.insert(8);
  my_t.swap(my_other);
  std::cout << "my__tree: swap size: " << my_t.size() << " | ";
  print_tree<int_tree>(my_t);
  std::cout << "my__tree: swap other size: " << my_other.size() << " | ";
  print_tree<int_tree>(my_other);

  s_other.clear();
  std::cout << "std_set: clear size: " << s_other.size()
            << " empty: " << (s_other.begin() == s_other.end()) << std::endl;
  s_other.insert(3);
  std::cout << "std_set: after clear size: " << s_other.size() << " | ";
  print_tree<std::set<int> >(s_other);

  my_other.clear();
  std::cout << "my__tree: clear size: " << my_other.size()
            << " empty: " << (my_other.begin() == my_other.end()) << std::endl;
  my_other.insert(3);
  std::cout << "my__tree: after clear size: " << my_other.size() << " | ";
  print_tree<int_tree>(my_other);
  std::cout << std::endl;
}

void rbtree_pair()
{
  std::set<std::pair<int, std::string> > s;
  pair_tree my_t;
  std::cout << "[ rbtree_pair ]" << std::endl;
  std::cout << "--------------------------" << std::endl;

  s.insert(std::make_pair(10, "ten"));
  s.insert(std::make_pair(5, "five"));
  s.insert(std::make_pair(15, "fifteen"));
  std::pair<std::set<std::pair<int, std::string> >::iterator, bool> p =
          s.insert(std::make_pair(10, "ten"));
  std::cout << "std_set: size: " << s.size() << " inserted: " << p.second
            << " | ";
  for (std::set<std::pair<int, std::string> >::iterator it = s.begin();
       it != s.end(); ++it)
    std::cout << it->first << ":" << it->second << " ";
  std::cout << std::endl;
  std::cout << "std_set: find: "
            << (s.find(std::make_pair(5, "five")) != s.end()) << " "
            << (s.find(std::make_pair(5, "x")) != s.end()) << std::endl;

  my_t.insert(ft::make_pair(10, "ten"));
  my_t.insert(ft::make_pair(5, "five"));
  my_t.insert(ft::make_pair(15, "fifteen"));
  ft::pair<pair_tree::iterator, bool> my_p =
          my_t.insert(ft::make_pair(10, "ten"));
  std::cout << "my__tree: size: " << my_t.size() << " inserted: " << my_p.second
            << " | ";
  for (pair_tree::iterator it = my_t.begin(); it != my_t.end(); ++it)
    std::cout << it->first << ":" << it->second << " ";
  std::cout << std::endl;
  std::cout << "my__tree: find: "
            << (my_t.find(ft::make_pair(5, "five")) != my_t.end()) << " "
            << (my_t.find(ft::make_pair(5, "x")) != my_t.end()) << std::endl;
  std::cout << std::endl;
}

void RBTree_test()
{
  std::cout << "[   Red_black_tree test   ]" << std::endl;
  std::cout << "----------------------------" << std::endl;
  rbtree_insert();
  rbtree_insert_hint();
  rbtree_iterator();
  rbtree_find_bound();
  rbtree_erase();
  rbtree_copy_swap_clear();
  rbtree_pair();
}
