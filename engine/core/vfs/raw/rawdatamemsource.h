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

#ifndef FIFE_VFS_RAW_RAWDATAMEMSOURE_H
#define FIFE_VFS_RAW_RAWDATAMEMSOURE_H

// Standard C++ library includes

// Platform specific includes
#include "util/base/fife_stdint.h"

// 3rd party library includes

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "rawdatasource.h"

namespace FIFE {

	/**
	 *	Simpe RawDataSource that reads data from raw memory
	 * 
	 * @note This Class is @b not incomplete. Use @c getRawData to
	 * write into the Buffer.
	 *
	 * @see RawDataSource
	 */
	class RawDataMemSource : public RawDataSource {
		public:
			/**
			 *	Create a new RawDataMemSource that allocates datalen bytes.
			 * @param datalen the datalen to allocate
			 */
			RawDataMemSource(unsigned int datalen);
			virtual ~RawDataMemSource();

			/** Get a Pointer to the data
			 * You need to use this function to fill the class with meaningfull data.
			 */
			uint8_t* getRawData() const;

			virtual unsigned int getSize() const;
			virtual void readInto(uint8_t* buffer, unsigned int start, unsigned int length);

		private:
			uint8_t* m_data;
			unsigned int m_datalen;

			RawDataMemSource(const RawDataMemSource&) {};
			RawDataMemSource& operator=(const RawDataMemSource) { return *this; }
	};

}

#endif
