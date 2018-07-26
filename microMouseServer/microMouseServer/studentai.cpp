/*
SHORTEST PATH PLAN
1. Find the destination.
2. Explore the maze, making a graph representation of it.
3. Dijkstra algorithm to find the shortest path
*/

#include "micromouseserver.h"
#include "vector"
#include "iostream"
#include "string"

//Enums! Enums galore!
enum MouseState {EXPLORING, COMPUTING_SHORTEST_PATH, END};
enum MouseMovement {MOVE_FORWARD, TURN_LEFT, TURN_RIGHT, TURN_AROUND};
enum Direction {NORTH, EAST, SOUTH, WEST};

//Forward declaration of Edge so Node can use it
class Edge;

//Node and edge are classes used to represent the maze as a graph.
//Each node is a "fork" in the maze, and the edge stores information about the connection of two nodes.
class Node
{
public:
    Node();
    Node(int x_, int y_);
    int x;
    int y;
    operator std::string() const;
    bool exploredNorth = false;
    bool exploredEast = false;
    bool exploredSouth = false;
    bool exploredWest = false;
    std::vector<Edge*> edges;
};

class Edge
{
public:
    Edge();
    Edge(Node * start_, int startDir_, Node * end_, int endDir_, int distance_);
    Node * start;
    Node * end;
    int startDir;
    int endDir;
    int distance;
};

