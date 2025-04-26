#include <windows.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <vector>
#include <random>
#include <set>
#include <string>
#include <algorithm>
using namespace std;

// Informasi Warna ANSI
/*
Info Warna Teks
    \033[30m: Hitam
    \033[31m: Merah
    \033[32m: Hijau
    \033[33m: Kuning
    \033[34m: Biru
    \033[35m: Magenta
    \033[36m: Cyan
    \033[37m: Putih
    Tambahkan ;1 untuk warna terang, misalnya \033[31;1m untuk merah terang.

Info Warna Background
    \033[40m: Hitam
    \033[41m: Merah
    \033[42m: Hijau
    \033[43m: Kuning
    \033[44m: Biru
    \033[45m: Magenta
    \033[46m: Cyan
    \033[47m: Putih
*/

// Aktifkan ANSI escape codes
void enableANSIEscapeCodes() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hConsole, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hConsole, consoleMode);
}

// Cetak teks dengan warna
void printColored(const string& text, const string& foregroundColor, const string& backgroundColor = "" ) {
    string colorCode = "\033[" + foregroundColor;
    if (!backgroundColor.empty()) {
        colorCode += ";" + backgroundColor;
    }
    colorCode += "m";
    cout << colorCode << text << "\033[0m";
}

struct Score {
    string name;
    int score;
};

class GameBoard {
private:
    vector<vector<int>> grid;
    vector<Score> highScores;
    int currentScore;

public:
    const vector<Score>& getHighScores() const { return highScores; }
    int getCurrentScore() const { return currentScore; }

    void detectMatches(set<pair<int, int>>& matches, int row, int col, int color) {
        if (row < 0 || row >= 6 || col < 0 || col >= 6 || grid[row][col] != color) return;
        if (matches.count({row, col})) return;
        matches.insert({row, col});
        detectMatches(matches, row + 1, col, color);
        detectMatches(matches, row - 1, col, color);
        detectMatches(matches, row, col + 1, color);
        detectMatches(matches, row, col - 1, color);
    }

    bool hasMatches() {
        set<pair<int, int>> matches;
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 6; ++col) {
                matches.clear();
                detectMatches(matches, row, col, grid[row][col]);
                if (matches.size() >= 3) return true;
            }
        }
        return false;
    }

    

    // Menghapus elemen yang sudah cocok kemudian diganti dengan elemen candy yang baru
    void removeMatches(set<pair<int, int>>& matches) {
        for (const auto& match : matches) {
            grid[match.first][match.second] = 0; // Tandai sebagai dihapus
        }
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 5);
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 6; ++col) {
                if (grid[row][col] == 0) {
                    grid[row][col] = dis(gen);
                }
            }
        }
    }

    void reset() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 5);
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 6; col++) {
                grid[row][col] = dis(gen);
            }
        }
        set<pair<int, int>> matches;
        for (int row = 0; row < 6; ++row) {
            for (int col = 0; col < 6; ++col) {
                matches.clear();
                detectMatches(matches, row, col, grid[row][col]);
                if (matches.size() >= 3) {
                    grid[row][col] = dis(gen);
                    row = 0; col = 0;
                }
            }
        }
        currentScore = 0;
    }

    void sortHighScores() {
        sort(highScores.begin(), highScores.end(), [](const auto& a, const auto& b) { return a.score > b.score; });
        if (highScores.size() > 6) {
            highScores.resize(6); // Hanya sampai 6 skor tertinggi
        }
    }

    GameBoard() : grid(6, vector<int>(6, 0)), currentScore(0) {
        reset();
        highScores.push_back({"Dani", 484});
        highScores.push_back({"Deni", 91});
        highScores.push_back({"Dina", 914});
        sortHighScores();
    }

    void swapCandies(int row1, int col1, int row2, int col2) {
        swap(grid[row1][col1], grid[row2][col2]);
    }

    int getColor(int row, int col) const {
        if (row >= 0 && row < 6 && col >= 0 && col < 6) return grid[row][col];
        return 0;
    }

    void addHighScore(const string& name, int score) {
        highScores.push_back({name, score});
        sortHighScores();
    }

    void addToCurrentScore(int points) {
        currentScore += points;
    }
};

