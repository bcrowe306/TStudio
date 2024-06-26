#ifndef SCROLLVIEW_H
#define SCROLLVIEW_H

#include <algorithm>
#include <any>
#include <utility> // For std::pair
#include <vector>
template <typename Type>
class ScrollView {
public:
  int start;
  int end;
  int view_index;
  ScrollView(std::vector<Type> &vec, int view_size = 5, int cursor = 0,
             int scroll_padding = 1)
      : start(0), view_size(view_size),
        scroll_padding(vec.size() > scroll_padding * 3 ? scroll_padding : 0),
        end(view_size), vec(vec), view_last_index(view_size - 1),
        view_index(cursor - start) 
  {
    set_cursor(cursor);
    render_view();
  }

  int limit_bounds(int number, int lower_bound, int upper_bound) {
    if (number < lower_bound) {
      return lower_bound;
    } else if (number > upper_bound) {
      return upper_bound;
    } else {
      return number;
    }
  }

  int get_offset(int start, int end) {
    if (_cursor < start) {
      return _cursor - start;
    } else if (_cursor > end) {
      return _cursor - end;
    } else {
      return 0;
    }
  }

  void render_view(bool reset=false) {
    if(reset){
      this->reset_cursor();
    }
    int lower_end = vec.size() >= view_size ? vec.size() - view_size : 0;
    std::pair<int, int> lower_bounds = {0, lower_end};
    std::pair<int, int> upper_bounds = {
        view_size < vec.size() ? view_size : vec.size(), vec.size()};

    int offset = get_offset(start + scroll_padding, end - (scroll_padding + 1));
    start =
        limit_bounds(start + offset, lower_bounds.first, lower_bounds.second);
    end = limit_bounds(end + offset, upper_bounds.first, upper_bounds.second);
    view_index = _cursor - start;
  }

  void shift(int amount) {
    set_cursor(_cursor + amount);
    render_view();
  }

  void increment() { return shift(1); }

  void decrement() { return shift(-1); }

  int get_cursor() const { return _cursor; }

  void set_cursor(int value) {
    _cursor = limit_bounds(value, 0, vec.size() - 1);
    render_view();
  }
  void reset_cursor(){
    return set_cursor(0);
  }
  const std::vector<Type> &get_vec() const { return vec; }

  void set_vec(std::vector<Type> &value) {
    vec = value;
    set_cursor(0);
  }

private:
  int view_size;
  int scroll_padding;
  std::vector<Type> &vec;
  int view_last_index;
  int _cursor;
};

#endif // !SCROLLVIEW_H
