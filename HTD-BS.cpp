#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include "read_data.cpp"
#include "HelperFunc.h"
using namespace std;






int main(int argc, char *argv[])
{
	clock_t start;
	clock_t run_start;
	int progress;

	string graphFile = "Dataset/Partial/40-unique-threads-ask-ubuntu.txt";

	// Read data
	start = clock();
	vector< vector<int> > node2hyperedge;
	vector< vector<int> > hyperedge2node;
	vector< unordered_set<int> > hyperedge2node_set;
	read_data(graphFile, node2hyperedge, hyperedge2node, hyperedge2node_set);

	int V = node2hyperedge.size(), E = hyperedge2node.size();
	cout << "# of nodes: " << V << '\n';
	cout << "# of hyperedges: " << E << '\n';
	cout << "Reading data done: "
		<< (double)(clock() - start) / CLOCKS_PER_SEC << " sec" << endl;
	cout << "------------------------------------------" << endl << endl;


	// Make adjacency list
	start = clock(); run_start = clock();
	hyperedge2node.resize(E); hyperedge2node_set.resize(E);
	vector< vector<int > > hyperedge_adj;
	vector< unordered_map<int, int> > hyperedge_inter;
	hyperedge_adj.resize(E);
	hyperedge_inter.resize(E);
	vector<long long> upd_time(E, -1LL);
		
	for (int hyperedge_a = 0; hyperedge_a < E; hyperedge_a++){
		long long l_hyperedge_a = (long long)hyperedge_a;
		for (const int &node: hyperedge2node[hyperedge_a]){
			for (const int &hyperedge_b: node2hyperedge[node]){
				if (hyperedge_b == hyperedge_a) continue;
				if ((upd_time[hyperedge_b] >> 31) ^ hyperedge_a){
					hyperedge_adj[hyperedge_b].push_back(hyperedge_a);
				}
			}
		}
	}

	
	removeDuplicatesFromHyperedgeAdj(hyperedge_adj);
	vector<unordered_set<int>> hyperedge_adjset = convertToSet(hyperedge_adj);
	vector<vector<int>> hyperedge_adj_filtered=convertToVectorFilteredByIndex(hyperedge_adj);

	cout << "Adjacency list construction done: "
		<< (double)(clock() - start) / CLOCKS_PER_SEC << " sec" << endl;
	cout << "------------------------------------------" << endl << endl;



	start = clock(); run_start = clock();
	vector<int> hyperedge_sup(hyperedge_adj.size(), 0);

	
	for(int i=0; i<hyperedge_adj_filtered.size();i++){

		const vector<int>& neighbors = hyperedge_adj_filtered[i];
		int node_1=i;
        for (int a = 0; a < neighbors.size(); ++a) {
            int node_2 = neighbors[a];
            const vector<int>& neighbors_j = hyperedge_adj_filtered[node_2];

  
            int b = a + 1;  
            int ptr_j = 0;  

            while (true) {
                if (b >= neighbors.size() || ptr_j >= neighbors_j.size()) break;
                int node_3 = neighbors[b];
                int node_4 = neighbors_j[ptr_j];
                if (node_3 == node_4) {
                    hyperedge_sup[node_1]+=1;
					hyperedge_sup[node_2]+=1;
					hyperedge_sup[node_3]+=1;
                    ++b;
                    ++ptr_j;
                } else if (node_3 < node_4) {
                    ++b;
					open_hyper_num+=1;
					open_hyper_num2+=1;
                } else {
                    ++ptr_j;
					open_hyper_num2+=1;
                }
            }

		}
	}

	
	
	
	cout << "support list construction done: "
		<< (double)(clock() - start) / CLOCKS_PER_SEC << " sec" << endl;
	cout << "------------------------------------------" << endl << endl;

    start = clock(); run_start = clock();



	vector<unordered_set<int>> Support_List=BuildSupportList(hyperedge_sup);
	std::vector<int> hyperedge_sup_copy = hyperedge_sup;

	int k=3;
    vector<int> Trussness(hyperedge_adj.size(), 0);
	int EndLoop=Support_List[0].size();

    while(EndLoop<E){
        queue<int> current_hyperedges=SetToQueue(Support_List[k-2]);
        unordered_set<int> adjust_list;
        //EndLoop+=current_hyperedges.size();
        while(current_hyperedges.size()!=0){
			int hyperedge_1=current_hyperedges.front();
            hyperedge_sup[hyperedge_1]=-1;
            Trussness[hyperedge_1]=k;
            current_hyperedges.pop();
            adjust_list.insert(hyperedge_1);

			for(int i=0; i<hyperedge_adj[hyperedge_1].size(); i++){
				int hyperedge_2=hyperedge_adj[hyperedge_1][i];
				if(hyperedge_sup[hyperedge_2]==-1) continue;
				
				
				for(int j=0; j<hyperedge_adj[hyperedge_2].size(); j++){
					
					int hyperedge_3=hyperedge_adj[hyperedge_2][j];
					if(hyperedge_3<hyperedge_2 || hyperedge_sup[hyperedge_3]==-1) continue;
					if(!hyperedge_adjset[hyperedge_1].count(hyperedge_3)) continue;

					if(hyperedge_sup[hyperedge_2]==k-1){
						current_hyperedges.push(hyperedge_2);
					}

					if(hyperedge_sup[hyperedge_3]==k-1){
						current_hyperedges.push(hyperedge_3);
					}

					hyperedge_sup[hyperedge_2]-=1;
					hyperedge_sup[hyperedge_3]-=1;
					adjust_list.insert(hyperedge_2);
                    adjust_list.insert(hyperedge_3);

				}
			}
		}
        for (int val : adjust_list) {
            if(hyperedge_sup[val]<=k-2){
                Support_List[hyperedge_sup_copy[val]].erase(val);
                EndLoop+=1;
            }else{
                Support_List[hyperedge_sup_copy[val]].erase(val);
                
                Support_List[hyperedge_sup[val]].insert(val);
				hyperedge_sup_copy[val]=hyperedge_sup[val];
            }
        }
        k=k+1;


    }


    cout << "Trusness list construction done: "
		<< (double)(clock() - start) / CLOCKS_PER_SEC << " sec" << endl;
	cout << "------------------------------------------" << endl << endl;




	return 0;
}
