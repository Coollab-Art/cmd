#pragma once

#include <cmd/cereal.hpp>
#include <cmd/cmd.hpp>
#include <cmd/imgui.hpp>

namespace cmd {

struct MaxSavedSizeWidget {
    size_t uncommited_max_saved_size{};

    template<Command CommandT>
    auto imgui(SerializationForHistory& serializer) -> bool
    {
        ImGui::Text("History saved size");
        internal::imgui_help_marker(
            "When saving your project, a small part of the history is saved too, "
            "so that upon reopening it you can still undo the things you did the last time."
        );
        const auto res = internal::imgui_input_history_size<CommandT>(
            &uncommited_max_saved_size,
            serializer.max_saved_size,
            1782167841
        );
        if (res.is_item_deactivated_after_edit)
        {
            serializer.max_saved_size = uncommited_max_saved_size;
        }
        if (!res.is_item_active) // Sync with the current max_size if we are not editing // Must be after the check for IsItemDeactivatedAfterEdit() otherwise the value can't be set properly when we finish editing
        {
            uncommited_max_saved_size = serializer.max_saved_size;
        }
        return res.is_item_deactivated_after_edit;
    }
};

template<Command CommandT>
class HistoryWithUiAndSerialization {
public:
    template<typename CommandToString>
    void imgui_show(CommandToString&& command_to_string)
    {
        _ui.imgui_show(_history, std::forward<CommandToString>(command_to_string));
    }

    auto imgui_max_size() -> bool { return _ui.imgui_max_size(_history); }
    auto imgui_max_saved_size() -> bool { return _max_saved_size_widget.imgui<CommandT>(_serialization); }

    // ---Boilerplate to replicate the API of an History---
    explicit HistoryWithUiAndSerialization(size_t max_size = 1000)
        : _history{max_size} {}
    template<typename MergerT>
        requires Merger<MergerT, CommandT>
    void push(const CommandT& command, const MergerT& merger)
    {
        _ui.push(_history, command, merger);
    }
    template<typename MergerT>
        requires Merger<MergerT, CommandT>
    void push(CommandT&& command, const MergerT& merger)
    {
        _ui.push(_history, std::move(command), merger);
    }
    template<typename ExecutorT>
        requires Executor<ExecutorT, CommandT>
    void move_forward(ExecutorT& executor)
    {
        _ui.move_forward(_history, executor);
    }
    template<typename ReverterT>
        requires Reverter<ReverterT, CommandT>
    void move_backward(ReverterT& reverter)
    {
        _ui.move_backward(_history, reverter);
    }
    void dont_merge_next_command() const { _history.dont_merge_next_command(); }
    // ---End of boilerplate---

private:
    History<CommandT>       _history;
    UiForHistory            _ui{};
    MaxSavedSizeWidget      _max_saved_size_widget{};
    SerializationForHistory _serialization{};

private:
    friend class cereal::access;

    template<class Archive>
    void save(Archive& archive) const
    {
        _serialization.save(archive, _history);
    }

    template<class Archive>
    void load(Archive& archive)
    {
        _serialization.load(archive, _history);
    }
};

} // namespace cmd