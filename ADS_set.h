#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

template <typename Key, size_t N = 1>
class ADS_set {
public:
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = key_type&;
  using const_reference = const key_type&;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  using key_equal = std::equal_to<key_type>; // Hashing
  using hasher = std::hash<key_type>;        // Hashing
private:
  // ****** Item ******
  struct Item
  {
    key_type key;
    Item* next{nullptr};
    
    ~Item() { delete next; };
  };
  
  // ****** Linked List of Items ******
  struct Linked_list
  {
    Item* head{nullptr};
    size_type length{0};
    
    ~Linked_list() { delete head; }
  };
  
  Linked_list* table{nullptr};
  size_type curr_size{0}; // number of elements in a hash table
  size_type table_size{N}; // number of buckets in a hash table
  size_type max_length{0};  // max length of the Linked list
  float max_lf{0.85};
  
  size_type hash_idx(const key_type& key) const { return hasher{}(key) % table_size; }
  
  Item* find_item(const key_type& key) const
  {
    size_type idx(hash_idx(key));
    switch(table[idx].length)
    {
      case 0:
        return nullptr;
        break;
      default:
        Item* temp{table[idx].head};
        while (temp)
        {
          if (key_equal{}(temp->key, key))
          {
            return temp;
            break;
          }
          temp = temp->next;
        }
        delete temp;
        return nullptr;
        break;
    }
  }
  
  // Inserts an Item at the beginning of the list
  Item* insert_newItem(const key_type& key)
  {
    Item* newItem {new Item()};
    newItem->key = key;
    size_type idx {hash_idx(key)};
    
    newItem->next = table[idx].head;
    table[idx].head = newItem;
    ++table[idx].length;
    ++curr_size;
    if (table[idx].length > max_length)
      max_length = table[idx].length;
    
    return newItem;
  }
  
  // Table rehash
  void rehash(size_type n)
  {
    size_type new_table_size {std::max(N, std::max(n, size_type(curr_size / max_lf)))};

    Linked_list* old_table {table};
    size_type old_table_size {table_size};
    
    table = nullptr;
    delete[] table;
    
    table = new Linked_list[new_table_size];
    table_size = new_table_size;
    curr_size = 0;
    max_length = 0;
    
    Item* temp {nullptr};
    for (size_type i {0}; i < old_table_size; ++i)
    {
      temp = old_table[i].head;
      while (temp)
      {
        insert_newItem(temp->key);
        temp = temp->next;
      }
    }
    delete temp;
    delete[] old_table;
  }
  
public:
  
  // ****** Konstruktoren ******
  ADS_set() { table = new Linked_list[table_size](); }
  
  ADS_set(std::initializer_list<key_type> ilist) : ADS_set() { insert(ilist); }
  
  template<typename InputIt> ADS_set(InputIt first, InputIt last) : ADS_set() { insert(first, last); }
  
  ADS_set(const ADS_set& other) : ADS_set()
  {
    for (const auto& key : other)
    {
      insert_newItem(key);
//      if (max_length > 9 && curr_size > table_size*max_lf)
//        rehash(table_size * 5.8);
    }
    rehash (curr_size);
    std::cout << "ADS_set& other - max_length = " << max_length << std::endl;
    std::cout << "ADS_set& other - table_load = " << (double)curr_size/table_size*100 << "%" <<  std::endl;
  }
  
  // ****** Destruktor ******
  ~ADS_set() { delete[] table; }
  
  // ****** Zuweisungsoperatoren ******
  ADS_set& operator=(const ADS_set& other)
  {
    if (this == &other)
      return *this;
    ADS_set temp{other};
    swap(temp);
    return *this;
  }
  
  ADS_set& operator=(std::initializer_list<key_type> ilist)
  {
    ADS_set temp{ilist};
    swap(temp);
    return *this;
  }
  
  // ****** Groesse ******
  size_type size() const { return curr_size; }
  bool empty() const { return !curr_size; }
  
  // ****** Einfuegen ******
  void insert(std::initializer_list<key_type> ilist)
  {
    for (const auto& key : ilist)
    {
      if (!find_item(key))
      {
        insert_newItem(key);
        if (max_length > 9 && curr_size > table_size*max_lf)
          rehash(table_size * 5.8);
      }
    }
    std::cout << "ilist - max_length = " << max_length << std::endl;
    std::cout << "ilist - load ilist = " << (double)curr_size/table_size*100 << "%" << std::endl;
  }
  
  std::pair<iterator,bool> insert(const key_type& key)
  {
    Item* pos {find_item(key)};
    if (pos)
      return {iterator{pos,table,hash_idx(key),table_size}, false};
    insert_newItem(key);
    if (max_length > 9 && curr_size > table_size*max_lf)
      rehash(table_size * 5.8);
    return {iterator{find_item(key),table,hash_idx(key),table_size}, true};
  }
  
  template<typename InputIt> void insert(InputIt first, InputIt last)
  {
    for (auto it = first; it != last; ++it)
    {
      if (!find_item(*it))
      {
        insert_newItem(*it);
        if (max_length > 9 && (double)(curr_size/table_size) > max_lf)
          rehash(table_size * 5.8);
      }
    }
    std::cout << "InputIt - table_size = " << table_size << std::endl;
    std::cout << "InputIt - curr_size = " << curr_size << std::endl;
    std::cout << "InputIt - max_length = " << max_length << std::endl;
    std::cout << "InputIt - load Inputit = " << (double)curr_size/table_size*100 << "%" << std::endl;
  }
  
  // ****** Loeschen ******
  void clear()
  {
    ADS_set temp;
    swap(temp);
  }
  
