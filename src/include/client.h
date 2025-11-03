#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Global variables for client
char client_map[35][35];           // Current state of the map
bool known_mine[35][35];    // True if we know this is a mine
bool known_safe[35][35];    // True if we know this is safe
int revealed_count;         // Number of revealed cells
int marked_count;           // Number of marked mines

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      client_map[i][j] = '?';
      known_mine[i][j] = false;
      known_safe[i][j] = false;
    }
  }
  revealed_count = 0;
  marked_count = 0;

  // Read and execute the first move
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      char c;
      std::cin >> c;
      client_map[i][j] = c;

      // Update knowledge
      if (c >= '0' && c <= '8') {
        known_safe[i][j] = true;
      } else if (c == '@') {
        known_mine[i][j] = true;
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
// Helper function to count adjacent cells
void CountAdjacent(int r, int c, int &unknown, int &marked, int &total_adj) {
  unknown = 0;
  marked = 0;
  total_adj = 0;

  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr;
      int nc = c + dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        total_adj++;
        if (client_map[nr][nc] == '?') {
          unknown++;
        } else if (client_map[nr][nc] == '@') {
          marked++;
        }
      }
    }
  }
}

// Try to find obvious safe cells or mines
bool FindObviousMove() {
  // First pass: mark obvious mines and find obvious safe cells
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int mine_count = client_map[i][j] - '0';
        int unknown, marked, total_adj;
        CountAdjacent(i, j, unknown, marked, total_adj);

        // If marked mines equal the number, all unknowns are safe
        if (marked == mine_count && unknown > 0) {
          // Auto explore this cell
          Execute(i, j, 2);
          return true;
        }

        // If unknown + marked equals the number, all unknowns are mines
        if (unknown + marked == mine_count && unknown > 0) {
          // Mark one of the unknown cells
          for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
              if (dr == 0 && dc == 0) continue;
              int nr = i + dr;
              int nc = j + dc;
              if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
                if (client_map[nr][nc] == '?') {
                  Execute(nr, nc, 1);
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}

// Advanced pattern matching and constraint solving
bool SolveConstraints() {
  // Build constraint system and try to deduce cells
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '0' && client_map[i][j] <= '8') {
        int mine_count = client_map[i][j] - '0';
        int unknown, marked, total_adj;
        CountAdjacent(i, j, unknown, marked, total_adj);

        if (unknown == 0) continue;

        // Check for subset relationships with neighbors
        for (int di = -2; di <= 2; di++) {
          for (int dj = -2; dj <= 2; dj++) {
            int ni = i + di;
            int nj = j + dj;
            if (ni < 0 || ni >= rows || nj < 0 || nj >= columns) continue;
            if (client_map[ni][nj] < '0' || client_map[ni][nj] > '8') continue;

            int neighbor_mines = client_map[ni][nj] - '0';
            int n_unknown, n_marked, n_total;
            CountAdjacent(ni, nj, n_unknown, n_marked, n_total);

            if (n_unknown == 0) continue;

            // Find common and unique unknown cells
            bool common_cells[35][35] = {false};
            bool unique_to_first[35][35] = {false};
            bool unique_to_second[35][35] = {false};
            int common_count = 0;
            int unique_first_count = 0;
            int unique_second_count = 0;

            // Find cells around first number
            for (int dr = -1; dr <= 1; dr++) {
              for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int r1 = i + dr;
                int c1 = j + dc;
                if (r1 >= 0 && r1 < rows && c1 >= 0 && c1 < columns && client_map[r1][c1] == '?') {
                  bool is_common = false;
                  // Check if also around second number
                  for (int dr2 = -1; dr2 <= 1; dr2++) {
                    for (int dc2 = -1; dc2 <= 1; dc2++) {
                      if (dr2 == 0 && dc2 == 0) continue;
                      int r2 = ni + dr2;
                      int c2 = nj + dc2;
                      if (r1 == r2 && c1 == c2) {
                        is_common = true;
                        break;
                      }
                    }
                    if (is_common) break;
                  }
                  if (is_common) {
                    common_cells[r1][c1] = true;
                    common_count++;
                  } else {
                    unique_to_first[r1][c1] = true;
                    unique_first_count++;
                  }
                }
              }
            }

            // Find cells unique to second number
            for (int dr = -1; dr <= 1; dr++) {
              for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                int r2 = ni + dr;
                int c2 = nj + dc;
                if (r2 >= 0 && r2 < rows && c2 >= 0 && c2 < columns && client_map[r2][c2] == '?') {
                  if (!common_cells[r2][c2] && !unique_to_first[r2][c2]) {
                    unique_to_second[r2][c2] = true;
                    unique_second_count++;
                  }
                }
              }
            }

            // Apply constraint reasoning
            int remaining_mines_first = mine_count - marked;
            int remaining_mines_second = neighbor_mines - n_marked;

            // If all mines in first are in common area, unique areas are safe
            if (remaining_mines_first == common_count && unique_first_count > 0) {
              for (int r = 0; r < rows; r++) {
                for (int c = 0; c < columns; c++) {
                  if (unique_to_first[r][c]) {
                    Execute(r, c, 0);
                    return true;
                  }
                }
              }
            }

            // Similar for second number
            if (remaining_mines_second == common_count && unique_second_count > 0) {
              for (int r = 0; r < rows; r++) {
                for (int c = 0; c < columns; c++) {
                  if (unique_to_second[r][c]) {
                    Execute(r, c, 0);
                    return true;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}

// Last resort: make an educated guess
void MakeGuess() {
  // Find the safest unknown cell (corner or edge cells are often safer)
  int best_r = -1, best_c = -1;
  int min_adjacent_unknown = 999;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '?') {
        int unknown, marked, total_adj;
        CountAdjacent(i, j, unknown, marked, total_adj);

        // Prefer cells with fewer unknown neighbors (more information)
        if (unknown < min_adjacent_unknown) {
          min_adjacent_unknown = unknown;
          best_r = i;
          best_c = j;
        }
      }
    }
  }

  if (best_r != -1) {
    Execute(best_r, best_c, 0);
  }
}

void Decide() {
  // Strategy 1: Look for obvious moves (safe cells and mines)
  if (FindObviousMove()) {
    return;
  }

  // Strategy 2: Advanced constraint solving
  if (SolveConstraints()) {
    return;
  }

  // Strategy 3: Make an educated guess
  MakeGuess();
}

#endif