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
enum MouseState {FINDING_FINISH, BACKTRACKING, EXPLORING, COMPUTING_SHORTEST_PATH, END};
enum MouseMovement {MOVE_FORWARD, TURN_LEFT, TURN_RIGHT, MOVE_BACKWARD};
enum Direction {NORTH, EAST, SOUTH, WEST};

//Forward declaration of Edge so Node can use it
class Edge;

//Mazemap is an array representation of the maze, with the added bonus that it'll return int max if you try to find how many times an invalid location has been accessed
class MazeMap
{
public:
    MazeMap();
    int & operator()(int x, int y);
    int map[MAZE_WIDTH][MAZE_HEIGHT];
};

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
    static MouseState state = FINDING_FINISH;
    static int x = 0; //Location var
    static int y = 0; //Location var
    static int direction = 0; //N = 0, E = 1, S = 2, W = 3
    static std::vector<MouseMovement> pastMoves; //Log of past movements (so I can go back to the start)

    //Graph
    static std::vector<Node*> nodes;
    static Node destination;

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
                pastMoves.push_back(TURN_LEFT);
                TurnLeft(server);
            }
        else
            while(direction != dir)
            {
                pastMoves.push_back(TURN_RIGHT);
                TurnRight(server);
            }
    };

    if(state == FINDING_FINISH)
    {
        destination.x = 7;
        destination.y = 11;
        state = EXPLORING;
        /*
        static MazeMap map = MazeMap();
        static int counter = 0;
        map(x, y)++;
        MouseMovement nextMove = MOVE_BACKWARD;

        //Get times gone left, right, and forward
        int timesGoneLeft = INT_MAX;
        if(!isWallLeft())
            switch(direction)
            {
            case 0: timesGoneLeft = map(x - 1, y); break;
            case 1: timesGoneLeft = map(x, y + 1); break;
            case 2: timesGoneLeft = map(x + 1, y); break;
            case 3: timesGoneLeft = map(x, y - 1); break;
            }
        int timesGoneForward = INT_MAX;
        if(!isWallForward())
            switch(direction)
            {
            case 0: timesGoneForward = map(x, y + 1); break;
            case 1: timesGoneForward = map(x + 1, y); break;
            case 2: timesGoneForward = map(x, y - 1); break;
            case 3: timesGoneForward = map(x - 1, y); break;
            }
        int timesGoneRight = INT_MAX;
        if(!isWallRight())
            switch(direction)
            {
            case 0: timesGoneRight = map(x + 1, y); break;
            case 1: timesGoneRight = map(x, y - 1); break;
            case 2: timesGoneRight = map(x - 1, y); break;
            case 3: timesGoneRight = map(x, y + 1); break;
            }

        //Figure out which way to go.
        if(timesGoneLeft <= timesGoneForward && timesGoneLeft <= timesGoneRight && timesGoneLeft != INT_MAX)
            nextMove = TURN_LEFT;
        else if(timesGoneForward <= timesGoneRight && timesGoneForward != INT_MAX)
            nextMove = MOVE_FORWARD;
        else if(timesGoneRight != INT_MAX)
            nextMove = TURN_RIGHT;

        pastMoves.push_back(nextMove);
        if(nextMove == TURN_LEFT)
        {
            if(counter < 0)
                counter = 0;
            counter++;
            TurnLeft(this);
        }
        else if(nextMove == TURN_RIGHT)
        {
            if(counter > 0)
                counter = 0;
            counter--;
            TurnRight(this);
        }
        else if(nextMove == MOVE_BACKWARD)
        {
            counter = 0; //reset counter (moving backward breaks streak)
            TurnRight(this);
            TurnRight(this);
        }
        else
        {
            pastMoves.pop_back(); //If the next move is moving forward, pop the last logged item because we don't want to love moveForward twice.
            counter = 0; //reset counter (moving forward breaks streak)
        }

        if(MoveForward(this)) //move forward and log
            pastMoves.push_back(MOVE_FORWARD);


        if(counter == 3 || counter == -3)
        {
            printUI("Found finish! Backtracking...");
            destination.x = x;
            destination.y = y;
            state = BACKTRACKING;
        }
        */
    }
    else if(state == BACKTRACKING) //Pretty simple. Undo whatever the most recent action is and then pop that action. (unoptimal!)
    {
        if(pastMoves.size() == 1)
        {
            printUI("Finished backtracking! Exploring...");
            state = EXPLORING;
        }
        if(pastMoves.back() == TURN_LEFT)
        {
            TurnRight(this);
        }
        else if(pastMoves.back() == TURN_RIGHT)
        {
            TurnLeft(this);
        }
        else if(pastMoves.back() == MOVE_BACKWARD)
        {
            MoveForward(this);
            TurnRight(this);
            TurnRight(this);
        }
        else
        {
            TurnRight(this);
            TurnRight(this);
            MoveForward(this);
            TurnRight(this);
            TurnRight(this);
        }
        pastMoves.pop_back();
    }
    else if(state == EXPLORING)
    {
        //If first run, initialize starting data for first node, path trace, and node list
        //If 0,0 fully explored, exploration finished
        //Count paths
            //<2 paths: default maze behavior
            //Else:
                //If I've never been here before, make a new node and link it to the previous node
                //If this is the node I most recently encountered...
                    //Mark the direction I just came from as explored (it may not be if I'm heading from a dead end
                    //Find how many directions I haven't explored
                        //0: Go back to the previous node in the path
                        //Else: Explore an unexplored direction
                //If this is another node, link the two nodes together and turn around (go back)
        static bool firstExplorationRun = true;
        static std::vector<Node*> pathTrace;
        static int directionFromLastNode = -1;
        static int distanceFromLastNode = 0;
        static bool dontMarkDirectionFromLastNodeAsExplored = true;

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
                foundFinish();
                return;
            }
            firstExplorationRun = false;
            destination.exploredNorth = true;
            destination.exploredEast = true;
            destination.exploredSouth = true;
            destination.exploredWest = true;
            nodes.push_back(&destination);
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
                TurnLeft(this);
                MoveForward(this);
                distanceFromLastNode++;
            }
            else if(!isWallForward())
            {
                MoveForward(this);
                distanceFromLastNode++;
            }
            else if(!isWallRight())
            {
                TurnRight(this);
                MoveForward(this);
                distanceFromLastNode++;
            }
            else
            {
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
            }
            else if(currentNode == pathTrace.back())
            {
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
                else //Already explored all the directions from this node? Turn to where the previous nodeis and begin backtracking
                {
                    pathTrace.pop_back();
                    if(pathTrace.empty())
                    {
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
            }
            else //Found another node? Link the two together and turn back
            {
                new Edge(currentNode, (direction + 2) % 4, pathTrace.back(), directionFromLastNode, distanceFromLastNode);
                distanceFromLastNode = 0;
                pathTrace.push_back(currentNode);
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

//MazeMap class definitions
MazeMap::MazeMap()
{
    for(int x = 0; x < MAZE_WIDTH; x++)
        for(int y = 0; y < MAZE_HEIGHT; y++)
            map[x][y] = 0;
}

int & MazeMap::operator()(int x, int y)
{
    if(x < 0 || x > MAZE_WIDTH - 1 || y < 0 || y > MAZE_HEIGHT - 1) //if accessing out of bounds, return a reference to a junk integer
    {
        int junk = INT_MAX; //memory leak risk, but it'll go out of scope as soon as StudentAI completes anyway.
        return junk;
    }
    return map[x][y];
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
