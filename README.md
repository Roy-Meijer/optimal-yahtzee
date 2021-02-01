# Yahtzee Max Expected Score



This repo contains code to calculate max expected score for yatzee. I will test my code with this website: http://www-set.win.tue.nl/~wstomv/misc/yahtzee/osyp.php

compile: `bash compile.sh`
run: `./yahtzee.out`

example run:

```
enter in score for ones (or -1 for not filled yet).............. -1
enter in score for twos (or -1 for not filled yet).............. -1
enter in score for threes (or -1 for not filled yet)............ 9
enter in score for fours (or -1 for not filled yet)............. 12
enter in score for fives (or -1 for not filled yet)............. -1
enter in score for sixes (or -1 for not filled yet)............. 12
enter in score for three of a kind (or -1 for not filled yet)... 19
enter in score for four of a kind (or -1 for not filled yet).... -1
enter in score for full house (or -1 for not filled yet)........ 25
enter in score for small straight (or -1 for not filled yet).... -1
enter in score for large straight (or -1 for not filled yet).... 40
enter in score for yahtzee (or -1 for not filled yet)........... -1
enter in score for chance (or -1 for not filled yet)............ 24
enter in dice roll (ex: 2 4 6 3 2).............................. 5 6 1 1 4
number of re-rolls left (0, 1, or 2)............................ 1

Options are:
Keep die 5 _ _ _ _ and reroll giving 208.82924 expected points.
Keep die 5 _ _ _ 4 and reroll giving 208.82140 expected points.
Keep die _ _ 1 1 _ and reroll giving 208.81006 expected points.
Keep die 5 6 _ _ 4 and reroll giving 208.62430 expected points.
Keep die _ _ _ _ 4 and reroll giving 208.57608 expected points.
Keep die _ _ _ _ _ and reroll giving 208.54993 expected points.
Keep die _ _ 1 1 4 and reroll giving 208.54234 expected points.
Keep die _ _ 1 _ 4 and reroll giving 208.49311 expected points.
Keep die _ _ 1 _ _ and reroll giving 208.44879 expected points.
Keep die 5 _ 1 1 _ and reroll giving 208.38386 expected points.
Keep die _ 6 1 1 _ and reroll giving 208.28239 expected points.
Keep die 5 6 _ _ _ and reroll giving 208.21692 expected points.
Keep die 5 _ 1 _ _ and reroll giving 208.21439 expected points.
Keep die _ 6 1 1 4 and reroll giving 208.10691 expected points.
Keep die 5 6 1 1 _ and reroll giving 208.10691 expected points.
Keep die 5 _ 1 1 4 and reroll giving 208.10691 expected points.
Keep die _ 6 _ _ _ and reroll giving 208.07231 expected points.
Keep die 5 _ 1 _ 4 and reroll giving 208.07041 expected points.
Keep die 5 6 1 _ 4 and reroll giving 208.05340 expected points.
Keep die _ 6 1 _ _ and reroll giving 207.94523 expected points.
Keep die _ 6 1 _ 4 and reroll giving 207.94117 expected points.
Score roll as ones giving 207.94024 expected points.
Keep die _ 6 _ _ 4 and reroll giving 207.93483 expected points.
Keep die 5 6 1 _ _ and reroll giving 207.75491 expected points.
Score roll as twos giving 204.25783 expected points.
Score roll as yahtzee giving 202.96577 expected points.
Score roll as fives giving 201.59302 expected points.
Score roll as four of a kind giving 198.13692 expected points.
Score roll as small straight giving 182.61921 expected points.
```
