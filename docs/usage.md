# Usage

## Build a game with Shooting Star

Shooting Star is a SO/DLL, therefore dynamic linking is required to use its functions. For example, if you use gcc as a linker, you can build it as shown below:

```sh
# Linux
gcc ./game.o -L. -lsstar -Wl,-rpath='$ORIGIN'
# Windows
gcc ./game.o ./sstar.dll
```

Of course, if the sstar.so/dll does not exist in the default linking path, this library must be located in the same directory as the game.

## Sequence Diagram

```mermaid
sequenceDiagram
    %%{init:{'theme':'forest'}}%%

    actor Player

    Player->>+Game: (run)
    Game->>SStar: (link)

    Note over Game: start program

    Game->>+SStar: ss_init
    Note right of SStar: create window and vulkan instance

    Game->>SStar: <load assets>

    Note over Game: start game

    loop every frame
        par game update
            break yes
                Game->>+SStar: ss_should_close
                SStar-->>-Game: <should close>
            end
            Game->>+SStar: ss_render
            SStar->>-Game: (wait for presentation)
        and player input
            Player->>SStar: (input)
        end
    end

    Note over Game: end game

    Game->>SStar: ss_terminate
    deactivate SStar
    Note right of SStar: delete window, vulkan and assets

    Note over Game: end program
    Game->>-Player: (exit)
```
