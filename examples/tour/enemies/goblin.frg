#include <stdio>
#include <"bosses">

void spawn(string name){
    print("goblin {name} appears (total enemies: {count()})");
}

int count(){
    return 1 + enemies.bosses.dragon.HEADS;
}
