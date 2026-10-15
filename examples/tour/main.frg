#include <stdio>
#include <math>
#include <"enemies">

``` 
  A tour of what Forge can do so far.
  Functions can be used before they're written - see describe() at the bottom.
```

const int MAX_LEVEL = 3;

int main(){
    print("== Forge tour ==");

    // variables need a type
    int hp = 15;
    double price = 19.99;
    float chance = 0.000001;
    bool alive = true;
    string name = "Hero";
    uint8 small = 250;
    small += 10;            // wraps around: 250 + 10 = 4
    int64 big = 9000000000;

    print("{name} has {hp} HP, alive = {alive}");
    print("price {price}, chance {chance}, small {small}, big {big}");
    print("7 / 2 = {7 / 2}, 7.0 / 2 = {7.0 / 2}, 7 % 3 = {7 % 3}");

    // functions from other files always use the file name
    player.jump(2.5);
    player.score += 100;
    print("score is now {player.score}");

    // subfolders: folder.file.function()
    enemies.goblin.spawn("Grok");
    enemies.bosses.dragon.roar();

    // loops
    for (int level = 1; level <= MAX_LEVEL; level++) {
        write("level {level} ");
    }
    print();

    int countdown = 3;
    while (countdown > 0) {
        write("{countdown}... ");
        countdown--;
    }
    print("go!");

    // and / or / not work just like && / || / !
    if (alive and not (hp < 10)) {
        print("healthy");
    } else if (alive && hp < 10) {
        print("hurt");
    } else {
        print("gone");
    }

    print(describe(hp));
    print("sqrt(2) = {math.sqrt(2)}, max = {math.max(3, 8)}, clamp = {math.clamp(1.5, 0.0, 1.0)}");
    print("int(\"42\") + 1 = {int(\"42\") + 1}, int(3.9) = {int(3.9)}, PI = {math.PI}");
    print("braces: {{like this}}");

    return 0;
}

string describe(int value){
    if (value > 10) {
        return "big";
    }
    return "small";
}
