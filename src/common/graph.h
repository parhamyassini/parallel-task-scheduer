#include "utils.h"
#include "quick_sort.h"
#include <fstream>

typedef std::pair<uintV, uintV> intPair;

template <class E>
struct ascendingF
{
    E operator()(const E &a, const E &b) const { return a > b; }
};

class Vertex
{
public:
    uintE out_degree_;
    uintE in_degree_;

    uintV *out_neighbors_;
    uintV *in_neighbors_;

    Vertex() : out_degree_(0), in_degree_(0), out_neighbors_(nullptr), in_neighbors_(nullptr) {}

    uintV getOutNeighbor(uintE i)
    {
        return out_neighbors_[i];
    }

    uintV getInNeighbor(uintE i)
    {
        return in_neighbors_[i];
    }

    void setOutNeighbors(uintV *t) { out_neighbors_ = t; }
    void setInNeighbors(uintV *t) { in_neighbors_ = t; }

    void setOutDegree(uintE d) { out_degree_ = d; }
    void setInDegree(uintE d) { in_degree_ = d; }

    uintE getOutDegree() { return out_degree_; }
    uintE getInDegree() { return in_degree_; }
    uintV *getOutNeighbors() { return out_neighbors_; }
    uintV *getInNeighbors() { return in_neighbors_; }
};

class Graph
{

    uintV *out_edges_;
    uintV *in_edges_;

public:
    uintV n_;
    uintE m_;
    Vertex *vertices_;
    Graph() = default;

    template<class T>
    void read_graph_from_binary(std::string input_file_path)
    {
        std::string input_file_path_csr = input_file_path + ".csr";
        std::string input_file_path_csc = input_file_path + ".csc";

        // Read csr file
        std::ifstream input_stream(input_file_path_csr, std::ifstream::in | std::ios::binary);
        input_stream.seekg(0, std::ios::end);
        T size = input_stream.tellg();
        input_stream.seekg(0);
        char *csr_char_array = (char *)malloc(size);
        input_stream.read(csr_char_array, size);
        input_stream.close();
        T* csr_contents = (T*) csr_char_array;

        // Read csc file
        std::ifstream input_stream2(input_file_path_csc, std::ifstream::in | std::ios::binary); 
        input_stream2.seekg(0, std::ios::end);
        size = input_stream2.tellg();
        input_stream2.seekg(0);
        char *csc_char_array = (char *)malloc(size);
        input_stream2.read(csc_char_array, size);
        input_stream2.close();
        T* csc_contents = (T*) csc_char_array;

        n_ = csr_contents[0];
        m_ = csr_contents[1];

        out_edges_ = new uintV[m_];
        in_edges_ = new uintV[m_];
        vertices_ = new Vertex[n_];

        T *out_offsets = new uintE[n_];
        T *in_offsets = new uintE[n_];

        // Update CSR ----------------
        for(uintV i = 0; i < n_; i++)
        {
            out_offsets[i] = csr_contents[i + 2];
            in_offsets[i] = csc_contents[i + 2];
        }

        for(uintE i = 0; i < m_; i++)
        {
            out_edges_[i] = csr_contents[i + (uintE)(n_ + 2)];
            in_edges_[i] = csc_contents[i + (uintE)(n_ + 2)];
        }

        free(csr_contents);
        free(csc_contents);

        for(uintV i = 0; i < n_; i++)
        {
            uintE start = out_offsets[i];
            uintE d = (i == n_ - 1) ? (m_ - start) : (out_offsets[i + 1] - start);
            vertices_[i].setOutNeighbors(out_edges_ + start);
            if(d>1){
                quickSort(out_edges_ + start, d, [](uintV val1, uintV val2) {
                    return val1 < val2;
                });
            }
            vertices_[i].setOutDegree(d);

            start = in_offsets[i];
            d = (i == n_ - 1) ? (m_ - start) : (in_offsets[i + 1] - start);
            vertices_[i].setInNeighbors(in_edges_ + start);
            if(d>1){
                quickSort(in_edges_ + start, d, [](uintV val1, uintV val2) {
                    return val1 < val2;
                });
            }
            vertices_[i].setInDegree(d);
        }

        delete[] out_offsets;
        delete[] in_offsets;
    }

    void printGraph(std::string output_file_path = "/tmp/output/a")
    {
        std::cout << "Graph :\n";
        std::cout << "n : " << n_ << "\n";
        std::cout << "m : " << m_ << "\n";
        std::cout << "Parsing outEdges\n";
        std::ofstream output_file;
        output_file.open(output_file_path + "1", std::ios::out);
        for (uintV i = 0; i < n_; i++)
        {
            Vertex &v = vertices_[i];
            auto d = v.getOutDegree();
            if (d != 0)
            {
                insertionSort(v.getOutNeighbors(), d, ascendingF<uintV>());
            }

            for (uintE j = 0; j < v.getOutDegree(); j++)
            {
                output_file << i << " " << v.getOutNeighbor(j) << "\n";
            }
        }
        output_file.close();

        std::cout << "Parsing inEdges\n";
        output_file.open(output_file_path + "2", std::ios::out);
        for (uintV i = 0; i < n_; i++)
        {
            Vertex &v = vertices_[i];
            auto d = v.getInDegree();
            if (d != 0)
            {
                insertionSort(v.getInNeighbors(), d, ascendingF<uintV>());
            }
            for (uintE j = 0; j < v.getInDegree(); j++)
            {
                output_file << i << " " << v.getInNeighbor(j) << "\n";
            }
        }
        output_file.close();
    }

    ~Graph()
    {
        delete[] out_edges_;
        delete[] in_edges_;
        delete[] vertices_;
    }
};