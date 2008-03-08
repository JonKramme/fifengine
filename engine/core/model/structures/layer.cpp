/***************************************************************************
 *   Copyright (C) 2005-2008 by the FIFE team                              *
 *   http://www.fifengine.de                                               *
 *   This file is part of FIFE.                                            *
 *                                                                         *
 *   FIFE is free software; you can redistribute it and/or modify          *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA              *
 ***************************************************************************/

// Standard C++ library includes

// 3rd party library includes
#include <SDL.h>

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "util/structures/purge.h"

#include "layer.h"
#include "instance.h"
#include "map.h"
#include "instancetree.h"

namespace FIFE {

	Layer::Layer(const std::string& identifier, Map* map, CellGrid* grid)
		: AttributedClass(identifier),
		m_map(map),
		m_instances_visibility(true),
		m_instanceTree(new InstanceTree()),
		m_grid(grid),
		m_pathingstrategy(CELL_EDGES_ONLY),
		m_changelisteners(),
		m_changedinstances(),
		m_changed(false) {
	}

	Layer::~Layer() {
		purge(m_instances);
		delete m_instanceTree;
	}

	bool Layer::hasInstances() const {
		return !m_instances.empty();
	}

	Instance* Layer::createInstance(Object* object, const ModelCoordinate& p, const std::string& id) {
		ExactModelCoordinate emc(static_cast<double>(p.x), static_cast<double>(p.y), static_cast<double>(p.z));
		return createInstance(object, emc, id);	
	}

	Instance* Layer::createInstance(Object* object, const ExactModelCoordinate& p, const std::string& id) {
		Location l;
		l.setLayer(this);
		l.setExactLayerCoordinates(p);

		Instance* instance = new Instance(object, l, id);
		m_instances.push_back(instance);
		m_instanceTree->addInstance(instance);
		
		std::vector<LayerChangeListener*>::iterator i = m_changelisteners.begin();
		while (i != m_changelisteners.end()) {
			(*i)->onInstanceCreate(this, instance);
			++i;
		}
		m_changed = true;
		return instance;
	}
	
	void Layer::deleteInstance(Instance* instance) {
		std::vector<LayerChangeListener*>::iterator i = m_changelisteners.begin();
		while (i != m_changelisteners.end()) {
			(*i)->onInstanceDelete(this, instance);
			++i;
		}
	
		std::vector<Instance*>::iterator it = m_instances.begin();
		for(; it != m_instances.end(); ++it) {
			if(*it == instance) {
				m_instanceTree->removeInstance(*it);
				delete *it;
				m_instances.erase(it);
				break;
			}
		}
		m_changed = true;
	}

	const std::vector<Instance*>& Layer::getInstances() {
		return m_instances;
	}

	ModelCoordinate parse_point(const std::string& value) {
		size_t delim = value.find(',');
		std::istringstream strx(value.substr(0,delim));
		std::istringstream stry(value.substr(delim + 1,value.length()));
		ModelCoordinate pt;
		strx >> pt.x;
		stry >> pt.y;
		return pt;
	}
	std::vector<Instance*> Layer::getInstances(const std::string& field, const std::string& value) {
		std::vector<Instance*> matches;

		if(field == "loc") {
			ModelCoordinate pt = parse_point(value);
			std::vector<Instance*>::iterator it = m_instances.begin();
			for(; it != m_instances.end(); ++it) {
				Location& loc = (*it)->getLocationRef();
				if(loc.getLayerCoordinates(this) == pt)
					matches.push_back(*it);
			}

			return matches;
		}

		std::vector<Instance*>::iterator it = m_instances.begin();
		for(; it != m_instances.end(); ++it) {
			if((*it)->get(field) == value)
				matches.push_back(*it);
		}

		return matches;
	}

	void Layer::getMinMaxCoordinates(ModelCoordinate& min, ModelCoordinate& max, const Layer* layer) const {
		
		if(layer == 0) {
			layer = this;
		}
		
		min = m_instances.front()->getLocationRef().getLayerCoordinates(layer);
		max = min;

		for(std::vector<Instance*>::const_iterator i = m_instances.begin();
			i != m_instances.end();
			++i) {

			ModelCoordinate coord = (*i)->getLocationRef().getLayerCoordinates(layer);

			if(coord.x < min.x) {
				min.x = coord.x;
			}
			
			if(coord.x > max.x) {
				max.x = coord.x;
			}

			if(coord.y < min.y) {
				min.y = coord.y;
			}
			
			if(coord.y > max.y) {
				max.y = coord.y;
			}
		}
	}

	void Layer::setInstancesVisible(bool vis) {
		m_instances_visibility = vis;
	}
	void Layer::toggleInstancesVisible() {
		m_instances_visibility = !m_instances_visibility;
	}

	bool Layer::cellContainsBlockingInstance(const ModelCoordinate& cellCoordinate) {
		std::list<Instance*> adjacentInstances;
		m_instanceTree->findInstances(cellCoordinate, 0, 0, adjacentInstances);
		bool blockingInstance = false;
		for(std::list<Instance*>::const_iterator j = adjacentInstances.begin(); j != adjacentInstances.end(); ++j) {
			if((*j)->getObject()->isBlocking() && (*j)->getLocationRef().getLayerCoordinates() == cellCoordinate) {
				blockingInstance = true;
			}
		}
		return blockingInstance;
	}

	bool Layer::update() {
		m_changedinstances.clear();
		unsigned int curticks = SDL_GetTicks();
		std::vector<Instance*>::iterator it = m_instances.begin();
		for(; it != m_instances.end(); ++it) {
			if ((*it)->update(curticks) != ICHANGE_NO_CHANGES) {
				m_changedinstances.push_back(*it);
				m_changed = true;
			}
		}
		if (!m_changedinstances.empty()) {
			std::vector<LayerChangeListener*>::iterator i = m_changelisteners.begin();
			while (i != m_changelisteners.end()) {
				(*i)->onLayerChanged(this, m_changedinstances);
				++i;
			}
			//std::cout << "Layer named " << Id() << " changed = 1\n";
		}
		//std::cout << "Layer named " << Id() << " changed = 0\n";
		bool retval = m_changed;
		m_changed = false;
		return retval;
	}
	
	void Layer::addChangeListener(LayerChangeListener* listener) {
		m_changelisteners.push_back(listener);
	}

	void Layer::removeChangeListener(LayerChangeListener* listener) {
		std::vector<LayerChangeListener*>::iterator i = m_changelisteners.begin();
		while (i != m_changelisteners.end()) {
			if ((*i) == listener) {
				m_changelisteners.erase(i);
				return;
			}
			++i;
		}
	}
} // FIFE
