#pragma once

#include "GameState.hpp"
#include "settings.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <tuple>

namespace play
{
    std::tuple<bool, bool, bool> playGame(bool isRed, std::uint8_t hostID, int client_fd, std::array<GameState, arraySize>& gamestates, std::array<std::mutex, arraySize>& gameMutexes);
}
