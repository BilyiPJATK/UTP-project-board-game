#include <jni.h>
#include <vector>
#include <set>
#include <utility> // for std::pair
#include <functional>
#include "GoBoard.h" // Include your generated JNI header

class GoBoard {
public:
    static const int BOARD_SIZE = 19;
    int board[BOARD_SIZE][BOARD_SIZE] = {0};
    bool blackTurn = true;
    int capturedWhite = 0;
    int capturedBlack = 0;
    int consecutivePasses = 0;
    bool gameState = true;

    GoBoard() {
        // Initialize the board to empty
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = 0; // Set all cells to empty
            }
        }
    }

    void placeStone(int x, int y) {
        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == 0) {
            int color = blackTurn ? 1 : 2;

            // If all adjacent stones are opponent's stones, then the stone is surrounded
            if (!isSurrounded(x,y)) {
                // Place the stone if it's a legal move
                board[x][y] = color; // Place black (1) or white (2) stone
                checkCaptures(x, y); // Check for captures after placing the stone
                blackTurn = !blackTurn; // Switch turns
            }

            if (
                    checkCaptures(x+1,y) != 0 ||
                    checkCaptures(x,y+1) != 0 ||
                    checkCaptures(x+2,y) != 0 ||
                    checkCaptures(x,y+2) != 0 ||
                    checkCaptures(x,y-1) != 0 ||
                    checkCaptures(x-1,y) != 0 ||
                    checkCaptures(x,y-2) != 0 ||
                    checkCaptures(x-2,y) != 0
            )
                blackTurn = !blackTurn;
        }
        consecutivePasses = 0;
    }





    int getStone(int x, int y) {
        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE) {
            return board[x][y]; // Return the value of the cell
        }
        return -1; // Invalid position
    }

    bool isSurrounded(int x, int y) {
        int color = blackTurn ? 1 : 2; // Determine the current player's color

        // Check if the position (x, y) is surrounded by the opponent's stones
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (abs(dx) + abs(dy) == 1) { // Only check directly adjacent cells
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && ny >= 0 && nx < BOARD_SIZE && ny < BOARD_SIZE) {
                        if (board[nx][ny] == 0 ) {
                                return false;
                        }
                        if (board[nx][ny] == color) {
                            return false;
                        }
                    }
                }
            }
        }

        // If all adjacent positions are occupied by the opponent's stones
        return true; // The stone is surrounded
    }

    int checkCaptures(int x, int y) {
        int color = board[x][y]; // Get the color of the newly placed stone
        std::vector<std::pair<int, int>> captured;
        std::set<std::pair<int, int>> visited;
        int capturesNum = 0;

        // Function to perform Depth-First Search
        std::function<void(int, int)> dfs = [&](int cx, int cy) {
            // Check bounds
            if (cx < 0 || cy < 0 || cx >= BOARD_SIZE || cy >= BOARD_SIZE || visited.count({cx, cy})) {
                return;
            }
            visited.insert({cx, cy});

            if (board[cx][cy] == 0) { // Found an empty space
                return; // This group is not surrounded
            }
            if (board[cx][cy] != color) { // Found an opposite color stone
                captured.push_back({cx, cy}); // Track the captured stone
                // Continue DFS to adjacent stones
                dfs(cx - 1, cy); // Up
                dfs(cx + 1, cy); // Down
                dfs(cx, cy - 1); // Left
                dfs(cx, cy + 1); // Right
            }
        };

        // Check adjacent cells
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (abs(dx) + abs(dy) == 1) { // Only check adjacent cells
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && ny >= 0 && nx < BOARD_SIZE && ny < BOARD_SIZE) {
                        if (board[nx][ny] != color && board[nx][ny] != 0) {
                            // If we found a stone of the opposite color, we start DFS from it
                            dfs(nx, ny);
                        }
                    }
                }
            }
        }

        // After DFS, check if the captured group is surrounded
        bool surrounded = true;
        for (const auto &pos : captured) {
            int cx = pos.first;
            int cy = pos.second;

            // Check if this stone is adjacent to an empty space
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (abs(dx) + abs(dy) == 1) { // Only check adjacent cells
                        int adjX = cx + dx;
                        int adjY = cy + dy;
                        if (adjX >= 0 && adjY >= 0 && adjX < BOARD_SIZE && adjY < BOARD_SIZE) {
                            if (board[adjX][adjY] == 0) {
                                surrounded = false;
                                break;
                            }
                        }
                    }
                }
                if (!surrounded) break;
            }
        }

        // If the group is surrounded, remove the captured stones
        if (surrounded) {
            for (auto &pos : captured) {
                board[pos.first][pos.second] = 0; // Remove captured stones
                capturesNum++;
            }
        }
        color == 1 ? capturedBlack += capturesNum : capturedWhite += capturesNum;
        return capturesNum;
    }

    void pass() {
        consecutivePasses++;
        blackTurn = !blackTurn;

        if (consecutivePasses == 2){
            gameState = false;
        }

    }

    std::pair<int, int> calculateTerritoryScore() {
        int blackTerritory = 0;
        int whiteTerritory = 0;
        bool visited[BOARD_SIZE][BOARD_SIZE] = {false};

        // Helper function for flood-fill (DFS)
        std::function<void(int, int, int&, int&)> dfs = [&](int x, int y, int& blackBoundary, int& whiteBoundary) {
            // Use a stack for DFS to avoid recursion depth issues
            std::vector<std::pair<int, int>> stack;
            stack.push_back({x, y});
            visited[x][y] = true;

            int territorySize = 0;
            bool isSurroundedByBlack = true;
            bool isSurroundedByWhite = true;

            while (!stack.empty()) {
                auto [cx, cy] = stack.back();
                stack.pop_back();
                territorySize++;

                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (abs(dx) + abs(dy) == 1) { // Only check adjacent cells
                            int nx = cx + dx;
                            int ny = cy + dy;

                            if (nx >= 0 && ny >= 0 && nx < BOARD_SIZE && ny < BOARD_SIZE) {
                                if (!visited[nx][ny]) {
                                    if (board[nx][ny] == 0) {
                                        stack.push_back({nx, ny});
                                        visited[nx][ny] = true;
                                    } else if (board[nx][ny] == 1) {
                                        isSurroundedByWhite = false;
                                    } else if (board[nx][ny] == 2) {
                                        isSurroundedByBlack = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // If the group of empty points is surrounded only by black, add to black's territory
            if (isSurroundedByBlack && !isSurroundedByWhite) {
                blackBoundary += territorySize;
            }
                // If the group of empty points is surrounded only by white, add to white's territory
            else if (isSurroundedByWhite && !isSurroundedByBlack) {
                whiteBoundary += territorySize;
            }
        };

        // Iterate through the board to find empty regions
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (board[i][j] == 0 && !visited[i][j]) {
                    dfs(i, j, blackTerritory, whiteTerritory);
                }
            }
        }

        // Calculate the final scores
        int finalBlackScore = blackTerritory + capturedWhite;
        int finalWhiteScore = whiteTerritory + capturedBlack;

        return {finalBlackScore, finalWhiteScore};
    }

    void endGame(int &blackScore, int &whiteScore) {
        if (!gameState) {
            auto finalScores = calculateTerritoryScore();
            blackScore = finalScores.first;
            whiteScore = finalScores.second;
        }
    }
    bool isGameOngoing() {
        return gameState; // Return the current game state
    }

};


// JNI methods
extern "C" {
JNIEXPORT jlong JNICALL Java_GoBoard_nativeInit(JNIEnv *env, jobject obj) {
    GoBoard* board = new GoBoard();
    return reinterpret_cast<jlong>(board); // Return pointer as jlong
}

JNIEXPORT void JNICALL Java_GoBoard_nativePlaceStone(JNIEnv *env, jobject obj, jlong boardPtr, jint x, jint y) {
    GoBoard* board = reinterpret_cast<GoBoard*>(boardPtr);
    board->placeStone(x, y);
}

JNIEXPORT jint JNICALL Java_GoBoard_nativeGetStone(JNIEnv *env, jobject obj, jlong boardPtr, jint x, jint y) {
    GoBoard* board = reinterpret_cast<GoBoard*>(boardPtr);
    return board->getStone(x, y);
}

JNIEXPORT void JNICALL Java_GoBoard_nativeFree(JNIEnv *env, jobject obj, jlong boardPtr) {
    GoBoard* board = reinterpret_cast<GoBoard*>(boardPtr);
    delete board; // Free memory
}
JNIEXPORT void JNICALL Java_GoBoard_nativeCheckCaptures(JNIEnv *env, jobject obj, jlong boardPtr, jint x, jint y) {
    GoBoard* board = reinterpret_cast<GoBoard*>(boardPtr);
    board->checkCaptures(x, y); // Call the checkCaptures method
}
JNIEXPORT void JNICALL Java_GoBoard_nativePass(JNIEnv *env, jobject obj, jlong boardPtr) {
    GoBoard* board = reinterpret_cast<GoBoard*>(boardPtr);
    board->pass(); // Call the pass method
}
JNIEXPORT jlong JNICALL Java_GoBoard_nativeEndGame(JNIEnv *env, jobject obj, jlong boardPtr) {
    GoBoard* board = reinterpret_cast<GoBoard*>(boardPtr);

    int blackScore = 0;
    int whiteScore = 0;

    board->endGame(blackScore, whiteScore); // Calculate scores

    return (static_cast<jlong>(blackScore) << 32) | static_cast<jlong>(whiteScore); // Return as a long
}
JNIEXPORT jboolean JNICALL Java_GoBoard_nativeIsGameOngoing(JNIEnv *env, jobject obj, jlong boardPtr) {
    GoBoard *board = reinterpret_cast<GoBoard *>(boardPtr);
    return board->isGameOngoing();
}
} // extern "C"
