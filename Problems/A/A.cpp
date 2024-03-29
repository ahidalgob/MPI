#include <assert.h>
#include <mpi.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <vector>
#include <algorithm>

using namespace std;

#define RANGE_TAG 0
#define ANSWER_TAG 1
#define TARGET_TAG 2

int processBomb(int val, int P){
	if(val>=0) return max(0,val-P);
	else return min(0,val+P);
}


void slave_no_bcast(bool bs){
	int buf[6];

	// receives the range
	int N, Y1, Y2;
	MPI_Recv(buf, 3, MPI_INT, 0, RANGE_TAG, MPI_COMM_WORLD, NULL);
	N=buf[0]; Y1=buf[1]; Y2=buf[2];

	// receives the targets, in no bcast version only the corresponding targets are received.
	vector<int> tx, ty, tval, tinival;
	while(true){
		MPI_Recv(buf, 3, MPI_INT, 0, TARGET_TAG, MPI_COMM_WORLD, NULL);
		if(buf[0]==-1) break;
		assert(buf[1] >= Y1);
		assert(buf[1] < Y2);
		tx.push_back(buf[0]);
		ty.push_back(buf[1]);
		tval.push_back(buf[2]);
	}
	tinival = tval;

	// for binsearch we sort the targets according to x
	vector<pair<int, int> > ord;
	if(bs){
		for(int i=0; i<(int)tx.size(); i++) ord.emplace_back(tx[i], i);
		sort(ord.begin(), ord.end());
	}

	// receives all bombs
	vector<int> bx1, bx2, by1, by2, p;
	while(true){
		MPI_Bcast(buf, 4, MPI_INT, 0, MPI_COMM_WORLD);
		if(buf[0]==-1) break;
		int BX=buf[0], BY=buf[1], R=buf[2], P=buf[3];
		int BX1 = max(0,BX-R), BY1 = max(Y1,BY-R);
		int BX2 = min(N, BX+R+1), BY2 = min(Y2, BY+R+1);

		if(BY2 <= BY1) continue;
		bx1.push_back(BX1);
		bx2.push_back(BX2);
		by1.push_back(BY1);
		by2.push_back(BY2);
		p.push_back(P);
	}

	for(int bi=0; bi < bx1.size(); bi++){
		int BX1 = bx1[bi], BX2 = bx2[bi], BY1 = by1[bi], BY2 = by2[bi], P = p[bi];
		// if bs we iterate over only the ones with correct x coordinates
		if(!bs){
			for(int i=0; i<(int)tx.size(); i++){
				if(!(BX1 <= tx[i] && tx[i] < BX2)) continue;
				if(!(BY1 <= ty[i] && ty[i] < BY2)) continue;
				tval[i] = processBomb(tval[i], P);
			}
		}else{
			int i = distance(ord.begin(), lower_bound(ord.begin(), ord.end(), pair<int,int>(BX1,-1)));
			for(; i<(int)ord.size() && ord[i].first < BX2; i++){
				int id = ord[i].second;
				if(!(BY1 <= ty[id] && ty[id] < BY2)) continue;
				tval[id] = processBomb(tval[id], P);
			}
		}
	}

	for(int i=0; i<6; i++) buf[i]=0;
	for(int i=0; i<(int)tx.size(); i++){
		if(tinival[i] < 0){
			if(tval[i]==0) buf[0]++;
			else if(tval[i] != tinival[i]) buf[1]++;
			else buf[2]++;
		}else{
			if(tval[i]==0) buf[3]++;
			else if(tval[i] != tinival[i]) buf[4]++;
			else buf[5]++;
		}
	}

	MPI_Send(buf, 6, MPI_INT, 0, ANSWER_TAG, MPI_COMM_WORLD);

	int id;
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	printf("Slave %d finished.\n",id);
}

