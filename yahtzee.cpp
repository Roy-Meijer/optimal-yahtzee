

#pragma GCC optimize("Ofast")
#pragma GCC optimize("unroll-loops")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx")

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <cassert>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace std::chrono;

//TODO: handle multiple yahtzee's
//TODO: handle +35 points if score of first part is >= 63
enum scores {
	//top part
	ones,
	twos,
	threes,
	fours,
	fives,
	sixes,

	//bottom part
	threeOfAKind,
	fourOfAKind,
	fullHouse,
	smallStraight,
	largeStraight,
	yahtzee,
	chance
};

int scoreForRoll[252][13];

vector<vector<int>> allRollsIndistinguishable;
vector<vector<int>> allRollsDistinguishable;

//scoreForRoll[i][j] = the score for roll with rollID=i if it counds for
//scores::x s.t. scores::x==j
void calculateScores() {
	//calculate score for each section for all 252 possible dice rolls
	for(int rollId = 0; rollId < (int)allRollsIndistinguishable.size(); ++rollId) {
		//take the values of the 5 dice which are rolled and store in array called "roll"
		const vector<int> &roll = allRollsIndistinguishable[rollId];
		//make a vector of size 7, initialized to 0
		vector<int> cnts(7,0);
		int mxCnt = 0;
		int sum = 0;
		//loop over the 5 rolls from "roll"
		for(int val : roll) {
			//add values from roll
			//this sum is not used for the upper section, it will be used below
			sum += val;
			//count number of 1s, 2s, 3s, 4s, 5s and 6s
			++cnts[val];
			//find out which die appears the most
			mxCnt = max(mxCnt, cnts[val]);
			//calculate the score for each upper section for a specific roll
			scoreForRoll[rollId][scores::ones] += (val == 1);
			scoreForRoll[rollId][scores::twos] += 2 * (val == 2);
			scoreForRoll[rollId][scores::threes] += 3 * (val == 3);
			scoreForRoll[rollId][scores::fours] += 4 * (val == 4);
			scoreForRoll[rollId][scores::fives] += 5 * (val == 5);
			scoreForRoll[rollId][scores::sixes] += 6 * (val == 6);
		}
		//add the sum of the 5 dice to chance
		scoreForRoll[rollId][scores::chance] = sum;
		//add the scode of 3 of a kind if applicable
		if(mxCnt >= 3) {
			scoreForRoll[rollId][scores::threeOfAKind] = sum;
		}
		//add the scode of 4 of a kind if applicable
		if(mxCnt >= 4) {
			scoreForRoll[rollId][scores::fourOfAKind] = sum;
		}
		//add the scode of yahtzee if applicable
		if(mxCnt >= 5) {
			scoreForRoll[rollId][scores::yahtzee] = 50;
		}
		
		//check if one dice number appears twice, and another appears trice
		bool seenCnt3 = false, seenCnt2 = false;
		for(int cnt : cnts) {
			seenCnt2 |= (cnt == 2);
			seenCnt3 |= (cnt == 3);
		}
		//add score for full house if applicable
		if(seenCnt2 && seenCnt3) {
			scoreForRoll[rollId][scores::fullHouse] = 25;
		}
		//check small and large straights
		for(int straightLen = 4; straightLen <= 5; ++straightLen) {
			//check start value 1 for large straight and start values 1 and 2 for small straight
			for(int startVal = 1; startVal + straightLen - 1 <= 6; ++startVal) {
				bool hasAll = true;
				//check if every die for the straight is there
				for(int val = startVal, len = straightLen; len--; ++val) {
					if(cnts[val] == 0) {
						hasAll = false;
						break;
					}
				}
				//if there is a straight
				if(hasAll) {
					//if it is a small one, we can get 30 points
					scoreForRoll[rollId][scores::smallStraight] = 30;
					if(straightLen == 5) {
						//if it is a large one, we can get 40 points
						scoreForRoll[rollId][scores::largeStraight] = 40;
					}
				}
			}
		}
	}
}

int getRollId(vector<int> roll) {
	//make sure there is 5 dice rolls
	assert(roll.size() == 5);
	//make sure all dice rolls are between 1 and 6
	for(int i = 0; i < (int)roll.size(); ++i) {
		assert(1 <= roll[i] && roll[i] <= 6);
	}
	//sort dice rolls
	sort(roll.begin(), roll.end());
	//get pointer to first value in allRollsIndistinguishable
	const auto it = lower_bound(allRollsIndistinguishable.begin(), allRollsIndistinguishable.end(), roll);
	//make sure this is the same value of roll
	assert(*it == roll);
	//return roll id
	return lower_bound(allRollsIndistinguishable.begin(), allRollsIndistinguishable.end(), roll) - allRollsIndistinguishable.begin();
}

int numberOfRoll[252];
int pow6[7];

