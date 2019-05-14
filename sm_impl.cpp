#define BOOST_HANA_CONFIG_ENABLE_STRING_UDL
#define USCXML_VERBOSE

#include "state_machine.h"
#include <cstring>
#include <cstdio>
#include <functional>
#include <boost/hana/string.hpp>
#include "MPDEF_DIRECTIVE.hpp"

namespace mountel {
    MPDEF_DIRECTIVE(State)
    MPDEF_DIRECTIVE(ParallelState)
    MPDEF_DIRECTIVE(Transition)
    MPDEF_DIRECTIVE_LEAF(Name)
    MPDEF_DIRECTIVE_LEAF(Action)
    MPDEF_DIRECTIVE_LEAF(OnEnter)
    MPDEF_DIRECTIVE_LEAF(OnExit)
    MPDEF_DIRECTIVE_LEAF(Guard)
    MPDEF_DIRECTIVE_LEAF(Initial)
    MPDEF_DIRECTIVE_LEAF(Target)
    MPDEF_DIRECTIVE_LIST(States)
    MPDEF_DIRECTIVE_TYPE(Event)

    constexpr auto statechart_elements = boost::hana::make_tuple(
        tag::State,
        tag::ParallelState,
        tag::Transition,
        tag::Name,
        tag::Action,
        tag::OnEnter,
        tag::OnExit,
        tag::Guard,
        tag::Initial,
        tag::Target,
        tag::States,
        tag::Event
    );

    template <typename Context>
    struct StateMachineLocals {
        Context& context;
        const double time;

        template <typename Event>
        void send(const Event& event);

        template <typename State>
        bool active(const State& state) const;

        bool after(double time) const;
    };

    // Num states

    struct count_tags_impl {
        template <typename T, typename Tags>
        constexpr auto operator()(T& t, Tags tags) const;

        constexpr auto operator()(...) const { return 0; }

        template <typename First, typename Second, typename Tags>
        constexpr auto operator()(mpdef::tree_node<First, Second> const node, Tags tags) const {
            using namespace boost::hana;
            using namespace boost::hana::literals;
            const auto states = find(node.second, tag::States);

            return if_(first(node) ^in^ tags, size_c<1>, size_c<0>)
            + count_if(keys(node.second), [&] (auto elem) { return elem ^in^ tags; }) 
            + maybe(
                size_c<0>,
                [&] (auto& states) {
                    return fold_right(states, size_c<0>, [&] (auto& elem, auto sum) { return sum + this->operator()(elem, tags); });
                },
                states
            );
        }
    };

    constexpr count_tags_impl count_tags{};

    template <typename T>
    constexpr auto num_states(T& t) {
        using namespace boost::hana;
        return count_tags(t, make_tuple(tag::State, tag::ParallelState));
    }

    template <typename T>
    constexpr auto num_transitions(T& t) {
        using namespace boost::hana;
        return count_tags(t, make_tuple(tag::Transition));
    }

    struct num_states_bytes_impl {
        template <typename T>
        constexpr auto operator()(T& t) const {
            using namespace boost::hana;
            return (num_states(t) + size_c<7>) / size_c<8>;
        }
    };

    constexpr num_states_bytes_impl num_states_bytes{};

    struct num_transitions_bytes_impl {
        template <typename T>
        constexpr auto operator()(T& t) const {
            using namespace boost::hana;
            return (num_transitions(t) + size_c<7>) / size_c<8>;
        }
    };

    constexpr num_transitions_bytes_impl num_transitions_bytes{};

    // Validate state chart

    struct validate_state_chart_impl {
        template <typename T>
        constexpr auto operator()(T& t) const {
            using namespace boost::hana;
            using namespace boost::hana::literals;
            const auto less_than_256_states = num_states(t) < size_c<256>;
            static_assert(less_than_256_states, "State chart must have less than 256 states");

            return true_c;
        }
    };

    constexpr validate_state_chart_impl validate_state_chart{};
}

namespace example {
    using namespace mountel;
    using namespace boost::hana::literals;
    // Events

