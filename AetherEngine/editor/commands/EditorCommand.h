#pragma once
#include <string>
#include <memory>

namespace aether {

    /**
     * Interface for all undoable editor actions.
     * Follows the Command Pattern to encapsulate requests as objects.
     */
    class EditorCommand
    {
    public:
        virtual ~EditorCommand() = default;

        /**
         * Executes the logic of the command.
         * Returns true if successful, false if it failed (and shouldn't be added to history).
         */
        virtual bool Execute() = 0;

        /**
         * Reverts the logic performed by Execute().
         */
        virtual void Undo() = 0;

        /**
         * Returns a human-readable name for UI (e.g., "Delete Asset", "Move Entity").
         */
        virtual std::string GetName() const = 0;
    };
}