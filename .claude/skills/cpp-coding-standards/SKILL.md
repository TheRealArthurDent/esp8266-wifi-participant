---
name: cpp-coding-standards
description: Use when writing, reviewing, or refactoring C++ code in esp8266-wifi-participant - firmware for the ESP8266 Arduino core (GCC 10.3, gnu++17, no exceptions, ~80 KB RAM) and its host-side unit tests.
metadata:
  origin: ECC, adapted to ESP8266/Arduino
---

# C++ Coding Standards (ESP8266 / Arduino)

The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines),
cut down to what applies to this firmware and adapted where the target
forces it. Rule IDs (`R.11`, `ES.20`, …) refer to the guidelines; rules
marked **ESP** are additions for this platform.

## The Target

Everything here follows from these facts. Check them again after a platform
bump (`pio pkg list -e nodemcuv2`).

| Fact | Consequence |
| --- | --- |
| xtensa GCC 10.3, `-std=gnu++17` | No C++20: no concepts, `requires`, ranges, `std::span`, designated initializers |
| `-fno-exceptions` | No `throw`/`try`/`catch`; a failed plain `new` panics ("OOM") |
| ~80 KB RAM, heap fragments | Allocate once in `setup()`, never in `loop()` |
| `.rodata` lives in RAM | Large constant tables and strings go to flash (`PROGMEM`, `F()`) |
| One thread, plus ISRs and SDK callbacks | No `std::thread`/`std::mutex`; ISR data needs `volatile` and interrupt locks |
| Software watchdog, WiFi stack runs between `loop()` calls | Never block for long; `delay()`/`yield()` in waiting loops |

The unit tests build the same sources on the host (`native`), so code must
compile with both toolchains. Keep it to standard C++17 plus the Arduino
API.

## Cross-Cutting Principles

1. **RAII everywhere** (P.8, R.1): bind every acquired thing (an interrupt
   lock, a pin state, a registration) to an object's lifetime.
2. **Immutability by default** (P.10, Con.1-5, ES.25): start with
   `const`/`constexpr`; mutability is the exception.
3. **Type safety** (P.4, I.4, ES.46-49, Enum.3): use the type system to catch
   errors at compile time.
4. **Static over dynamic** (R.5, **ESP**): objects live in static storage or
   on the stack; the heap is for setup-time allocations only.
5. **Never block the loop** (**ESP**): every wait yields to the SDK.
6. **Keep it simple** (F.2-3, ES.5, Per.4-5): simple code is correct code.

## Philosophy & Interfaces (P.\*, I.\*)

| Rule | Summary |
| --- | --- |
| **P.1** | Express ideas directly in code |
| **P.3** | Express intent |
| **P.5** | Prefer compile-time checking to run-time checking |
| **P.8** | Don't leak any resources |
| **P.10** | Prefer immutable data to mutable data |
| **I.2** | Avoid non-const global variables |
| **I.4** | Make interfaces precisely and strongly typed |
| **I.11** | Never transfer ownership by a raw pointer or reference |
| **I.23** | Keep the number of function arguments low |

```cpp
// I.4: the unit is part of the type, not a comment
struct milliseconds
{
  unsigned long value;
};

auto blink(uint8_t pin, milliseconds period) -> void;

// DON'T: which unit? which argument is which?
void blink(int a, int b);
```

`setup()`/`loop()` force some globals. Keep them in `main.cpp`, give them
static storage duration, and pass references into the classes that need
them (I.2, I.3).

## Functions (F.\*)

| Rule | Summary |
| --- | --- |
| **F.1** | Package meaningful operations as carefully named functions |
| **F.2** | A function should perform a single logical operation |
| **F.3** | Keep functions short and simple |
| **F.4** | If a function might be evaluated at compile time, declare it `constexpr` |
| **F.6** | If your function must not fail, declare it `noexcept` |
| **F.8** | Prefer pure functions |
| **F.16** | For "in" parameters, pass cheaply-copied types by value and others by `const&` |
| **F.20** | For "out" values, prefer return values to output parameters |
| **F.21** | To return multiple "out" values, prefer returning a struct |
| **F.43** | Never return a pointer or reference to a local object |

```cpp
// F.16: cheap types by value, others by const&
auto set_level(uint8_t level) -> void;
auto log_line(std::string_view text) -> void;   // non-owning view, no copy

// F.20 + F.21: return a struct, not output parameters
struct reading
{
  float value;
  bool valid;
};
auto read_sensor() -> reading;

// F.4 + F.8: pure and constexpr, checked at compile time
constexpr auto blink_count(unsigned long period_ms) noexcept -> unsigned
{
  return period_ms == 0 ? 0 : 1000 / period_ms;
}
static_assert(blink_count(250) == 4, "4 blinks per second");
```

