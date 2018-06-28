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
    Edge * north = NULL;
    Edge * east = NULL;
    Edge * south = NULL;
    Edge * west = NULL;
    std::vector<Edge*> edges;
};

class Edge
{
public:
    Edge();
    Edge(Node * start_, int startDir, Node * end_, int endDir, int distance_);
    Node * start;
    Node * end;
    int distance;
};

void microMouseServer::studentAI()
{
    //Universal variables
    static MouseState state = FINDING_FINISH;
    static int x = 0; //Location var
    static int y = 0; //Location var
    static int finishX = 0;
    static int finishY = 0;
    static int direction = 0; //N = 0, E = 1, S = 2, W = 3
    static std::vector<MouseMovement> pastMoves; //Log of past movements (so I can go back to the start)
    static bool firstRun = true;

    //Graph
    static std::vector<Node> nodes;

    //First run stuff. Add 0,0 to the graph and print that it's destination searching
    if(firstRun)
    {
        printUI("Searching for finish...");
        nodes.push_back(Node(0, 0));
        firstRun = false;
    }

    //Helper functions...I'm so sorry
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
        {
            pastMoves.push_back(MOVE_FORWARD);
        }

        if(counter == 3 || counter == -3)
        {
            printUI("Found finish! Backtracking...");
            finishX = x;
            finishY = y;
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
        static bool isBacktracking = false;
        static Node * lastVisitedNode = &nodes.at(0);
        static int directionFromLastNode = 0;
        static int movementsFromLastNode = 0;
        static int distanceFromLastNode = 0;

        if(isBacktracking)
        {

        }
        else
        {
            int pathCount = 0;
            if(!isWallLeft())
                pathCount++;
            if(!isWallRight())
                pathCount++;
            if(!isWallForward())
                pathCount++;

            if (pathCount == 0)
            {
                TurnRight(this);
                TurnRight(this);
                MoveForward(this);
                pastMoves.push_back(MOVE_BACKWARD);
                movementsFromLastNode += 3;
                distanceFromLastNode += 1;
            }
            else if (pathCount == 1)
            {
                //Default exploration routine
                if(!isWallLeft())
                {
                    TurnLeft(this);
                    MoveForward(this);
                    pastMoves.push_back(TURN_LEFT);
                    pastMoves.push_back(MOVE_FORWARD);
                    movementsFromLastNode += 2;
                    distanceFromLastNode += 1;
                }
                else if(!isWallForward())
                {
                    MoveForward(this);
                    pastMoves.push_back(MOVE_FORWARD);
                    movementsFromLastNode += 1;
                    distanceFromLastNode += 1;
                }
                else
                {
                    TurnRight(this);
                    MoveForward(this);
                    pastMoves.push_back(TURN_RIGHT);
                    pastMoves.push_back(MOVE_FORWARD);
                    movementsFromLastNode += 2;
                    distanceFromLastNode += 1;
                }
            }
            else //Fork handler
            {
                Node * nodeAlreadyVisited = NULL;
                for(Node node: nodes)
                    if(node.x == x && node.y == y)
                        nodeAlreadyVisited = &node;
                if(nodeAlreadyVisited == NULL) //create a new node
                {
                    nodes.push_back(Node(x, y));
                    Edge(lastVisitedNode, directionFromLastNode, &nodes.back(), direction, distanceFromLastNode); //link the two nodes together.
                    lastVisitedNode = &nodes.back();
                    directionFromLastNode = direction;
                    movementsFromLastNode = 0;
                    distanceFromLastNode = 0;
                }
                else //handler for reaching a node I already know about
                {
                    if(nodeAlreadyVisited->x == lastVisitedNode->x && nodeAlreadyVisited->y == lastVisitedNode->y) //if we just backtracked to the same node
                    {
                        //this is because I'm either backtracking from a dead end or some kind of loop.
                    }
                    else
                    {
                        Edge(lastVisitedNode, directionFromLastNode, nodeAlreadyVisited, direction, distanceFromLastNode); //link the two nodes together
                        isBacktracking = true;
                    }
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

Edge::Edge(Node * start_, int startDir, Node * end_, int endDir, int distance_)
{
    start = start_;
    end = end_;

    start->edges.push_back(this);
    end->edges.push_back(this);

    switch(startDir)
    {
    case 0: start->north = this; break;
    case 1: start->east = this; break;
    case 2: start->south = this; break;
    case 3: start->west = this; break;
    }
    switch(endDir)
    {
    case 0: end->north = this; break;
    case 1: end->east = this; break;
    case 2: end->south = this; break;
    case 3: end->west = this; break;
    }

    distance = distance_;
}
