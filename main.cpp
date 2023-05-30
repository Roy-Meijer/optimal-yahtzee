#include <iostream>
#include <vector>
#include <algorithm>
//--------------------------
#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
//--------------------------
#include "Node.h"

#include "csvfile.h"
#include "rapidcsv.h"

#include <conio.h>
#include <sstream> 



#undef USE_GENERATED_RESULTS
#define TREE_RESULT_COLORS 5
#define GENERATE_RESULTS

#ifdef TREE_RESULT_COLORS
std::vector<double> root_color;
std::vector<double> dice_1_color;
std::vector<double> rr_choice_1_color;
std::vector<double> outcome_1_color;
std::vector<double> score_1_color;
std::vector<double> dice_2_color;
std::vector<double> rr_choice_2_color;
std::vector<double> outcome_2_color;
std::vector<double> score_2_color;
#endif


#define NR_OF_ROUNDS 2

int nodeCount = 0;
int nodeCountDepth[9] = {0};
std::vector<Node*>* nodeCountVector = new std::vector<Node*>();
std::vector<Node*>* nodeCountVectorDepth[9];

void AssignOutcomesForRerolls(Node* node, std::vector<Node*>* outcomeNodes); 
Node * AssignRerollDecisions(Node* node, int childIndex, Node* child);
void AssignDiceValues(Node* node, int childIndex);
void generateChildrenCount(Node* node, int depth);

std::vector<Node*>* finalScoreNodes = new std::vector<Node*>();

void generateNodeTree(Node* node, int depth) {
    //Base case:  Break if we have reached the necessary depth
    if (depth == 0) {
        return;
    }
      
    generateChildrenCount(node, depth);

    // Generating the children
    for (int i = 0; i < node->getChildrenCount(); ++i) {

        // Reroll choice nodes have outcome nodes as children, outcome nodes are pre-generated
        if (node->getType() == Node::REROLL_CHOICE_NODE)
        {   
            Node* rootNode = node->getParent()->front()->getParent()->front();
            std::vector<Node*>* outcomeNodes = rootNode->generateOutcomeNodes(rootNode);
            
            AssignOutcomesForRerolls(node, outcomeNodes);

            if (node->getChildren() != nullptr)
            {
                for (Node* outcome: *(node->getChildren()))
                {
                    // Recursively make children
                    if (!(outcome->childHandled)) {
                        outcome->childHandled = true;
                        generateNodeTree(outcome, depth - 1);
                    }
                }
            }
            
        }
        
        else
        {
            Node* child;
            if (node->getType() == Node::OUTCOME_NODE) {
                if (depth > 4) { //first round
                    if (i == 0) { //score in ones
                        int scoreOnes = 1 * ((node->firstDice == 1) + (node->secondDice == 1));
                        child = new Node(node->getChildType(), scoreOnes, node->getScoreTwos());
                    } else { //score in twos
                        int scoreTwos = 2 * ((node->firstDice == 2) + (node->secondDice == 2));
                        child = new Node(node->getChildType(), node->getScoreOnes(), scoreTwos);
                    }
                } else { //second round
                    int scoreOnes = (node->getScoreOnes() == -1 ? 1 * ((node->firstDice == 1) + (node->secondDice == 1)) : node->getScoreOnes());
                    int scoreTwos = (node->getScoreTwos() == -1 ? 2 * ((node->firstDice == 2) + (node->secondDice == 2)) : node->getScoreTwos());
                    bool finalScoreNodeExists = false;
                    for (Node* finalScoreNode : *finalScoreNodes) { //seach in vector of final score nodes
                        if (finalScoreNode->getScoreOnes() == scoreOnes && finalScoreNode->getScoreTwos() == scoreTwos) { //if there is a node with the same score
                            child = finalScoreNode; //add this node as child
                            finalScoreNodeExists = true;
                            break;
                        }
                    }
                    if (finalScoreNodeExists == false) { //if there is no node with this score
                        child = new Node(node->getChildType(), scoreOnes, scoreTwos); //make a new child
                        finalScoreNodes->push_back(child); //add this node to all final score nodes
                    }
                }
                node->addChild(child);
            } else if (node->getType() == Node::NodeType::DICE_NODE) {
                // Assigning Reroll decisions for the Reroll nodes (i.e., Children of Dice Nodes)
                //child = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
                //node->addChild(child);
                child = AssignRerollDecisions(node, i, child);
            } else if (node->getType() == Node::NodeType::ROOT_NODE || node->getType() == Node::NodeType::SCORE_ROOT_NODE) {
                child = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
                node->addChild(child);
                AssignDiceValues(node, i);
            } else {
                child = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
                node->addChild(child);
            }

            // Recursively make children
            generateNodeTree(child, depth - 1);
        }
        
    }
}

typedef struct {
    int r, g, b;
} Color;

#ifdef TREE_RESULT_COLORS
std::string generateColorGradient(double value) {

    double x0 = 0.0;
    double x1 = 1.0;
    Color y0 = {255, 255, 255};  // R, G, B values at X = 0.0
    Color y1 = {60, 255, 60};  // R, G, B values at X = 1.0
    
    // Clamp the value between 0.0 and 1.0
    value = std::max(x0, std::min(x1, value));

    // Calculate the RGB components based on the exponential value
    int red, green, blue;

    if (value == 0.0) {
        red = green = blue = 200;
    } else if (value == 1.0) {
        red = 98;
        green = 176;
        blue = 255;
    } else {
        red = (int)(y0.r + (y1.r - y0.r) * (value - x0) / (x1 - x0));
        green = (int)(y0.g + (y1.g - y0.g) * (value - x0) / (x1 - x0));
        blue = (int)(y0.b + (y1.b - y0.b) * (value - x0) / (x1 - x0));
    }

    // Convert the RGB values to a hexadecimal color string
    std::stringstream ss;
    ss << "#" << std::hex << std::setw(2) << std::setfill('0') << red
       << std::hex << std::setw(2) << std::setfill('0') << green
       << std::hex << std::setw(2) << std::setfill('0') << blue;

    return ss.str();
}
#endif


