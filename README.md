# FURY OF DRACULA

An AI based Heuristic game based on graph algorithms, which is played by predicting the next move of hunter by dracula based on the previous set of hunter's visible moves. We use basic djikstra's and breadth first search for the purpose of this project with a slight variation of minimax algorithm to predit the next move.

![drac](https://user-images.githubusercontent.com/50875291/179397855-52892535-c31e-4bff-8073-1802540d10a4.png)

The game takes place in Europe during 1898.The game involves the arch-vampire Dracula, four intrepid vampire hunters and the splendid cities of Victorian Europe. The aim for Dracula is to evade the hunters for long enough to realise his Grand Plan - to release hordes of vampires all over Europe, while the aim for the hunters is to kill Dracula before he can do so.

![alt text](https://www.cse.unsw.edu.au/~cs2521/20T2/ass/ass2/Pics/map0.png)

## Design:

So, in order to abstract out the complexity and hide the implement we have created a views for each component in the game.
here is an example of the files.
```python
Game.h             # definitions of game constants; interface to game engine
GameView.h         # interface to the GameView ADT
GameView.c         # skeleton implementation of the GameView ADT
TestGameView.c     # main program containing a few tests for the GameView ADT
DraculaView.h      # interface to the DraculaView ADT
DraculaView.c      # skeleton implementation of the DraculaView ADT
TestDraculaView.c  # main program containing a few tests for the DraculaView ADT
HunterView.h       # interface to the HunterView ADT
HunterView.c       # skeleton implementation of the HunterView ADT
TestHunterView.c   # main program containing a few tests for the HunterView ADT
TestUtils.h        # interface to common test utilities
TestUtils.c        # implementation of common test utilities
Places.h           # interface to the Places ADT
Places.c           # implementation of the Places ADT
Map.h              # interface to the Map ADT
Map.c              # implementation of the Map ADT
```

## A view of the GamePlay:

![furry](https://user-images.githubusercontent.com/50875291/179211964-fbd4f00f-f487-4525-91ed-6fd4b52d958e.jpg)
