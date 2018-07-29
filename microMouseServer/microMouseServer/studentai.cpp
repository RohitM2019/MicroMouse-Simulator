#include "micromouseserver.h"
#include "vector"
#include "string"

enum MouseState
{
   EXPLORING, COMPUTING_SHORTEST_PATH, RUNNING_SHORTEST_PATH, END
};
enum MouseMovement
{
   MOVE_FORWARD, TURN_LEFT, TURN_RIGHT, TURN_AROUND
};
enum Direction
{
   NORTH, EAST, SOUTH, WEST
};

class Edge;

class Node
{
   public:
      Node(int x_, int y_);
      int x = -1;
      int y = -1;
      bool exploredNorth = false;
      bool exploredEast = false;
      bool exploredSouth = false;
      bool exploredWest = false;
      int distance = INT_MAX;
      std::vector<int> path;
      std::vector<Edge*> edges;
};

class Edge
{
   public:
      Edge(Node * start_, int startDir_, Node * end_, int endDir_, int distance_);
      Node * start = NULL;
      Node * end = NULL;
      int startDir = -1;
      int endDir = -1;
      int distance = INT_MAX;
};

void microMouseServer::studentAI()
{
   static MouseState state = EXPLORING;
   static int x = 0; //Location var
   static int y = 0; //Location var
   static int direction = 0; //N = 0, E = 1, S = 2, W = 3
   static int finishX = -1;
   static int finishY = -1;
   static std::vector<Node*> nodes;
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
   if (state == EXPLORING)
   {
      static bool firstExplorationRun = true;
      static std::vector<Node*> pathTrace;
      static int directionFromLastNode = -1;
      static int distanceFromLastNode = 0;
      static bool dontMarkDirectionFromLastNodeAsExplored = true;
      static int counter = 0;
      if (firstExplorationRun)
      {
         nodes.push_back(new Node(0, 0));
         nodes.front()->distance = 0;
         pathTrace.push_back(nodes.front());
         nodes.front()->exploredWest = true;
         nodes.front()->exploredSouth = true;
         nodes.front()->exploredNorth = isWallForward();
         nodes.front()->exploredEast = isWallRight();
         if (!isWallForward())
            directionFromLastNode = 0;
         else if (!isWallRight())
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
      if ((isWallLeft() + isWallForward() + isWallRight()) > 1 && !(x == 0 && y == 0)) //Navigate by normal rules
      {
         if (!isWallLeft())
         {
            counter = 0;
            turnLeft();
            direction = (direction + 3) % 4;
         }
         else if (!isWallForward())
         {
            counter = 0;
         }
         else if (!isWallRight())
         {
            counter++;
            turnRight();
            direction = (direction + 1) % 4;
         }
         else
         {
            counter = 0;
            turnRight();
            turnRight();
            direction = (direction + 2) % 4;
         }
         distanceFromLastNode++;
         MoveForward(this);
      }
      else
      {
         Node * currentNode = NULL;
         for (Node * node : nodes)
         {
            if (node->x == x && node->y == y)
               currentNode = node;
         }
         if (currentNode == NULL)
         {
            currentNode = new Node(x, y); //create a new node
            new Edge(pathTrace.back(), directionFromLastNode, currentNode, (direction + 2) % 4, distanceFromLastNode); //Link it with the last explored node
            distanceFromLastNode = 0;
            switch (direction) //Figure out which ways the mouse can explore from this node
               {
               case 0:
                  currentNode->exploredWest = isWallLeft();
                  currentNode->exploredNorth = isWallForward();
                  currentNode->exploredEast = isWallRight();
                  break;
               case 1:
                  currentNode->exploredNorth = isWallLeft();
                  currentNode->exploredEast = isWallForward();
                  currentNode->exploredSouth = isWallRight();
                  break;
               case 2:
                  currentNode->exploredEast = isWallLeft();
                  currentNode->exploredSouth = isWallForward();
                  currentNode->exploredWest = isWallRight();
                  break;
               case 3:
                  currentNode->exploredSouth = isWallLeft();
                  currentNode->exploredWest = isWallForward();
                  currentNode->exploredNorth = isWallRight();
                  break;
               }
            pathTrace.push_back(currentNode); //Append the node to the pathTrace and node vectors
            nodes.push_back(currentNode);
            dontMarkDirectionFromLastNodeAsExplored = true;
            counter = 0;
         }
         else if (currentNode == pathTrace.back())
         {
            if (counter == 3)
            {
               printUI("Found finish!");
               finishX = x;
               finishY = y;
            }
            if (!dontMarkDirectionFromLastNodeAsExplored)
               switch (directionFromLastNode)
                  {
                  case 0:
                     currentNode->exploredNorth = true;
                     break;
                  case 1:
                     currentNode->exploredEast = true;
                     break;
                  case 2:
                     currentNode->exploredSouth = true;
                     break;
                  case 3:
                     currentNode->exploredWest = true;
                     break;
                  }
            dontMarkDirectionFromLastNodeAsExplored = false;
            switch (direction) //Figure out which ways the mouse can explore from this node
               {
               case 0:
                  currentNode->exploredSouth = true;
                  break;
               case 1:
                  currentNode->exploredWest = true;
                  break;
               case 2:
                  currentNode->exploredNorth = true;
                  break;
               case 3:
                  currentNode->exploredEast = true;
                  break;
               }
            if (!currentNode->exploredNorth)
            {
               directionFromLastNode = 0;
            }
            else if (!currentNode->exploredEast)
            {
               directionFromLastNode = 1;
            }
            else if (!currentNode->exploredSouth)
            {
               directionFromLastNode = 2;
            }
            else if (!currentNode->exploredWest)
            {
               directionFromLastNode = 3;
            }
            else //Already explored all the directions from this node? Turn to where the previous node is and begin backtracking
            {
               pathTrace.pop_back();
               if (pathTrace.empty())
               {
                  if (finishX == -1)
                  {
                     printUI("Exploration complete. No destination found!");
                     state = END;
                  }
                  else
                  {
                     printUI("Exploration complete. Press \"start run\" again to run shortest path.");
                     state = COMPUTING_SHORTEST_PATH;
                  }
                  while(direction != 0)
                  {
                      turnRight();
                      direction = (direction + 1) % 4;
                  }
                  foundFinish();
                  return;
               }
               for (Edge * edge : pathTrace.back()->edges)
               {
                  if (edge->start == currentNode)
                  {
                     directionFromLastNode = edge->startDir;
                     break;
                  }
                  if (edge->end == currentNode)
                  {
                     directionFromLastNode = edge->endDir;
                     break;
                  }
               }
               dontMarkDirectionFromLastNodeAsExplored = true;
            }
            while(direction != directionFromLastNode)
            {
                turnRight();
                direction = (direction + 1) % 4;
            }
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
   else if (state == COMPUTING_SHORTEST_PATH)
   {
      printUI("Calculating shortest path...");
      Node * currentNode = nodes.at(0);
      std::vector<Node*> unvisitedNodes = nodes;
      std::vector<Node*> visitedNodes;
      while (!(currentNode->x == finishX && currentNode->y == finishY))
      {
         for (Edge * edge : currentNode->edges) //Calculate and assign shortest distances/paths to neighboors of current node
         {
            if (currentNode == edge->start)
            {
               if (edge->end->distance > currentNode->distance + edge->distance)
               {
                  edge->end->distance = edge->start->distance + edge->distance;
                  edge->end->path = currentNode->path;
                  edge->end->path.push_back(edge->startDir);
               }
            }
            else
            {
               if (edge->start->distance > currentNode->distance + edge->distance)
               {
                  edge->start->distance = edge->end->distance + edge->distance;
                  edge->start->path = currentNode->path;
                  edge->start->path.push_back(edge->endDir);
               }
            }
         }
         visitedNodes.push_back(currentNode); //Add current node to visited nodes and remove it from unvisited nodes.
         for (auto iter = unvisitedNodes.begin(); iter != unvisitedNodes.end(); iter++)
         {
            if (*iter == currentNode)
            {
               unvisitedNodes.erase(iter);
               break;
            }
         }
         currentNode = unvisitedNodes.front(); //Select next current node
         for (auto iter = unvisitedNodes.begin() + 1; iter != unvisitedNodes.end(); iter++)
         {
            if ((*iter)->distance < currentNode->distance)
            {
               currentNode = *iter;
            }
         }
      }
      printUI("Shortest path calculated!");
      state = RUNNING_SHORTEST_PATH;
   }
   else if (state == RUNNING_SHORTEST_PATH)
   {
      static bool firstRun = true;
      static Node * destination = NULL;
      static int forkNum = 0;
      if (firstRun)
      {
         for (Node * node : nodes)
         {
            if (node->x == finishX && node->y == finishY)
            {
               destination = node;
               break;
            }
         }
      }
      if ((isWallLeft() + isWallForward() + isWallRight()) > 1 && !(x == 0 && y == 0)) //Navigate by normal rules
      {
         if (!isWallLeft())
         {
            turnLeft();
            direction = (direction + 3) % 4;
         }
         else if (isWallForward() && !isWallRight())
         {
            turnRight();
            direction = (direction + 3) % 4;
         }
         else
         {
            turnRight();
            turnRight();
            direction = (direction + 2) % 4;
         }
         MoveForward(this);
      }
      else //If at a fork, figure out which way to go and go that way
      {
          while(direction != destination->path.at(forkNum))
          {
              turnRight();
              direction = (direction + 1) % 4;
          }
         MoveForward(this);
         forkNum++;
      }
      if (forkNum == destination->path.size())
      {
         printUI("Done.");
         state = END;
      }
   }
   else if (state == END)
   {
      foundFinish();
   }
}

Node::Node(int x_, int y_)
{
   x = x_;
   y = y_;
}

Edge::Edge(Node * start_, int startDir_, Node * end_, int endDir_, int distance_)
{
   start = start_;
   end = end_;
   startDir = startDir_;
   endDir = endDir_;
   start->edges.push_back(this);
   end->edges.push_back(this);

   switch (startDir)
      {
      case 0:
         start->exploredNorth = true;
         break;
      case 1:
         start->exploredEast = true;
         break;
      case 2:
         start->exploredSouth = true;
         break;
      case 3:
         start->exploredWest = true;
         break;
      }
   switch (endDir)
      {
      case 0:
         end->exploredNorth = true;
         break;
      case 1:
         end->exploredEast = true;
         break;
      case 2:
         end->exploredSouth = true;
         break;
      case 3:
         end->exploredWest = true;
         break;
      }
   distance = distance_;
}
