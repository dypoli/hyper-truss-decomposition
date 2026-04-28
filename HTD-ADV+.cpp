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
#include <queue>
#include <cassert>
#include <bitset>
#include <list>
#include <cstdint>
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
    vector< unordered_set<int> > node2hyperedge_set;
	read_data(graphFile, node2hyperedge, hyperedge2node, hyperedge2node_set);

	


	int V = node2hyperedge.size(), E = hyperedge2node.size();
	cout << "# of nodes: " << V << '\n';
	cout << "# of hyperedges: " << E << '\n';



	// Make adjacency list
	start = clock(); run_start = clock();
	hyperedge2node.resize(E); hyperedge2node_set.resize(E);
	vector<vector<int>> hyperedge_adj;

	hyperedge_adj.resize(E);
	vector<long long> upd_time(E, -1LL);


    // new structure, only for this version
    
    vector<int> Index_signal(E,-1);
    vector<vector<vector<int>>> Edge_node_Edge;
    Edge_node_Edge.resize(E);


    vector<vector<vector<int>>> Edge_node_Edge_count;
    Edge_node_Edge_count.resize(E);



    for(int i=0; i<E; i++){
        Edge_node_Edge[i].resize(hyperedge2node[i].size());
        Edge_node_Edge_count[i].resize(hyperedge2node[i].size());

        for(int j=0; j<hyperedge2node[i].size(); j++){
            Edge_node_Edge[i][j].resize(node2hyperedge[hyperedge2node[i][j]].size(),0);
            Edge_node_Edge_count[i][j].resize(node2hyperedge[hyperedge2node[i][j]].size(),0);

        }
    } 

	

    vector<unordered_map<int, uint32_t>> hyperedge_bitmap(E);


    


	for (int hyperedge_a = 0; hyperedge_a < E; hyperedge_a++){
		long long l_hyperedge_a = (long long)hyperedge_a;
		for (int i=0; i<hyperedge2node[hyperedge_a].size(); i++){
            int node=hyperedge2node[hyperedge_a][i];
			for (int j=0; j<node2hyperedge[node].size(); j++){
                int hyperedge_b=node2hyperedge[node][j];
				if (hyperedge_b == hyperedge_a) continue;
				if ((upd_time[hyperedge_b] >> 31) ^ hyperedge_a){
					hyperedge_adj[hyperedge_b].push_back(hyperedge_a);
					Edge_node_Edge[hyperedge_a][i][j]=hyperedge_b;
                }
			}
		}
	}

   
    
	


	// and convert the adjacent list to set for faster query
	removeDuplicatesFromHyperedgeAdj(hyperedge_adj);
    convertToVectorWithIndex(hyperedge_adj,Index_signal);
    vector<vector<int>> hyperedge_TriangleNum;
    hyperedge_TriangleNum.resize(E);
    for(int i=0; i<E; i++) hyperedge_TriangleNum[i].resize(hyperedge_adj[i].size(),0);

    vector<vector<int>> hyperedge_TriangleNum2;
	hyperedge_TriangleNum2.resize(E);
    for(int i=0; i<E; i++) hyperedge_TriangleNum2[i].resize(hyperedge_adj[i].size(),0);








    // support number counting and build  blooms structure

    start = clock(); run_start = clock();
	vector<int> hyperedge_sup(hyperedge_adj.size(), 0);
	vector< unordered_map<int,vector<int> > > hyperedge_triangles;
	hyperedge_triangles.resize(E);




	vector<int> signal_firstlevel(E, -1);
	vector<int> signal_secondlevel(E, -1);
	
	vector<vector<int>> node_connection(V);
	vector<int> node_signal(V, -1);
	vector<int> node_position(V, 0);
	vector<int> hyperedge_signal_firstlevel(E, -1);
	vector<int> hyperedge_signal_secondlevel(E, -1);
	vector<uint32_t> SingleEdge_bitmap(E);
    vector<uint32_t> SingleEdge_bitmap_reverse(E);
    vector<int> Triangle_count(E, 0);
    vector<unordered_map<int,int>> hyperedge_pair_complex(E);
    vector<unordered_map<int,int>> hyperedge_pair_complex_count(E);


    vector<int> repeat_Num(E,0);
    vector<int> Complex_Num(E,0);
    vector<int> Sparse_Num(E,0);

    vector<int> Same_Num(E,0);
    //vector<int> complex_triangle(E, 0);
	long repeat_count_num=0;

	for(int i=0; i<hyperedge_adj.size();i++){

		const vector<int>& neighbors = hyperedge_adj[i];
		int hyperedge_1=i;

        for (int a = Index_signal[hyperedge_1]; a < neighbors.size(); ++a) {
            int hyperedge_2 = neighbors[a];

            Triangle_count[hyperedge_2]=a;
            const vector<int>& nodes_2 = hyperedge2node[hyperedge_2];

			int p = 0, t = 0;
			SingleEdge_bitmap[hyperedge_2] = 0;
			SingleEdge_bitmap_reverse[hyperedge_2] = 0;
            Same_Num[hyperedge_2] = 0;
			while (p < hyperedge2node[hyperedge_2].size() && t < hyperedge2node[hyperedge_1].size()) {
				int node_3 = hyperedge2node[hyperedge_2][p];
				int node_4 = hyperedge2node[hyperedge_1][t];

				if (node_3 == node_4) {
					SingleEdge_bitmap[hyperedge_2] |= (1U << p);
					SingleEdge_bitmap_reverse[hyperedge_2] |= (1U << t);
                    Same_Num[hyperedge_2]+=1;
					++p; ++t;
				} else if (node_3 < node_4) {

					int cur_node = node_3;
					if (node_signal[cur_node] != hyperedge_1) {
						node_signal[cur_node] = hyperedge_1;
						node_position[cur_node] = 0;
						node_connection[cur_node].clear();
						node_connection[cur_node].push_back(hyperedge_2);
					} else {
						node_connection[cur_node].push_back(hyperedge_2);
					}
					++p;
				} else {

					++t;
				}
			}


			while (p < hyperedge2node[hyperedge_2].size()) {
				int cur_node = hyperedge2node[hyperedge_2][p];
				if (node_signal[cur_node] != hyperedge_1) {
					node_signal[cur_node] = hyperedge_1;
					node_position[cur_node] = 0;
					node_connection[cur_node].clear();
					node_connection[cur_node].push_back(hyperedge_2);
				} else {
					node_connection[cur_node].push_back(hyperedge_2);
				}
				++p;
			}
        }

		


		for (int a = Index_signal[hyperedge_1]; a < neighbors.size(); ++a) {
			int hyperedge_2 = neighbors[a];


			const vector<int>& nodes_2 = hyperedge2node[hyperedge_2];
			for (size_t p = 0; p < hyperedge2node[hyperedge_2].size(); ++p) {

				if (!((SingleEdge_bitmap[hyperedge_2] >> p) & 1)) {
					int cur_node=hyperedge2node[hyperedge_2][p];
					node_position[cur_node]+=1;
					int loc=node_position[cur_node];
					while(loc<node_connection[cur_node].size()){
						int hyperedge_3=node_connection[cur_node][loc];
                        
						if(hyperedge_signal_firstlevel[hyperedge_3]!=hyperedge_1){
							hyperedge_signal_firstlevel[hyperedge_3]=hyperedge_1;
							hyperedge_signal_secondlevel[hyperedge_3]=hyperedge_2;
							hyperedge_signal_secondlevel[hyperedge_3]=hyperedge_2;			
							int common = SingleEdge_bitmap_reverse[hyperedge_2] & SingleEdge_bitmap_reverse[hyperedge_3];
							
							if(common==0){
								hyperedge_sup[hyperedge_1]+=1;
								hyperedge_sup[hyperedge_2]+=1;
								hyperedge_sup[hyperedge_3]+=1;

							}else{
                                repeat_count_num+=1;
                            }
						}else{
							if(hyperedge_signal_secondlevel[hyperedge_3]==hyperedge_2){
								loc+=1;
								continue;
							}else{
								hyperedge_signal_secondlevel[hyperedge_3]=hyperedge_2;									
								int common = SingleEdge_bitmap_reverse[hyperedge_2] & SingleEdge_bitmap_reverse[hyperedge_3];

                                
								if(common==0){			
									hyperedge_sup[hyperedge_1]+=1;
									hyperedge_sup[hyperedge_2]+=1;
									hyperedge_sup[hyperedge_3]+=1;
									
								}else{
                                    repeat_count_num+=1;
                                }
							}
						}
						loc+=1;
					}
				}else{
                    
					int cur_node=hyperedge2node[hyperedge_2][p];
					for(int l=0; l<node2hyperedge[cur_node].size(); l++){
						int hyperedge_3=node2hyperedge[cur_node][l];
                        if(hyperedge_3<=hyperedge_2) continue;
						if(signal_firstlevel[hyperedge_3]==hyperedge_1 && signal_secondlevel[hyperedge_3]==hyperedge_2){
                            if(repeat_Num[hyperedge_3]==-1){
                                repeat_Num[hyperedge_3]=1;
                                hyperedge_pair_complex[hyperedge_1][hyperedge_2]+=1;
                                hyperedge_pair_complex[hyperedge_1][hyperedge_3]+=1;
                                hyperedge_pair_complex[hyperedge_2][hyperedge_3]+=1;
                            }
                            continue;
                        }


                        repeat_Num[hyperedge_3]=-1;
                        
						signal_firstlevel[hyperedge_3]=hyperedge_1;
						signal_secondlevel[hyperedge_3]=hyperedge_2;
						
						hyperedge_sup[hyperedge_1]+=1;
						hyperedge_sup[hyperedge_2]+=1;
						hyperedge_sup[hyperedge_3]+=1;

                        hyperedge_TriangleNum[hyperedge_1][Triangle_count[hyperedge_2]]+=1;
                        hyperedge_TriangleNum[hyperedge_1][Triangle_count[hyperedge_3]]+=1;
						Edge_node_Edge_count[hyperedge_2][p][l]+=1;

					}

				}
			}

		}

	}
    node_connection.clear();
    node_signal.clear();
    node_position.clear();
    hyperedge_signal_firstlevel.clear();
    hyperedge_signal_secondlevel.clear();
    Triangle_count.clear();


    vector<int> counter(E,0);
    for(int i=0; i<E; i++){
        for(int j=0; j<Edge_node_Edge[i].size(); j++){
            for(int k=0; k<Edge_node_Edge[i][j].size(); k++){
                int hyperedge_2=Edge_node_Edge[i][j][k];
                counter[hyperedge_2]+=Edge_node_Edge_count[i][j][k];
            }
            Edge_node_Edge_count[i][j].clear();
            Edge_node_Edge[i][j].clear();
        }
        Edge_node_Edge_count[i].clear();
        Edge_node_Edge[i].clear();
        for(int j=Index_signal[i];j<hyperedge_adj[i].size();j++){
            hyperedge_TriangleNum[i][j]+=counter[hyperedge_adj[i][j]];
            counter[hyperedge_adj[i][j]]=0;
        }
    }
    Edge_node_Edge_count.clear();
    Edge_node_Edge.clear();

	completeSymmetricValues(hyperedge_adj, hyperedge_TriangleNum);
  

	



    

    start = clock(); run_start = clock();

    vector<unordered_set<int>> Support_List=BuildSupportList(hyperedge_sup);
	std::vector<int> hyperedge_sup_copy = hyperedge_sup;
	signal_firstlevel.assign(E, -1);
	signal_secondlevel.assign(E, -1);
	


	int k=3;
    vector<int> Trussness(hyperedge_adj.size(), 0);
	int EndLoop=Support_List[0].size();
	vector<std::list<int>> node2hyperedge_list= convertToListVector(node2hyperedge);
    vector<int> node_removal(V,0);
    vector<int> Edge_removal(E,0);
    vector<int> ready_to_remove(E,0);

    vector<int> count_signal(E,-1);
    repeat_Num.assign(E,0);


    vector<int> fortest(E,0);

    vector<vector<int>> node_connection2(V);
	vector<int> node_signal2(V, -1);
	vector<int> node_position2(V, 0);
    vector<int> hyperedge_signal_firstlevel2(E, -1);
	vector<int> hyperedge_signal_secondlevel2(E, -1);

	while(EndLoop<E){
        queue<int> current_hyperedges=SetToQueue(Support_List[k-2]);
        unordered_set<int> adjust_list;
        while(current_hyperedges.size()!=0){
			int hyperedge_1=current_hyperedges.front();
            hyperedge_sup[hyperedge_1]=-1;
            EndLoop+=1;
            Trussness[hyperedge_1]=k;
            current_hyperedges.pop();
            adjust_list.insert(hyperedge_1);


			

            const vector<int>& neighbors = hyperedge_adj[hyperedge_1];
            int signal=0;
            
            for (int a = 0; a < neighbors.size(); ++a) {
                int remove_num=0;
                int hyperedge_2 = neighbors[a];
                if(hyperedge_sup[hyperedge_2]<0) continue;
                const vector<int>& nodes_2 = hyperedge2node[hyperedge_1];

                if(signal==0){
                    signal=1;
                    int p = 0, t = 0;
                    SingleEdge_bitmap[hyperedge_2] = 0;
                    SingleEdge_bitmap_reverse[hyperedge_2] = 0;

                    while (p < hyperedge2node[hyperedge_1].size() && t < hyperedge2node[hyperedge_2].size()) {
                        int node_3 = hyperedge2node[hyperedge_1][p];
                        int node_4 = hyperedge2node[hyperedge_2][t];

                        if (node_3 == node_4) {

                            SingleEdge_bitmap[hyperedge_2] |= (1U << t);
                            SingleEdge_bitmap_reverse[hyperedge_2] |= (1U << p);
                            remove_num += node_removal[node_3];
                            node_removal[node_3] += 1;
                            ++p;
                            ++t;
                        } 
                        else if (node_3 < node_4) {
                            int cur_node = node_3;
                            node_removal[cur_node] += 1;
                            
                            ++p;
                        } 
                        else {
                            int cur_node = node_4;
                            if (node_signal2[cur_node] != hyperedge_1) {
                                node_signal2[cur_node] = hyperedge_1;
                                node_position2[cur_node] = 0;
                                node_connection2[cur_node].clear();
                                node_connection2[cur_node].push_back(hyperedge_2);
                            } else {
                                node_connection2[cur_node].push_back(hyperedge_2);
                            }
                            ++t;
                        }
                    }

                    while (p < hyperedge2node[hyperedge_1].size()) {
                        int cur_node = hyperedge2node[hyperedge_1][p];
                        node_removal[cur_node] += 1; 
                        ++p;
                    }

                    while (t < hyperedge2node[hyperedge_2].size()) {
                        int cur_node = hyperedge2node[hyperedge_2][t];
                        if (node_signal2[cur_node] != hyperedge_1) {
                            node_signal2[cur_node] = hyperedge_1;
                            node_position2[cur_node] = 0;
                            node_connection2[cur_node].clear();
                            node_connection2[cur_node].push_back(hyperedge_2);
                        } else {
                            node_connection2[cur_node].push_back(hyperedge_2);
                        }
                        ++t;
                    }


                }else{
                    int p = 0, t = 0;
                    SingleEdge_bitmap[hyperedge_2] = 0;
                    SingleEdge_bitmap_reverse[hyperedge_2] = 0;

                    while (p < hyperedge2node[hyperedge_1].size() && t < hyperedge2node[hyperedge_2].size()) {
                        int node_3 = hyperedge2node[hyperedge_1][p];
                        int node_4 = hyperedge2node[hyperedge_2][t];

                        if (node_3 == node_4) {
                            SingleEdge_bitmap[hyperedge_2] |= (1U << t);
                            SingleEdge_bitmap_reverse[hyperedge_2] |= (1U << p);
                            remove_num += node_removal[node_3];
                            remove_num -= 1;
                            ++p;
                            ++t;
                        } 
                        else if (node_3 < node_4) {                    
                            ++p;
                        } 
                        else {
                            int cur_node = node_4;
                            
                            if (node_signal2[cur_node] != hyperedge_1) {
                                node_signal2[cur_node] = hyperedge_1;
                                node_position2[cur_node] = 0;
                                node_connection2[cur_node].clear();
                                node_connection2[cur_node].push_back(hyperedge_2);
                            } else {
                                node_connection2[cur_node].push_back(hyperedge_2);
                            }
                            ++t;
                        }
                    }
                    while (t < hyperedge2node[hyperedge_2].size()) {
                        int cur_node = hyperedge2node[hyperedge_2][t];
                        if (node_signal2[cur_node] != hyperedge_1) {
                            node_signal2[cur_node] = hyperedge_1;
                            node_position2[cur_node] = 0;
                            node_connection2[cur_node].clear();
                            node_connection2[cur_node].push_back(hyperedge_2);
                        } else {
                            node_connection2[cur_node].push_back(hyperedge_2);
                        }   
                        ++t;
                    }

                }

                int addNum=0;
                if(hyperedge_1<hyperedge_2){
                    if (hyperedge_pair_complex_count[hyperedge_1].find(hyperedge_2) != hyperedge_pair_complex_count[hyperedge_1].end()) {
                     
                        addNum=hyperedge_pair_complex_count[hyperedge_1][hyperedge_2];
                    } 
                }else{
                    if (hyperedge_pair_complex_count[hyperedge_2].find(hyperedge_1) != hyperedge_pair_complex_count[hyperedge_2].end()) {
                     
                        addNum=hyperedge_pair_complex_count[hyperedge_2][hyperedge_1];
                    } 
                }

                Edge_removal[hyperedge_2]=hyperedge_TriangleNum[hyperedge_1][a]-remove_num+addNum;
                

            }


            for(int i=0; i<hyperedge_adj[hyperedge_1].size(); i++){
				int hyperedge_2=hyperedge_adj[hyperedge_1][i];
				if(hyperedge_sup[hyperedge_2]<0){
                    continue;
                } 
      

                int boolNum=hyperedge_1<hyperedge_2?hyperedge_pair_complex[hyperedge_1][hyperedge_2]:hyperedge_pair_complex[hyperedge_2][hyperedge_1];
                if(boolNum<=0){
 
                    if((hyperedge_sup[hyperedge_2]-Edge_removal[hyperedge_2])<=k-2 && ready_to_remove[hyperedge_2]==0 && hyperedge_sup[hyperedge_2]>k-2){
                        ready_to_remove[hyperedge_2]=1;
                        current_hyperedges.push(hyperedge_2); 
                    }
                    
                    hyperedge_sup[hyperedge_2]-=Edge_removal[hyperedge_2];
                   
                    adjust_list.insert(hyperedge_2);



                    const vector<int>& nodes_2 = hyperedge2node[hyperedge_2];
                    for (size_t p = 0; p < hyperedge2node[hyperedge_2].size(); ++p) {

                        if (!((SingleEdge_bitmap[hyperedge_2] >> p) & 1)) {
                            int cur_node=hyperedge2node[hyperedge_2][p];
                            node_position2[cur_node]+=1;
                            int loc=node_position2[cur_node];
                            while(loc<node_connection2[cur_node].size()){
                                int hyperedge_3=node_connection2[cur_node][loc];
                                if(hyperedge_3<=hyperedge_2) continue;
                                if(hyperedge_sup[hyperedge_3]<0) continue;
                                if(signal_firstlevel[hyperedge_3]==hyperedge_1 && signal_secondlevel[hyperedge_3]==hyperedge_2){
                                    ++loc;
                                    continue;
                                }
                                if(hyperedge_signal_firstlevel2[hyperedge_3]!=hyperedge_1){
                                    hyperedge_signal_firstlevel2[hyperedge_3]=hyperedge_1;
                                    hyperedge_signal_secondlevel2[hyperedge_3]=hyperedge_2;
                                    hyperedge_signal_secondlevel2[hyperedge_3]=hyperedge_2;			
                                    int common = SingleEdge_bitmap_reverse[hyperedge_2] & SingleEdge_bitmap_reverse[hyperedge_3];
                                    
                                    if(common==0){

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
                                }else{
                                    if(hyperedge_signal_secondlevel2[hyperedge_3]==hyperedge_2){
                                        loc+=1;
                                        continue;
                                    }else{
                                        hyperedge_signal_secondlevel2[hyperedge_3]=hyperedge_2;									
                                        int common = SingleEdge_bitmap_reverse[hyperedge_2] & SingleEdge_bitmap_reverse[hyperedge_3];

                                        if(common==0){			
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
                                loc+=1;
                            }
                        }
                    }
                    

                } else{



                    //cout<<"hyperedge_2: "<<hyperedge_2<<endl;
                    for (size_t p = 0; p < hyperedge2node[hyperedge_1].size(); ++p) {
                        if (!((SingleEdge_bitmap_reverse[hyperedge_2] >> p) & 1)) continue;
                        int node=hyperedge2node[hyperedge_1][p];
    
                        for (auto it = node2hyperedge_list[node].begin(); it != node2hyperedge_list[node].end(); ) {
                            int hyperedge_3 = *it;  
                    
                            if (hyperedge_3 == hyperedge_1 || hyperedge_sup[hyperedge_3]<0) {
                                it = node2hyperedge_list[node].erase(it); 
                            } else {
                                if(hyperedge_3==hyperedge_2 ) {
                                    ++it;
                                    continue;
                                }
                                
                                if(signal_firstlevel[hyperedge_3]==hyperedge_1 && signal_secondlevel[hyperedge_3]==hyperedge_2){
                                    if(repeat_Num[hyperedge_3]==1){
                                        if(hyperedge_2<hyperedge_3){
                                            hyperedge_pair_complex_count[hyperedge_2][hyperedge_3]+=1;
                                        }
                                        ++it;
                                        continue;
                                    }
                                    if(repeat_Num[hyperedge_3]==-1){
                                        repeat_Num[hyperedge_3]=1;
                                        if(hyperedge_2<hyperedge_3){
                                            hyperedge_pair_complex[hyperedge_2][hyperedge_3]-=1;
                                            hyperedge_pair_complex_count[hyperedge_2][hyperedge_3]+=1;
                                        }
                                    }
                                    ++it;
                                    continue;
                                }
    
                                repeat_Num[hyperedge_3]=-1;
                                signal_firstlevel[hyperedge_3]=hyperedge_1;
                                signal_secondlevel[hyperedge_3]=hyperedge_2;
    
                                
                                if(hyperedge_sup[hyperedge_2]==(k-1) && ready_to_remove[hyperedge_2]==0){
                                    ready_to_remove[hyperedge_2]=1;
                                    current_hyperedges.push(hyperedge_2);
                                } 
                                hyperedge_sup[hyperedge_2]-=1;
                                adjust_list.insert(hyperedge_2);
                                ++it;
                            }
                        }
    
                    }


                    const vector<int>& nodes_2 = hyperedge2node[hyperedge_2];
                    for (size_t p = 0; p < hyperedge2node[hyperedge_2].size(); ++p) {

                        if (!((SingleEdge_bitmap[hyperedge_2] >> p) & 1)) {
                            int cur_node=hyperedge2node[hyperedge_2][p];
                            node_position2[cur_node]+=1;
                            int loc=node_position2[cur_node];
                            while(loc<node_connection2[cur_node].size()){
                                int hyperedge_3=node_connection2[cur_node][loc];
                                if(hyperedge_3<=hyperedge_2) continue;
                                if(hyperedge_sup[hyperedge_3]<0) continue;
                                if(signal_firstlevel[hyperedge_3]==hyperedge_1 && signal_secondlevel[hyperedge_3]==hyperedge_2){
                                    ++loc;
                                    continue;
                                }
                                if(hyperedge_signal_firstlevel2[hyperedge_3]!=hyperedge_1){
                                    hyperedge_signal_firstlevel2[hyperedge_3]=hyperedge_1;
                                    hyperedge_signal_secondlevel2[hyperedge_3]=hyperedge_2;
                                    hyperedge_signal_secondlevel2[hyperedge_3]=hyperedge_2;			
                                   

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
                                    
                                }else{
                                    if(hyperedge_signal_secondlevel2[hyperedge_3]==hyperedge_2){
                                        loc+=1;
                                        continue;
                                    }else{
                                        hyperedge_signal_secondlevel2[hyperedge_3]=hyperedge_2;									 	
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
                                loc+=1;
                            }
                        }
                    }

                }
				
				
                
				
			}
                


    	}


		for (int val : adjust_list) {

			if(hyperedge_sup[val]<=k-2){
                
				Support_List[hyperedge_sup_copy[val]].erase(val);
				//EndLoop+=1;
			}else{
				Support_List[hyperedge_sup_copy[val]].erase(val);
				Support_List[hyperedge_sup[val]].insert(val);
                
				hyperedge_sup_copy[val]=hyperedge_sup[val];
                
			}
		}
		k=k+1;

	}





    for (int i = 0; i < 25; ++i) {
        std::cout << "Hyperedge ID: " << i 
                  << ", Trussness: " << Trussness[i] << "\n";
    }
    


	return 0;
}






