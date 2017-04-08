#include <iostream>
#include <vector>

//#include "flow.h"

#pragma once

namespace sgtl {
    typedef unsigned int vertex;
    typedef unsigned long long flow_t;
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
            flow_ = flow_ += addition;
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
     /*   base_iterator(const base_iterator& v) : pos_(v.pos_), clock_(v.clock_), data_(v.data_), isEnd_(v.isEnd_) {
            ///This construstor allows operation (const_iterator) = (iterator);
        }*/
        GraphBaseIterator(Graph<E>* const graph, vertex v, size_t shift)
            : graph_(graph), v_(v), shift_(shift) {
        }

        ///Main operators
        virtual reference operator*() {
            if (graph_->isVertex(v_) && (shift_ < graph_->edges_[v_].size())) {
                return graph_->edgeList_[graph_->edges_[v_][shift_]];
            } else {
                throw std::runtime_error("Graph:: iterator:: end() called");
            }
        }

        virtual GraphBaseIterator& operator+=(const difference_type& indx)
        {
            shift_ = std::min(shift_ + indx, graph_->edgesCount(v_));
            return *this;
        }

        ///Clone operators
        virtual GraphBaseIterator& operator++()
        {
            return (*this) += 1;
        }

        virtual GraphBaseIterator operator++(int)
        {
            GraphBaseIterator temp(*this);
            (*this) += 1;
            return temp;
        }

        virtual GraphBaseIterator operator+(const difference_type& idx) const
        {
            GraphBaseIterator buf(*this);
            return buf += idx;
        }
        virtual bool operator==(const GraphBaseIterator& other) const
        {
            return (graph_ == other.graph_) && (v_ == other.v_) && (shift_ == other.shift_);
        }
        virtual bool operator!=(const GraphBaseIterator& other) const
        {
            return !((*this) == other) ;
        }

    protected:
        Graph<E>* const graph_;
        vertex v_;
        size_t shift_;
    };

    template <class E>
    class GraphIterator: public GraphBaseIterator<E> {
    public:
        GraphIterator(Graph<E>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<E>(graph, v, shift) {
        }
    };

    template <>
    class GraphIterator<EdgeFlow>: public GraphBaseIterator<EdgeFlow> {
    public:
        GraphIterator(Graph<EdgeFlow>* const graph, vertex v, size_t shift)
            : GraphBaseIterator<EdgeFlow>(graph, v, shift) {
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

        virtual size_t vertexCount() const {
            return edges_.size();
        }

        virtual size_t edgesCount() const {
            return edgeList_.size();
        }

        virtual size_t edgesCount(vertex v) const {
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

        virtual base_iterator getEdge(vertex v, size_t shift) {
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
            graph_->edgeList_[backEdgeIndex].pushFlow(pushed);
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


/*
    class Graph {
    public:
        friend class base_iterator;
        virtual vertex pushVertex() {
            edges_.push_back(std::vector<vertex>());
            return edges_.size() - 1;
        }

        virtual void pushEdge(Edge e) {
            if (isVertex(e.from()) && isVertex(e.to())) {
                edges_[e.from()].push_back(edgeList_.size());
                edgeList_.push_back(e);
            } else {
                throw std::runtime_error("Graph:: vertex out of bounds");
            }
        }

        virtual size_t vertexCount() const {
            return edges_.size();
        }

        virtual size_t edgesCount() const {
            return edgeList_.size();
        }

        virtual size_t edgesCount(vertex v) const {
            return edges_[v].size();
        }

        class base_iterator: public std::iterator <std::forward_iterator_tag, const Edge> {
        public:
            typedef size_t size_type;

         //  base_iterator(const base_iterator& v) : pos_(v.pos_), clock_(v.clock_), data_(v.data_), isEnd_(v.isEnd_) {
                ///This construstor allows operation (const_iterator) = (iterator);
           // }
            base_iterator(Graph* const graph, vertex v, size_t shift)
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

            base_iterator& operator+=(const difference_type& indx)
            {
                shift_ = std::min(shift_ + indx, graph_->edgesCount(v_));
                return *this;
            }

            ///Clone operators
            base_iterator& operator++()
            {
                return (*this) += 1;
            }
            base_iterator operator++(int)
            {
                base_iterator temp(*this);
                (*this) += 1;
                return temp;
            }
            base_iterator operator+(const difference_type& idx) const
            {
                base_iterator buf(*this);
                return buf += idx;
            }
            bool operator==(const base_iterator& other) const
            {
                return (graph_ == other.graph_) && (v_ == other.v_) && (shift_ == other.shift_);
            }
            bool operator!=(const base_iterator& other) const
            {
                return !((*this) == other) ;
            }
        protected:
            Graph* const graph_;
            vertex v_;
            size_t shift_;
        };

        base_iterator begin(vertex v) {
            return base_iterator(this, v, 0);
        }

        base_iterator end(vertex v) {
            return base_iterator(this, v, edges_[v].size());
        }

        virtual base_iterator getEdge(vertex v, size_t shift) {
            if (isVertex(v) && (shift < edges_[v].size())) {
                return base_iterator(this, v, shift);
            } else {
                throw std::runtime_error("Graph:: vertex out of bounds");
            }
        }

    private:
        std::vector<std::vector<vertex>> edges_;
        std::vector<Edge> edgeList_;
        bool isVertex(vertex v) const {
            return v < vertexCount();
        }

    };*/




////////////////////////////////////////////////////////////


#pragma once
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

            while (true) {
                level_ = bfs_<EdgeFlow>(g, g.getSource());
                if (level_[g.getTarget()] == 0) {
                    break;
                }
                ptr_.clear();
                for (vertex v = 0; v < g.vertexCount(); ++v) {
                    ptr_.push_back(g.begin(v));
                }
                while (push = dfsPushFlow_(g, ~(flow_t)0)) {
                    flow += push;
                }
            }
            return flow;
        }

        template <class E>
        std::vector<vertex> bfs_(Graph<E>& g, vertex v) {
            if (!g.isVertex(v)) {
                throw std::runtime_error("bsf :: vertex out of bounds");
            }
            std::vector<vertex> level(g.vertexCount(), 0);
            level[v] = 1;

            std::queue <vertex> qq;
            qq.push(v);

            while (!qq.empty()) { // RMRFME while ((!qq.empty()) && (level[n] == 0)) {
                v = qq.front();
                qq.pop();

                for (auto edge = g.begin(v); edge != g.end(v); ++edge) {
                    if ((level[(*edge).to()] == 0) && (*edge).unsaturated()) {
                        level[(*edge).to()] = level[v] + 1;
                        qq.push((*edge).to());
                    }
                }
            }
            return level;
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
                        if (pushed = dfsPushFlow_(g, tryPushFlow, (*ptr_[v]).to())) {
                            ptr_[v].pushFlow(pushed);
                            return pushed;
                        }
                    }
            }
            return 0;
        }

    };
}

int main() {
    while (true) {
        int n;
        std::cin >> n;
        if (n == 0) {
            return 0;
        }

        int s = 1, t = n, c;
        std::cin >> /*s >> t >>*/ c;
        sgtl::Network g(n, --s, --t);

        while (c--) {
            int a, b, capacity;
            std::cin >> a >> b >> capacity;
            --a; --b; // Zero-numeration
            g.pushEdge(sgtl::EdgeFlow(a, b, capacity));
            //g.pushEdge(sgtl::EdgeFlow(b, a, capacity));
        }
        std::cout << sgtl::Dinitz::getMaxFlow(g) << "\n";
        return 0;
    }
}