The repo uses trailing return types (`auto f() -> void`), as
`modernize-use-trailing-return-type` asks.

Anti-patterns: returning `T&&` (F.45), C-style variadics (F.55; `printf`
from the Arduino core is the exception), returning `const T` (F.49).

## Classes & Class Hierarchies (C.\*)

| Rule | Summary |
| --- | --- |
| **C.2** | Use `class` if an invariant exists; `struct` if members vary independently |
| **C.9** | Minimize exposure of members |
| **C.20** | If you can avoid defining default operations, do (Rule of Zero) |
| **C.21** | If you define or `=delete` any copy/move/destructor, handle them all (Rule of Five) |
| **C.35** | Base class destructor: public virtual or protected non-virtual |
| **C.41** | A constructor should create a fully initialized object |
| **C.46** | Declare single-argument constructors `explicit` |
| **C.67** | A polymorphic class should suppress public copy/move |
| **C.128** | Virtual functions: specify exactly one of `virtual`, `override`, or `final` |

Most classes here own hardware or are registered somewhere, so they must
not be copied. Delete all five, as `metronome.hpp` does:

```cpp
class led_blinker
{
public:
  explicit led_blinker(uint8_t pin) : pin(pin) {}

  led_blinker(const led_blinker &) = delete;
  led_blinker(led_blinker &&) = delete;
  auto operator=(const led_blinker &) -> led_blinker & = delete;
  auto operator=(led_blinker &&) -> led_blinker & = delete;
  ~led_blinker() = default;

private:
  uint8_t pin;
  bool on = false;   // C.48: default member initializers
};
```

Interfaces get a virtual destructor and no copies (C.35, C.67, C.121):

```cpp
class wifi_dependent
{
public:
  virtual ~wifi_dependent() = default;
  virtual auto on_connected() -> void = 0;
  virtual auto on_disconnected() -> void = 0;

protected:
  wifi_dependent() = default;
  wifi_dependent(const wifi_dependent &) = default;
  wifi_dependent(wifi_dependent &&) = default;
  auto operator=(const wifi_dependent &) -> wifi_dependent & = default;
  auto operator=(wifi_dependent &&) -> wifi_dependent & = default;
};
```

Anti-patterns: virtual calls in constructors/destructors (C.82),
`memset`/`memcpy` on non-trivial types (C.90), different default arguments
in a virtual function and its override (C.140).

## Resources, Memory & Lifetimes (R.\*, ESP)

| Rule | Summary |
| --- | --- |
| **R.1** | Manage resources automatically using RAII |
| **R.3** | A raw pointer (`T*`) is non-owning |
| **R.5** | Prefer scoped objects; don't heap-allocate unnecessarily |
| **R.10** | Avoid `malloc()`/`free()` |
| **R.11** | Avoid calling `new` and `delete` explicitly |
| **R.20** | Use `unique_ptr` to represent ownership |
| **ESP** | No heap allocation in `loop()` or in callbacks |
| **ESP** | Anything handed to a callback or registry outlives it |
| **ESP** | No `shared_ptr` (heap + reference count for no gain here) |

```cpp
// GOOD: static storage, lives as long as the program
dummy_wifi_dependent dependent_a{"A"};
dummy_wifi_dependent dependent_b{"B"};

auto setup() -> void
{
  wifi_connection::init(&dependent_a, &dependent_b);
}

// BAD: the objects die when setup() returns, but wifi_connection keeps the
// pointers (the known bug in main.cpp, see CLAUDE.md)
auto setup() -> void
{
  dummy_wifi_dependent dependent_a{"A"};
  wifi_connection::init(&dependent_a);
}
```

If something must be allocated at run time, do it once in `setup()`, own it
with `std::unique_ptr`, and handle failure: a plain `new` panics with "OOM"
when the heap is exhausted, `new (std::nothrow)` returns `nullptr`.

RAII for the platform's own resources, e.g. the core's interrupt lock:

```cpp
#include <interrupts.h>

auto read_counter() -> uint32_t
{
  esp8266::InterruptLock lock;   // disables interrupts, restores on exit
  return isr_counter;            // a multi-byte read the ISR can't tear
}
```

## Interrupts, Callbacks & the Loop (ESP, replaces CP.\*)

There are no threads, so the concurrency rules (CP.\*) don't apply. What
runs "concurrently" is interrupt handlers and SDK callbacks (WiFi events).

- ISRs are `IRAM_ATTR`, short, and only record what happened: set a
  `volatile` flag or counter, nothing else. No `Serial`, no `delay()`, no
  allocation.
