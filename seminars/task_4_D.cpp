#include <iostream>
#include <vector>
#include <exception>
#include <stdexcept>
#include <climits>

#include <algorithm>


////////////////////////////////////////////////////////////
// graph.h
////////////////////////////////////////////////////////////

namespace sgtl {
    typedef unsigned int vertex;
    typedef long long flow_t;
    const flow_t MAX_FLOW = LLONG_MAX;

    class Edge {
    public:
        Edge(vertex from, vertex to) : from_(from), to_(to) {
        }
        vertex from() const {
            return from_;
        }
        vertex to() const {
            return to_;
        }
    private:
        vertex from_;
        vertex to_;
    };

    template <class T>
    class EdgeContainer : public Edge {
    public:
        EdgeContainer(vertex from, vertex to) : Edge(from, to) {
        }
        EdgeContainer(vertex from, vertex to, T value) : Edge(from, to), value(value) {
        }
        T value;
    private:
        vertex from_;
        vertex to_;
    };

    class EdgeFlow : public Edge {
    public:
        EdgeFlow(vertex from, vertex to, flow_t capacity)
            : Edge(from, to), capacity_(capacity), flow_(0) {
        }
        flow_t capacity() const {
            return capacity_;
        }
        flow_t flow() const {
            return flow_;
        }
        flow_t pushFlow(flow_t flow) {
            flow_t addition = std::min(flow, capacity_ - flow_);
            flow_ += addition;
            return addition;
        }
        void pushCapacity(flow_t flow) {
            capacity_ += flow;
        }
        bool unsaturated() const {
            return flow_ < capacity_;
        }
        flow_t residualCapacity() const {
            return capacity_ - flow_;
        }

    private:
        flow_t capacity_;
        flow_t flow_;
    };

    template<class E>
    class Graph;

    template <class E>
    class GraphBaseIterator: public std::iterator <std::forward_iterator_tag, const E> {
    public:
        typedef size_t size_type;
        typedef typename std::iterator <std::forward_iterator_tag, const E>::reference reference;
        typedef typename std::iterator <std::forward_iterator_tag, const E>::difference_type difference_type;
        GraphBaseIterator()
            : graph_(NULL) {
        }
        GraphBaseIterator(Graph<E>* const graph, vertex v, size_t shift)
            : graph_(graph), v_(v), shift_(shift) {
        }



        ///Main operators
        reference operator*() {
            if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
                return graph_->edgeList_[graph_->edges_[v_][shift_]];
            } else {
                throw std::runtime_error("Graph:: iterator:: end() called");
            }
        }

        GraphBaseIterator& operator+=(const difference_type& indx)
        {
            shift_ = std::min(shift_ + indx, graph_->edgesCount(v_));
            return *this;
        }

        ///Clone operators
        GraphBaseIterator& operator++()
        {
            return (*this) += 1;
        }

        GraphBaseIterator operator++(int)
        {
            GraphBaseIterator temp(*this);
            (*this) += 1;
            return temp;
        }

        GraphBaseIterator operator+(const difference_type& idx) const
        {
            GraphBaseIterator buf(*this);
            return buf += idx;
        }
        bool operator==(const GraphBaseIterator& other) const
        {
            return (v_ == other.v_) && (shift_ == other.shift_);
        }
        bool operator!=(const GraphBaseIterator& other) const
        {
            return (v_ != other.v_) || (shift_ != other.shift_) ;
        }
        bool isNatural() {
            return (graph_->edges_[v_][shift_] % 2) == 0;
        }