void initAllRolls() {
	//calculate 0^6, 1^6, ..., 6^6
	//why 6^6? there are only 5 dice?
	pow6[0] = 1;
	for(int i = 1; i < 7; ++i) {
		pow6[i] = 6 * pow6[i-1];
	}
	//loop values 0, ..., 5^6
	for(int rollVal = 0; rollVal < pow6[5]; ++rollVal) {
		//temp = i
		int temp = rollVal;
		//make integer vector of size 5
		vector<int> roll(5);
		//calculate 5 dice rolls: [1, 1, 1, 1, 1], [1, 1, 1, 1, 2], ..., [6, 6, 6, 6, 6]
		for(int i = 0; i < 5; ++i) {
			roll[i] = temp%6 + 1;
			temp /= 6;
		}
		//add rolls to this vector
		allRollsDistinguishable.push_back(roll);
		//sort rolls so e.g. [1, 2, 1, 1, 1] == [1, 1, 1, 1, 2]
		sort(roll.begin(), roll.end());
		//add sorted rolls to vector (there will be many duplicated)
		allRollsIndistinguishable.push_back(roll);
	}
	sort(allRollsIndistinguishable.begin(), allRollsIndistinguishable.end());
	//remove duplicates from vector
	allRollsIndistinguishable.erase(unique(allRollsIndistinguishable.begin(), allRollsIndistinguishable.end()), allRollsIndistinguishable.end());
	//check if there are 252 different dice rolls (which is expected)
	assert(allRollsIndistinguishable.size() == 252);
	//assign index of a specific roll in allRollsDistinguishable to numberOfRoll
	for(const auto &roll : allRollsDistinguishable) {
		++numberOfRoll[getRollId(roll)];
	}
}

//maxEV[subset scores filled][num rerolls][roll] = max expected score
double maxEV[1<<13][3][252];
double averageMaxEV[1<<13];
vector<pair<int,int>> distinctSubsetsForReroll[252];
vector<pair<int,int>> cntReroll[252][1<<5];
int tempCnt[252];
vector<int> tempRolls[7776];

struct Move {
	int subsetReroll, scoreTaken;
	double evForMove;
};

bool operator<(const Move &x, const Move &y) {
	return x.evForMove > y.evForMove;
}

vector<Move> transitions[1<<13][3][252];

vector<pair<int,int>> rollToSubsetKeptCnts[252];

