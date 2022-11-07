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
This repo contains a GUI app, as well. 

I am by no mean an UI developer, so esthetics was no priority when writing the code.

Focus was put on designing the app in a way where the UI does not get stuck while waiting on backend operations.

## Howto Build&Run
You can build and run the gui app by running `./build_n_run.sh`. It uses internally the build steps described in `Dockerfile`.

### Building the apps
Currently building the apps requires Ubuntu 22.04.

Running the GUI app requires X11/xorg packages installed (most of those are installed by conan).

Other dependencies are pulled in from conan recipes, either defined here in the `./conan` folder or from Conan Central. Therefore a conan installation is also required.

Try (when in doubt have a look at the provided github action/dockerfile):
```
sudo apt install libxft-dev
conan create ./conan/tensorflow
conan create ./conan/cppflow
conan create ./conan/csv-parser
conan create ./conan/nana
mkdir build && cd build
conan install ../cpp/gui_app --build=missing
conan install ../cpp/core --build=missing
cmake ../cpp -DBUILD_GUI=1
make -j6
```
Then run the binary `gui_app/bin/ctwenty48_gui`. 

## TODOS
* Extend iterative searching to depend on intermittent node evaluations.
* Make GUI more userfriendly.
* Add Dockerfile, for running application.
* Extend neural nets to encapsulate the geometry of the board. 
* Add docs

## CI/CD
Unit tests/Github actions results: ![Build status](https://github.com/petrovito/ctwenty48/actions/workflows/cmake.yml/badge.svg)

## Credit
Inspired by [Stockfish](https://github.com/official-stockfish/Stockfish) chess engine.

