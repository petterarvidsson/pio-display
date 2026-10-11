#include <action.hpp>
namespace sdhi {
  bool CommandQueue::enqueue(std::ranges::input_range auto&& commands) {
    for(auto command: commands) {
      midi_commands.push_back(command);
    }
    //    midi_commands.append_range(commands);
    return true;
  }
  template<typename T> void ActionBase<T>::execute(std::vector<Command> &commands) {
    T calculated_state = calculate_state();
    if(state != calculated_state) {
      enqueue_commands(calculated_state, commands);
      state = calculated_state;
    }
  }
}
