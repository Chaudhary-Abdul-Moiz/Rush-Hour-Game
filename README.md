
# 🚕🚚 RUSH HOUR

A top-down, randomly generated driving game with two modes: **Taxi** and **Truck**. Drive, deliver, and dodge obstacles in a procedurally created city grid. Compete for the highest score on the leaderboard!

---

## 🖥️ Platform Compatibility

- ✅ Windows (via `game.exe`)
- ✅ Ubuntu Linux (via `make` and `./game`)
- ⚠️ **Important**: The game **must** be played at:
  - Resolution: `1920x1080`
  - Scaling: `100%`
  - Otherwise, clicking and graphics may not align due to coordinate mismatches.
- ⚠️ Note: The game was originally developed on Ubuntu. Windows users may experience **slight coordinate mismatches**, but these do **not affect playability**.

---

## 🔧 Installation & Running

### ▶️ On Windows

1. Navigate to the extracted folder.
2. Double-click on `game.exe`.
3. Play!

### 🐧 On Ubuntu

1. Open the terminal in the game directory (where `game.cpp` is located).
2. Ensure required libraries are installed:
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential libsdl2-dev libsdl2-mixer-dev freeglut3-dev
   ```
3. Compile the game:
   ```bash
   make
   ```
4. Run the game:
   ```bash
   ./game
   ```

> Note: Library installation is **one-time only**. Re-run `make` only if you modify source files. Else you don't even have to type `make` just run `./game` on the terminal.

---

## 🎮 Game Features

### 🗺️ Map & Objects

- The city map is **randomly generated** each playthrough.
- However, you always:
  - **Spawn at the top-left** corner.
  - **Garage is at the bottom-left** corner.
- Map elements:
  - **White tiles** = Roads
  - **Black tiles** = Buildings
  - **Other objects**:
    - Fuel stations
    - Trees
    - AI-controlled cars
    - Passengers (Taxi mode)
    - Packages (Truck mode)

### 🚖 Game Modes

| Mode   | Score per Mission | Penalties | Fuel Tank Size | Fuel Consumption | Target |
|--------|-------------------|-----------|----------------|------------------|--------|
| Taxi   | Lower             | Lower     | Normal         | Normal           | Passengers |
| Truck  | Higher            | Higher    | 80% Bigger     | 2x Higher        | Packages |

Switch between modes **inside the garage** using a specific control (check the game controls screen).

### 🧠 AI Behavior

- AI cars move randomly in one direction.
- Every step, there's a **5% chance** they change direction.

---

## 🎯 Scoring System

- **Positive score**:
  - Successfully delivering passengers/packages.
- **Negative score**:
  - Running into:
    - Trees
    - Fuel stations
    - Passengers / Packages
    - AI Cars
- **Important**:
  - To engage (pickup/deliver), **stand next to** the object and press the appropriate key.
  - Trying to run over interactable objects will result in a **blop sound** and **score penalty**.
  - Hitting an AI car causes it to disappear and respawn elsewhere with a score penalty.

---

## 🛑 Game Over Conditions

- Score drops below zero
- Fuel runs out
- Time expires

---

## 🏆 Leaderboard

- Stores top 10 scores with player names.
- Aim to climb to the top!

---

## 🕹️ Controls

- View full control scheme via the **Game Controls** button on the main menu and on the game paused screen.
- Includes:
  - Movement
  - Interaction
  - Volume
  - Mode switching

---

## 📁 File Structure (Overview)

```
Project/
│
├── game.exe              # Windows executable
├── game.cpp              # Main source code
├── util.cpp/h            # Utility functions
├── game.o / util.o       # Object files
├── Makefile              # For compiling on Linux
├── CImg.h                # Image handling header
├── install-libraries.sh  # Helper script for Ubuntu
├── leaderboard.txt       # Stores leaderboard scores
├── SDL2.dll, *.dll       # Required DLLs for Windows
```

---

## ✅ Notes

- Game was designed for educational purposes.
- Code is modular and separated for clarity (`util.cpp`, `game.cpp`, etc.).
