
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
#include <list>

#include <sstream>
#include <string>
using namespace std;



// struct used to store the hyperedge

struct Hyperedge { 
    int ID;
    Hyperedge* prev;
    Hyperedge* next;
    Hyperedge(int id) : ID(id), prev(nullptr), next(nullptr) {}
};


struct Triangle { 
    int sigal;
    int node_1;
    int node_2;
    int node_3;
    Triangle(int a, int b, int c) : sigal(1), node_1(a), node_2(b), node_3(c){}
};

struct BloomNode; 
struct EdgeInfo;
struct ChildNode;

struct ChildNode { 
    int Id;
    vector<BloomNode*> Bloom_List;
    ChildNode(int id) : Id(id), Bloom_List(vector<BloomNode*>()) {}
};

struct BloomNode { 
    int sigal;
    int node_1;
    int node_2;
    vector<ChildNode*> Child_List;
    BloomNode(int a, int b) : sigal(1), node_1(a), node_2(b), Child_List(vector<ChildNode*>()) {}
};

struct EdgeInfo {
    vector<BloomNode*> Bloom_List;
    ChildNode* Child;
    EdgeInfo(): Bloom_List(vector<BloomNode*>()), Child(nullptr) {}
};

struct SupportLayer {
    int support;
    unordered_set<int> hyperedges;
    SupportLayer(int supportnum): support(supportnum) {}
};

struct SupportData {
    vector<SupportLayer*> Support_List;
    vector<SupportLayer*> HyperedgeToSupport_List;
};



inline long long convert_id(int hyperedge_a, int hyperedge_b){
	return hyperedge_a * (1LL << 31) + hyperedge_b;
}

/*       help function, used to check the list       */


vector<int> get_sorted_hyperedges_by_degree(const vector<vector<int>>& hyperedge_adj) {
    int n = hyperedge_adj.size();
    vector<pair<int, int>> degrees;  // pair<degree, id>

    for (int i = 0; i < n; ++i) {
        degrees.emplace_back(hyperedge_adj[i].size(), i);
    }


    sort(degrees.begin(), degrees.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
        if (a.first != b.first) return a.first > b.first;
        return a.second > b.second;
    });

    vector<int> sorted_ids;
    for (const auto& p : degrees) {
        sorted_ids.push_back(p.second);
    }

    return sorted_ids;
}

void sort_adj_lists_by_degree_desc(vector<vector<int>>& hyperedge_adj) {
    int n = hyperedge_adj.size();
    vector<int> degree(n);
    for (int i = 0; i < n; ++i) {
        degree[i] = hyperedge_adj[i].size();
    }

    for (int i = 0; i < n; ++i) {
        sort(hyperedge_adj[i].begin(), hyperedge_adj[i].end(), [&](int a, int b) {
            if (degree[a] != degree[b]) return degree[a] > degree[b];
            return a > b;
        });
    }
}

void filter_and_sort_adj_lists_by_degree(vector<vector<int>>& hyperedge_adj) {
    int n = hyperedge_adj.size();
    vector<int> degree(n);

    for (int i = 0; i < n; ++i) {
        degree[i] = hyperedge_adj[i].size();
    }

    for (int i = 0; i < n; ++i) {
        vector<int> filtered;

        for (int j : hyperedge_adj[i]) {
            if (degree[j] < degree[i] || (degree[j] == degree[i] && j < i)) {
                filtered.push_back(j);
            }
        }

        sort(filtered.begin(), filtered.end(), [&](int a, int b) {
            if (degree[a] != degree[b]) return degree[a] > degree[b];
            return a > b;
        });

        hyperedge_adj[i] = std::move(filtered);
    }
}


std::vector<Hyperedge*> buildHyperedgePtrMap(Hyperedge* head, int total_hyperedges) {
    std::vector<Hyperedge*> hyperedge_ptrs(total_hyperedges, nullptr);
    for (Hyperedge* curr = head; curr != nullptr; curr = curr->next) {
        hyperedge_ptrs[curr->ID] = curr;
    }
    return hyperedge_ptrs;
}

