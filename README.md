# hyper-truss-decomposition
code for the hyper-truss decomposition algorithm


## Dataset Format

The input dataset is a **hypergraph**, where each line represents a hyperedge and contains a list of vertex IDs separated by commas.

## Example

Input file (`dblp_graph.txt`):

This dataset defines a hypergraph \( H = (V, E) \), where each line corresponds to a hyperedge.

For example:

- \( e_1 = \{0,1,2,3\} \)
- \( e_2 = \{6,7\} \)
- \( e_3 = \{12,13\} \)
- \( e_4 = \{21,23,16\} \)
- ...
- \( e_{22} = \{100,101\} \)
- ...
  
Each hyperedge contains a variable number of vertices.

### Description

- Each line corresponds to **one hyperedge**.
- Integers represent **vertex IDs**.
- Vertex IDs within a hyperedge are separated by commas `,`.
- The number of vertices per hyperedge can vary.

### Notes

- Vertex IDs are assumed to be **non-negative integers**.
- The dataset does **not require ordering**, but consistent ordering (e.g., sorted IDs) is recommended.


## Real-world Datasets

For additional real-world hypergraph datasets, please refer to:

- Cornell University Network Dataset Collection  
  https://www.cs.cornell.edu/~arb/data/


## Basic Implementation (HTD-BS)


### File

- Source code: `HTD-BS.cpp`

---

### Input

The input file is specified in the following line:

string graphFile = "dblp_graph.txt";

### Output

The trussness of each hyperedge is stored in:

```cpp
vector<int> Trussness;
