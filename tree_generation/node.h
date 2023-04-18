#pragma once
#include <vector> 

class Node {
public:
    enum NodeType {
        ROOT_NODE,
        DICE_NODE,
        REROLL_CHOICE_NODE,
        OUTCOME_NODE, //outcomes are (1,1), (2,1)/(1,2), (2,2)
        SCORE_ROOT_NODE
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
    int firstDice = {-1};
    int secondDice = {-1};

    // Constructor
// Constructor overloading, in the case of a reroll node being implemented
Node(NodeType type = DICE_NODE, std::vector<Node*>* parent=nullptr, std::vector<Node*>* scoreOptions = nullptr, REROLL_TYPE reroll_decision = REROLL_NA) : type(type), reroll_decision(reroll_decision), parent(parent), maxExpectedScore(-1.0), numChildren(0) {}

    // Destructor
    virtual ~Node() {
        for (auto child : *children) {
            delete child;
        }
    }

    NodeType getType() const {
        return type;
    }
    
    void setType(Node::NodeType NodeType)  {
        type = NodeType;
    }


    std::vector<Node*>* getParent() const {
        return parent;
    }

    REROLL_TYPE getRerollDecision() const {
        return reroll_decision;
    }

    void setRerollDecision(Node* node, Node::REROLL_TYPE reroll_dec)
    {
        node->reroll_decision = reroll_dec;
    }

    void setParent(Node* newParent) {
        if (parent == nullptr)
            parent = new std::vector<Node*>();

        parent->push_back(newParent);
    }

    void addChild(Node* child) {
        // set the child node type based on the current node type
        if (type == Node::NodeType::ROOT_NODE || type == Node::NodeType::SCORE_ROOT_NODE)
            child->setType(Node::NodeType::DICE_NODE);
        else if (type == Node::NodeType::DICE_NODE)
            child->setType(Node::NodeType::REROLL_CHOICE_NODE);
        else if (type == Node::NodeType::REROLL_CHOICE_NODE)
            child->setType(Node::NodeType::OUTCOME_NODE);
        else if (type == Node::NodeType::OUTCOME_NODE)
            child->setType(Node::NodeType::SCORE_ROOT_NODE);
        else
            std::cerr << "Error: undefined node type\n"; 
            
        children->push_back(child);
        child->setParent(this);
    }

    int getChildrenCount()
    {
        return numChildren;
    }

    int setChildrenCount(int count)
    {
        numChildren = count;
    }

    void removeChild(Node* child) {
        auto it = std::find_if(children->begin(), children->end(),
            [=](Node* node) { return node == child; });

        if (it != children->end()) {
            children->erase(it);
            child->setParent(nullptr);
        }
    }

    std::vector<Node*>* getChildren() {
        return children;
    }

    // Can be called only from a root node
    std::vector<Node*>* generateOutcomeNodes(Node* node)
    {
        if (node->scoreOptions == nullptr)
        {
            if (node->getType() == Node::NodeType::ROOT_NODE)
            {
                // Generate three nodes
                Node* score_11 = new Node(Node::OUTCOME_NODE);
                Node* score_12 = new Node(Node::OUTCOME_NODE);
                Node* score_22 = new Node(Node::OUTCOME_NODE);

                scoreOptions->push_back(score_11);
                scoreOptions->push_back(score_12);
                scoreOptions->push_back(score_22);
            }
        }
        return scoreOptions;
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