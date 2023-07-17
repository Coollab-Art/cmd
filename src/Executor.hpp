#pragma once

/// An Executor is the user-defined class responsible for executing commands.
/// This is where they should put (or at least dispatch) all their logic.

#include <memory>
#include <optional>
#include "Command.hpp"

namespace cmd {

template<typename ExecutorT, typename CommandT>
concept ExecutorC = requires(ExecutorT executor, CommandT command) {
    executor.execute(command);
};

template<typename ReverterT, typename CommandT>
concept ReverterC = requires(ReverterT reverter, CommandT command) {
    reverter.revert(command);
};

template<typename MergerT, typename CommandT>
concept MergerC = requires(MergerT merger, CommandT command) {
    // clang-format off
    // clang-format doesn't know about concepts yet and messes up the syntax :-(
    { merger.merge(command, command) } -> std::convertible_to<std::optional<CommandT>>;
    // clang-format on
};

template<CommandC CommandT>
class Executor {
public:
    void execute(CommandT const& command) const { _pimpl->execute(command); }

public: // Type-erasure implementation details
    Executor() = default;

    template<ExecutorC<CommandT> ExecutorT>
    Executor(ExecutorT model) // NOLINT(*-explicit-constructor, *-explicit-conversions) A type-erased object should be implicitly created from objects matching its requirements.
        : _pimpl{std::make_unique<Model<ExecutorT>>(std::move(model))}
    {}

    Executor(Executor const& other)
        : _pimpl{other._pimpl->clone()}
    {}
    auto operator=(Executor const& other) -> Executor&
    {
        auto tmp = Executor{other};
        std::swap(_pimpl, tmp._pimpl);
        return *this;
    }
    Executor(Executor&&) noexcept                    = default;
    auto operator=(Executor&&) noexcept -> Executor& = default;
    ~Executor()                                      = default;

private:
    struct Concept { // NOLINT(*-special-member-functions)
        virtual ~Concept() = default;

        virtual void execute(CommandT const&) const = 0;

        [[nodiscard]] virtual auto clone() const -> std::unique_ptr<Concept> = 0;
    };

    template<ExecutorC<CommandT> ExecutorT>
    struct Model : public Concept {
        Model() = default;
        explicit Model(ExecutorT model)
            : _model{std::move(model)}
        {}

        void execute(CommandT const& command) const override
        {
            _model.execute(command);
        }

        [[nodiscard]] auto clone() const -> std::unique_ptr<Concept> override
        {
            return std::make_unique<Model>(*this);
        }

        ExecutorT _model;
    };

private:
    std::unique_ptr<Concept> _pimpl;
};

} // namespace cmd