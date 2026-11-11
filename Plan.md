# Battleship Plan

## Implementations in order
1. 7-Segment display using message scroller function
    - Initially it will display the name of the game (Battleship)
    - Throughout the game it will display different messages
    - The messages should include announcing the player turns, current score as the game progresses, and to announce the winner at the end
    - Will use the message scroller if >8 characters

2. Boat Placement Functionality
    - Each player will place 3 single segment boats and 2 double segment boats
    - The map consists of two separate bitmaps, one for the vertical segments which is an array of 2 by 16 elements (32 total), and the other for horizontal segments which is an array of 3 by 8 elements (24 total)
    - the most memory efficient method is to use two unsigned integers. The vertical segments can be allocated to the upper 16 bits of a 32 bit integer for the top row of vertical segments, and the lower 16 bits of a 32 bit integer for the bottom row of vertical segments. The three rows of horizontal segments can be represented by three 8 bit bytes withing a 32 bit integer. 1’s would represent a boat and 0’s empty space.

3. Cursor Control
    - Use two potentiometers for vertical and horizontal movement for square selection
    - A switch needs to be allocated to select between the cursor being on the vertical or the horizontal segment maps
    - The cursor should blink to differentiate from previous shots/boats

4. Shot Maps
    - For each player's boat map we also need to record where shots were placed by the opponent
    - During game play, shots that missed should be shown at 50% brighness and shots that hit should be shown at 100% brightness
    - 1’s represent shot placement with 0’s representing non shots

5. Turns
    - Players will alternate turns taking one shot each until one player has hit all 7 of their opponents boats
    - Once a player wins there an appropriate message should appear declaring the winner

6. Display Buffer
    - A display buffer is highly recommended to create an image of what is to be written to the 7 segment displays., making it easier to manage the displays rather than trying to update them directly
    - The Systik handler is a great place to manage the display buffer and display updating, as well as implementing the PWM for brightness control and cursor blinking functions
    - We will have 10 levels of dimming if a 10 millisecond frame rate is used

7. Sound Effects
    - Sound can be used to punctuate hits and misses (explosions vs. splashes)
    - a song can be played to celebrate the winner (nautical themes are recommended)
    - Timer 7 is the best place to manage sounds and you can use lab 6 as a basis to aid with this

## Deliverables
1. Submit your C program files (.C and .H files that you modified) to Canvas in a ZIP file. Be sure to comment your code or you will be docked points! This submittal is a team effort.

2. Include a paragraph in your report discussing what you felt was the most challenging part of your project and how you met this challenge.

3. Obtain check-off by showing the operation of your program to your instructor or lab assistant.

4. Individually answer the three ABET questions found in the Assignments section in Canvas. You will get points for answering these questions.
