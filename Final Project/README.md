Main Program Overview

This file implements the final project game logic, display control, audio system, and task scheduler for an AVR-based embedded game inspired by Stacker Blocks. The system integrates LED matrix displays, a TFT screen, audio output, push buttons, and status LEDs, all coordinated using a timer-driven task scheduler.

Core Architecture

The program uses a cooperative task scheduler driven by a hardware timer interrupt. Two main tasks run concurrently:

Game Controller Task – handles gameplay, input, graphics, and state transitions

Audio Controller Task – generates sound effects and melodies for stacking, winning, and losing

The TimerISR() function updates each task at fixed intervals using a GCD-based scheduler, ensuring predictable timing without blocking delays.

Game Logic

The game simulates a stacking block game using four chained MAX7219 LED matrix displays (32 columns total):

A 2×4 block moves vertically within a column

Pressing the button “locks” the block into place

Each new layer must overlap the previous one

Missed overlap results in a loss

Successfully filling all columns results in a win

Game state is stored in a gameBoard[32] array, where each column represents an 8-bit vertical slice of the LED matrix.

Game States

GAME_INIT – reset game and hardware

GAME_WAITING_START – wait for start button

GAME_PLAYING – active gameplay

GAME_WIN – win condition reached

GAME_LOSE – loss condition reached

Display Systems
LED Matrix (MAX7219)

Four MAX7219 drivers control a 32×8 LED matrix

SPI-like bit-banging is used for communication

Each frame redraws the game board and moving block

Displays are mapped logically so the game scrolls left to right

TFT Screen (ST7735)

Used for text-based feedback

Displays:

“YOU WIN” screen

“YOU LOSE” screen

Ready screen

Letters are drawn manually using rectangle primitives (custom font)

Audio System

Audio is generated using Timer0 PWM output:

Different sound sequences are played for:

Block stacking

Winning

Losing

Each sound effect consists of:

A sequence of frequencies

Per-note beat durations

The audio task advances notes based on beat counters rather than delays

This allows audio playback to run in parallel with gameplay without blocking execution.

Input & Feedback
Buttons

One button for stacking blocks

One start/reset button

Edge detection is used to avoid repeated triggers

LEDs

Green LEDs indicate a win

Red LEDs indicate a loss

All LEDs reset cleanly between games
