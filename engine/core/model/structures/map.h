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

#ifndef FIFE_MAP_MAP_H
#define FIFE_MAP_MAP_H

// Standard C++ library includes
#include <string>
#include <vector>

// 3rd party library includes

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "util/base/attributedclass.h"
#include "util/base/resourceclass.h"
#include "util/resource/resource.h"
#include "model/metamodel/timeprovider.h"

#include "location.h"

namespace FIFE {

	class Layer;
	class CellGrid;
	class Map;
	class Dataset;

	/** Listener interface for changes happening on map
	 */
	class MapChangeListener {
	public:
		virtual ~MapChangeListener() {};
		
		/** Called when some layer is changed on map. @see LayerChangeListener
		 * Layer is effectively changed, in case some of its instances
		 * is created, deleted or changed during latest update cycle
		 * @param map where change occurred
		 * @param changedLayers list of layers containing some changes
		 * @note Does not report layer creations and deletions
		 */
		virtual void onMapChanged(Map* map, std::vector<Layer*>& changedLayers) = 0;
		
		/** Called when some layer gets created on the map
		 * @param map where change occurred
		 * @param layer which got created
		 */
		virtual void onLayerCreate(Map* map, Layer* layer) = 0;
		
		/** Called when some instance gets deleted on layer
		 * @param map where change occurred
		 * @param layer which will be deleted
		 * @note right after this call, layer actually gets deleted!
		 */
		virtual void onLayerDelete(Map* map, Layer* layer) = 0;
	};

	/** A container of \c Layer(s).
	 *
	 * The actual data is contained in \c Layer objects
	 * @see Layer
	 */
	class Map : public AttributedClass {
		public:

			/** Construct a map
			 * To add map to model, one should call Model::addMap (otherwise
			 * map is not registered with the engine properly)
			 */
			Map(const std::string& identifier, TimeProvider* tp_master=NULL);

			/** Destructor
			 */
			~Map();

			/** Adds dataset into map. Does not transfer ownership
			 */
			void addDataset(Dataset* dataset);

			/** Get the datasets used by this map.
			 */
			std::list<Dataset*> getDatasets();

			/** Get the datasets used by this map, and recursively get any
			 * datasets referred to by these datasets.
			 */
			std::list<Dataset*>  getDatasetsRec();

			/** Add a Layer to this Map. Map owns
			 * the returned pointer to the new Layer, so don't
			 * delete it!
			 */
			Layer* createLayer(const std::string& identifier, CellGrid* grid);

			/** Delete a layer from the map
			 */
			void deleteLayer(Layer*);

			/** Get the layers on this map.
			 */
			std::list<Layer*> getLayers() const;

			/** Get a set of layers by a value.
			 *
			 * @param the field to search on
			 * @param the value to be searched for in the field
			 */
			std::list<Layer*> getLayers(const std::string& field, const std::string& value) const;

			/** Get all layers
			 */
			const std::vector<Layer*>& getLayers() { return m_layers; }

			/** Get the overall number of layers
			 */
			size_t getNumLayers() const;

			/** Delete all layers from the map
			 */
			void deleteLayers();

			/** Maps coordinate from one layer to another
			 */
			void getMatchingCoordinates(const ModelCoordinate& coord_to_map, const Layer* from_layer, 
				const Layer* to_layer, std::vector<ModelCoordinate>& matching_coords) const;

			/** Called periodically to update events on map
			 * @returns true, if map was changed
			 */
			bool update();
			
			/** Sets speed for the map. See Model::setTimeMultiplier.
			 */
			void setTimeMultiplier(float multip) { m_timeprovider.setMultiplier(multip); }
			
			/** Gets model speed. @see setTimeMultiplier.
			 */
			float getTimeMultiplier() const { return m_timeprovider.getMultiplier(); }
			
			/** Gets timeprovider used in the map
			 */
			TimeProvider* getTimeProvider() { return &m_timeprovider; }
			
			/** Adds new change listener
			* @param listener to add
			*/
			void addChangeListener(MapChangeListener* listener);
	
			/** Removes associated change listener
			* @param listener to remove
			*/
			void removeChangeListener(MapChangeListener* listener);
			
			/** Returns true, if map information was changed during previous update round
			*/
			bool isChanged() { return !m_changedlayers.empty(); }

			/** Returns layers that were changed during previous update round
			*/
			std::vector<Layer*>& getChangedLayers() { return m_changedlayers; }

		private:
			std::string m_mapname;
			std::vector<Dataset*> m_datasets;
			std::vector<Layer*> m_layers;
			TimeProvider m_timeprovider;

			Map(const Map& map);
			Map& operator=(const Map& map);
	
			// listeners for map changes
			std::vector<MapChangeListener*> m_changelisteners;

			// holds changed layers after each update
			std::vector<Layer*> m_changedlayers;
			
			// true, if something was changed on map during previous update (layer change, creation, deletion)
			bool m_changed;
	};

} //FIFE

#endif
/* vim: set noexpandtab: set shiftwidth=2: set tabstop=2: */
