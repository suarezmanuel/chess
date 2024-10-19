# whats this?

a chess platform coded mainly in cpp and js, that heavily resembles chess.com, served to a web page via nodejs and ngrok, using the performance gains of compiling cpp into wasm.

# what can it do?

allows for online multiplayer, meaning you can play with other ppl online.

has an option to play against a chess bot.

# how do I play?

click the following image

[![chess](./public/resources/image.png)](https://suarezmanuel.github.io/chess/)

> can you solve the puzzle?

# the chess bot

the bot for now is able to see 0 moves ahead,

it makes use of the following optimizations:
1. none
2. none
3. none
4. none

heres the improvement of the bot overtime 

[![](./public/resources/comparison.png)]()


# misc commands

```
emcc ./public/cpp/main.cpp -o ./bin/temp.js   -std=c++17  -s MODULARIZE=1     -s ENVIRONMENT=node     -s EXPORTED_FUNCTIONS='["_change_board", "_setB
oardFromFen", "_malloc", "_free"]'     -s EXPORTED_RUNTIME_METHODS='["cwrap", "getValue", "setValue", "UTF8ToString"]'

node testcpp.js

g++ test.cpp -o test -lgtest -lgtest_main -pthread
```