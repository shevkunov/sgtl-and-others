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
    typedef long long cost_t;

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
        EdgeCostFlow(vertex from, vertex to, flow_t capacity, cost_t cost)
            : EdgeFlow(from, to, capacity), cost_(cost) {
        }
        cost_t cost() const {
            return cost_;
        }

    private:
        cost_t cost_;
    };

    class EdgeFlowMarked : public EdgeFlow {
    public:
        EdgeFlowMarked(vertex from, vertex to, flow_t capacity)
            : EdgeFlow(from, to, capacity), mark(0) {
        }
        EdgeFlowMarked(vertex from, vertex to, flow_t capacity, int mark)
            : EdgeFlow(from, to, capacity), mark(mark) {
        }
        int mark;
    };

    class EdgeCostFlowMarked : public EdgeCostFlow {
    public:
        EdgeCostFlowMarked(vertex from, vertex to, flow_t capacity, cost_t cost)
            : EdgeCostFlow(from, to, capacity, cost), mark(0) {
        }
        EdgeCostFlowMarked(vertex from, vertex to, flow_t capacity, cost_t cost, int mark)
            : EdgeCostFlow(from, to, capacity, cost), mark(mark) {
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

    template <>
    class GraphIterator<EdgeCostFlowMarked>: public GraphBaseIterator<EdgeCostFlowMarked> {
    public:
        GraphIterator(Graph<EdgeCostFlowMarked>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<EdgeCostFlowMarked>(graph, v, shift) {
        }
        GraphIterator()
            : GraphBaseIterator<EdgeCostFlowMarked>() {
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

    flow_t GraphIterator<EdgeCostFlowMarked>::pushFlow(flow_t flow) {
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

    void GraphIterator<EdgeCostFlowMarked>::mark(int v) {
        if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
            graph_->edgeList_[graph_->edges_[v_][shift_]].mark = v;
        } else {
            throw std::runtime_error("Graph:: iterator:: end() called");
        }
    }

    int GraphIterator<EdgeCostFlowMarked>::getMark() const {
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

    template <>
    void Network<EdgeCostFlowMarked>::pushEdge(EdgeCostFlowMarked edge) {
        if (this->isVertex(edge.from()) && this->isVertex(edge.to())) {
            EdgeCostFlowMarked backEdge(edge.to(), edge.from(), 0, 0 - edge.cost());
            Graph<EdgeCostFlowMarked>::pushEdge(edge);
            Graph<EdgeCostFlowMarked>::pushEdge(backEdge);
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
    void dijkstra(Graph<E>& g, vertex v, std::vector<Weight> &shortest, std::function<Weight(const E& e)> cost, Weight INF) {
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
    void fordBellman(Graph<E>& g, vertex v, std::vector<Weight> &shortest,
                     std::function<Weight(const E& e)> cost, Weight INF) {
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


    template <class EdgeCostFlow>
    bool dfsPushMinCostFlow(Network<EdgeCostFlow>& g, vertex v, const std::vector<cost_t>& shortest,
                            std::function<cost_t(const EdgeCostFlow& e)>& cost, std::vector<cost_t>& order) {
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

    template<class EdgeCostFlow>
    void dfsPushMinCostFlow(Network<EdgeCostFlow>& g, const std::vector<cost_t>& shortest,
                            std::function<cost_t(const EdgeCostFlow& e)>& cost) {
        std::vector<cost_t> order(g.vertexCount());
        dfsPushMinCostFlow(g, g.getSource(), shortest, cost, order);
    }

    template<class EdgeCostFlow>
    void minCostMaxFlow(Network<EdgeCostFlow>& g, flow_t& flow, cost_t& cost, cost_t INF) {
        std::vector<cost_t> p;
        std::vector<cost_t> shortest;
        flow = cost = 0;
        std::function<cost_t(const EdgeCostFlow& e)> costFBLambda = [&p, INF](const EdgeCostFlow& e) -> cost_t {
            if (e.unsaturated()) {
                return e.cost();
            } else {
                return INF;
            }
        };
        fordBellman<EdgeCostFlow, cost_t>(g, g.getSource(), p, costFBLambda, INF);
        std::function<cost_t(const EdgeCostFlow& e)> costDLambda = [&p, INF](const EdgeCostFlow& e) -> cost_t {
            if (e.unsaturated()) {
                return (int)(p[e.from()] - p[e.to()] + e.cost());
            } else {
                return INF;
            }
        };

        while (true) {
            dijkstra<EdgeCostFlow, cost_t>(g, g.getSource(), shortest, costDLambda,  INF);
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
// RAW
////////////////////////////////////////////////////////////

template<class E>
void dfsDecompose01Flow(sgtl::Network<E>& g, std::vector<E>& ans, sgtl::vertex v) {
    for (auto e = g.begin(v); e != g.end(v); ++e) {
        if (!(*e).unsaturated() && ((*e).mark > 0) && ((*e).capacity() > 0)) {
            e.mark(0 - (*e).mark);
            ans.push_back((*e));
            dfsDecompose01Flow(g, ans, (*e).to());
            return;
        }
    }
}

template<class E>
std::vector<std::vector<E>> decompose01Flow(sgtl::Network<E>& g) {
    std::vector<std::vector<E>> ans;
    for (auto e = g.begin(g.getSource()); e != g.end(g.getSource()); ++e) {
        if (!(*e).unsaturated() && ((*e).mark > 0) && ((*e).capacity() > 0)) {
            e.mark(0 - (*e).mark);
            ans.push_back(std::vector<E>());
            ans.back().push_back((*e));
            dfsDecompose01Flow(g, ans.back(), (*e).to());
        }
    }
    return ans;
}

////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////

#include <algorithm>

using namespace sgtl;
using std::vector;
using std::cin;
using std::cout;

const cost_t INF = 2 * 1E9;

struct my{
    cost_t s, t, c;
    int i, ans;
};

bool s1(const my& a, const my& b) {
    return a.s < b.s;
}

bool s2(const my& a, const my& b) {
    return a.i < b.i;
}

vector<my> v;
int main() {
    int n, k;
    cin >> n >> k;
    Network<EdgeCostFlow> g(2*n + 3, 2*n + 1, 2*n + 2);

    v.resize(n);


    for (int i = 0; i < n; ++i) {
        cin >> v[i].s >> v[i].t >> v[i].c;
        v[i].i = i;
    }

    std::sort(v.begin(), v.end(), s1);

    for (int i = 0; i < n; ++i) {

        g.pushEdge(EdgeCostFlow(2*n, 2*i, 1, 0));
        g.pushEdge(EdgeCostFlow(2*i, 2*i + 1, 1, -v[i].c));
        g.pushEdge(EdgeCostFlow(2*i + 1, g.getTarget(), 1, 0));
    }

    for (int i = 0; i < n; ++i) {
        if (i + 1 < n) {
            for (int j = 0; j < k; ++j) {
                g.pushEdge(EdgeCostFlow(2*i, 2*(i+1), 1, 0));
            }
        }
        for (int j = 0; j < n; ++j) {
            if (v[i].s + v[i].t <= v[j].s) {
                g.pushEdge(EdgeCostFlow(2*i + 1, 2*j, 1, 0));
                break;
            }
        }
    }
/*
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (v[i].s + v[i].t <= v[j].s) {
                g.pushEdge(EdgeCostFlow(2*i + 1, 2*j, 1, 0));
            }
        }
    }*/

    for (int i = 0; i < k; ++i) {
        g.pushEdge(EdgeCostFlow(g.getSource(), 2*n, 1, 0));
    }

    cost_t cost;
    flow_t flow;
    minCostMaxFlow<EdgeCostFlow>(g, flow, cost, INF);
    for (int i = 0; i < n; ++i) {
        for (auto e = g.begin(2*i); e != g.end(2*i); ++e) {
            if ((*e).to() == 2*i + 1) {
                v[i].ans = (*e).flow();
            }
        }
    }

    sort(v.begin(), v.end(), s2);

    for (int i = 0; i < v.size(); ++i) {
        cout << v[i].ans << " ";
    }
    return 0;
}
