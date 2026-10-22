/**
 * @file ConsolePanel.hpp
 * @brief Output Log / Console panel displaying categorized log stream with filtering and search.
 */

#pragma once

#include "core/Log.hpp"

namespace Engine {

    /**
     * @class ConsolePanel
     * @brief Console log viewer with real-time log ingestion, search, and severity filters.
     */
    class ConsolePanel {
    public:
        ConsolePanel() = default;

        /**
         * @brief Renders the console panel.
         */
        void Render();

    private:
        bool m_bShowInfo{true};
        bool m_bShowWarnings{true};
        bool m_bShowErrors{true};
        bool m_bAutoScroll{true};
        char m_FilterBuffer[128]{""};
    };

} // namespace Engine
