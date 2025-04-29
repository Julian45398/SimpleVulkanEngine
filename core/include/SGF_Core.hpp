#pragma once

#include "Logger.hpp"

namespace SGF {
    /**
     * @brief Initializes the SGF backend
     */
    void init();
    /**
     * @brief Terminates the SGF backend and frees allocated resources
     */
    void terminate();
}