void microMouseServer::studentAI()
{
    //Universal variables
    static MouseState state = EXPLORING;
    static int x = 0; //Location var
    static int y = 0; //Location var
    static int direction = 0; //N = 0, E = 1, S = 2, W = 3
    static int finishX = -1;
    static int finishY = -1;

    //Graph
    static std::vector<Node*> nodes;

    static std::function<void(microMouseServer * server)> TurnLeft = [](microMouseServer * server)
    {
        server->turnLeft();
        direction += 3;
        direction = direction % 4;
    };
    static std::function<void(microMouseServer * server)> TurnRight = [](microMouseServer * server)
    {
        server->turnRight();
        direction += 1;
        direction = direction % 4;
    };
    static std::function<bool(microMouseServer * server)> MoveForward = [](microMouseServer * server)
    {
        if(!server->moveForward())
            return false;
        if(direction == 0)
            y++;
        else if(direction == 1)
            x++;
        else if(direction == 2)
            y--;
        else
            x--;
        return true;
    };
    static std::function<void(microMouseServer * server, int dir)> TurnDir = [](microMouseServer * server, int dir)
    {
        if(std::abs(direction - dir < 2))
            while(direction != dir)
            {
                TurnLeft(server);
            }
        else
            while(direction != dir)
            {
                TurnRight(server);
            }
    };

    if(state == EXPLORING)
    {
        //If first run, initialize starting data for first node, path trace, and node list
        //If 0,0 fully explored, exploration finished
        //Count paths
            //<2 paths: default maze behavior
            //Else:
                //If I've never been here before, make a new node and link it to the previous node.
                //If this is the node I most recently encountered...
                    //Mark the direction I just came from as explored (it may not be if I'm heading from a dead end)
                    //Find how many directions I haven't explored
                        //0: Go back to the previous node in the path
                            //No more nodes? Search for destination. Done exploring.
                        //Else: Explore an unexplored direction
                //If this is another node, link the two nodes together and turn around (go back)
        static bool firstExplorationRun = true;
        static std::vector<Node*> pathTrace;
        static int directionFromLastNode = -1;
        static int distanceFromLastNode = 0;
        static bool dontMarkDirectionFromLastNodeAsExplored = true;
        static int counter = 0;

        if(firstExplorationRun)
        {
            nodes.push_back(new Node(0,0));
            pathTrace.push_back(nodes.front());
            //It wouldn't make sense for the mouse to be able to go west or south from the starting corner
            nodes.front()->exploredWest = true;
            nodes.front()->exploredSouth = true;
            nodes.front()->exploredNorth = isWallForward();
            nodes.front()->exploredEast = isWallRight();
            //Special handlers for starting direction from the first node
            if(!isWallForward())
                directionFromLastNode = 0;
            else if(!isWallRight())
                directionFromLastNode = 1;
            else
            {
                printUI("Maze is completely blocked off!");
                state = END;
                foundFinish();
            }
            firstExplorationRun = false;
            return;
        }

        int paths = 3; //Calculate how many paths possible
        if(isWallForward())
            paths--;
        if(isWallLeft())
            paths--;
        if(isWallRight())
            paths--;

        if(paths < 2 && !(x == 0 && y == 0)) //Navigate by normal rules
        {
            if(!isWallLeft())
            {
                if(counter < 0)
                {
                    counter = 0;
                }
                counter++;
                TurnLeft(this);
                MoveForward(this);
                distanceFromLastNode++;
            }
            else if(!isWallForward())
            {
                counter = 0;
                MoveForward(this);
                distanceFromLastNode++;
            }
            else if(!isWallRight())
            {
                if(counter > 0)
                {
                    counter = 0;
                }
                counter--;
                TurnRight(this);
                MoveForward(this);
                distanceFromLastNode++;
            }
            else
            {
                counter = 0;
                TurnRight(this);
                TurnRight(this);
                MoveForward(this);
                distanceFromLastNode++;
            }
        }
        else
        {
            Node * currentNode = NULL;
            for(Node * node: nodes)
            {
                if(node->x == x && node->y == y)
                    currentNode = node;
            }
            if(currentNode == NULL)
            {
                currentNode = new Node(x, y); //create a new node
                new Edge(pathTrace.back(), directionFromLastNode, currentNode, (direction + 2) % 4, distanceFromLastNode); //Link it with the last explored node
                distanceFromLastNode = 0;
                switch(direction) //Figure out which ways the mouse can explore from this node
                {
                case 0: currentNode->exploredWest = isWallLeft(); currentNode->exploredNorth = isWallForward(); currentNode->exploredEast = isWallRight(); break;
                case 1: currentNode->exploredNorth = isWallLeft(); currentNode->exploredEast = isWallForward(); currentNode->exploredSouth = isWallRight(); break;
                case 2: currentNode->exploredEast = isWallLeft(); currentNode->exploredSouth = isWallForward(); currentNode->exploredWest = isWallRight(); break;
                case 3: currentNode->exploredSouth = isWallLeft(); currentNode->exploredWest = isWallForward(); currentNode->exploredNorth = isWallRight(); break;
                }
                pathTrace.push_back(currentNode); //Append the node to the pathTrace and node vectors
                nodes.push_back(currentNode);
                dontMarkDirectionFromLastNodeAsExplored = true;
                counter = 0;
            }
            else if(currentNode == pathTrace.back())
            {
                if(counter == 3 || counter == -3)
                {
                    printUI("Found finish!");
                    finishX = x;
                    finishY = y;
                }
                if(!dontMarkDirectionFromLastNodeAsExplored)
                    switch (directionFromLastNode)
                    {
                    case 0: currentNode->exploredNorth = true; break;
                    case 1: currentNode->exploredEast = true; break;
                    case 2: currentNode->exploredSouth = true; break;
                    case 3: currentNode->exploredWest = true; break;
                    }
                dontMarkDirectionFromLastNodeAsExplored = false;
                switch(direction) //Figure out which ways the mouse can explore from this node
                {
                case 0: currentNode->exploredSouth = true; break;
                case 1: currentNode->exploredWest = true; break;
                case 2: currentNode->exploredNorth = true; break;
                case 3: currentNode->exploredEast = true; break;
                }
                if(!currentNode->exploredNorth)
                {
                    directionFromLastNode = 0;
                }
                else if(!currentNode->exploredEast)
                {
                    directionFromLastNode = 1;
                }
                else if(!currentNode->exploredSouth)
                {
                    directionFromLastNode = 2;
                }
                else if(!currentNode->exploredWest)
                {
                    directionFromLastNode = 3;
                }
                else //Already explored all the directions from this node? Turn to where the previous node is and begin backtracking
                {
                    pathTrace.pop_back();
                    if(pathTrace.empty())
                    {
                        if(finishX == -1)
                        {
                            printUI("Exploration complete. No destination found!");
                            state = END;
                        }
                        else
                        {
                            printUI("Exploration complete. Press \"start run\" again to run shortest path.");
                            state = COMPUTING_SHORTEST_PATH;
                        }
                        foundFinish();
                        return;
                    }
                    for(Edge * edge: pathTrace.back()->edges)
                    {
                        if(edge->start == currentNode)
                        {
                            directionFromLastNode = edge->startDir;
                            break;
                        }
                        if(edge->end == currentNode)
                        {
                            directionFromLastNode = edge->endDir;
                            break;
                        }
                    }
                    dontMarkDirectionFromLastNodeAsExplored = true;
                }
                TurnDir(this, directionFromLastNode);
                MoveForward(this);
                distanceFromLastNode = 1;
                counter = 0;
            }
            else //Found another node? Link the two together and turn back
            {
                new Edge(currentNode, (direction + 2) % 4, pathTrace.back(), directionFromLastNode, distanceFromLastNode);
                distanceFromLastNode = 0;
                pathTrace.push_back(currentNode);
                counter = 0;
            }
        }
    }
    else if(state == COMPUTING_SHORTEST_PATH)
    {
        state = END;
    }
    else if(state == END)
    {
        foundFinish();
    }
}

//Node and Edge class definitions.
Node::Node()
{
    x = -1;
    y = -1;
}

Node::Node(int x_, int y_)
{
    x = x_;
    y = y_;
}

Node::operator std::string() const
{
    return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
}

Edge::Edge()
{
    start = NULL;
    end = NULL;
    distance = INT_MAX;
}

Edge::Edge(Node * start_, int startDir_, Node * end_, int endDir_, int distance_)
{
    start = start_;
    end = end_;
    startDir = startDir_;
    endDir = endDir_;

    start->edges.push_back(this);
    end->edges.push_back(this);

    switch(startDir)
    {
    case 0: start->exploredNorth = true; break;
    case 1: start->exploredEast = true; break;
    case 2: start->exploredSouth = true; break;
    case 3: start->exploredWest = true; break;
    }
    switch(endDir)
    {
    case 0: end->exploredNorth = true; break;
    case 1: end->exploredEast = true; break;
    case 2: end->exploredSouth = true; break;
    case 3: end->exploredWest = true; break;
    }

    distance = distance_;
}
