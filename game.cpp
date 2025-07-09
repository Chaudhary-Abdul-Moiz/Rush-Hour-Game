//============================================================================
// Name        : .cpp
// Author      : Dr. Sibt Ul Hussain
// Version     :
// Copyright   : (c) Reserved
// Description : Basic 2D game...
//============================================================================

#ifndef RushHour_CPP_
#define RushHour_CPP_
#include "util.h"
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <ctime>
// FOR SOUND
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

using namespace std;

/* Function sets canvas size (drawing area) in pixels...
 *  that is what dimensions (x and y) your game will have
 *  Note that the bottom-left coordinate has value (0,0) and top-right coordinate has value (width-1,height-1)
 * */
 
 
// Sound effects
Mix_Chunk* pickupSound = nullptr;
Mix_Chunk* dropoffSound = nullptr;
Mix_Chunk* gameStartSound = nullptr;
Mix_Chunk* gameLoseSound = nullptr;
Mix_Chunk* gameWinSound = nullptr;
Mix_Chunk* hitObstacleSound = nullptr;
Mix_Chunk* modeChangeSound = nullptr;
Mix_Chunk* carCollisionSound = nullptr;
Mix_Chunk* fuelSound = nullptr;
Mix_Chunk* buttonSound = nullptr;
bool audioInitialized = false;

Mix_Music* backgroundMusic = nullptr;
bool isMusicPlaying = false;

// Function to initialize SDL2 audio
bool InitAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
        return false;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
        return false;
    }
    
    // Load all sound effects
    pickupSound = Mix_LoadWAV("sounds/pickup.wav");
    dropoffSound = Mix_LoadWAV("sounds/dropoff.wav");
    gameStartSound = Mix_LoadWAV("sounds/RushHourThemeSong.wav");
    gameLoseSound = Mix_LoadWAV("sounds/game_lose.wav");
    gameWinSound = Mix_LoadWAV("sounds/game_win.wav");
    hitObstacleSound = Mix_LoadWAV("sounds/hit.wav");
    modeChangeSound = Mix_LoadWAV("sounds/mode_change.wav");
    carCollisionSound = Mix_LoadWAV("sounds/car_collision.wav");
    fuelSound = Mix_LoadWAV("sounds/fuel.wav");
    buttonSound = Mix_LoadWAV("sounds/Button Selection.wav");
    

    backgroundMusic = Mix_LoadMUS("sounds/RushHourThemeSong.wav");
    if (!backgroundMusic) {
        cout << "Failed to load background music! Error: " << Mix_GetError() << endl;
    }
    
    audioInitialized = true;
    return true;
}

// Function to play any sound
void PlaySound(Mix_Chunk* sound) {
    if (audioInitialized && sound != nullptr) {
        Mix_PlayChannel(-1, sound, 0);
    }
}
void PlayBackgroundMusic() {
    if (audioInitialized && backgroundMusic && !isMusicPlaying) {
        Mix_PlayMusic(backgroundMusic, -1); // -1 means loop indefinitely
        isMusicPlaying = true;
    }
}

void StopBackgroundMusic() {
    if (audioInitialized) {
        Mix_HaltMusic();
        isMusicPlaying = false;
    }
}

void PauseBackgroundMusic() {
    if (audioInitialized && isMusicPlaying) {
        Mix_PauseMusic();
    }
}

