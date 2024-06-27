#pragma once
/*
 * Класс, реализующий взвешенный ориентированный граф
 * 1) Вершины и рёбра графа нумеруются автоинкрементно беззнаковыми целыми числами, хранящимися в типах VertexId и EdgeId.
 * Вершины нумеруются от нуля до количества вершин минус один в соответствии с пользовательской логикой.
 * 2) Номер очередного ребра выдаётся методом AddEdge, он равен нулю для первого вызова метода и при каждом следующем вызове увеличивается на единицу.
 * 3) Память, нужная для хранения графа, линейна относительно суммы количеств вершин и рёбер.
 * 4) Конструктор и деструктор графа имеют линейную сложность, а остальные методы константны или амортизированно константны.
 *
 * Вершины графа хранятся в DirectedWeightedGraph::incidence_lists_, который представляет собой std::vector<std::vector<EdgeId>>
 * Номер вершины - это индекс в векторе.
 * Рёбра графа хранятся в DirectedWeightedGraph::edges_, который представляет собой std::vector<Edge<Weight>> (требуется указание типа поля weight в структуре Edge)
 */
#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count);
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    size_t GetEdgeCount() const;
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

/*
 * При добавлении ребра, для вершины from привязываем edge_id добавленного ребра
 */
template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph