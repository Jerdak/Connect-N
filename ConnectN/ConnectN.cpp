//#include "EventTimer.h"
#include "ConnectBoard.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>

#ifdef _WIN32
	#include <windows.h>
	#include "EventTimer.h"

	#define TESTTIME 100	//100ms
#else 

	#include <cstdlib>
	
	#include <unistd.h>
	#include <signal.h>

	#define Sleep(A) usleep(A)
	#define TESTTIME 1	//alarm doesn't support milliseconds, this is the smallest test val.
#endif

//macros, DO NOT USE FUNCTIONS IN MACROS.  Ex, do not do: MAX(f1(),f2(), this expands to f1()>f2()?f1():f2() calling both functions twice.
#define MAX(A,B) A>B?A:B
#define MIN(A,B) A<B?A:B

using namespace std;

#pragma region Prototypes
vector<int> GetMoves();
int minimax(int move,int maxDepth, bool bMax, bool bDebug=false);
int minValue(int id, int depth, int a,int b);
int maxValue(int id, int depth, int a,int b);
int alphabeta(int depth,bool bDebug=false);


#pragma endregion 


//Kludgy way of tracking node id.
int nCalls = 0;
  
//Helper flag to turn on certain portions of the debugging messages at will.
//bool bDebug = false;

//The one and only game board.  Trear 'er kindly.
ConnectBoard board;

bool AddMove(int move, bool bPlayer){
	return board.AddMove(move,bPlayer);
}

bool RemoveMove(int move, bool bPlayer){
	board.RemoveMove(move,bPlayer);
	return true;
}

int EvaluateBoard(bool bPlayer){
	return board.GetValue(bPlayer);
}
int EvaluateBoard(){
	return board.GetValue(true) - board.GetValue(false);
}

/** Return a vector of valid moves on the current board
	@notes
		Only valid moves, that is moves in columns with empty
		space, are returned.  
*/
std::vector<int> GetMoves(){
	std::vector<int> ret;
	for(int i = 0; i < board.get_width(); i++){
		if(board.ColumnCount(i)<board.get_height()){
			ret.push_back(i);
		}
	}

	return ret;
}

/**	Minimax algorithm
	@inputs
		move - integer value of the column where we are adding a new token.
		maxDepth - integer depth limit.  Decrements during each recursive until it reaches 0.
		bMax - bool flag specifying current player.  True = us, False = enemy
*/
int minimax(int move,int maxDepth, bool bMax, bool bDebug){
	int id = nCalls;
	nCalls++;
	//fprintf(stderr,"Add move: %d\n",move);
	//update the board with our next move
	bool bMoveAdded = AddMove(move,bMax);

	if(!bMoveAdded){
		fprintf(stderr,"  - not added\n");
	}
	int val = EvaluateBoard(true);
	
	if(val>=1000){
		if(bMoveAdded)RemoveMove(move,bMax);
		if(bDebug)fprintf(stderr,"NodeMini[%d]: %d.  Added %d to %d\n",id,val,move,bMax);
		if(bMax)return val;
		else return -val;
	}

	//terminal nodes return some value T, the 
	if(maxDepth <= 0 || !bMoveAdded){
		if(bMoveAdded)RemoveMove(move,bMax);
		if(bDebug)fprintf(stderr,"NodeMini[%d]: %d.  Added %d to %d\n",id,val,move,bMax);
		return val;
	}

	//get next set of valid moves based on current board.
	std::vector<int> validMoves = GetMoves();
	if(validMoves.size() <= 0){
		if(bMoveAdded)RemoveMove(move,bMax);
		if(bDebug)fprintf(stderr,"NodeMini[%d]: %d.  Added %d to %d\n",id,val,move,bMax);
		return val;

	}
	//set to wee small
	int result = -1e6;

	//crawl possible moves

	for(size_t i=0; i < validMoves.size(); i++){
		int v = -minimax(validMoves[i],maxDepth-1,!bMax);
		if(bDebug)fprintf(stderr,"  - v: %d\n",v);
		result = MAX(result,v);
	}
	
	if(bMoveAdded)RemoveMove(move,bMax);
	if(bDebug)fprintf(stderr,"NodeMini[%d]: %d.  Added %d to %d\n",id,result,move,bMax);
	
	return result;
}


int action = -1;
int nNode = 1;
bool bTimerPop = false;

/**	Return the max value of the current board
	@inputs
		nodeID - integer identity for the current alphabeta tree node.
		depth - integer depth limit.  Decrements during each recursive until it reaches 0.
		a - alpha  :)
		b - beta   :)
*/
int maxValue(int nodeID, int depth,int a,int b){
	//Timer pop, unwind stack
	if(bTimerPop)return -1;

	int v1 = EvaluateBoard(true);
	int v2 = EvaluateBoard(false);
	int ret = v1 - v2;
	std::vector<int> moves = GetMoves();

	//assume values > 1000 are end game states, avoid drilling down further.
	if(v1>=1000 || v2>=1000 || depth <=0 || moves.size()<= 0){
		return ret;
	}
	
	int v = -1e6;
	for(size_t i = 0; i < moves.size(); i++){
			int nextNodeID = nNode++;
			AddMove(moves[i],true);
			//fprintf(stream,"\tn%d -- n%d;\n",nodeID,nextNodeID);
			int tmp = minValue(nextNodeID,depth-1,a,b);
			//fprintf(stderr,"i: %d tmp: %d  v:%d  move: %d\n",i,tmp,v,moves[i]);
			//fprintf(stream,"\tn%d [label=\"%d\"];\n",nextNodeID,tmp);
			if(tmp > v && nodeID == 0){
				action = moves[i];
			}
			v = MAX(v,tmp);
			//fprintf(stderr,"Player1 move[%d]: %d\n",moves[i],v);
			//board.Display(true);
			RemoveMove(moves[i],true);

			if(bTimerPop)return -1;
			//fprintf(stderr,"Move[%d]  Depth: %d  Valu: %d\n",moves[i],depth,tmp);	
			if(v>b){
				return v;
			}
			a = MAX(a,v);
	}
	return v;
}

/**	Return the min value of the current board
	@inputs
		nodeID - integer identity for the current alphabeta tree node.
		depth - integer depth limit.  Decrements during each recursive until it reaches 0.
		a - alpha  :)
		b - beta   :)
*/
int minValue(int nodeID, int depth, int a,int b){
	//Timer pop, unwind stack
	if(bTimerPop)return -1;

	int v1 = EvaluateBoard(true);
	int v2 = EvaluateBoard(false);
	int ret = v1 - v2;
	std::vector<int> moves = GetMoves();

	//assume values > 1000 are end game states, avoid drilling down further.
	if(v1>=1000 || v2>=1000 || depth <=0 || moves.size()<= 0){
		return ret;
	}
	
	
	int v = 1e6;
	for(size_t i = 0; i < moves.size(); i++){
			int nextNodeID = nNode++;
			//fprintf(stderr,"Move[%d]  Depth: %d\n",moves[i],depth);
			//fprintf(stream,"\tn%d -- n%d;\n",nodeID,nextNodeID);
			AddMove(moves[i],false);
			int tmp = MIN(v,maxValue(nextNodeID,depth-1,a,b));
			
			//fprintf(stream,"\tn%d [label=\"%d,%d\"];\n",nextNodeID,nextNodeID,tmp);
			v = MIN(v,tmp);
			
			RemoveMove(moves[i],false);
			if(bTimerPop)return -1;

			if(v<=a){
				return v;
			}
			b = MAX(b,v);
	}

	return v;
}

