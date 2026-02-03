#include "CommandHistory.h"
#include "../../engine/core/Log.h"

namespace aether {

    // Define static members
    std::stack<std::shared_ptr<EditorCommand>> CommandHistory::s_UndoStack;
    std::stack<std::shared_ptr<EditorCommand>> CommandHistory::s_RedoStack;

    void CommandHistory::Execute(std::shared_ptr<EditorCommand> cmd)
    {
        if (!cmd) return;

        // Attempt to execute the command logic
        if (cmd->Execute())
        {
            s_UndoStack.push(cmd);

            // Branching History: New action clears potential redos
            while (!s_RedoStack.empty()) {
                s_RedoStack.pop();
            }

            AETHER_CORE_INFO("Command Executed: {}", cmd->GetName());
        }
        else
        {
            AETHER_CORE_ERROR("Command Failed: {}", cmd->GetName());
        }
    }

    void CommandHistory::Undo()
    {
        if (s_UndoStack.empty()) return;

        auto cmd = s_UndoStack.top();
        s_UndoStack.pop();

        cmd->Undo();
        s_RedoStack.push(cmd);

        AETHER_CORE_INFO("Undo: {}", cmd->GetName());
    }

    void CommandHistory::Redo()
    {
        if (s_RedoStack.empty()) return;

        auto cmd = s_RedoStack.top();
        s_RedoStack.pop();

        // Re-execute. We assume it succeeds since it worked the first time.
        if (cmd->Execute())
        {
            s_UndoStack.push(cmd);
            AETHER_CORE_INFO("Redo: {}", cmd->GetName());
        }
        else
        {
            // If a redo fails (e.g., file locked externally), we have a state desync.
            // For now, we log it.
            AETHER_CORE_ERROR("Redo Failed: {}", cmd->GetName());
        }
    }

    bool CommandHistory::CanUndo() { return !s_UndoStack.empty(); }
    bool CommandHistory::CanRedo() { return !s_RedoStack.empty(); }

    void CommandHistory::Clear()
    {
        while (!s_UndoStack.empty()) s_UndoStack.pop();
        while (!s_RedoStack.empty()) s_RedoStack.pop();
    }
}