void ResumeBackgroundMusic() {
    if (audioInitialized && Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

void IncreaseMusicVolume() {
    if (audioInitialized) {
        int currentVolume = Mix_VolumeMusic(-1); // Get current volume
        Mix_VolumeMusic(min(currentVolume + 16, MIX_MAX_VOLUME)); // Increase by 16 (max 128)
    }
}

void DecreaseMusicVolume() {
    if (audioInitialized) {
        int currentVolume = Mix_VolumeMusic(-1); // Get current volume
        Mix_VolumeMusic(max(currentVolume - 16, 0)); // Decrease by 16 (min 0)
    }
}

void ToggleMuteMusic() {
    if (audioInitialized) {
        if (Mix_VolumeMusic(-1) > 0) { // If volume is not zero
            Mix_VolumeMusic(0); // Mute
        } else {
            Mix_VolumeMusic(MIX_MAX_VOLUME / 2); // Unmute to half volume
        }
    }
}

// Function to cleanup audio
void CleanupAudio() {
    StopBackgroundMusic();
    Mix_FreeMusic(backgroundMusic);
    backgroundMusic = nullptr;

    Mix_FreeChunk(pickupSound);
    Mix_FreeChunk(dropoffSound);
    Mix_FreeChunk(gameStartSound);
    Mix_FreeChunk(gameLoseSound);
    Mix_FreeChunk(gameWinSound);
    Mix_FreeChunk(hitObstacleSound);
    Mix_FreeChunk(modeChangeSound);
    Mix_FreeChunk(carCollisionSound);
    Mix_FreeChunk(fuelSound);
    Mix_FreeChunk(buttonSound);
    
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
} 
 

void SetCanvasSize(int width, int height) {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1); // set the screen size to given width and height.
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
}

/*
 * Main Canvas drawing function.
 * */
bool occupied[17][32] = {false};
bool occupied_passenger[17][32] = {false};
bool occupied_package[17][32] = {false};
bool occupied_tree[17][32] = {false};
bool occupied_fuel[17][32] = {false};

class Map {
private:
    const int cols = 32;
    const int rows = 17;
    int grid[17][32];

    void FloodFill(int row, int col, bool visited[17][32]) const {
        // Check bounds
        if (row < 0 || row >= rows || col < 0 || col >= cols) return;
        
        // Check if already visited or not a road
        if (visited[row][col] || grid[row][col] != 1) return;
        
        // Mark as visited
        visited[row][col] = true;
        
        // Recursively visit all 4 directions
        FloodFill(row + 1, col, visited);  // Up
        FloodFill(row - 1, col, visited);  // Down
        FloodFill(row, col + 1, visited);  // Right
        FloodFill(row, col - 1, visited);  // Left
    }

public:
    Map() {
        srand(time(0));
        for(int i=0; i<rows; i++) {
            for(int j=0; j<cols; j++) {
                int random_number = rand()%4;
                if(random_number)
                    grid[i][j] = 1; // ROAD...
                else
                    grid[i][j] = 0; // BUILDING
            }
        }
        for(int i=0;i<17;i++) {      // FIRST COLUMN WILL ALWAYS BE ROAD.....
            grid[i][0] = 1;
        }  
    }

    void Draw() {
        int x_board = 0, y_board = 0;
        for(int i=0; i<rows; i++) {
            x_board = 0;
            for(int j=0; j<cols; j++) {
                if(grid[i][j] == 1)
                    DrawSquare(x_board, y_board, 60, colors[WHITE]);
                else
                    DrawSquare(x_board, y_board, 60, colors[BLACK]);
                x_board += 60;
            }
            y_board += 60;
        }
    }
    
    bool IsRoad(int row, int col) const {
        return grid[row][col];
    }

    int GetRows() const {
        return rows;
    }

    int GetCols() const {
        return cols;
    }
    
    bool IsReachable(int targetRow, int targetCol) const {
        // Player starts at (0, 960) which is grid position (16, 0)
        int startRow = 16;  // y=960 -> row=16
        int startCol = 0;   // x=0 -> col=0
        
        // Create visited array
        bool visited[17][32] = {false};
        
        // Perform flood fill from starting position
        FloodFill(startRow, startCol, visited);
        
        // Check if target position was reached
        return visited[targetRow][targetCol];
    }
    
    // NEW FUNCTION: Get a random reachable road position
    bool GetRandomReachablePosition(int& row, int& col) const {
        const int MAX_ATTEMPTS = 1000;  // Prevent infinite loops
        int attempts = 0;
        
        while(attempts < MAX_ATTEMPTS) {
            col = rand() % 32;
            row = rand() % 17;
            
            if(IsRoad(row, col) && !occupied[row][col] && IsReachable(row, col) && !(row==0 && col==0) && !(row==16 && col==0)) {
                return true;  // Found a valid position
            }
            attempts++;
        }

        // If we can't find a random position, let's systematically search
        for(int i = 0; i < rows; i++) {
            for(int j = 0; j < cols; j++) {
                if(IsRoad(i, j) && !occupied[i][j] && IsReachable(i, j)) {
                    row = i;
                    col = j;
                    return true;
                }
            }
        }
        
        return false;  // No valid position found
    }
};

Map gameMap;


const int count_passenger = rand()%3 + 2;
const int count_package = rand()%3 + 2;


class Passenger {
public:
    int x, y;
    bool isActive;
    Passenger() { Place(); }

    void Place() {
        int row, col;
        if(gameMap.GetRandomReachablePosition(row, col)) {
            x = col * 60;
            y = row * 60;
            occupied[row][col] = true;
            occupied_passenger[row][col] = true;
            isActive = true;
        } else {
            // Fallback: place at starting position if no other position available
            x = 0;
            y = 960;
            occupied[16][0] = true;
            occupied_passenger[16][0] = true;
            isActive = true;
        }
    }

    void Draw() {
        if(isActive) {
            DrawCircle(x + 30, y + 50, 8, colors[BLACK]);                   //HEAD
            DrawLine( x + 30, y + 20, x + 30, y + 50, 3 , colors[BLACK] );  // BODY
            DrawLine( x + 30, y + 20, x + 20, y + 5, 3 , colors[BLACK] );   // LEFT LEG
            DrawLine( x + 30, y + 20, x + 40, y + 5, 3 , colors[BLACK] );   // RIGHT LEG
            DrawLine( x + 30, y + 40, x + 15, y + 25, 3 , colors[BLACK] );   // LEFT ARM
            DrawLine( x + 30, y + 40, x + 45, y + 25, 3 , colors[BLACK] );   // RIGHT ARM
        }
    }
    void Respawn() {
        // Clear old position
        int oldRow = y / 60;
        int oldCol = x / 60;
        occupied[oldRow][oldCol] = false;
        occupied_passenger[oldRow][oldCol] = false;
        
        Place();
    }
};

class Package {
public:
    int x, y;
    bool isActive;
    Package() { Place(); }

    void Place() {
        int row, col;
        if(gameMap.GetRandomReachablePosition(row, col)) {
            x = col * 60;
            y = row * 60;
            occupied[row][col] = true;
            occupied_package[row][col] = true;
            isActive = true;
        } else {
            // Fallback: place at starting position if no other position available
            x = 0;
            y = 960;
            occupied[16][0] = true;
            occupied_package[16][0] = true;
            isActive = true;
        }
    }

    void Draw() {
        if(isActive) {
            DrawRoundRect(x + 10, y + 8, 40, 40, colors[SADDLE_BROWN], 10);
            DrawLine(x + 30, y + 8,  x + 30, y + 48, 4 , colors[BLACK]);
            DrawLine(x + 10, y + 28,  x + 50, y + 28, 4 , colors[BLACK]);
            DrawLine(x + 30, y + 48,  x + 36, y + 58, 4 , colors[BLACK]);
            DrawLine(x + 30, y + 48,  x + 24, y + 58, 4 , colors[BLACK]);
        }
    }
    void Respawn() {
        // Clear old position
        int oldRow = y / 60;
        int oldCol = x / 60;
        occupied[oldRow][oldCol] = false;
        occupied_package[oldRow][oldCol] = false;
        
        Place();
    }
};

class Tree {
public:
    int x, y;
    bool isActive;
    Tree() { Place(); }

    void Place() {
        while (true) {
            int col = rand() % 32;
            int row = rand() % 17;
            if(gameMap.IsRoad(row, col) && !occupied[row][col]) {
                x = col * 60;
                y = row * 60;
                occupied[row][col] = true;
                occupied_tree[row][col] = true;
                isActive = true;
                break;
            }
        }
    }

    void Draw() {
        if(isActive) {
            DrawTriangle(x + 20, y + 5, x + 40, y + 5, x + 30, y + 50, colors[BROWN] );
            DrawCircle(x + 30, y + 40, 15, colors[DARK_GREEN]);
        }
    }
};

class FuelStation {
public:
    int x, y;
    bool isActive;
    FuelStation() { Place(); }

    void Place() {
        int row, col;
        if(gameMap.GetRandomReachablePosition(row, col)) {
            x = col * 60;
            y = row * 60;
            occupied[row][col] = true;
            occupied_fuel[row][col] = true;
            isActive = true;
        } else {
            // If no position available, don't place the fuel station
            isActive = false;
        }
    }

    void Draw() {
        if(isActive) {
            DrawRectangle(x + 15, y + 5, 30, 50, colors[RED]);
            DrawRectangle(x + 18, y + 35, 24, 17, colors[BLACK]);
            DrawLine(x + 45, y + 10,  x + 48, y + 15, 2 , colors[BLACK]);
            DrawLine(x + 48, y + 15,  x + 48, y + 45, 2 , colors[BLACK]);
            DrawLine(x + 48, y + 45,  x + 54, y + 48 , 3 , colors[BLACK]);
            DrawLine(x + 54, y + 48,  x + 59, y + 45 , 3 , colors[BLACK]);
        }
    }
};

class AICar {
public:
    int x, y;
    int direction; // 0=UP, 1=RIGHT, 2=DOWN, 3=LEFT ...
    bool isActive;

    AICar() {
        Place();
        direction = rand() % 4;
        isActive = true;
    }

    void Place() {
        while(true) {
            int col = rand() % 32;
            int row = rand() % 17;
            if(gameMap.IsRoad(row, col) && !occupied[row][col]) {
                x = col * 60;
                y = row * 60;
                occupied[row][col] = true;
                break;
            }
        }
    }
    
    void TryMove() {
        int col = x / 60;
        int row = y / 60;
        occupied[row][col] = false;  //  CLEARING CURRENT POSITION...
        
        switch(direction) {
            case 0: row++; break; // UP
            case 1: col++; break; // RIGHT
            case 2: row--; break; // DOWN
            case 3: col--; break; // LEFT
        }

        // CHECKING IF MOTION IS POSSIBLE OR NOT...
        if(col < 0 || col >= gameMap.GetCols() || row < 0 || row >= gameMap.GetRows() || !gameMap.IsRoad(row, col) || occupied[row][col]) {
            // CHANGING DIRECTION IF UNABLE TO MOVE...
            direction = (direction + 2) % 4; // REVERSING DIRECTION...
        }
        else {
            switch(direction) {
                case 0: y += 60; break;
                case 1: x += 60; break;
                case 2: y -= 60; break;
                case 3: x -= 60; break;
            }
        }

        // GIVING A 5% CHANCE TO CHANGE DIRECTION RANDOMLY...
        if(rand() % 100 < 5) {
            direction = rand() % 4;
        }

        // MARKING NEW POSITION AS OCCUPIED...
        int newCol = x / 60;
        int newRow = y / 60;
        occupied[newRow][newCol] = true;
    }

    void Draw() {
        if(isActive) {
            DrawCircle(x + 48, y + 10, 6, colors[BLACK]);
            DrawCircle(x + 12, y + 10, 6, colors[BLACK]);
            DrawRectangle(x + 5, y + 10, 50, 20, colors[BLUE]);
            DrawRectangle(x + 15, y + 25, 30, 15, colors[BLUE]);
        }
    }
};

int aiCarCount = 4;
AICar* aiCars = new AICar[aiCarCount];

Passenger *passenger = new Passenger[count_passenger];
Package *package = new Package[count_package];

const int tree_count = 3;
Tree tree[tree_count];

const int fuel_station_count = 2;
FuelStation fuelStation[fuel_station_count];

int totalTime = 180;
int remainingTime = totalTime;
int score = 0;
int wallet = 0;
int fare = 0;

string playerName = "";
bool inMenu = true;
bool showControls = false;
bool isPaused = false;
bool nameInputActive = false;
bool showLeaderboard = false; 


const int MAX_LEADERBOARD_ENTRIES = 10;
int leaderboardEntries = 0;

class Leaderboard {
private:
    struct Entry {
        string name;
        int score;
        int money;
    };
    const int MAX_ENTRIES = 10;
    Entry entries[10];
    int entryCount;
    
public:
    Leaderboard() : entryCount(0) {}
    
    void Load() {
        ifstream file("leaderboard.txt");
        entryCount = 0;
        
        if (file.is_open()) {
            string name;
            int score, money;
            
            while (file >> name >> score >> money && entryCount < MAX_ENTRIES) {
                // Convert underscores back to spaces in the name
                for (int j = 0; j < name.length(); j++) {
                    if (name[j] == '_') {
                        name[j] = ' ';
                    }
                }
                
                // Check for duplicates
                bool duplicate = false;
                for (int i = 0; i < entryCount; i++) {
                    if (entries[i].name == name && 
                        entries[i].score == score && 
                        entries[i].money == money) {
                        duplicate = true;
                        break;
                    }
                }
                
                if (!duplicate) {
                    entries[entryCount].name = name;
                    entries[entryCount].score = score;
                    entries[entryCount].money = money;
                    entryCount++;
                }
            }
            file.close();
        }
        else {
            cout << "Could not open leaderboard.txt" << endl;
        }
    }
    
    void Save() {
        ofstream file("leaderboard.txt");
        
        if (file.is_open()) {
            for (int i = 0; i < entryCount; i++) {
                string name_to_save = entries[i].name;
                for (int j = 0; j < name_to_save.length(); j++) {
                    if (name_to_save[j] == ' ') {
                        name_to_save[j] = '_';
                    }
                }
                file << name_to_save << " " << entries[i].score << " " << entries[i].money << endl;
            }
            file.close();
        } else {
            cout << "Error: Could Not Open leaderboard.txt For Writing" << endl;
        }
    }
    
    void AddEntry(string name, int score, int money) {
        // FIRST OF ALL LOAD THE ALREADY SAVED LEADER-BOARD... 
        Load();
        
        // CHECK IF THE SAME ENTRY ALREADY EXISTS...
        for (int i = 0; i < entryCount; i++) {
            if (entries[i].name == name && 
                entries[i].score == score && 
                entries[i].money == money) {
                return; // ENTRY ALREADY EXISTS, DON'T ADD IT AGAIN...
            }
        }
        
        // FINDING WHERE TO INSERT THE NEW SCORE...
        int insertPos = entryCount;
        for (int i = 0; i < entryCount; i++) {
            if (score > entries[i].score) {
                insertPos = i;
                break;
            }
        }
        
        // ONLY ADD IF THE SCORE IS IN THE TOP TEN ENTRIES...
        if (insertPos < MAX_ENTRIES) {
            // SHIFTING THE EXISTING ENTRIES DOWN...
            for (int i = min(entryCount, MAX_ENTRIES - 1); i > insertPos; i--) {
                entries[i] = entries[i-1];
            }
            
            // Insert new entry
            entries[insertPos].name = name;
            entries[insertPos].score = score;
            entries[insertPos].money = money;
            
            // Increase count if not full
            if (entryCount < MAX_ENTRIES) {
                entryCount++;
            }
            
            // SAVING THE UPDATED LEADER-BOARD...
            Save();
        }
    }
    
    void Draw() {
        DrawRectangle(0, 0, 1925, 1080, colors[BLACK]);
        
        DrawString(880, 980, "LEADERBOARD", colors[GOLD]);
        DrawLine(760, 960, 1170, 960, 3, colors[GOLD]);
        
        DrawString(700, 900, "Rank", colors[WHITE]);
        DrawString(800, 900, "Name", colors[WHITE]);
        DrawString(1100, 900, "Score", colors[WHITE]);
        DrawString(1200, 900, "Money", colors[WHITE]);
        
        for (int i = 0; i < entryCount; i++) {
            DrawString(715, 850 - i*50, Num2Str(i+1), colors[WHITE]);
            DrawString(800, 850 - i*50, entries[i].name, colors[WHITE]);
            DrawString(1100, 850 - i*50, Num2Str(entries[i].score), colors[WHITE]);
            DrawString(1200, 850 - i*50, "$" + Num2Str(entries[i].money), colors[WHITE]);
        }
        
        DrawRectangle(820, 300, 300, 60, colors[GREEN]);
        DrawString(940, 320, "BACK", colors[WHITE]);
        
        DrawRectangle(820, 220, 300, 60, colors[RED]);
        DrawString(940, 240, "EXIT", colors[WHITE]);
    }
    
    int GetEntryCount() const {
        return entryCount;
    }
    
    bool IsEntryInTopTen(int score) const {
        if (entryCount < MAX_ENTRIES) {
            return true; // THERE IS SPACE STILL AVAILABLE...
        }
        
        // CHECKING IF THE SCORE IS HIGHER THAN THE LOWEST SCORE IN THE LEADER-BOARD...
        return score > entries[entryCount - 1].score;
    }
};

Leaderboard leaderboard;

class Car {
public:
    int x, y;
    bool isTaxi;
    bool hasPassenger;
    bool hasPackage;
    int dropX=-1, dropY=-1;
    int passenger_number=0;
    int package_number=0;
    int fuel_taxi = 100;
    int fuel_truck = 180;
    int jobsCompleted = 0;
    int totalJobsCompleted = 0;
    
    Car(int startX, int startY, bool taxiMode) {
        x = startX;
        y = startY;
        isTaxi = taxiMode;
        hasPassenger = false;
        hasPackage = false;
    }

    void SwitchMode() {
        if((x/60 == 0) && (y/60 == 0)) {
            isTaxi = !isTaxi;
            Draw();
        }
        PlaySound(modeChangeSound);
    }

    void PickUp(Passenger &p, int n) {
        if(isTaxi && !hasPassenger && p.isActive && ( (abs(x - p.x) == 60 && y == p.y) || (abs(y - p.y) == 60 && x == p.x) )) {
            passenger_number = n;
            hasPassenger = true;
            p.isActive = false;
            int passengerX = p.x/60;
            int passengerY = p.y/60;
            occupied[passengerY][passengerX] = false;
            occupied_passenger[passengerY][passengerX] = false;
            dropX = rand() % 32;
            dropY = rand() % 17;
            if(gameMap.GetRandomReachablePosition(dropY, dropX)) {
                dropX = dropX; // Already set by GetRandomReachablePosition
                dropY = dropY;
            } else {
                // Fallback position if no reachable position found
                dropX = 0;
                dropY = 16;
            }
            PlaySound(pickupSound);
        }
    }
    
    void PickUp(Package &pkg, int n) {
        if(!isTaxi && !hasPackage && pkg.isActive && ( (abs(x - pkg.x) == 60 && y == pkg.y) || (abs(y - pkg.y) == 60 && x == pkg.x) )) {
            package_number = n;
            hasPackage = true;
            pkg.isActive = false;
            int packageX = pkg.x/60;
            int packageY = pkg.y/60;
            occupied[packageY][packageX] = false;
            occupied_package[packageY][packageX] = false;
            dropX = rand() % 32;
            dropY = rand() % 17;
            if(gameMap.GetRandomReachablePosition(dropY, dropX)) {
                dropX = dropX;
                dropY = dropY;
            } else {
                // Fallback position if no reachable position found
                dropX = 0;
                dropY = 16;
            }
            PlaySound(pickupSound);
        }
    }

    void DropPassenger(Passenger &p) {
        if (isTaxi && hasPassenger && dropX*60==x && dropY*60==y) {
            hasPassenger = false;
            score += 10;
            ++jobsCompleted;
            ++totalJobsCompleted;
            int fare = rand()%11 + 30;
            wallet += fare;
            p.Respawn();
            PlaySound(dropoffSound);
        }
    }

    void DropPackage(Package &pkg) {
        if (!isTaxi && hasPackage && dropX*60==x && dropY*60==y) {
            hasPackage = false;
            score += 20;
            ++jobsCompleted;
            ++totalJobsCompleted;
            int fare = rand()%11 + 50;
            wallet += fare;
            pkg.Respawn();
            PlaySound(dropoffSound);
        }
    }

    void Refill(FuelStation fuelStation) {
        if(wallet>=50 && ( (abs(x - fuelStation.x) == 60 && y == fuelStation.y) || (abs(y - fuelStation.y) == 60 && x == fuelStation.x) )) {
            if(isTaxi) {
                fuel_taxi = 100;
                wallet -= 50;
            }
            else {
                fuel_truck = 180;
                wallet -= 50;
            }
            PlaySound(fuelSound);
        }
    }

    void Draw() {
        if(dropX != -1 && dropY != -1 && (hasPackage || hasPassenger)) {
            DrawSquare(dropX * 60, dropY * 60, 60, colors[GREEN]);
        }
        if(isTaxi) {
            DrawCircle(x + 48, y + 10, 6, colors[BLACK]);
            DrawCircle(x + 12, y + 10, 6, colors[BLACK]);
            DrawRectangle(x + 5, y + 10, 50, 20, colors[DARK_ORANGE]);
            DrawRectangle(x + 15, y + 25, 30, 15, colors[DARK_ORANGE]);
        }
        if(!isTaxi) {
            DrawCircle(x + 48, y + 10, 6, colors[BLACK]);
            DrawCircle(x + 12, y + 10, 6, colors[BLACK]);
            DrawRectangle(x + 5, y + 10, 50, 20, colors[RED]);
            DrawRectangle(x + 15, y + 25, 30, 15, colors[RED]);
        }
        
        if(isTaxi && hasPassenger) {
            DrawCircle(x + 48, y + 10, 6, colors[BLACK]);
            DrawCircle(x + 12, y + 10, 6, colors[BLACK]);
            DrawLine(x + 10, y + 10,  x + 10, y + 55, 4 , colors[BLACK]);
            DrawTriangle(x + 10, y + 55, x + 10, y + 45, x, y + 50, colors[RED] );
            DrawRectangle(x + 5, y + 10, 50, 20, colors[DARK_ORANGE]);
            DrawRectangle(x + 15, y + 25, 30, 15, colors[DARK_ORANGE]);
            
        }
        if(!isTaxi && hasPackage) {
            DrawCircle(x + 48, y + 10, 6, colors[BLACK]);
            DrawCircle(x + 12, y + 10, 6, colors[BLACK]);
            DrawLine(x + 10, y + 10,  x + 10, y + 55, 4 , colors[BLACK]);
            DrawTriangle(x + 10, y + 55, x + 10, y + 45, x, y + 50, colors[RED] );
            DrawRectangle(x + 5, y + 10, 50, 20, colors[RED]);
            DrawRectangle(x + 15, y + 25, 30, 15, colors[RED]);
        }
    }
    void DrawFuel() {
        
        if(isTaxi) {
            DrawString(1400, 1040, "Fuel: ", colors[MISTY_ROSE]);
            DrawRectangle(1455, 1035, 100, 25, colors[SLATE_GRAY]);
            DrawRectangle(1455, 1035, fuel_taxi, 25, colors[GREEN]);
        }
        else {
            DrawString(1345, 1040, "Fuel: ", colors[MISTY_ROSE]);
            DrawRectangle(1400, 1035, 180, 25, colors[SLATE_GRAY]);
            DrawRectangle(1400, 1035, fuel_truck, 25, colors[GREEN]);
        }
    }
    
    void MoveUp() { y += 60; }
    void MoveDown() { y -= 60; }
    void MoveLeft() { x -= 60; }
    void MoveRight() { x += 60; }
    
    void TryMove(int dx, int dy) {
        int newX = x/60 + dx;
        int newY = y/60 + dy;

        if(newX<0 || newX>=gameMap.GetCols() || newY<0 || newY>=gameMap.GetRows()) {
            return;
        }

        if(!gameMap.IsRoad(newY, newX)) {
            return;
        }

        bool hitObstacle = false;
        int penalty = 0;

        if(isTaxi) {
            if(occupied_passenger[newY][newX]) {
                hitObstacle = true;
                penalty = 5;
            }
            else if(occupied_tree[newY][newX] || occupied_fuel[newY][newX]) {
                hitObstacle = true;
                penalty = 2;
            }
        }
        else {
            if(occupied_package[newY][newX]) {
                hitObstacle = true;
                penalty = 8;
            }
            else if(occupied_tree[newY][newX] || occupied_fuel[newY][newX]) {
                hitObstacle = true;
                penalty = 4;
            }
        }

        if(hitObstacle) {
            score -= penalty;
            PlaySound(hitObstacleSound);
        }
        else {
            if(dx == -1) { MoveLeft(); }
            else if(dx == 1) { MoveRight(); }
            else if(dy == 1) { MoveUp(); }
            else if(dy == -1) { MoveDown(); }
            
            if(isTaxi) { --fuel_taxi; }
            else { fuel_truck-=2; }
        }
    }

};

Car car(0,960,true);
bool gameOver = false;
string gameOverReason = "";


void CheckCollisionWithAICars() {
    int carCol = car.x / 60;
    int carRow = car.y / 60;
    
    for(int i = 0; i < aiCarCount; i++) {
        if(aiCars[i].isActive) {
            int aiCol = aiCars[i].x / 60;
            int aiRow = aiCars[i].y / 60;
            if(carCol == aiCol && carRow == aiRow) {
                aiCars[i].isActive = false;
                score -= 3;
                PlaySound(carCollisionSound);
                aiCars[i].Place();
                aiCars[i].isActive = true;
            }
        }
    }
}


void CheckGameOver() {
    if(remainingTime <= 0) {
        gameOver = true;
        gameOverReason = "      Time's Up!";
    }
    if((car.fuel_taxi<=0) || (car.fuel_truck <= 0)) {
        gameOver = true;
        gameOverReason = "     Out of Fuel!";
    }
    if(score<0) {
        gameOver = true;
        gameOverReason = "Score Below Zero!";
    }
    if(gameOver && playerName != "" && !showLeaderboard) {
        leaderboard.AddEntry(playerName, score, wallet);
    }
}

void DrawControlsScreen() {
    DrawRectangle(0, 0, 1925, 1080, colors[BLACK]);
    
    DrawString(850, 980, "GAME CONTROLS", colors[GOLD]);
    DrawLine(760, 960, 1170, 960, 3, colors[GOLD]);
    DrawString(700, 900, "Arrow Keys:", colors[WHITE]);DrawString(850, 900, "Move Vehicle", colors[WHITE]);
    DrawString(700, 850, "T:", colors[WHITE]);DrawString(850, 850, "Toggle Mode (Taxi/Truck) - Only at Garage", colors[WHITE]);
    DrawString(700, 800, "E:", colors[WHITE]);DrawString(850, 800, "Pick Up/Drop Off Passenger/Package", colors[WHITE]);
    DrawString(700, 750, "F:", colors[WHITE]);DrawString(850, 750, "Refuel at Fuel Station (Costs $50)", colors[WHITE]);
    DrawString(700, 700, "P:", colors[WHITE]);DrawString(850, 700, "Pause/Resume Game", colors[WHITE]);
    DrawString(700, 650, "ESC:", colors[WHITE]);DrawString(850, 650, "Exit Game", colors[WHITE]);

    DrawString(830, 580, "VOLUME CONTROLS", colors[GOLD]);
    DrawLine(760, 560, 1170, 560, 3, colors[GOLD]);
    DrawString(700, 510, "+:", colors[WHITE]);DrawString(850, 510, "Increase Background Music", colors[WHITE]);
    DrawString(700, 460, "-:", colors[WHITE]);DrawString(850, 460, "Decrease Background Music", colors[WHITE]);
    DrawString(700, 410, "M:", colors[WHITE]);DrawString(850, 410, "Mute Background Music", colors[WHITE]);

    // Back button
    DrawRectangle(820, 300, 300, 60, colors[GREEN]);
    DrawString(940, 320, "BACK", colors[WHITE]);
    // Exit Button
    DrawRectangle(820, 220, 300, 60, colors[RED]);
    DrawString(940, 240, "EXIT", colors[WHITE]);
}

void DrawPauseMenu() {
    DrawRectangle(0, 0, 1925, 1080, colors[BLACK]);
    DrawString(850, 900, "GAME PAUSED", colors[WHITE]);

    DrawRectangle(750, 680, 400, 60, colors[DARK_ORANGE]);
    DrawString(880, 700, "RESUME (P)", colors[WHITE]);

    DrawRectangle(750, 600, 400, 60, colors[GREEN]);
    DrawString(870, 620, "CONTROLS", colors[WHITE]);

    DrawRectangle(750, 520, 400, 60, colors[RED]);
    DrawString(880, 540, "QUIT GAME", colors[WHITE]);
}

bool cursorVisible = false;
int cursorBlinkTimer = 0;
const int CURSOR_BLINK_INTERVAL = 10;
void DrawStartMenu() {
    DrawRectangle(0, 0, 1925, 1080, colors[WHITE]);

    DrawString(780, 700, "PLEASE ENTER YOUR NAME!", colors[BLACK]);

    DrawRectangle(750, 600, 400, 50, colors[GRAY]);
    string displayText = "Name: " + playerName;
    if (nameInputActive && cursorVisible) {
        displayText += "_";  // Add cursor
    }
    DrawString(760, 615, displayText, colors[BLACK]);

    DrawRectangle(750, 500, 180, 50, colors[DARK_ORANGE]);
    DrawString(770, 515, "Taxi Driver", colors[BLACK]);

    DrawRectangle(970, 500, 180, 50, colors[RED]);
    DrawString(990, 515, "Truck Driver", colors[BLACK]);

    DrawString(775, 450, "Click A Role To Start The Game!", colors[BLACK]);

    DrawRectangle(750, 380, 400, 50, colors[BLUE]);
    DrawString(890, 395, "Leaderboard", colors[WHITE]);

    DrawRectangle(750, 300, 400, 50, colors[GREEN]);
    DrawString(872, 315, "Game Controls", colors[WHITE]);


    // R
    DrawRectangle(290, 1030, 125, 20, colors[BLUE]);
    DrawRectangle(290, 915, 125, 20, colors[BLUE]);
    DrawRectangle(290, 800, 20, 250, colors[BLUE]);
    DrawRectangle(395, 915, 20, 125, colors[BLUE]);
    DrawRectangle(310, 892, 20, 23, colors[BLUE]);
    DrawRectangle(330, 869, 20, 23, colors[BLUE]);
    DrawRectangle(350, 846, 20, 23, colors[BLUE]);
    DrawRectangle(370, 823, 20, 23, colors[BLUE]);
    DrawRectangle(390, 800, 20, 23, colors[BLUE]);
    // U
    DrawRectangle(445, 800, 20, 250, colors[BLUE]);
    DrawRectangle(555, 800, 20, 250, colors[BLUE]);
    DrawRectangle(445, 800, 125, 20, colors[BLUE]);
    // S
    DrawRectangle(600, 1030, 125, 20, colors[BLUE]);
    DrawRectangle(600, 915, 125, 20, colors[BLUE]);
    DrawRectangle(600, 800, 125, 20, colors[BLUE]);
    DrawRectangle(600, 915, 20, 125, colors[BLUE]);
    DrawRectangle(705, 800, 20, 125, colors[BLUE]);
    // H
    DrawRectangle(755, 800, 20, 250, colors[BLUE]);
    DrawRectangle(860, 800, 20, 250, colors[BLUE]);
    DrawRectangle(755, 915, 125, 20, colors[BLUE]);

    // H
    DrawRectangle(975, 800, 20, 250, colors[BLUE]);
    DrawRectangle(1080, 800, 20, 250, colors[BLUE]);
    DrawRectangle(975, 915, 125, 20, colors[BLUE]);
    // O
    DrawRectangle(1125, 800, 20, 250, colors[BLUE]);
    DrawRectangle(1230, 800, 20, 250, colors[BLUE]);
    DrawRectangle(1125, 800, 125, 20, colors[BLUE]);
    DrawRectangle(1125, 1030, 125, 20, colors[BLUE]);
    // U
    DrawRectangle(1275, 800, 20, 250, colors[BLUE]);
    DrawRectangle(1390, 800, 20, 250, colors[BLUE]);
    DrawRectangle(1285, 800, 125, 20, colors[BLUE]);
    // R
    DrawRectangle(1440, 1030, 125, 20, colors[BLUE]);
    DrawRectangle(1440, 915, 125, 20, colors[BLUE]);
    DrawRectangle(1440, 800, 20, 250, colors[BLUE]);
    DrawRectangle(1545, 915, 20, 125, colors[BLUE]);
    DrawRectangle(1460, 892, 20, 23, colors[BLUE]);
    DrawRectangle(1480, 869, 20, 23, colors[BLUE]);
    DrawRectangle(1500, 846, 20, 23, colors[BLUE]);
    DrawRectangle(1520, 823, 20, 23, colors[BLUE]);
    DrawRectangle(1540, 800, 20, 23, colors[BLUE]);
}

void DrawGameOverScreen() {
    DrawRectangle(0, 0, 1925, 1080, colors[BLACK]);
    
    DrawString(890, 900, "GAME OVER", colors[RED]);
    DrawString(870, 830, gameOverReason, colors[WHITE]);
    DrawString(895, 650, "Final Score: " + Num2Str(score), colors[WHITE]);
    DrawString(875, 610, "Money Earned: $" + Num2Str(wallet), colors[WHITE]);
    
    DrawRectangle(820, 350, 300, 60, colors[DARK_ORANGE]);
    DrawString(895, 370, "PLAY AGAIN", colors[WHITE]);
    
    DrawRectangle(820, 270, 300, 60, colors[GREEN]);
    DrawString(900, 290, "MAIN MENU", colors[WHITE]);
    
    DrawRectangle(820, 190, 300, 60, colors[BLUE]);
    DrawString(880, 210, "LEADERBOARD", colors[WHITE]);
    
    DrawRectangle(820, 110, 300, 60, colors[RED]);
    DrawString(940, 130, "EXIT", colors[WHITE]);
}


void GameDisplay()/**/{
	// set the background color using function glClearColor.
	// to change the background play with the red, green and blue values below.
	// Note that r, g and b values must be in the range [0,1] where 0 means dim rid and 1 means pure red and so on.

	glClearColor(0/*Red Component*/, 0,	//148.0/255/*Green Component*/,
	0.0/*Blue Component*/, 0 /*Alpha component*/); // Red==Green==Blue==1 --> White Colour
	glClear (GL_COLOR_BUFFER_BIT); //Update the colors
	
	// DISPLAY ROADS & BUILDINGS...
	gameMap.Draw();
	
	// DISPLAY PLAYABLE CAR...
	car.Draw();
	
	// DISPLAY OBSTACLES & OBJECTS...
	if(car.isTaxi) {
	    for(int i=0;i<count_passenger;i++) {
	        passenger[i].Draw();
	    }
	}
	if(!car.isTaxi) {
	    for(int i=0;i<count_package;i++) {
	        package[i].Draw();
	    }
	}
	for(int i=0;i<tree_count;i++) {
        tree[i].Draw();
    }
    for(int i=0;i<fuel_station_count;i++) {
        fuelStation[i].Draw();
    }
    for(int i = 0; i < aiCarCount; i++) {
        aiCars[i].Draw();
    }

	
	//DISPLAY SCORE...
	DrawString( 50, 1040, "Score: " + Num2Str(score), colors[MISTY_ROSE]);
	//DISPLAY TIME...
	DrawString(250, 1040, "Time Left: " + Num2Str(remainingTime) + " secs", colors[MISTY_ROSE]);
	//DISPLAY PLAYER'S NAME...
	DrawString(800, 1040, "Player: " + playerName, colors[GREEN]);
	//Display Fuel...
	car.DrawFuel();
	//DISPLAY MONEY...
	DrawString(1600, 1040, "$: " + Num2Str(wallet), colors[GREEN]);
	//DISPLAY MODE...
	if(car.isTaxi) {
        DrawString(1750, 1040, "Mode: Taxi", colors[DARK_ORANGE]);
    }
    else {
        DrawString(1750, 1040, "Mode: Truck", colors[RED]);
    }
	
	// DISPLAY GARAGE...
	DrawRectangle(5, 3, 50, 40, colors[BLACK]);
	DrawRectangle(10, 3, 40, 30, colors[SLATE_GRAY]);
	DrawLine(10, 9, 50, 9, 1, colors[WHITE]); 
	DrawLine(10, 15, 50, 15, 1, colors[WHITE]); 
	DrawLine(10, 21, 50, 21, 1, colors[WHITE]); 
	DrawLine(10, 27, 50, 27, 1, colors[WHITE]); 
	DrawLine(10, 33, 50, 33, 1, colors[WHITE]); 
	DrawLine(10, 39, 50, 39, 1, colors[WHITE]);
	DrawTriangle(3, 41, 28, 58, 57, 41, colors[BLACKO]); 
	
	
	// DISPLAY BOUNDARY...
	DrawLine( 0 , 0 ,  0 , 1020 , 10 , colors[car.isTaxi?DARK_ORANGE:RED] );           // LEFT BOUNDARY
	DrawLine( 1920 , 0 ,  1920 , 1020 , 10 , colors[car.isTaxi?DARK_ORANGE:RED] );     // RIGHT BOUNDARY
	DrawLine( 0 , 1020 ,  1920 , 1020 , 3 , colors[car.isTaxi?DARK_ORANGE:RED] );     // UP BOUNDARY
	DrawLine( 0 , 0 ,  1920 , 0 , 3 , colors[car.isTaxi?DARK_ORANGE:RED] );     // DOWN BOUNDARY
	
    if(showLeaderboard) {
        leaderboard.Draw();
    }
    else if(showControls) {
        DrawControlsScreen();
    }
    else if (isPaused) {
        DrawPauseMenu();
    }
    else if(inMenu) {
        DrawStartMenu();
    }
    else if(gameOver) {
        DrawGameOverScreen();
        StopBackgroundMusic();
    }
    else if(!inMenu && !showLeaderboard && !gameOver && !showControls) {
        PlayBackgroundMusic();
    }
    

    // DrawCircle(50,670,10,colors[RED]);
	// DrawRoundRect(350,100,100,50,colors[LIME_GREEN],20);
	// DrawSquare( 250 , 250 ,60,colors[GOLD]);
	// DrawLine( 0 , 0 ,  0 , 1020 , 10 , colors[GREEN] );
	// DrawTriangle( 300, 50 , 500, 50 , 400 , 250, colors[RED] ); 
	
	glutSwapBuffers(); // do not modify this line..

}



/*This function is called (automatically) whenever any non-printable key (such as up-arrow, down-arraw)
 * is pressed from the keyboard
 *
 * You will have to add the necessary code here when the arrow keys are pressed or any other key is pressed...
 *
 * This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
 * program coordinates of mouse pointer when key was pressed.
 *
 * */

void NonPrintableKeys(int key, int x, int y) {        
    if(!gameOver && !inMenu && !showLeaderboard) {
        int newX = car.x/60;
        int newY = car.y/60;
        if ((key == GLUT_KEY_LEFT) && !isPaused) {
            car.TryMove(-1, 0);
        }
        else if ((key == GLUT_KEY_RIGHT) && !isPaused) {
                car.TryMove(1, 0);
        }
        else if ((key == GLUT_KEY_UP) && !isPaused) {
                car.TryMove(0, 1);
        }
        else if ((key == GLUT_KEY_DOWN) && !isPaused) {
                car.TryMove(0, -1);
        }
    }
	/* This function calls the Display function to redo the drawing. Whenever you need to redraw just call
	 * this function*/

	glutPostRedisplay();

}

/*This function is called (automatically) whenever any printable key (such as x,b, enter, etc.)
 * is pressed from the keyboard
 * This function has three argument variable key contains the ASCII of the key pressed, while x and y tells the
 * program coordinates of mouse pointer when key was pressed.
 * */
 
void PrintableKeys(unsigned char key, int x, int y) {
    if (key == 27) {  // 27 IS ASCII FOR ESCAPE
    exit(1);
    }
    if (!gameOver && !inMenu) {
        if (key == 'p' || key == 'P') {  // Pause/Unpause
            isPaused = !isPaused;
            if(showControls)
                showControls = false;
            if (isPaused) {
                PauseBackgroundMusic();
            } else {
                ResumeBackgroundMusic();
            }
        }
        if ((key == 'f' || key == 'F') && !isPaused) {
            for(int i=0;i<fuel_station_count;i++) {
                car.Refill(fuelStation[i]);
            }
        }
        if ((key == 'e' || key == 'E') && !isPaused) {  // Pick up/Drop off
            // Pick up
            for(int i=0;i<count_passenger;i++) {
                car.PickUp(passenger[i], i);
            }
            for(int i=0;i<count_package;i++) {
                car.PickUp(package[i], i);
            }
            // Drop off
            car.DropPassenger(passenger[car.passenger_number]);
            car.DropPackage(package[car.package_number]);
        }
        if ((key == 't' || key == 'T') && !isPaused) {
            if(!(car.hasPackage || car.hasPassenger)) {
                car.SwitchMode();
            }
        }
        if (key == '+' || key == '=') {
            IncreaseMusicVolume();
        }
        else if (key == '-' || key == '_') {
            DecreaseMusicVolume();
        }
        else if (key == 'm' || key == 'M') {
            ToggleMuteMusic();
        }
    }
        
    if(inMenu && nameInputActive) {
        cursorVisible = true; // Make cursor visible when typing
        cursorBlinkTimer = 0; // Reset blink timer
        if(key==8) { // 8 IS ASCII FOR BACKSPACE
            if(!(playerName==""))
                playerName.pop_back();
        }
        else if(key==13) { // 13 IS ASCII FOR ENTER
            nameInputActive = false;
        }
        else if((key>='a' && key<='z') || (key>='A' && key<='Z') || (key==' ')) {
            if(playerName.length() < 25)
                playerName += key;
        }
    }
	glutPostRedisplay();
}



/*
 * This function is called after every 1000.0/FPS milliseconds
 * (FPS is defined on in the beginning).
 * You can use this function to animate objects and control the
 * speed of different moving objects by varying the constant FPS.
 *
 * */

 
void Timer(int m) {
	// implement your functionality here
    if (nameInputActive) {
        cursorBlinkTimer++;
        if (cursorBlinkTimer >= CURSOR_BLINK_INTERVAL) {
            cursorVisible = !cursorVisible;
            cursorBlinkTimer = 0;
        }
    }
	if(!inMenu && !gameOver && !showLeaderboard && !isPaused) {
	    // FOR TIME...
	    static int counter_time = 0;
	    ++counter_time;
	    if(counter_time>=10) {
            if(remainingTime > 0) {
                --remainingTime;
            }
            counter_time = 0;
	    }
	    // FOR MOVING AI CARS...
        static int frameCounter = 0;
        frameCounter++;

        int speedMultiplier = 1 + (car.jobsCompleted / 2);  // EVERY 2 JOBS = +1 SPEED MULTIPLIER...
        int movementThreshold = max(2, 10 - (speedMultiplier - 1) * 2);

        if(frameCounter >= movementThreshold) {
            for(int i = 0; i < aiCarCount; i++) {
                if(aiCars[i].isActive) {
                    aiCars[i].TryMove();
                }
            }
            CheckCollisionWithAICars();
            frameCounter = 0;
        }
    }
    CheckGameOver();

	glutPostRedisplay();
	// once again we tell the library to call our Timer function after next 1000/FPS
	glutTimerFunc(100, Timer, 0);
}

/*This function is called (automatically) whenever your mouse moves witin inside the game window
 *
 * You will have to add the necessary code here for finding the direction of shooting
 *
 * This function has two arguments: x & y that tells the coordinate of current position of move mouse
 *
 * */
void MousePressedAndMoved(int x, int y) {
	glutPostRedisplay();
}
void MouseMoved(int x, int y) {
	glutPostRedisplay();
}

/*This function is called (automatically) whenever your mouse button is clicked witin inside the game window
 *
 * You will have to add the necessary code here for shooting, etc.
 *
 * This function has four arguments: button (Left, Middle or Right), state (button is pressed or released),
 * x & y that tells the coordinate of current position of move mouse
 *
 * */
 
void GameOverButtonFunctionality() {
    gameOver = false;
    score = 0;
    wallet = 0;
    remainingTime = totalTime;
    car.x = 0;
    car.y = 960;
    car.fuel_taxi = 100;
    car.fuel_truck = 180;
    car.hasPassenger = false;
    car.hasPackage = false;
    car.dropX = -1;
    car.dropY = -1;
    car.jobsCompleted = 0;
    car.totalJobsCompleted = 0;

    for(int i=0;i<gameMap.GetRows();i++) {
        for(int j=0;j<gameMap.GetCols();j++) {
            occupied_passenger[i][j] = false;
            occupied_package[i][j] = false;
        }
    }
    for (int i = 0; i < count_passenger; i++) {
        passenger[i].Respawn();
    }
    for (int i = 0; i < count_package; i++) {
        package[i].Respawn();
    }
    for(int i = 0; i < aiCarCount; i++) {
        aiCars[i].Place();
        aiCars[i].isActive = true;
    }
}
 
void MouseClicked(int button, int state, int x, int y) {
    if (showLeaderboard && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= 790 && x <= 1077 && y >= 677 && y <= 733) {     // CHECKING IF BACK BUTTON IS PRESSED
            showLeaderboard = false;
            PlaySound(buttonSound);
        }
        else if (x >= 790 && x <= 1077 && y >= 753 && y <= 807) {     // CHECKING IF EXIT BUTTON IS PRESSED
            exit(1);
            PlaySound(buttonSound);
        }
    }
    else if (showControls && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= 790 && x <= 1077 && y >= 677 && y <= 733) {     // CHECKING IF BACK BUTTON IS PRESSED
            showControls = false;
            PlaySound(buttonSound);
        }
        else if (x >= 790 && x <= 1077 && y >= 753 && y <= 807) {     // CHECKING IF EXIT BUTTON IS PRESSED
            exit(1);
            PlaySound(buttonSound);
        }
    }
    else if(isPaused && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= 723 && x <= 1107 && y >= 320 && y <= 375) {     // CHECKING IF RESUME BUTTON IS PRESSED
            if(!showControls) {
                isPaused = false;
                ResumeBackgroundMusic();
                PlaySound(buttonSound);
            }
        }
        else if (x >= 723 && x <= 1107 && y >= 395 && y <= 450) {     // CHECKING IF CONTROLS IS PRESSED
            showControls = true;
            PlaySound(buttonSound);
        }
        else if (x >= 723 && x <= 1107 && y >= 470 && y <= 525) {     // CHECKING IF EXIT BUTTON IS PRESSED
            exit(1);
            PlaySound(buttonSound);
        }
    }
	else if(inMenu && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        bool clickedNameBox = (x>=723 && x<=1105 && y>=400 && y<=450);
        if(clickedNameBox) {
            nameInputActive = true;
            cursorVisible = true;
            cursorBlinkTimer = 0;
            PlaySound(buttonSound);
        }
        else {
            if(nameInputActive) {
                nameInputActive = false;
            }
        }

        if(x>=723 && x<=895 && y>=500 && y<=545) {     //  CHECKING IF TAXI BUTTON IS PRESSED
            if (playerName.length() > 0) {
                car.isTaxi = true;
                showLeaderboard = false;
                inMenu = false;
                PlaySound(buttonSound);
            }
        }
        else if (x>=935 && x<=1105 && y>=500 && y<=545) {     //  CHECKING IF TRUCK BUTTON IS PRESSED
            if (playerName.length() > 0) {
                car.isTaxi = false;
                showLeaderboard = false;
                inMenu = false;
                PlaySound(buttonSound);
            }
        }
        else if (x>=723 && x<=1105 && y>=610 && y<=655) {     //  CHECKING IF LEADER-BOARD BUTTON IS PRESSED
            leaderboard.Load();
            showLeaderboard = true;
            PlaySound(buttonSound);
        }
        else if (x>=723 && x<=1105 && y>=684 && y<=729) {     // CHECKING IF CONTROLS BUTTON IS PRESSED
            showControls = true;
            PlaySound(buttonSound);
        }
	}
    else if (gameOver && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (x >= 790 && x <= 1075 && y >= 630 && y <= 685) {      //  CHECKING IF PLAY AGAIN BUTTON IS PRESSED
            GameOverButtonFunctionality();
            PlaySound(buttonSound);
        }
        else if (x >= 790 && x <= 1075 && y >= 705 && y <= 760) {     //  CHECKING IF MAIN MENU BUTTON IS PRESSED
            inMenu = true;
            GameOverButtonFunctionality();
            PlaySound(buttonSound);
        }
        else if (x >= 790 && x <= 1075 && y >= 780 && y <= 835) {     //  CHECKING IF LEADER-BOARD BUTTON IS PRESSED
            leaderboard.Load();
            showLeaderboard = true;
            PlaySound(buttonSound);
        }
        else if (x >= 790 && x <= 1075 && y >= 855 && y <= 910) {     //  CHECKING IF EXIT BUTTON IS PRESSED
            exit(1);
            PlaySound(buttonSound);
        }
    }
	else if (button == GLUT_RIGHT_BUTTON) {}
	glutPostRedisplay();
}
/*
 * our gateway main function
 * */


