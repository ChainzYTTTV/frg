# Practice Projects

The fastest way to learn programming is to make things. These projects go from tiny console programs to small games. Each one says what it practices, splits the work into steps, and gives hints for the tricky parts.

You don't have to do them all, or in order. Pick one that sounds fun at your level. If one is too hard, do an easier one first and come back.

Each project names a **stage**: the point in learning where it fits. Roughly:

| Stage | You know |
|---|---|
| 1-5 | printing, variables, `if`, loops, functions ([chapters 1-6](../language/01-basics.md)) |
| 6-10 | lists and maps, structs and classes, enums and optionals, files ([chapters 7-10](../language/08-lists-and-maps.md)) |
| 11-14 | the window, drawing, keyboard, physics ([the game loop](game-loop.md)) |
| 15-20 | tasks, images and sound, menus, components, 3D |

## How to do a project

- **Build it in small steps.** Each project is split into steps. Get each step running before starting the next. A program that works but does less is better than a big one that doesn't run.
- **Run it often.** After every few lines, `frg run` it. If something breaks, it's in the lines you just wrote.
- **Read the error.** Forge's messages say what's wrong and usually how to fix it (look at the `hint:` line).
- **Use `print` to look inside.** If a number isn't what you expect, `print("x = {x}")` shows it. The console window shows `print` output even in window games.
- **Stuck for more than 15 minutes?** Read the hints for that project. They name the functions you'll need and show the trickiest piece.
- **Finished?** Try a "make it yours" idea, or make up your own. Changing a working program is one of the best ways to learn.

The **Stage** says which lesson stage the project fits, from the table above. The projects get harder from top to bottom.

## Level 1: Console programs

These only use the console: no window. They need `#include <stdio>` for `print` and `input`, plus whatever the hints mention.

### 1. Number guessing game

**Stage 4.** The computer thinks of a number from 1 to 100. The player guesses, and is told "too high" or "too low" until they get it. At the end, show how many guesses it took.

**Practices:** `random.range`, `input`, turning text into a number, `while`, `if` / `else`, counting.

Steps:

1. Pick the secret number and print it (just while testing, so you can check the game works).
2. Ask for one guess and say whether it's too high, too low or right.
3. Put the asking in a loop that stops when the guess is right.
4. Count the guesses and print the count at the end. Remove the line that printed the secret.

Hints:

- `#include <random>`, then `int secret = random.range(1, 100);`. Both 1 and 100 can come up.
- `input` gives back text. Turn it into a number with `int(...)`: `int guess = int(input("Your guess: "));`. Typing something that isn't a number stops the program with an error. To ask again instead, check the text first with `isInt()`: `string typed = input("Your guess: ");`, then `if (typed.isInt()) { int guess = int(typed); ... } else { print("Please type a number"); }`.
- A `bool found = false;` and `while (!found) { ... }` is one way to loop until the guess is right.

Make it yours: allow only 7 guesses. Ask "Play again? (y/n)" at the end. Let the player choose easy (1-50) or hard (1-500).

### 2. Math quiz

**Stage 5.** Ten questions like `7 + 12 = ?` with random numbers. Count the right answers, then show the score as a percentage.

**Practices:** `for` loops, functions that `return` a value, `switch`, and how division works.

Steps:

1. Write `bool askQuestion()`. It makes two random numbers, asks for their sum, and gives back `true` if the answer was right.
2. Call it 10 times in a `for` loop, counting the `true`s.
3. Print the score, and the percentage.
4. Pick `+`, `-` or `*` at random for each question, with `switch`.

Hints:

- The percentage is the classic division trap. `score / 10 * 100` uses whole numbers, so 7 right gives `0`. Use `float(score) / 10 * 100`. (Forge warns about the whole-number version when it goes into a `float`.) Show it with one decimal: `print("{percent:.1}%");`.
- `random.range(1, 3)` picks which operation, and `switch` on that number chooses what to ask.

Make it yours: time the whole quiz with `<time>` (`time.now()` at the start and end). Make the numbers bigger as the score goes up.

### 3. Shopping list

**Stage 6.** A menu that repeats until the player quits:

```
1) add an item   2) remove an item   3) show the list   4) quit
```

**Practices:** `list<string>` and its methods, a menu loop, `switch` on text.

