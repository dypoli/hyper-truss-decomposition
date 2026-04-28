# hyper-truss-decomposition
code for the hyper-truss decomposition algorithm


## Dataset Format

The input dataset is a **hypergraph**, where each line represents a hyperedge and contains a list of vertex IDs separated by commas.

### Example
0,1,2,3\\
6,7
12,13
21,23,16
20,18
24,22
31,32
33,34
41,42
46,47,1,45
48,49

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


  
