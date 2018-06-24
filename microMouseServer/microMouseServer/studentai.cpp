/*
SHORTEST PATH PLAN
1. Find the destination.
2. Explore the maze, making a graph representation of it.
3. Dijkstra algorithm to find the shortest path
4. Print out the details of the shortest math.
*/

#include "micromouseserver.h"
#include "vector"
#include "iostream"

//Enums! Enums galore!
enum MouseState {FINDING_FINISH, BACKTRACKING, EXPLORING, COMPUTING_SHORTEST_PATH, END};
enum MouseMovement {MOVE_FORWARD, TURN_LEFT, TURN_RIGHT, MOVE_BACKWARD};

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
    Edge * north = NULL;
    Edge * east = NULL;
    Edge * south = NULL;
    Edge * west = NULL;
};

class Edge
{
public:
    Edge();
    Edge(Node * start_, Node * end_, int distance_);
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

    //I'm so sorry
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
        static int mazeMap[MAZE_WIDTH][MAZE_HEIGHT] = {0};
        static int counter = 0;
        mazeMap[x][y]++;
        MouseMovement nextMove = MOVE_BACKWARD;

        //descision making on when to go left, right, and forward (TODO)

        pastMoves.push_back(nextMove);
        if(nextMove == TURN_LEFT)
        {
            counter++;
            TurnLeft(this);
        }
        else if(nextMove == TURN_RIGHT)
        {
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

Edge::Edge()
{
    start = NULL;
    end = NULL;
    distance = INT_MAX;
}

Edge::Edge(Node * start_, Node * end_, int distance_)
{
    start = start_;
    end = end_;
    distance = distance_;
}
