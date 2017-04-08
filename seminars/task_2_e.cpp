#include <iostream>
#include <vector>
#include <exception>
#include <stdexcept>
#include <climits>

//#include "flow.h"



////////////////////////////////////////////////////////////
// graph.h
////////////////////////////////////////////////////////////

//#pragma once

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
    protected:
    };

    template <class E>
    class Graph {
    public:
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
// flow.h
////////////////////////////////////////////////////////////
//#pragma once
//#include "graph.h"
#include <queue>

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
                bfsUntil_<EdgeFlow>(g, g.getSource(), g.getTarget(), level_);
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

        template <class E>
        void bfsUntil_(Graph<E>& g, vertex v, vertex to, std::vector<vertex>& level) {
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

bool broken(char c) {
    return c == '*';
}

sgtl::vertex vGet(int i, int j, int n, int m) {
    (void)n;
    return i*m + j;
}

int main() {
    size_t n, m;
    int a, b;
    int brokenCount = 0;
    std::cin >> n >> m >> a >> b;

    std::vector<std::string> mas(n);
    sgtl::Network g(n*m + 2, n*m, n*m + 1);

    for (size_t i = 0; i < n; ++i) {
        std::cin >> mas[i];
        for (size_t j = 0; j < mas[i].size(); ++j) {
            brokenCount += broken(mas[i][j]);
        }
    }

    if (b * 2 <= a) {
        std::cout << brokenCount * b << "\n";
        return 0;
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j< m; ++j) {
            if ((i + j) % 2) {
                g.pushEdge(sgtl::EdgeFlow(g.getSource(), vGet(i, j, n, m), 1));
            } else {
                g.pushEdge(sgtl::EdgeFlow(vGet(i, j, n, m), g.getTarget(), 1));
            }
        }
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            if (((i + j) % 2) && broken(mas[i][j])) {
                if ((j > 0)  && broken(mas[i][j - 1])) {
                    g.pushEdge(sgtl::EdgeFlow(vGet(i, j, n, m), vGet(i, j - 1, n, m), 1));
                }
                if ((i > 0)  && broken(mas[i - 1][j])) {
                    g.pushEdge(sgtl::EdgeFlow(vGet(i, j, n, m), vGet(i - 1, j, n, m), 1));
                }
                if ((j + 1 < m) && broken(mas[i][j+1])) {
                    g.pushEdge(sgtl::EdgeFlow(vGet(i, j, n, m), vGet(i, j + 1, n, m), 1));
                }
                if ((i + 1 < n) && broken(mas[i + 1][j])) {
                    g.pushEdge(sgtl::EdgeFlow(vGet(i, j, n, m), vGet(i + 1, j, n, m), 1));
                }
            }
        }
    }

    sgtl::flow_t flow = sgtl::Dinitz::getMaxFlow(g);

    std::cout << (brokenCount - 2 * flow) * b + flow * a << "\n";
    return 0;
}
