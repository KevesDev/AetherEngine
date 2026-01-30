#include "LayerStack.h"
#include <algorithm>

namespace aether {

	// --- Constructor ---
    LayerStack::LayerStack()
    {
    }

	// --- Destructor ---
    LayerStack::~LayerStack()
    {
        for (Layer* layer : m_Layers) {
            delete layer; // TODO: In production, we might use smart pointers here
        }
    }

	// --- Push Layer ---
    void LayerStack::PushLayer(Layer* layer)
    {
		// this adds the layer before the first overlay
        m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
        m_LayerInsertIndex++;
        layer->OnAttach();
    }

	// --- Push Overlay ---
    void LayerStack::PushOverlay(Layer* overlay)
    {
		// this adds the overlay to the end of the vector/stack
        m_Layers.emplace_back(overlay);
        overlay->OnAttach();
    }

	// --- Pop Layer ---
    void LayerStack::PopLayer(Layer* layer)
    {
		// Find the layer in the stack
        auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
        if (it != m_Layers.end())
        {
			// Remove the layer and call OnDetach
            m_Layers.erase(it);
            m_LayerInsertIndex--;
            layer->OnDetach();
        }
    }

	// --- Pop Overlay ---
    void LayerStack::PopOverlay(Layer* overlay)
    {
        auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
        if (it != m_Layers.end())
        {
            m_Layers.erase(it);
            overlay->OnDetach();
        }
    }

}