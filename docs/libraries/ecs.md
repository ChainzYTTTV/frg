# `<ECS>`: Entities and Components

```cpp
#include <ECS>
```

An alternative way to organize a game. Instead of one big class per kind of thing (a `Player` class, a `Goblin` class...), every game object is an **Entity**: a named container. You give it **Components**: small, reusable pieces of behavior like `Health`, `Mover` or `Sprite`. Mixing components makes different things: a goblin might be `Health` + `Mover` + `ChasePlayer`, and a crate just `Health`.

(ECS stands for Entity Component System. Forge's version is the friendly "game objects with components" style, as in Unity, rather than a data-oriented engine.)

## Components

A component is a class that inherits from `Component`, filling in any of these methods:

| Method | Runs |
|---|---|
| `void start()` | once, just before the component's first `update` |
| `void update(float dt)` | every time `ECS.update(dt)` is called |
| `void draw()` | every time `ECS.draw()` is called |

| Method you can call | Gives back | What it does |
|---|---|---|
| `entity()` | `Entity?` | the entity this component belongs to (null if it hasn't been added to one) |

```cpp
class Health : Component {
    int hp = 3;

    void update(float dt) {
        if (hp <= 0) {
            entity().destroy();
        }
    }
}

class Mover : Component {
    vec2 position = vec2.ZERO;
    vec2 velocity = vec2.ZERO;

    void update(float dt) {
        position += velocity * dt;
    }

    void draw() {
        graphics2d.drawCircle(position, 10, color.GOLD);
    }
}
```

## Entity

| Making one | |
|---|---|
| `ECS.create(string name)` | makes an entity **and adds it to the world** (usually what you want) |
| `Entity(string name)` | makes an entity that isn't in the world yet; add it later with `ECS.add(e)` |

| Field | Type | Meaning |
|---|---|---|
| `name` | `string` | its name, used by `ECS.find` |
| `active` | `bool` | when false, its components don't update or draw (true to start with) |
| `components` | `list<Component>` | its components, in the order they were added |

| Method | Gives back | What it does |
|---|---|---|
| `add(Component c)` | — | attaches a component |
| `get(string type)` | `Component?` | the first component of that class (or of a class that inherits from it), or null |
| `has(string type)` | `bool` | true if it has one |
| `remove(string type)` | `bool` | removes the first component of that class |
| `destroy()` | — | removes the entity from the world at the end of the current `ECS.update` |

Getting a component as its real class uses [`as`](../language/09-structs-and-classes.md#is-and-as-what-kind-of-object-is-it):

```cpp
Health h = slime.get("Health") as Health;
h.hp -= 1;
```

The type name is written as text because Forge has no way to pass a type itself. A typo just means nothing is found (`get` gives `null`).

## The world

| Function | What it does |
|---|---|
| `ECS.create(string name)` | makes and adds an entity |
| `ECS.add(Entity e)` | adds an entity made with `Entity(...)` |
| `ECS.update(float dt)` | runs `start()` (the first time) and `update(dt)` on every component of every active entity, then removes destroyed entities |
| `ECS.draw()` | runs `draw()` on every component of every active entity |
| `ECS.find(string name)` | the first entity with that name (`Entity?`) |
| `ECS.all()` | a list of every entity |
| `ECS.withComponent(string type)` | a list of every entity that has that component |
| `ECS.clear()` | removes every entity |
| `ECS.count()` | how many entities there are |

It's safe for a component to create or destroy entities (including its own) during `update`.

## Putting it together

This uses the `Mover` and `Health` components from above, written in the same file.

```cpp
#include <window>
#include <graphics2d>
#include <ECS>

int main(){
    window.open(800, 450, "ECS");

    Entity ball = ECS.create("ball");
    Mover m = Mover();
    m.position = vec2(100, 200);
    m.velocity = vec2(120, 0);
    ball.add(m);
    ball.add(Health());

    window.run();
    return 0;
}

void update(float dt){
    ECS.update(dt);
}

void draw(){
    ECS.draw();
}
```

## Components talking to each other

A component can find its siblings through its entity:

```cpp
class Poison : Component {
    void update(float dt) {
        Health? h = entity().get("Health") as Health;
        if (h != null) {
            h.hp -= 1;
        }
    }
}
```