void master_nobcast(){
	struct timeval t0, tf, t;
	assert (gettimeofday (&t0, NULL) == 0);

	int nSlaves;
	MPI_Comm_size (MPI_COMM_WORLD, &nSlaves);
	nSlaves--;

	int N, T;
	scanf("%d %d", &N, &T);

	// slave i ( i in [0, nSlaves) ) has range [i*N/nSlaves, (i+1)*N/nSlaves)
	vector<int> starts;
	for(int i=0; i<nSlaves; i++) starts.push_back(i*N/nSlaves);
	starts.push_back(N); // centinel


	// (0) Send range to each slave
	printf("Sending ranges to the %d slaves.\n",nSlaves);
	int msg[3];
	msg[0] = N;
	for(int i=0; i<nSlaves; i++){
		msg[1] = starts[i];
		msg[2] = starts[i+1];

		MPI_Send(msg, 3, MPI_INT, i+1, RANGE_TAG, MPI_COMM_WORLD);
	}

	// (1) Send the targets to relevant slaves
	printf("Sending the targets to the relevant slaves.\n",nSlaves);
	int tar[3];
	for(int i=0; i<T; i++){
		scanf("%d %d %d",&tar[0],&tar[1],&tar[2]);

		int j = distance(starts.begin(), upper_bound(starts.begin(), starts.end(), tar[1])) - 1;
		MPI_Send(tar, 3, MPI_INT, j+1, TARGET_TAG, MPI_COMM_WORLD);

	}
	// end of targets
	tar[0] = -1;
	for(int i=0; i<nSlaves; i++)
		MPI_Send(tar, 1, MPI_INT, i+1, TARGET_TAG, MPI_COMM_WORLD);



	// (2) Send the bombs to all the slaves
	printf("Sending the bombs to the slaves.\n",nSlaves);
	int B; scanf("%d",&B);
	int bomb[4];
	for(int i=0; i<B; i++){
		scanf("%d %d %d %d",&bomb[0],&bomb[1],&bomb[2],&bomb[3]);
		//int X, Y, R, P; scanf("%d %d %d %d",&X,&Y,&R,&P);
		//int X1 = max(0,X-R), Y1 = max(0,Y-R);
		//int X2 = min(N-1, X+R), Y2 = min(N-1, Y+R);
		//bomb[0]=X; bomb[1]=Y; bomb[2]=R; bomb[3]=P;
		MPI_Bcast(bomb, 4, MPI_INT, 0, MPI_COMM_WORLD);
	}
	bomb[0] = -1;
	MPI_Bcast(bomb, 1, MPI_INT, 0, MPI_COMM_WORLD);


	printf("Waiting for slaves\n",nSlaves);
	int ans[6] = {0,0,0,0,0,0};
	int buf[6];
	for(int i=0; i<nSlaves; i++){
		MPI_Recv(buf, 6, MPI_INT, i+1, ANSWER_TAG, MPI_COMM_WORLD, NULL);
		for(int j=0; j<6; j++) ans[j]+=buf[j];
	}

	printf("Military Targets totally destroyed: %d\n", ans[0]);
	printf("Military Targets partially destroyed: %d\n", ans[1]);
	printf("Military Targets not affected: %d\n", ans[2]);
	printf("Civilian Targets totally destroyed: %d\n", ans[3]);
	printf("Civilian Targets partially destroyed: %d\n", ans[4]);
	printf("Civilian Targets not affected: %d\n", ans[5]);

	assert (gettimeofday (&tf, NULL) == 0);
	timersub(&tf, &t0, &t);
	printf ("total time = %ld,%03ld\n", t.tv_sec, t.tv_usec/1000);

}

void slave(bool bs){
	int buf[6];

	int N, Y1, Y2;
	MPI_Recv(buf, 3, MPI_INT, 0, RANGE_TAG, MPI_COMM_WORLD, NULL);
	N=buf[0]; Y1=buf[1]; Y2=buf[2];

	vector<int> tx, ty, tval, tinival;
	while(true){
		MPI_Bcast(buf, 3, MPI_INT, 0, MPI_COMM_WORLD);
		if(buf[0]==-1) break;
		if(buf[1] < Y1) continue;
		if(buf[1] >= Y2) continue;
		tx.push_back(buf[0]);
		ty.push_back(buf[1]);
		tval.push_back(buf[2]);
	}
	tinival = tval;

	vector<pair<int, int> > ord;
	if(bs){
		for(int i=0; i<(int)tx.size(); i++) ord.emplace_back(tx[i], i);
		sort(ord.begin(), ord.end());
	}

	vector<int> bx1, bx2, by1, by2, p;
	while(true){
		MPI_Bcast(buf, 4, MPI_INT, 0, MPI_COMM_WORLD);
		if(buf[0]==-1) break;
		int BX=buf[0], BY=buf[1], R=buf[2], P=buf[3];
		int BX1 = max(0,BX-R), BY1 = max(Y1,BY-R);
		int BX2 = min(N, BX+R+1), BY2 = min(Y2, BY+R+1);

		if(BY2 <= BY1) continue;
		bx1.push_back(BX1);
		bx2.push_back(BX2);
		by1.push_back(BY1);
		by2.push_back(BY2);
		p.push_back(P);
	}

	for(int bi=0; bi < bx1.size(); bi++){
		int BX1 = bx1[bi], BX2 = bx2[bi], BY1 = by1[bi], BY2 = by2[bi], P = p[bi];

		if(!bs){
			for(int i=0; i<(int)tx.size(); i++){
				if(!(BX1 <= tx[i] && tx[i] < BX2)) continue;
				if(!(BY1 <= ty[i] && ty[i] < BY2)) continue;
				tval[i] = processBomb(tval[i], P);
			}
		}else{
			int i = distance(ord.begin(), lower_bound(ord.begin(), ord.end(), pair<int,int>(BX1,-1)));
			for(; i<(int)ord.size() && ord[i].first < BX2; i++){
				int id = ord[i].second;
				if(!(BY1 <= ty[id] && ty[id] < BY2)) continue;
				tval[id] = processBomb(tval[id], P);
			}
		}
	}

	for(int i=0; i<6; i++) buf[i]=0;
	for(int i=0; i<(int)tx.size(); i++){
		if(tinival[i] < 0){
			if(tval[i]==0) buf[0]++;
			else if(tval[i] != tinival[i]) buf[1]++;
			else buf[2]++;
		}else{
			if(tval[i]==0) buf[3]++;
			else if(tval[i] != tinival[i]) buf[4]++;
			else buf[5]++;
		}
	}

	MPI_Send(buf, 6, MPI_INT, 0, ANSWER_TAG, MPI_COMM_WORLD);

	int id;
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	printf("Slave %d finished.\n",id);
}

