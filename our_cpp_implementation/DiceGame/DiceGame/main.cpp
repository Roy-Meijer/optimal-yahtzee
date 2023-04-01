#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>

#define NR_OF_DICE 5
#define DICE_TYPE 6
#define NR_OF_SCORE_SECTIONS 6

void maxScore(double* scoreCard, double& maxValue, int& section) {
	maxValue = 0.0;
	section = 0;
	for (int i = 0; i < NR_OF_SCORE_SECTIONS; i++) {
		if (scoreCard[i] > maxValue) {
			maxValue = scoreCard[i];
			section = i;
		}
	}
}

int main() {
	//set random seed
	srand(time(0));

	int scoreCard[NR_OF_SCORE_SECTIONS];

	for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
		scoreCard[scoreSection] = -1;
	}

	bool scoreCardFilled = false;
	int diceThrow[NR_OF_DICE] = { 0 };

	while (!scoreCardFilled) {
		//intermediate results of score card
		double intermediateResults[NR_OF_SCORE_SECTIONS] = { 0.0 };

		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] != -1) {
				intermediateResults[scoreSection] = -1.0;
			}
		}

		std::cout << "Roll dice 1: ";

		//throw 3 dice once
		for (int dice = 0; dice < NR_OF_DICE; dice++) {
			//throw a die
			diceThrow[dice] = (rand() % DICE_TYPE) + 1;

			std::cout << diceThrow[dice] << " ";
			
			//calculate what would be the new score for ones, twos, threes, fours, fives, sixes
			for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
				if (scoreCard[scoreSection] == -1) {
					intermediateResults[scoreSection] += (double)((scoreSection + 1) * (diceThrow[dice] == scoreSection + 1));
				}
			}
		}

		std::cout << "\nPotential scores:\n";

		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] == -1) {
				std::cout << scoreSection+1 << "'s: " << intermediateResults[scoreSection] << '\n';
			}
		}

		//find out what gives the max expected score
		int maxExpectedScoreSection = 0;
		double maxExpectedScore = 0.0;
		maxScore(intermediateResults, maxExpectedScore, maxExpectedScoreSection);

		//which dice should we throw again?
		int diceThrowAgain[NR_OF_DICE] = { 0 };
		for (int diceCombinations = 0; diceCombinations < (1 << NR_OF_DICE); diceCombinations++) {
			double newScores[NR_OF_SCORE_SECTIONS] = { -1 };
			//what dice are thrown now?
			for (int dice = 0; dice < NR_OF_DICE; dice++) {
				//what scores are expected for each sectionbased on the dece which are thrown again?
				for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
					//if this dice will be thrown again
					if (diceCombinations & (1 << dice)) {
						//we throw the die again
						if (scoreCard[scoreSection] == -1) {
							newScores[scoreSection] += (double)(scoreSection + 1) * 1 / DICE_TYPE;
						}
					} else {
						//we keep the die
						if (scoreCard[scoreSection] == -1) {
							newScores[scoreSection] += (double)((scoreSection + 1) * (diceThrow[dice] == scoreSection + 1));
						}
					}
				}
			}
			//what is the maximum new score?
			int newMaxExpectedScoreSection = 0;
			double newMaxExpectedScore = 0.0;
			maxScore(newScores, newMaxExpectedScore, newMaxExpectedScoreSection);
			//if this new dice throw gives the best average result
			if (newMaxExpectedScore > maxExpectedScore) {
				//save it
				maxExpectedScore = newMaxExpectedScore;
				newMaxExpectedScoreSection = newMaxExpectedScoreSection;
				for (int dice = 0; dice < NR_OF_DICE; dice++) {
					//which dice throw combination gave this result?
					if (diceCombinations & (1 << dice)) {
						//we are going to throw this die again
						diceThrowAgain[dice] = 1;
					} else {
						//we are ging to keep this die
						diceThrowAgain[dice] = 0;
					}
				}

			}
		}

		//clear intermediate results;
		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] == -1) {
				intermediateResults[scoreSection] = 0.0;
			}
		}

		std::cout << "Roll dice 2: ";

		//throw dice second turn
		for (int dice = 0; dice < NR_OF_DICE; dice++) {
			//if we have to throw this die again
			if (diceThrowAgain[dice]) {
				//trow it
				diceThrow[dice] = (rand() % DICE_TYPE) + 1;
				std::cout << diceThrow[dice] << " ";
			} else {
				std::cout << 'k' << " ";
			}

			//calculate what would be the new score for ones, twos, threes, fours, fives, sixes
			for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
				if (scoreCard[scoreSection] == -1) {
					intermediateResults[scoreSection] += (double)((scoreSection + 1) * (diceThrow[dice] == scoreSection + 1));
				}
			}
		}

		std::cout << "\nPotential scores:\n";

		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] == -1) {
				std::cout << scoreSection+1 << "'s: " << intermediateResults[scoreSection] << '\n';
			}
		}

		//find out what gives the max expected score
		maxScore(intermediateResults, maxExpectedScore, maxExpectedScoreSection);

		//which dice should we throw again?
		for (int diceCombinations = 0; diceCombinations < (1 << NR_OF_DICE); diceCombinations++) {
			double newScores[NR_OF_SCORE_SECTIONS] = { 0 };
			//what dice are thrown now?
			for (int dice = 0; dice < NR_OF_DICE; dice++) {
				//what scores are expected for each sectionbased on the dece which are thrown again?
				for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
					//if this dice will be thrown again
					if (diceCombinations & (1 << dice)) {
						//we throw the die again
						if (scoreCard[scoreSection] == -1) {
							newScores[scoreSection] += (double)(scoreSection + 1) * 1 / DICE_TYPE;
						}
					} else {
						//we keep the die
						if (scoreCard[scoreSection] == -1) {
							newScores[scoreSection] += (double)((scoreSection + 1) * (diceThrow[dice] == scoreSection + 1));
						}
					}
				}
			}
			//what is the maximum new score?
			int newMaxExpectedScoreSection = 0;
			double newMaxExpectedScore = 0.0;
			maxScore(newScores, newMaxExpectedScore, newMaxExpectedScoreSection);
			//if this new dice throw gives the best average result
			if (newMaxExpectedScore > maxExpectedScore) {
				//save it
				maxExpectedScore = newMaxExpectedScore;
				newMaxExpectedScoreSection = newMaxExpectedScoreSection;
				for (int dice = 0; dice < NR_OF_DICE; dice++) {
					//which dice throw combination gave this result?
					if (diceCombinations & (1 << dice)) {
						//we are going to throw this die again
						diceThrowAgain[dice] = 1;
					}
					else {
						//we are ging to keep this die
						diceThrowAgain[dice] = 0;
					}
				}

			}
		}

		//clear intermediate results;
		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] == -1) {
				intermediateResults[scoreSection] = 0.0;
			}
		}

		std::cout << "Roll dice 3: ";

		//throw dice third turn
		for (int dice = 0; dice < NR_OF_DICE; dice++) {
			//if we have to throw this die again
			if (diceThrowAgain[dice]) {
				//trow it
				diceThrow[dice] = (rand() % DICE_TYPE) + 1;
				std::cout << diceThrow[dice] << " ";
			} else {
				std::cout << 'k' << " ";
			}

			//calculate what would be the new score for ones, twos, threes, fours, fives, sixes
			for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
				if (scoreCard[scoreSection] == -1) {
					intermediateResults[scoreSection] += (double)((scoreSection + 1) * (diceThrow[dice] == scoreSection + 1));
				}
			}
		}

		std::cout << "\nPotential scores:\n";

		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] == -1) {
				std::cout << scoreSection+1 << "'s: " << intermediateResults[scoreSection] << '\n';
			}
		}

		//find out what gives the max expected score
		maxScore(intermediateResults, maxExpectedScore, maxExpectedScoreSection);

		//fill in the achieved score
		scoreCard[maxExpectedScoreSection] = (int)(maxExpectedScore);

		std::cout << "Score " << (int)(maxExpectedScore) << " in " << maxExpectedScoreSection + 1 << "\n\n";

		bool weAreDone = true;
		for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
			if (scoreCard[scoreSection] == -1) {
				weAreDone = false;
			}
		}
		scoreCardFilled = weAreDone;
	}

	int scoreSum = 0;

	std::cout << "\nFinal scores:\n";

	for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) {
		std::cout << scoreSection + 1 << "'s: " << scoreCard[scoreSection] << '\n';
		scoreSum += scoreCard[scoreSection];
	}

	std::cout << "\nFinal score: " << scoreSum << '\n';
}