#include "api.h"
#include "utils.h"
#include <iostream>
#include <random>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

using namespace cycles;

class AdaptiveBot {
  Connection connection;
  std::string name;
  GameState state;
  Player my_player;
  std::mt19937 rng;
  Direction primaryDirection = Direction::east; // Direction for forward movement
  bool switchingDirection = false; // Switch zigzag direction
  Direction zigzagDirection = Direction::north; // Zigzag direction

  bool isValidMove(Direction direction) {
    sf::Vector2i newPos = my_player.position + getDirectionVector(direction);
    return state.isInsideGrid(newPos) && state.getGridCell(newPos) == 0;
  }

  Direction decideMove() {
    // Determine zigzag, fallback to primary movement
    Direction potentialMove = switchingDirection ? Direction::north : Direction::south;

    if (isValidMove(potentialMove)) {
      return potentialMove;
    }

    // Zigzag is blocked, move forward and switch directions for the next move
    if (isValidMove(primaryDirection)) {
      switchingDirection = !switchingDirection;
      return primaryDirection;
    }

    // If both primary and zigzag movements are not possible, alternative moves
    std::vector<Direction> fallbackMoves = {Direction::west, Direction::north, Direction::south, Direction::east};
    for (const auto& dir : fallbackMoves) {
      if (isValidMove(dir)) {
        return dir;
      }
    }

    // No valid moves
    return primaryDirection;
  }

  void receiveGameState() {
    state = connection.receiveGameState();
    for (const auto& player : state.players) {
      if (player.name == name) {
        my_player = player;
        break;
      }
    }
  }

  void sendMove() {
    Direction move = decideMove();
    connection.sendMove(move);
  }

public:
  AdaptiveBot(const std::string& botName) : name(botName) {
    std::random_device rd;
    rng.seed(rd());
    connection.connect(name);
    if (!connection.isActive()) {
      spdlog::critical("{}: Connection failure", name);
      exit(1);
    }
  }

  void run() {
    while (connection.isActive()) {
      receiveGameState();
      sendMove();
    }
  }
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Use: " << argv[0] << " <client_josemiguel>" << std::endl;
    return 1;
  }

  std::string botName = argv[1];
  AdaptiveBot bot(botName);
  bot.run();
  return 0;
}