Hyperedge* buildHyperedgeList(const std::vector<int>& hyperedge_sup) {
    std::vector<Hyperedge*> nodes;
    for (int i = 0; i < hyperedge_sup.size(); ++i) {
        nodes.push_back(new Hyperedge(i));
    }

   
    std::sort(nodes.begin(), nodes.end(), [&](Hyperedge* a, Hyperedge* b) {
        if (hyperedge_sup[a->ID] != hyperedge_sup[b->ID])
            return hyperedge_sup[a->ID] < hyperedge_sup[b->ID];
        return a->ID < b->ID;
    });

    
    for (int i = 0; i < nodes.size(); ++i) {
        if (i > 0) nodes[i]->prev = nodes[i - 1];
        if (i < nodes.size() - 1) nodes[i]->next = nodes[i + 1];
    }

    return nodes.empty() ? nullptr : nodes[0]; 
}

void deleteHyperedgeList(Hyperedge* head) {
    while (head) {
        Hyperedge* temp = head;
        head = head->next;
        delete temp;
    }
}

vector<int> buildSortedHyperedgesBySup(const vector<int>& hyperedge_sup) {
    int n = hyperedge_sup.size();
    vector<int> SortSup(n);

    
    for (int i = 0; i < n; ++i) {
        SortSup[i] = i;
    }

    
    sort(SortSup.begin(), SortSup.end(), [&](int a, int b) {
        if (hyperedge_sup[a] != hyperedge_sup[b])
            return hyperedge_sup[a] < hyperedge_sup[b];
        return a < b;
    });

    return SortSup;
}


vector<vector<int>> buildHyperedgeAdjSup(const vector<vector<int>>& hyperedge_adj, const vector<int>& hyperedge_sup) {
    vector<vector<int>> hyperedge_adjSup(hyperedge_adj.size());

    for (size_t i = 0; i < hyperedge_adj.size(); ++i) {
        for (int j : hyperedge_adj[i]) {
            if (
                hyperedge_sup[j] > hyperedge_sup[i] ||
                (hyperedge_sup[j] == hyperedge_sup[i] && j > static_cast<int>(i))
            ) {
                hyperedge_adjSup[i].push_back(j);
            }
        }

        
        sort(hyperedge_adjSup[i].begin(), hyperedge_adjSup[i].end(),
            [&](int a, int b) {
                if (hyperedge_sup[a] != hyperedge_sup[b])
                    return hyperedge_sup[a] < hyperedge_sup[b];
                return a < b;
            }
        );
    }

    return hyperedge_adjSup;
}

map<int, vector<int>> groupIndicesBySupport(const vector<int>& hyperedge_sup) {
    map<int, vector<int>> hyperedge_supLayer;

    for (size_t i = 0; i < hyperedge_sup.size(); ++i) {
        int sup = hyperedge_sup[i];
        hyperedge_supLayer[sup].push_back(i);
    }

    return hyperedge_supLayer;
}

void writeHyperedgeAdjToFile(const vector<vector<int>>& hyperedge_adj, const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    for (const auto& edge : hyperedge_adj) {
        for (size_t i = 0; i < edge.size(); ++i) {
            outfile << edge[i];
            if (i + 1 < edge.size()) outfile << " ";
        }
        outfile << "\n";
    }

    outfile.close();
    cout << "Data written to " << filename << endl;
}

void removeDuplicatesFromHyperedgeAdj(vector<vector<int>>& hyperedge_adj) {
    for (auto& edge : hyperedge_adj) {
        edge.erase(unique(edge.begin(), edge.end()), edge.end());
    }
}

void removeDuplicatesFromSingleHyperedgeAdj(vector<int>& hyperedge_adj) {

    hyperedge_adj.erase(unique(hyperedge_adj.begin(), hyperedge_adj.end()), hyperedge_adj.end());

}

void completeSymmetricValues(
    const std::vector<std::vector<int>>& hyperedge_adj,
    std::vector<std::vector<int>>& hyperedge_TriangleNum
) {
    int n = hyperedge_adj.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < hyperedge_adj[i].size(); ++j) {
            int nei = hyperedge_adj[i][j];
            if (i < nei) {
                
                auto& nei_list = hyperedge_adj[nei];
                auto it = std::find(nei_list.begin(), nei_list.end(), i);
                if (it != nei_list.end()) {
                    int pos = it - nei_list.begin();
                   
                    hyperedge_TriangleNum[nei][pos] = hyperedge_TriangleNum[i][j];
                }
            }
        }
    }
}



