#include <iostream>
#include <vector>
#include <exception>
#include <stdexcept>
#include <climits>
#include <functional>

/// 09 October 2016

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

    class EdgeCostFlow : public EdgeFlow {
    public:
        EdgeCostFlow(vertex from, vertex to, flow_t capacity, int cost)
            : EdgeFlow(from, to, capacity), cost_(cost) {
        }
        int cost() const {
            return cost_;
        }

    private:
        int cost_;
    };

    class EdgeFlowMarked : public EdgeFlow {
    public:
        EdgeFlowMarked(vertex from, vertex to, flow_t capacity) : EdgeFlow(from, to, capacity), mark(0) {
        }
        int mark;
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

    template <>
    class GraphIterator<EdgeCostFlow>: public GraphBaseIterator<EdgeCostFlow> {
    public:
        GraphIterator(Graph<EdgeCostFlow>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<EdgeCostFlow>(graph, v, shift) {
        }
        GraphIterator()
            : GraphBaseIterator<EdgeCostFlow>() {
        }
        flow_t pushFlow(flow_t flow);
        void pushCapacity(flow_t flow);
    protected:
    };

    template <>
    class GraphIterator<EdgeFlowMarked>: public GraphBaseIterator<EdgeFlowMarked> {
    public:
        GraphIterator(Graph<EdgeFlowMarked>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<EdgeFlowMarked>(graph, v, shift) {
        }
        GraphIterator()
            : GraphBaseIterator<EdgeFlowMarked>() {
        }

        void mark(int v);
        int getMark() const;
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

    flow_t GraphIterator<EdgeCostFlow>::pushFlow(flow_t flow) {
        // return (*(*this)).pushCostFlow(flow); is incorrect because reference is const EdgeFlow
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

    flow_t GraphIterator<EdgeFlowMarked>::pushFlow(flow_t flow) {
        // return (*(*this)).pushCostFlow(flow); is incorrect because reference is const EdgeFlow
        if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
            flow_t pushed = graph_->edgeList_[graph_->edges_[v_][shift_]].pushFlow(flow);
            size_t backEdgeIndex = (graph_->edges_[v_][shift_]) ^ 1;
            graph_->edgeList_[backEdgeIndex].pushFlow((flow_t)0 - pushed);
            return pushed;
        } else {
            throw std::runtime_error("Graph:: iterator:: end() called");
        }
    }

    void GraphIterator<EdgeFlowMarked>::mark(int v) {
        if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
            graph_->edgeList_[graph_->edges_[v_][shift_]].mark = v;
        } else {
            throw std::runtime_error("Graph:: iterator:: end() called");
        }
    }

    int GraphIterator<EdgeFlowMarked>::getMark() const {
        if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
            return graph_->edgeList_[graph_->edges_[v_][shift_]].mark;
        } else {
            throw std::runtime_error("Graph:: iterator:: end() called");
        }
    }

    template <class E>
    class Network : public Graph<E> {
    public:
        // Hell, yeah!
        Network(size_t vertexCount, vertex source, vertex target) :source_(source), target_(target) {
            this->pushVertex(vertexCount);
            if (!this->isVertex(source_) || !this->isVertex(target_)) {
                throw std::runtime_error("Network:: source or target out of bounds");
            }
        }

        vertex getSource() const {
            return source_;
        }

        vertex getTarget() const {
            return target_;
        }

        virtual void pushEdge(E edge) {
            if (this->isVertex(edge.from()) && this->isVertex(edge.to())) {
                E backEdge(edge.to(), edge.from(), 0);
                Graph<E>::pushEdge(edge);
                Graph<E>::pushEdge(backEdge);
            } else {
                throw std::runtime_error("Graph:: vertex out of bounds");
            }
        }

    private:
        vertex source_;
        vertex target_;
    };

    template <>
    void Network<EdgeCostFlow>::pushEdge(EdgeCostFlow edge) {
        if (this->isVertex(edge.from()) && this->isVertex(edge.to())) {
            EdgeCostFlow backEdge(edge.to(), edge.from(), 0, 0 - edge.cost());
            Graph<EdgeCostFlow>::pushEdge(edge);
            Graph<EdgeCostFlow>::pushEdge(backEdge);
        } else {
            throw std::runtime_error("Graph:: vertex out of bounds");
        }
    }

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
        template <class E>
        static void bfsUntil(Graph<E>& g, vertex v, vertex to, std::vector<vertex>& level) {
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
    template<class Network>
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
        std::vector<typename Network::base_iterator> ptr_;

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
// dijkstra.h
////////////////////////////////////////////////////////////

namespace sgtl{
    template <class E, class Weight>
    void dijkstra(Graph<E> g, vertex v, std::vector<int> &shortest, std::function<int(const EdgeCostFlow& e)> cost, Weight INF) {
        shortest.assign(g.vertexCount(), INF);
        shortest[v] = 0;
        std::priority_queue< std::pair<Weight, vertex> > qq;
        qq.push(std::make_pair(Weight(), v));
        while (!qq.empty()) {
            std::pair <Weight, vertex> q = qq.top(); qq.pop();
            Weight v = q.second;
            Weight vCost = -q.first;
            if (shortest[q.second] == vCost) {
                for (auto e = g.begin(v); e != g.end(v); ++e) {
                    Weight newCost = vCost + cost(*e);
                    if (shortest[(*e).to()] > newCost) {
                        shortest[(*e).to()] = newCost;
                        qq.push(std::make_pair(-newCost, (*e).to()));
                    }
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////
// mincostmaxflow.h
////////////////////////////////////////////////////////////


namespace sgtl{
    template <class E, class Weight>
    void fordBellman(Graph<E> g, vertex v, std::vector<int> &shortest,
                     std::function<int(const EdgeCostFlow& e)> cost, Weight INF) {
        shortest.assign(g.vertexCount(), INF);
        shortest[v] = Weight();
        bool modified = true;
        while (modified) {
            modified = false;
            for (vertex v = 0; v < g.vertexCount(); ++v) {
                for (auto e = g.begin(v); e != g.end(v); ++e) {
                    Weight c = cost(*e);
                    if ((c != INF) && (shortest[(*e).to()] > shortest[v] + c)) {
                        shortest[(*e).to()] = shortest[v] + c;
                        modified = true;
                    }
                }
            }
        }
    }

    bool dfsPushMinCostFlow(Network<EdgeCostFlow>& g,
                            vertex v, const std::vector<int>& shortest,
                            std::function<int(const EdgeCostFlow& e)>& cost, std::vector<int>& order) {
        order[v] = 1;
        if (v == g.getTarget()) {
            return true;
        }
        for (auto e = g.begin(v); e != g.end(v); ++e) {
            if ((*e).unsaturated() && (!order[(*e).to()])
                    && (shortest[v] + cost(*e) == shortest[(*e).to()]) ) {
                if (dfsPushMinCostFlow(g, (*e).to(), shortest, cost, order)) {
                    e.pushFlow(1);
                    return true;
                }
            }
        }
        return false;
    }

    void dfsPushMinCostFlow(Network<EdgeCostFlow>& g, const std::vector<int>& shortest,
                            std::function<int(const EdgeCostFlow& e)>& cost) {
        std::vector<int> order(g.vertexCount());
        dfsPushMinCostFlow(g, g.getSource(), shortest, cost, order);
    }

    void minCostMaxFlow(Network<EdgeCostFlow>& g, flow_t& flow, int& cost, int INF) {
        std::vector<int> p;
        std::vector<int> shortest;
        flow = cost = 0;
        std::function<int(const EdgeCostFlow& e)> costFBLambda = [&p, INF](const EdgeCostFlow& e) -> int {
            if (e.unsaturated()) {
                return e.cost();
            } else {
                return INF;
            }
        };
        fordBellman<EdgeCostFlow, int>(g, g.getSource(), p, costFBLambda, INF);
        std::function<int(const EdgeCostFlow& e)> costDLambda = [&p, INF](const EdgeCostFlow& e) -> int {
            if (e.unsaturated()) {
                return (int)(p[e.from()] - p[e.to()] + e.cost());
            } else {
                return INF;
            }
        };

        while (true) {
            dijkstra<EdgeCostFlow, int>(g, g.getSource(), shortest, costDLambda,  INF);
            if (shortest[g.getTarget()] == INF) {
                break;
            }
            dfsPushMinCostFlow(g, shortest, costDLambda);
            flow += 1;
            cost += (shortest[g.getTarget()] + p[g.getTarget()] - p[g.getSource()]);
            for (vertex v = 0; v < g.vertexCount(); ++v) {
                p[v] += (shortest[v] != INF) ? shortest[v] : shortest[g.getTarget()];
            }
        }
    }
}


////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////

#include <algorithm>

using namespace sgtl;
using std::vector;
using std::cin;
using std::cout;

int main() {
    int n, k;
    cin >> n >> k;
    Network<EdgeCostFlow> g(2*n + 2, 2*n, 2*n + 1);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int v;
            cin >> v;
            g.pushEdge(EdgeCostFlow(i, j + n, 1, v));
        }
    }

    for (int i = 0; i < n; ++i) {
        g.pushEdge(EdgeCostFlow(g.getSource(), i, k, 0));
        g.pushEdge(EdgeCostFlow(i + n, g.getTarget(), k, 0));
    }

    flow_t flow;
    int cost;
    minCostMaxFlow(g, flow, cost, (int)1E6);

    vector <vector<int>> ans(n);
    for (int i = 0; i < n; ++i) {
        for (auto e = g.begin(i); e != g.end(i); ++e) {
            if (!(*e).unsaturated()) {
                ans[i].push_back((*e).to());
            }
        }
    }

    Network<EdgeFlowMarked> gP(2*n + 2, 2*n, 2*n + 1);
    for (int i = 0; i < n; ++i) {
        for (auto e = g.begin(i); e != g.end(i); ++e) {
            if (!(*e).unsaturated()) {
                gP.pushEdge(EdgeFlowMarked((*e).from(), (*e).to(), 1));
            }
        }
    }


    cout << cost << "\n";
    while (k--) {
        for (int i = 0; i < n; ++i) {
            gP.pushEdge(EdgeFlowMarked(gP.getSource(), i, 1));
            gP.pushEdge(EdgeFlowMarked(n + i, gP.getTarget(), 1));
        }
        Dinitz<Network<EdgeFlowMarked>>::getMaxFlow(gP);
        for (int i = 0; i < n; ++i) {
            for (auto e = gP.begin(i); e != gP.end(i); ++e) {
                if (!(*e).unsaturated() && (!e.getMark())) {
                    e.mark(true);
                    cout << (*e).to() - n + 1 << " ";
                    break;
                }
            }
        }
        cout << "\n";
    }
    return 0;
}
