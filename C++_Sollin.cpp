

#include "stdafx.h"
#include<iostream>
#include<fstream>
#include<cstdlib>
#include<string>
#define SIZE 100
#define vertexNUM 7

using namespace std;

typedef struct edge *edgePointer;
typedef struct edge
{
	edgePointer right;

	int cost;
	int u;//start vertx u.
	int v;//destination vertex v.


};




typedef struct vertex *vertexPointer;
typedef struct vertex
{
	edgePointer right;

	vertexPointer down;
	int value;

};


//to read in and store the input from the file.
void read_input_from_file(int read_in[][vertexNUM])
{



	fstream fin;
	fin.open("adjacency matrix.txt", ios::in);
	for (int i = 0; i < vertexNUM; i++)
	{

		for (int j = 0; j < vertexNUM; j++)
		{
			fin >> read_in[i][j];


		}


	}


}


//generate and initialize the first edge node
edgePointer attach_first_edge(vertexPointer tail, int cost, int i, int j)
{
	edgePointer new_edge;
	new_edge = (edgePointer)malloc(sizeof(edge));

	new_edge->cost = cost;
	new_edge->right = NULL;
	new_edge->u = i;
	new_edge->v = j;
	tail->right = new_edge;


	return tail->right;



}

//generate and initialize the edge node
edgePointer attach_other_edge(edgePointer tail, int cost, int i, int j)
{
	edgePointer new_edge;
	new_edge = (edgePointer)malloc(sizeof(edge));

	new_edge->cost = cost;
	new_edge->right = NULL;
	new_edge->u = i;
	new_edge->v = j;
	tail->right = new_edge;
	tail = tail->right;

	return tail;



}

//generate and initialize the vertex node
vertexPointer attach_vertex(vertexPointer tail, int value_of_vertex)
{
	vertexPointer new_vertex;
	new_vertex = (vertexPointer)malloc(sizeof(vertex));
	new_vertex->right = NULL;
	new_vertex->down = NULL;
	new_vertex->value = value_of_vertex;
	tail->down = new_vertex;
	tail = new_vertex;

	return tail;



}

//將edge掛上所屬的vertex  各vertex間也以指標連接  形成adjacency list
void link_the_edges(vertexPointer head, const int read_in[][vertexNUM])
{


	edgePointer next_edge;//to point to the newly created edge.

	int i = 0;
	for (vertexPointer next_vertex = head; next_vertex != NULL || i < vertexNUM; next_vertex = next_vertex->down, i++)
	{

		next_edge = attach_first_edge(next_vertex, read_in[i][0], i, 0);

		for (int j = 1; j < vertexNUM; j++)
		{//attach  other edges behind the previous edge.
			next_edge = attach_other_edge(next_edge, read_in[i][j], i, j);
		}


	}



}



unsigned total_edge_NUM;

vertexPointer change_the_link(vertexPointer v, int smaller_vertex, int larger_vertex)
{
	vertexPointer leader = v;

	while (leader->value != smaller_vertex)
	{
		leader = leader->down;
	}

	vertexPointer subordinate = v;
	vertexPointer parent_of_subordinate = NULL;
	while (subordinate->value != larger_vertex)
	{
		parent_of_subordinate = subordinate;
		subordinate = subordinate->down;
	}

	edgePointer last_edge_in_list = leader->right;
	while (last_edge_in_list->right != NULL)
	{
		last_edge_in_list = last_edge_in_list->right;
	}

	last_edge_in_list->right = subordinate->right;
	parent_of_subordinate->down = subordinate->down;
	vertexPointer the_parent_of_current_vertex = parent_of_subordinate;
	delete subordinate ;
	//free( subordinate);

	return the_parent_of_current_vertex;
}







//加入合法的minimum cost edge 加入spanning tree  並將該edge兩端的vertex注記為屬於同一棵樹
vertexPointer add_min_cost_tree_edge(vertexPointer v, edgePointer min_cost_edge, int joined_party[], bool min_cost_spanning_tree[vertexNUM][vertexNUM])
{
	
	int smaller_vertex = joined_party[ min_cost_edge->u];
	int larger_vertex = joined_party[ min_cost_edge->v];
		if (larger_vertex < smaller_vertex)
		{
			smaller_vertex = joined_party[min_cost_edge->v];
			larger_vertex = joined_party[min_cost_edge->u];
		}

		joined_party[larger_vertex] = joined_party[smaller_vertex];

		//display that there is an edge between vertex u and vertex v.
		min_cost_spanning_tree[min_cost_edge->u][min_cost_edge->v] = 1;
		min_cost_spanning_tree[min_cost_edge->v][min_cost_edge->u] = 1;

		//change the link
		vertexPointer the_parent_of_current_vertex = change_the_link(v, smaller_vertex, larger_vertex);
		total_edge_NUM++;//add one edge to the min cost spanning tree.
	


		return the_parent_of_current_vertex;



}

//check whether the vertex on each side of the edge is in the different tree or not.
bool whether_different_group(edgePointer forward, int joined_party[])
{
	//test if the two vertex of the edge are in the different group
	if (joined_party[forward->u] != joined_party[forward->v])
		return 1;

	else
		return 0;

}

//find the minimum cost edge of the current vertex's.
edgePointer get_min_cost_edge(edgePointer forward, edgePointer min_cost_edge)
{
	if (forward->cost < min_cost_edge->cost)
		return forward;
	else
		return min_cost_edge;


}

//implement the Sollin algorithm
void Sollin(vertexPointer v, int joined_party[], bool min_cost_spanning_tree[vertexNUM][vertexNUM])
{
	vertexPointer go_down = v;//to go down the vertex node.
	edgePointer forward = NULL;//go through all the vertex's participated edge.
	edgePointer min_cost_edge = NULL;//record the minimum cost edge.
	unsigned times ;

	while (go_down != NULL)//go to next vertex.
	{

		forward = go_down->right;
		times = 1;
		
		while (forward != NULL)//go to the last edge.
		{

			

				if (forward->cost != -1)
				{
					if (times == 1)
					{
						min_cost_edge = forward;
						times++;
					}


					bool legal_edge_of_spanning_tree = whether_different_group(forward, joined_party);
					
					if (legal_edge_of_spanning_tree)
						min_cost_edge = get_min_cost_edge(forward, min_cost_edge);

				}
				//parent_of_min_cost_edge = forward;

			
			forward = forward->right;
		}
		
		vertexPointer the_parent_of_current_vertex = add_min_cost_tree_edge(v, min_cost_edge, joined_party, min_cost_spanning_tree);
		go_down = the_parent_of_current_vertex;

		go_down = go_down->down;//go to the next vertex node.


		//if (go_down->down == NULL)
		//break;
	}
}

//print out the minimum cost spanning tree made with Sollin
void print_out_min_cost_spanning_tree(bool min_cost_spanning_tree[][vertexNUM])
{
	for (int i = 0; i < vertexNUM; i++)
	{
		for (int j = 0; j < vertexNUM; j++)
		{
			cout << min_cost_spanning_tree[i][j] << " ";
		}

		cout << endl;
	}


}

int _tmain(int argc, _TCHAR* argv[])
{
	/*read in the adjacency matrix from file */
	int read_in[vertexNUM][vertexNUM];//to store the input from the file.

	read_input_from_file(read_in);//read in the input from the file and store in the 2D array "read_in".


	/*create the first vertex list of the adjacency list*/
	vertexPointer v, tail;
	v = (vertexPointer)malloc(sizeof(vertex));
	v->right = NULL;
	v->down = NULL;
	v->value = 0;

	tail = attach_vertex(v, 1);

	for (int i = 2; i < vertexNUM; i++)
	{
		tail = attach_vertex(tail, i); //attach other head node to the back of the head node list.
	}


	/*create the the edge list of adjacency list*/

	link_the_edges(v, read_in);



	/*to avoid the multiple copies of edges and cycle's generation.*/
	int joined_party[vertexNUM];//to store the  party which each vertex joins in. 
	//initialize the all the vertex's joined party as the vertex itself.
	for (int i = 0; i < vertexNUM; i++)
	{
		joined_party[i] = i;
	}


	


	/*show the minimun cost spanning tree*/

	//initialize the output minimum cost spanning tree.
	bool min_cost_spanning_tree[vertexNUM][vertexNUM];
	for (int i = 0; i < vertexNUM; i++)
	{
		for (int j = 0; j < vertexNUM; j++)
		{
			min_cost_spanning_tree[i][j] = 0;
		}
	}

	while (total_edge_NUM < vertexNUM - 1)
	{
		Sollin(v, joined_party, min_cost_spanning_tree);
	}

	print_out_min_cost_spanning_tree(min_cost_spanning_tree);


	

	system("pause");


	return 0;
}



