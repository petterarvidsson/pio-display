#include <action.hpp>
#include <overloaded.hpp>
namespace sdhi {
  bool CommandQueue::enqueue(std::ranges::input_range auto&& commands) {
    for(auto command: commands) {
      std::visit(util::overloaded{
          [this](const Log message) {
            log.push_back(message.msg);
          }
            }, command);
    }
    //    midi_commands.append_range(commands);
    return true;
  }
  template<typename T> void ActionBase<T>::execute(CommandQueue &queue) {
    T calculated_state = calculate_state();
    std::vector<Command> commands;
    if(state != calculated_state) {
      enqueue_commands(calculated_state, commands);
      if(queue.enqueue(commands))
        state = calculated_state;
    }
  }
}
