#pragma once

// This file defines the version of the Aether Engine.
namespace aether
{
	// Engine version information
    struct EngineVersion
    {
        static constexpr int Major = 0;
        static constexpr int Minor = 1;
        static constexpr int Patch = 0;

		// Returns the version as a string in the format "Major.Minor.Patch"
        static std::string ToString() {
            return std::to_string(Major) + "." +
                   std::to_string(Minor) + "." +
                   std::to_string(Patch);
		}
    };


}
