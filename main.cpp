#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <vector>

template <typename T1, typename T2>
class pipe {
  T1 left_;
  T2 right_;

public:
  pipe(T1 left, T2 right) : left_(left), right_(right) {}

  auto operator()() {
    return right_(left_());
  }

  template <typename T3>
  auto operator|(const T3& rhs) {
    return pipe<pipe<T1, T2>, T3>(*this, rhs);
  }

};

template <typename Fn, typename... Args>
class bind {
  Fn fn_;
  std::tuple<Args...> args_;

  template <class F, size_t... Is>
  constexpr auto index_apply_impl(F f, std::index_sequence<Is...>) {
    return f(std::integral_constant<size_t, Is> {}...);
  }

  template <size_t N, class F>
  constexpr auto index_apply(F f) {
    return  index_apply_impl(f, std::make_index_sequence<N>{});
  }

  // Unpacks Tuple into a parameter pack
  // Calls f(parameter_pack)
  template <class Tuple, class F>
  constexpr auto apply(Tuple t, F f) {
    return index_apply<std::tuple_size<Tuple>{}>(
      [&](auto... Is) { return f(std::get<Is>(t)...); }
    );
  }

public:
  bind(Fn fn, Args... args): fn_(fn), args_(args...) {}

  auto operator()() {
    return apply(args_, fn_);
  }

  template <typename... T>
  auto operator()(T... left_args) {
    return apply(std::tuple_cat(std::make_tuple(left_args...), args_), fn_);
  }
  
  template <typename T2>
  auto operator|(const T2& rhs) {
    return pipe(*this, rhs);
  }

  constexpr Fn fn() const {
    return fn_;
  }

  constexpr std::tuple<Args...> args() const {
    return args_;
  }
};

int main() {

  auto add = bind([](int a, int b) { return a + b; }, 5, 10);
  auto square = bind([](int a) { return a * a; });
  auto pretty_print_square = bind([](int result, std::string msg) { std::cout << msg << std::to_string(result); }, std::string{"Result = "});

  auto pipeline = add | square | pretty_print_square;
  pipeline();

  // auto pipeline = add | square;
  // std::cout << pipeline();

  // task sum([](int a, int b) -> int { return a + b; });
  // std::cout << sum(1, 2) << "\n";

  // task prod([](int a, int b, int c) -> int { return a * b * c; });
  // std::cout << prod(1, 2, 3) << "\n";

  // task greet([]() -> void { std::cout << "Hello World!\n"; });
  // greet();

  // task square([](int a) { return a * a; });

  // pipeline p(sum, square);
  // std::cout << p.get<0>()(5, 6) << "\n";
}