    struct a {
        int data = 0;
    };
    struct b { };
    struct c { };

    // State chart
    BOOST_HANA_CONSTEXPR_STATELESS_LAMBDA auto state_chart = State(
        Name("root"_s),
        Initial("A"_s),
        States(
            State(
                Name("A"_s),
                Transition(
                    Event<b>,
                    Target("B"_s),
                    Action([] { std::puts("A -> B"); })
                )
            ),
            State(
                Name("B"_s),
                OnEnter(
                    [] (auto&& locals ) {
                        std::puts("Entered B");
                        locals.send(c{});
                    }
                ),
                Transition(
                    Event<c>,
                    Target("C"_s)
                ),
                OnExit([] { std::puts("Leaving C"); })
            ),
            ParallelState(
                Name("C"_s),
                States(
                    State(
                        Name("C1"_s),
                        Transition(
                            Event<a>,
                            Target("A"_s),
                            Guard(
                                [] (const auto&& locals, a a_) {
                                    return locals.active("C2") && a_.data;
                                }
                            )
                        )
                    ),
                    State(
                        Name("C2"_s)
                    )
                )
            )
        )
    );

    BOOST_HANA_CONSTEXPR_STATELESS_LAMBDA auto statechart_simple = State(
        Name("root"_s),
        Transition(
            Event<a>
        )
    );
}

struct event_t {
    const char* name;
};

static event_t event[1] = {{"hello"}};

int main() {
    using namespace mountel;
    using namespace example;
    using namespace boost;

    BOOST_HANA_CONSTEXPR_LAMBDA auto name = hana::find(state_chart, tag::Name);
    static_assert(name != hana::nothing, "Root state machine must have a name");

    BOOST_HANA_CONSTEXPR_LAMBDA auto states = hana::find(state_chart, tag::States);
    static_assert(states != hana::nothing, "Couldn't find states");

    BOOST_HANA_CONSTEXPR_LAMBDA auto first_state = hana::find_if(states.value(), [] (auto x) {
        return hana::first(x) == tag::State;
    });
    static_assert(first_state != hana::nothing, "Couldn't find first state");
    
    BOOST_HANA_CONSTEXPR_LAMBDA auto transition = hana::find(first_state.value(), tag::Transition);
    static_assert(transition != hana::nothing, "No transition found");

    BOOST_HANA_CONSTEXPR_LAMBDA auto action = hana::find(transition.value(), tag::Action);
    static_assert(action != hana::nothing, "No action found");

    action.value()();

    BOOST_HANA_CONSTANT_ASSERT(validate_state_chart(state_chart));

    std::printf("Num states: %lu\n", hana::value(num_states(state_chart)));
    std::printf("Num transitions: %lu\n", hana::value(num_transitions(state_chart)));
    std::printf("sizeof(state_chart): %lu\n", sizeof(state_chart));

    std::printf("Num names: %lu\n", hana::value(count_tags(state_chart, boost::hana::make_tuple(tag::Name))));

    using uscxml = USCXML<1, 1, std::deque>;

    uscxml::uscxml_ctx ctx;
    // memset(&ctx, 0, sizeof(ctx));
    ctx.machine = &USCXML_MACHINE;

    ctx.is_matched = [] (const uscxml::uscxml_ctx* ctx, const uscxml::uscxml_transition* transition, const void* event) -> int {
        return strcmp(transition->event, ((const struct event_t*)event)->name) == 0;
    };

    int err = USCXML_ERR_OK;

    while((err = uscxml_step(&ctx)) != USCXML_ERR_IDLE) {}
    std::puts("idle");
    
    event[0] = {"b"};
    ctx.external_queue.push_back((void*)event);
    while((err = uscxml_step(&ctx)) != USCXML_ERR_IDLE) {}

    using f = std::function<void(int)>;
    f a = [] (int i) { std::printf("hello %d\n", i); };
    f b = [] (auto a) {
        return [a] (int i) {
            return a(i + 1);
        };
    }(a);

    b(1);
}