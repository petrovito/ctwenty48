# Welcome
This repository is providing an implementation of an engine for the game [2048](https://en.m.wikipedia.org/wiki/2048_(video_game)). You can try the game here https://play2048.co.

<img src="https://elgoog.im/2048/2048-game.png"  
     alt="Markdown Monster icon"  width="400" height="400"
     style="float: left; margin-right: 10px;" />

This project is currently very much WIP, but it can already reach consistently 2048 (which is the goal of the game).

## Implementation
The implementation uses convolutional neural networks (tensorflow/keras) and iterative searching for gametree-node evaluation.

TODO extend this section

## GUI
This repo contains a GUI part, as well.

## Howto Build&Run
Currently building the apps requires Ubuntu 22.04.
Running the GUI app requires X11/xorg packages installed.

Other dependencies are pulled in from conan recipes, either defined here in the `./conan ` folder or from Conan Central. Therefore a conan installation is also required.

Try (when in doubt have a look at the provided github action):
```
conan create ./conan/tensorflow
conan create ./conan/cppflow
conan create ./conan/csv-parser
mkdir build && cd build
cmake .. -DBUILD_GUI=1
make -j6
```
Then run the binary `bin/ctwenty48_gui`. You might need to specify the folder containing the tensorflow libraries for the linker, they are most likely located here: `~/.conan/data/tensorflow/2.10/_/_/package/062863c92a5a0a247840166e9f84ebe8d10786b9/lib/`.

 

## TODOS
* Extend iterative searching to depend on intermittent node evaluations.
* Make GUI more userfriendly.
* Add Dockerfile, for running application.
* Extend CNN to encapsulate the geometry of the board. 

## CI/CD
Unit tests/Github actions results: ![Build status](https://github.com/petrovito/ctwenty48/actions/workflows/cmake.yml/badge.svg)

## Credit
Inspired by [Stockfish](https://github.com/official-stockfish/Stockfish) chess engine.

