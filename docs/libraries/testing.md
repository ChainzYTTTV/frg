# `<testing>`: Testing Your Game's Code

```cpp
#include <testing>
```

**Tests** are small functions that use your game's code and check that it gives the right answers: "after healing 5, the hero has 15 hp". `frg test` runs them all and says which ones passed. They catch mistakes without playing the game, and they keep catching them after you change something.

## Where tests go

Tests live in a folder called **`tests`** next to `main.frg`. Folders only join a game when a file `#include`s them, so the tests are never part of the game itself; `frg test` adds them when it runs.

```
mygame/
  main.frg
  hero.frg
  tests/
    hero_test.frg
```

Inside `tests/`, every function whose name starts with `test` (and takes nothing and gives nothing back) is a test. Other functions there are ordinary helpers. Test files reach the game's code the usual way, with the file's name first: `hero.Hero`, `level.build()`.

```cpp
// tests/hero_test.frg
#include <testing>

void testHealAddsHp(){
    hero.Hero h = hero.Hero();
    h.heal(5);
    testing.check(h.hp == 15);
}

void testHealStopsAtTwenty(){
    hero.Hero h = hero.Hero();
    h.heal(50);
    testing.check(h.hp == 20, "hp should never go over 20");
}
```

A folder inside `tests/` joins the tests the same way as anywhere in Forge: a file next to it writes `#include <"folder">`.

## Running them

```
frg test
```

In VS Code: **Forge: Run Tests** (Ctrl+Shift+P). The game's own `main()` doesn't run; the tests do, one after another:

```
  PASS  testHealAddsHp  (tests/hero_test.frg)
  FAIL  testHealStopsAtTwenty  (tests/hero_test.frg)
        tests/hero_test.frg:12: testing.check(h.hp == 20, "hp should never go over 20");
          left side: 25, right side: 20
          hp should never go over 20

1 test passed, 1 failed (2 checks).
```

- A failed check doesn't stop its test, so one run shows every check that failed.
- A runtime error stops only that test (it fails, with the error and the calls that led to it); the next test still runs.
- `frg test` finishes with exit code 1 if anything failed, and 0 if everything passed.
- The test program is built in `build/tests/`, so the game's own `.exe` isn't touched.

## Functions

| Function | What it does |
|---|---|
| `testing.check(bool ok, string message = "")` | the test fails if `ok` is false. It shows the line as written, and for a comparison (`==`, `!=`, `<`, `>`, `<=`, `>=` between numbers, text, `bool`s or enum values) both values too. The message is shown under it. |
| `testing.fail(string message = "")` | the test fails here, with the message (for code that should never be reached, or a test that isn't finished) |

## Tips

- **One idea per test**, named after it: `testJumpNeedsGround`, `testCoinAddsScore`. When one fails, its name says what broke.
- **Make fresh objects in each test.** Global variables keep their values from one test to the next, because the tests run one after another in the same program.
- **Test the rules, not the drawing.** Code that needs the window (drawing, input) can't run in a test, but the rules of the game (scores, health, inventories, levels made from text) can.
- `frg check` and VS Code's error underlines check the test files too.