vector<unordered_set<int>> convertToSet(const vector<vector<int>>& hyperedge_adj) {
    vector<unordered_set<int>> hyperedge_adj_set;
    hyperedge_adj_set.reserve(hyperedge_adj.size());

    for (const auto& vec : hyperedge_adj) {
        unordered_set<int> s(vec.begin(), vec.end());
        hyperedge_adj_set.push_back(std::move(s));
    }

    return hyperedge_adj_set;
}

vector<unordered_set<int>> convertToSetFiltered(
    const vector<vector<int>>& hyperedge_adj,
    const vector<int>& hyperedge2order
) {
    vector<unordered_set<int>> hyperedge_adj_set;
    hyperedge_adj_set.reserve(hyperedge_adj.size());

    for (int i = 0; i < hyperedge_adj.size(); ++i) {
        unordered_set<int> s;
        int current_order = hyperedge2order[i];

        for (int neighbor : hyperedge_adj[i]) {
            if (hyperedge2order[neighbor] > current_order) {
                s.insert(neighbor);
            }
        }

        hyperedge_adj_set.push_back(std::move(s));
    }

    return hyperedge_adj_set;
}





vector<unordered_set<int>> convertToSetFilteredByIndex(const vector<vector<int>>& hyperedge_adj) {
    vector<unordered_set<int>> hyperedge_adj_set;
    hyperedge_adj_set.reserve(hyperedge_adj.size());

    for (int i = 0; i < hyperedge_adj.size(); ++i) {
        unordered_set<int> s;
        for (int neighbor : hyperedge_adj[i]) {
            if (neighbor > i) {
                s.insert(neighbor);
            }
        }
        hyperedge_adj_set.push_back(std::move(s));
    }

    return hyperedge_adj_set;
}



vector<vector<int>> convertToVectorFiltered(
    const vector<vector<int>>& hyperedge_adj,
    const vector<int>& hyperedge2order
) {
    vector<vector<int>> hyperedge_adj_set;
    hyperedge_adj_set.resize(hyperedge_adj.size());

    for (int i = 0; i < hyperedge_adj.size(); ++i) {
        vector<int> s;
        int current_order = hyperedge2order[i];

        for (int neighbor : hyperedge_adj[i]) {
            if (hyperedge2order[neighbor] > current_order) {
                s.push_back(neighbor);
            }
        }

        hyperedge_adj_set[i]=s;
    }

    return hyperedge_adj_set;
}

vector<vector<int>> convertToVectorWithIndex(
    const vector<vector<int>>& hyperedge_adj,
    vector<int>& Index_signal
) {
    vector<vector<int>> hyperedge_adj_set;
    hyperedge_adj_set.resize(hyperedge_adj.size());

    for (int i = 0; i < hyperedge_adj.size(); ++i) {
        vector<int> s;

        for (int j=0;j<hyperedge_adj[i].size();++j) {
            int neighbor=hyperedge_adj[i][j];
            if (neighbor > i) {
                if(Index_signal[i]==-1) Index_signal[i]=j;
            }
        }
        if(Index_signal[i]==-1) Index_signal[i]=hyperedge_adj[i].size();
    }

    return hyperedge_adj_set;
}


vector<int> convertToSingleVectorWithIndex(
    int i,
    const vector<int>& hyperedge_adj,
    vector<int>& Index_signal
) {
    vector<int> hyperedge_adj_set;

    for (int j=0;j<hyperedge_adj.size();++j) {
        int neighbor=hyperedge_adj[j];
        if (neighbor > i) {
            if(Index_signal[i]==-1) Index_signal[i]=j;
        }
    }

    

    return hyperedge_adj_set;
}

