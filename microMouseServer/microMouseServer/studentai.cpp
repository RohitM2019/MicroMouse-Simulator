/*
SHORTEST PATH PLAN
1. Find the destination using left-forward-right.
2. Explore the maze, making a graph representation of it.
3. Dijkstra algorithm to find the shortest path
4. Print out the details of the shortest math.
*/

#include "micromouseserver.h"
#include "vector"

//Maze state enum, as well as movement enum.
enum MazeState {FINDING_DESTINATION, BACKTRACKING, EXPLORING, COMPUTING_SHORTEST_PATH, END};
enum MouseMovement {MOVE_FORWARD, TURN_LEFT, TURN_RIGHT};

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
