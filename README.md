# mountel
A state charts library for C++

Taking inspiration from and combining code of the libraries uscxml, [boost]::sml, nbdl, and relying on the boost::hana meta programming library, to offer a convenient way to construct and use state charts in modern C++ code. The goal is to have the following compile, work as expected (if you're familiar with state charts), and be fast (at least as fast as the transpiled uscxml outputs):

```C++
#include <mountel>

int main() {
  using namespace mountel;
  
  struct done_init {
    const int answer;
  };
  
  const auto statechart = State(
    Name("root"_s),
    States(
      State(
        Name("initializing"_s),
        Transition(
          Event<done_init>,
          Target("waiting_for_question"),
          Guard([] (auto& sm, auto& e) { return e.answer > 10; }),
          Action(
            [] (auto& sm, auto& e) {
              std::printf("...? %d\n", e.answer);
            }
          )
        )
      ),
      State(
        Name("waiting_for_answer"),
        OnEnter(
          [] { std::puts("entered waiting_for_answer"); }
        )
      )
    )
  );
  
  auto machine = Machine(statechart);
  machine.process(done_init{42});
  
  assert(machine.isin("waiting_for_answer"));
}
```

## Status of project:

No where near working.

The definition of state charts now compiles, and the guards/actions are saved and are callable.

Can compute the number of states and transitions during compile time.