vector<vector<int>> convertToVectorFilteredByIndex(
    const vector<vector<int>>& hyperedge_adj
) {
    vector<vector<int>> hyperedge_adj_set;
    hyperedge_adj_set.resize(hyperedge_adj.size());

    for (int i = 0; i < hyperedge_adj.size(); ++i) {
        vector<int> s;

        for (int neighbor : hyperedge_adj[i]) {
            if (neighbor > i) {
                s.push_back(neighbor);
            }
        }

        hyperedge_adj_set[i]=s;
    }

    return hyperedge_adj_set;
}

void removeNode(Hyperedge*& head, Hyperedge* node) {
    if (node->prev) node->prev->next = node->next;
    else head = node->next; 

    if (node->next) node->next->prev = node->prev;

    node->prev = node->next = nullptr;
}

void insertNodeSorted(Hyperedge*& head, Hyperedge* node, const std::vector<int>& hyperedge_sup) {
    if (!head) {
        head = node;
        return;
    }

    Hyperedge* curr = head;
    while (curr) {
        int support_curr = hyperedge_sup[curr->ID];
        int support_node = hyperedge_sup[node->ID];

        if (support_node < support_curr || 
            (support_node == support_curr && node->ID < curr->ID)) {
           
            node->next = curr;
            node->prev = curr->prev;
            if (curr->prev) curr->prev->next = node;
            else head = node;
            curr->prev = node;
            return;
        }

        if (!curr->next) break;
        curr = curr->next;
    }


    curr->next = node;
    node->prev = curr;
    node->next = nullptr;
}

void RecurseFunc(int depth, vector<int>& ValidHyperedge, vector<int>& hyperedge_sup, vector<int>& hyperedge_adj_i, 
    vector<unordered_set<int>>& hyperedge_adjset, unordered_map<int, int>& hyperwedge_adj_i,
    vector<Hyperedge*>& hyperedge_ptrs, Hyperedge* iter) {

    int hyperedge_2=hyperedge_adj_i[depth];
    if (depth == hyperedge_adj_i.size()-1) {
        if(hyperedge_sup[hyperedge_2]>0) {
            ValidHyperedge.insert(ValidHyperedge.begin(), hyperedge_2);
        }
        return;
    }else{
        RecurseFunc(depth+1, ValidHyperedge, hyperedge_sup, hyperedge_adj_i, hyperedge_adjset, hyperwedge_adj_i, hyperedge_ptrs, iter);  
        if(hyperedge_sup[hyperedge_2]<=0) return;
        ValidHyperedge.insert(ValidHyperedge.begin(), hyperedge_2);
		if(hyperwedge_adj_i[hyperedge_2]==0) return;

        for(int j=1; j<ValidHyperedge.size(); j++){
            int hyperedge_3=ValidHyperedge[j];
            if(!hyperedge_adjset[hyperedge_2].count(hyperedge_3)) continue;

            hyperedge_sup[hyperedge_2]-=1;
            hyperedge_sup[hyperedge_3]-=1;
            removeNode(iter, hyperedge_ptrs[hyperedge_2]);
            removeNode(iter, hyperedge_ptrs[hyperedge_3]);

            
            insertNodeSorted(iter, hyperedge_ptrs[hyperedge_2], hyperedge_sup);
            insertNodeSorted(iter, hyperedge_ptrs[hyperedge_3], hyperedge_sup);

        }
    }

}


SupportData BuildSupportData(const vector<int>& hyperedge_sup) {
  
    int max_support = 0;
    for (int s : hyperedge_sup) {
        if (s > max_support) max_support = s;
    }


    vector<SupportLayer*> Support_List;
    for (int s = 0; s <= max_support; ++s) {
        Support_List.push_back(new SupportLayer(s));
    }


    vector<SupportLayer*> HyperedgeToSupport_List(hyperedge_sup.size());

    for (int hyperedge_id = 0; hyperedge_id < hyperedge_sup.size(); ++hyperedge_id) {
        int support = hyperedge_sup[hyperedge_id];
        SupportLayer* layer = Support_List[support];
        layer->hyperedges.insert(hyperedge_id);
        HyperedgeToSupport_List[hyperedge_id] = layer;
    }

    return {Support_List, HyperedgeToSupport_List};
}

queue<int> SetToQueue(const unordered_set<int>& input_set) {
    queue<int> q;
    for (int val : input_set) {
        q.push(val);
    }
    return q;
}

