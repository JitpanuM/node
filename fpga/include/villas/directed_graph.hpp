#pragma once

#include <map>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "log.hpp"


namespace villas {
namespace graph {

// use vector indices as identifiers

// forward declarations
class Edge;
class Vertex;


class Vertex {
	template<typename VertexType, typename EdgeType>
	friend class DirectedGraph;

public:
	using Identifier = std::size_t;

	friend std::ostream&
	operator<< (std::ostream& stream, const Vertex& vertex)
	{ return stream << vertex.id; }

	bool
	operator==(const Vertex& other)
	{ return this->id == other.id; }

private:
	Identifier id;
	// HACK: how to resolve this circular type dependency?
	std::list<std::size_t> edges;
};


class Edge {
	template<typename VertexType, typename EdgeType>
	friend class DirectedGraph;

public:
	using Identifier = std::size_t;

	friend std::ostream&
	operator<< (std::ostream& stream, const Edge& edge)
	{ return stream << edge.id; }

	bool
	operator==(const Edge& other)
	{ return this->id == other.id; }

private:
	Identifier id;
	Vertex::Identifier from;
	Vertex::Identifier to;
};


template<typename VertexType = Vertex, typename EdgeType = Edge>
class DirectedGraph {
public:

	using VertexIdentifier = Vertex::Identifier;
	using EdgeIdentifier = Edge::Identifier;
	using Path = std::list<EdgeIdentifier>;

	DirectedGraph(const std::string& name = "DirectedGraph") :
	    lastVertexId(0), lastEdgeId(0)
	{
		logger = loggerGetOrCreate(name);
	}

	std::shared_ptr<VertexType> getVertex(VertexIdentifier vertexId) const
	{
		if(vertexId >= lastVertexId)
			throw std::invalid_argument("vertex doesn't exist");

		// cannot use [] operator, because creates non-existing elements
		// at() will throw std::out_of_range if element does not exist
		return vertices.at(vertexId);
	}

	template<class UnaryPredicate>
	VertexIdentifier findVertex(UnaryPredicate p)
	{
		for(auto& v : vertices) {
			auto& vertexId = v.first;
			auto& vertex = v.second;

			if(p(vertex)) {
				return vertexId;
			}
		}

		throw std::out_of_range("vertex not found");
	}

	std::shared_ptr<EdgeType> getEdge(EdgeIdentifier edgeId) const
	{
		if(edgeId >= lastEdgeId)
			throw std::invalid_argument("edge doesn't exist");

		// cannot use [] operator, because creates non-existing elements
		// at() will throw std::out_of_range if element does not exist
		return edges.at(edgeId);
	}

	std::size_t getEdgeCount() const
	{ return edges.size(); }

	std::size_t getVertexCount() const
	{ return vertices.size(); }

	VertexIdentifier addVertex(std::shared_ptr<VertexType> vertex)
	{
		vertex->id = lastVertexId++;

		logger->debug("New vertex: {}", *vertex);
		vertices[vertex->id] = vertex;

		return vertex->id;
	}

	EdgeIdentifier addEdge(std::shared_ptr<EdgeType> edge,
	                       VertexIdentifier fromVertexId,
	                       VertexIdentifier toVertexId)
	{
		// allocate edge id
		edge->id = lastEdgeId++;

		// connect it
		edge->from = fromVertexId;
		edge->to = toVertexId;

		logger->debug("New edge {}: {} -> {}", *edge, edge->from, edge->to);

		// this is a directed graph, so only push edge to starting vertex
		getVertex(edge->from)->edges.push_back(edge->id);

		// add new edge to graph
		edges[edge->id] = edge;

		return edge->id;
	}


	EdgeIdentifier addDefaultEdge(VertexIdentifier fromVertexId,
	                              VertexIdentifier toVertexId)
	{
		// create a new edge
		std::shared_ptr<EdgeType> edge(new EdgeType);

		return addEdge(edge, fromVertexId, toVertexId);
	}

	void removeEdge(EdgeIdentifier edgeId)
	{
		auto edge = getEdge(edgeId);
		auto startVertex = getVertex(edge->from);

		// remove edge only from starting vertex (this is a directed graph)
		logger->debug("Remove edge {} from vertex {}", edgeId, edge->from);
		startVertex->edges.remove(edgeId);

		logger->debug("Remove edge {}", edgeId);
		edges.erase(edgeId);
	}

	void removeVertex(VertexIdentifier vertexId)
	{
		// delete every edge that start or ends at this vertex
		auto it = edges.begin();
		while(it != edges.end()) {
			auto& edgeId = it->first;
			auto& edge = it->second;

			bool removeEdge = false;

			if(edge->to == vertexId) {
				logger->debug("Remove edge {} from vertex {}'s edge list",
				              edgeId, edge->from);

				removeEdge = true;

				auto startVertex = getVertex(edge->from);
				startVertex->edges.remove(edge->id);
			}

			if((edge->from == vertexId) or removeEdge) {
				logger->debug("Remove edge {}", edgeId);
				// remove edge from global edge list
				it = edges.erase(it);
			} else {
				++it;
			}
		}

		logger->debug("Remove vertex {}", vertexId);
		vertices.erase(vertexId);
	}

	const std::list<EdgeIdentifier>&
	vertexGetEdges(VertexIdentifier vertexId) const
	{ return getVertex(vertexId)->edges; }


	using check_path_fn = std::function<bool(const Path&)>;

	static bool
	checkPath(const Path&)
	{ return true; }

	bool getPath(VertexIdentifier fromVertexId,
	             VertexIdentifier toVertexId,
	             Path& path,
	             check_path_fn pathCheckFunc = checkPath)
	{
		if(fromVertexId == toVertexId) {
			// arrived at the destination
			return true;
		} else {
			auto fromVertex = getVertex(fromVertexId);

			for(auto& edgeId : fromVertex->edges) {
				auto edgeOfFromVertex = getEdge(edgeId);

				// loop detection
				bool loop = false;
				for(auto& edgeIdInPath : path) {
					auto edgeInPath = getEdge(edgeIdInPath);
					if(edgeInPath->from == edgeOfFromVertex->to) {
						loop = true;
						break;
					}
				}

				if(loop) {
					logger->debug("Loop detected via edge {}", edgeId);
					continue;
				}

				// remember the path we're investigating to detect loops
				path.push_back(edgeId);

				// recursive, depth-first search
				if(getPath(edgeOfFromVertex->to, toVertexId, path, pathCheckFunc) and
				   pathCheckFunc(path)) {
					// path found, we're done
				    return true;
				} else {
					// tear down path that didn't lead to the destination
					path.pop_back();
				}
			}
		}

		return false;
	}

	void dump()
	{
		logger->info("Vertices:");
		for(auto& v : vertices) {
			auto& vertex = v.second;

			// format connected vertices into a list
			std::stringstream ssEdges;
			for(auto& edge : vertex->edges) {
				ssEdges << getEdge(edge)->to << " ";
			}

			logger->info("  {} connected to: {}", *vertex, ssEdges.str());
		}

		logger->info("Edges:");
		for(auto& e : edges) {
			auto& edge = e.second;

			logger->info("  {}: {} -> {}", *edge, edge->from, edge->to);
		}
	}

private:
	VertexIdentifier lastVertexId;
	EdgeIdentifier lastEdgeId;

	std::map<VertexIdentifier, std::shared_ptr<VertexType>> vertices;
	std::map<EdgeIdentifier, std::shared_ptr<EdgeType>> edges;

	SpdLogger logger;
};

} // namespacae graph
} // namespace villas