/**	Minimax w/ alphabeta pruning
	@inputs
		depth - integer max depth to search
		bDebug - bool flag telling the code if it should dump more verbose log info.
	@notes
		alphabeta is augmented with iterative (or progressive) deepening. Each depth d,
		1 <= d < n, is evaluated by depth limting alphabeta.  This gives the full
		breadth search at all depths up to d.  If our AI runs out of time,
		it can fall back to the last completed depth.
*/
int alphabeta(int depth,bool bDebug){
	int a = -1e6;
	int b = 1e6;
	int v = -1e6;
	int finalAction = 1;
	for(int i = 1; i < depth; i++){
		int tmpv = maxValue(0,i,a,b);

		//If timer popped during depth search, use last last completed depth's action(TODO, we might emit our answer in the timer_handler() to avoid stack unwind)
		if(bTimerPop)return finalAction;
		finalAction=action;

		if(bDebug)fprintf(stderr,"Depth: %d tmpv: %d  action: %d  finalaction: %d\n",i,tmpv,action,finalAction);
	}
	return finalAction;
}


#pragma region Timer_Handling
#ifdef _WIN32
Timers::EventTimer popTest("PopTest");
MMRESULT timerID;
void CALLBACK timer_handler(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2){
	popTest.AfterEvent();
	bTimerPop = true;
	//fprintf(stderr,"Timer Popped: %f\n",popTest.GetTimeSinceLastEvent());
}
#else
void timer_handler(int sig){
	bTimerPop = true;
	fprintf(stderr,"Timer popped\n");
}
#endif
void setTimer(int timeLimit){
	bTimerPop = false;

#ifdef _WIN32
	//fprintf(stderr,"Set Timer\n");
	popTest.BeforeEvent();
	timeKillEvent(timerID);
	//Make sure convert timeLimit to seconds.  Since *Nix alarm() only handles seconds, we do the same
	timerID = timeSetEvent(timeLimit*1000, 0, timer_handler, 0, TIME_ONESHOT );
	//fprintf(stderr,"Timer should be going: %d\n",timerID);
#else
	//fprintf(stderr,"Set timer\n");
	signal(SIGALRM,timer_handler);
	alarm(timeLimit);
#endif
}

#pragma endregion 

///Test that time pops when it should
void timerTest(){
	setTimer(TESTTIME);
	while(!bTimerPop){
		Sleep(1);
	}
	setTimer(TESTTIME);
	while(!bTimerPop){
		Sleep(1);
	}
	setTimer(TESTTIME);
	while(!bTimerPop){
		Sleep(1);
	}
	setTimer(TESTTIME);
	while(!bTimerPop){
		Sleep(1);
	}
}

/** Play Connect-N
	@notes
		This is the official play loop as outlined
		in the homework documentation.
*/
void play(){
	int width,height,n,player,timeLimit, move;

	//spit out a name, you have 10 seconds to play with here.
	//printf("Joshua\n");
	cout << "Joshua" << endl;

	//parse width/height/n of Connect-n/turn (0 = me, 1 = them)
	scanf("%d %d %d %d %d",&width,&height,&n,&player,&timeLimit);
	
	//!!CS534 referee is emitting width and height in reverse order, swap them or risk producing invalid choices.
	{ 
		int tmp = width;
		width = height;
		height = width;

		timeLimit = timeLimit - 1;
	}

	//fprintf(stderr,"loading board.  w: %d  h: %d  n: %d  player: %d timelime: %d\n",width,height,n,player,timeLimit);
	

	board.set_width(width);
	board.set_height(height);
	board.set_n(n);

	bool is_me = (player==0)?true:false;
	
	//'Killer move', steal middle of the table to dominate as much of the lower row as possible.
	if(is_me){cout << width/2 << endl; is_me = !is_me;AddMove(width/2,true);}

	while(true){
		if(is_me){
			setTimer(timeLimit);

			//depth limited alphabeta (default 15 deep)
			int move = alphabeta(15);
			AddMove(move,true);
			cout << move << endl;			
			//printf("my move: %d\n",move);
		} else {
			int move;

			//Play nice with opponent's resources, idle in scanf until they give a response.
			scanf("%d",&move);
			if(move < 0){
				break;
			}
			AddMove(move,false);
		}
		board.Display(true);
		is_me = !is_me;
	}
}
int main(int argc, char* argv[]){
	//Call main game loop
	play();
	return 1;
}