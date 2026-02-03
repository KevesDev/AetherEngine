#pragma once
#include <string>

namespace aether {

    class EditorPanel {
    public:
        virtual ~EditorPanel() = default;

        // TODO: Context is "void*" for now, but we will pass the Scene/Registry here later
        virtual void OnImGuiRender() = 0;
    };

}