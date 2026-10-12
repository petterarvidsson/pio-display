#pragma once
#include<variant>
#include<vector>
#include<functional>
#include<ranges>

namespace sdhi {

  struct Log {
    std::string_view msg;
  };

  using Command = std::variant<Log>;

  class CommandQueue {
  public:
    std::vector<std::string_view> log;
    bool enqueue(std::ranges::input_range auto&& comands);
  };

  class Action {
  public:
    virtual void execute(std::vector<Command> &commands) = 0;
  };

  template<typename T> class ActionBase : public Action {
    std::function<T> calculate_state;
    std::function<void(T, std::vector<Command>&)> enqueue_commands;
    T state;
  public:
    ActionBase(std::function<T> calculate_state, std::function<void(T,  std::vector<Command>&)> enqueue_commands) : calculate_state(calculate_state), enqueue_commands(enqueue_commands) {}
    void execute(CommandQueue &queue);
  };
}
