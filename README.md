# dolbear
Bear for GameCube and Wii

Bear is the first video game I have ever made. The game is based on a "Game Maker Kids" tutorial, and as such, the first iteration of Bear was a Game Maker game. Since then, Bear has become my "Hello World". I try to recreate Bear in every programming language I learn.

This iteration of Bear is written for the Nintendo GameCube and Wii. The game was made in one day for GameCube and ported to the Wii the day after. It is my first ever C project (and maybe it shows).

![GameCube title screen](https://user-images.githubusercontent.com/6349952/182355941-184c39a7-e930-4a76-9c0e-9b0b1599e8fa.png)
![Wii gameplay](https://user-images.githubusercontent.com/6349952/182356011-d936aa9f-203a-4ea4-ac2d-bc18391d2184.png)

## Controls

### GameCube

* START: start game/restart game
* Analog stick: move bear
* D-pad: move bear

### Wii

#### Wii Remote (sideways)

* PLUS: start game/restart game
* HOME: return to Homebrew Channel
* D-pad: move bear

#### Wii Remote + Nunchuk

* PLUS: start game/restart game
* HOME: return to Homebrew Channel
* Analog stick: move bear

#### Classic Controller

* PLUS: start game/restart game
* HOME: return to Homebrew Channel
* D-pad: move bear
* Left analog stick: move bear

#### GameCube Controller

* START: start game/restart game
* Analog stick: move bear
* D-pad: move bear

## Compiling

This project hinges on [devkitppc](https://github.com/devkitPro) and [GRRLIB](https://github.com/GRRLIB/GRRLIB). These libraries need to be installed in order to be able to compile Bear.

### Compiling for GameCube

In the dolbear directory:
```
make
```

### Compiling for Wii

In the dolbear directory:
```
make -f Makefile_Wii
```

If you switch between compiling for the GameCube and the Wii, I advise you to run `make clean` first. The compiler gets confused sometimes.