int main(int argc, char*argv[]) {

	int width = 1920, height = 1080; // i have set my window size to be 800 x 600
        
    if (!InitAudio())
        cout << "Audio initialization failed - continuing without audio" << endl;
    
	InitRandomizer(); // seed the random number generator...
	glutInit(&argc, argv); // initialize the graphics library...
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // we will be using color display mode
	glutInitWindowPosition(50, 50); // set the initial position of our window
	glutInitWindowSize(width, height); // set the size of our window
	glutCreateWindow("OOP Project"); // set the title of our game window
	SetCanvasSize(width, height); // set the number of pixels...
	
	
	// Register your functions to the library,
	// you are telling the library names of function to call for different tasks.
	//glutDisplayFunc(display); // tell library which function to call for drawing Canvas.

	glutDisplayFunc(GameDisplay); // tell library which function to call for drawing Canvas.
	glutSpecialFunc(NonPrintableKeys); // tell library which function to call for non-printable ASCII characters
	glutKeyboardFunc(PrintableKeys); // tell library which function to call for printable ASCII characters
	// This function tells the library to call our Timer function after 1000.0/FPS milliseconds...
	glutTimerFunc(1000.0, Timer, 0);

	glutMouseFunc(MouseClicked);
	glutPassiveMotionFunc(MouseMoved); // Mouse
	glutMotionFunc(MousePressedAndMoved); // Mouse
	
	
	// now handle the control to library and it will call our registered functions when
	// it deems necessary...
	glutMainLoop();
	
	delete[] passenger;
	delete[] package;
	delete[] aiCars;
	CleanupAudio();
	return 1;
}
#endif /* RushHour_CPP_ */