- Data shared between an ISR and `loop()` is `volatile`. Read anything
  wider than 32 bits, or more than one variable, under
  `esp8266::InterruptLock`.
- WiFi event callbacks run in the SDK's context: record the event and
  handle it in `loop()`.
- Never block for long. Waiting loops call `delay()` or `yield()` so the
  WiFi stack runs and the watchdog is fed; prefer `millis()`-based state
  machines (like `metronome`) over long `delay()`s.
- Use `millis()` differences, not absolute values, so the 49-day wraparound
  doesn't matter: `if (millis() - start >= period)`.

```cpp
volatile bool button_pressed = false;

IRAM_ATTR auto on_button() -> void
{
  button_pressed = true;
}

auto loop() -> void
{
  if (button_pressed)
  {
    button_pressed = false;
    handle_button();
  }
}
```

## Expressions & Statements (ES.\*)

| Rule | Summary |
| --- | --- |
| **ES.5** | Keep scopes small |
| **ES.20** | Always initialize an object |
| **ES.23** | Prefer `{}` initializer syntax |
| **ES.25** | Declare objects `const` or `constexpr` unless modification is intended |
| **ES.28** | Use lambdas for complex initialization of `const` variables |
| **ES.45** | Avoid magic constants; use symbolic constants |
| **ES.46** | Avoid narrowing/lossy arithmetic conversions |
| **ES.47** | Use `nullptr` rather than `0` or `NULL` |
| **ES.48** | Avoid casts |
| **ES.50** | Don't cast away `const` |

```cpp
constexpr unsigned long blink_period_ms{500};   // ES.45 instead of 500
constexpr uint8_t status_led{LED_BUILTIN};
const auto start = millis();                    // ES.25
```

Anti-patterns: uninitialized variables (ES.20), C-style casts (ES.48; use
`static_cast`), mixing signed and unsigned arithmetic (ES.100), reusing names
in nested scopes (ES.12). Integer sizes matter here: `int` is 32 bits on the
ESP8266 but may differ in host tests; use `<cstdint>` types where the width
matters.

## Error Handling (ESP, replaces E.\*)

Exceptions are disabled, so E.2/E.14/E.15 don't apply. Instead:

- Report failure through the return type: `bool`, an `enum class` status, or
  `std::optional<T>` (C++17).
- Mark results that must not be ignored `[[nodiscard]]`.
- Log with `DEBUG_PRINT`/`DEBUG_PRINTLN` (`debug.h`); they compile to nothing
  in the release build, so never put side effects in their arguments.
- Decide early what the device does on failure (retry, show it on the LED,
  restart) (E.1), and keep that decision in one place.
- Destructors and `swap` must not fail (E.16), and `noexcept` still
  documents that a function cannot fail (E.12).

```cpp
enum class connect_result
{
  connected,
  timed_out,
  wrong_password
};

[[nodiscard]] auto connect(std::string_view ssid) -> connect_result;
```

## Constants & Immutability (Con.\*)

| Rule | Summary |
| --- | --- |
| **Con.1** | By default, make objects immutable |
| **Con.2** | By default, make member functions `const` |
| **Con.3** | By default, pass pointers and references to `const` |
| **Con.4** | Use `const` for values that don't change after construction |
| **Con.5** | Use `constexpr` for values computable at compile time |

```cpp
class sensor
{
public:
  explicit sensor(uint8_t pin) : pin(pin) {}

  [[nodiscard]] auto last_reading() const -> float { return reading; }  // Con.2
  auto record(float value) -> void { reading = value; }

private:
  uint8_t pin;          // not const: that would disable assignment (C.12)
  float reading{0.0F};
};

constexpr float volts_per_step = 3.3F / 1024;   // Con.5, lower_snake_case
```

## Templates (T.\*)

Concepts are C++20 and unavailable. Constrain with `static_assert` and type
traits instead, and keep templates rare: every instantiation costs flash.

| Rule | Summary |
| --- | --- |
| **T.1** | Use templates to raise the level of abstraction |
| **T.43** | Prefer `using` over `typedef` |
| **T.120** | Use template metaprogramming only when you really need to |
| **T.144** | Don't specialize function templates (overload instead) |

```cpp
#include <type_traits>

template <typename T>
constexpr auto clamp_to_byte(T value) -> uint8_t
{
  static_assert(std::is_integral_v<T>, "integers only");
  return value < 0 ? 0 : (value > 255 ? 255 : static_cast<uint8_t>(value));
}
```

## Standard Library & Strings (SL.\*, ESP)

