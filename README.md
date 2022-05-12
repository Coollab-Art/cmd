# cmd

The idea behind the *Command* pattern is really simple: you describe what you want to happen, without having to worry about any implementation details or dependency on anything. This is basically a case of *declarative programming*.<br/>
A command could be represented by a string, an enum, a type, ... Being C++ developers we love types and represent commands as simple structs, sometimes empty, sometimes with a little bit of data attached to them.

For example:

```cpp
struct Command_LogMessage { // Asks for a message to be logged.
    std::string message;
};
template<typename T>
struct Command_SetValue { // Asks for the variable identified by `id` to be set to `value`.
    Id id;
    T value;
};
struct Command_SaveProject{}; // Asks for the current project to be saved. This is an empty struct, there is no need for any data in this case.
```

This is then the responsibility of the rest of the application to know how to execute these commands:

```cpp
class CommandDispatcher {
public:
    void dispatch(Command_LogMessage command) { /*...*/ }
    template<typename T>
    void dispatch(Command_SetValue<T> command) { /*...*/ }
    void dispatch(Command_SaveProject) { /*...*/ }

private:
    // All the implementation details required
    // to execute the commands
};
```

and things that can send commands (the user interface, the history, the scripting system, etc.) only have that `CommandDispatcher` at their disposal.

The *cmd* library provides utilities that rely on commands; you are responsible for implementing your command type, and we provide support for an history, scripting, and many more!

## Advantages

### History

### Scripting

Commands give a C API to your code: just structs with some data.

### Recording

### Testing

Since everything that is doable in your UI should correspond to a command in code, this makes integration tests easy: simply launch the corresponding commands to reproduce any user action.