void printPanel(const GameBoard& game, int selectedRow, int selectedCol, bool inGrid, const string& playerName, const string& inputName, bool hasSelectedCandy, pair<int, int> selectedCandy, bool isEnteringName) {
    cout << "┌──────────────────────────────────────────┐" << endl;
    cout << "│ Candy Crush Lokal                  ─ □ x │" << endl;
    cout << "├───────────┬─────────────┬────────────────┤" << endl;
    cout << "│Leaderboard│ Nama: " << (playerName.length() == 0 ? "   " : playerName.length() == 1 ? playerName + "   " : playerName.length() == 2 ? playerName + "   " : playerName.length() == 3 ? playerName + "   " : playerName + "  " );
    for (int i = playerName.length(); i < 3; ++i) cout << " ";
    cout << "│ Score : " + to_string(game.getCurrentScore()) + (game.getCurrentScore() < 10 ? "      │" : game.getCurrentScore() < 100 ? "     │" : "    │") << endl;
    cout << "├───────────┴─────────────┴────────────────┤" << endl;

    const auto& highScores = game.getHighScores();
    for (int row = 0; row < 6; ++row) {
        // Panel leaderboard
        cout << "│ ";
        if (row < highScores.size()) {
            string scoreStr = to_string(highScores[row].score);
            string entry = highScores[row].name + " " + scoreStr;
            printColored(entry, "33;1"); // Kuning terang
            for (int i = entry.length(); i < 10; ++i) {
                cout << " ";
            }
        } else {
            cout << "          ";
        }

        // Panel grid
        cout << "│ ";
            for (int col = 0; col < 6; ++col) {
                int color = game.getColor(row, col);
                char symbol = (color == 1) ? '*' : (color == 2) ? '#' : (color == 3) ? '$' : (color == 4) ? '@' : '%';
                string colorCode;
                switch (color) {
                    case 1: colorCode = "31;1"; break; // Merah terang
                    case 2: colorCode = "32;1"; break; // Hijau terang
                    case 3: colorCode = "34;1"; break; // Biru terang
                    case 4: colorCode = "33;1"; break; // Kuning terang
                    case 5: colorCode = "35;1"; break; // Magenta terang
                    default: colorCode = "37"; break; // Putih
                }
                bool isSelected = inGrid && row == selectedRow && col == selectedCol;
                bool isSelectedCandy = hasSelectedCandy && row == selectedCandy.first && col == selectedCandy.second;
                if (isSelected || isSelectedCandy) {
                    printColored(string(1, symbol) + " ", colorCode, "43"); // Latar kuning
                } else {
                    printColored(string(1, symbol) + " ", colorCode);
                }
            }
        

        // Panel skor
        cout << "│ ";
        if (row == 0) {
            string scoreStr = "     Play";
            if (!inGrid && selectedRow == 0) {
                printColored(scoreStr, "34;1"); // biru
                for (int i = scoreStr.length(); i < 15; ++i) cout << " ";
            } else {
                printColored(scoreStr, "37;1"); // putih
                for (int i = scoreStr.length(); i < 15; ++i) cout << " ";
            }
        } else if (row == 2) {
            string scoreStr = "    Restart";
            if (!inGrid && selectedRow == 2) {
                printColored(scoreStr, "34;1"); //biru
                for (int i = scoreStr.length(); i < 15; ++i) cout << " ";
            } else {
                printColored(scoreStr, "37;1"); //putih
                for (int i = scoreStr.length(); i < 15; ++i) cout << " ";
            }
        } else if (row == 4) {
            string scoreStr = "     Exit";
            if (!inGrid && selectedRow == 4) {
                printColored(scoreStr, "34;1"); //biru
                for (int i = scoreStr.length(); i < 15; ++i) cout << " ";
            } else {
                printColored(scoreStr, "37;1"); //putih
                for (int i = scoreStr.length(); i < 15; ++i) cout << " ";
            }
        } else {
            cout << "               ";
        }
        cout << "│" << endl;
    }

    cout << "└──────────────────────────────────────────┘" << endl;
}

