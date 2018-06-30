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
    static MouseState state = EXPLORING;
    static int x = 0; //Location var
    static int y = 0; //Location var
    static int direction = 0; //N = 0, E = 1, S = 2, W = 3
    static std::vector<MouseMovement> pastMoves; //Log of past movements (so I can go back to the start)
    static bool firstRun = true;

    //Graph
    static std::vector<Node*> nodes;
    static Node destination;

    //First run stuff. Add 0,0 to the graph and print that it's destination searching
    if(firstRun)
    {
        printUI("Searching for finish...");
        nodes.push_back(new Node(0, 0));
        firstRun = false;
    }

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
        static bool firstExplorationRun = true;
        static bool isBacktracking = false;
        static std::vector<Node*> pathTrace;
        static int directionFromLastNode = 0;
        static int distanceFromLastNode = 0;

        if(firstExplorationRun)
        {
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
            else //Edge case if the mouse is completely boxed in for some stupid reason
                isBacktracking = true;
            firstExplorationRun = false;
        }
        if(x == destination.x && y == destination.x) //If at destination, create a new, special node to mark that
        {
            Edge(pathTrace.back(), directionFromLastNode, &destination, (direction + 2) % 4, distanceFromLastNode);
            isBacktracking = true;
        }
        if(isBacktracking)
        {
            if(pathTrace.back()->x == x && pathTrace.back()->y == y && (pastMoves.empty() || pastMoves.back() == MOVE_FORWARD || pastMoves.back() == MOVE_BACKWARD))
            {
                isBacktracking = false;
                if(x == 0 && y == 0 && pathTrace.back()->exploredNorth && pathTrace.back()->exploredEast) //If all nodes have been explored, call foundFinish
                {
                    state = COMPUTING_SHORTEST_PATH;
                    printUI("Exploration phase complete. Start run again to compute and run shortest path");
                    foundFinish();
                    return;
                }
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
        if(!isBacktracking) //That way it can seamlessly transition between backtracking and not backtracking in a single run of studentAI()
        {
            int paths = 3; //Calculate how many paths possible
            if(isWallForward())
                paths--;
            if(isWallLeft())
                paths--;
            if(isWallRight())
                paths--;

            if(paths == 0) //Dead end! Turn around
            {
                TurnRight(this);
                TurnRight(this);
                MoveForward(this);
                pastMoves.push_back(TURN_RIGHT);
                pastMoves.push_back(TURN_RIGHT);
                pastMoves.push_back(MOVE_FORWARD);
            }
            else if(paths == 1) //Navigate by normal rules
            {
                if(!isWallLeft())
                {
                    TurnLeft(this);
                    MoveForward(this);
                    pastMoves.push_back(TURN_LEFT);
                    pastMoves.push_back(MOVE_FORWARD);
                    distanceFromLastNode += 1;
                }
                else if(!isWallForward())
                {
                    MoveForward(this);
                    pastMoves.push_back(MOVE_FORWARD);
                    distanceFromLastNode += 1;
                }
                else
                {
                    TurnRight(this);
                    MoveForward(this);
                    pastMoves.push_back(TURN_RIGHT);
                    pastMoves.push_back(MOVE_FORWARD);
                    distanceFromLastNode += 1;
                }
            }
            else //Node detection handler (multiple possible paths)
            {
                Node * alreadyFound = NULL;
                for(Node * node: nodes) //Check if node is already logged
                    if(node->x == x && node->y == y)
                    {
                        alreadyFound = node;
                        break;
                    }
                if(alreadyFound == NULL) //If this is a new node
                {
                    Node * newNode = new Node(x, y); //create a new node
                    Edge(pathTrace.back(), directionFromLastNode, newNode, (direction + 2) % 2, directionFromLastNode); //Link it with the last explored node
                    switch(direction) //Figure out which ways the mouse can explore from this node
                    {
                    case 0: newNode->exploredWest = isWallLeft(); newNode->exploredNorth = isWallForward(); newNode->exploredEast = isWallRight(); break;
                    case 1: newNode->exploredNorth = isWallLeft(); newNode->exploredEast = isWallForward(); newNode->exploredSouth = isWallRight(); break;
                    case 2: newNode->exploredEast = isWallLeft(); newNode->exploredSouth = isWallForward(); newNode->exploredEast = isWallRight(); break;
                    case 3: newNode->exploredSouth = isWallLeft(); newNode->exploredWest = isWallForward(); newNode->exploredNorth = isWallRight(); break;
                    }
                    pathTrace.push_back(newNode); //Append the node to the pathTrace and node vectors
                    nodes.push_back(newNode);
                    //Next run of studentAI, the mouse will treat this as a node it "just found", so it'll find a way to explore and explore that way
                }
                else if(x == pathTrace.back()->x && y == pathTrace.back()->y) //if this is a node we just found (we're backtracking or coming from a dead end)
                {
                    distanceFromLastNode = 1;
                    if(!alreadyFound->exploredNorth)
                    {
                        alreadyFound->exploredNorth = true;
                        TurnDir(this, 0);
                        directionFromLastNode = 0;
                        MoveForward(this);
                        pastMoves.push_back(MOVE_FORWARD);
                    }
                    else if(!alreadyFound->exploredEast)
                    {
                        alreadyFound->exploredEast = true;
                        TurnDir(this, 1);
                        directionFromLastNode = 1;
                        MoveForward(this);
                        pastMoves.push_back(MOVE_FORWARD);
                    }
                    else if(!alreadyFound->exploredSouth)
                    {
                        alreadyFound->exploredSouth = true;
                        TurnDir(this, 2);
                        directionFromLastNode = 2;
                        MoveForward(this);
                        pastMoves.push_back(MOVE_FORWARD);
                    }
                    else if(!alreadyFound->exploredWest)
                    {
                        alreadyFound->exploredWest = true;
                        TurnDir(this, 3);
                        directionFromLastNode = 3;
                        MoveForward(this);
                        pastMoves.push_back(MOVE_FORWARD);
                    }
                    else //this node is fully explored! Backtrack to the previous node!
                    {
                        pathTrace.pop_back();
                        isBacktracking = true;
                    }
                }
                else //if this is a node we haven't just found, link the two nodes together and backtrack
                {
                    Edge(alreadyFound, (direction + 2) % 4, pathTrace.back(), directionFromLastNode, distanceFromLastNode);
                    isBacktracking = true;
                }
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