    protected:
        Graph<E>* graph_;
        vertex v_;
        size_t shift_;
    };

    template <class E>
    class GraphIterator: public GraphBaseIterator<E> {
    public:
        GraphIterator(Graph<E>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<E>(graph, v, shift) {
        }
        GraphIterator()
            : GraphBaseIterator<E>() {
        }
    };

    template <>
    class GraphIterator<EdgeFlow>: public GraphBaseIterator<EdgeFlow> {
    public:
        GraphIterator(Graph<EdgeFlow>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<EdgeFlow>(graph, v, shift) {
        }
        GraphIterator()
            : GraphBaseIterator<EdgeFlow>() {
        }
        flow_t pushFlow(flow_t flow);
        void pushCapacity(flow_t flow);
    protected:
    };

    template <class E>
    class Graph {
    public:
        Graph() {
        }

        Graph(size_t vertexCount) {
            this->pushVertex(vertexCount);
        }

        friend class GraphBaseIterator<E>;
        friend class GraphIterator<E>;
        virtual vertex pushVertex() {
            edges_.push_back(std::vector<vertex>());
            return edges_.size() - 1;
        }

        virtual vertex pushVertex(size_t count) {
            edges_.resize(edges_.size() + count);
            return edges_.size() - 1;
        }

        virtual void pushEdge(E e) {
            if (isVertex(e.from()) && isVertex(e.to())) {
                edges_[e.from()].push_back(edgeList_.size());
                edgeList_.push_back(e);
            } else {
                throw std::runtime_error("Graph:: vertex out of bounds");
            }
        }

        size_t vertexCount() const {
            return edges_.size();
        }

        size_t edgesCount() const {
            return edgeList_.size();
        }

        size_t edgesCount(vertex v) const {
            return edges_[v].size();
        }

        bool isVertex(vertex v) const {
            return v < vertexCount();
        }

        typedef GraphIterator<E> base_iterator;

        base_iterator begin(vertex v) {
            return base_iterator(this, v, 0);
        }

        base_iterator end(vertex v) {
            return base_iterator(this, v, edges_[v].size());
        }

        base_iterator getEdge(vertex v, size_t shift) {
            if (isVertex(v) && (shift < edges_[v].size())) {
                return base_iterator(this, v, shift);
            } else {
                throw std::runtime_error("Graph:: vertex out of bounds");
            }
        }

    private:
        std::vector<std::vector<vertex>> edges_;
        std::vector<E> edgeList_;
    };

    flow_t GraphIterator<EdgeFlow>::pushFlow(flow_t flow) {
        // return (*(*this)).pushFlow(flow); is incorrect because reference is const EdgeFlow
        if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
            flow_t pushed = graph_->edgeList_[graph_->edges_[v_][shift_]].pushFlow(flow);
            size_t backEdgeIndex = (graph_->edges_[v_][shift_]) ^ 1;
            graph_->edgeList_[backEdgeIndex].pushFlow((flow_t)0 - pushed);
            return pushed;
        } else {
            throw std::runtime_error("Graph:: iterator:: end() called");
        }
    }

    void GraphIterator<EdgeFlow>::pushCapacity(flow_t flow) {
        // return (*(*this)).pushFlow(flow); is incorrect because reference is const EdgeFlow
        if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
            graph_->edgeList_[graph_->edges_[v_][shift_]].pushCapacity(flow);
        } else {
            throw std::runtime_error("Graph:: iterator:: end() called");
        }
    }

    class Network : public Graph<EdgeFlow> {
    public:
        // Hell, yeah!
        Network(size_t vertexCount, vertex source, vertex target) :source_(source), target_(target) {
            this->pushVertex(vertexCount);
            if (!isVertex(source_) || !isVertex(target_)) {
                throw std::runtime_error("Network:: source or target out of bounds");
            }
        }

        vertex getSource() const {
            return source_;
        }

        vertex getTarget() const {
            return target_;
        }

        virtual void pushEdge(EdgeFlow edge) {
            if (isVertex(edge.from()) && isVertex(edge.to())) {
                EdgeFlow backEdge(edge.to(), edge.from(), 0);
                Graph<EdgeFlow>::pushEdge(edge);
                Graph<EdgeFlow>::pushEdge(backEdge);
            } else {
                throw std::runtime_error("Graph:: vertex out of bounds");
            }
        }

    private:
        vertex source_;
        vertex target_;
    };
}

////////////////////////////////////////////////////////////
// bfs.h
////////////////////////////////////////////////////////////

#include <queue>

namespace sgtl {
    class DFS {
    public:
        static void dfs(vertex v, Graph<Edge> &gr, std::vector<int> &order, int color) {
            order[v] = color;
            for (auto e = gr.begin(v); e != gr.end(v); ++e) {
                if (!order[(*e).to()]) {
                    dfs((*e).to(), gr, order, color);
                }
            }
        }
        static void dfsSeries(Graph<Edge> &gr, std::vector<int> &order) {
            order.assign(gr.vertexCount(), 0);
            for (vertex i = 0; i < gr.vertexCount(); ++i) {
                if (!order[i]) {
                    dfs(i, gr, order, i + 1);
                }
            }
        }
    private:
        DFS() { // only static class
        }
    };

    class BFS {
    public:
        static void bfsUntil(Graph<EdgeFlow>& g, vertex v, vertex to, std::vector<vertex>& level) {
            if (!g.isVertex(v)) {
                throw std::runtime_error("bsf :: vertex out of bounds");
            }
            level.assign(g.vertexCount(), 0);
            level[v] = 1;

            std::queue <vertex> qq;
            qq.push(v);

            while (!qq.empty() && (level[to] == 0)) {
                v = qq.front();
                qq.pop();

                for (auto edge = g.begin(v); edge != g.end(v); ++edge) {
                    if ((level[(*edge).to()] == 0) && (*edge).unsaturated()) {
                        level[(*edge).to()] = level[v] + 1;
                        qq.push((*edge).to());
                    }
                }
            }
        }
    private:
        BFS() { // only static class
        }
    };
}

