#include <iostream>
#include <vector>

#include <graph.h>


using namespace std;

int main() {
    sgtl::Graph<sgtl::Edge> g;
    g.pushVertex();
    g.pushVertex();
    g.pushEdge(sgtl::Edge(0, 1));
    for (auto v = g.begin(0); v != g.end(0); ++v) {
        cout << (*v).to() << "\n";
    }

    sgtl::Network n;
    n.pushVertex();
    n.pushVertex();
    n.pushEdge(sgtl::EdgeFlow(0, 1, 255));
    for (auto v = n.begin(0); v != n.end(0); ++v) {
        cout << (*v).from() << " " << (*v).to() << " " << (*v).flow() << " " << (*v).capacity() << "\n";
        v.pushFlow(1);
        cout << (*v).from() << " " << (*v).to() << " " << (*v).flow() << " " << (*v).capacity() << "\n";
    }
    return 0;
}
