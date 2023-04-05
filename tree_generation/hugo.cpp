#include <iostream> //cout cin
#include <stdlib.h> //rand

#define NR_OF_DICE 2
#define NR_OF_SIDES 2
#define NR_OF_REROLLS 1
#define NR_OF_SCORE_SECTIONS 2

#define NR_OF_ROUNDS 2 //should be equal to NR_OF_SCORE_SECTIONS, but made separate for testing purposes.

// This is the final scorecard.
int scorecard[NR_OF_SCORE_SECTIONS] = {};

// This is the set of thrown dice
int diceThrow[NR_OF_DICE] = {};


struct root_scorecard_node
{
    // id of node
    int ID = {};
    // should contain ES

    // scorecard:
    int scorecard[NR_OF_SCORE_SECTIONS] = {};
    // 4 dice_node children
    struct dice_node* child1_1;
    struct dice_node* child1_2;
    struct dice_node* child2_1;
    struct dice_node* child2_2;
};

struct dice_node
{
    // id of node
    int ID = {};
    // the specific dice roll (array of values)
    int diceRoll[NR_OF_DICE] = {};
    // the probability of getting this dice roll (for naive implementation all probabilities are the same since we don't cluster)
    //double probability = {};
    // 4 choice_node children
    struct choice_node* zero_rerolls;
    struct choice_node* one_rerolls;
    struct choice_node* other_reroll;
    struct choice_node* both_reroll;
};

struct choice_node
{
    // id of node
    int ID = {};
    // (should contain ES)
    // contains 1, 2, or 3 reroll_node children
    struct reroll_node* child1;         // always used, sometimes as passthrough for no_reroll parent
    struct reroll_node* child2;         // used if a dice is rerolled, or both 
    struct reroll_node* child3;         // only used if both dice are rerolled
};

struct reroll_node
{
    // id of node
    int ID = {};
    // the specific dice roll (array of values)
    int diceRoll[NR_OF_DICE] = {};
    // (the probability)
    //double probability = {};
    // 2 root_scorecard_node children
    struct root_scorecard_node* option_ones;
    struct root_scorecard_node* option_twos;
};

// Function to create a new node:



void init ()
{
    // Init the scorecard, no score filled in is -1.
    for (int scoreSection = 0; scoreSection < NR_OF_SCORE_SECTIONS; scoreSection++) 
    {
        scorecard[scoreSection] = -1;
    }
    // Init the dice, dice not thrown is -1. (not really needed I guess)
    for (int dice = 0; dice < NR_OF_DICE; dice++) 
    {
        diceThrow[dice] = -1;
    }
    
}

/*void throwDice () 
{
    // UPDATE THIS: only rethrow the dice that need to be rethrown
    // currently, it rethrows all the dice

    // set the random seed
    srand((unsigned int)time(NULL));
    // for all dice, 
    for (int dice = 0; dice < NR_OF_DICE; dice++)
    {
        diceThrow[dice] =  rand() % NR_OF_SIDES + 1;
    }
    
}*/

void printScorecard () 
{
    std::cout << "-------\n";
    for (int score_section = 0; score_section < NR_OF_SCORE_SECTIONS; score_section++)
    {
        std::cout << score_section + 1 << "'s: " << scorecard[score_section] << '\n';
    }
    std::cout << "-------\n";    
}



int main ( )
{  

    /*
    // Init variables 
    init();

    // Play the game! There are NR_OF_SCORE_SECTIONS rounds, and in each round one part of the scorecard is filled
    for (int round = 0; round < NR_OF_ROUNDS; round++)
    {
        // - First dice roll of the round -
        //throwDice();


        // - Calculate expected score for all possible reroll combinations (reroll 0 dice, reroll first dice, reroll second dice, reroll both dice)

        // - Pick the choice with the highest ES 
        
        // - Update the scorecard - 

        // - Print the current scorecard -
        std::cout << "The scorecard for round " << round + 1 << " is: \n"; 
        printScorecard();

        // - End round -
    }
    
    

    //print final score sum
    int scoreSum = {};
    for (int score_section = 0; score_section < NR_OF_SCORE_SECTIONS; score_section++)
    {
        scoreSum += scorecard[score_section];
    }
    
    std::cout << "Final score is " << scoreSum << ".\n";
    */

   std::cout << "hello, world\n";

    // declare and allocate new root node
    struct root_scorecard_node* root = new struct root_scorecard_node();
    // assign ID
    root->ID = 1;

    // declare and allocate new dice node child
    struct dice_node* child1_1_pointer = new struct dice_node();
    // point in right direction
    root->child1_1 = child1_1_pointer;
    // change the ID    
    root->child1_1->ID = 2;

    // declare and allocate new choice node child
    struct choice_node* child1_1_choice_node_pointer = new struct choice_node();
    // point in right direction
    root->child1_1->zero_rerolls = child1_1_choice_node_pointer;
    // change the ID    
    root->child1_1->zero_rerolls->ID = 6;

    // declare and allocate new reroll node child
    struct reroll_node* child1_1_choice_node_reroll_node_pointer = new struct reroll_node();
    // point in right direction
    root->child1_1->zero_rerolls->child1 = child1_1_choice_node_reroll_node_pointer;
    // change the ID    
    root->child1_1->zero_rerolls->child1->ID = 100;

    return 0;
}