Steps:

1. Make an empty `list<string> items = [];` and a loop that shows the menu and reads a choice.
2. Make "add" and "show" work. Show each item with its number: `1. milk`.
3. Make "remove" work. Say so if the item isn't in the list.
4. Make "quit" end the loop.

Hints:

- `switch (choice) { case "1": ... case "4": ... }` works on text.
- `items.remove(name)` gives back `false` if the item wasn't there.
- `for (int i = 0; i < items.length; i++)` gives you the position, so you can print `{i + 1}. {items[i]}`.

Make it yours: use a `map<string, int>` to keep how many of each item. Save the list with `<file>` (`file.writeLines("list.txt", items)`) and load it at the start (`file.readLines`, after checking `file.exists`).

### 4. High-score table

**Stage 6.** Players type a name and a score. Keep the top 5, best first, and save them so they're still there next time.

**Practices:** a `struct`, lists of structs, putting things in order yourself, `<file>`.

Steps:

1. Make `struct Entry { string name; int score; }` and a `list<Entry>`.
2. Add entries and print them.
3. Keep the list in order: put each new entry **before** the first entry with a lower score (`insert`). If there's none, add it at the end.
4. After adding, if there are more than 5, remove the last one (`pop()`).
5. Save each entry as a line like `Ana,120`, and load them at the start.

Hints:

- `sort()` only sorts lists of numbers or text, which is why step 3 keeps the list in order by hand.
- Loading: `file.readLines("scores.txt")` gives a list of lines. For each line, `line.split(",")` gives `["Ana", "120"]`, and `int(parts[1])` is the score.

Make it yours: show a medal (`*`) next to the top score. Don't allow empty names (`name.trim().isEmpty()`).

### 5. Text adventure

**Stage 9.** A tiny world of 4 or 5 rooms. The player types `north`, `south`, `look` or `take key`, and has to find a key to open the last door.

**Practices:** `enum`, `switch`, functions, `bool` flags, planning a program before writing it.

Steps:

1. Draw your rooms on paper first, with arrows between them.
2. Make `enum Room { Hall, Kitchen, Garden, Cellar }` and a variable for the room the player is in.
3. Write `void describe(Room room)`, which prints a description of each room using `switch`.
4. Read commands in a loop. Moving changes the room, if there's a way to go in that direction.
5. Add the key: `bool hasKey = false;`. One room has it, and the last door only opens with it.

Hints:

- Write one function per room, like `Room kitchen(string command)`, that gives back the room the player ends up in. Then the main loop stays short.
- `command.trim().lower()` makes `"  North "` into `"north"`.

Make it yours: an inventory `list<string>`, and an `inventory` command. A room that's dark until you find a lamp. Split it into files: `rooms.frg` with the room functions, called as `rooms.kitchen(command)`. That's stage 10.

### 6. Tic-tac-toe

**Stage 6.** Two players take turns on a 3×3 board in the console, until someone wins or the board is full.

**Practices:** a list as a game board, nested lists, functions that check things.

Steps:

1. Store the board as `list<string> board = [" ", " ", " ", " ", " ", " ", " ", " ", " "];`, positions 0 to 8.
2. Write `void showBoard()`, which prints it as three rows.
3. Take turns: ask for a position (1-9), and put `X` or `O` there if it's free.
4. Write `bool hasWon(string mark)`, which checks every winning line.
5. Stop on a win or a full board.

Hints:

- All 8 winning lines, as positions:

  ```cpp
  list<list<int>> LINES = [[0, 1, 2], [3, 4, 5], [6, 7, 8], [0, 3, 6], [1, 4, 7], [2, 5, 8], [0, 4, 8], [2, 4, 6]];
  ```

  Then `for (list<int> line in LINES)` checks whether `board[line[0]]`, `board[line[1]]` and `board[line[2]]` are all `mark`.
- The board is full when `!board.contains(" ")`.

Make it yours: a computer player. First let it pick a random free square. Then make it smarter: win if it can, otherwise block the other player.

### 7. Turn-based battle

**Stage 8.** A hero fights a line of monsters, one at a time. Each turn the player picks attack, special or heal, and then the monster hits back.

**Practices:** classes, inheritance, replacing methods, lists of objects, `random`.

Steps:

1. Make `class Fighter` with `name`, `hp`, `maxHp`, `strength`, and `void attack(Fighter target)`.
2. Make a hero and one monster. Loop turns until one has 0 `hp`.
3. Make kinds of monster: `class Goblin : Fighter`, `class Dragon : Fighter`. Each replaces `attack` with its own (the dragon sometimes breathes fire for double damage).
4. Put several monsters in a `list<Fighter>`, and fight them in order.

Hints:

- Random damage: `int damage = random.range(strength - 2, strength + 2);`.
- Keep `hp` from going below 0 with `math.max(0, hp - damage)`.
- A `Goblin` can go into a `list<Fighter>`, and calling `attack` on it runs the goblin's own version.

Make it yours: an `enum Action { Attack, Special, Heal }`. Items the hero might be holding: `Weapon? weapon = null;`. Gold and a shop between fights.

## Level 2: Window games

These open a window: `#include <window>` and `<graphics2d>`, and `<input>` for the keyboard. Remember that **y goes down**: `y = 0` is the top of the window. Use `window.run()` with `update(float dt)` and `draw()`, and multiply speeds by `dt`.

### 8. Bouncing balls

**Stage 11.** A ball moves around the window and bounces off the edges. Then lots of balls, each with its own color and speed.

**Practices:** `vec2`, moving with `dt`, edges, a list of structs.

Steps:

1. One ball: a `vec2 position` and a `vec2 velocity`, moved every frame and drawn with `drawCircle`.
2. Bounce: when it goes past an edge, flip that part of the velocity.
3. Make `struct Ball { vec2 position; vec2 velocity; color tint; }`, and a list of 20 random balls.
4. Space adds a ball at the mouse.

Hints, the core of one ball:

```cpp
vec2 position = vec2(400, 225);
vec2 velocity = vec2(200, 150);
float RADIUS = 20;

void update(float dt){
    position += velocity * dt;
    if (position.x < RADIUS or position.x > window.width() - RADIUS) {
        velocity.x = -velocity.x;
    }
    if (position.y < RADIUS or position.y > window.height() - RADIUS) {
        velocity.y = -velocity.y;
    }
}

void draw(){
    graphics2d.drawCircle(position, RADIUS, color.ORANGE);
}
```

- A ball in a list is a struct, which is a copy. To change it, change `balls[i].position`, not a copy taken out of the list with `for (Ball b in balls)`.
- A random color: `random.color()`. For brighter colors, pick each part yourself: `color(random.range(50, 255), random.range(50, 255), random.range(50, 255))`.

Make it yours: balls bounce off each other (`a.position.distance(b.position) < 2 * RADIUS`). Gravity pulls them down.

### 9. Pong

**Stage 12.** Two paddles, one ball. W/S moves the left paddle, Up/Down the right one. A player scores when the ball gets past the other paddle.

**Practices:** `rect`, `overlaps`, keyboard input, keeping score, resetting.

Steps:

1. Draw two paddles (`rect`) and move them with the keys. Keep them inside the window.
2. Add the ball from project 8, bouncing off the top and bottom only.
3. When the ball's rect overlaps a paddle, flip its x velocity.
4. When it leaves on the left or right, score a point and put it back in the middle.
5. Show the score at the top with `drawText`.

Hints:

- The ball as a rect: `rect(position.x - 10, position.y - 10, 20, 20)`, then `ballRect.overlaps(leftPaddle)`.
- After flipping, also move the ball out of the paddle, or it can stick inside and flip every frame.
- Center text: `float x = (window.width() - graphics2d.textWidth(text, 40)) / 2.0;` (the `.0` keeps the half pixel; with `/ 2` Forge warns that the decimals are cut off).

