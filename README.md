# span_challenge_cpp #

### League Rank Calculator

This is a command-line application written in `C++` that will calculate the ranking table for a
soccer league.

### Requirements

- g++

### NOTE:
- Results of your soccer league must be in a file with the following structure.
- Please make sure the file ends with the last line of match outcomes, i.e. do not leave any empty lines in the file.

```
Lions 3, Snakes 3
Tarantulas 1, FC Awesome 0
Lions 1, FC Awesome 1
Tarantulas 3, Snakes 1
Lions 4, Grouches 0
```

- You need to know the absolute file path to copy and paste it into the terminal

#### NB:
- This program CANNOT handle tildes (`~`) at the start of the file path. 

### Run
A script has been provided that automatically compiles `main.cpp` and runs the output file.

All you have to do is run the following from the project `root` :

```
sh run.sh
```