void playGame(GameBoard& game) {
    enum GameState { Menu, EnteringName, Playing };
    GameState gameState = Menu;
    string playerName;
    string inputName; // Untuk menampilkan nama yang sedang diketik
    int selectedRow = 0; // Mulai di tombol "Play"
    int selectedCol = 0;
    int lastGridRow = 0; // Simpan baris grid terakhir
    bool inGrid = false; // Mulai di panel tombol
    bool hasSelectedCandy = false;
    pair<int, int> selectedCandy = {-1, -1};

    while (true) {
        system("cls"); // Bersihkan layar
        printPanel(game, selectedRow, selectedCol, inGrid, playerName, inputName, hasSelectedCandy, selectedCandy, gameState == EnteringName);

        if(gameState == EnteringName) {
            cout << "Masukkan nama Anda (maksimal 4 karakter) : " << inputName << endl;
        }
        
        if (game.getCurrentScore() == 999)
        {
            cout << "Selamat! Anda telah mencapai skor maksimum!" << endl;
            game.addHighScore(playerName, game.getCurrentScore());
            game.reset();
            gameState = Menu;
            inGrid = false;
            selectedRow = 0;
            selectedCol = 0;
            lastGridRow = 0;
            playerName.clear();
            inputName.clear();
            hasSelectedCandy = false;
            selectedCandy = {-1, -1};
            continue;
        }
        

        // Tangkap input
        int ch = _getch();
        if (gameState == EnteringName) {
            if (ch == 13) { // Enter
                if (!inputName.empty() && inputName.length() <= 4) {
                    playerName = inputName;
                    gameState = Playing;
                    inGrid = true;
                    selectedRow = 0;
                    selectedCol = 0; // Pindah ke grid (0, 0)
                    inputName.clear();
                }
            } else if (ch == 8 && !inputName.empty()) { // Backspace
                inputName.pop_back();
            } else if (ch >= 32 && ch <= 126 && inputName.length() < 4) { // Karakter yang valid
                inputName += static_cast<char>(ch);
            }
            continue;
        }

        // Navigasi WASD
        if (gameState == Menu || gameState == Playing) {
            if (ch == 'w' || ch == 'W') {
                if (inGrid) {
                    if (selectedRow > 0) {
                        selectedRow--;
                    } else {
                        inGrid = false;
                        selectedRow = 0; // Kembali ke "Play"
                        lastGridRow = 0;
                    }
                } else {
                    if (selectedRow == 2) selectedRow = 0; // Dari "Restart" ke "Play"
                    else if (selectedRow == 4) selectedRow = 2; // Dari "Exit" ke "Restart"
                }
            } else if (ch == 's' || ch == 'S') {
                if (inGrid) {
                    if (selectedRow < 5) {
                        selectedRow++;
                    } else {
                        inGrid = false;
                        selectedRow = 0; // Kembali ke "Play"
                        lastGridRow = 5;
                    }
                } else {
                    if (selectedRow == 0) selectedRow = 2; // Dari "Play" ke "Restart"
                    else if (selectedRow == 2) selectedRow = 4; // Dari "Restart" ke "Exit"
                }
            } else if (ch == 'a' || ch == 'A') {
                if (inGrid && selectedCol > 0) {
                    selectedCol--;
                } else if (!inGrid && gameState == Playing && selectedRow == 0) {
                    inGrid = true;
                    selectedCol = 5; // Kembali ke grid, kolom paling kanan
                    selectedRow = lastGridRow; // Gunakan baris grid terakhir
                }
            } else if (ch == 'd' || ch == 'D') {
                if (inGrid && selectedCol < 5) {
                    selectedCol++;
                } else if (inGrid && selectedCol == 5) {
                    inGrid = false;
                    selectedRow = 0; // Kembali ke "Play"
                    lastGridRow = selectedRow; // Simpan baris grid
                }
            } else if (ch == 13) { // Enter
                if (!inGrid) {
                    if (selectedRow == 0 && gameState == Menu) { // Play
                        gameState = EnteringName;
                        inputName.clear();
                    } else if (selectedRow == 2 && gameState == Playing) { // Restart
                        game.addHighScore(playerName, game.getCurrentScore());
                        game.reset();
                        gameState = Menu;
                        inGrid = false;
                        selectedRow = 0;
                        selectedCol = 0;
                        lastGridRow = 0;
                        playerName.clear();
                        inputName.clear();
                        hasSelectedCandy = false;
                        selectedCandy = {-1, -1};
                    } else if (selectedRow == 4) { // Exit
                        break; // Keluar dari permainan
                    }
                } else if (gameState == Playing) {
                    if (!hasSelectedCandy) {
                        hasSelectedCandy = true;
                        selectedCandy = {selectedRow, selectedCol};
                    } else {
                        // Coba tukar dengan candy yang dipilih sebelumnya
                        int r1 = selectedCandy.first, c1 = selectedCandy.second;
                        int r2 = selectedRow, c2 = selectedCol;
                        if (abs(r1 - r2) + abs(c1 - c2) == 1) { // cek apakah posisi bersebelahan
                            game.swapCandies(r1, c1, r2, c2);
                            set<pair<int, int>> matches;
                            game.detectMatches(matches, r1, c1, game.getColor(r1, c1));
                            if (matches.size() < 3) {
                                matches.clear();
                                game.detectMatches(matches, r2, c2, game.getColor(r2, c2));
                            }
                            if (matches.size() >= 3) {
                                int points = matches.size();
                                game.addToCurrentScore(points);
                                game.removeMatches(matches);
                            } else {
                                game.swapCandies(r1, c1, r2, c2); // nilai ditukar ke semula lagi
                            }
                        }else{
                            hasSelectedCandy = false;
                        }   
                        selectedCandy = {-1, -1};
                    }
                }
            }
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    enableANSIEscapeCodes();
    GameBoard game;
    playGame(game);
    return 0;
}