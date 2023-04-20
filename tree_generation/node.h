#pragma once
#include <vector> 
#include <algorithm>

class Node {
public:
    enum NodeType {
        ROOT_NODE,
        DICE_NODE,
        REROLL_CHOICE_NODE,
        OUTCOME_NODE, //outcomes are (1,1), (2,1)/(1,2), (2,2)
        SCORE_ROOT_NODE,
        NODE_ERROR
    };

    // Not yet implemented
    double maxExpectedScore;

    // It would be better to implement each node in a separate class and use inheritance instead, but to save time that is skipped for now
    enum REROLL_TYPE {
        NO_REROLLS,
        REROLL_SINGLE_DICE_ONE, //Reroll the single dice with value = 1
        REROLL_SINGLE_DICE_TWO, //Reroll the single dice with value = 2
        REROLL_BOTH,
        REROLL_NA
    };

    // The two possible dice rolls. Make it public to not have to deal with getters and setters
    // Two ints instead of array so I don't have to deal with array pointers and stuff
    int firstDice = -1;
    int secondDice = -1;

    // Constructor
    // Constructor overloading, in the case of a reroll node being implemented
    Node(NodeType type = DICE_NODE, std::vector<Node*>* parent=nullptr, std::vector<Node*>* scoreOptions = nullptr, std::vector<Node*>* children = nullptr, REROLL_TYPE reroll_decision = REROLL_NA)
    : type(type), reroll_decision(reroll_decision), parent(parent), maxExpectedScore(-1.0), numChildren(0), children(children), scoreOptions(scoreOptions) {


    }

    // Destructor
    virtual ~Node() 
    {
        if (children != nullptr) { // added a check to prevent null pointer exception
            for (auto child : *children) {
                delete child;
            }
            delete children; // added to avoid memory leak
        }
        delete scoreOptions; // added to avoid memory leak
    }

    NodeType getType()  
    {
        return type;
    }
    
    void setType(Node::NodeType NodeType)  
    {
        type = NodeType;
    }

    void setDiceValues(int val1, int val2)
    {
        firstDice = val1;
        secondDice = val2;
    }

    std::vector<Node*>* getParent()  
    {
        return parent;
    }

    REROLL_TYPE getRerollDecision()  
    {
        return reroll_decision;
    }

    //Can set children node reroll decision, so keep the node as an input parameter
    void setRerollDecision(Node* node, Node::REROLL_TYPE reroll_dec)
    {
        node->reroll_decision = reroll_dec;
    }

    void setParent(Node* newParent) 
    {   
        // If there is no parent yet, we add the parent
        if (parent == nullptr) 
        {
            parent = new std::vector<Node*>();
            parent->push_back(newParent);
        }
        
        // ONLY OUTCOME nodes can have multiple parents, and the parent is not already a parent of the node
        if (parent != nullptr && this->getType() == Node::NodeType::OUTCOME_NODE)
        {
            //only add the newParent if it is not already a parent of the current node
            if (std::find(parent->begin(), parent->end(), newParent) == parent->end() ) 
            {
                parent->push_back(newParent);
            }
        }
    }

    // get the type of the children nodes of the current node
    Node::NodeType getChildType()
    {
        // set the child node type based on the current node type
        if (this->getType() == Node::NodeType::ROOT_NODE || this->getType() == Node::NodeType::SCORE_ROOT_NODE) 
            return Node::NodeType::DICE_NODE;
        else if (this->getType() == Node::NodeType::DICE_NODE) 
            return Node::NodeType::REROLL_CHOICE_NODE;
        else if (this->getType() == Node::NodeType::REROLL_CHOICE_NODE) 
            return Node::NodeType::OUTCOME_NODE;
        else if (this->getType() == Node::NodeType::OUTCOME_NODE) 
            return Node::NodeType::SCORE_ROOT_NODE;
        else
        {
            std::cerr << "Error: undefined node type\n";
            return Node::NodeType::NODE_ERROR;
        }       
    }

    void addChild(Node* child, int& nodeCount) 
    {
        if (children == nullptr) 
        {
            children = new std::vector<Node*>();
        }
       
        // only add the child if it does not exist 
        if (std::find(children->begin(), children->end(), child) == children->end() ) 
        {
            children->push_back(child);
            nodeCount++;

            if (this->getType() == Node::NodeType::DICE_NODE) 
            {
                // if current node is dice node, assign dice values to the child nodes (reroll nodes)
                // This will simplify the process of deciding the children of reroll nodes (outcomes)
                child->setDiceValues(firstDice, secondDice);
            }
        }
        child->setParent(this);
        
    }

    int getChildrenCount()
    {
        return numChildren;
    }

    void setChildrenCount(int count)
    {
        numChildren = count;
    }

    std::vector<Node*>* getChildren() 
    {
        return children;
    }

    // Should be called only from a root node
    std::vector<Node*>* generateOutcomeNodes(Node* node, int& nodeCount)
    {
        assert (node->getType() == Node::NodeType::ROOT_NODE || node->getType() == Node::NodeType::SCORE_ROOT_NODE);
        if (node->scoreOptions == nullptr)
        {
            node->scoreOptions = new std::vector<Node*>();
            if (node->getType() == Node::NodeType::ROOT_NODE || node->getType() == Node::NodeType::SCORE_ROOT_NODE)
            {
                // Generate three nodes
                Node* score_11 = new Node(Node::OUTCOME_NODE);
                Node* score_12 = new Node(Node::OUTCOME_NODE);
                Node* score_22 = new Node(Node::OUTCOME_NODE);

                score_11->setDiceValues(1,1);
                score_12->setDiceValues(1,2);
                score_22->setDiceValues(2,2);

                std::cout << "scoreOptions: " << node->scoreOptions << "->push_back(" << score_11 << ")\n";

                std::cout << "scoreOptions Values are: (" << score_11->firstDice << "," << score_11->secondDice << ")\n";
                node->scoreOptions->push_back(score_11);
                node->scoreOptions->push_back(score_12);
                node->scoreOptions->push_back(score_22);

                nodeCount += 3;
            }
        }
        return node->scoreOptions;
    }
    

    // generateNodeTree is a friend so that it can access the private std::vector<Node*> children variable;
    friend void generateNodeTree(Node* node, int depth);

private:
    NodeType type;
    NodeType childType;
    std::vector<Node*>* parent;
    std::vector<Node*>* children;
    int numChildren;
    std::vector<Node*>* scoreOptions;
    // To declare the reroll_decision variable
    REROLL_TYPE reroll_decision; 

};