| Rule | Summary |
| --- | --- |
| **SL.con.1** | Prefer `std::array` over C arrays |
| **SL.str.2** | Use `std::string_view` to refer to character sequences |
| **ESP** | Prefer fixed-size containers; `std::vector`/`std::string`/`String` only when sized once at setup |
| **ESP** | String literals for `Serial` go through `F()` so they stay in flash |
| **ESP** | Use `Serial`/`DEBUG_PRINT`, never `<iostream>` (pulls in a lot of flash) |

```cpp
std::array<wifi_dependent *, 2> dependents{&dependent_a, &dependent_b};

Serial.println(F("WiFi connected"));   // literal stays in flash

// BAD in loop(): allocates and fragments the heap on every call
String message = "RSSI: " + String(WiFi.RSSI());
```

## Enumerations (Enum.\*)

| Rule | Summary |
| --- | --- |
| **Enum.1** | Prefer enumerations over macros |
| **Enum.3** | Prefer `enum class` over plain `enum` |
| **Enum.5** | Don't use ALL_CAPS for enumerators |
| **Enum.6** | Avoid unnamed enumerations |

```cpp
enum class led_state : uint8_t { off, on, blinking };   // fixed size: 1 byte

// BAD: leaks names, ALL_CAPS clashes with macros (the core defines many)
enum { OFF, ON };
#define BLINK_MS 500   // Enum.1 / ES.31: use constexpr
```

## Files & Naming (SF.\*, NL.\*)

| Rule | Summary |
| --- | --- |
| **SF.7** | Don't write `using namespace` at global scope in a header |
| **SF.8** | Every header is guarded: here with `#pragma once` |
| **SF.11** | Header files should be self-contained |
| **NL.5** | No Hungarian notation |
| **NL.8** | Use a consistent naming style |
| **NL.9** | ALL_CAPS for macro names only |
| **NL.10** | Prefer `lower_snake_case` names |

Repo conventions:

- Headers in `include/`: `.hpp` for classes, `.h` for C-style helpers
  (`debug.h`, `led.h`). Sources in `src/`. File names in kebab-case
  (`wifi-connection.hpp`).
- No subfolders in `src/`/`include/` and no duplicate file names:
  `scripts/migrate_to_ino.sh` flattens them into one Arduino IDE sketch.
- Types, functions, variables, members and enumerators are
  `lower_snake_case` (NL.10), like the standard library. Members have no
  prefix or suffix. Macros, and only macros, are `ALL_CAPS`.
- Names from the Arduino core, the SDK and test libraries (`digitalWrite`,
  `WiFi.begin`, Unity's `setUp`/`tearDown`) stay as they are.
- `.clang-tidy` enforces this with `readability-identifier-naming`.
- Public classes and methods get Doxygen comments (`\brief`, `\param`).

## Performance & Footprint (Per.\*, ESP)

| Rule | Summary |
| --- | --- |
| **Per.1** | Don't optimize without reason |
| **Per.6** | Don't make claims about performance without measurements |
| **Per.11** | Move computation from run time to compile time |
| **ESP** | Watch the RAM/Flash summary of `pio run` when adding code |
| **ESP** | Large constant tables go to flash with `PROGMEM` |

Constant arrays and tables, `constexpr` ones included, still occupy RAM on
the ESP8266, because `.rodata` is mapped to RAM. Only `PROGMEM` keeps them
in flash, read with `pgm_read_*`:

```cpp
#include <pgmspace.h>

const uint8_t gamma_table[256] PROGMEM = {/* ... */};

auto apply_gamma(uint8_t level) -> uint8_t
{
  return pgm_read_byte(&gamma_table[level]);
}
```

## Quick Reference Checklist

Before marking C++ work complete:

- [ ] Compiles with `-std=gnu++17` on GCC 10.3: no C++20 features
- [ ] No `throw`/`try`/`catch`; failure goes through return types
- [ ] No heap allocation in `loop()`, ISRs or callbacks; no `shared_ptr`
- [ ] Everything handed to a callback or registry outlives it
- [ ] No long blocking waits; waiting loops `delay()` or `yield()`
- [ ] ISRs are `IRAM_ATTR`, short, and only touch `volatile` data
- [ ] `F()` for `Serial` literals, `PROGMEM` for large tables
- [ ] Objects initialized at declaration (ES.20), `const`/`constexpr` by
      default (Con.1, ES.25), `const` member functions (Con.2)
- [ ] Single-argument constructors are `explicit` (C.46)
- [ ] Rule of Zero, or all five special members handled (C.20, C.21)
- [ ] Base class destructors public virtual or protected non-virtual (C.35)
- [ ] `enum class`, `nullptr`, no C-style casts, no magic numbers
- [ ] `lower_snake_case` names, `ALL_CAPS` only for macros
- [ ] Headers have `#pragma once`, are self-contained and Doxygen-documented
