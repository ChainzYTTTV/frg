# 4. Operators

An **operator** combines or changes values: `+` adds, `>` compares, `and` checks two conditions.

**Every operator in Forge has two spellings: a symbol and a word.** They mean exactly the same thing and can be mixed freely, so use whichever reads better to you:

```cpp
if (hp <= 0 && !invincible) { ... }
if (hp less or equal 0 and not invincible) { ... }     // the same thing
```

## Math

| Symbol | Word | Meaning | Example | Result |
|---|---|---|---|---|
| `a + b` | `a plus b` | add | `7 plus 3` | `10` |
| `a - b` | `a minus b` | subtract | `7 minus 3` | `4` |
| `a * b` | `a times b` | multiply | `7 times 3` | `21` |
| `a / b` | `a div b` | divide | `7 div 2` | `3` (whole numbers, see below) |
| `a % b` | `a mod b` | remainder after dividing | `7 mod 3` | `1` |
| `-a` | `negative a` | flip the sign | `negative 5` | `-5` |

- **Division of two whole numbers gives a whole number**, cut toward zero: `7 / 2` is `3`. Make one side a decimal to get a decimal answer: `7.0 / 2` is `3.5`. See [How division works](02-variables-and-types.md#how-division-works) for the full rules, including the one special case.
- `%` / `mod` gives the remainder: `10 mod 3` is `1`. It's handy for "every Nth time" (`if (frame mod 10 == 0)`) and for wrapping around (`index = (index + 1) mod count`). It works on decimal numbers too: `7.5 mod 2` is `1.5`.
- Dividing a whole number by zero (with `/` or `%`) stops the game with a runtime error.
- `+` also joins two strings: `"Game" + "Over"`.
- `+` and `-` work on vectors (`vec2 + vec2`), and `*` and `/` multiply or divide a vector by a number. See [Built-in types](../libraries/core-types.md).
- **There is no power operator.** `^` means something else in Forge (see [Bits](#bits)). To square or cube a number, use `x * x`, `x * x * x`, or `math.pow(x, 2)`.

## Comparing

Comparisons give back `true` or `false`.

| Symbol | Word | Meaning |
|---|---|---|
| `a == b` | `a equals b` | equal |
| `a != b` | `a not equals b` | not equal |
| `a > b` | `a greater b` | greater than |
| `a < b` | `a less b` | less than |
| `a >= b` | `a greater or equal b` | greater than or equal to |
| `a <= b` | `a less or equal b` | less than or equal to |

```cpp
if (score greater or equal 100) { print("You win!"); }
if (name equals "Ana") { ... }
if (lives not equals 0) { ... }
```

What can be compared:

| Values | `==` `!=` | `<` `>` `<=` `>=` |
|---|---|---|
| numbers (any mix of number types) | yes | yes |
| strings | yes | yes (alphabetical order) |
| bools | yes | no |
| enums of the same type | yes | no |
| structs of the same type | yes: every field must match | no |
| classes | yes: true only if both are **the same object** | no |
| lists, maps | yes: same items in the same order | no |
| an optional and `null` | yes: `target == null` | no |

`=` (one equals sign) **sets** a variable; `==` (two) **compares**. Writing `if (x = 5)` is an error that tells you to use `==`.

## Logic

| Symbol | Word | Meaning |
|---|---|---|
| `a && b` | `a and b` | true if **both** are true |
| `a \|\| b` | `a or b` | true if **at least one** is true |
| `!a` | `not a` | flips true and false |

Both sides must be `bool`. `if (hp and alive)` is an error if `hp` is a number; write `if (hp > 0 and alive)`.

**`and` and `or` stop early:** in `a and b`, if `a` is false, `b` isn't even worked out (the answer is already false). That makes this safe:

```cpp
if (target != null and target.hp > 0) { ... }   // target.hp is only read when target isn't null
```

**`not` vs `!`:** they mean the same thing, but `not` is weaker, like in Python. `not a == b` means `not (a == b)`, while `!a == b` means `(!a) == b`. When in doubt, add parentheses.

## Choosing between two values

| Symbol | Word | Meaning |
|---|---|---|
| `condition ? a : b` | `a if condition else b` | `a` when the condition is true, otherwise `b` |
| `maybe ?? backup` | `maybe otherwise backup` | `maybe`, unless it's `null`, in which case `backup` |

```cpp
string label = hp > 50 ? "healthy" : "hurt";
string label2 = "healthy" if hp > 50 else "hurt";      // the same thing (Python style)

int? best = loadBestScore();
int shown = best ?? 0;
int shown2 = best otherwise 0;                          // the same thing
```

Both choices must be the same type, or types that fit together (like `int` and `float`, giving a `float`). For `if ... else`, the word `if` must be on the same line as the value before it. See [Optionals](10-enums-optionals-pointers.md) for more about `null`.

## Changing variables

| Symbol | Word | Meaning |
|---|---|---|
| `x = 5;` | `set x to 5;` | set `x` to 5 |
| `x += 5;` | `increase x by 5;` | add 5 to `x` |
| `x -= 5;` | `decrease x by 5;` | subtract 5 from `x` |
| `x *= 2;` | `multiply x by 2;` | multiply `x` by 2 |
| `x /= 2;` | `divide x by 2;` | divide `x` by 2 (whole-number division if `x` is a whole number) |
| `x %= 3;` | `set x to x mod 3;` | the remainder (no shorter word form) |
| `x++;` | `increase x;` | add 1 |
| `x--;` | `decrease x;` | subtract 1 |

These are statements, so they each go on their own line. They work on anything that can be changed: variables, fields, list items and map entries.

```cpp
set player.hp to 100;
increase scores[0] by 10;
decrease lives;
for (int i = 0; i less 10; increase i) { ... }
```

- `+=` works on strings too: `message += "!";`
- On a vector, `+=` and `-=` take a vector, and `*=` and `/=` take a number: `position += velocity * dt;`
- `++`/`--` (and `increase x;`/`decrease x;`) only work on numbers, and only as a statement on their own. There's no `y = x++;`.

## Bits

These work on the individual binary bits of whole numbers. Most games don't need them.

| Symbol | Word | Meaning | Example | Result |
|---|---|---|---|---|
| `a & b` | `a bitand b` | bits that are 1 in both | `6 & 3` | `2` |
| `a \| b` | `a bitor b` | bits that are 1 in either | `6 \| 3` | `7` |
| `a ^ b` | `a bitxor b` | bits that are 1 in exactly one | `6 ^ 3` | `5` |
| `~a` | `bitnot a` | flip every bit | `~0` | `-1` |
| `a << n` | `a shiftleft n` | move bits left (multiply by 2ⁿ) | `1 << 4` | `16` |
| `a >> n` | `a shiftright n` | move bits right (divide by 2ⁿ) | `256 >> 2` | `64` |

Writing `&` or `|` with two bools is an error that asks "did you mean 'and'?".

## Order of operations

When an expression has several operators, some happen before others, as in school math: `2 + 3 * 4` is `14`, because `*` happens first. From **first** to **last**:

| Order | Operators (symbol / word) |
|---|---|
| 1 (first) | `f()` calls, `a.b` fields, `a[i]` indexing |
| 2 | `-a` / `negative`, `!a` / `not` (written directly before a value), `~a` / `bitnot` |
| 3 | `*` `/` `%` (times, div, mod) |
| 4 | `+` `-` (plus, minus) |
| 5 | `<<` `>>` (shiftleft, shiftright) |
| 6 | `<` `>` `<=` `>=` (less, greater, ...), `is`, `as` |
| 7 | `==` `!=` (equals, not equals) |
| 8 | `&` (bitand) |
| 9 | `^` (bitxor) |
| 10 | `\|` (bitor) |
| 11 | `not` (written before a whole condition) |
| 12 | `&&` / `and` |
| 13 | `\|\|` / `or` |
| 14 | `??` / `otherwise` |
| 15 (last) | `? :` / `if ... else` |

Operators on the same row are worked out left to right: `10 - 3 - 2` is `5`.

**Parentheses always win.** If you're unsure, add them: `(a + b) * c`. One common trap: the bit operators come *after* `==`, so `x & 1 == 0` means `x & (1 == 0)`, which is an error. Write `(x & 1) == 0`.

## The word versions and names

Most word operators only count as operators when they sit **between two values**, which is a place where a plain name can never be. So you can still use these words as names:

```cpp
int times = 3;
int set = 2;
print(times times set);     // 6
```

The statement words (`set`, `increase`, `decrease`, `multiply`, `divide`) only count at the start of a statement that has the matching `to` or `by`, so a function called `set(...)` still works. The only words fully reserved are `negative` and `bitnot`, plus `and`, `or`, `not`, `is` and `as`. See [Reserved words](13-reserved-words.md).

## Operators for your own types

Your own structs and classes can use the same symbols and words as numbers. Give the type a method named after the operator's word:

| Method | Makes these work |
|---|---|
| `plus(other)` | `a + b`, `a plus b`, `a += b` |
| `minus(other)` | `a - b`, `a minus b`, `a -= b` |
| `times(other)` | `a * b`, `a times b`, `a *= b` |
| `div(other)` | `a / b`, `a div b`, `a /= b` |
| `mod(other)` | `a % b`, `a mod b`, `a %= b` |
| `bool less(other)` | all four comparisons: `<`, `>`, `<=`, `>=` (and their words), and `list.sort()` |
| `bool equals(other)` | `==` and `!=` (and `equals` / `not equals`), instead of the automatic comparison |
| `string toText()` | what `print(x)` and `"{x}"` show |

```cpp
struct Money {
    int cents;

    Money plus(Money other){
        return Money(cents + other.cents);
    }

    Money times(int n){             // the other side doesn't have to be the same type
        return Money(cents * n);
    }

    bool less(Money other){
        return cents < other.cents;
    }

    string toText(){
        return "${cents / 100}.{cents % 100}";
    }
}

Money a = Money(250);
Money b = Money(175);
print(a + b);          // $4.25
print(a times 2);      // $5.0
print(a > b);          // true: worked out as b.less(a)
total += a;            // total = total.plus(a)
prices.sort();         // a list<Money>, smallest first
```

Things to know:

- Your type has to be on the **left**: `a * 3` uses `a.times(3)`, but `3 * a` is an error.
- The method takes one value. `less` and `equals` have to give back a `bool`; the others can give back anything (usually the same type).
- With `less`, `a > b` is `b.less(a)`, `a <= b` is "not `b.less(a)`", and `a >= b` is "not `a.less(b)`". So one method gives all four.
- Without `equals`, `==` compares every field of a struct, and for classes it checks whether both sides are the **same object**. A struct's `equals` is also used by `contains`, `indexOf` and `remove` on its lists.
- These are normal methods too: `a.plus(b)` works.

## What's next

[Making decisions and repeating](05-control-flow.md): `if`, loops and `switch`.
