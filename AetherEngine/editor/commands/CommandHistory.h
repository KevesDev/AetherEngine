#pragma once
#include "EditorCommand.h"
#include <stack>
#include <memory>
#include <vector>

namespace aether {

    /**
     * Manages the execution, undo, and redo of Editor Commands.
     */
    class CommandHistory
    {
    public:
        // Executes a command and pushes it onto the Undo stack.
        // Clears the Redo stack (branching history logic).
        static void Execute(std::shared_ptr<EditorCommand> cmd);

        static void Undo();
        static void Redo();

        static bool CanUndo();
        static bool CanRedo();

        // Clears history (e.g., when loading a new project)
        static void Clear();

    private:
        static std::stack<std::shared_ptr<EditorCommand>> s_UndoStack;
        static std::stack<std::shared_ptr<EditorCommand>> s_RedoStack;
    };
}