////////////////////////////////////////////////////////////
// flow.h
////////////////////////////////////////////////////////////

namespace sgtl {
    class Dinitz {
    public:
        static Dinitz& instance() {
            static Dinitz forewerAlone;
            return forewerAlone;
        }
        static flow_t getMaxFlow(Network& g) {
            return instance().getMaxFlow_(g);
        }

    private:
        Dinitz(){}  // singleton? yes, thanks
        Dinitz(const Dinitz& root);
        Dinitz& operator=(const Dinitz&);

        std::vector<vertex> level_;
        std::vector<Network::base_iterator> ptr_;

        flow_t getMaxFlow_(Network& g) {
            flow_t flow = 0;
            flow_t push;

            ptr_.resize(g.vertexCount());

            while (true) {
                BFS::bfsUntil(g, g.getSource(), g.getTarget(), level_);
                if (level_[g.getTarget()] == 0) {
                    break;
                }
                for (vertex v = 0; v < g.vertexCount(); ++v) {
                    ptr_[v] = g.begin(v);
                }
                while ((push = dfsPushFlow_(g, MAX_FLOW))) {
                    flow += push;
                }
            }
            return flow;
        }



        flow_t dfsPushFlow_(Network& g, flow_t flow) {
            return dfsPushFlow_(g, flow, g.getSource());
        }

        flow_t dfsPushFlow_(Network& g, flow_t flow, vertex v) {
            if (!flow) {
                return 0;
            }
            if (v == g.getTarget()) {
                return flow;
            }
            for (; ptr_[v] != g.end(v); ++ptr_[v]) {
                if (level_[v] + 1 == level_[(*ptr_[v]).to()])
                    if ((*ptr_[v]).unsaturated()) {
                        flow_t pushed = 0;
                        flow_t tryPushFlow = std::min(flow, (*ptr_[v]).residualCapacity());
                        if ((pushed = dfsPushFlow_(g, tryPushFlow, (*ptr_[v]).to()))) {
                            ptr_[v].pushFlow(pushed);
                            return pushed;
                        }
                    }
            }
            return 0;
        }

    };
}

////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////


using std::cin;
using std::vector;
using std::cout;
using std::pair;
using std::make_pair;
using namespace sgtl;

struct my{
    int time;
    std::pair<int, int> st;
    std::pair<int, int> fn;
};

int dest(const std::pair<int, int>& st, const std::pair<int, int>& fn) {
    return abs(st.first - fn.first) + abs(st.second - fn.second);
}

int time(const my& v) {
    return dest(v.st, v.fn);
}

int dest (const my& from, const my& to) {
    return dest(from.fn, to.st);
}

std::vector <my> q;

my read() {
    my v;
    int h, m;
    char trash;
    std::cin >> h >> trash >> m >> v.st.first >> v.st.second >> v.fn.first >> v.fn.second;
    v.time = h * 60 + m;
    return v;
}

int main() {
    int n;
    std::cin >> n;
    q.resize(n);
    for (int i = 0; i < n; ++i) {
        q[i] = read();
    }

    Network g(2*n + 2, 2*n, 2*n + 1);
    for (int i = 0; i < n; ++i) {
        g.pushEdge(sgtl::EdgeFlow(g.getSource(), 2*i, 1));
        //g.pushEdge(sgtl::EdgeFlow(2*i, 2*i + 1, 1));
        g.pushEdge(sgtl::EdgeFlow(2*i + 1, g.getTarget(), 1));
    }

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (q[i].time + time(q[i]) + dest(q[i], q[j]) < q[j].time) {
                g.pushEdge(sgtl::EdgeFlow(2*i, 2*j + 1, 1));
            }
        }
    }

    Dinitz::getMaxFlow(g);


    vector<int> order(n);
    Graph<Edge> gr(n);

    for (int i = 0; i < n; ++i) {
        for (auto e = g.begin(2*i); e != g.end(2*i); ++e) {
            if (!(*e).unsaturated() && ((*e).to() / 2 < n)) {
                gr.pushEdge(Edge(i, (*e).to() / 2));
                gr.pushEdge(Edge((*e).to() / 2, i));
            }
        }
    }

    DFS::dfsSeries(gr, order);

    std::sort(order.begin(), order.end());
    std::vector<int>::iterator it = std::unique(order.begin(), order.end());
    order.resize( std::distance(order.begin(),it) );
    cout << order.size();
    cout.flush();
    return 0;
}