void calcExpectedValue() {
	cout << "Calculating expected values, should take 3-4 seconds... " << flush; //more like 40 seconds
	//measure start time
	auto start = high_resolution_clock::now();
	//make 2d vector for "allDieKept"
	vector<vector<int>> allDieKept;
	//loop over all combinations of 2 dice rolls
	for(int i = 0; i < 36; ++i) {
		//take two dice rolls
		int die1 = allRollsDistinguishable[i][0];
		int die2 = allRollsDistinguishable[i][1];
		//sort the two dice from small to large
		if(die1 > die2) swap(die1, die2);
		//put empty in vector
		allDieKept.push_back({});
		//put first die in vector
		allDieKept.push_back({die1});
		//put first and second dice in vector
		allDieKept.push_back({die1, die2});
	}
	//sort the vector
	sort(allDieKept.begin(), allDieKept.end());
	//remove duplictates
	allDieKept.erase(unique(allDieKept.begin(), allDieKept.end()), allDieKept.end());
	//make sure there are 28 possible combinations when duplicates are removed
	assert(allDieKept.size() == 28);

	//i don't know why this { thing is here
	{
		//make 2d vector of rerollCnts with 28 slots initialized to a vector of 252 times 0
		vector<vector<int>> rerollCnts(28, vector<int>(252,0));
		//loop from 0 to 28
		for(int dieKeptID = 0; dieKeptID < (int)allDieKept.size(); ++dieKeptID) {
			//take the number of dice which can be kept?
			const auto &dieKept = allDieKept[dieKeptID];
			//we can roll again 5, 4, or 3 dice
			const int numReroll = 5 - (int)dieKept.size();
			//6^5, 6^4 or 6^3 combinations
			const int iters = pow6[numReroll];
			//loop from 0 to 6^5, 6^4 or 6^3
			for(int id = 0; id < iters; ++id) {
				vector<int> newRoll = dieKept;
				//roll 5, 4, or 3 dice again
				for(int i = 0; i < numReroll; ++i) {
					//store the new roll values
					newRoll.push_back(allRollsDistinguishable[id][i]);
				}
				//make sure 5 dice in total are rolled
				assert(newRoll.size() == 5);
				//sort the rolled dice
				sort(newRoll.begin(), newRoll.end());
				//count number of dice which are rolled again
				++rerollCnts[dieKeptID][getRollId(newRoll)];
			}
		}
		//check all 252 possible dice combinations
		for(int rollID = 0; rollID < 252; ++rollID) {
			//for all dice which are not rolled again...
			for(int dieKeptID = 0; dieKeptID < (int)allDieKept.size(); ++dieKeptID) {
				if(rerollCnts[dieKeptID][rollID] > 0) {
					//add number of times it is rolled again to list
					rollToSubsetKeptCnts[rollID].push_back({dieKeptID, rerollCnts[dieKeptID][rollID]});
				}
			}
		}
	}

	//loop from 0 to 252
	for(int roll = 0; roll < (int)allRollsIndistinguishable.size(); ++roll) {
		map<vector<int>,pair<int,int>> keptDieToSubset;
		//there are 5 dice, each die can be kept or be rolled again. therefore loop from 0 to 2^5
		for(int subsetRerolled = 1; subsetRerolled < (1<<5); ++subsetRerolled) {
			vector<int> keptDie;
			//loop over 5 dice
			for(int die = 0; die < 5; ++die) {
				//if this specific die has not been rolled again...
				if((subsetRerolled&(1<<die)) == 0) {
					//add die to list of dice which are not rolled again
					keptDie.push_back(allRollsIndistinguishable[roll][die]);
				}
			}
			//sort list of dice which are not rolled again
			sort(keptDie.begin(), keptDie.end());
			//assign the roll and id of the dice which are kept and put it in a list
			keptDieToSubset[keptDie] = {subsetRerolled, lower_bound(allDieKept.begin(), allDieKept.end(), keptDie) - allDieKept.begin()};
		}
		//loop over all dice which are kept
		for(auto &p : keptDieToSubset) {
			//put the id of the dice which are kept in the list
			distinctSubsetsForReroll[roll].push_back(p.second);
		}
	}

	//loop from 0 to 252
	for(int roll = 0; roll < (int)allRollsIndistinguishable.size(); ++roll) {
		for(auto [subsetRerolled,keptDieID] : distinctSubsetsForReroll[roll]) {
			//__builtin_popcount counts the number of 1's in a number
			//so iters = number of dice rolled ^ 6
			const int iters = pow6[__builtin_popcount(subsetRerolled)];
			int sz = 0;
			map<vector<int>, int> cnts;
			for(int id = 0; id < iters; ++id) {
				//take a roll
				vector<int> newRoll = allRollsIndistinguishable[roll];
				int ptr = 0;
				for(int die = 0; die < 5; ++die) {
					//if this specific die has been rolled again...
					if(subsetRerolled&(1<<die)) {
						//take next dice roll and store it
						newRoll[die] = allRollsDistinguishable[id][ptr++];
					}
				}
				//here, we have a triplet: (start roll, subset die re-rolled, end roll)
				//sort dice rolls
				sort(newRoll.begin(), newRoll.end());
				//???
				++cnts[newRoll];
				//store roll in array and increase size
				tempRolls[sz++] = newRoll;
			}
			//sort roll array
			sort(tempRolls, tempRolls + sz);
			//initialize tempCnt to 0
			for(int endRoll = 0; endRoll < 252; ++endRoll) {
				tempCnt[endRoll] = 0;
			}
			int ptr = 0;
			for(int i = 0; i < sz; ++i) {
				//search for a speficic roll which is the same as tempRolls
				while(ptr < (int)allRollsIndistinguishable.size() && allRollsIndistinguishable[ptr] < tempRolls[i]) ++ptr;
				assert(tempRolls[i] == allRollsIndistinguishable[ptr]);
				++tempCnt[ptr];
			}
			for(int endRoll = 0; endRoll < 252; ++endRoll) {
				if(tempCnt[endRoll] > 0) {
					cntReroll[roll][subsetRerolled].push_back({tempCnt[endRoll], endRoll});
				}
			}
		}
	}

	//fill all categories in every order with evert possible roll
	for(int subsetFilled = 1; subsetFilled < (1<<13); ++subsetFilled) {
		vector<double> sumOfDpValsForSubsetKept(28,0.0);
		//roll 1, 2, or 3 times
		for(int numberRerolls = 0; numberRerolls <= 2; ++numberRerolls) {
			vector<double> newSumOfDpVals(28, 0.0);
			//loop from 0 to 252 (every roll combination)
			for(int roll = 0; roll < (int)allRollsIndistinguishable.size(); ++roll) {
				//take a pointer to maxEV
				double &currDp = maxEV[subsetFilled][numberRerolls][roll];
				//take pointer to transitions
				vector<Move> &currTransitions = transitions[subsetFilled][numberRerolls][roll];

				//take roll
				//for each score slot
				for(int scoreVal = 0; scoreVal < 13; ++scoreVal) {
					//if score slot (category) is not empty
					if(subsetFilled & (1<<scoreVal)) {
						//add average expected value to current score
						const double nextScore = scoreForRoll[roll][scoreVal] + averageMaxEV[subsetFilled ^ (1<<scoreVal)];
						currTransitions.push_back({-1, scoreVal, nextScore});
						currDp = max(currDp, nextScore);
					}
				}

				//re-roll
				if(numberRerolls > 0) {
					//for each subset of die that you can re-roll
					for(auto [subsetRerolled,keptDieID] : distinctSubsetsForReroll[roll]) {
						//find average of expected values
						double nextScore = 0;
						if(__builtin_popcount(subsetRerolled) >= 3) {
							nextScore = sumOfDpValsForSubsetKept[keptDieID];
						} else {
							for(auto [cnt, endRoll] : cntReroll[roll][subsetRerolled]) {
								nextScore += cnt * maxEV[subsetFilled][numberRerolls-1][endRoll];
							}
						}
						nextScore /= double(pow6[__builtin_popcount(subsetRerolled)]);
						currTransitions.push_back({subsetRerolled,-1,nextScore});
						currDp = max(currDp, nextScore);
					}
				}

				if(numberRerolls == 2) {
					//add to score to average expected value for specific category
					averageMaxEV[subsetFilled] += numberOfRoll[roll] * currDp / double(pow6[5]);
				} else {
					for(auto [subsetKeptID, cnt] : rollToSubsetKeptCnts[roll]) {
						newSumOfDpVals[subsetKeptID] += cnt * currDp;
					}
				}
			}
			sumOfDpValsForSubsetKept = newSumOfDpVals;
		}
	}
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << "Finished in " << duration.count()/double(1000 * 1000) << " seconds." << endl << endl;
	cout << "The maximum expected score for a single Yahtzee round is" << endl <<
		averageMaxEV[(1<<13)-1] << " points. This is lower than the true value as" << endl <<
		"the program doesn't consider multiple yahtzees (each worth 100 points), or" << endl <<
		"the +35 point bonus for scoring >= 63 points in the top section." << endl << endl;
}