//static std::string colorToString(int color) {
//    std::stringstream ss;
//    ss << "#" << std::hex << std::setw(6) << std::setfill('0') << (color & 0xFFFFFF);
//    return ss.str();
//}

bool edgeExists(Agraph_t* graph, Agnode_t* node1, Agnode_t* node2) {
    Agedge_t* edge = agfindedge(graph, node1, node2);
    return edge != nullptr;
}

void printGraphvizNodeTree(Node* node, Agraph_t* graph, Agnode_t* parentAgNode, int depth, int siblingIndex, int maxSiblings) 
{
    static const std::string TYPE_STRINGS[] = {
        "root",
        "dice",
        "reroll",
        "outcome",
        "score",
        "_"
    };
    #ifndef TREE_RESULT_COLORS
    static const std::string COLORS[] = {
        "red",
        "green",
        "blue",
        "purple",
        "brown",
        "orange"
    };
    #endif

    static const std::string REROLL_TYPE[] = {
        "N,N",
        "Y,N",
        "N,Y",
        "Y,Y",
    };

    static const std::string DICE_RESULT[] = {
        "1,1",
        "1,2",
        "2,2",
    };

    if (siblingIndex >= maxSiblings) 
    {
        if (siblingIndex == maxSiblings) {
           std::string nodeName = "ellipsis_" + std::to_string(depth) + "_" + std::to_string(reinterpret_cast<uintptr_t>(node));
            Agnode_t* currentNode = agnode(graph, const_cast<char*>(nodeName.c_str()), 1);
            #ifdef TREE_RESULT_COLORS
            agsafeset(currentNode, const_cast<char*>("fillcolor"), const_cast<char*>(generateColorGradient(0.5).c_str()), const_cast<char*>(""));
            agsafeset(currentNode, const_cast<char*>("style"), const_cast<char*>("filled"), const_cast<char*>(""));
            #else
            agsafeset(currentNode, const_cast<char*>("color"), const_cast<char*>(COLORS[node->getType()].c_str()), const_cast<char*>(""));
            #endif
            agset(currentNode, const_cast<char*>("label"), const_cast<char*>("..."));
            agedge(graph, parentAgNode, currentNode, nullptr, 1);
        }
        return;
    }

    if (std::find(nodeCountVector->begin(), nodeCountVector->end(), node) == nodeCountVector->end())  {
        nodeCountVector->push_back(node);
        node->setNodeIndexGlobal(nodeCount);
        nodeCount++;

        if (std::find((nodeCountVectorDepth[depth])->begin(), (nodeCountVectorDepth[depth])->end(), node) == (nodeCountVectorDepth[depth])->end())  {
            (nodeCountVectorDepth[depth])->push_back(node);
            node->setNodeIndexDepth(nodeCountDepth[depth]);
            nodeCountDepth[depth]++;
        }
    }

    Node::NodeType nodeType = node->getType();
    std::string nodeName;
    double nodeColor = 0;

    if (depth < 5) {
        std::string roundIndicator = "1st";
        switch(nodeType) {
            case Node::ROOT_NODE: {
                std::string scoreOnesString = node->getScoreOnes() >= 0 ? std::to_string(node->getScoreOnes()) : " ";
                std::string scoreTwosString = node->getScoreTwos() >= 0 ? std::to_string(node->getScoreTwos()) : " ";
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " [" + scoreOnesString + "][" + scoreTwosString + "]" + " wr=" + std::to_string((int)(100*root_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = root_color[node->getNodeIndexDepth()];
            }
            break;
            case Node::DICE_NODE:
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " " + DICE_RESULT[node->getDiceSum() - 2] + " wr=" + std::to_string((int)(100*dice_1_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = dice_1_color[node->getNodeIndexDepth()];
            break;
            case Node::REROLL_CHOICE_NODE:
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " " + REROLL_TYPE[node->getRerollDecision()] + " wr=" + std::to_string((int)(100*rr_choice_1_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = rr_choice_1_color[node->getNodeIndexDepth()];
            break;
            case Node::OUTCOME_NODE:
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " " + DICE_RESULT[node->getDiceSum() - 2] + " wr=" + std::to_string((int)(100*outcome_1_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = outcome_1_color[node->getNodeIndexDepth()];
            break;
            case Node::SCORE_ROOT_NODE: {
                std::string scoreOnesString = node->getScoreOnes() >= 0 ? std::to_string(node->getScoreOnes()) : " ";
                std::string scoreTwosString = node->getScoreTwos() >= 0 ? std::to_string(node->getScoreTwos()) : " ";
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " [" + scoreOnesString + "][" + scoreTwosString + "]" + " wr=" + std::to_string((int)(100*score_1_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = score_1_color[node->getNodeIndexDepth()];
            }
            break;
            default:
                std::cout << "Invalid node type!";
            break;
        }
    } else {
        std::string roundIndicator = "2nd";
        switch(nodeType) {
            case Node::DICE_NODE:
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " " + DICE_RESULT[node->getDiceSum() - 2] + " wr=" + std::to_string((int)(100*dice_2_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = dice_2_color[node->getNodeIndexDepth()];
            break;
            case Node::REROLL_CHOICE_NODE:
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " " + REROLL_TYPE[node->getRerollDecision()] + " wr=" + std::to_string((int)(100*rr_choice_2_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = rr_choice_2_color[node->getNodeIndexDepth()];
            break;
            case Node::OUTCOME_NODE:
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " " + DICE_RESULT[node->getDiceSum() - 2] + " wr=" + std::to_string((int)(100*outcome_2_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = outcome_2_color[node->getNodeIndexDepth()];
            break;
            case Node::SCORE_ROOT_NODE: {
                std::string scoreOnesString = node->getScoreOnes() >= 0 ? std::to_string(node->getScoreOnes()) : " ";
                std::string scoreTwosString = node->getScoreTwos() >= 0 ? std::to_string(node->getScoreTwos()) : " ";
                nodeName = std::to_string(depth) + "." + std::to_string(node->getNodeIndexDepth()) + " " + TYPE_STRINGS[nodeType] + " [" + scoreOnesString + "][" + scoreTwosString + "]" + " wr=" + std::to_string((int)(100*score_2_color[node->getNodeIndexDepth()])) + "%";
                nodeColor = score_2_color[node->getNodeIndexDepth()];
            }
            break;
            default:
                std::cout << "Invalid node type!";
            break;
        }
    }


    Agnode_t* currentNode = agnode(graph, const_cast<char*>(nodeName.c_str()), 1);
    #ifdef TREE_RESULT_COLORS
    agsafeset(currentNode, const_cast<char*>("fillcolor"), const_cast<char*>(generateColorGradient(nodeColor).c_str()), const_cast<char*>(""));
    agsafeset(currentNode, const_cast<char*>("style"), const_cast<char*>("filled"), const_cast<char*>(""));
    #else
    agsafeset(currentNode, const_cast<char*>("color"), const_cast<char*>(COLORS[node->getType()].c_str()), const_cast<char*>(""));
    #endif

    if (parentAgNode) 
    {
        if (!edgeExists(graph, parentAgNode, currentNode)) 
        {
            agedge(graph, parentAgNode, currentNode, nullptr, 1);
        }
    }

    if (node->getChildren() != nullptr) 
    {
        int childIndex = 0;
        for (auto Childnode : (*node->getChildren())) {
            if (Childnode != nullptr && currentNode != nullptr) {
                 // This is the recursive part
                printGraphvizNodeTree(Childnode, graph, currentNode, depth + 1, 0, maxSiblings);
                ++childIndex;
            }
        }
    }
}

void printTreeAsGraph(const std::vector<Node*>& rootNodes, const std::string& outputFilename, const std::string& outputFiletype, int maxSiblings = 1) 
{
    GVC_t* gvc = gvContext();
    Agraph_t* graph = agopen(const_cast<char*>("NodeTree"), Agdirected, nullptr);

    Agnode_t* sharedParentNode = agnode(graph, const_cast<char*>("start"), 1);
    agset(sharedParentNode, const_cast<char*>("color"), const_cast<char*>("black"));

    int initialSiblingIndex = 0;

    // Recursively generate tree visualisation
    for (const auto& node : rootNodes) {
        printGraphvizNodeTree(node, graph, sharedParentNode, 0, initialSiblingIndex, maxSiblings);
        ++initialSiblingIndex;
    }

    agattr(graph, AGRAPH, (char *)"rankdir", (char *)"LR");  // print from left to right
    agattr(graph, AGRAPH, (char *)"nodesep", (char *)"0.0"); // increase the nodesep attribute
    agattr(graph, AGRAPH, (char *)"overlap", (char *)"false"); // increase the nodesep attribute
    gvLayout(gvc, graph, "dot"); // Use "dot" or "neato", more options: https://graphviz.org/docs/layouts/

    // Save to 'graphs' folder:
    std::string outputFolderName = "";
    std::string outputFilenameIncludingFolder = outputFolderName + outputFilename;
    gvRenderFilename(gvc, graph, const_cast<char*>(outputFiletype.c_str()), const_cast<char*>(outputFilenameIncludingFolder.c_str()));
    gvFreeLayout(gvc, graph);

    agclose(graph);
    gvFreeContext(gvc);
}

void generateChildrenCount(Node* node, int depth)
{
    if (node->getType() == Node::ROOT_NODE || node->getType() == Node::SCORE_ROOT_NODE) {
        // There are three children. Throwing 1 and 1, (throwing 1 and 2 AND throwing 2 and 1), and finally throwing 2 and 2
        node->setChildrenCount(3);

    } else if (node->getType() == Node::DICE_NODE) {
        // There are four children. Don't reroll, reroll dice with value 1, reroll dice with value 2, or rerolling both dice.
        node->setChildrenCount(4);

        // When first roll is 1,1 or 2,2 then rerolling dice 1 and dice 2 is the same. Gives three children!
        if(node->firstDice == node->secondDice)
            node->setChildrenCount(3);

    } else if (node->getType() == Node::REROLL_CHOICE_NODE) {
        //childType = Node::OUTCOME_NODE;
        // Scoring options node
       
        
        // No rerolls: Only one child (the same outcome of the parent. i.e., dice values do not change)
        // reroll dice one/reroll dice two: two children each (rerolled die either one or two, not rerolled die stays the same)
        // reroll both: has three children (1,1), (1,2)/(2,1), (2,2)
        if (node->getRerollDecision() == Node::REROLL_TYPE::NO_REROLLS)
            node->setChildrenCount(1);
        if ((node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE) || (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO)) 
            node->setChildrenCount(2);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_BOTH) 
            node->setChildrenCount(3);

    } else if (node->getType() == Node::OUTCOME_NODE) {
        if (depth > 1) {
            // There are two ways to score in the first round
            node->setChildrenCount(2);
        } else {
            // This is for the final nodes, now there is only one remaining option
            node->setChildrenCount(1);
        }
    } else {
        std::cerr << "Error: This statement should not occur!\n";
        return;
    }
}

void AssignDiceValues(Node* node, int childIndex)
{
    // Assigning value to DICE_NODES (1,1), (1,2), (2,2)
    assert(node->getType() == Node::NodeType::ROOT_NODE || node->getType() == Node::NodeType::SCORE_ROOT_NODE);
    
    std::vector<Node*>* children = node->getChildren();
    if(childIndex == 0)
    {
        // Since generateNodeTree is a 'friend' function to node, it can access the private 'children' members.
        children->at(childIndex)->firstDice = 1;
        children->at(childIndex)->secondDice = 1;
    }
    // This is the dual case (so right now this only works correctly when the NAIVE optimization is NOT ON, but the 1,2=2,1 optimization is ON)
    else if (childIndex == 1) 
    {
        children->at(childIndex)->firstDice = 1;
        children->at(childIndex)->secondDice = 2;
    }
    else if(childIndex == 2)
    {
        children->at(childIndex)->firstDice = 2;
        children->at(childIndex)->secondDice = 2;
    } else {
        std::cerr << "Assign Dice Values: Child index invalid: " << childIndex << "\n";
    }
}

Node * AssignRerollDecisions(Node* node, int childIndex, Node* newChild) {
    assert(node->getType() == Node::NodeType::DICE_NODE);
    Node* parrent = node->getParent()->front();
    std::vector<Node*>* siblings = parrent->getChildren();
    if (node->firstDice == 1 && node->secondDice == 1) {
        if (childIndex == 0) {
            //make new child
            newChild = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
            node->addChild(newChild);
            node->setRerollDecision(newChild, Node::REROLL_TYPE::NO_REROLLS);
        } else {
            bool takeOver = false;
            for(Node* sib: *siblings) {
                if (sib->getChildren() != nullptr) {
                    if ((sib->firstDice == 1 && sib->secondDice == 2) || (sib->firstDice == 2 && sib->secondDice == 1)) {
                        if (childIndex == 1) {
                            if (sib->getChildrenCount() >= 3) { //reroll 0,2 is the 3th child, index 2
                                newChild = (*(sib->getChildren()))[2];
                                node->addChild(newChild);
                                takeOver = true;
                            }
                        } else { //childIndex == 2
                            if (sib->getChildrenCount() >= 4) { //reroll 1,2 is the 4th child, index 3
                                newChild = (*(sib->getChildren()))[3];
                                node->addChild(newChild);
                                takeOver = true;
                            }
                        }
                    } else if (childIndex == 2 && sib->firstDice == 2 && sib->secondDice == 2) {
                        if (sib->getChildrenCount() >= 3) { //reroll 1,2 is the 3th child, index 2
                            newChild = (*(sib->getChildren()))[2];
                            node->addChild(newChild);
                            takeOver = true;
                        }
                    }
                }
            }
            if (takeOver == false) {
                //make new child
                newChild = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
                node->addChild(newChild);
                if (childIndex == 1) {
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO);
                } else { //childIndex == 2
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_BOTH);
                }
            }
        }
    } else if ((node->firstDice == 1 && node->secondDice == 2) || (node->firstDice == 2 && node->secondDice == 1)) {
        if (childIndex == 0) {
            //make new child
            newChild = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
            node->addChild(newChild);
            node->setRerollDecision(newChild, Node::REROLL_TYPE::NO_REROLLS);
        } else {
            bool takeOver = false;
            for(Node* sib: *siblings) {
                if (sib->getChildren() != nullptr) {
                    if ((childIndex == 1 && sib->firstDice == 2 && sib->secondDice == 2) || (childIndex == 2 && sib->firstDice == 1 && sib->secondDice == 1)) {
                        if (sib->getChildrenCount() >= 2) { //reroll 0,2 and reroll 1,0 are the 2nd child, index 1
                            newChild = (*(sib->getChildren()))[1];
                            node->addChild(newChild);
                            takeOver = true;
                        }
                    } else if (childIndex == 3 && ((sib->firstDice == 2 && sib->secondDice == 2) || (sib->firstDice == 1 && sib->secondDice == 1))) {
                        if (sib->getChildrenCount() >= 3) { //reroll 1,2 is the 3th child, index 2
                            newChild = (*(sib->getChildren()))[2];
                            node->addChild(newChild);
                            takeOver = true;
                        }
                    }
                }
            }
            if (takeOver == false) {
                //make new child
                newChild = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
                node->addChild(newChild);
                if (childIndex == 1) {
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE);
                } else if (childIndex == 2) {
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO);
                } else { //childIndex == 3
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_BOTH);
                }
            }
        }
    } else if (node->firstDice == 2 && node->secondDice == 2) {
        if (childIndex == 0) {
            //make new child
            newChild = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
            node->addChild(newChild);
            node->setRerollDecision(newChild, Node::REROLL_TYPE::NO_REROLLS);
        } else {
            bool takeOver = false;
            for(Node* sib: *siblings) {
                if (sib->getChildren() != nullptr) {
                    if ((sib->firstDice == 1 && sib->secondDice == 2) || (sib->firstDice == 2 && sib->secondDice == 1)) {
                        if (childIndex == 1) {
                            if (sib->getChildrenCount() >= 2) { //reroll 0,2 is the 2th child, index 1
                                newChild = (*(sib->getChildren()))[1];
                                node->addChild(newChild);
                                takeOver = true;
                            }
                        } else { //childIndex == 2
                            if (sib->getChildrenCount() >= 4) { //reroll 1,2 is the 4th child, index 3
                                newChild = (*(sib->getChildren()))[3];
                                node->addChild(newChild);
                                takeOver = true;
                            }
                        }
                    } else if (childIndex == 2 && sib->firstDice == 1 && sib->secondDice == 1) {
                        if (sib->getChildrenCount() >= 3) { //reroll 1,2 is the 3th child, index 2
                            newChild = (*(sib->getChildren()))[2];
                            node->addChild(newChild);
                            takeOver = true;
                        }
                    }
                }
            }
            if (takeOver == false) {
                //make new child
                newChild = new Node(node->getChildType(), node->getScoreOnes(), node->getScoreTwos());
                node->addChild(newChild);
                if (childIndex == 1) {
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE);
                } else { //childIndex == 2
                    node->setRerollDecision(newChild, Node::REROLL_TYPE::REROLL_BOTH);
                }
            }
        }
    }

    return newChild;
}

void AssignOutcomesForRerolls(Node* node, std::vector<Node*>* outcomeNodes)
{
    Node* outcome_11 = outcomeNodes->at(0);
    Node* outcome_12 = outcomeNodes->at(1);
    Node* outcome_22 = outcomeNodes->at(2);
    
    if (node->firstDice == 1 && node->secondDice == 1)
    {
        node->addChild(outcome_11);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE)
            node->addChild(outcome_12);
    }
        
    else if (node->firstDice == 1 && node->secondDice == 2)
    {
         node->addChild(outcome_12);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE)
            node->addChild(outcome_22);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO)
            node->addChild(outcome_11);
    }
    else if (node->firstDice == 2 && node->secondDice == 2)
    {
        node->addChild(outcome_22);
        if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO)
             node->addChild(outcome_12);
    }
    
    //if reroll decision is to reroll both, all three outcomes are available
    if (node->getRerollDecision() == Node::REROLL_TYPE::REROLL_BOTH)
    {
        node->addChild(outcome_11);
        node->addChild(outcome_12);
        node->addChild(outcome_22);
    }
}


std::vector<double> strVecToFpVec(const std::vector<std::string>& stringVec) {
    std::vector<double> floatVec;
    
    for (const auto& str : stringVec) {
        if (str.empty()) {
            break;  // Stop conversion when an empty string is encountered
        }
        
        std::istringstream iss(str);
        double value;
        
        if (iss >> value) {
            floatVec.push_back(value);
        } else {
            std::cout << "Invalid string: " << str << std::endl;
            // Handle error or skip the invalid string
        }
    }
    
    return floatVec;
}


int main() {
    srand(static_cast<unsigned>(time(NULL)));

    #ifdef TREE_RESULT_COLORS
    rapidcsv::Document doc("winning_from_" + std::to_string(TREE_RESULT_COLORS) + ".csv");

    root_color = strVecToFpVec(doc.GetColumn<std::string>("Root"));
    dice_1_color = strVecToFpVec(doc.GetColumn<std::string>("Dice 1"));
    rr_choice_1_color = strVecToFpVec(doc.GetColumn<std::string>("RR choice 1"));
    outcome_1_color = strVecToFpVec(doc.GetColumn<std::string>("Outcome 1"));
    score_1_color = strVecToFpVec(doc.GetColumn<std::string>("Score 1"));
    dice_2_color = strVecToFpVec(doc.GetColumn<std::string>("Dice 2"));
    rr_choice_2_color = strVecToFpVec(doc.GetColumn<std::string>("RR choice 2"));
    outcome_2_color = strVecToFpVec(doc.GetColumn<std::string>("Outcome 2"));
    score_2_color = strVecToFpVec(doc.GetColumn<std::string>("Score 2"));
    #endif

    std::cout << "Using all optimizations!\n";

    // Make the root node (start of round 1)
    std::vector<Node*> rootNodes(1);

    // Generate tree of two rounds (each round has 4 levels of nodes)
    int rni = 0;
    for (auto& rootNode : rootNodes) {
        rootNode = new Node(Node::ROOT_NODE);
        generateNodeTree(rootNode, NR_OF_ROUNDS*4 );
        rootNode->setNodeIndexDepth(rni);
        rni++;
    }

    // Save as tree.svg (picture which you can open with your internet browser)
    std::string outputFilename = "tree";
    #ifdef TREE_RESULT_COLORS
    outputFilename += "_win_" + std::to_string(TREE_RESULT_COLORS);
    #endif 
    std::string outputFiletype = "svg"; //you can also change this to png
    outputFilename += "." + outputFiletype;

    // Increasing this number shows more nodes which are on the same layer and makes the tree less readable. it also takes more time to generate tree
    int maxSiblings = 100; 
    // Generate tree figure

    for (int i = 0; i < 9; i++) {
        nodeCountVectorDepth[i] = new std::vector<Node*>();
    }

    printTreeAsGraph(rootNodes, outputFilename, outputFiletype, maxSiblings); 
    std::cout << "In total, there are " << nodeCount << " nodes.\n";

    #ifdef GENERATE_RESULTS

    //simulate program 100000 random times
    const unsigned long simulationTimes = 100000000;
    //the depth of the node tree = 9. we want to store the visited states of every game
    int *gameSimulations = new int[9 * simulationTimes];
    //we want to store the scores of every game
    int *gameScores = new int[simulationTimes];

    Node* firstNode = rootNodes.front();

    std::cout << "running " << simulationTimes << " simulations...\n";

    for (unsigned long sim = 0; sim < simulationTimes; sim++) {
        Node* currentNode = firstNode;
        int nrOfChildren = currentNode->getChildrenCount();
        int nodeDepth = 0;
        //std::cout << "game " << sim << ": ";

        //as long as current node has children, it is not the last
        while (nrOfChildren != 0 && nodeDepth < 9) {
            //find the index of the next child
            int nextChildIndex;
            //all dice throws are not completely random (because 10 = 01)
            if ( 
                 currentNode->getType() == Node::ROOT_NODE ||
                 currentNode->getType() == Node::SCORE_ROOT_NODE ||
                 (currentNode->getType() == Node::REROLL_CHOICE_NODE && currentNode->getRerollDecision() == Node::REROLL_TYPE::REROLL_BOTH)
               ) {
                nextChildIndex = (rand() % 2) + (rand() % 2); // 00, 10, 01, 11
            } else {
                //all choices are completely
                nextChildIndex = rand() % currentNode->getChildrenCount();
            }

            gameSimulations[sim * 9 + nodeDepth] = currentNode->getNodeIndexDepth();

            //std::cout << gameSimulations[sim][nodeDepth] << ", ";
            //how many children has this node?
            nrOfChildren = currentNode->getChildrenCount();
            if (nrOfChildren > 0) {
                currentNode = currentNode = (*(currentNode->getChildren()))[nextChildIndex];
                nodeDepth++;
            }
        }
        gameScores[sim] = currentNode->getScoreOnes() + currentNode->getScoreTwos();
        //std::cout << ": " << gameScores[sim] << "\n";
    }

    std::cout << "writing simulation results to csv files...\n";

    for (int winningScore = 0; winningScore <= 6; winningScore++) {

        csvfile csv("winning_from_" + std::to_string(winningScore) + ".csv");
        csv << "Index" << "Root" << "Dice 1" << "RR choice 1" << "Outcome 1" << "Score 1" << "Dice 2" << "RR choice 2" << "Outcome 2" << "Score 2" << endrow;

        unsigned long winning_root[1] = {0};
        unsigned long winning_dice_1[3] = {0};
        unsigned long winning_rerollchoice_1[6] = {0};
        unsigned long winning_outcome_1[3] = {0};
        unsigned long winning_score_1[6] = {0};
        unsigned long winning_dice_2[18] = {0};
        unsigned long winning_rerollchoice_2[36] = {0};
        unsigned long winning_outcome_2[18] = {0};
        unsigned long winning_score_2[9] = {0};
        
        unsigned long total_root[1] = {0};
        unsigned long total_dice_1[3] = {0};
        unsigned long total_rerollchoice_1[6] = {0};
        unsigned long total_outcome_1[3] = {0};
        unsigned long total_score_1[6] = {0};
        unsigned long total_dice_2[18] = {0};
        unsigned long total_rerollchoice_2[36] = {0};
        unsigned long total_outcome_2[18] = {0};
        unsigned long total_score_2[9] = {0};

        for (unsigned long sim = 0; sim < simulationTimes; sim++) {
            if (gameScores[sim] > winningScore) {
                winning_root[gameSimulations[sim * 9 + 0]]++;
                winning_dice_1[gameSimulations[sim * 9 + 1]]++;
                winning_rerollchoice_1[gameSimulations[sim * 9 + 2]]++;
                winning_outcome_1[gameSimulations[sim * 9 + 3]]++;
                winning_score_1[gameSimulations[sim * 9 + 4]]++;
                winning_dice_2[gameSimulations[sim * 9 + 5]]++;
                winning_rerollchoice_2[gameSimulations[sim * 9 + 6]]++;
                winning_outcome_2[gameSimulations[sim * 9 + 7]]++;
                winning_score_2[gameSimulations[sim * 9 + 8]]++;
            } 
            total_root[gameSimulations[sim * 9 + 0]]++;
            total_dice_1[gameSimulations[sim * 9 + 1]]++;
            total_rerollchoice_1[gameSimulations[sim * 9 + 2]]++;
            total_outcome_1[gameSimulations[sim * 9 + 3]]++;
            total_score_1[gameSimulations[sim * 9 + 4]]++;
            total_dice_2[gameSimulations[sim * 9 + 5]]++;
            total_rerollchoice_2[gameSimulations[sim * 9 + 6]]++;
            total_outcome_2[gameSimulations[sim * 9 + 7]]++;
            total_score_2[gameSimulations[sim * 9 + 8]]++;
        }

        for (int i = 0; i < 36; i++) {
            csv << i;
            if (i < 1) {
                csv << (double)(winning_root[i]) / (double)(total_root[i]);
            } else {
                csv << "";
            }
            if (i < 3) {
                csv << (double)(winning_dice_1[i]) / (double)(total_dice_1[i]);
            } else {
                csv << "";
            }
            if (i < 6) {
                csv << (double)(winning_rerollchoice_1[i]) / (double)(total_rerollchoice_1[i]);
            } else {
                csv << "";
            }
            if (i < 3) {
                csv << (double)(winning_outcome_1[i]) / (double)(total_outcome_1[i]);
            } else {
                csv << "";
            }
            if (i < 6) {
                csv << (double)(winning_score_1[i]) / (double)(total_score_1[i]);
            } else {
                csv << "";
            }
            if (i < 18) {
                csv << (double)(winning_dice_2[i]) / (double)(total_dice_2[i]);
            } else {
                csv << "";
            }
            csv << (double)(winning_rerollchoice_2[i]) / (double)(total_rerollchoice_2[i]);
            if (i < 18) {
                csv << (double)(winning_outcome_2[i]) / (double)(total_outcome_2[i]);
            } else {
                csv << "";
            }
            if (i < 9) {
                csv << (double)(winning_score_2[i]) / (double)(total_score_2[i]);
            } else {
                csv << "";
            }
            csv << endrow;
        }
    }

    std::cout << "done\n";

    delete[] gameSimulations;
    delete[] gameScores;

    #endif

    #ifdef USE_GENERATED_RESULTS

    int scoreToWinFrom = 0;

    std::cout << "This program simulates a coin flipping game. ";
    std::cout << "There are 2 coins, 2 rounds and 2 score options. ";
    std::cout << "In each round, you first have to flip both coins. Your result will be 1 x heads and 1 x tails (H,T), 2 x heads (H,H) or 2 x tails (T,T). ";
    std::cout << "After you flipped both coins 1 time in a round, you can choose to flip no, one of the two or both coins again. ";
    std::cout << "After this, you can choose to score either in the heads or coins section. ";
    std::cout << "Your score for the heads section will be equal to the sum of the number of heads coins you have flipped the last time. ";
    std::cout << "Your score for the tails section will be equal to 2 x the sum of the number of tails coins you have flipped the last time. ";
    std::cout << "After you scored once, the second round starts and you have to do exactly the same. ";
    std::cout << "At the end of the second round, you must put your score in the empty section. ";
    std::cout << "Your final score will be the sum of the number of points you have received from both sections.\n\n";

    std::cout << "In this instance of the game, you can enter a score which you want to defeat. You can see this score as the score of a second player. ";
    std::cout << "The program will help you to statistically make the best choices and maximize your probability of winning.\n\n";

    std::cout << "Enter which score you want to win from (0 - 5).\n";
    std::cin >> scoreToWinFrom;

    while (scoreToWinFrom > 5 || scoreToWinFrom < 0) {
        if (scoreToWinFrom == 6) {
            std::cout << "You cannot win from a score of 6, only lose or draw.\n";
        } else {
            std::cout << "Invalid!\n";
        }
        std::cout << "Enter which score you want to win from (0 - 5).\n";
        std::cin >> scoreToWinFrom;
    }

    std::cout << "Reading CSV file... \n";

    rapidcsv::Document doc("winning_from_" + std::to_string(scoreToWinFrom) + ".csv");

    std::vector<double> rr_choice_1 = strVecToFpVec(doc.GetColumn<std::string>("RR choice 1"));
    std::vector<double> score_1 = strVecToFpVec(doc.GetColumn<std::string>("Score 1"));
    std::vector<double> rr_choice_2 = strVecToFpVec(doc.GetColumn<std::string>("RR choice 2"));

    std::cout << "Done.\n";
    
    std::cout << "Press enter to flip the two coins.\n";
    getch();

    int result_coin_1 = rand() % 2;
    int result_coin_2 = rand() % 2;
    int result_total = result_coin_1 + result_coin_2;     
    if (result_total == 0) {
       std::cout << "(H,H)\n"; 
    } else if (result_total == 1) {
       std::cout << "(H,T)\n"; 
    } else {
       std::cout << "(T,T)\n"; 
    }

    Node* currentNode = (*(rootNodes.front()->getChildren()))[result_total];

    std::cout << "result: " << result_total << "\n";
    std::cout << "child count: " << currentNode->getChildrenCount() << "\n";

    bool invalid_input = true;

    Node::REROLL_TYPE rethrow_choice;

    bool oneistwo = false;

        while (invalid_input) {
        std::cout << "Choose a reflip option. You can choose between:\n";
        if (currentNode->getChildrenCount() == 3) {
            std::cout << "(N,N) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[0]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,N) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[1]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,Y) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[2]->getNodeIndexDepth()] << "\n";
        } else {
            std::cout << "(N,N) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[0]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,N) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[1]->getNodeIndexDepth()] << "\n";
            std::cout << "(N,Y) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[2]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,Y) with win rate " << rr_choice_1[(*(currentNode->getChildren()))[3]->getNodeIndexDepth()] << "\n";
        }
        std::cout << "N means keep, Y means flip again. Please enter your answer in the format X,X \n";
        std::string rethrow_choice_string;
        std::cin >> rethrow_choice_string;

        if (currentNode->getChildrenCount() == 3) {
            if (rethrow_choice_string == "N,N" || rethrow_choice_string == "n,n") {
                rethrow_choice = Node::REROLL_TYPE::NO_REROLLS;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,N" || rethrow_choice_string == "N,Y" || rethrow_choice_string == "y,n" || rethrow_choice_string == "n,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE;
                oneistwo = true;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,Y" || rethrow_choice_string == "y,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_BOTH;
                invalid_input = false;
            } else {
                std::cout << "Invalid!\n";
            }
        } else {
            if (rethrow_choice_string == "N,N" || rethrow_choice_string == "n,n") {
                rethrow_choice = Node::REROLL_TYPE::NO_REROLLS;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,N" || rethrow_choice_string == "y,n") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE;
                invalid_input = false;
            } else if (rethrow_choice_string == "N,Y" || rethrow_choice_string == "n,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,Y" || rethrow_choice_string == "y,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_BOTH;
                invalid_input = false;
            } else {
                std::cout << "Invalid!\n";
            }
        }
    }

    for (int child_index = 0; child_index < currentNode->getChildrenCount(); child_index++) {
        if (oneistwo) {
            if ((*(currentNode->getChildren()))[child_index]->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE || (*(currentNode->getChildren()))[child_index]->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO) {
                currentNode = (*(currentNode->getChildren()))[child_index];
                break;
            }
        } else {
            if ((*(currentNode->getChildren()))[child_index]->getRerollDecision() == rethrow_choice) {
                currentNode = (*(currentNode->getChildren()))[child_index];
                break;
            } 
        }
    }

    oneistwo = false;

    switch (rethrow_choice) {
        case Node::REROLL_SINGLE_DICE_ONE:
            result_coin_1 = rand() % 2;
        break;
        case Node::REROLL_SINGLE_DICE_TWO:
            result_coin_2 = rand() % 2;
        break;
        case Node::REROLL_BOTH:
            result_coin_1 = rand() % 2;
            result_coin_2 = rand() % 2;
        break;
    }

    result_total = result_coin_1 + result_coin_2;

    if (result_total == 0) {
       std::cout << "(H,H)\n"; 
    } else if (result_total == 1) {
       std::cout << "(H,T)\n"; 
    } else {
       std::cout << "(T,T)\n"; 
    }

    //go to outcome node
    for (int child_index = 0; child_index < currentNode->getChildrenCount(); child_index++) {
        if ((*(currentNode->getChildren()))[child_index]->getDiceSum() - 2 == result_total) {
            currentNode = (*(currentNode->getChildren()))[child_index];
            break;
        }
    }

    invalid_input = true;
    int scoreChoice = -1;

    #define SCORE_ONES_CHOICE(idx) ((*(currentNode->getChildren()))[idx]->getScoreOnes() > 0 ? std::to_string((*(currentNode->getChildren()))[idx]->getScoreOnes()) : "X")
    #define SCORE_TWOS_CHOICE(idx) ((*(currentNode->getChildren()))[idx]->getScoreTwos() > 0 ? std::to_string((*(currentNode->getChildren()))[idx]->getScoreTwos()) : "X")

    while (invalid_input) {
        std::cout << "Choose a score section. You can choose between:\n";
        std::cout << "Heads, [" << SCORE_ONES_CHOICE(0) << "][" << SCORE_TWOS_CHOICE(0) << "], with win rate " << score_1[(*(currentNode->getChildren()))[0]->getNodeIndexDepth()] << "\n";
        std::cout << "Tails, [" << SCORE_ONES_CHOICE(1) << "][" << SCORE_TWOS_CHOICE(1) << "], with win rate " << score_1[(*(currentNode->getChildren()))[1]->getNodeIndexDepth()] << "\n";

        std::cout << "Please enter your answer as H or T \n";
        std::string score_choice_string;
        std::cin >> score_choice_string;

        if (score_choice_string == "H" || score_choice_string == "h") {
            scoreChoice = 0;
            invalid_input = false;
        } else if (score_choice_string == "T" || score_choice_string == "t") {
            scoreChoice = 1;
            invalid_input = false;
        } else {
            std::cout << "Invalid!\n";
        }
    }

    std::cout << "[" << SCORE_ONES_CHOICE(scoreChoice) << "][" << SCORE_TWOS_CHOICE(scoreChoice) << "]\n";

    //go to score root node
    currentNode = (*(currentNode->getChildren()))[scoreChoice];

    std::cout << "Second round. ";
    std::cout << "Press enter to flip the two coins.\n";
    getch();

    //second round

    result_coin_1 = rand() % 2;
    result_coin_2 = rand() % 2;
    result_total = result_coin_1 + result_coin_2;     
    if (result_total == 0) {
       std::cout << "(H,H)\n"; 
    } else if (result_total == 1) {
       std::cout << "(H,T)\n"; 
    } else {
       std::cout << "(T,T)\n"; 
    }

    currentNode = (*(currentNode->getChildren()))[result_total];

    invalid_input = true;

    while (invalid_input) {
        std::cout << "Choose a reflip option. You can choose between:\n";
        if (currentNode->getChildrenCount() == 3) {
            std::cout << "(N,N) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[0]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,N) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[1]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,Y) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[2]->getNodeIndexDepth()] << "\n";
        } else {
            std::cout << "(N,N) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[0]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,N) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[1]->getNodeIndexDepth()] << "\n";
            std::cout << "(N,Y) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[2]->getNodeIndexDepth()] << "\n";
            std::cout << "(Y,Y) with win rate " << rr_choice_2[(*(currentNode->getChildren()))[3]->getNodeIndexDepth()] << "\n";
        }
        std::cout << "N means keep, Y means flip again. Please enter your answer in the format X,X \n";
        std::string rethrow_choice_string;
        std::cin >> rethrow_choice_string;

        if (currentNode->getChildrenCount() == 3) {
            if (rethrow_choice_string == "N,N" || rethrow_choice_string == "n,n") {
                rethrow_choice = Node::REROLL_TYPE::NO_REROLLS;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,N" || rethrow_choice_string == "N,Y" || rethrow_choice_string == "y,n" || rethrow_choice_string == "n,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE;
                oneistwo = true;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,Y" || rethrow_choice_string == "y,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_BOTH;
                invalid_input = false;
            } else {
                std::cout << "Invalid!\n";
            }
        } else {
            if (rethrow_choice_string == "N,N" || rethrow_choice_string == "n,n") {
                rethrow_choice = Node::REROLL_TYPE::NO_REROLLS;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,N" || rethrow_choice_string == "y,n") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE;
                invalid_input = false;
            } else if (rethrow_choice_string == "N,Y" || rethrow_choice_string == "n,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO;
                invalid_input = false;
            } else if (rethrow_choice_string == "Y,Y" || rethrow_choice_string == "y,y") {
                rethrow_choice = Node::REROLL_TYPE::REROLL_BOTH;
                invalid_input = false;
            } else {
                std::cout << "Invalid!\n";
            }
        }
    }

    for (int child_index = 0; child_index < currentNode->getChildrenCount(); child_index++) {
        if (oneistwo) {
            if ((*(currentNode->getChildren()))[child_index]->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_ONE || (*(currentNode->getChildren()))[child_index]->getRerollDecision() == Node::REROLL_TYPE::REROLL_SINGLE_DICE_TWO) {
                currentNode = (*(currentNode->getChildren()))[child_index];
                break;
            }
        } else {
            if ((*(currentNode->getChildren()))[child_index]->getRerollDecision() == rethrow_choice) {
                currentNode = (*(currentNode->getChildren()))[child_index];
                break;
            } 
        }
    }

    switch (rethrow_choice) {
        case Node::REROLL_SINGLE_DICE_ONE:
            result_coin_1 = rand() % 2;
        break;
        case Node::REROLL_SINGLE_DICE_TWO:
            result_coin_2 = rand() % 2;
        break;
        case Node::REROLL_BOTH:
            result_coin_1 = rand() % 2;
            result_coin_2 = rand() % 2;
        break;
    }

    result_total = result_coin_1 + result_coin_2;

    if (result_total == 0) {
       std::cout << "(H,H)\n"; 
    } else if (result_total == 1) {
       std::cout << "(H,T)\n"; 
    } else {
       std::cout << "(T,T)\n"; 
    }

    //go to outcome node
    for (int child_index = 0; child_index < currentNode->getChildrenCount(); child_index++) {
        if ((*(currentNode->getChildren()))[child_index]->getDiceSum() - 2 == result_total) {
            currentNode = (*(currentNode->getChildren()))[child_index];
            break;
        }
    }

    //go to final score node
    currentNode = (*(currentNode->getChildren()))[0];
    
    int scoreOnesPrint = currentNode->getScoreOnes() > 0 ? currentNode->getScoreOnes() : 0;
    int scoreTwosPrint = currentNode->getScoreTwos() > 0 ? currentNode->getScoreTwos() : 0;

    std::cout << "Final score:\n";
    std::cout << "[" << currentNode->getScoreOnes() << "][" << currentNode->getScoreTwos() << "]\n";

    if (currentNode->getScoreOnes() + currentNode->getScoreTwos() > scoreToWinFrom) {
        std::cout << "You won.\n";
    } else {
        std::cout << "You lost.";
    }
    

    #endif

    for (int i = 0; i < 9; i++) {
        delete nodeCountVectorDepth[i];
    }

    return 0;
}