# C_Pacman

this is a pacman simplified made in C. <b>The game is only in french for now</b>.

##installation

run `gcc server.c -o server` to compile the server 
and `gcc player.c -o player` to compile the player client.


##the clients

###server

The server has to be running first (on local), you launch it by typing `./server N` 
in a terminal, with `N` being the number of clients able to connect at once.
you can use the `--debug` or `-d` to have the server display all the moves being made by the player
and by the computer.

You can do `./server --help` to display the help to describe the use of the server
or just `./server` (this command has to at least have the number 
of client or it defaults to an error and displays the help).

###player

the player has to be run in a different terminal than the server. Just do `./player`,
this program doesn't have any option.

##Playing and rules

###init of the player

When launching the game you'll be prompted to select the game mode, there are three : 

`1`: easy ------> grid has a size of 10x6, and there are 2 ghosts chasing you, and a total of 30 points to collect.

`2`: medium --> grid has a size of 13x9, there are 4 ghosts, and a total of 50 points.

`3`: hard -------> grid has a size of 16x12, there are 6 ghosts, and a total of 70 points.

Depending on the difficulty, you either have 2, 3 or 4 minutes to collect all the points.

###game 

To move around the grid you have to use the WASD/ZQSD keys (support for Qwerty and Azerty layouts).

If you get one of the edges of the grid and try to move towards it, you won't move.

The player is displayed, in a default terminal, in yellow by a `C`, and the ghosts are in red by a `A`. 

(if you changed the color palette of your terminal the color will differ)


###end of game

<br>
If you get eaten by a ghosts before collecting all the points, you lose the game, and your points are displayed.

If you finish the game before the end of the timer, you get a time bonus depending on how much time you have left on the timer.


# made by Night4dead / Quentin KACZMAREK