bool cmpSeconds(const pair<int,int> &x, const pair<int,int> &y) {
	return x.second < y.second;
}

void inputOutput() {
	vector<string> scoreDescription = {
		"ones",
		"twos",
		"threes",
		"fours",
		"fives",
		"sixes",
		"three of a kind",
		"four of a kind",
		"full house",
		"small straight",
		"large straight",
		"yahtzee",
		"chance"
	};
	while(true) {
		int sumFilledScores = 0;
		int subsetFilled = 0;
		for(int i = 0; i < 13; ++i) {
			cout << "enter in score for " << scoreDescription[i] << " (or -1 for not filled yet)";
			cout << string(18 - (int)scoreDescription[i].size(), '.') << ' ';
			int score;
			cin >> score;
			if(score >= 0) {
				sumFilledScores += score;
			} else {
				subsetFilled += (1<<i);
			}
		}
		cout << "enter in dice roll (ex: 2 4 6 3 2)" << string(30, '.') << ' ';
		vector<pair<int,int>> roll(5);
		vector<int> rollInt(5);
		for(int i = 0; i < 5; ++i) {
			cin >> roll[i].first;
			roll[i].second = i;
			rollInt[i] = roll[i].first;
		}
		cout << "number of re-rolls left (0, 1, or 2)" << string(28, '.') << ' ';
		int rerolls;
		cin >> rerolls;

		vector<Move> &currTransitions = transitions[subsetFilled][rerolls][getRollId(rollInt)];
		sort(currTransitions.begin(), currTransitions.end());

		cout << endl << "Options are:" << endl;
		for(const auto &currMove : currTransitions) {
			if(currMove.subsetReroll == -1) {//score roll
				cout << "Score roll as " << scoreDescription[currMove.scoreTaken] << " giving " << double(sumFilledScores) + currMove.evForMove << " expected points." << endl;
			} else {//re roll
				assert(currMove.scoreTaken == -1);
				cout << "Keep die";
				sort(roll.begin(), roll.end());
				vector<bool> reroll(5,false);
				for(int die = 0; die < 5; ++die) {
					if(currMove.subsetReroll & (1<<die)) {
						reroll[roll[die].second] = true;
					}
				}
				sort(roll.begin(), roll.end(), cmpSeconds);
				for(int die = 0; die < 5; ++die) {
					if(reroll[die]) {
						cout << " _";
					} else {
						cout << " " << roll[die].first;
					}
				}
				cout << " and reroll giving " << double(sumFilledScores) + currMove.evForMove << " expected points." << endl;
			}
		}
		cout << endl << endl;
	}
}

int main() {
	cout << setprecision(5) << fixed;
	// calculate all possible combinations to roll the 5 dice
	initAllRolls();
	// calculate all scores for each section for all possible dice combinations
	calculateScores();
	// calculate the expected value
	calcExpectedValue();
	// printing
	inputOutput();
}