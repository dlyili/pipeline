#pragma once
#include <functional>
#include <future>
#include <pipeline/details.hpp>
#include <pipeline/fn.hpp>
#include <thread>

namespace pipeline {

template <typename Fn, typename... Fns> class fork_into_async {
  std::tuple<Fn, Fns...> fns_;

public:
  typedef Fn left_type;

  fork_into_async(Fn first, Fns... fns) : fns_(first, fns...) {}

  template <typename... Args> decltype(auto) operator()(Args&&... args) {
    typedef typename std::result_of<Fn(Args...)>::type result_type;

    std::vector<std::future<result_type>> futures;

    auto apply_fn = [&futures, args_tuple = std::tuple<Args...>(args...)](auto fn) {
      auto unpack = [](auto tuple, auto fn) { return details::apply(tuple, fn); };
      futures.push_back(std::async(std::launch::async | std::launch::deferred, unpack, args_tuple, fn));
    };

    details::for_each_in_tuple(fns_, apply_fn);

    return futures;
  }

  template <typename T3> auto operator|(T3 &&rhs) {
    return pipe_pair<fork_into_async<Fn, Fns...>, T3>(*this, std::forward<T3>(rhs));
  }
};

} // namespace pipeline