  size_type erase(const key_type& key)
  {
    size_type idx{hash_idx(key)};
    switch(table[idx].length)
    {
      case 0:
        return 0;
        break;
      default:
        Item* to_delete {table[idx].head};
        if (key_equal{}(to_delete->key, key))  //when key is the first element in Linked list
        {
          table[idx].head = to_delete->next;
          to_delete->next = nullptr;
          delete to_delete;
          --table[idx].length;
          --curr_size;
          
          if (curr_size < table_size*0.2)
            rehash (curr_size);

          return 1;
          break;
        }
        while (to_delete && !key_equal{}(to_delete->key,key)) //when key is not the first element in Linked list
          to_delete = to_delete->next;
        if (!to_delete) //if key is not found
        {
          return 0;
          break;
        }
        Item* help {table[idx].head};
        while (help->next != to_delete)
          help = help->next;
        help->next = to_delete->next;
        to_delete->next = nullptr;
        
        delete to_delete;
        help = nullptr;
        delete help;
        --table[idx].length;
        --curr_size;

        if (curr_size < table_size*0.2)
          rehash (curr_size);

        return 1;
        break;
    }
  }
  
  // ****** Sonstige Modifikationen ******
  void swap(ADS_set& other)
  {
    std::swap(table, other.table);
    std::swap(curr_size, other.curr_size);
    std::swap(table_size, other.table_size);
  }
  
  // ****** Suchen ******
  size_type count(const key_type& key) const { return !!find_item(key); }
  iterator find(const key_type& key) const
  {
    if (Item* temp {find_item(key)})
      return iterator{temp, table, hash_idx(key), table_size};
    return end();
  }
  
  // ****** Iteratoren ******
  const_iterator begin() const
  {
    if (!curr_size)
      return end();
    return const_iterator(table[this->find_first_item()].head, table, find_first_item(), table_size);
  }
  
  const_iterator end() const { return const_iterator(); }
  
  //used in begin() to find table_index of first element
  size_type find_first_item() const
  {
    for (size_type i{0}; i < table_size; ++i)
    {
      if (table[i].head) { return i; }
    }
    return 0;
  }
  
  // ****** Debugging ******
  void dump(std::ostream& o = std::cerr) const
  {
    for (size_type i {0}; i < table_size; ++i)
    {
      o << i << ": ";
      switch (table[i].length)
      {
        case 0:
          o << "-\n";
          break;
        default:
          Item* temp {table[i].head};
          while (temp)
          {
            o << temp->key;
            if (temp->next)
              o << " -> ";
            temp = temp->next;
          }
          o << '\n';
          delete temp;
          break;
      }
    }
  }
  
  // ****** Abschlussklausur ******
  bool y(const key_type& a, const key_type& b) const
  {
    for (size_type row = 0; row < table_size; ++row)  //looking through the table
    {
      Item* help_a = table[row].head;
      while (help_a)  //as long as there are elements in the row, iterate through them
      {
        Item* help_b = help_a->next;
        if (!help_b)  //if help_a pointed to the last element in the row & we need to get the first element of the next row
        {
          size_type row_b = 1+row;
          while(row_b < table_size && !help_b)
          {
            help_b = table[row_b].head;
            ++row_b;
          }
          if (!help_b) { return false; }  //if help_b reached the end of the table
        }      
        
        if (key_equal{}(help_a->key, a) && key_equal{}(help_b->key, b)) { return true; }  //y(a,b)
        if (key_equal{}(help_a->key, b) && key_equal{}(help_b->key, a)) { return true; }  //y(b,a)
          
        help_a = help_a->next;
        help_b = help_a->next;
      }
    }
    return false;
  }
  // ******************************


  // ****** Non-member functions ******
  friend bool operator==(const ADS_set& lhs, const ADS_set& rhs)
  {
    if (lhs.curr_size != rhs.curr_size)
      return false;
    for (const auto& key : rhs)
    {
      if (!lhs.find_item(key)) return false;
    }
    return true;
  }
  friend bool operator!=(const ADS_set& lhs, const ADS_set& rhs) { return !(lhs == rhs); }
};


////////////////////////////////////////////////////////////
// Iterator
template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
private:
  Item* pos;
  Linked_list* table;
  size_type index; //is used to look through Linked list
  size_type table_size; //is used to look through Table
public:
  using value_type = Key;
  using difference_type = std::ptrdiff_t;
  using reference = const value_type&;
  using pointer = const value_type*;
  using iterator_category = std::forward_iterator_tag;
  
  explicit Iterator(Item* pos = nullptr, Linked_list* table = nullptr, size_type index = 0, size_type table_size = N) : pos{pos}, table{table}, index{index}, table_size{table_size}
  {}
  
  ~Iterator()
  {
    pos = nullptr;
    table = nullptr;
    delete pos;
    delete table;
  }
  
  reference operator*() const { return pos->key; }
  pointer operator->() const { return &pos->key; }
  
  void skip()
  {
    while (index < table_size && table[index].head == nullptr)
      ++index;
  }
  
  Iterator& operator++()
  {
    if (pos->next && pos)
    {
      pos = pos->next;
    } else {
      ++index;
      skip();
      if (index == table_size)
      {
        pos = nullptr;
        return *this;
      }
      pos = table[index].head;
    }
    return *this;
  }
  
  Iterator operator++(int)
  {
    Iterator rc {*this};
    ++*this;
    return rc;
  }
  
  friend bool operator==(const Iterator &lhs, const Iterator &rhs) { return lhs.pos == rhs.pos; }
  friend bool operator!=(const Iterator &lhs, const Iterator &rhs) { return lhs.pos != rhs.pos; }
};
// END Iterator
////////////////////////////////////////////////////////////

template <typename Key, size_t N>
void swap(ADS_set<Key,N>& lhs, ADS_set<Key,N>& rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H
