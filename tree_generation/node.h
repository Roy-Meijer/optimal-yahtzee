//#pragma once

class Node {
public:
    enum NodeType {
        ROOT_NODE,
        DICE_NODE,
        REROLL_CHOICE_NODE,
        DICE_REROLL_OPTIONS_NODE
    };

    // Not yet implemented
    double maxExpectedScore;

    // It would be better to implement each node in a separate class and use inheritance instead, but to save time that is skipped for now
    enum REROLL_TYPE {
        NO_REROLLS,
        REROLL_ONEDICE,
        REROLL_BOTH,
        REROLL_NA
    };

   

    // The two possible dice rolls. Make it public to not have to deal with getters and setters
    // Two ints instead of array so I don't have to deal with array pointers and stuff
    int diceOne = {-1};
    int diceTwo = {-1};

    // Constructor
    // Constructor overloading, in the case of a reroll node being implemented
    Node(NodeType type, REROLL_TYPE reroll_decision=REROLL_NA, Node* parent = nullptr): type(type), parent(parent), maxExpectedScore(-1.0), reroll_decision(REROLL_NA)  {}

    // Destructor
    virtual ~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    NodeType getType() const {
        return type;
    }

    Node* getParent() const {
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
        parent = newParent;
    }

    void addChild(Node* child) {
        children.push_back(child);
        child->setParent(this);
    }

    void removeChild(Node* child) {
        auto it = std::find_if(children.begin(), children.end(),
            [=](Node* node) { return node == child; });

        if (it != children.end()) {
            children.erase(it);
            child->setParent(nullptr);
        }
    }

    const std::vector<Node*>& getChildren() const {
        return children;
    }

    // Made public so that I can access it more easily lol (don't try this at home)
    std::vector<Node*> children;
private:
    NodeType type;
    Node* parent;
    REROLL_TYPE reroll_decision; // add this line to declare the reroll_decision variable
    //std::vector<Node*> children;
};