Make it yours: the ball speeds up a little on every hit. A computer player that follows the ball (not too fast, or it can't lose). First to 5 wins.

### 10. Catch the falling stars

**Stage 13.** Stars fall from the top. The player moves a basket left and right to catch them. A missed star costs a life; three misses and it's game over.

**Practices:** spawning things, removing things from a list, a game-over state.

Steps:

1. The basket: a rect at the bottom, moved with Left/Right.
2. One falling star that starts at a random x at the top.
3. A `list<vec2> stars`, with a new one added every second.
4. Caught stars and missed stars are removed from the list, and change the score or lives.
5. Game over: stop the game and show "Game over - press Enter". Enter starts again.

Hints:

- A timer for spawning: add `dt` to a counter every frame, and when it passes 1 second, spawn a star and take 1 off.

  ```cpp
  float spawnTimer = 0;
  // in update:
  spawnTimer += dt;
  if (spawnTimer >= 1) {
      spawnTimer -= 1;
      stars.add(vec2(random.range(20, window.width() - 20), -20));
  }
  ```

- **Removing while looping:** go through the list **backwards**, so removing an item doesn't skip the next one:

  ```cpp
  for (int i = stars.length - 1; i >= 0; i--) {
      if (stars[i].y > window.height()) {
          stars.removeAt(i);
          lives--;
      }
  }
  ```

- `bool gameOver = false;`. In `update`, when it's true, only check for Enter. In `draw`, show the message.

Make it yours: stars fall faster as the score grows. Rare golden stars worth 5. A sound when one is caught (`<audio>`).

### 11. Snake

**Stage 13.** The classic: the snake moves on a grid, eats food, grows, and dies if it hits a wall or itself.

**Practices:** grid positions, moving on a fixed beat instead of every frame, using a list as a queue.

Steps:

1. Pick a cell size (20 pixels) and keep the snake as a `list<vec2>` of **grid** positions (like `(5, 7)`), with the head first.
2. Draw each part as a square: `drawRect(part.x * 20, part.y * 20, 20, 20, color.GREEN)`.
3. Move a step every 0.15 seconds (the same timer trick as project 10).
4. The arrow keys change the direction, but not straight back into itself.
5. Food at a random cell. Eating it grows the snake; hitting a wall or itself ends the game.

Hints:

- One step: make the new head, put it at the front, and take the tail off the end.

  ```cpp
  vec2 head = snake[0] + direction;   // direction is vec2(1, 0), vec2(0, -1)...
  snake.insert(0, head);
  snake.pop();                        // skip this line on a step where it ate, and it grows
  ```

- Hit itself: check whether any **other** part equals the new head (`==` compares vectors).

Make it yours: walls in the middle of the field. Speed up every 5 food. A best score saved with `<file>`.

### 12. Breakout

**Stage 13.** A paddle at the bottom, a ball, and rows of bricks at the top. The ball breaks a brick when it hits it.

**Practices:** building a grid of things with nested loops, collisions from different sides.

Steps:

1. The paddle (mouse or arrow keys) and a ball that bounces off the walls and the paddle. Losing the ball off the bottom costs a life.
2. Bricks: a `list<rect>`, made with two `for` loops, rows and columns.
3. When the ball overlaps a brick, remove the brick and flip the ball's y velocity.
4. Win when the list is empty.

Hints:

- Making the bricks:

  ```cpp
  for (int row = 0; row < 5; row++) {
      for (int col = 0; col < 10; col++) {
          bricks.add(rect(col * 80 + 2, row * 30 + 40, 76, 26));
      }
  }
  ```

- Only break **one** brick per frame (`break` out of the loop after a hit), or the ball can flip twice and carry on through.

Make it yours: a different color and score for each row. Where the ball hits the paddle changes its angle. Power-ups that fall from broken bricks.

### 13. Clicker game

**Stage 17.** Click a big cookie to earn cookies. Spend them on upgrades: more per click, or helpers that earn cookies every second. Progress is saved.

**Practices:** the mouse, `<ui>` buttons, earning over time with `dt`, saving and loading, scenes.

Steps:

1. A big circle in the middle. Clicking inside it adds 1 cookie. Show the total.
2. A `ui.button` "Better click (cost 10)". Buying it makes clicks worth more, and raises its cost.
3. A "Helper" upgrade: each helper earns 1 cookie per second (`cookies += helpers * dt`).
4. Save the numbers to a file when the game closes, and load them when it starts: `file.writeLines("save.txt", ["{cookies}", "{helpers}"])`.
5. A title scene with "Play" and "Reset save" buttons (`<scenes>`).

Hints:

- Clicked inside the circle: `input.mousePressed(MouseButton.LEFT) and input.mousePosition().distance(center) < radius`.
- Helpers earn fractions of a cookie, so keep `float cookies`, and show `int(cookies)` (or `"{cookies:.1}"` for one decimal).
- Save several numbers as several lines with `file.writeLines`, and read them back in the same order.

Make it yours: the cookie shrinks a little when clicked, then grows back. Achievements at 100, 1,000 and 10,000 cookies.

### 14. Dodge

**Stage 15.** Blocks fly in from the sides. Survive as long as possible. The difficulty is run by **tasks**, which makes waves, pauses and flashes easy.

**Practices:** `task` functions and `wait`, timing events without freezing the game.

Steps:

1. A player square that moves in all four directions, and a survival timer at the top.
2. A `task spawnWaves()` that loops forever: spawn 5 blocks, `wait(3)`, spawn more, and every few waves make them faster.
3. Getting hit takes a life, and starts a `task flash()` that makes the player blink for a second (and can't be hit while blinking).
4. Game over when the lives run out.

Hints:

- A blink with a task:

  ```cpp
  bool visible = true;

  task flash(){
      for (int i = 0; i < 6; i++) {
          visible = !visible;
          wait(0.15);
      }
      visible = true;
  }
  ```

  Start it with `flash();`. The game keeps running while it waits.

Make it yours: "Wave 3!" text that appears for 2 seconds (another task). A shield pickup.

### 15. Platformer remix

**Stage 16.** Take the example platformer (Part 5) and make it your own game.

**Practices:** reading and changing a bigger program, `<physics>`, `<assets>`, `<audio>`, `<scenes>`.

Ideas, roughly from easiest to hardest:

1. Change the colors, the jump height and the speed until it feels right to you.
2. Add more platforms and coins, and a door that only opens when all coins are collected.
3. Give the player a sprite and a jump sound.
4. A moving platform: a static `Body` whose position you change every frame, going back and forth.
5. An enemy that walks left and right on a platform. Touching it sends you back to the start.
6. A second level, using `<scenes>`, and a title screen.

## Level 3: Bigger projects

### 16. Top-down shooter with components

**Stage 19.** A ship in the middle of the screen turns and shoots at asteroids coming from every side. Built from `<ECS>` components.

**Practices:** designing with components, `is` / `as`, angles with `math.sin` / `math.cos`.

Steps:

1. Components: `Movement` (position and velocity) and `Health`, plus one per kind of thing that says how it looks and acts: `Ship`, `Bullet`, `Asteroid`.
2. The ship turns with Left/Right and shoots with Space. A bullet is an entity with a `Movement` and a `Lifetime` (it destroys itself after 1 second).
3. Asteroids are entities made at the edges, moving toward the middle.
4. Collisions: compare the distance between bullets and asteroids. Big asteroids split into two small ones.

Hints:

- A direction from an angle: `vec2(math.cos(angle), math.sin(angle))`, with the angle in radians (`degrees * math.PI / 180`).
- `ECS.withComponent("Asteroid")` gives every asteroid entity, ready to check against the bullets.

### 17. Solar system in 3D

**Stage 20.** A sun in the middle, planets going round it at different speeds, and a moon going round one planet. The camera can turn around the scene.

**Practices:** `<graphics3d>`, circles with `math.sin` and `math.cos`, `Camera`.

Steps:

1. A camera looking at the middle, a `drawGrid`, and a yellow sphere for the sun.
2. One planet: its position is `vec3(math.cos(t) * distance, 0, math.sin(t) * distance)`, where `t` grows every frame.
3. Several planets, each a struct with its own distance, speed, size and color.
4. A moon: the same circle, but around its planet's position instead of the middle.
5. Left/Right turns the camera: keep a `Camera cam`, call `cam.orbit(degrees)`, then `graphics3d.setCamera(cam)`.

Make it yours: lines showing the orbits (many short `drawLine`s around a circle). Up/Down zooms. Labels with each planet's name, placed with `graphics3d.toScreen`.

### 18. Your own game

**Any stage.** Pick a simple game you like and make a small version of it. Good first choices: Flappy Bird, Space Invaders, Frogger, Asteroids, Tetris (hard!), a maze game.

Before writing code:

1. Write down, in one sentence, what the player does.
2. List the **smallest** version that's still fun to play for a minute. That's version 1.
3. Write the steps for version 1, like the projects above.

Then build version 1, play it, and only then add more.