void master(){
	struct timeval t0, tf, t;
	assert (gettimeofday (&t0, NULL) == 0);

	int nSlaves;
	MPI_Comm_size (MPI_COMM_WORLD, &nSlaves);
	nSlaves--;

	int N, T;
	scanf("%d %d", &N, &T);

	// slave i ( i in [0, nSlaves) ) has range [i*N/nSlaves, (i+1)*N/nSlaves)
	vector<int> starts;
	for(int i=0; i<nSlaves; i++) starts.push_back(i*N/nSlaves);
	starts.push_back(N); // centinel


	// (0) Send range to each slave
	printf("Sending ranges to the %d slaves.\n",nSlaves);
	int msg[3];
	msg[0] = N;
	for(int i=0; i<nSlaves; i++){
		msg[1] = starts[i];
		msg[2] = starts[i+1];

		MPI_Send(msg, 3, MPI_INT, i+1, RANGE_TAG, MPI_COMM_WORLD);
	}

	// (1) Send the targets to all the slaves
	printf("Broadcasting the targets to ALL the slaves.\n",nSlaves);
	int tar[3];
	for(int i=0; i<T; i++){
		scanf("%d %d %d",&tar[0],&tar[1],&tar[2]);
		MPI_Bcast(tar, 3, MPI_INT, 0, MPI_COMM_WORLD);
	}
	// end of targets
	tar[0] = -1;
	MPI_Bcast(tar, 1, MPI_INT, 0, MPI_COMM_WORLD);


	// (2) Send the bombs to all the slaves
	printf("Sending the bombs to the slaves.\n",nSlaves);
	int B; scanf("%d",&B);
	int bomb[4];
	for(int i=0; i<B; i++){
		scanf("%d %d %d %d",&bomb[0],&bomb[1],&bomb[2],&bomb[3]);
		//int X, Y, R, P; scanf("%d %d %d %d",&X,&Y,&R,&P);
		//int X1 = max(0,X-R), Y1 = max(0,Y-R);
		//int X2 = min(N-1, X+R), Y2 = min(N-1, Y+R);
		//bomb[0]=X; bomb[1]=Y; bomb[2]=R; bomb[3]=P;
		MPI_Bcast(bomb, 4, MPI_INT, 0, MPI_COMM_WORLD);
	}
	bomb[0] = -1;
	MPI_Bcast(bomb, 1, MPI_INT, 0, MPI_COMM_WORLD);


	printf("Waiting for slaves\n",nSlaves);
	int ans[6] = {0,0,0,0,0,0};
	int buf[6];
	for(int i=0; i<nSlaves; i++){
		MPI_Recv(buf, 6, MPI_INT, i+1, ANSWER_TAG, MPI_COMM_WORLD, NULL);
		for(int j=0; j<6; j++) ans[j]+=buf[j];
	}

	printf("Military Targets totally destroyed: %d\n", ans[0]);
	printf("Military Targets partially destroyed: %d\n", ans[1]);
	printf("Military Targets not affected: %d\n", ans[2]);
	printf("Civilian Targets totally destroyed: %d\n", ans[3]);
	printf("Civilian Targets partially destroyed: %d\n", ans[4]);
	printf("Civilian Targets not affected: %d\n", ans[5]);

	assert (gettimeofday (&tf, NULL) == 0);
	timersub(&tf, &t0, &t);
	printf ("total time = %ld,%03ld\n", t.tv_sec, t.tv_usec/1000);

}


int main( int argc, char *argv[] ) {
	setbuf (stdout, NULL);
	bool binsearch = false, bcast = false;
	for(int i=1; i<argc; i++){
		if(strcmp(argv[i], "--binsearch")==0) binsearch=true;
		else if(strcmp(argv[i], "--bcast")==0) bcast=true;
		else{
			printf("\nFlag not recognized\nPossible flags: --bcast --binsearch");
			return 0;
		}
	}
	int id;
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	if(bcast){
		if(id==0) master();
		else slave(binsearch);
	}else{
		if(id==0) master_nobcast();
		else slave_no_bcast(binsearch);
	}

	MPI_Finalize();
	return 0;
}