queue<int> VectorToQueue(const vector<int>& input_set) {
    queue<int> q;
    for (int i=0; i<input_set.size(); i++) {
        q.push(input_set[i]);
    }
    return q;
}




std::vector<std::unordered_set<int>> BuildSupportList(const std::vector<int>& hyperedge_sup) {
 
    int max_support = 0;
    for (int sup : hyperedge_sup) {
        max_support = std::max(max_support, sup);
    }


    std::vector<std::unordered_set<int>> Support_List(max_support + 1);


    for (int hyperedge = 0; hyperedge < hyperedge_sup.size(); ++hyperedge) {
        int support = hyperedge_sup[hyperedge];
        Support_List[support].insert(hyperedge);
    }

    return Support_List;
}

void sortHyperedges(
    const vector<int>& hyperedge_NodeDegree,
    vector<vector<int>>& hyperedge_adj,
    vector<int>& hyperedge_order
) {
    int E = hyperedge_NodeDegree.size();

  
    for (int i = 0; i < E; i++) {
        sort(hyperedge_adj[i].begin(), hyperedge_adj[i].end(), [&](int a, int b) {
            if (hyperedge_NodeDegree[a] != hyperedge_NodeDegree[b])
                return hyperedge_NodeDegree[a] < hyperedge_NodeDegree[b];
            return a < b; // tie-breaker: smaller id first
        });
    }


    hyperedge_order.resize(E);
    for (int i = 0; i < E; i++) hyperedge_order[i] = i;

    sort(hyperedge_order.begin(), hyperedge_order.end(), [&](int a, int b) {
        if (hyperedge_NodeDegree[a] != hyperedge_NodeDegree[b])
            return hyperedge_NodeDegree[a] < hyperedge_NodeDegree[b];
        return a < b; // tie-breaker
    });
}

bool OrderCheck(const vector<int>& hyperedge_NodeDegree, int a, int b){
    if (hyperedge_NodeDegree[a] != hyperedge_NodeDegree[b])
            return hyperedge_NodeDegree[a] < hyperedge_NodeDegree[b];
        return a < b; // tie-breaker: smaller id first
}


vector<unordered_set<int>> generateHyperedgeNeighborNodeSet(
    const vector<unordered_set<int>>& hyperedge_adjset,
    const vector<vector<int>>& hyperedge2node
) {
    int E = hyperedge_adjset.size();
    vector<unordered_set<int>> hyperedge_NeighborNodeSet(E);

    for (int i = 0; i < E; ++i) {
        for (int neighbor : hyperedge_adjset[i]) {
            for (int node : hyperedge2node[neighbor]) {
                hyperedge_NeighborNodeSet[i].insert(node);
            }
        }
    }

    return hyperedge_NeighborNodeSet;
}


std::vector<std::vector<int>> readBitmapIndices(const std::string& filename) {
    std::vector<std::vector<int>> result;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "OPEN FAIL: " << filename << std::endl;
        return result;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<int> indices;
        std::istringstream iss(line);
        int num;
        while (iss >> num) {
            indices.push_back(num);
        }
        result.push_back(indices);
    }

    file.close();
    return result;
}


std::vector<std::list<int>> convertToListVector(const std::vector<std::vector<int>>& vecVec) {
    std::vector<std::list<int>> result;
    result.reserve(vecVec.size()); // 

    for (const auto& innerVec : vecVec) {
        result.emplace_back(innerVec.begin(), innerVec.end());
    }

    return result;
}
/*       end of the help function         */


void WriteDegreeToFile(const vector<int>& Degree_hyperedges, const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return;
    }

    for (int degree : Degree_hyperedges) {
        outfile << degree << '\n';
    }

    outfile.close();
}


void WriteAdjacencyToFile(const vector<vector<int>>& hyperedge_adj, const string& filename) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return;
    }

    for (const auto& row : hyperedge_adj) {
        for (size_t i = 0; i < row.size(); ++i) {
            outfile << row[i];
            if (i != row.size() - 1) outfile << ' ';  
        }
        outfile << '\n';  
    }

    outfile.close();
}

